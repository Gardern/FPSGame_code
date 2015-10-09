// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGTeamGameMode.h"
#include "FPSGGameState.h"
#include "FPSGPlayerState.h"

AFPSGTeamGameMode::AFPSGTeamGameMode()
	: Super()
{
	currentGametype = EGametype::TEAM;
	numberOfTeams = 2; //TODO Make sure this is not more than number of teams in ETeamNumber
}

void AFPSGTeamGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AFPSGTeamGameMode::StartPlay()
{
	Super::StartPlay();

	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("AFPSGTeamGameMode::StartPlay"));
}

void AFPSGTeamGameMode::PostLogin(APlayerController* NewPlayer)
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Magenta, TEXT("AFPSGTeamGameMode::PostLogin"));
	
	AFPSGGameState* myGameState = Cast<AFPSGGameState>(GameState);
	AFPSGPlayerState* playerState = Cast<AFPSGPlayerState>(NewPlayer->PlayerState);

	if (myGameState != NULL && playerState != NULL)
	{
		//Selects a team for the player and assign the player to the selected team
		ETeamNumber teamNumber = chooseTeam();

		myGameState->teamList[static_cast<int32>(teamNumber)].numberOfPlayers++;
		playerState->teamNumber = teamNumber;
		playerState->playerColor = myGameState->teamList[static_cast<int32>(teamNumber)].color;
	}
	
	Super::PostLogin(NewPlayer);
}

void AFPSGTeamGameMode::Logout(AController * Exiting)
{
	Super::Logout(Exiting);

	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Magenta, TEXT("AFPSGTeamGameMode::Logout"));

	AFPSGGameState* myGameState = Cast<AFPSGGameState>(GameState);
	AFPSGPlayerState* playerState = Cast<AFPSGPlayerState>(Exiting->PlayerState);

	//A player have quit the game, so there is one less player on the team
	if (myGameState != NULL && playerState != NULL)
	{
		myGameState->teamList[static_cast<int32>(playerState->teamNumber)].numberOfPlayers--;
	}
}

void AFPSGTeamGameMode::InitGameState()
{
	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Yellow, TEXT("AFPSGTeamGameMode::InitGameState"));

	Super::InitGameState();

	//Set the gametype
	setCurrentGametype(EGametype::TEAM);
	
	//Create all the teams
	createTeams();
}

void AFPSGTeamGameMode::onScoreUpdate()
{
	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Yellow, TEXT("AFPSGGameState::onScoreUpdate"));

	AFPSGGameState* gameState = GetGameState<AFPSGGameState>();

	//Find the highest player or team score so far and update it
	if (gameState != NULL)
	{
		int32 highestMatchScore = findHighestScore();
		gameState->setCurrentHighestMatchScore(highestMatchScore);
	}

	Super::onScoreUpdate();
}

int32 AFPSGTeamGameMode::findHighestScore() const
{
	Super::findHighestScore();
	
	AFPSGGameState* myGameState = GetGameState<AFPSGGameState>();

	int32 highestScore = TNumericLimits<int32>::Min();

	if (myGameState != NULL)
	{
		for (const FTeam& teamEntry : myGameState->teamList)
		{
			if (teamEntry.score > highestScore)
			{
				highestScore = teamEntry.score;
			}
		}
	}
	return highestScore;
}

void AFPSGTeamGameMode::createTeams() const
{
	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Yellow, TEXT("AFPSGTeamGameMode::createTeams"));

	AFPSGGameState* myGameState = GetGameState<AFPSGGameState>();

	//Create all the teams
	if (myGameState != NULL)
	{
		for (int32 i = 0; i < numberOfTeams; ++i)
		{
			ETeamNumber teamNumber = static_cast<ETeamNumber>(i);
			FTeam aTeam(teamNumber);
			myGameState->teamList.Add(aTeam);
		}
	}
}

ETeamNumber AFPSGTeamGameMode::chooseTeam() const
{
	AFPSGGameState* myGameState = Cast<AFPSGGameState>(GameState);
	
	if (myGameState == NULL) return static_cast<ETeamNumber>(0);

	ETeamNumber teamNumber = static_cast<ETeamNumber>(0);
	int32 lowestNumberOfPlayers = TNumericLimits<int32>::Max();

	//Find the team with the least amount of players
	for (int32 i = 0; i < myGameState->teamList.Num(); ++i)
	{
		if (myGameState->teamList.IsValidIndex(i))
		{
			if (myGameState->teamList[i].numberOfPlayers < lowestNumberOfPlayers)
			{
				lowestNumberOfPlayers = myGameState->teamList[i].numberOfPlayers;
				teamNumber = myGameState->teamList[i].teamNumber;
			}
		}
	}
	return teamNumber;
}
