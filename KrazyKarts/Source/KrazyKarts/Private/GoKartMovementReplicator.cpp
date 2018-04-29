// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementReplicator.h"

#include "Net/UnrealNetwork.h"


// Sets default values for this component's properties
UGoKartMovementReplicator::UGoKartMovementReplicator()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	SetIsReplicated(true);
}


// Called when the game starts
void UGoKartMovementReplicator::BeginPlay()
{
	Super::BeginPlay();

	// ...
	MovementComp = GetOwner()->FindComponentByClass<UGoKartMovementComp>();
}


// Called every frame
void UGoKartMovementReplicator::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (MovementComp == nullptr) return;

	if (GetOwnerRole() == ROLE_AutonomousProxy)
	{
		FGoKartMove Move = MovementComp->CreateMove(DeltaTime);

		MovementComp->SimulateMove(Move);

		UnacknowledgedMoves.Add(Move);

		Server_SendMove(Move);
	}

	// We are the server and in control of the pawn
	if (GetOwnerRole() == ROLE_Authority && GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		FGoKartMove Move = MovementComp->CreateMove(DeltaTime);

		Server_SendMove(Move);
	}

	if (GetOwnerRole() == ROLE_SimulatedProxy) MovementComp->SimulateMove(ServerState.LastMove);
}

void UGoKartMovementReplicator::OnRep_ServerState()
{
	if (MovementComp == nullptr) return;

	GetOwner()->SetActorTransform(ServerState.Transform);
	MovementComp->SetVelocity(ServerState.Velocity);

	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComp->SimulateMove(Move);
	}
}

void UGoKartMovementReplicator::ClearAcknowledgedMoves(FGoKartMove LastMove)
{
	TArray<FGoKartMove> NewMoves;

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		if (Move.Time > LastMove.Time) NewMoves.Add(Move);
	}

	UnacknowledgedMoves = NewMoves;
}

void UGoKartMovementReplicator::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (MovementComp == nullptr) return;

	MovementComp->SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetOwner()->GetActorTransform();
	ServerState.Velocity = MovementComp->GetVelocity();
}

bool UGoKartMovementReplicator::Server_SendMove_Validate(FGoKartMove Move)
{
	// TODO: Make better validation
	return true;
}

void UGoKartMovementReplicator::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UGoKartMovementReplicator, ServerState);
}

