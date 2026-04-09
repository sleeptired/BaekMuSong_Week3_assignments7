// Fill out your copyright notice in the Description page of Project Settings.


#include "Week3Drone.h"
#include "Components/SphereComponent.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "Week3DroneController.h"
#include "DrawDebugHelpers.h"

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
	bIsGrounded = false;

	SphereComp->SetCollisionProfileName(TEXT("Pawn"));//충돌처리 예방
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
	if (!LookInput.IsNearlyZero())
	{
		FRotator NewRotation = FRotator(
			LookInput.Y * RotationSpeed * FixedDeltaTime,
			LookInput.X * RotationSpeed * FixedDeltaTime,
			LookInput.Z * RotationSpeed * FixedDeltaTime
		);
		AddActorLocalRotation(NewRotation, true);
	}

	//Line Trace
	FVector StartLocation = GetActorLocation();//현재 드론의 중심점
	float SphereRadius = SphereComp->GetScaledSphereRadius(); //sphereComp의 반지름
	FVector EndLocation = StartLocation + FVector(0.0f, 0.0f, -(SphereRadius + 5.0f));//드론의 중심점 기준으로 sphereComp의 반지름의 10을 추가해서 레이저를 쏜다

	FHitResult HitResult;
	FCollisionQueryParams TraceParams;
	TraceParams.AddIgnoredActor(this);//자기 자신은 무시

	bIsGrounded = GetWorld()->LineTraceSingleByChannel(HitResult, StartLocation, EndLocation, ECC_Visibility, TraceParams);//LineTrace 쏘기 충돌은 world기준으로 모든 물체와 확인하는거라서 world좌표 사용

	//디버그 모드
	FColor LineColor = bIsGrounded ? FColor::Green : FColor::Red;
	DrawDebugLine(GetWorld(), StartLocation, EndLocation, LineColor, false, -1.0f, 0, 2.0f);

	if (bIsGrounded)
	{
		DrawDebugPoint(GetWorld(), HitResult.ImpactPoint, 10.0f, FColor::Yellow, false, -1.0f);
	}
	//


	//중력 상태 구현

	//착지 상태이고 위로 안움직일 때 (Space)
	//if (bIsGrounded && MoveInput.Z <= 0.0f)
	//{
	//	FallSpeed = 0.0f; // 착지 순간 낙하 속도 0
	//}
	//else
	//{ 
	//	//공중에서는 항상 중력이 누적
	//	FallSpeed += Gravity * FixedDeltaTime;
	//
	//	// 공기 저항 중력때문에 빠르게 위로 못올라가는 로직
	//	// (MoveSpeed보다 작게 제한해야, 스페이스바를 눌렀을 때 중력을 이기고 날 수 있음)
	//	FallSpeed = FMath::Clamp(FallSpeed, -200.0f, 0.0f);
	//}

	// 날고 있을 때 이동속도 제한




	if (!MoveInput.IsNearlyZero()) 
	{
		FVector DeltaLocation = MoveInput * MoveSpeed * FixedDeltaTime;
		AddActorLocalOffset(DeltaLocation, true);
	}
	LookInput.X = 0.0f;
	LookInput.Y = 0.0f;
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
