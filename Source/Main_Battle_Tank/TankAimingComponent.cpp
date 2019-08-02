// Fill out your copyright notice in the Description page of Project Settings.


#include "TankAimingComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"

// Sets default values for this component's properties
UTankAimingComponent::UTankAimingComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;

	// ...
}


// Called when the game starts
void UTankAimingComponent::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}


// Called every frame
void UTankAimingComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	// ...
}

void UTankAimingComponent::Initialise(USkeletalMeshComponent * CannonToSet, UStaticMeshComponent * TurretToSet)
{
	Cannon = CannonToSet;
	Turret = TurretToSet;
}

void UTankAimingComponent::MoveCannonTowards(FVector AimDirection)
{
	if (!ensure(Cannon) || !ensure(Turret))
	{
		auto DeltaRotator = AimDirection.Rotation() - Cannon->GetForwardVector().Rotation();
		ElevateCannon(DeltaRotator.Pitch);
		if (FMath::Abs(DeltaRotator.Yaw) < 180)
		{
			RotateTurret(DeltaRotator.Yaw);
		}
		else
		{
			RotateTurret(-DeltaRotator.Yaw);
		}
	}
}

void UTankAimingComponent::AimAt(FVector HitLocation)
{
	if (!ensure(Cannon) || !ensure(Turret)) { return; }
	FVector OutLaunchVelocity;
	FVector StartLocation = Cannon->GetSocketLocation(FName("Muzzle"));
	bool bHaveAimSolution = UGameplayStatics::SuggestProjectileVelocity(
		this,
		OutLaunchVelocity,
		StartLocation,
		HitLocation,
		LaunchSpeed,
		0,
		0,
		ESuggestProjVelocityTraceOption::DoNotTrace);

	if (bHaveAimSolution)
	{
		UE_LOG(LogTemp, Warning, TEXT("AimDirection: %s"), *AimDirection.ToString());
		AimDirection = OutLaunchVelocity.GetSafeNormal();
		MoveCannonTowards(AimDirection);
	}
}

void UTankAimingComponent::ElevateCannon(float RelativeSpeed)
{
	float maxDegreePerSecond = 90;
	auto DegreeChanges = RelativeSpeed * maxDegreePerSecond * GetWorld()->DeltaTimeSeconds;
	RelativeSpeed = FMath::Clamp<float>(RelativeSpeed, -1, +1);
	Cannon->SetRelativeRotation(
		FRotator(DegreeChanges + Cannon->GetRelativeTransform().Rotator().Pitch, 0, 0));
}

void UTankAimingComponent::RotateTurret(float RelativeSpeed)
{
	float maxDegreePerSecond = 90;
	auto DegreeChanges = RelativeSpeed * maxDegreePerSecond * GetWorld()->DeltaTimeSeconds;
	RelativeSpeed = FMath::Clamp<float>(RelativeSpeed, -1, +1);
	Turret->SetRelativeRotation(
		FRotator(0, DegreeChanges + Cannon->GetRelativeTransform().Rotator().Yaw, 0));
}

