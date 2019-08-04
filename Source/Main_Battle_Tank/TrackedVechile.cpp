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
	// ����֪ʶ
	// I��ת��������= m��������* r���ʵ��ת�ᴹֱ���룩
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
	/// ����֪ʶ
	// M(����/Torque) = I(ת������/MomentInertia) * a(�Ǽ��ٶ�/AngularAccelaration)

	// ��������
	// Total torque on right track
	TrackRightTorque = DriveRightTorque + TrackFrictionTorqueRight + TrackRollingFrictionTorqueRight;
	// Total torque on left track
	TrackLeftTorque = DriveLeftTorque + TrackFrictionTorqueLeft + TrackRollingFrictionTorqueLeft;

	// ������ٶ�
	TrackRightAngularVelocity = ApplyBrake(TrackRightTorque / MomentInertia * GetWorld()->DeltaTimeSeconds + TrackRightAngularVelocity, BrakeRatio);
	TrackLeftAngularVelocity = ApplyBrake(TrackLeftTorque / MomentInertia * GetWorld()->DeltaTimeSeconds + TrackLeftAngularVelocity, BrakeRatio);

	// �������ٶ�
	TrackRightLinearVelocity = TrackRightAngularVelocity * SproketRadius_cm;
	TrackLeftLinearVelocity = TrackLeftAngularVelocity * SproketRadius_cm;
}

void ATrackedVechile::UpdateAxlsVelocity()
{
	AxisAngularVelocity = (UKismetMathLibrary::Abs(TrackRightAngularVelocity) + UKismetMathLibrary::Abs(TrackLeftAngularVelocity)) / 2;
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
	/// ����֪ʶ
	// w(���ٶ�) = 2 * PI * n��ת�٣�
	return (AxlsAngularVelocity * GearRatios[CurrentGear] * DifferentialRatio * 60) / UKismetMathLibrary::GetPI() / 2;
}

void ATrackedVechile::CalculateEngineAndUpdateDrive()
{
	float maxEngineTorque = GetEngineTorque(GetEngineRPMFromAxls(AxisAngularVelocity));
	EngineTorque = maxEngineTorque * Throttle;
	DriveAxlsTorque = GetGearBoxTorque(EngineTorque);

	DriveRightTorque = TrackTorqueTransferRight * DriveAxlsTorque;
	DriveLeftTorque = TrackTorqueTransferLeft * DriveAxlsTorque;

	// M�����أ� = F * L
	DriveRightForce = GetActorForwardVector() * DriveRightTorque / SproketRadius_cm;
	DriveLeftForce = GetActorForwardVector() * DriveLeftTorque / SproketRadius_cm;
}



