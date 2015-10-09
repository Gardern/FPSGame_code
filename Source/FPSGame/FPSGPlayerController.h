// FPSGame created by Gard and Gorm

#pragma once

#include "FPSGGameSession.h"
#include "FPSGGameMode.h"

#include "GameFramework/PlayerController.h"
#include "FPSGPlayerController.generated.h"

UCLASS(Blueprintable)
class FPSGAME_API AFPSGPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	AFPSGPlayerController();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	// Called after PreInitializeComponents and before BeginPlay
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when this actor is about to be destroyed
	virtual void Destroyed() override;

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	// Called to bind functionality to input
	virtual void SetupInputComponent() override;

	/**
	* Function performs respawning of the player
	*/
	virtual void UnFreeze() override;

	/**
	* Calls UnPossess for the pawn
	* @param inPawn The pawn that we unpossess (Must equal this->Pawn or this->GetPawn())
	*/
	virtual void PawnPendingDestroy(APawn* inPawn) override;

	/**
	* Used to start a online session
	* Gives clients the ability to locally start an online session as they dont have access to FPSGGameSession.startSession
	* @param sessionName Name of session
	* @return true if successful, false otherwise
	*/
	bool startSession(FName sessionName) const;

	/**
	* Used to end an online session
	* Gives clients the ability to locally end an online session as they dont have access to FPSGGameSession.endSession
	* @param sessionName Name of session
	* @return true if successful, false otherwise
	*/
	bool endSession(FName sessionName) const;

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
	* Function called when a player clicks the Quit to Main Menu button from the In Game Menu.
	* Will only attempt to quit the game if the currently running session and match are in progress
	*/
	UFUNCTION(BlueprintCallable, Category = UserInterface)
	void onClickQuitToMainMenu();

	/**
	* Server: Function called when the host player clicks the End Game button from the In Game Menu.
	* Will only attempt to end the game if the currently running session and match are in progress
	*/
	UFUNCTION(BlueprintAuthorityOnly, BlueprintCallable, Category = UserInterface)
	void onClickEndGame() const;

	/**
	* Called on local client and server when dying
	*/
	UFUNCTION()
	void onRep_DieOwnerOnly();

	/**
	* Called on local client and server when spawning
	*/
	UFUNCTION()
	void onRep_SpawnOwnerOnly();

	/**
	* Calculates the time in seconds until the player respawns.
	* @return time until respawn (in seconds) if isAlive = false, otherwise or if failed returns 0
	*/
	UFUNCTION(BlueprintPure, Category = "Respawn")
	float timeUntilRespawn();

	/**
	* Client function called from server to notify the client that it should start its session
	*/
	UFUNCTION(Client, Reliable, WithValidation)
	void clientStartSession();
	void clientStartSession_Implementation();
	bool clientStartSession_Validate();

	/**
	* Client function called from server to notify the client that it should end its session
	*/
	UFUNCTION(Client, Reliable, WithValidation)
	void clientEndSession() const;
	void clientEndSession_Implementation() const;
	bool clientEndSession_Validate() const;

	/**
	* Client function called from server to notify the client that it should destroy its session
	*/
	UFUNCTION(Client, Reliable, WithValidation)
	void clientDestroySession() const;
	void clientDestroySession_Implementation() const;
	bool clientDestroySession_Validate() const;

	/**
	* Function called from server to notify all remote clients and itself that an online match has started
	*/
	UFUNCTION(Client, Reliable, WithValidation) //TODO Use this
	void clientOnlineMatchHasStarted();
	void clientOnlineMatchHasStarted_Implementation();
	bool clientOnlineMatchHasStarted_Validate();

	/**
	* Function called from server to notify all remote clients and itself that an online match has ended
	* @param endMatchReason The reason for ending the match
	*/
	UFUNCTION(Client, Reliable, WithValidation)
	void clientOnlineMatchHasEnded(EEndMatchReason endMatchReason);
	void clientOnlineMatchHasEnded_Implementation(EEndMatchReason endMatchReason);
	bool clientOnlineMatchHasEnded_Validate(EEndMatchReason endMatchReason);

	bool getIsAlive() const;
	FDateTime getTimeOfDeath() const;
	float getRespawnTime() const;
	bool isInGameMenuOpen() const;
	bool getPostLoginFinished() const;

	void setDieCallback(bool inDieCallback);
	void setSpawnCallback(bool inSpawnCallback);
	void setPostLoginFinished(bool inPostLoginFinished);

	/*
	* Class variables exposed to blueprints
	*/
	//The UMG HUDAlive instance
	UPROPERTY(BlueprintReadWrite, Category = UserInterface)
	UUserWidget* HUDAlive;

	//The UMG HUDDead instance
	UPROPERTY(BlueprintReadWrite, Category = UserInterface)
	UUserWidget* HUDDead;

	//The UMG HUDInGameMenu instance
	UPROPERTY(BlueprintReadWrite, Category = UserInterface)
	UUserWidget* HUDInGameMenu;

	//The UMG HUDInGameLeaderboard instance
	UPROPERTY(BlueprintReadWrite, Category = UserInterface)
	UUserWidget* HUDInGameLeaderboard;

private:
	/**
	* Retrieves the GameSession object (Only accessible on server, will be NULL on clients)
	* @return the GameSession reference set in game mode
	*/
	AFPSGGameSession* getGameSession() const;

	/**
	* Functions that handles input
	*/
	void openLeaderboard();
	void openInGameMenu();

	/**
	* Client function that resets the camera when the player dies
	* @param CameraLocation The location to set the camera on
	* @param CameraRotation The rotation to use for the camera
	*/
	UFUNCTION(Client, Reliable, WithValidation)
	void clientSetDeathCamera(const FVector& cameraLocation, const FRotator& cameraRotation);
	void clientSetDeathCamera_Implementation(const FVector& cameraLocation, const FRotator& cameraRotation);
	bool clientSetDeathCamera_Validate(const FVector& cameraLocation, const FRotator& cameraRotation);

	/**
	* Server function called by clients in clientOnlineMatchHasEnded to notify server that they have done all work they should before the match is finished
	* @param endMatchReason The reason for ending the match
	*/
	UFUNCTION(Server, Reliable, WithValidation)
	void serverPostOnlineMatchHasEnded(EEndMatchReason endMatchReason);
	void serverPostOnlineMatchHasEnded_Implementation(EEndMatchReason endMatchReason);
	bool serverPostOnlineMatchHasEnded_Validate(EEndMatchReason endMatchReason);

	//true if the player is alive
	bool isAlive;

	//Stores the system time each time the player dies
	struct FDateTime timeOfDeath;

	//true if the in game menu is open
	bool inGameMenuIsOpen;

	//true if the in game menu is disabled
	bool inGameMenuIsDisabled;

	//true if the in game leaderboard is open
	bool inGameLeaderboardIsOpen;

	//A helper timer used to invoke clientStartSession until it succeeds
	FTimerHandle invokeStartSessionTimerHandle;

	//The UMG HUD visible on the players screen when alive
	UPROPERTY(EditDefaultsOnly, Category = UserInterface)
	TSubclassOf<UUserWidget> HUDAliveTemplate;

	//The UMG HUD visible on the players screen when dead
	UPROPERTY(EditDefaultsOnly, Category = UserInterface)
	TSubclassOf<UUserWidget> HUDDeadTemplate;

	//The UMG HUD for the in game menu
	UPROPERTY(EditDefaultsOnly, Category = UserInterface)
	TSubclassOf<UUserWidget> HUDInGameMenuTemplate;

	//The UMG HUD for the in game leaderboard
	UPROPERTY(EditDefaultsOnly, Category = UserInterface)
	TSubclassOf<UUserWidget> HUDInGameLeaderboardTemplate;

	//Calls the callback function onRep_DieOwnerOnly on local client and server each time a player dies
	UPROPERTY(ReplicatedUsing = onRep_DieOwnerOnly)
	bool dieCallback;

	//Calls the callback function onRep_SpawnOwnerOnly on local client and server each time a player spawns
	UPROPERTY(ReplicatedUsing = onRep_SpawnOwnerOnly)
	bool spawnCallback;

	//This is set to MinRespawnDelay from gamemode. Makes MinRespawnDelay accessible on clients
	UPROPERTY(Replicated, Transient)
	float respawnTime;

	//Indicates that this player is successfully logged in and is ready to play the match. Set In PostLogin
	UPROPERTY(Replicated)
	bool postLoginFinished;

	//Indicates that this player is ready to end the match. Set locally by client in clientOnlineMatchHasEnded and by server in serverPostOnlineMatchHasEnded
	UPROPERTY(Replicated)
	bool postMatchEndFinished;
};
