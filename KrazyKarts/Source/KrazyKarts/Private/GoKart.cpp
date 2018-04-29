// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"

#include "Components/InputComponent.h"
#include "Engine/World.h"
#include "UnrealNetwork.h"
#include "DrawDebugHelpers.h"


// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	MovementComp = CreateDefaultSubobject<UGoKartMovementComp>(TEXT("MovementComp"));
}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
	if (HasAuthority()) NetUpdateFrequency = 1;
}

void AGoKart::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AGoKart, ServerState);
}

FString GetEnumText(ENetRole Role)
{
	switch (Role)
	{
	case ROLE_None:
		return "None";
	case ROLE_SimulatedProxy:
		return "SimulatedProxy";
	case ROLE_AutonomousProxy:
		return "AutonomousProxy";
	case ROLE_Authority:
		return "Authority";
	default:
		return "Error";
	}
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	if (MovementComp == nullptr) return;

	if (Role == ROLE_AutonomousProxy)
	{
		FGoKartMove Move = MovementComp->CreateMove(DeltaTime);

		MovementComp->SimulateMove(Move);

		UnacknowledgedMoves.Add(Move);

		Server_SendMove(Move);
	}

	// We are the server and in control of the pawn
	if (Role == ROLE_Authority && GetRemoteRole() == ROLE_SimulatedProxy)
	{
		FGoKartMove Move = MovementComp->CreateMove(DeltaTime);

		Server_SendMove(Move);
	}

	if (Role == ROLE_SimulatedProxy) MovementComp->SimulateMove(ServerState.LastMove);

	DrawDebugString(GetWorld(), FVector(0, 0, 100), GetEnumText(Role), this, FColor::White, DeltaTime);
}

// Called to bind functionality to input
void AGoKart::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	// Set up game play key bindings
	check(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AGoKart::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AGoKart::MoveRight);
}

void AGoKart::OnRep_ServerState()
{
	if (MovementComp == nullptr) return;

	SetActorTransform(ServerState.Transform);
	MovementComp->SetVelocity(ServerState.Velocity);

	ClearAcknowledgedMoves(ServerState.LastMove);

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		MovementComp->SimulateMove(Move);
	}
}

void AGoKart::ClearAcknowledgedMoves(FGoKartMove LastMove)
{
	TArray<FGoKartMove> NewMoves;

	for (const FGoKartMove& Move : UnacknowledgedMoves)
	{
		if (Move.Time > LastMove.Time) NewMoves.Add(Move);
	}

	UnacknowledgedMoves = NewMoves;
}

void AGoKart::MoveForward(float Value)
{
	if (MovementComp == nullptr) return;

	MovementComp->SetThrottle(Value);
}

void AGoKart::MoveRight(float Value)
{
	if (MovementComp == nullptr) return;

	MovementComp->SetSteeringThrow(Value);
}

void AGoKart::Server_SendMove_Implementation(FGoKartMove Move)
{
	if (MovementComp == nullptr) return;

	MovementComp->SimulateMove(Move);

	ServerState.LastMove = Move;
	ServerState.Transform = GetActorTransform();
	ServerState.Velocity = MovementComp->GetVelocity();
}

bool AGoKart::Server_SendMove_Validate(FGoKartMove Move)
{
	// TODO: Make better validation
	return true;
}

