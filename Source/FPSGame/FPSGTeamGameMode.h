// FPSGame created by Gard and Gorm

#pragma once

#include "FPSGGameMode.h"
#include "FPSGTeamGameMode.generated.h"

UCLASS()
class FPSGAME_API AFPSGTeamGameMode : public AFPSGGameMode
{
	GENERATED_BODY()

public:
	AFPSGTeamGameMode();

	virtual void Tick(float DeltaSeconds) override;

	virtual void StartPlay() override;

	/**
	* Called after a successful login
	*/
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/**
	* Called when a Controller with a PlayerState leaves the match
	*/
	virtual void Logout(AController * Exiting) override;

	/**
	* Initialize GameState data
	*/
	virtual void InitGameState() override;

	/**
	* Called when a players score is updated
	*/
	virtual void onScoreUpdate() override;

	/**
	* Finds the highest team score currently in the match
	* @return Returns the highest team score, or TNumericLimits<int32>::Min() if it failed to find a score
	*/
	virtual int32 findHighestScore() const override;

protected:
	/**
	* Creates all teams for the team list in gamestate
	*/
	void createTeams() const;

	/**
	* Chooses a team for the player to join. Selects the team with least amount of players or the first team if equal
	* @return Returns ETeamNumber the team number for the selected team, or team number 0/BLUE (first team) if function failed
	*/
	ETeamNumber chooseTeam() const;

	//Default number of teams for a team game
	int32 numberOfTeams;
};
