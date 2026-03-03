// Fill out your copyright notice in the Description page of Project Settings.


#include "MyCustomClientBubbleInfo.h"

#include "MassCommonFragments.h"
#include "MassEntityConfigAsset.h"
#include "MassEntitySubsystem.h"
#include "MassEntityView.h"
#include "Net/UnrealNetwork.h"

#if UE_REPLICATION_COMPILE_CLIENT_CODE
void FMyCustomClientBubbleHandler::PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize)
{
    auto AddRequirementsForSpawnQuery = [this](FMassEntityQuery& InQuery)
    {
        TransformHandler.AddRequirementsForSpawnQuery(InQuery);
    };

    auto CacheFragmentViewsForSpawnQuery = [this](FMassExecutionContext& InExecContext)
    {
        TransformHandler.CacheFragmentViewsForSpawnQuery(InExecContext);
    };

    auto SetSpawnedEntityData = [this](const FMassEntityView& EntityView, const FMyCustomReplicatedAgent& ReplicatedEntity, const int32 EntityIdx)
    {
        TransformHandler.SetSpawnedEntityData(EntityIdx, ReplicatedEntity.GetReplicatedPositionYawData());
    };

    auto SetModifiedEntityData = [this](const FMassEntityView& EntityView, const FMyCustomReplicatedAgent& Item)
    {
        PostReplicatedChangeEntity(EntityView, Item);
    };

    PostReplicatedAddHelper(AddedIndices, AddRequirementsForSpawnQuery, CacheFragmentViewsForSpawnQuery, SetSpawnedEntityData, SetModifiedEntityData);

    TransformHandler.ClearFragmentViewsForSpawnQuery();
}

void FMyCustomClientBubbleHandler::PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize)
{
    auto SetModifiedEntityData = [this](const FMassEntityView& EntityView, const FMyCustomReplicatedAgent& Item)
    {
        PostReplicatedChangeEntity(EntityView, Item);
    };

    PostReplicatedChangeHelper(ChangedIndices, SetModifiedEntityData);
    
}
#endif //UE_REPLICATION_COMPILE_SERVER_CODE

#if UE_REPLICATION_COMPILE_CLIENT_CODE
void FMyCustomClientBubbleHandler::PostReplicatedChangeEntity(const FMassEntityView& EntityView, const FMyCustomReplicatedAgent& Item) const
{
    TransformHandler.SetModifiedEntityData(EntityView, Item.GetReplicatedPositionYawData());
}
#endif //UE_REPLICATION_COMPILE_SERVER_CODE


AMyCustomClientBubbleInfo::AMyCustomClientBubbleInfo(const FObjectInitializer& ObjectInitializer)
    : Super(ObjectInitializer)
{
    bReplicates = true;
    bAlwaysRelevant = true;
    
    // Register our serializer with the base class
    Serializers.Add(&BubbleSerializer);
}

void AMyCustomClientBubbleInfo::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);

    FDoRepLifetimeParams SharedParams;
    SharedParams.bIsPushBased = true; // Use push model for efficiency

    // Register the Fast Array for replication
    DOREPLIFETIME_WITH_PARAMS_FAST(AMyCustomClientBubbleInfo, BubbleSerializer, SharedParams);
}

