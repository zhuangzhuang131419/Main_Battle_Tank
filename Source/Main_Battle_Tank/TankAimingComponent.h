// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "TankAimingComponent.generated.h"


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MAIN_BATTLE_TANK_API UTankAimingComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	// Sets default values for this component's properties
	UTankAimingComponent();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable, Category = "SetUp")
	void Initialise(USkeletalMeshComponent* CannonToSet, UStaticMeshComponent* TurretToSet);

	void AimAt(FVector HitLocation);

private:
	USkeletalMeshComponent* Cannon;
	UStaticMeshComponent* Turret;

	UPROPERTY(EditDefaultsOnly, Category = "Firing")
	float LaunchSpeed = 8000;

	void MoveCannonTowards(FVector AimDirection);

	
	void ElevateCannon(float RelativeSpeed);
	void RotateTurret(float RelativeSpeed);

	FVector AimDirection;
		
};
