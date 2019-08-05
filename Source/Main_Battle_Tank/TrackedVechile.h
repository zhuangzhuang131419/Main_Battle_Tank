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

UENUM(BlueprintType)
enum class ESide : uint8
{
	Left,
	Right
};

USTRUCT(BlueprintType)
struct FSuspensionSetUp
{
	GENERATED_USTRUCT_BODY()
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector RootLoc;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FRotator RootRot;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float MaxLength;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float CollisionRadius;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float StiffnessForce; // 刚度 材料或结构在受力时抵抗弹性形变的能力
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float DampingForce; // 阻尼力

	//FSuspensionSetUp() {}

	//FSuspensionSetUp(FVector RootLoc, FRotator RootRot, float MaxLength = 23, float CollisionRadius = 34, float StiffnessForce = 4000000, float DampingForce = 4000)
	//{
	//	this->RootLoc = RootLoc;
	//	this->RootRot = RootRot;
	//	this->MaxLength = MaxLength;
	//	this->CollisionRadius = CollisionRadius;
	//	this->StiffnessForce = StiffnessForce;
	//	this->DampingForce = DampingForce;
	//}
};

USTRUCT(BlueprintType)
struct FSuspensionInternalProcessingC
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector RootLoc;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FRotator RootRot;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Length;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Radius;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Stiffness;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float Damping;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	float PreviousLength;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector SuspensionForce;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector WheelCollisionLocation;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	FVector WheelCollisionNormal;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	bool Engaged = false;
	EPhysicalSurface HitMaterial;

	FSuspensionInternalProcessingC() {}

	FSuspensionInternalProcessingC(FVector RootLoc, FRotator RootRot, float Length = 100, float Radius = 100, float Stiffness = 0.5, float Damping = 0.5)
	{
		this->RootLoc = RootLoc;
		this->RootRot = RootRot;
		this->Length = Length;
		this->Radius = Radius;
		this->Stiffness = Stiffness;
		this->Damping = Damping;
	}
};

UCLASS()
class MAIN_BATTLE_TANK_API ATrackedVechile : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	ATrackedVechile();

	UFUNCTION(BlueprintCallable, Category = "SetUp")
	void Initialise(UStaticMeshComponent* BodyToSet,
			UArrowComponent* COMToSet,
			USkeletalMeshComponent* TreadRToSet,
			USkeletalMeshComponent* TreadLToSet,
			UStaticMeshComponent* WheelSweepToSet,
			UStaticMeshComponent* TurrentToSet,
			USkeletalMeshComponent* CannonToSet);

	UFUNCTION(BlueprintCallable)
	void PreCalculateMomentOfInteria();



protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UFUNCTION(BlueprintCallable)
	void UpdateThrottle();
	UFUNCTION(BlueprintCallable)
	void UpdateWheelsVelocity();
	UFUNCTION(BlueprintCallable)
	void UpdateAxlsVelocity();

	
	UFUNCTION(BlueprintCallable)
	bool PutToSleep();
	// local
	UFUNCTION(BlueprintCallable)
	FVector GetVelocityAtPointWorld(FVector PointLoc);


	float ApplyBrake(float AngularVelocity, float BrakeRatio);

	UFUNCTION(BlueprintCallable)
	void AnimateSprocketOrIdler(UStaticMeshComponent* SprocketOrIdlerComponnet, float TrackAngularVelocity, bool FlipAnimation180Degrees);
	
	UFUNCTION(BlueprintCallable)
	void BuildTrackSplineCPlusPlus(USplineComponent * SplineComponent, UInstancedStaticMeshComponent * TreadsMeshComponent, TArray<FVector> SplineCoordinates, TArray<FVector> SplineTangents, float TreadsOnSide);

	UFUNCTION(BlueprintCallable)
	void AnimateTreadsInstancedMeshCPlusPlus(USplineComponent* SplineComponent, UInstancedStaticMeshComponent* TreadsMeshComponent);

	UFUNCTION(BlueprintCallable)
	void AddGravity();

	UFUNCTION(BlueprintCallable)
	void PositionAndAnimateDriveWheels(UStaticMeshComponent* WheelComponent, FSuspensionInternalProcessingC SuspensionSet, int32 SuspensionIndex, ESide side, bool FlipAnimation180Degrees);

	float GetEngineTorque(float RevolutionPerMinute);
	float GetGearBoxTorque(float EngineTorque);
	float GetEngineRPMFromAxls(float AxlsAngularVelocity);
	UFUNCTION(BlueprintCallable)
	void CalculateEngineAndUpdateDrive();
	UFUNCTION(BlueprintCallable)
	void CountFrictionContactPoint(TArray<FSuspensionInternalProcessingC> SuspSide);
	void GetMuFromFrictionEllipse(FVector VelocityDirectionNormalized, FVector ForwardVector, float Mu_X_Static, float Mu_Y_Static, float Mu_X_Kinetic, float Mu_Y_Kinetic, OUT float& Mu_Static, OUT float& Mu_Kinetic);

	UPROPERTY(BlueprintReadWrite)
	int32 TreadsLastIndexCPlusPlus = 63;
	UPROPERTY(BlueprintReadWrite)
	int32 NeutralGearIndex;
	UPROPERTY(BlueprintReadWrite)
	int32 CurrentGear;

	UPROPERTY(BlueprintReadWrite)
	float EngineRPM;
	UPROPERTY(BlueprintReadWrite)
	bool ReverseGear;
	UPROPERTY(BlueprintReadWrite)
	bool SleepMode = false;
	UPROPERTY(BlueprintReadWrite)
	float SleepDelayTimer;

	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	int32 TotalNumFrictionPoints;

	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float MomentInertia;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float Throttle;

	// Wheel
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float WheelRightCoefficient;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float WheelForwardCoefficient;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float WheelLeftCoefficient;

	// Torque
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float EngineTorque;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float TrackTorqueTransferRight;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float TrackTorqueTransferLeft;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float TrackRightTorque;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float TrackLeftTorque;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float DriveRightTorque;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float DriveLeftTorque;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float DriveAxlsTorque;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float TrackFrictionTorque;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float TrackFrictionTorqueRight;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float TrackFrictionTorqueLeft;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float TrackRollingFrictionTorque;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float TrackRollingFrictionTorqueRight;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float TrackRollingFrictionTorqueLeft;

	// Velocity
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float TrackRightAngularVelocity;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float TrackLeftAngularVelocity;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float AxisAngularVelocity;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float TrackRightLinearVelocity;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float TrackLeftLinearVelocity;

	// Force
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	FVector DriveRightForce;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	FVector DriveLeftForce;

	//
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	float BrakeRatio;

	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	TArray<FSuspensionInternalProcessingC> SuspensionsInternalRight;
	UPROPERTY(BlueprintReadWrite, Category = "Physics")
	TArray<FSuspensionInternalProcessingC> SuspensionsInternalLeft;

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

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	UCurveFloat* EngineTorqueCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	TArray<float> GearRatios = { 4.35, 0, 3.81, 1.93, 1 };

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	TArray<FSuspensionSetUp> SuspensionSetUpRight;
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Performance")
	TArray<FSuspensionSetUp> SuspensionSetUpLeft;

	

private:
	UStaticMeshComponent* Body;
	UArrowComponent* COM;
	USkeletalMeshComponent* TreadR;
	USkeletalMeshComponent* TreadL;
	UStaticMeshComponent* WheelSweep;
	UStaticMeshComponent* Turrent;
	USkeletalMeshComponent* Cannon;

	


};
