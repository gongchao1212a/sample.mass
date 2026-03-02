# UE5 Mass Replication 分析

基于 `MassSample` 项目和 UE5 Mass Gameplay 源代码的分析，以下是 Mass Replication 的工作原理。

## 1. 实体如何从服务器同步到客户端

Mass Entity 从服务器到客户端的同步**不是直接的 Entity 到 Entity 的复制**。相反，它结合使用了 **Proxy Actor（代理 Actor）** 和 **Fast Array Serialization（快速数组序列化）** 方法。

### 核心架构

| 组件 | 角色 | 所在端 |
| :--- | :--- | :--- |
| **`UMSReplicator`** | **驱动器**。一个 Mass Processor，负责从 ECS 系统中查询实体数据。 | 仅限服务器 |
| **`AMSUnitClientBubbleInfo`** | **数据载体**。为每个客户端连接（PlayerController）生成的标准 `AActor`。它持有 "Bubble"（气泡）数据。 | 共享 |
| **`FMSUnitClientBubbleSerializer`** | **序列化器**。包含一个 Fast Array (`Units`) 的结构体，用于高效的网络增量压缩。 | 共享 |
| **`FMSReplicatedUnitAgent`** | **载荷**。实际被复制的数据结构（例如，位置、偏航角）。 | 共享 |

### 数据管线流程

1.  **数据收集 (服务器)**
    *   `UMSReplicator` 每帧运行。
    *   它查询实体（例如，那些带有 `TransformFragment` 的实体）。
    *   它提取数据并将其推送到对应客户端（应该看到此实体的客户端）的 `AMSUnitClientBubbleInfo` 中。
    *   代码参考：`UMSReplicator::ProcessClientReplication` 查询实体并调用 `ModifyEntityCallback`。

2.  **网络传输**
    *   `AMSUnitClientBubbleInfo` Actor 使用标准的 Unreal Engine 复制功能同步其 `UnitSerializer` 属性。
    *   `FFastArraySerializer` 确保仅通过网络发送已更改的元素（增量）。

3.  **实体重建 (客户端)**
    *   客户端接收 `UnitSerializer` 更新。
    *   **添加时 (On Add)**：`PostReplicatedAdd` 触发 `SpawnerSubsystem` 在客户端的 Mass World 中生成一个 **代理实体 (Proxy Entity)**。
    *   **变更时 (On Change)**：`PostReplicatedChange` 使用接收到的数据更新本地代理实体的 Fragment（例如 `TransformFragment`）。

---

## 2. 客户端如何知道实体结构 (Trait/Fragment)

客户端能够知道实体应该拥有哪些 Fragment，是因为**共享实体模板 (Shared Entity Templates)**。

### 核心机制：模板 ID (Template ID)

1.  **共享配置**
    *   服务器和客户端在启动时都会加载相同的 `MassEntityConfig` (DataAsset)。
    *   `MassSpawnerSubsystem` 处理这些资产并注册 `MassEntityTemplate`。
    *   关键点在于，为特定资产生成的 **`FMassEntityTemplateID`** 是确定性的，并且在服务器和客户端上**完全相同**。

2.  **复制流程**
    *   `FReplicatedAgentBase` 结构体（`FMSReplicatedUnitAgent` 继承自它）包含一个 `TemplateID`。
    
    ```cpp
    struct FReplicatedAgentBase {
        FMassEntityTemplateID TemplateID; // 关键的钥匙
        // ...
    };
    ```

3.  **客户端生成**
    *   当客户端在 Fast Array 中接收到一个新代理 (Agent) 时：
        1. 读取 `TemplateID`。
        2. 使用此 ID 查找本地的 `MassEntityTemplate`。
        3. 这个本地模板告诉 Mass 系统确切使用哪个 Archetype 以及附加哪些 Fragment（生命值、可视化等）。
    *   这使得客户端能够利用完整的结构知识重建实体，即使它只从服务器接收到了一个 ID 和位置信息。

### 客户端特定的数据处理
虽然 ID 是相同的，但模板的*内容*可能会略有不同：
*   **服务器**：可能包含重逻辑的 Trait（AI、寻路）。
*   **客户端**：可能包含重表现的 Trait（可视化、声音）。
这是通过 Trait 上的 `IsServerOnly` / `IsClientOnly` 标志控制的。

## 3. 模板 ID 的存储位置与生成逻辑

客户端加载 `DataAssets` (即 `MassEntityConfig`) 后，生成的实体模板 ID (`TemplateID`) 及其对应的模板被保存在 **`UMassSpawnerSubsystem`** 子系统中。

具体存储在 `UMassSpawnerSubsystem` 的成员变量 `TemplateRegistryInstance` 中，这是一个 `FMassEntityTemplateRegistry` 类型的实例。

### 核心可确定性 (Determinism)

为什么客户端和服务器生成的 ID 是一样的？

1.  **资产源头一致**：每个 `MassEntityConfig` 资产在编辑器中创建时，都会生成一个唯一的 **GUID (`ConfigGuid`)**，并序列化在资产文件中。
2.  **ID 生成算法**：在运行时，`TemplateID` 是通过 `FMassEntityTemplateIDFactory::Make(ConfigGuid)` 计算得出的。这个函数基于资产的 GUID 进行哈希运算。
3.  **结果**：因为 GUID 在资产中是固定的，且算法是确定性的，所以无论是在服务器还是客户端加载该资产，计算出的 `TemplateID` 都是完全相同的。

### 数据结构引用

*   **注册表**：`FMassEntityTemplateRegistry`
*   **容器**：`TMap<FMassEntityTemplateID, TSharedRef<FMassEntityTemplate>> TemplateIDToTemplateMap`

