// FPSGame created by Gard and Gorm

#pragma once

#include "FPSGGameSession.h"

#include "GameFramework/PlayerController.h"
#include "FPSGMainMenuPlayerController.generated.h"

/**
* Represent a single element in the game list (Shows all server data for a particular server)
*/
USTRUCT(BlueprintType)
struct FGameListElement
{
	GENERATED_USTRUCT_BODY()

	//The gametype
	UPROPERTY(BlueprintReadOnly, Category = ServerData)
	FString gameType;

	//The mapname
	UPROPERTY(BlueprintReadOnly, Category = ServerData)
	FString mapName;

	//The number of current players
	UPROPERTY(BlueprintReadOnly, Category = ServerData)
	int32 currentPlayers;

	//The number of maximum players allowed
	UPROPERTY(BlueprintReadOnly, Category = ServerData)
	int32 maxPlayers;

	//The ping
	UPROPERTY(BlueprintReadOnly, Category = ServerData)
	int32 ping;

	//The type of match (etc, LAN or Online)
	UPROPERTY(BlueprintReadOnly, Category = ServerData)
	FString matchType;

	FGameListElement()
	{
		gameType = "";
		mapName = "";
		currentPlayers = 0;
		maxPlayers = 0;
		ping = 0;
		matchType = "";
	}
};

/**
* This is the player controller class that is used when the player is in the main menus and lobby, before playing any game
*/
UCLASS(Blueprintable)
class FPSGAME_API AFPSGMainMenuPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFPSGMainMenuPlayerController();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when this actor is about to be destroyed
	virtual void Destroyed() override;
	
	virtual void Tick(float DeltaSeconds) override;

	//TODO Document later
	void onHostGameComplete();
	void onHostGameCompleteSuccess();
	void onSearchForGamesComplete();
	void onSearchForGamesCompleteSuccess();
	void onJoinGameComplete();
	void onJoinGameCompleteSuccess();

	/**
	* Used to destroy a online session
	* Gives clients the ability to locally destroy an online session as they dont have access to FPSGGameSession.endAndDestroySession
	* @param sessionName Name of session
	* @return true if successful, false otherwise
	*/
	bool destroySession(FName sessionName) const;

	/**
	* Returns the client to the main menu
	*/
	virtual void ClientReturnToMainMenu_Implementation(const FString& ReturnReason) override;

	/**
	* Function called when a player clicks the Leave button in the lobby
	* Will only attempt to leave the game if the currently running session is pending or has been ended
	*/
	UFUNCTION(BlueprintCallable, Category = UserInterface)
	void onClickLeave();

	/**
	* Notify the clients and server that the server is starting a match, and let them perform work before it is started
	*/
	UFUNCTION(Client, Reliable, WithValidation)
	void clientPreStartOnlineMatch();
	void clientPreStartOnlineMatch_Implementation();
	bool clientPreStartOnlineMatch_Validate();

	/**
	* Client function called from server to notify the client that it should destroy its session
	*/
	UFUNCTION(Client, Reliable, WithValidation)
	void clientDestroySession() const;
	void clientDestroySession_Implementation() const;
	bool clientDestroySession_Validate() const;

	/**
	* Class variables exposed to blueprints
	*/
	//The UMG HUDJoinGameMenu instance
	UPROPERTY(BlueprintReadWrite, Category = UserInterface)
	UUserWidget* HUDJoinGameMenu;

	//The UMG HUDLobbyMenu instance
	UPROPERTY(BlueprintReadWrite, Category = UserInterface)
	UUserWidget* HUDLobbyMenu;

	//true if we are currently searching for servers
	UPROPERTY(BlueprintReadWrite, Category = Network)
	bool searchingForServers;

	//Stores the currently selected server index from the game list
	UPROPERTY(BlueprintReadWrite, Category = Network)
	int32 currentServerIndex;

	//Stores all the server data for the game list
	UPROPERTY(BlueprintReadOnly, Category = ServerData)
	TArray<FGameListElement> gameListData; //TODO This array should be initialized to a start capacity

	//true if we have finished searching for games
	UPROPERTY(BlueprintReadWrite, Category = Network)
	bool finishedSearchingForGames;

private:
	/**
	* Retrieves the GameSession object (Only accessible on server, will be NULL on clients)
	* @return the GameSession reference set in game mode
	*/
	AFPSGGameSession* getGameSession() const;
};
