// Fill out your copyright notice in the Description page of Project Settings.


#include "StateTreeTask_CustomMove.h"



#include "MassMovementFragments.h"
#include "MassStateTreeExecutionContext.h"
#include "StateTreeLinker.h"

bool FStateTreeTask_CustomMove::Link(FStateTreeLinker& Linker)
{
    Linker.LinkExternalData(Location);
    Linker.LinkExternalData(VelocityHandle);
    Linker.LinkExternalData(MassSignalSubsystemHandle);
    return true;
}

EStateTreeRunStatus FStateTreeTask_CustomMove::EnterState(FStateTreeExecutionContext& Context,
    const FStateTreeTransitionResult& Transition) const
{

    const FMassStateTreeExecutionContext& MassContext = static_cast<FMassStateTreeExecutionContext&>(Context);
    FMassVelocityFragment& MassVelocityFragment = MassContext.GetExternalData(VelocityHandle);
    MassVelocityFragment.Value = { FMath::FRandRange(-100.f, 100.f), FMath::FRandRange(-100.f, 100.f), 0 };

    UMassSignalSubsystem& MassSignalSubsystem = MassContext.GetExternalData(MassSignalSubsystemHandle);
    MassSignalSubsystem.DelaySignalEntity(UE::Mass::Signals::StandTaskFinished, MassContext.GetEntity(), 2);

    return EStateTreeRunStatus::Running;
}

EStateTreeRunStatus FStateTreeTask_CustomMove::Tick(FStateTreeExecutionContext& Context, const float DeltaTime) const
{
    return EStateTreeRunStatus::Succeeded;
}