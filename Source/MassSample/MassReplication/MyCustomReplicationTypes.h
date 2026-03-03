// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "MassReplicationTransformHandlers.h"
#include "MassClientBubbleHandler.h"
#include "MassClientBubbleSerializerBase.h"

#include "MyCustomReplicationTypes.generated.h"

/**
 * 
 */

USTRUCT()
struct FMyCustomReplicatedAgent : public FReplicatedAgentBase
{
	GENERATED_BODY()

	const FReplicatedAgentPositionYawData& GetReplicatedPositionYawData() const { return PositionYaw; }
	FReplicatedAgentPositionYawData& GetReplicatedPositionYawDataMutable() { return PositionYaw; }
	
private:
	UPROPERTY()
	FReplicatedAgentPositionYawData PositionYaw; // replicated data
};

USTRUCT()
struct FMyCustomReplicatedEntityData : public FMassFastArrayItemBase
{
	GENERATED_BODY()

	FMyCustomReplicatedEntityData() = default;
	FMyCustomReplicatedEntityData(const FMyCustomReplicatedAgent& InAgent, const FMassReplicatedAgentHandle InHandle)
		: FMassFastArrayItemBase(InHandle)
		, Agent(InAgent)
	{}

	/** This typedef is required to be provided in FMassFastArrayItemBase derived classes (with the associated FReplicatedAgentBase derived class) */
	typedef FMyCustomReplicatedAgent FReplicatedAgentType;

	UPROPERTY()
	FMyCustomReplicatedAgent Agent;
};
