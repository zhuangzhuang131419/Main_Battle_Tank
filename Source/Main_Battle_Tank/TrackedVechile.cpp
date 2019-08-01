// Fill out your copyright notice in the Description page of Project Settings.


#include "TrackedVechile.h"

// Sets default values
ATrackedVechile::ATrackedVechile()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Body = CreateDefaultSubobject<UStaticMeshComponent>(FName("Body"));
	COM = CreateDefaultSubobject<UArrowComponent>(FName("COM"));
	TreadR = CreateDefaultSubobject<USkeletalMeshComponent>(FName("TreadR"));
	TreadL = CreateDefaultSubobject<USkeletalMeshComponent>(FName("TreadL"));
	WheelSweep = CreateDefaultSubobject<UStaticMeshComponent>(FName("WheelSweep"));
	Turrent = CreateDefaultSubobject<UStaticMeshComponent>(FName("Turrent"));
	MainCam = CreateDefaultSubobject<USpringArmComponent>(FName("MainCam"));
	Camera = CreateDefaultSubobject<UCameraComponent>(FName("Camera"));
	Cannon = CreateDefaultSubobject<USkeletalMeshComponent>(FName("Cannon"));
	TurrentCam = CreateDefaultSubobject<USpringArmComponent>(FName("TurrentCam"));
	Front = CreateDefaultSubobject<USpringArmComponent>(FName("Front"));
	LookRight = CreateDefaultSubobject<USpringArmComponent>(FName("LookRight"));
	LookLeft = CreateDefaultSubobject<USpringArmComponent>(FName("LookLeft"));

	COM->AttachToComponent(Body, FAttachmentTransformRules::KeepWorldTransform);
	TreadR->AttachToComponent(Body, FAttachmentTransformRules::KeepWorldTransform);
	TreadL->AttachToComponent(Body, FAttachmentTransformRules::KeepWorldTransform);
	WheelSweep->AttachToComponent(Body, FAttachmentTransformRules::KeepWorldTransform);
	Turrent->AttachToComponent(Body, FAttachmentTransformRules::KeepWorldTransform);
	MainCam->AttachToComponent(Turrent, FAttachmentTransformRules::KeepWorldTransform);
	Camera->AttachToComponent(MainCam, FAttachmentTransformRules::KeepWorldTransform);
	Cannon->AttachToComponent(Turrent, FAttachmentTransformRules::KeepWorldTransform);
	TurrentCam->AttachToComponent(Cannon, FAttachmentTransformRules::KeepWorldTransform);
	Front->AttachToComponent(Body, FAttachmentTransformRules::KeepWorldTransform);
	LookRight->AttachToComponent(Body, FAttachmentTransformRules::KeepWorldTransform);
	LookLeft->AttachToComponent(Body, FAttachmentTransformRules::KeepWorldTransform);
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

