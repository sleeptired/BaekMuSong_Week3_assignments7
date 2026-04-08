// Fill out your copyright notice in the Description page of Project Settings.


#include "Week3GameMode.h"
#include "Week3Drone.h"

AWeek3GameMode::AWeek3GameMode()
{
	DefaultPawnClass = AWeek3Drone::StaticClass();
}
