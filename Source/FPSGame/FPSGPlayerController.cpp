// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGPlayerController.h"
#include "FPSGCharacter.h"
#include "FPSGGameMode.h"
#include "FPSGPlayerState.h"
#include "FPSGUtility.h"
#include "FPSGGameInstance.h"
#include "FPSGGameState.h"

// Sets default values
AFPSGPlayerController::AFPSGPlayerController()
	: Super()
{
	HUDAlive = NULL;
	HUDDead = NULL;
	HUDInGameMenu = NULL;
	HUDInGameLeaderboard = NULL;
	isAlive = true;
	timeOfDeath = FDateTime::MinValue();
	inGameMenuIsOpen = false;
	inGameMenuIsDisabled = false;
	inGameLeaderboardIsOpen = false;
	dieCallback = false;
	spawnCallback = false;
	respawnTime = 0.0f;
	postLoginFinished = false;
	postMatchEndFinished = false;
}

//Place all variables that should be replicated in here
void AFPSGPlayerController::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AFPSGPlayerController, dieCallback, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFPSGPlayerController, spawnCallback, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFPSGPlayerController, postLoginFinished, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFPSGPlayerController, postMatchEndFinished, COND_OwnerOnly);
	DOREPLIFETIME(AFPSGPlayerController, respawnTime);
}

void AFPSGPlayerController::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	UWorld* world = GetWorld();
	if (world != NULL)
	{
		AFPSGGameMode* gameMode = world->GetAuthGameMode<AFPSGGameMode>();

		if (Role == ROLE_Authority && gameMode != NULL)
		{
			respawnTime = gameMode->MinRespawnDelay;
		}
	}
}

void AFPSGPlayerController::BeginPlay()
{
	Super::BeginPlay();

	FInputModeGameOnly inputMode;
	SetInputMode(inputMode);
}

void AFPSGPlayerController::Destroyed()
{
	Super::Destroyed();
	
	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Yellow, TEXT("AFPSGPlayerController::Destroyed"));

	//Make sure all widgets that belongs to this player controller are destroyed
	FPSGUtility::destroyUserWidget(&HUDAlive);
	FPSGUtility::destroyUserWidget(&HUDDead);
	FPSGUtility::destroyUserWidget(&HUDInGameMenu);
	FPSGUtility::destroyUserWidget(&HUDInGameLeaderboard);
}

void AFPSGPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

// Called to bind functionality to input
void AFPSGPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();
	
	InputComponent->BindAction("OpenLeaderboard", IE_Pressed, this, &AFPSGPlayerController::openLeaderboard);
	InputComponent->BindAction("OpenInGameMenu", IE_Pressed, this, &AFPSGPlayerController::openInGameMenu);
}

void AFPSGPlayerController::UnFreeze()
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Yellow, TEXT("AFPSGPlayerController::UnFreeze"));

	ServerRestartPlayer();
}

void AFPSGPlayerController::PawnPendingDestroy(APawn* inPawn)
{
	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("AFPSGPlayerController::PawnPendingDestroy"));
	}

	FVector lastDeathLocation = inPawn->GetActorLocation();
	FVector cameraLocation = lastDeathLocation + FVector(0, 0, 300.0f);
	FRotator cameraRotation(-60.0f, 0.0f, 0.0f);
	
	Super::PawnPendingDestroy(inPawn);

	clientSetDeathCamera(cameraLocation, cameraRotation);
}

bool AFPSGPlayerController::startSession(FName sessionName) const
{
	bool success = false;

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (sessions.IsValid())
	{
		//Start the session
		success = sessions->StartSession(sessionName);
		if (success)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("AFPSGPlayerController::startSession success"));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("AFPSGPlayerController::startSession NOT success"));
		}
	}
	return success;
}

bool AFPSGPlayerController::endSession(FName sessionName) const
{
	bool success = false;

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (sessions.IsValid())
	{
		//End the session
		success = sessions->EndSession(sessionName);
		if (success)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("AFPSGPlayerController::endSession success"));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("AFPSGPlayerController::endSession NOT success"));
		}
	}
	return success;
}

bool AFPSGPlayerController::destroySession(FName sessionName) const
{
	bool success = false;

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (sessions.IsValid())
	{
		//Destroy the session
		success = sessions->DestroySession(sessionName);
		if (success)
		{
			GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("AFPSGPlayerController::destroySession success"));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("AFPSGPlayerController::destroySession NOT success"));
		}
	}
	return success;
}

void AFPSGPlayerController::ClientReturnToMainMenu_Implementation(const FString& ReturnReason)
{
	//GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("AFPSGPlayerController::ClientReturnToMainMenu_Implementation"));

	FString URL = FString::Printf(TEXT("/Game/FPSGameContent/Levels/%s"), *GAMEENTRY_MAP_NAME.ToString());

	//GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("ClientTravel URL: ") + URL);

	ClientTravel(URL, TRAVEL_Absolute);
}

void AFPSGPlayerController::onClickQuitToMainMenu()
{
	//GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, "AFPSGPlayerController::onClickQuitToMainMenu");

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (!sessions.IsValid()) return;

	UWorld* world = GetWorld();
	AFPSGGameState* gameState = world != NULL ? world->GetGameState<AFPSGGameState>() : NULL;

	if (gameState != NULL)
	{
		//Retrieve the session state and match state
		FName matchState = gameState->GetMatchState();
		EOnlineSessionState::Type sessionState = EOnlineSessionState::Type::NoSession;

		if (Role == ROLE_Authority && GetNetMode() < NM_Client)
		{
			sessionState = sessions->GetSessionState(GameSessionName);
		}
		else
		{
			sessionState = PlayerState != NULL ? sessions->GetSessionState(PlayerState->SessionName) : EOnlineSessionState::Type::NoSession;
		}

		//Only attempt to end the game if both the session and match are in progress
		if (sessionState == EOnlineSessionState::Type::InProgress && matchState == MatchState::InProgress)
		{
			//Check if it is a server or client that wants to quit to the main menu
			if (Role == ROLE_Authority && GetNetMode() < NM_Client)
			{
				gameState->onClickQuitToMainMenu();
			}
			else
			{
				//Client will end and destroy its session locally, and then return to the main menu
				clientEndSession();
				clientOnlineMatchHasEnded(EEndMatchReason::NONE);
				clientDestroySession();
				ClientReturnToMainMenu_Implementation("");
			}
		}
	}
}

void AFPSGPlayerController::onClickEndGame() const
{
	//Make sure it is the server calling the function
	if (Role < ROLE_Authority && GetNetMode() == NM_Client) return;

	//GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, "AFPSGPlayerController::onClickEndGame");

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (sessions.IsValid())
	{
		UWorld* world = GetWorld();
		AFPSGGameState* gameState = world != NULL ? world->GetGameState<AFPSGGameState>() : NULL;

		if (gameState != NULL)
		{
			//Retrieve the session state and match state
			EOnlineSessionState::Type sessionState = sessions->GetSessionState(GameSessionName);
			FName matchState = gameState->GetMatchState();

			//Only attempt to end the game if both the session and match are in progress
			if (sessionState == EOnlineSessionState::Type::InProgress && matchState == MatchState::InProgress)
			{
				gameState->onClickEndGame();
			}
		}
	}
}

void AFPSGPlayerController::onRep_DieOwnerOnly()
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Magenta, "AFPSGPlayerController::onRep_DieOwnerOnly");

	if (dieCallback)
	{
		spawnCallback = false;

		timeOfDeath = FDateTime::Now();
		isAlive = false;

		if (IsLocalPlayerController())
		{
			//Removes the HUD from the screen
			FPSGUtility::destroyUserWidget(&HUDAlive);

			//Create the HUD drawn on player screen when alive
			if (HUDDead == NULL)
			{
				HUDDead = FPSGUtility::createUserWidget(this, HUDDeadTemplate, true);
			}
		}
	}
}

void AFPSGPlayerController::onRep_SpawnOwnerOnly()
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Magenta, "AFPSGPlayerController::onRep_SpawnOwnerOnly");

	if (spawnCallback)
	{
		dieCallback = false;

		timeOfDeath = FDateTime::MinValue();
		isAlive = true;

		if (IsLocalPlayerController())
		{
			FPSGUtility::destroyUserWidget(&HUDDead);

			//Create the HUD drawn on player screen when alive
			if (HUDAlive == NULL)
			{
				HUDAlive = FPSGUtility::createUserWidget(this, HUDAliveTemplate, true);
			}
		}
	}
}

float AFPSGPlayerController::timeUntilRespawn()
{
	float timeToRespawn = 0.0f;
	
	if (!isAlive)
	{
		if (timeOfDeath != FDateTime::MinValue())
		{
			FTimespan timeSinceDeath = FDateTime::Now() - timeOfDeath;
			timeToRespawn = respawnTime - static_cast<float>(timeSinceDeath.GetTotalSeconds());
		}
	}
	
	return timeToRespawn;
}

void AFPSGPlayerController::clientStartSession_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("AFPSGPlayerController::clientStartSession_Implementation"));
	
	AFPSGPlayerState* myPlayerState = Cast<AFPSGPlayerState>(PlayerState);

	//Clear timer in case it was used (Usually nothing to clear)
	GetWorldTimerManager().ClearTimer(invokeStartSessionTimerHandle);

	if (myPlayerState != NULL)
	{
		//Start the session
		startSession(myPlayerState->SessionName);
	}
	else
	{
		//In case the player state has not been replicated yet, keep executing this function
		GetWorldTimerManager().SetTimer(invokeStartSessionTimerHandle, this, &AFPSGPlayerController::clientStartSession, 0.2f, false);
	}
}

bool AFPSGPlayerController::clientStartSession_Validate()
{
	return true;
}

void AFPSGPlayerController::clientEndSession_Implementation() const
{
	GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("AFPSGPlayerController::clientEndSession_Implementation"));

	AFPSGPlayerState* myPlayerState = Cast<AFPSGPlayerState>(PlayerState);

	if (myPlayerState != NULL)
	{
		//End the session
		endSession(myPlayerState->SessionName);
	}
}

bool AFPSGPlayerController::clientEndSession_Validate() const
{
	return true;
}

void AFPSGPlayerController::clientDestroySession_Implementation() const
{
	GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("AFPSGPlayerController::clientDestroySession_Implementation"));

	AFPSGPlayerState* myPlayerState = Cast<AFPSGPlayerState>(PlayerState);

	if (myPlayerState != NULL)
	{
		//Destroy the session
		destroySession(myPlayerState->SessionName);
	}
}

bool AFPSGPlayerController::clientDestroySession_Validate() const
{
	return true;
}

void AFPSGPlayerController::clientOnlineMatchHasStarted_Implementation()
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("AFPSGPlayerController::clientOnlineMatchHasStarted_Implementation"));
}

bool AFPSGPlayerController::clientOnlineMatchHasStarted_Validate()
{
	return true;
}

void AFPSGPlayerController::clientOnlineMatchHasEnded_Implementation(EEndMatchReason endMatchReason)
{
	//GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("AFPSGPlayerController::clientOnlineMatchHasEnded_Implementation"));

	//Destroy and disable the in game menu
	FPSGUtility::destroyUserWidget(&HUDInGameMenu);
	inGameMenuIsDisabled = true;

	//Client or server locally marks itself as finished and notifies the server
	postMatchEndFinished = true;

	if (endMatchReason != EEndMatchReason::NONE)
	{
		//Clients will notify the server that they have finished all work before the match can be finished (The server also notifies itself)
		serverPostOnlineMatchHasEnded(endMatchReason);
	}
}

bool AFPSGPlayerController::clientOnlineMatchHasEnded_Validate(EEndMatchReason endMatchReason)
{
	return true;
}

bool AFPSGPlayerController::getIsAlive() const
{
	return isAlive;
}

FDateTime AFPSGPlayerController::getTimeOfDeath() const
{
	return timeOfDeath;
}

float AFPSGPlayerController::getRespawnTime() const
{
	return respawnTime;
}

bool AFPSGPlayerController::isInGameMenuOpen() const
{
	return inGameMenuIsOpen;
}
bool AFPSGPlayerController::getPostLoginFinished() const
{
	return postLoginFinished;
}

void AFPSGPlayerController::setDieCallback(bool inDieCallback)
{
	dieCallback = inDieCallback;
}

void AFPSGPlayerController::setSpawnCallback(bool inSpawnCallback)
{
	spawnCallback = inSpawnCallback;
}

void AFPSGPlayerController::setPostLoginFinished(bool inPostLoginFinished)
{
	postLoginFinished = inPostLoginFinished;
}

AFPSGGameSession* AFPSGPlayerController::getGameSession() const
{
	AFPSGGameSession* myGameSession = NULL;
	
	UWorld* world = GetWorld();
	if (world != NULL)
	{
		AGameMode* gameMode = world->GetAuthGameMode();
		if (gameMode != NULL)
		{
			myGameSession = Cast<AFPSGGameSession>(gameMode->GameSession);
		}
	}
	return myGameSession;
}

void AFPSGPlayerController::openLeaderboard()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, "AFPSGPlayerController::openLeaderboard");

	//Close the in game leaderboard if its already open
	if (inGameLeaderboardIsOpen)
	{
		inGameLeaderboardIsOpen = false;

		//Remove the leaderboard HUD
		FPSGUtility::destroyUserWidget(&HUDInGameLeaderboard);
	}
	else
	{
		inGameLeaderboardIsOpen = true;

		//Create the leaderboard HUD
		if (HUDInGameLeaderboard == NULL)
		{
			HUDInGameLeaderboard = FPSGUtility::createUserWidget(this, HUDInGameLeaderboardTemplate, true);
		}
	}
}

void AFPSGPlayerController::openInGameMenu()
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, "AFPSGPlayerController::openPauseMenu");

	//Dont open/close the in game menu if it is disabled
	if (inGameMenuIsDisabled) return;

	//If in game menu is open, close it
	if (inGameMenuIsOpen)
	{
		inGameMenuIsOpen = false;
		bShowMouseCursor = false;

		//Set input to only accept game input (keyboard, mouse). Also turn on movement and look input
		FInputModeGameOnly inputMode;
		SetInputMode(inputMode);
		SetIgnoreLookInput(false);
		SetIgnoreMoveInput(false);

		//Remove the in game menu HUD from the screen
		FPSGUtility::destroyUserWidget(&HUDInGameMenu);

		//Set all other widgets to visible if they should be on screen
		if (HUDAlive != NULL && HUDDead != NULL && HUDInGameLeaderboard != NULL)
		{
			HUDAlive->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			HUDDead->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
			HUDInGameLeaderboard->SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		}
	}
	else
	{
		inGameMenuIsOpen = true;
		bShowMouseCursor = true;

		//Set input to accept UI input aswell as game input. Also turn of movement and look input
		FInputModeGameAndUI inputMode;
		inputMode.SetLockMouseToViewport(true);
		SetInputMode(inputMode);
		SetIgnoreLookInput(true);
		SetIgnoreMoveInput(true);
		
		//Set all other widgets to hidden if they are on the screen
		if (HUDAlive != NULL && HUDDead != NULL && HUDInGameLeaderboard != NULL)
		{
			HUDAlive->SetVisibility(ESlateVisibility::Hidden);
			HUDDead->SetVisibility(ESlateVisibility::Hidden);
			HUDInGameLeaderboard->SetVisibility(ESlateVisibility::Hidden);
		}
		
		//Create the in game menu HUD
		if (HUDInGameMenu == NULL)
		{
			HUDInGameMenu = FPSGUtility::createUserWidget(this, HUDInGameMenuTemplate, true);
		}
	}
}

void AFPSGPlayerController::clientSetDeathCamera_Implementation(const FVector& cameraLocation, const FRotator& cameraRotation)
{
	SetInitialLocationAndRotation(cameraLocation, cameraRotation);
	SetViewTarget(this);
}

bool AFPSGPlayerController::clientSetDeathCamera_Validate(const FVector& cameraLocation, const FRotator& cameraRotation)
{
	return true;
}

void AFPSGPlayerController::serverPostOnlineMatchHasEnded_Implementation(EEndMatchReason endMatchReason)
{
	//TODO This function may not work if any clients disconnects or connects during game ending?
	//TODO NOT PRIORITY - Make it impossible to join matches that are in the process of ending
	//TODO NOT PRIORITY - Call this function repedeatly every second from a timer in case any clients disconnects while match is ending

	//GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("AFPSGPlayerController::serverPostOnlineMatchHasEnded_Implementation"));

	//Server marks itself or the calling client as finished
	postMatchEndFinished = true;

	UWorld* world = GetWorld();

	if (world != NULL)
	{
		for (FConstPlayerControllerIterator iter = world->GetPlayerControllerIterator(); iter; ++iter)
		{
			AFPSGPlayerController* player = Cast<AFPSGPlayerController>(*iter);

			//If any clients have not yet finished, we exit
			if (player != NULL && !player->postMatchEndFinished) return;
		}

		//GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("All Clients have finished"));


		//All clients have finished, perform the final work of ending the match
		AFPSGGameMode* gameMode = world->GetAuthGameMode<AFPSGGameMode>();
		if (gameMode != NULL)
		{
			gameMode->terminateMatch(endMatchReason);
		}
	}
}

bool AFPSGPlayerController::serverPostOnlineMatchHasEnded_Validate(EEndMatchReason endMatchReason)
{
	return true;
}
