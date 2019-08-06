// Fill out your copyright notice in the Description page of Project Settings.


#include "TrackedVechile.h"
#include "Kismet/KismetMathLibrary.h"
#include "Curves/CurveFloat.h"

// Sets default values
ATrackedVechile::ATrackedVechile()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

void ATrackedVechile::PreCalculateMomentOfInteria()
{
	// 物理知识
	// I（转动惯量）= m（质量）* r（质点和转轴垂直距离）
	MomentInertia = (SproketMass_kg * 0.5 + TrackMass_kg) * SproketRadius_cm * SproketRadius_cm;
	UE_LOG(LogTemp, Warning, TEXT("The MomentInertia of sprocket is %f"), MomentInertia);
}

// Called when the game starts or when spawned
void ATrackedVechile::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ATrackedVechile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ATrackedVechile::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void ATrackedVechile::UpdateThrottle()
{
	TrackTorqueTransferRight = FMath::Clamp<float>(WheelRightCoefficient + WheelForwardCoefficient, -1, 2);
	TrackTorqueTransferLeft = FMath::Clamp<float>(WheelLeftCoefficient + WheelForwardCoefficient, -1, 2);

	float ThrottleIncrement;
	if (UKismetMathLibrary::Max(UKismetMathLibrary::Abs(TrackTorqueTransferRight), UKismetMathLibrary::Abs(TrackTorqueTransferLeft)) != 0)
	{
		ThrottleIncrement = 0.5;
	}
	else
	{
		ThrottleIncrement = -1;
	}

	Throttle += GetWorld()->DeltaTimeSeconds * ThrottleIncrement;
	Throttle = FMath::Clamp<float>(Throttle, 0, 1);
}

void ATrackedVechile::UpdateWheelsVelocity()
{
	/// 物理知识
	// M(力矩/Torque) = I(转动惯量/MomentInertia) * a(角加速度/AngularAccelaration)

	// 计算力矩
	// Total torque on right track
	TrackRightTorque = DriveRightTorque + TrackFrictionTorqueRight + TrackRollingFrictionTorqueRight;
	// Total torque on left track
	TrackLeftTorque = DriveLeftTorque + TrackFrictionTorqueLeft + TrackRollingFrictionTorqueLeft;

	// 计算角速度
	TrackRightAngularVelocity = ApplyBrake(TrackRightTorque / MomentInertia * GetWorld()->DeltaTimeSeconds + TrackRightAngularVelocity, BrakeRatio);
	TrackLeftAngularVelocity = ApplyBrake(TrackLeftTorque / MomentInertia * GetWorld()->DeltaTimeSeconds + TrackLeftAngularVelocity, BrakeRatio);

	// 计算线速度
	TrackRightLinearVelocity = TrackRightAngularVelocity * SproketRadius_cm;
	TrackLeftLinearVelocity = TrackLeftAngularVelocity * SproketRadius_cm;
}

void ATrackedVechile::UpdateAxlsVelocity()
{
	AxisAngularVelocity = (UKismetMathLibrary::Abs(TrackRightAngularVelocity) + UKismetMathLibrary::Abs(TrackLeftAngularVelocity)) / 2;
}

bool ATrackedVechile::PutToSleep()
{
	if (SleepMode)
	{
		if (SleepDelayTimer < SleepTimerSeconds)
		{
			SleepDelayTimer += GetWorld()->DeltaTimeSeconds;
			return SleepMode;
		}

	}

	if (Body->GetPhysicsLinearVelocity().Size() < SleepVelocity
		&& SleepVelocity < Body->GetPhysicsAngularVelocityInDegrees().Size())
	{
		if (!SleepMode)
		{
			SleepMode = true;
			TArray<UActorComponent*> Components = GetComponentsByClass(UStaticMeshComponent::StaticClass());
			for (size_t i = 0; i < Components.Num(); i++)
			{
				Cast<UPrimitiveComponent>(Components[i])->PutRigidBodyToSleep();
				SleepDelayTimer = 0;
			}
		}
	}
	else
	{
		if (SleepMode)
		{
			SleepMode = false;
			SleepDelayTimer = 0;
		}
	}
	return SleepMode;
}

FVector ATrackedVechile::GetVelocityAtPointWorld(FVector PointLoc)
{
	FVector localLinearVelocity = UKismetMathLibrary::InverseTransformDirection(GetActorTransform(), Body->GetPhysicsLinearVelocity());
	FVector localVelocityInRadius = UKismetMathLibrary::InverseTransformDirection(GetActorTransform(), Body->GetPhysicsAngularVelocityInRadians());

	FVector localCenterOfMass = UKismetMathLibrary::InverseTransformLocation(GetActorTransform(), Body->GetCenterOfMass());
	FVector localAngualrVelocity = UKismetMathLibrary::Cross_VectorVector(localVelocityInRadius, UKismetMathLibrary::InverseTransformLocation(GetActorTransform(), PointLoc) - localCenterOfMass/*距离重心的位置*/);
	FVector localVelocity = localLinearVelocity + localAngualrVelocity;
	return UKismetMathLibrary::TransformDirection(GetActorTransform(), localVelocity);
}

float ATrackedVechile::ApplyBrake(float AngularVelocity, float BrakeRatio)
{
	float BrakeImpulse = GetWorld()->DeltaTimeSeconds * BrakeForce * BrakeRatio;

	if (UKismetMathLibrary::Abs(AngularVelocity) > UKismetMathLibrary::Abs(BrakeImpulse))
	{
		return AngularVelocity - UKismetMathLibrary::SignOfFloat(AngularVelocity) * BrakeImpulse;
	}
	else
	{
		return 0.0;
	}
}

void ATrackedVechile::AnimateSprocketOrIdler(UStaticMeshComponent * SprocketOrIdlerComponnet, float TrackAngularVelocity, bool FlipAnimation180Degrees)
{
	// 转动主动轮和惰轮
	FRotator localRotator;
	if (FlipAnimation180Degrees)
	{
		localRotator = FRotator(UKismetMathLibrary::RadiansToDegrees(TrackAngularVelocity) * GetWorld()->DeltaTimeSeconds, 0, 0);
	}
	else
	{
		localRotator = FRotator(-UKismetMathLibrary::RadiansToDegrees(TrackAngularVelocity) * GetWorld()->DeltaTimeSeconds, 0, 0);
	}
	SprocketOrIdlerComponnet->AddLocalRotation(localRotator);
}

void ATrackedVechile::Initialise(UStaticMeshComponent * BodyToSet, UArrowComponent * COMToSet, USkeletalMeshComponent * TreadRToSet, USkeletalMeshComponent * TreadLToSet, UStaticMeshComponent * WheelSweepToSet, UStaticMeshComponent * TurrentToSet, USkeletalMeshComponent * CannonToSet)
{
	Body = BodyToSet;
	COM = COMToSet;
	TreadR = TreadRToSet;
	TreadL = TreadLToSet;
	WheelSweep = WheelSweepToSet;
	Turrent = TurrentToSet;
	Cannon = CannonToSet;
}

void ATrackedVechile::BuildTrackSplineCPlusPlus(USplineComponent * SplineComponent, UInstancedStaticMeshComponent * TreadsMeshComponent, TArray<FVector> SplineCoordinates, TArray<FVector> SplineTangents, float TreadsOnSide)
{
	auto TreadsLastIndex = (int32)TreadsOnSide - 1;
	SplineComponent->SetSplinePoints(SplineCoordinates, ESplineCoordinateSpace::Local);

	for (size_t i = 0; i < SplineTangents.Num(); i++)
	{
		SplineComponent->SetTangentAtSplinePoint(i, SplineTangents[i], ESplineCoordinateSpace::Local);
	}

	for (size_t i = 0; i < TreadsLastIndex; i++)
	{
		float distance = (SplineComponent->GetSplineLength() / TreadsOnSide) * i;
		FVector location = SplineComponent->GetLocationAtDistanceAlongSpline(distance, ESplineCoordinateSpace::Local);
		FRotator rotation = SplineComponent->GetRotationAtDistanceAlongSpline(distance, ESplineCoordinateSpace::Local);
		FVector right = SplineComponent->GetRightVectorAtDistanceAlongSpline(distance, ESplineCoordinateSpace::Local);

		if (right.Y < 0)
		{
			rotation.Roll = 180;
		}

		TreadsMeshComponent->AddInstance(
			UKismetMathLibrary::MakeTransform(
				location,
				rotation,
				FVector(1, 1, 1)
			)
		);
	}
}

void ATrackedVechile::AnimateTreadsInstancedMeshCPlusPlus(USplineComponent * SplineComponent, UInstancedStaticMeshComponent * TreadsMeshComponent)
{

}

void ATrackedVechile::AddGravity()
{
	Body->AddForce(Body->GetMass() * FVector(0, 0, -980));
}

void ATrackedVechile::PositionAndAnimateDriveWheels(UStaticMeshComponent * WheelComponent, FSuspensionInternalProcessingC SuspensionSet, int32 SuspensionIndex, ESide side, bool FlipAnimation180Degrees)
{
	// 设置轮胎位置
	WheelComponent->SetWorldLocation(
		UKismetMathLibrary::TransformLocation(
			GetActorTransform(),
			UKismetMathLibrary::TransformLocation(
				UKismetMathLibrary::MakeTransform(SuspensionSet.RootLoc, SuspensionSet.RootRot, FVector(1, 1, 1)),
				FVector(0, 0, -SuspensionSet.PreviousLength)
			)
		)
	);

	FRotator localRotator;
	float trackAngularVelocity = 0;
	switch (side)
	{
	case ESide::Left:
		trackAngularVelocity = TrackLeftAngularVelocity;
		break;
	case ESide::Right:
		trackAngularVelocity = TrackRightAngularVelocity;
		break;
	default:
		break;
	}

	if (FlipAnimation180Degrees)
	{
		localRotator = FRotator(UKismetMathLibrary::RadiansToDegrees(trackAngularVelocity) * GetWorld()->DeltaTimeSeconds, 0, 0);
	}
	else
	{
		localRotator = FRotator(-UKismetMathLibrary::RadiansToDegrees(trackAngularVelocity) * GetWorld()->DeltaTimeSeconds, 0, 0);
	}

	WheelComponent->AddLocalRotation(localRotator);
}

float ATrackedVechile::GetEngineTorque(float RevolutionPerMinute)
{
	float MinTime;
	float MaxTime;
	EngineTorqueCurve->GetTimeRange(MinTime, MaxTime);
	EngineRPM = FMath::Clamp<float>(RevolutionPerMinute, MinTime, MaxTime);
	// MaxTorque
	return EngineTorqueCurve->GetFloatValue(EngineRPM) * 100; // cm to m
}

float ATrackedVechile::GetGearBoxTorque(float EngineTorque)
{
	float GearBoxTorque = GearRatios[CurrentGear] * DifferentialRatio * TransmissionEfficiency * EngineTorque * EngineExtraPowerRatio;
	return ReverseGear ? -GearBoxTorque : GearBoxTorque;
}

float ATrackedVechile::GetEngineRPMFromAxls(float AxlsAngularVelocity)
{
	/// 物理知识
	// w(角速度) = 2 * PI * n（转速）
	return (AxlsAngularVelocity * GearRatios[CurrentGear] * DifferentialRatio * 60) / UKismetMathLibrary::GetPI() / 2;
}

void ATrackedVechile::CalculateEngineAndUpdateDrive()
{
	float maxEngineTorque = GetEngineTorque(GetEngineRPMFromAxls(AxisAngularVelocity));
	EngineTorque = maxEngineTorque * Throttle;
	DriveAxlsTorque = GetGearBoxTorque(EngineTorque);

	DriveRightTorque = TrackTorqueTransferRight * DriveAxlsTorque;
	DriveLeftTorque = TrackTorqueTransferLeft * DriveAxlsTorque;

	// M（力矩） = F * L
	DriveRightForce = GetActorForwardVector() * DriveRightTorque / SproketRadius_cm;
	DriveLeftForce = GetActorForwardVector() * DriveLeftTorque / SproketRadius_cm;
}

void ATrackedVechile::CountFrictionContactPoint(TArray<FSuspensionInternalProcessingC> SuspSide)
{
	for (size_t i = 0; i < SuspSide.Num(); i++)
	{
		// 有接触才计入TotalNumFrctionPoints中
		if (SuspSide[i].Engaged)
		{
			TotalNumFrictionPoints++;
		}
	}
}

void ATrackedVechile::ShiftGear(int32 ShiftUpOrDown)
{
	int32 min;
	int32 max;

	if (ReverseGear)
	{
		min = 0;
		max = NeutralGearIndex + 1;
	}
	else
	{
		min = NeutralGearIndex - 1;
		max = GearRatios.Num();
	}


	if (AutoGearBox)
	{
		CurrentGear = FMath::Clamp<float>(NeutralGearIndex + 1, min, max);
	}
	else
	{
		CurrentGear = FMath::Clamp<float>(CurrentGear + ShiftUpOrDown, 0, GearRatios.Num());
		ReverseGear = CurrentGear >= NeutralGearIndex ? false : true;
	}
}

void ATrackedVechile::UpdateAutoGearBox()
{
	if (Throttle > 0 && AutoGearBox)
	{
		float MinTime;
		float MaxTime;
		EngineTorqueCurve->GetTimeRange(MinTime, MaxTime);
		float GearShift = (EngineRPM - MinTime) / (MaxTime - MinTime);
		if (AxisAngularVelocity > LastAutoGearBoxAxisCheck)
		{
			if (GearShift >= GearUpShiftPrc)
			{
				if (ReverseGear)
				{
					ShiftGear(-1);
				}
				else
				{
					ShiftGear(1);
				}
			}
		}

		if (AxisAngularVelocity < LastAutoGearBoxAxisCheck)
		{
			if (GearShift < GearDownShiftPrc)
			{
				if (ReverseGear)
				{
					ShiftGear(1);
				}
				else
				{
					ShiftGear(-1);
				}
			}
		}

		LastAutoGearBoxAxisCheck = AxisAngularVelocity;
	}
}

void ATrackedVechile::GetMuFromFrictionEllipse(FVector VelocityDirectionNormalized, FVector ForwardVector, float Mu_X_Static, float Mu_Y_Static, float Mu_X_Kinetic, float Mu_Y_Kinetic, OUT float & Mu_Static, OUT float & Mu_Kinetic)
{
	float forwardVelocity = UKismetMathLibrary::Dot_VectorVector(VelocityDirectionNormalized, ForwardVector);
	Mu_Static = FVector2D(Mu_X_Static * forwardVelocity,
		UKismetMathLibrary::Sqrt(1 - forwardVelocity * forwardVelocity) * Mu_Y_Static).Size();
	Mu_Kinetic = FVector2D(Mu_X_Kinetic * forwardVelocity,
		UKismetMathLibrary::Sqrt(1 - forwardVelocity * forwardVelocity) * Mu_Y_Kinetic).Size();
}

void ATrackedVechile::GetThrottleInputForAutoHandling(float InputVehicleLeftRight, float InputVehicleForwardBackward)
{
	AxisInputValue = InputVehicleForwardBackward;
	FVector localVelocity = UKismetMathLibrary::InverseTransformDirection(GetActorTransform(), GetVelocity());
	if (AxisInputValue != 0)
	{
		if (localVelocity.Size() > 10) // Are we moving?
		{
			if (AxisInputValue > 0) // Is forward pressed?
			{
				if (localVelocity.X > 0) // We are moving forward with forward pressed
				{
					// Forward
					Forward();
				}
				else
				{
					// Brake
					Brake();
				}
			}
			else
			{
				if (localVelocity.X > 0) // We are moving forward with backwards pressed
				{
					// Brake
					Brake();
				}
				else
				{
					// Backwards
					Backward();
				}
			}
		}
		else
		{
			if (AxisInputValue > 0) // Not moving yet but forward/backward is pressed
			{
				// Forward
				Forward();
			}
			else
			{
				Backward();
			}
		}
	}
	else
	{
		// Forward/backward is not pressed, no throttle but maybe we are steering
		if (InputVehicleLeftRight != 0 && localVelocity.Size() > 10)
		{
			// We are steering without throttle and not rolling
			Forward();
		}
		else
		{
			UpdateCoefficient();
		}
	}
}

void ATrackedVechile::Forward()
{
	ReverseGear = false;
	ShiftGear(0);
	UpdateCoefficient();
}

void ATrackedVechile::Brake()
{
	BrakeRatio = UKismetMathLibrary::Abs(AxisInputValue);
	WheelForwardCoefficient = 0;
}

void ATrackedVechile::Backward()
{
	ReverseGear = true;
	ShiftGear(0);
	UpdateCoefficient();
}

void ATrackedVechile::UpdateCoefficient()
{
	BrakeRatio = 0;
	WheelForwardCoefficient = UKismetMathLibrary::Abs(AxisInputValue);
}


