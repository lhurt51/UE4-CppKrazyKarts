// Fill out your copyright notice in the Description page of Project Settings.

#include "GoKart.h"

#include "Components/InputComponent.h"
#include "Engine/World.h"


// Sets default values
AGoKart::AGoKart()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void AGoKart::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void AGoKart::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	FVector Force = GetActorForwardVector() * MaxDrivingForce * Throttle;
	// Applying the air drag to the Force
	Force += GetAirResistance();
	// Applying the rolling resistance to the Force
	Force += GetRollingResistance();
	// a = F / m
	FVector Acceleration = Force / Mass;
	// dv = a * dt
	Velocity += Acceleration * DeltaTime;

	ApplyRotation(DeltaTime);

	UpdateLocationFromVelocity(DeltaTime);
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

void AGoKart::MoveForward(float Value)
{
	// Direction of the car * speed of car in MPS * the value of the throttle
	Throttle = Value;
}

void AGoKart::MoveRight(float Value)
{
	// Right vector of the car * speed of steering in Deg/s
	SteeringThrow = Value;
}

FVector AGoKart::GetAirResistance()
{
	return -Velocity.GetSafeNormal() * Velocity.SizeSquared() * DragCoef;
}

FVector AGoKart::GetRollingResistance()
{
	float AccelerationDueToGravity = -GetWorld()->GetGravityZ() / 100.0f;
	float NormalForce = Mass * AccelerationDueToGravity;

	return -Velocity.GetSafeNormal() * RollingResistanceCoef * NormalForce;
}

void AGoKart::ApplyRotation(float DeltaTime)
{
	// (MaxDeg * DeltaTime = Angle we can mover per frame) * steeringthrow 
	float RotationAngle = MaxDegPerSec * DeltaTime * SteeringThrow;
	FQuat RotationDelta(GetActorUpVector(), FMath::DegreesToRadians(RotationAngle));

	// Rotating our velocity as we rotate the car
	Velocity = RotationDelta.RotateVector(Velocity);

	AddActorWorldRotation(RotationDelta);
}

void AGoKart::UpdateLocationFromVelocity(float DeltaTime)
{
	FHitResult Hit;
	// MPS * 100 = CMPS * S = Velocity in CM.
	FVector Translation = Velocity * 100 * DeltaTime;

	AddActorWorldOffset(Translation, true, &Hit);
	if (Hit.IsValidBlockingHit()) Velocity = FVector::ZeroVector;
}

