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

void ATrackedVechile::Initialise(UStaticMeshComponent * BodyToSet, UArrowComponent * COMToSet, UStaticMeshComponent * TurrentToSet, USkeletalMeshComponent * CannonToSet)
{
	Body = BodyToSet;
	COM = COMToSet;
	Turrent = TurrentToSet;
	Cannon = CannonToSet;
}

void ATrackedVechile::PreCalculateMomentOfInteria()
{
	// 物理知识
	// I（转动惯量）= m（质量）* r（质点和转轴垂直距离）
	MomentInertia = (SprocketMass_kg * 0.5 + TrackMass_kg) * SproketRadius_cm * SproketRadius_cm;
	UE_LOG(LogTemp, Warning, TEXT("The MomentInertia of sprocket is %f"), MomentInertia);
}

void ATrackedVechile::VisualizeCenterOfMass()
{
	COM->SetWorldLocation(Body->GetCenterOfMass());
}

void ATrackedVechile::ConstructSuspension()
{
	for (size_t i = 0; i < SuspensionHandleRight.Num(); i++)
	{
		SuspensionsInternalRight.Add(
			FSuspensionInternalProcessing(
				SuspensionHandleRight[i]->GetRelativeTransform().GetLocation(),
				SuspensionHandleRight[i]->GetRelativeTransform().GetRotation().Rotator(),
				SuspensionSetUpRight[i].MaxLength,
				SuspensionSetUpRight[i].CollisionRadius,
				SuspensionSetUpRight[i].StiffnessForce,
				SuspensionSetUpRight[i].DampingForce
			)
		);
	}

	for (size_t i = 0; i < SuspensionHandleLeft.Num(); i++)
	{
		SuspensionsInternalLeft.Add(
			FSuspensionInternalProcessing(
				SuspensionHandleLeft[i]->GetRelativeTransform().GetLocation(),
				SuspensionHandleLeft[i]->GetRelativeTransform().GetRotation().Rotator(),
				SuspensionSetUpLeft[i].MaxLength,
				SuspensionSetUpLeft[i].CollisionRadius,
				SuspensionSetUpLeft[i].StiffnessForce,
				SuspensionSetUpLeft[i].DampingForce
			)
		);
	}
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

void ATrackedVechile::CheckWheelCollision(int32 SuspIndex, UPARAM(ref)TArray<FSuspensionInternalProcessing>& SuspensionArray, ESide Side)
{
	auto SuspensionLength = (SuspensionArray)[SuspIndex].Length;
	auto SuspensionStiffness = SuspensionArray[SuspIndex].Stiffness;
	auto SuspensionDamping = SuspensionArray[SuspIndex].Damping;
	auto SuspensionPreviousLength = SuspensionArray[SuspIndex].PreviousLength;

	// 计算世界坐标
	auto SuspensionWorldX = UKismetMathLibrary::TransformDirection(GetActorTransform(), UKismetMathLibrary::GetForwardVector(SuspensionArray[SuspIndex].RootRot));
	auto SuspensionWorldY = UKismetMathLibrary::TransformDirection(GetActorTransform(), UKismetMathLibrary::GetRightVector(SuspensionArray[SuspIndex].RootRot));
	auto SuspensionWorldZ = UKismetMathLibrary::TransformDirection(GetActorTransform(), UKismetMathLibrary::GetUpVector(SuspensionArray[SuspIndex].RootRot));
	auto SuspensionWorldLocation = UKismetMathLibrary::TransformLocation(GetActorTransform(), SuspensionArray[SuspIndex].RootLoc);

	FVector Location;
	FVector ImpactPoint;
	FVector ImpactNormal;
	bool bBlockingHit;
	UPrimitiveComponent* Component = nullptr;

	TraceForSuspension(
		SuspensionWorldLocation,
		SuspensionWorldLocation - SuspensionWorldZ * SuspensionLength,
		SuspensionArray[SuspIndex].Radius,
		bBlockingHit,
		Location,
		ImpactPoint,
		ImpactNormal,
		Component
	);

	FVector SuspensionForce;
	if (bBlockingHit)
	{
		auto SuspensionNewLength = (SuspensionWorldLocation - Location).Size();

		SuspensionForce = AddSuspensionForce(
			SuspensionLength,
			SuspensionNewLength,
			SuspensionPreviousLength,
			SuspensionStiffness,
			SuspensionDamping,
			SuspensionWorldZ,
			SuspensionWorldLocation);

		SuspensionArray[SuspIndex].PreviousLength = SuspensionNewLength;
		SuspensionArray[SuspIndex].SuspensionForce = SuspensionForce;
		SuspensionArray[SuspIndex].WheelCollisionLocation = ImpactPoint;
		SuspensionArray[SuspIndex].WheelCollisionNormal = ImpactNormal;
		SuspensionArray[SuspIndex].Engaged = true;
		PushSuspesionToEnvironment(Component, SuspensionForce, Location);
	}
	else
	{
		SuspensionForce = FVector();
		SuspensionArray[SuspIndex].PreviousLength = SuspensionLength;
		SuspensionArray[SuspIndex].SuspensionForce = FVector();
		SuspensionArray[SuspIndex].WheelCollisionLocation = FVector();
		SuspensionArray[SuspIndex].WheelCollisionNormal = FVector();
		SuspensionArray[SuspIndex].Engaged = false;
	}
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
	// 刚体定轴转动
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

void ATrackedVechile::AnimateTreadsSplineControlPoints(UStaticMeshComponent * WheelMeshComponent, USplineComponent * TreadSplineComponent, TArray<FVector> SplineCoordinates, TArray<FSuspensionSetUp> SuspensionSetUp, int32 SuspensionIndex)
{
	if (ensure(WheelMeshComponent) && ensure(TreadSplineComponent))
	{
		auto BottomCPIndex = SuspensionIndex + 3;
		auto TopCPIndex = 16 - SuspensionIndex;
		TreadSplineComponent->SetLocationAtSplinePoint(
			BottomCPIndex,
			FVector(
				WheelMeshComponent->GetRelativeTransform().GetLocation().X,
				SplineCoordinates[BottomCPIndex].Y,
				WheelMeshComponent->GetRelativeTransform().GetLocation().Z - SuspensionSetUp[SuspensionIndex].CollisionRadius + TreadHalfThickness
			),
			ESplineCoordinateSpace::Local,
			true
		);

		TreadSplineComponent->SetLocationAtSplinePoint(
			TopCPIndex,
			FVector(
				WheelMeshComponent->GetRelativeTransform().GetLocation().X,
				SplineCoordinates[TopCPIndex].Y,
				UKismetMathLibrary::Max(
					WheelMeshComponent->GetRelativeTransform().GetLocation().Z + SuspensionSetUp[SuspensionIndex].CollisionRadius - TreadHalfThickness,
					SplineCoordinates[TopCPIndex].Z
				)
			),
			ESplineCoordinateSpace::Local,
			true
		);
	}
}

//void ATrackedVechile::AnimateTreadsMaterial()
//{
//	if (ensure(TreadMaterialRight) && ensure(TreadMaterialLeft))
//	{
//		TreadUVOffsetRight += GetWorld()->DeltaTimeSeconds * TrackRightLinearVelocity / TreadLength * TreadUVTiles;
//		TreadUVOffsetLeft += GetWorld()->DeltaTimeSeconds * TrackLeftLinearVelocity / TreadLength * TreadUVTiles;
//		TreadMaterialRight->SetScalarParameterValue(FName("UVOffset"), TreadUVOffsetRight);
//		TreadMaterialLeft->SetScalarParameterValue(FName("UVOffset"), TreadUVOffsetLeft);
//	}
//}

void ATrackedVechile::UpdateTreadRelatedToVelocity(USplineComponent * RightSpline, UInstancedStaticMeshComponent * TreadsRight, float LinearVelocity, float& TreadMeshOffset)
{
	if (ensure(RightSpline) && ensure(TreadsRight))
	{
		float SplineLength = RightSpline->GetSplineLength();
		TreadMeshOffset = FMath::Fmod(LinearVelocity * GetWorld()->DeltaTimeSeconds + TreadMeshOffset, SplineLength);
		for (size_t i = 0; i <= TreadsLastIndexCPlusPlus; i++)
		{
			float TreadOffset = (RightSpline->GetSplineLength() / TreadsOnSide) * i + TreadMeshOffset;
			float distance = FMath::Fmod(TreadOffset, SplineLength);
			if (distance < 0)
			{
				distance = FMath::Fmod(TreadOffset, SplineLength) + SplineLength;
			}
			FVector location = RightSpline->GetLocationAtDistanceAlongSpline(distance, ESplineCoordinateSpace::Local);
			FRotator rotation = RightSpline->GetRotationAtDistanceAlongSpline(distance, ESplineCoordinateSpace::Local);
			FVector right = RightSpline->GetRightVectorAtDistanceAlongSpline(distance, ESplineCoordinateSpace::Local);

			if (right.Y < 0)
			{
				rotation.Roll = 180;
			}

			TreadsRight->UpdateInstanceTransform(
				i,
				UKismetMathLibrary::MakeTransform(
					location,
					rotation,
					FVector(1, 1, 1)
				),
				false,
				i == TreadsLastIndexCPlusPlus,
				false
			);
		}
	}
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

void ATrackedVechile::AnimateTreadsInstancedMeshCPlusPlus(USplineComponent * SplineComponent, UInstancedStaticMeshComponent * TreadsMeshComponent, ESide side)
{
	switch (side)
	{
	case ESide::Left:
		UpdateTreadRelatedToVelocity(SplineComponent, TreadsMeshComponent, TrackLeftLinearVelocity, TreadMeshOffsetLeft);
		break;
	case ESide::Right:
		UpdateTreadRelatedToVelocity(SplineComponent, TreadsMeshComponent, TrackRightLinearVelocity, TreadMeshOffsetRight);
		break;
	default:
		break;
	}

}

void ATrackedVechile::PositionAndAnimateDriveWheels(UStaticMeshComponent * WheelComponent, FSuspensionInternalProcessing SuspensionSet, int32 SuspensionIndex, ESide side, bool FlipAnimation180Degrees)
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

void ATrackedVechile::TraceForSuspension(FVector Start, FVector End, float Radius, bool & BlockingHit, FVector & Location, FVector & ImpactPoint, FVector & ImpactNormal, UPrimitiveComponent *& Component)
{
	FHitResult HitResult;
	FCollisionQueryParams CollisionQueryParams;
	CollisionQueryParams.AddIgnoredActor(this);
	GetWorld()->SweepSingleByChannel(
		HitResult,
		Start,
		End,
		FQuat::Identity,
		ECollisionChannel::ECC_Visibility,
		FCollisionShape::MakeSphere(Radius),
		CollisionQueryParams

	);
	BlockingHit = HitResult.bBlockingHit;
	Location = HitResult.Location;
	ImpactPoint = HitResult.ImpactPoint;
	ImpactNormal = HitResult.ImpactNormal;
	Component = HitResult.GetComponent();
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
	// 变速器
	float GearBoxTorque = GearRatio * DifferentialRatio * TransmissionEfficiency * EngineTorque * EngineExtraPowerRatio;
	return ReverseGear ? -GearBoxTorque : GearBoxTorque;
}

float ATrackedVechile::GetEngineRPMFromAxls(float AxlsAngularVelocity)
{
	/// 物理知识
	// w(角速度) = 2 * PI * n（转速）
	return (AxlsAngularVelocity * GearRatio * DifferentialRatio * 60/*second to minute*/) / UKismetMathLibrary::GetPI() / 2;
}

void ATrackedVechile::CalculateEngineAndUpdateDrive()
{
	float maxEngineTorque = GetEngineTorque(GetEngineRPMFromAxls(AxisAngularVelocity));
	EngineTorque = maxEngineTorque * Throttle;

	// 引擎torque有损耗
	DriveAxlsTorque = GetGearBoxTorque(EngineTorque);

	DriveRightTorque = TrackTorqueTransferRight * DriveAxlsTorque;
	DriveLeftTorque = TrackTorqueTransferLeft * DriveAxlsTorque;

	// M（力矩） = F * L
	DriveRightForce = GetActorForwardVector() * DriveRightTorque / SproketRadius_cm;
	DriveLeftForce = GetActorForwardVector() * DriveLeftTorque / SproketRadius_cm;
}

void ATrackedVechile::CountFrictionContactPoint(TArray<FSuspensionInternalProcessing> SuspSide)
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

void ATrackedVechile::ApplyDriveForceAndGetFrictionForceOnSide(TArray<FSuspensionInternalProcessing> SuspensionsSide, FVector DriveForceSide, float TrackLinearVelocitySide, float & TotalFrictionTorqueSide, float & TotalRollingFrictionToqueSide)
{
	float TotalTrackFrictionTorque = 0;
	float TotalTrackRollingFrictionTorque = 0;
	// 便利所有的Suspension
	for (FSuspensionInternalProcessing SuspensionSide : SuspensionsSide)
	{
		// 有接触才有摩擦
		if (SuspensionSide.Engaged)
		{
			// 投影到法向量
			auto WheelLoadN = UKismetMathLibrary::ProjectVectorOnToVector(SuspensionSide.SuspensionForce, SuspensionSide.WheelCollisionNormal).Size();
			auto RelativeTrackVelocity = UKismetMathLibrary::ProjectVectorOnToPlane(
				GetVelocityAtPointWorld(SuspensionSide.WheelCollisionLocation) - UKismetMathLibrary::GetForwardVector(GetActorRotation()) * TrackLinearVelocitySide,
				SuspensionSide.WheelCollisionNormal
			);
			float MuStatic;
			float MuKinetic;



			GetMuFromFrictionEllipse(
				RelativeTrackVelocity.GetSafeNormal(),
				UKismetMathLibrary::GetForwardVector(GetActorRotation()),
				Mu_X_Static,
				Mu_Y_Static,
				Mu_X_Kinetic,
				Mu_Y_Kinetic,
				MuStatic,
				MuKinetic
			);

			// FT = MV
			FVector FrictionForce = -Body->GetMass() * RelativeTrackVelocity / GetWorld()->DeltaTimeSeconds / TotalNumFrictionPoints;

			// Full friction force from vehicle movement
			auto FullStaticFrictionForce = UKismetMathLibrary::ProjectVectorOnToVector(
				FrictionForce,
				UKismetMathLibrary::ProjectVectorOnToPlane(UKismetMathLibrary::GetForwardVector(GetActorRotation()), SuspensionSide.WheelCollisionNormal).GetSafeNormal()
			) * Mu_X_Static + UKismetMathLibrary::ProjectVectorOnToVector(
				FrictionForce,
				UKismetMathLibrary::ProjectVectorOnToPlane(UKismetMathLibrary::GetRightVector(GetActorRotation()), SuspensionSide.WheelCollisionNormal).GetSafeNormal()
			) * Mu_Y_Static;

			// Full friction force from vehicle movement
			auto FullKineticFrictionForce = UKismetMathLibrary::ProjectVectorOnToVector(
				FrictionForce,
				UKismetMathLibrary::ProjectVectorOnToPlane(UKismetMathLibrary::GetForwardVector(GetActorRotation()), SuspensionSide.WheelCollisionNormal).GetSafeNormal()
			) * Mu_X_Kinetic + UKismetMathLibrary::ProjectVectorOnToVector(
				FrictionForce,
				UKismetMathLibrary::ProjectVectorOnToPlane(UKismetMathLibrary::GetRightVector(GetActorRotation()), SuspensionSide.WheelCollisionNormal).GetSafeNormal()
			) * Mu_Y_Kinetic;

			auto FullStaticDriveForce = Mu_X_Static * UKismetMathLibrary::ProjectVectorOnToPlane(DriveForceSide, SuspensionSide.WheelCollisionNormal);
			auto FullKineticDriveForce = Mu_X_Kinetic * UKismetMathLibrary::ProjectVectorOnToPlane(DriveForceSide, SuspensionSide.WheelCollisionNormal);

			FVector FullFrictionForceNorm;
			FVector ApplicationForce;

			// We want to apply higher friction if forces are bellow static friction limit
			if ((FullStaticFrictionForce + FullStaticDriveForce).Size() >= WheelLoadN * MuStatic)
			{
				FullFrictionForceNorm = FullKineticFrictionForce.GetSafeNormal();
				ApplicationForce = UKismetMathLibrary::ClampVectorSize(FullKineticFrictionForce + FullKineticDriveForce, 0, WheelLoadN * MuKinetic);
			}
			else
			{
				FullFrictionForceNorm = FullStaticFrictionForce.GetSafeNormal();
				ApplicationForce = UKismetMathLibrary::ClampVectorSize(FullStaticFrictionForce + FullStaticDriveForce, 0, WheelLoadN * MuStatic);
			}
			Body->AddForceAtLocation(ApplicationForce, SuspensionSide.WheelCollisionLocation);
			FVector FrictionEffectTransmission = -UKismetMathLibrary::ProjectVectorOnToVector(ApplicationForce, FullFrictionForceNorm) / Body->GetMass() * (TrackMass_kg + SprocketMass_kg);
			TrackFrictionTorque = UKismetMathLibrary::ProjectVectorOnToVector(
				UKismetMathLibrary::InverseTransformDirection(GetActorTransform(), FrictionEffectTransmission),
				FVector(1, 0, 0)
			).X * SproketRadius_cm;

			// Make this force instead of torque
			TrackRollingFrictionTorque = -UKismetMathLibrary::SignOfFloat(TrackLinearVelocitySide) * WheelLoadN *
				(RollingFrictionCoeffient + UKismetMathLibrary::Abs(TrackLinearVelocitySide) * 0.000015);

			TotalTrackFrictionTorque += TrackFrictionTorque;
			TotalTrackRollingFrictionTorque += TrackRollingFrictionTorque;
		}
	}
	TotalFrictionTorqueSide = TotalTrackFrictionTorque;
	TotalRollingFrictionToqueSide = TotalTrackRollingFrictionTorque;

}

void ATrackedVechile::UpdateAutoGearBox()
{
	return;
	/*if (Throttle > 0 && AutoGearBox)
	{
		UE_LOG(LogTemp, Warning, TEXT("UpdateAutoGearBox"));
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
	}*/
}

void ATrackedVechile::GetMuFromFrictionEllipse(FVector VelocityDirectionNormalized, FVector ForwardVector, float Mu_X_Static, float Mu_Y_Static, float Mu_X_Kinetic, float Mu_Y_Kinetic, float & Mu_Static, float & Mu_Kinetic)
{
	// 计算合成的摩擦力，在椭圆内计算
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
	UpdateCoefficient();
}

void ATrackedVechile::UpdateCoefficient()
{
	BrakeRatio = 0;
	WheelForwardCoefficient = UKismetMathLibrary::Abs(AxisInputValue);
}

FVector ATrackedVechile::AddSuspensionForce(float SuspensionLength, float SuspensionNewLength, float SuspensionPreviousLength, float SuspensionStiffness, float SuspensionDamping, FVector SuspensionWorldZ, FVector SuspensionWorldLocation)
{
	float SpringCompressionRatio = (SuspensionLength - SuspensionNewLength) / SuspensionLength;
	float SuspensionVelocity = (SuspensionNewLength - SuspensionPreviousLength) / GetWorld()->DeltaTimeSeconds;
	// 物理知识
	// F（阻尼力） = -c（阻尼系数）* v（振子运动速度）
	auto SuspensionForce = (FMath::Clamp<float>(SpringCompressionRatio, 0, 1) * SuspensionStiffness - SuspensionDamping * SuspensionVelocity) * SuspensionWorldZ;
	Body->AddForceAtLocation(SuspensionForce, SuspensionWorldLocation);
	return SuspensionForce;
}

void ATrackedVechile::PushSuspesionToEnvironment(UPrimitiveComponent* CollisionPrimitive, FVector SuspensionForce, FVector WheelCollisionLocation)
{
	if (ensure(CollisionPrimitive) && CollisionPrimitive->IsSimulatingPhysics())
	{
		CollisionPrimitive->AddForceAtLocation(-SuspensionForce, WheelCollisionLocation);
	}
}


