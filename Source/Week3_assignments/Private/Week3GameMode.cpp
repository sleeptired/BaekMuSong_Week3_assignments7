// Fill out your copyright notice in the Description page of Project Settings.


#include "Week3GameMode.h"
#include "Week3Drone.h"
#include "Week3DroneController.h"

AWeek3GameMode::AWeek3GameMode()
{
	DefaultPawnClass = AWeek3Drone::StaticClass();
	PlayerControllerClass = AWeek3DroneController::StaticClass();
}
