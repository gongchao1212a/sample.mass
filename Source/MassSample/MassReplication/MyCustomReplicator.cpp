// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCustomReplicator.h"

#include "MyCustomClientBubbleInfo.h"
#include "MassCommonFragments.h"
#include "MassReplicationFragments.h"

void UMyCustomReplicator::AddRequirements(FMassEntityQuery& EntityQuery)
{
	FMassReplicationProcessorPositionYawHandler::AddRequirements(EntityQuery);
}

void UMyCustomReplicator::ProcessClientReplication(FMassExecutionContext& Context,
	FMassReplicationContext& ReplicationContext)
{
#if UE_REPLICATION_COMPILE_SERVER_CODE

    FMassReplicationProcessorPositionYawHandler PositionYawHandler;
    FMassReplicationSharedFragment* RepSharedFrag = nullptr;

    auto CacheViewsCallback = [&RepSharedFrag, &PositionYawHandler](FMassExecutionContext& Context)
    {
        PositionYawHandler.CacheFragmentViews(Context);
        RepSharedFrag = &Context.GetMutableSharedFragment<FMassReplicationSharedFragment>();
        check(RepSharedFrag);
    };

    auto AddEntityCallback = [&RepSharedFrag, &PositionYawHandler](FMassExecutionContext& Context, const int32 EntityIdx, FMyCustomReplicatedAgent& InReplicatedAgent, const FMassClientHandle ClientHandle) -> FMassReplicatedAgentHandle
    {
        AMyCustomClientBubbleInfo& UnitBubbleInfo = RepSharedFrag->GetTypedClientBubbleInfoChecked<AMyCustomClientBubbleInfo>(ClientHandle);

        PositionYawHandler.AddEntity(EntityIdx, InReplicatedAgent.GetReplicatedPositionYawDataMutable());

        return UnitBubbleInfo.GetUnitSerializer().Bubble.AddAgent(Context.GetEntity(EntityIdx), InReplicatedAgent);
    };

    auto ModifyEntityCallback = [&RepSharedFrag, &PositionYawHandler](FMassExecutionContext& Context, const int32 EntityIdx, const EMassLOD::Type LOD, const double Time, const FMassReplicatedAgentHandle Handle, const FMassClientHandle ClientHandle)
    {
        AMyCustomClientBubbleInfo& UnitBubbleInfo = RepSharedFrag->GetTypedClientBubbleInfoChecked<AMyCustomClientBubbleInfo>(ClientHandle);

        auto& Bubble = UnitBubbleInfo.GetUnitSerializer().Bubble;

        PositionYawHandler.ModifyEntity<FMyCustomReplicatedEntityData>(Handle, EntityIdx, Bubble.GetTransformHandlerMutable());
    };

    auto RemoveEntityCallback = [&RepSharedFrag](FMassExecutionContext& Context, const FMassReplicatedAgentHandle Handle, const FMassClientHandle ClientHandle)
    {
        AMyCustomClientBubbleInfo& UnitBubbleInfo = RepSharedFrag->GetTypedClientBubbleInfoChecked<AMyCustomClientBubbleInfo>(ClientHandle);

        UnitBubbleInfo.GetUnitSerializer().Bubble.RemoveAgentChecked(Handle);
    };

    CalculateClientReplication<FMyCustomReplicatedEntityData>(Context, ReplicationContext, CacheViewsCallback, AddEntityCallback, ModifyEntityCallback, RemoveEntityCallback);
#endif // UE_REPLICATION_COMPILE_SERVER_CODE
}
