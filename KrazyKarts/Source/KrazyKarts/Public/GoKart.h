// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GoKart.generated.h"

UCLASS()
class KRAZYKARTS_API AGoKart : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AGoKart();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

private:
	// The Mass of the car (kg). 1000kg = 1ton
	UPROPERTY(EditAnywhere)
	float Mass = 1000.0f;

	// The force applied to the car when the throttle is fully down (N)
	UPROPERTY(EditAnywhere)
	float MaxDrivingForce = 10000.0f;

	// Minimum radius of the car turning circle at full lock (m)
	UPROPERTY(EditAnywhere)
	float MinTurningRadius = 10.0f;

	// Higher means more drag. 
	UPROPERTY(EditAnywhere)
	float DragCoef = 16.0f;

	// Higher means more Rolling Resistance.
	UPROPERTY(EditAnywhere)
	float RollingResistanceCoef = 0.015;

	FVector Velocity;

	float Throttle;

	float SteeringThrow;

	void MoveForward(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveForward(float Value);

	void MoveRight(float Value);

	UFUNCTION(Server, Reliable, WithValidation)
	void Server_MoveRight(float Value);

	FVector GetAirResistance();

	FVector GetRollingResistance();

	void ApplyRotation(float DeltaTime);

	void UpdateLocationFromVelocity(float DeltaTime);
	
};
