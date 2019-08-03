// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "Components/PrimitiveComponent.h"
#include "Components/SplineComponent.h"
#include "PhysicsEngine/PhysicsConstraintComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TrackedVechile.generated.h"

UCLASS()
class MAIN_BATTLE_TANK_API ATrackedVechile : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATrackedVechile();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable, Category = "SetUp")
	void Initialise(UStaticMeshComponent* BodyToSet, 
		UArrowComponent* COMToSet, 
		USkeletalMeshComponent* TreadRToSet,
		USkeletalMeshComponent* TreadLToSet,
		UStaticMeshComponent* WheelSweepToSet,
		UStaticMeshComponent* TurrentToSet,
		USkeletalMeshComponent* CannonToSet);

	UFUNCTION(BlueprintCallable)
	void BuildTrackSplineCPlusPlus(USplineComponent * SplineComponent, UInstancedStaticMeshComponent * TreadsMeshComponent, TArray<FVector> SplineCoordinates, TArray<FVector> SplineTangents, float TreadsOnSide);

	UFUNCTION(BlueprintCallable)
	void AnimateTreadsInstancedMeshCPlusPlus(USplineComponent* SplineComponent, UInstancedStaticMeshComponent* TreadsMeshComponent);

	UFUNCTION(BlueprintCallable)
	void AddGravity();

	UPROPERTY(BlueprintReadWrite)
	int32 TreadsLastIndexCPlusPlus = 63;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float TrackMass_kg = 600;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float SproketMass_kg = 65;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float AirDensity = 1.2922;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float DragSurfaceArea = 10;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float DragCoefficient = 0.8;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float SproketRadius_cm = 24;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float DifferentialRatio = 3.5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float TransmissionEfficiency = 0.9;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float Mu_X_Static = 1;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float Mu_Y_Static = 0.85;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float Mu_X_Kinetic = 0.5;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float Mu_Y_Kinetic = 0.45;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float RollingFrictionCoefficient = 0.02;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float BrakeForce = 30;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float TreadLength = 972.5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float TreadUVTiles = 32.5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float TreadsOnSide = 64;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float TreadHalfThickness = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float SleepVelocity = 5;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float SleepTimerSeconds = 2;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	bool AutoGearBox = true;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float GearUpShiftPrc = 0.9;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float GearDownShiftPrc = 0.05;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	float EngineExtraPowerRatio = 3;


private:
	UStaticMeshComponent* Body;
	UArrowComponent* COM;
	USkeletalMeshComponent* TreadR;
	USkeletalMeshComponent* TreadL;
	UStaticMeshComponent* WheelSweep;
	UStaticMeshComponent* Turrent;
	USkeletalMeshComponent* Cannon;

	


};
