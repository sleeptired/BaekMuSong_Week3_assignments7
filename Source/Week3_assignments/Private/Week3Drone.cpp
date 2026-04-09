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

	MoveSpeed = 1000.0f;
	RotationSpeed = 100.0f;
	bIsGrounded = false;

	SphereComp->SetCollisionProfileName(TEXT("Pawn"));//충돌처리 예방

	Gravity = -980.0f;
	FallSpeed = 0.0f;

	ShiftSpeed = 1000.0f;
	UpSpeed = 2500.0f;

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

	float ZInput = MoveInput.Z;

	//착지 상태이고 위로 안움직일 때 (Space)
	if (bIsGrounded && ZInput <= 0.0f)
	{
		FallSpeed = 0.0f; // 착지 순간 낙하 속도 0
	}
	else
	{ 
		FallSpeed += Gravity * FixedDeltaTime;

		if (ZInput > 0.0f) // 스페이스바 (상승)
		{
			// 중력을 솟구칠 수 있는 힘 부여
			FallSpeed += UpSpeed * FixedDeltaTime;
		}
		else if (ZInput < 0.0f) // 쉬프트 (하강)
		{
			// 밑으로 꽂히는 힘
			FallSpeed -= ShiftSpeed * FixedDeltaTime;
		}
		else 
		{
			FallSpeed += (-Gravity) * FixedDeltaTime; // 중력 상쇄
			float TargetHoverSpeed = -20.0f; // 아주 천천히 떨어지는 목표 속도

			// FInterpTo로 목표 하강 속도에 맞춰 부드럽게 감속
			FallSpeed = FMath::FInterpTo(FallSpeed, TargetHoverSpeed, FixedDeltaTime, 3.0f);
		}
	}

	//중력적용 부분 
	FVector GravityMove = FVector(0.0f, 0.0f, FallSpeed * FixedDeltaTime);
	AddActorWorldOffset(GravityMove, true);
	//
	
	// 날고 있을 때 이동속도 제한
	FVector LocalInput = MoveInput;
	LocalInput.Z = 0.0f;

	if (!LocalInput.IsNearlyZero())
	{
		if (bIsGrounded)//땅바닥에서 걸어가기 위한 함수
		{
			// 드론이 쳐다보는 방향을 월드 방향으로 변환
			FVector WorldDir = GetActorRotation().RotateVector(LocalInput);
			// 위/아래 값을 지워버림 바닥과 평행하게 함
			WorldDir.Z = 0.0f;

			// 평행 방향으로 이동
			if (!WorldDir.IsNearlyZero())
			{
				WorldDir.Normalize();
				FVector DeltaLocation = WorldDir * MoveSpeed * FixedDeltaTime;
				AddActorWorldOffset(DeltaLocation, true);
			}
		}
		else
		{
			// 에어 컨트롤: 속도 40% 제한
			float AirSpeed = MoveSpeed * 0.4f;
			FVector DeltaLocation = LocalInput * AirSpeed * FixedDeltaTime;
			AddActorLocalOffset(DeltaLocation, true);
		}
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
