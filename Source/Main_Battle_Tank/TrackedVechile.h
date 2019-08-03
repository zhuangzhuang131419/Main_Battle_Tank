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

	UPROPERTY(BlueprintReadWrite)
	int32 TreadsLastIndexCPlusPlus = 63;

private:
	UStaticMeshComponent* Body;
	UArrowComponent* COM;
	USkeletalMeshComponent* TreadR;
	USkeletalMeshComponent* TreadL;
	UStaticMeshComponent* WheelSweep;
	UStaticMeshComponent* Turrent;
	USkeletalMeshComponent* Cannon;

	


};
