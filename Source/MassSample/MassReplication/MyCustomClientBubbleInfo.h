// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MassClientBubbleInfoBase.h"
#include "MyCustomReplicationTypes.h"
#include "MyCustomClientBubbleInfo.generated.h"

struct FMyCustomEntityBubbleSerializer;

/**
 * Handler for processing replicated data on the client.
 * Using TClientBubbleHandlerBase<FMyCustomReplicatedEntityData> matches your FastArrayItem type.
 */
class FMyCustomClientBubbleHandler : public TClientBubbleHandlerBase<FMyCustomReplicatedEntityData>
{
public:
    typedef TClientBubbleHandlerBase<FMyCustomReplicatedEntityData> Super;

    typedef TMassClientBubbleTransformHandler<FMyCustomReplicatedEntityData> FMassClientBubbleTransformHandler;
    
    FMyCustomClientBubbleHandler():TransformHandler(*this)
    {}

#if UE_REPLICATION_COMPILE_SERVER_CODE
    const FMassClientBubbleTransformHandler& GetTransformHandler() const { return TransformHandler; }
    FMassClientBubbleTransformHandler& GetTransformHandlerMutable() { return TransformHandler; }
#endif // UE_REPLICATION_COMPILE_SERVER_CODE


protected:
#if UE_REPLICATION_COMPILE_CLIENT_CODE
    /** Called when new entities are added from the server */
    virtual void PostReplicatedAdd(const TArrayView<int32> AddedIndices, int32 FinalSize) override;
    /** Called when existing entities are updated by the server */
    virtual void PostReplicatedChange(const TArrayView<int32> ChangedIndices, int32 FinalSize) override;

    
    
    void PostReplicatedChangeEntity(const FMassEntityView& EntityView, const FMyCustomReplicatedAgent& Item) const;
#endif // UE_REPLICATION_COMPILE_CLIENT_CODE

    
    FMassClientBubbleTransformHandler TransformHandler;
};

/**
 * The Fast Array Serializer that holds the array of replicated agents.
 */
USTRUCT()
struct FMyCustomEntityBubbleSerializer : public FMassClientBubbleSerializerBase
{
    GENERATED_BODY()

    FMyCustomEntityBubbleSerializer()
    {
        // Link the Bubble Handler to the Array
        Bubble.Initialize(Entities, *this);
    };

    bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParams)
    {
        return FFastArraySerializer::FastArrayDeltaSerialize<FMyCustomReplicatedEntityData, FMyCustomEntityBubbleSerializer>(Entities, DeltaParams, *this);
    }

public:
    FMyCustomClientBubbleHandler Bubble;

protected:
    /** The actual Fast Array containing our data */
    UPROPERTY(Transient)
    TArray<FMyCustomReplicatedEntityData> Entities;
};

/**
 * Trait required for Unreal's Fast Array Serialization to work with our custom serializer.
 */
template<>
struct TStructOpsTypeTraits<FMyCustomEntityBubbleSerializer> : public TStructOpsTypeTraitsBase2<FMyCustomEntityBubbleSerializer>
{
    enum
    {
        WithNetDeltaSerializer = true,
        WithCopy = false,
    };
};

/**
 * The Actor that exists on the client (one per player controller) to handle the replication channel.
 */
UCLASS()
class  AMyCustomClientBubbleInfo : public AMassClientBubbleInfoBase
{
    GENERATED_BODY()
    
public:
    AMyCustomClientBubbleInfo(const FObjectInitializer& ObjectInitializer);
    
    FMyCustomEntityBubbleSerializer& GetUnitSerializer() { return BubbleSerializer; }

protected:
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

public:
    /** The serializer instance that will be replicated */
    UPROPERTY(Replicated,Transient)
    FMyCustomEntityBubbleSerializer BubbleSerializer;
  
};