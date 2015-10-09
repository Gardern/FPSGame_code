// FPSGame created by Gard and Gorm

#pragma once

#include "GameFramework/GameState.h"

#include "FPSGGameMode.h"

#include "FPSGGameState.generated.h"

/**
* Describes a single team
*/
USTRUCT(BlueprintType)
struct FTeam
{
	GENERATED_USTRUCT_BODY()

	//Enum describing which team this is (The team number)
	UPROPERTY(BlueprintReadOnly, Category = TeamStruct)
	ETeamNumber teamNumber;

	//The actual color of this team
	UPROPERTY(BlueprintReadOnly, Category = TeamStruct)
	FLinearColor color;

	//The number of players currently on this team
	UPROPERTY(BlueprintReadOnly, Category = TeamStruct)
	int32 numberOfPlayers;

	//The total score for this team
	UPROPERTY(BlueprintReadOnly, Category = TeamStruct)
	int32 score;

	/**
	* Create a default team
	*/
	FTeam()
	{
		teamNumber = ETeamNumber::BLUE;
		color = FLinearColor(0.0f, 0.0f, 1.0f, 0.5f);
		numberOfPlayers = 0;
		score = 0;
	}

	/**
	* Create the team inTeam
	*/
	FTeam(ETeamNumber inTeam)
	{
		teamNumber = inTeam;
		numberOfPlayers = 0;
		score = 0;

		switch (inTeam)
		{
			case ETeamNumber::BLUE:
				color = FLinearColor(0.0f, 0.0f, 1.0f, 0.5f);
				break;
			case ETeamNumber::RED:
				color = FLinearColor(1.0f, 0.0f, 0.0f, 0.5f);
				break;
			case ETeamNumber::GREEN:
				color = FLinearColor(0.0f, 1.0f, 0.0f, 0.5f);
				break;
			case ETeamNumber::YELLOW:
				color = FLinearColor(1.0f, 1.0f, 0.0f, 0.5f);
				break;
			default:
				teamNumber = ETeamNumber::BLUE;
				color = FLinearColor(0.0f, 0.0f, 1.0f, 0.5f);
				break;
		}
	}
};

UCLASS()
class FPSGAME_API AFPSGGameState : public AGameState
{
	GENERATED_BODY()

public:
	AFPSGGameState();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	/**
	* Server: Update the team score for a players team
	* @param inScore The amount of score to add
	* @param team The players team number
	*/
	void updateTeamScore(int32 inScore, ETeamNumber team);

	/**
	* Server: Function called by onClickQuitToMainMenu in player controller
	*/
	void onClickQuitToMainMenu();

	/**
	* Server: Function called by onClickEndGame in player controller
	*/
	void onClickEndGame();

	bool getHostWantsToQuitGame() const;
	bool getHostWantsToEndGame() const;
	EEndMatchReason getEndMatchReason() const;
	int32 getCurrentHighestMatchScore() const;

	//setCurrentGametype is Server only
	void setCurrentGametype(EGametype inCurrentGametype);

	void setEndMatchReason(EEndMatchReason inEndMatchReason);
	void setCurrentHighestMatchScore(int32 inCurrentHighestMatchScore);

	/**
	* Contains all teams and their information (The team number, the actual team color, number of players on this team and the score)
	*/
	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
	TArray<FTeam> teamList;

protected:
	//Stores what kind of gametype we are playing
	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
	EGametype currentGametype;

private:
	//The remaining time of the match in seconds
	float remainingMatchTime;

	//Set by the host to indicate that he wants to quit to the main menu (Not usable for clients)
	bool hostWantsToQuitGame;

	//Set by the host to indicate that he wants to end the game (Not usable for clients)
	bool hostWantsToEndGame;

	//Stores the reason for ending a match
	EEndMatchReason endMatchReason;

	//This represents the current match score for the highest scoring player (FFA) or team (Team game)
	UPROPERTY(Transient, Replicated)
	int32 currentHighestMatchScore;
};
