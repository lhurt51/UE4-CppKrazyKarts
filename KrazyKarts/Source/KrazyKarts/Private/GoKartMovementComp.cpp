// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKartMovementComp.h"


// Sets default values for this component's properties
UGoKartMovementComp::UGoKartMovementComp()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UGoKartMovementComp::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UGoKartMovementComp::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwnerRole() == ROLE_AutonomousProxy || GetOwner()->GetRemoteRole() == ROLE_SimulatedProxy)
	{
		LastMove = CreateMove(DeltaTime);
		SimulateMove(LastMove);
	}
}

void UGoKartMovementComp::SimulateMove(const FGoKartMove & Move)
{
	FVector Force = GetOwner()->GetActorForwardVector() * MaxDrivingForce * Move.Throttle;
	// Applying the air drag to the Force
	Force += GetAirResistance();
	// Applying the rolling resistance to the Force
	Force += GetRollingResistance();
	// a = F / m
	FVector Acceleration = Force / Mass;
	// dv = a * dt
	Velocity += Acceleration * Move.DeltaTime;

	ApplyRotation(Move.DeltaTime, Move.SteeringThrow);

	UpdateLocationFromVelocity(Move.DeltaTime);
}

FGoKartMove UGoKartMovementComp::CreateMove(float DeltaTime)
{
	FGoKartMove Move;
	Move.DeltaTime = DeltaTime;
	Move.SteeringThrow = SteeringThrow;
	Move.Throttle = Throttle;
	Move.Time = GetWorld()->TimeSeconds;

	return Move;
}

FVector UGoKartMovementComp::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoef;
}

FVector UGoKartMovementComp::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100.0f;
	float NormalForce = Mass * AccelerationDueToGravity;

	return -Velocity.GetSafeNormal() * RollingResistanceCoef * NormalForce;
}

void UGoKartMovementComp::ApplyRotation(float DeltaTime, float InSteeringThrow)
{
	float DeltaLocation = FVector::DotProduct(GetOwner()->GetActorForwardVector(), Velocity) * DeltaTime;
	float RotationAngle = DeltaLocation / MinTurningRadius * InSteeringThrow;
	FQuat RotationDelta(GetOwner()->GetActorUpVector(), RotationAngle);

	// Rotating our velocity as we rotate the car
	Velocity = RotationDelta.RotateVector(Velocity);

	GetOwner()->AddActorWorldRotation(RotationDelta);
}

void UGoKartMovementComp::UpdateLocationFromVelocity(float DeltaTime)
{
	FHitResult Hit;
	// MPS * 100 = CMPS * S = Velocity in CM.
	FVector Translation = Velocity * 100 * DeltaTime;

	GetOwner()->AddActorWorldOffset(Translation, true, &Hit);
	if (Hit.IsValidBlockingHit()) Velocity = FVector::ZeroVector;
}

