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

	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	UStaticMeshComponent* Body;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	UArrowComponent* COM;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	USkeletalMeshComponent* TreadR;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	USkeletalMeshComponent* TreadL;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	UStaticMeshComponent* WheelSweep;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	UStaticMeshComponent* Turrent;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	USpringArmComponent* MainCam;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	UCameraComponent* Camera;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	USkeletalMeshComponent* Cannon;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	USpringArmComponent* TurrentCam;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	USpringArmComponent* Front;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	USpringArmComponent* LookRight;
	UPROPERTY(EditAnyWhere, BlueprintReadWrite)
	USpringArmComponent* LookLeft;

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

};
