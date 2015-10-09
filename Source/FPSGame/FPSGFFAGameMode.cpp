// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGFFAGameMode.h"
#include "FPSGGameState.h"

AFPSGFFAGameMode::AFPSGFFAGameMode()
	: Super()
{
	currentGametype = EGametype::FFA;
}

void AFPSGFFAGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AFPSGFFAGameMode::StartPlay()
{
	Super::StartPlay();

	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("AFPSGFFAGameMode::StartPlay"));
}

void AFPSGFFAGameMode::InitGameState()
{
	Super::InitGameState();

	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Yellow, TEXT("AFPSGFFAGameMode::InitGameState"));

	//Set the gametype
	setCurrentGametype(EGametype::FFA);
}

void AFPSGFFAGameMode::onScoreUpdate()
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, "AFPSGFFAGameMode::onScoreUpdate");

	AFPSGGameState* gameState = GetGameState<AFPSGGameState>();

	//Find the highest player or team score so far and update it
	if (gameState != NULL)
	{
		int32 highestMatchScore = findHighestScore();
		gameState->setCurrentHighestMatchScore(highestMatchScore);
	}

	Super::onScoreUpdate();
}

int32 AFPSGFFAGameMode::findHighestScore() const
{
	Super::findHighestScore();
	
	AFPSGGameState* myGameState = GetGameState<AFPSGGameState>();

	int32 highestScore = TNumericLimits<int32>::Min();

	if (myGameState != NULL)
	{
		for (const APlayerState* playerState : myGameState->PlayerArray)
		{
			if (playerState != NULL && playerState->Score > highestScore)
			{
				highestScore = playerState->Score;
			}
		}
	}
	return highestScore;
}
