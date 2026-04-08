// Fill out your copyright notice in the Description page of Project Settings.


#include "Week3Drone.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "Week3DroneController.h"

// Sets default values
AWeek3Drone::AWeek3Drone()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	SphereComp = CreateDefaultSubobject<USphereComponent>(TEXT("SphereComp"));
	SetRootComponent(SphereComp);
	SphereComp->SetSimulatePhysics(false); //off

	SkeletalMeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("SkeletalMeshComp"));
	SkeletalMeshComp->SetupAttachment(RootComponent);
	SkeletalMeshComp->SetSimulatePhysics(false); //off

	StaticMeshComp = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("StaticMeshComp"));
	StaticMeshComp->SetupAttachment(RootComponent);
	StaticMeshComp->SetSimulatePhysics(false);//off

	SpringArmComp = CreateDefaultSubobject<USpringArmComponent>(TEXT("SpringArmComp"));
	SpringArmComp->SetupAttachment(RootComponent);
	SpringArmComp->TargetArmLength = 400.0f;
	SpringArmComp->bUsePawnControlRotation = false;

	CameraComp = CreateDefaultSubobject<UCameraComponent>(TEXT("CameraComp"));
	CameraComp->SetupAttachment(SpringArmComp, USpringArmComponent::SocketName); 
	CameraComp->bUsePawnControlRotation = false;

	MoveInput = FVector(0, 0, 0);
	LookInput = FVector(0, 0, 0);

	MoveSpeed = 500.0f;
	RotationSpeed = 100.0f;
}

// Called when the game starts or when spawned
void AWeek3Drone::BeginPlay()
{
	Super::BeginPlay();
}


// Called every frame
void AWeek3Drone::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	Accumulator += DeltaTime;

	while (Accumulator >= TimeStep) 
	{
		CustomTick(TimeStep);

		Accumulator -= TimeStep;
	}

}


void AWeek3Drone::CustomTick(float FixedDeltaTime)
{

}

// Called to bind functionality to input
void AWeek3Drone::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInput = Cast<UEnhancedInputComponent>(PlayerInputComponent)) 
	{
		if (AWeek3DroneController* PlayerController = Cast<AWeek3DroneController>(GetController()))
		{
			if (PlayerController->MoveAction) 
			{
				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Triggered,
					this,
					&AWeek3Drone::Move
				);

				EnhancedInput->BindAction(
					PlayerController->MoveAction,
					ETriggerEvent::Completed,
					this,
					&AWeek3Drone::Move
				);
			}

			if (PlayerController->LookAction) 
			{
				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Triggered,
					this,
					&AWeek3Drone::Look
				);

				EnhancedInput->BindAction(
					PlayerController->LookAction,
					ETriggerEvent::Completed,
					this,
					&AWeek3Drone::Look
				);
			}
		}
	}

}

void AWeek3Drone::Move(const FInputActionValue& Value)
{
	MoveInput = Value.Get<FVector>();
}

void AWeek3Drone::Look(const FInputActionValue& Value)
{
	LookInput = Value.Get<FVector>();
}
