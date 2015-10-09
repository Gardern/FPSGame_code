// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGPlayerState.h"
#include "FPSGGameState.h"
#include "FPSGCharacter.h"
#include "Net/UnrealNetwork.h"

AFPSGPlayerState::AFPSGPlayerState() 
	: Super()
{
	numberOfKills = 0;
	numberOfDeaths = 0;
	numberOfAssists = 0;
	teamNumber = ETeamNumber::NONE;
	playerColor = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f);
}

//Place all variables that should be replicated in here
void AFPSGPlayerState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME(AFPSGPlayerState, numberOfKills);
	DOREPLIFETIME(AFPSGPlayerState, numberOfDeaths);
	DOREPLIFETIME(AFPSGPlayerState, numberOfAssists);
	DOREPLIFETIME(AFPSGPlayerState, teamNumber);
	DOREPLIFETIME(AFPSGPlayerState, playerColor);
}

void AFPSGPlayerState::updateKills(int32 amount)
{
	if (Role == ROLE_Authority)
	{
		numberOfKills += amount;
	}
}

void AFPSGPlayerState::updateDeaths(int32 amount)
{
	if (Role == ROLE_Authority)
	{
		numberOfDeaths += amount;
	}
}

void AFPSGPlayerState::updateScore(int32 amount)
{
	if (Role == ROLE_Authority)
	{
		UWorld* world = GetWorld();
		AFPSGGameState* gameState = world != NULL ? world->GetGameState<AFPSGGameState>() : NULL;
		AFPSGGameMode* gameMode = world != NULL ? world->GetAuthGameMode<AFPSGGameMode>() : NULL;

		if (gameState == NULL || gameMode == NULL) return;

		//Update the players individual score
		Score += amount;
		
		//Update the team score
		gameState->updateTeamScore(amount, teamNumber);
		
		//Notify the game mode that a players score or team score has been updated
		gameMode->onScoreUpdate();
	}
}
