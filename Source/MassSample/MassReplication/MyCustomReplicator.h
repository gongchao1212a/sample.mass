// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "MyCustomReplicationTypes.h"
#include "UObject/Object.h"

#include "MassReplicationProcessor.h"
#include "MyCustomReplicator.generated.h"

/**
 * Replicator 是真正的“搬运工”。它运行在服务端，负责把 Mass 实体的 Fragment 数据抽取出来，塞进 BubbleSerializer 中
 */
UCLASS()
class  UMyCustomReplicator : public UMassReplicatorBase
{
	GENERATED_BODY()
	
public:
	virtual void AddRequirements(FMassEntityQuery& EntityQuery) override;
	
	virtual void ProcessClientReplication(FMassExecutionContext& Context, FMassReplicationContext& ReplicationContext) override;
	
};
