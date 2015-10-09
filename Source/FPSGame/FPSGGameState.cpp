// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGGameState.h"
#include "Net/UnrealNetwork.h"

AFPSGGameState::AFPSGGameState() 
	: Super()
{
	currentGametype = EGametype::FFA; //Initialize this to the same as in FPSGGameMode
	remainingMatchTime = 0.0f;
	hostWantsToQuitGame = false;
	hostWantsToEndGame = false;
	endMatchReason = EEndMatchReason::NONE;
	currentHighestMatchScore = 0;
}

//Place all variables that should be replicated in here
void AFPSGGameState::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSGGameState, teamList);
	DOREPLIFETIME(AFPSGGameState, currentHighestMatchScore);
	DOREPLIFETIME(AFPSGGameState, currentGametype);
}

void AFPSGGameState::updateTeamScore(int32 inScore, ETeamNumber team)
{
	if (Role == ROLE_Authority)
	{
		AFPSGGameMode* gameMode = Cast<AFPSGGameMode>(AuthorityGameMode);

		//Only update the team score if we are playing a team game
		if (gameMode != NULL && gameMode->getCurrentGametype() == EGametype::TEAM)
		{
			int32 teamNumber = static_cast<int32>(team);

			if (teamList.IsValidIndex(teamNumber))
			{
				teamList[teamNumber].score += inScore;
			}
		}
	}
}

void AFPSGGameState::onClickQuitToMainMenu()
{
	if (Role == ROLE_Authority)
	{
		hostWantsToQuitGame = true;

		AFPSGGameMode* gameMode = Cast<AFPSGGameMode>(AuthorityGameMode);

		if (gameMode != NULL)
		{
			//TODO Make this protected like in base class + other functions
			if (gameMode->ReadyToEndMatch_Implementation())
			{
				gameMode->EndMatch();
			}
		}
	}
}

void AFPSGGameState::onClickEndGame()
{
	if (Role == ROLE_Authority)
	{
		hostWantsToEndGame = true;

		AFPSGGameMode* gameMode = Cast<AFPSGGameMode>(AuthorityGameMode);

		if (gameMode != NULL)
		{
			//TODO Make this protected like in base class + other functions
			if (gameMode->ReadyToEndMatch_Implementation())
			{
				gameMode->EndMatch();
			}
		}
	}
}

bool AFPSGGameState::getHostWantsToQuitGame() const
{
	return hostWantsToQuitGame;
}

bool AFPSGGameState::getHostWantsToEndGame() const
{
	return hostWantsToEndGame;
}

EEndMatchReason AFPSGGameState::getEndMatchReason() const
{
	return endMatchReason;
}

int32 AFPSGGameState::getCurrentHighestMatchScore() const
{
	return currentHighestMatchScore;
}

void AFPSGGameState::setCurrentGametype(EGametype inCurrentGametype)
{
	if (Role == ROLE_Authority)
	{
		currentGametype = inCurrentGametype;
	}
}

void AFPSGGameState::setEndMatchReason(EEndMatchReason inEndMatchReason)
{
	endMatchReason = inEndMatchReason;
}

void AFPSGGameState::setCurrentHighestMatchScore(int32 inCurrentHighestMatchScore)
{
	currentHighestMatchScore = inCurrentHighestMatchScore;
}
