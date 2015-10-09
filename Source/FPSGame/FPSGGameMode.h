// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "GameFramework/GameMode.h"

#include "FPSGPlayerStart.h"
#include "FPSGWeapon.h"

#include "FPSGGameMode.generated.h"

/**
* Defines different gametypes we can play
*/
UENUM(BlueprintType)
enum class EGametype : uint8
{
	/** A Free for all gametype for 1v1, FFA (Used by AFPSGFFAGameMode) */
	FFA,

	/** A team gametype (Used by AFPSGTeamGameMode) */
	TEAM
};

/**
* Defines the different teams (team numbers) for team gametypes
*/
UENUM(BlueprintType)
enum class ETeamNumber : uint8
{
	/** The blue team */
	BLUE = 0,

	/** The red team */
	RED = 1,

	/** The Green team */
	GREEN = 2,

	/** The Yellow team */
	YELLOW = 3,

	/** None value (Default value to indicate no team) */
	NONE = 255
};

/**
* Defines different reasons for ending a game match
*/
UENUM()
enum class EEndMatchReason : uint8
{
	/** None value (Default, or to indicate that the match has not ended) */
	NONE,

	/** The match was ended due to normal behavior (e.g. someone won or time ran out) */
	NORMAL_END_MATCH,

	/** The match was ended because the host ended it */
	HOST_END_MATCH,

	/** The match was ended because the host quit to main menu and disconnected */
	HOST_QUIT_MATCH
};

UCLASS()
class FPSGAME_API AFPSGGameMode : public AGameMode
{
	GENERATED_BODY()
		
public:
	AFPSGGameMode();

	virtual void Tick(float DeltaSeconds) override;

	virtual void StartPlay() override;

	//Called when the match has started
	virtual void HandleMatchHasStarted() override;

	//Called when the match has ended
	virtual void HandleMatchHasEnded() override;

	//Custom logic that decides when a match should be started
	virtual bool ReadyToStartMatch_Implementation() override;

	//Custom logic that decides when a match should be ended
	virtual bool ReadyToEndMatch_Implementation() override;

	/**
	* Accept or reject a player attempting to join the server
	*/
	virtual void PreLogin(const FString& Options, const FString& Address, const TSharedPtr<class FUniqueNetId>& UniqueId, FString& ErrorMessage) override;

	/**
	* Called to login new players by creating a player controller
	*/
	virtual APlayerController* Login(class UPlayer* NewPlayer, ENetRole RemoteRole, const FString& Portal, const FString& Options, const TSharedPtr<class FUniqueNetId>& UniqueId, FString& ErrorMessage) override;
	
	/**
	* Called after a successful login
	*/
	virtual void PostLogin(APlayerController* NewPlayer) override;

	/**
	* Function to find a spawnpoint for a player at the start of a match and when respawning
	* @param player The controller of the player to find a spawnpoint for
	*/
	//virtual AActor* ChoosePlayerStart(AController* Player) override; //TODO ChoosePlayerStart overwrite in FFA and Team Gamemodes

	virtual AActor* ChoosePlayerStart_Implementation(AController* Player) override; //TODO ChoosePlayerStart overwrite in FFA and Team Gamemodes  //TODO Test that function is called

	/**
	* Function returns true if the current start point should be used to spawn the player.
	* We return false here so that we always find a new spawning point
	* @param Player The controller of the player to use the current start point for (not in use here)
	*/
	virtual bool ShouldSpawnAtStartSpot(AController* Player) override;

	/**
	* Returns the game session class to use
	*/
	virtual TSubclassOf<AGameSession> GetGameSessionClass() const override;

	/**
	* Notify the game mode that a player killed another player
	* @param killer The player that killed killedPlayer
	* @param killedPlayer The player that was killed by killer
	*/
	void notifyKill(class AFPSGPlayerController* killer, class AFPSGPlayerController* killedPlayer);

	/**
	* Called when a players score is updated
	*/
	virtual void onScoreUpdate();
	
	/**
	* Called to find the highest score in the match
	*/
	virtual int32 findHighestScore() const;

	/**
	* This function ends the game and returns all players to main menu or lobby
	* @param endMatchReason The reason for ending the match
	*/
	void terminateMatch(EEndMatchReason endMatchReason) const;

	EGametype getCurrentGametype() const;
	const TArray<TSubclassOf<AFPSGWeapon>>& getStartWeapons() const;

protected:
	/**
	* Function checks whether a playerstart is curently valid for use as a spawn location
	* @param playerStart The playerstart to check if valid
	* @param player The controller of the player to use the current start point for (not in use here)
	* @return Returns true if playerstart is valid to use as spawn location
	*/
	bool isPlayerstartValid(AFPSGPlayerStart* playerStart) const;

	/**
	* Sets the gametype we are using (This also sets the gametype in the gamestate class
	* Should not be able to set this from outside of gamemode
	*/
	void setCurrentGametype(EGametype inCurrentGametype);

	//Stores what kind of gametype we are playing
	EGametype currentGametype;

	//The weapons to start with for this gamemode
	UPROPERTY(EditDefaultsOnly, Category = Rules)
	TArray<TSubclassOf<AFPSGWeapon>> startWeapons;

	//The total score needed to win
	UPROPERTY(EditDefaultsOnly, Category = Rules) //TODO ShooterGame uses config, what is that?
	int32 scoreToWin;

	//The score achieved for a kill
	UPROPERTY(EditDefaultsOnly, Category = Rules)
	int32 killScore;

	//The total time of the match in seconds (Ends the match if Score To Win has not been reached)
	UPROPERTY(EditDefaultsOnly, Category = Rules)
	int32 totalMatchTime;

	//true to turn on radar
	UPROPERTY(EditDefaultsOnly, Category = Rules)
	bool useRadar;
};
