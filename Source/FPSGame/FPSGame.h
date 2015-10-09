// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Engine.h"

#include "Runtime/UMG/Public/UMG.h"
//#include "Runtime/UMG/Public/UMGStyle.h"
//#include "Runtime/UMG/Public/Slate/SObjectWidget.h"
//#include "Runtime/UMG/Public/IUMGModule.h"
//#include "Runtime/UMG/Public/Blueprint/UserWidget.h"

//For projectiles used in projectile weapons
#define COLLISION_PROJECTILE			ECC_GameTraceChannel1

//Used for line tracing (In single line trace)
#define COLLISION_INSTANT_HIT			ECC_GameTraceChannel2

//For all pickups
#define COLLISION_PICKUP				ECC_GameTraceChannel3

//Used for line tracing (In multi line trace)
#define COLLISION_INSTANT_HIT_MULTI		ECC_GameTraceChannel4

//Custom macro to check if we are playing the game from inside the editor (Set to 0 when packaging the game)
#define isPlayingFromEditor 1

//Name of the lobby map
const FName LOBBY_MAP_NAME(TEXT("Lobby"));

//Name of the Game entry map
const FName GAMEENTRY_MAP_NAME(TEXT("GameEntry"));
