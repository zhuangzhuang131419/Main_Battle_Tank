// Fill out your copyright notice in the Description page of Project Settings.


#include "TankAimingComponent.h"
#include "Kismet/KismetMathLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "TrackedVechile.h"

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

void UTankAimingComponent::MoveCannonTowards(FVector AimDirection)
{
	if (ensure(Cannon) && ensure(Turret))
	{

		auto DeltaRotator = AimDirection.Rotation() - Cannon->GetForwardVector().Rotation();

		// UE_LOG(LogTemp, Warning, TEXT("AimDirectionAsRotator: %s"), *AimDirection.Rotation().ToString());

		// Turret->SetRelativeRotation(FRotator(0, 180 + AimDirection.Rotation().Yaw, 0));
		// UE_LOG(LogTemp, Warning, TEXT("AimDirectionAsRotator: Yaw: %f, Cannon: Yaw: %f"), AimDirection.Rotation().Yaw, Cannon->GetRelativeTransform().GetRotation().Z);
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

void UTankAimingComponent::Initialise(USkeletalMeshComponent * CannonToSet, UStaticMeshComponent * TurretToSet, TSubclassOf<AProjectile> ProjectileToSet, UAnimationAsset * ConnonAnimationToSet)
{
	Cannon = CannonToSet;
	Turret = TurretToSet;
	ProjectileBlueprint = ProjectileToSet;
	CannonRecoil = ConnonAnimationToSet;
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
		
		AimDirection = OutLaunchVelocity.GetSafeNormal();
		MoveCannonTowards(AimDirection);
	}
}

void UTankAimingComponent::Fire()
{
	// Spawn a projectile at the socket location
	if (!ensure(Cannon)) { return; }
	if (!ensure(ProjectileBlueprint)) { return; }
	auto Projectile = GetWorld()->SpawnActor<AProjectile>(
		ProjectileBlueprint,
		Cannon->GetSocketLocation(FName("Muzzle")),
		Cannon->GetSocketRotation(FName("Muzzle"))
		);

	Projectile->LaunchProjectile(LaunchSpeed);
	// LastFireTime = FPlatformTime::Seconds();
	// RoundsLeft--;
	
	// 施加后坐力
	if (ensure(CannonRecoil))
	{
		Cannon->PlayAnimation(CannonRecoil, false);
	}
	
	/*ATrackedVechile* targetVehicle = Cast<ATrackedVechile>(GetOwner());
	if (ensure(targetVehicle))
	{
		UE_LOG(LogTemp, Warning, TEXT("Recoil"))
		UStaticMeshComponent* targetBody = targetVehicle->GetBody();
		FVector worldCannonLocation = UKismetMathLibrary::TransformLocation(targetVehicle->GetActorTransform(), Cannon->GetRelativeTransform().GetLocation());
		FVector ConnonDirection = (worldCannonLocation - Cannon->GetSocketLocation(FName("Muzzle"))).GetSafeNormal();
		targetBody->AddImpulseAtLocation(ConnonDirection * RecoilImpulse, targetBody->GetCenterOfMass());
	}*/
	
	
}

void UTankAimingComponent::ElevateCannon(float RelativeSpeed)
{
	RelativeSpeed = FMath::Clamp<float>(RelativeSpeed, -1, +1);
	float maxDegreePerSecond = 5;
	auto DegreeChanges = RelativeSpeed * maxDegreePerSecond * GetWorld()->DeltaTimeSeconds;
	Cannon->SetRelativeRotation(
		FRotator(DegreeChanges + Cannon->GetRelativeTransform().Rotator().Pitch, 0, 0));
}

void UTankAimingComponent::RotateTurret(float RelativeSpeed)
{
	float maxDegreePerSecond = 50;
	RelativeSpeed = FMath::Clamp<float>(RelativeSpeed, -1, +1);
	auto RotationChanges = RelativeSpeed * maxDegreePerSecond * GetWorld()->DeltaTimeSeconds;
	auto NewRotation = Turret->GetRelativeTransform().Rotator().Yaw + RotationChanges;
	Turret->SetRelativeRotation(FRotator(0, NewRotation, 0));
}

