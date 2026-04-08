// Fill out your copyright notice in the Description page of Project Settings.


#include "Week3DroneController.h"
#include "EnhancedInputSubsystems.h"

AWeek3DroneController::AWeek3DroneController():
	DroneMappingContext(nullptr),
	MoveAction(nullptr),
	LookAction(nullptr)
{
}

void AWeek3DroneController::BeginPlay()
{
	Super::BeginPlay();

	if (ULocalPlayer* LocalPlayer = GetLocalPlayer())
	{
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = LocalPlayer->GetSubsystem<UEnhancedInputLocalPlayerSubsystem>()) 
		{
			if (DroneMappingContext)
			{
				Subsystem->AddMappingContext(DroneMappingContext, 0);
			}
		}
	}
}
