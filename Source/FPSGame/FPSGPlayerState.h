// FPSGame created by Gard and Gorm

#pragma once

#include "GameFramework/PlayerState.h"

#include "FPSGGameMode.h"

#include "FPSGPlayerState.generated.h"

UCLASS()
class FPSGAME_API AFPSGPlayerState : public APlayerState
{
	GENERATED_BODY()

public:
	// Sets default values for this player states properties
	AFPSGPlayerState();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	/**
	* Server: if amount > 0, increase kills for this player by the specified amount, otherwise decrease by the specified amount
	* @param amount The amount to increase kills by
	*/
	void updateKills(int32 amount);

	/**
	* Server: if amount > 0, increase deaths for this player by the specified amount, otherwise decrease by the specified amount
	* @param amount The amount to increase deaths by
	*/
	void updateDeaths(int32 amount);

	/**
	* Server: if amount > 0, increase this players score and team score by the specified amount, otherwise decrease by the specified amount
	* @param amount The amount of score to add
	*/
	void updateScore(int32 amount);

	//The number of kills for a player
	UPROPERTY(Transient, Replicated, BlueprintReadOnly, Category = FPSGPlayerState)
	int32 numberOfKills;

	//The number of deaths for a player
	UPROPERTY(Transient, Replicated, BlueprintReadOnly, Category = FPSGPlayerState)
	int32 numberOfDeaths;

	//The number of assists for a player
	UPROPERTY(Transient, Replicated, BlueprintReadOnly, Category = FPSGPlayerState)
	int32 numberOfAssists;
	
	//TODO Use a setTeamNumber function that also sets the character mesh color and variabel playerColor (See shootergame)
	//Specifies which team this player is on
	UPROPERTY(Transient, Replicated, BlueprintReadOnly, Category = FPSGPlayerState)
	ETeamNumber teamNumber;

	//The color of this player (The color the player have chosen or the team color for team games)
	UPROPERTY(Transient, Replicated, BlueprintReadOnly, Category = FPSGPlayerState)
	FLinearColor playerColor; //TODO: Add for feature. Should be able to select player color in settings (Not used for team games)

private:
};
