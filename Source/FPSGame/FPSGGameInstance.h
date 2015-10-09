// FPSGame created by Gard and Gorm

#pragma once

#include "FPSGame.h"
#include "FPSGGameSession.h"
#include "Engine/GameInstance.h"
#include "FPSGGameInstance.generated.h"

/*
USTRUCT() //BlueprintType
struct FMatchSettings
{
	GENERATED_USTRUCT_BODY()

	FString mapName;
	FString gameType;
	bool isLanMatch;
	int32 maxNumberOfPlayers;
	bool isPresence;
	FString matchURL;
	
	FMatchSettings()
		: mapName(TEXT("")), gameType(TEXT("")), isLanMatch(false), maxNumberOfPlayers(0), isPresence(false), matchURL(TEXT(""))
	{
	}
	
	void defaultSettings()
	{
		mapName = TEXT("TestMap1_Gard"); //Lobby //TestMap1_Gard
		gameType = TEXT("FFA");
		isLanMatch = true;
		maxNumberOfPlayers = 8;
		isPresence = true;
		matchURL = FString::Printf(TEXT("/Game/FPSGameContent/Levels/%s?game=%s%s"), *mapName, TEXT("?listen"), isLanMatch ? TEXT("?bIsLanMatch") : TEXT(""));
	}
};
*/

UCLASS()
class FPSGAME_API UFPSGGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	UFPSGGameInstance();

	/**
	* Server: Called to end a game with the specified URL. (Calls ServerTravel)
	* @param mapName Map to travel to after ending the game.
	* @param isLanMatch If this is a lan match or not.
	*/
	void endGame(const FString& mapName, bool isLanMatch) const;

	/**
	* Called to host a game with the specified settings. Calls hostSession in game session
	* @param mapName Map to use for the hosted game.
	* @param gameType Gametype to use for the hosted game.
	* @param isLanMatch If this is a lan match or not.
	* @param maxNumberOfPlayers Max number of players for the hosted game.
	* @return true if successful, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = Network)
	bool hostGame(const FString& mapName, const FString& gameType, bool isLanMatch, int32 maxNumberOfPlayers);

	/**
	* Called to search for games. Calls findSessions in game session
	* @param isLanMatch If this is a lan match or not.
	* @return true if successful, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = Network)
	bool findGames(bool isLanMatch);

	/**
	* Called to join a game. Calls joinSession in game session
	* @param serverIndex The index of the server to join
	* @return true if successful, false otherwise
	*/
	UFUNCTION(BlueprintCallable, Category = Network)
	bool joinGame(int32 serverIndex);

	/**
	* Called to start a game with the specified URL. (Calls ServerTravel).
	* Will only start the game if the session has been created/ended and contains at least one player
	* @param mapName Map to use for the started game.
	* @param isLanMatch If this is a lan match or not.
	* @return true if successful, false otherwise
	*/
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = Network)
	bool startGame(const FString& mapName, bool isLanMatch);

	//Represents the number of registered players to the session when starting the game (Does not update dynamically if players connect/disconnects)
	int32 numRegisteredPlayersAtSessionStart; //TODO Always keep it updated?

	//UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = Network)
	//void setMatchSettings(const FString& mapName, const FString& gameType, bool isLanMatch, int32 maxNumberOfPlayers, bool isPresence);

	//UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = Network)
	//void setDefaultMatchSettings();

	//UPROPERTY(BlueprintReadWrite)
	//FMatchSettings currentMatchSettings;

private:
	/**
	* Retrieves the GameSession object (Only accessible on server, will be NULL on clients)
	* @return the GameSession reference set in game mode
	*/
	AFPSGGameSession* getGameSession() const;
};
