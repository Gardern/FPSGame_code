// Fill out your copyright notice in the Description page of Project Settings.

#include "FPSGame.h"
#include "FPSGGameMode.h"
#include "FPSGCharacter.h"
#include "FPSGHUD.h"
#include "FPSGPlayerStart.h"
#include "FPSGPlayerState.h"
#include "FPSGGameState.h"
#include "FPSGGameSession.h"
#include "FPSGGameInstance.h"

#include "Engine.h"

AFPSGGameMode::AFPSGGameMode()
	: Super()
{
	//Set the gamemode to use our character blueprint
	static ConstructorHelpers::FClassFinder<APawn> playerPawn(TEXT("Pawn'/Game/FPSGameContent/Blueprints/BP_FPSGCharacter.BP_FPSGCharacter_C'"));
	if (playerPawn.Class != NULL)
	{
		DefaultPawnClass = playerPawn.Class;
	}

	// Set the game mode to use our player controller blueprint
	static ConstructorHelpers::FClassFinder<APlayerController> playerController(TEXT("Blueprint'/Game/FPSGameContent/Blueprints/BP_FPSGPlayerController.BP_FPSGPlayerController_C'"));
	if (playerController.Class != NULL)
	{
		PlayerControllerClass = playerController.Class;
	}

	//Set the player state to use
	PlayerStateClass = AFPSGPlayerState::StaticClass();

	//Set the game state to use
	GameStateClass = AFPSGGameState::StaticClass();

	//Set the gamemode to use our HUD c++ class
	HUDClass = AFPSGHUD::StaticClass();
	
	MinRespawnDelay = 5.0f;

	//Default gametype
	currentGametype = EGametype::FFA;

	//Define the start weapon(s) for this gamemode
	static ConstructorHelpers::FClassFinder<AFPSGWeapon> weaponRifle(TEXT("Blueprint'/Game/FPSGameContent/Blueprints/BP_FPSGRifle.BP_FPSGRifle_C'"));
	if (weaponRifle.Class != NULL)
	{
		startWeapons.Add(weaponRifle.Class);
	}
	
	//TODO Set rules to some good default values
	//Default rules
	scoreToWin = 5; //10
	killScore = 1;
	totalMatchTime = 60;
	useRadar = true;
}

void AFPSGGameMode::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("NumPlayers: ") + FString::FromInt(NumPlayers));
	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("NumTravellingPlayers: ") + FString::FromInt(NumTravellingPlayers));
}

void AFPSGGameMode::StartPlay()
{
	Super::StartPlay();

	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("AFPSGGameMode::StartPlay"));
}

void AFPSGGameMode::HandleMatchHasStarted()
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("AFPSGGameMode::HandleMatchHasStarted"));

	Super::HandleMatchHasStarted();
}

void AFPSGGameMode::HandleMatchHasEnded()
{
	GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("AFPSGGameMode::HandleMatchHasEnded"));

	Super::HandleMatchHasEnded();

	UWorld* world = GetWorld();
	AFPSGGameState* gameState = GetGameState<AFPSGGameState>();
	if (world != NULL && gameState != NULL)
	{
		//Stuff only server should do should be executed here

		//TODO: Can overwrite GameHasEnded and loop through all players on server and call this here

		//Server notifies itself and all remote clients that the online match has ended
		for (FConstPlayerControllerIterator iter = world->GetPlayerControllerIterator(); iter; ++iter)
		{
			AFPSGPlayerController* clientPlayer = Cast<AFPSGPlayerController>(*iter);
			if (clientPlayer != NULL)
			{
				clientPlayer->clientOnlineMatchHasEnded(gameState->getEndMatchReason());
			}
		}
	}
}

bool AFPSGGameMode::ReadyToStartMatch_Implementation()
{
	bool readyToStart = Super::ReadyToStartMatch_Implementation();

//If we are playing the game from the editor, just use default ReadyToStartMatch logic
#if isPlayingFromEditor == 1
	return readyToStart;
#endif

	if (readyToStart)
	{
		//Retrieve and validate the online subsystem we are using
		IOnlineSubsystem* const onlineSubsystem = IOnlineSubsystem::Get();
		if (onlineSubsystem == NULL) return false;

		IOnlineSessionPtr sessions = onlineSubsystem->GetSessionInterface();
		if (!sessions.IsValid()) return false;

		//Retrieve and validate the online session and our game instance
		FNamedOnlineSession* onlineSession = sessions->GetNamedSession(GameSessionName);
		UFPSGGameInstance* myGameInstance = Cast<UFPSGGameInstance>(GetGameInstance());
		if (onlineSession == NULL || myGameInstance == NULL) return false;

		int32 numRegisteredPlayers = onlineSession->RegisteredPlayers.Num();

		//GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Cyan, TEXT("numRegisteredPlayers Session: ") + FString::FromInt(numRegisteredPlayers));
		//GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Cyan, TEXT("numRegisteredPlayers GameInstance: ") + FString::FromInt(myGameInstance->numRegisteredPlayersAtSessionStart));

		//First check: Pass if the number of registered players set in game instance equals to number of registered players in the session
		if (myGameInstance->numRegisteredPlayersAtSessionStart > 0 && myGameInstance->numRegisteredPlayersAtSessionStart == numRegisteredPlayers)
		{
			UWorld* world = GetWorld();
			if (world == NULL) return false;

			//Second check: Loop through all player controllers in the world list.
			//In case any is NULL or player is not yet logged in (Called PostLogin), dont continue
			//Pass if the number of player controllers in the world list equals to registered players set in game instance
			int32 numPlayersInWorld = 0;
			for (FConstPlayerControllerIterator iter = world->GetPlayerControllerIterator(); iter; ++iter)
			{
				AFPSGPlayerController* player = Cast<AFPSGPlayerController>(*iter);

				if (player == NULL) return false;
				if (!player->getPostLoginFinished()) return false;
				numPlayersInWorld++;
			}

			//GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Cyan, TEXT("numPlayersInWorld: ") + FString::FromInt(numPlayersInWorld));

			if (numPlayersInWorld == myGameInstance->numRegisteredPlayersAtSessionStart)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Cyan, TEXT("NumPlayers Gamemode: ") + FString::FromInt(NumPlayers));
				//GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Cyan, TEXT("NumTravellingPlayers Gamemode: ") + FString::FromInt(NumTravellingPlayers));

				//Last check: Pass if the number of registered players set in game instance equals to the current number of players in gamemode
				//and if the number of travelling players are 0
				if (myGameInstance->numRegisteredPlayersAtSessionStart == NumPlayers && NumTravellingPlayers == 0)
				{
					//GEngine->AddOnScreenDebugMessage(-1, 30.0f, FColor::Cyan, TEXT("Succeed, Start Match"));

					//If all checks succeeded, we can start the match
					return true;
				}
			}
		}
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Magenta, TEXT("AFPSGGameMode::ReadyToStartMatch - numRegisteredPlayers: ") + FString::FromInt(numRegisteredPlayers));
	}
	return false;
}

bool AFPSGGameMode::ReadyToEndMatch_Implementation()
{
	bool readyToEnd = Super::ReadyToEndMatch_Implementation();

//If we are playing the game from the editor, just use default ReadyToEndMatch logic
#if isPlayingFromEditor == 1
	return readyToEnd;
#endif
	
	UWorld* world = GetWorld();
	AFPSGGameState* gameState = world != NULL ? world->GetGameState<AFPSGGameState>() : NULL;

	if (gameState != NULL)
	{
		//If the match should be ended because someone has won
		if (gameState->getCurrentHighestMatchScore() >= scoreToWin)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("Game was ended because EEndMatchReason::NORMAL_END_MATCH"));

			gameState->setEndMatchReason(EEndMatchReason::NORMAL_END_MATCH);
			return true;
		}

		//If the match should be ended because of time limit (remaining time <= 0, return true)

		//If the match should be ended because the host clicked the End Game button
		if (gameState->getHostWantsToEndGame())
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("Game was ended because EEndMatchReason::HOST_END_MATCH"));

			gameState->setEndMatchReason(EEndMatchReason::HOST_END_MATCH);
			return true;
		}

		//If the match should be ended because the host clicked the Quit to Main Menu button and disconnected
		if (gameState->getHostWantsToQuitGame())
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Cyan, TEXT("Game was ended because EEndMatchReason::HOST_QUIT_MATCH"));

			gameState->setEndMatchReason(EEndMatchReason::HOST_QUIT_MATCH);
			return true;
		}
	}

	return false;
}

void AFPSGGameMode::PreLogin(const FString& Options, const FString& Address, const TSharedPtr<class FUniqueNetId>& UniqueId, FString& ErrorMessage)
{
	Super::PreLogin(Options, Address, UniqueId, ErrorMessage);

	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Magenta, TEXT("AFPSGGameMode::PreLogin"));
}

APlayerController* AFPSGGameMode::Login(class UPlayer* NewPlayer, ENetRole RemoteRole, const FString& Portal, const FString& Options, const TSharedPtr<class FUniqueNetId>& UniqueId, FString& ErrorMessage)
{
	APlayerController* myPlayerController = Super::Login(NewPlayer, RemoteRole, Portal, Options, UniqueId, ErrorMessage);

	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Magenta, TEXT("AFPSGGameMode::Login"));

	return myPlayerController;
}

void AFPSGGameMode::PostLogin(APlayerController* NewPlayer)
{
	Super::PostLogin(NewPlayer);

	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Magenta, TEXT("AFPSGGameMode::PostLogin"));
	
	AFPSGPlayerController* newPlayerController = Cast<AFPSGPlayerController>(NewPlayer);

	if (newPlayerController != NULL)
	{
		//Clients starts their session (Calls StartSession) at post login if the match is already in progress
		if (!newPlayerController->IsLocalPlayerController() && IsMatchInProgress())
		{
			//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Magenta, TEXT("AFPSGGameMode::PostLogin clientStartSession"));

			newPlayerController->clientStartSession();
		}
		
		//The player have successfully logged in and is ready to play
		newPlayerController->setPostLoginFinished(true);
	}
}

AActor* AFPSGGameMode::ChoosePlayerStart_Implementation(AController* Player)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Magenta, TEXT("AFPSGGameMode::ChoosePlayerStart"));

	TArray<AFPSGPlayerStart*> validPlayerstarts;

	// Loop through all playerstarts and find all valid playerstarts
	for (TActorIterator<APlayerStart> playerStartIter(GetWorld()); playerStartIter; ++playerStartIter) //TODO Test TActorIterator
	{
		AFPSGPlayerStart* playerstart = Cast<AFPSGPlayerStart>(*playerStartIter);

		if (playerstart != NULL)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Magenta, TEXT("playerstart != NULL"));

			// Check if playerstart is valid
			if (isPlayerstartValid(playerstart))
			{
				//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Magenta, TEXT("Found valid playerstart with ID=" + FString::FromInt(playerstart->GetUniqueID())));

				validPlayerstarts.Add(playerstart);
			}
		}
	}

	/*
	// Loop through all playerstarts and find all valid playerstarts
	for (int32 i = 0; i < PlayerStarts.Num(); ++i)
	{
		AFPSGPlayerStart* playerstart = Cast<AFPSGPlayerStart>(PlayerStarts[i]);

		if (playerstart != NULL)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Magenta, TEXT("playerstart != NULL"));

			// Check if playerstart is valid
			if (isPlayerstartValid(playerstart))
			{
				//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Magenta, TEXT("Found valid playerstart with ID=" + FString::FromInt(playerstart->GetUniqueID())));

				validPlayerstarts.Add(playerstart);
			}
		}
	}
	*/

	// Spawn randomly at one of the valid playerstarts
	if (validPlayerstarts.Num() > 0)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Magenta, TEXT("Found valid Playerstarts"));

		int32 rand = FMath::RandHelper(validPlayerstarts.Num());

		if (GEngine != NULL)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, TEXT("Spawn at playerstart with ID=" + FString::FromInt(validPlayerstarts[rand]->GetUniqueID())));
		}

		return validPlayerstarts[rand];
	}

	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Green, TEXT("Spawn at Super::ChoosePlayerStart"));
	}

	// If no playerstart was valid, call super to choose playerstart instead
	return Super::ChoosePlayerStart_Implementation(Player);
}

bool AFPSGGameMode::ShouldSpawnAtStartSpot(AController* Player)
{
	return false;
}

TSubclassOf<AGameSession> AFPSGGameMode::GetGameSessionClass() const
{
	return AFPSGGameSession::StaticClass();
}

void AFPSGGameMode::notifyKill(AFPSGPlayerController* killer, AFPSGPlayerController* killedPlayer)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("AFPSGGameMode::notifyKill"));

	AFPSGPlayerState *killerPlayerState = killer ? Cast<AFPSGPlayerState>(killer->PlayerState) : NULL;
	AFPSGPlayerState *victimPlayerState = killedPlayer ? Cast<AFPSGPlayerState>(killedPlayer->PlayerState) : NULL;
	
	//Update kill score for the player that killed
	if (killerPlayerState != NULL && killerPlayerState != victimPlayerState)
	{
		killerPlayerState->updateKills(1);
	}

	//Update death score for the player that got killed
	if (victimPlayerState != NULL)
	{
		victimPlayerState->updateDeaths(1);
	}

	//Update score for the player that killed
	if (killerPlayerState != NULL && killerPlayerState != victimPlayerState)
	{
		killerPlayerState->updateScore(killScore);
	}
}

void AFPSGGameMode::onScoreUpdate()
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, "AFPSGGameMode::onScoreUpdate");

	//Check if the match should be ended
	if (ReadyToEndMatch())
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, "Match can be ended");
		
		EndMatch();
	}
}

int32 AFPSGGameMode::findHighestScore() const
{
	return 0;
}

void AFPSGGameMode::terminateMatch(EEndMatchReason endMatchReason) const
{
	//GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("AFPSGGameMode::terminateMatch"));

	AFPSGGameSession* gameSession = Cast<AFPSGGameSession>(GameSession);
	UFPSGGameInstance* gameInstance = Cast<UFPSGGameInstance>(GetGameInstance());

	//Decide how the game should be ended based on the end reason
	if (gameSession != NULL && gameInstance != NULL)
	{
		if (endMatchReason == EEndMatchReason::NORMAL_END_MATCH)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("EEndMatchReason::NORMAL_END_MATCH"));

			//Bring all players back to the lobby
			gameInstance->endGame(LOBBY_MAP_NAME.ToString(), true); //TODO hardcoded Lan match, change later
		}
		else if (endMatchReason == EEndMatchReason::HOST_END_MATCH)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("EEndMatchReason::HOST_END_MATCH"));

			//Bring all players back to the lobby
			gameInstance->endGame(LOBBY_MAP_NAME.ToString(), true); //TODO hardcoded Lan match, change later
		}
		else if (endMatchReason == EEndMatchReason::HOST_QUIT_MATCH)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("EEndMatchReason::HOST_QUIT_MATCH"));

			//Destroy the session and return each player to the main menu
			gameSession->endAndDestroySession(GameSessionName);
		}
	}
}

EGametype AFPSGGameMode::getCurrentGametype() const
{
	return currentGametype;
}

const TArray<TSubclassOf<AFPSGWeapon>>& AFPSGGameMode::getStartWeapons() const
{
	return startWeapons;
}

void AFPSGGameMode::setCurrentGametype(EGametype inCurrentGametype)
{
	currentGametype = inCurrentGametype;
	
	AFPSGGameState* myGameState = GetGameState<AFPSGGameState>();

	if (myGameState != NULL)
	{
		//Sets the gametype variable in the gamestate
		myGameState->setCurrentGametype(inCurrentGametype);
	}
}

bool AFPSGGameMode::isPlayerstartValid(AFPSGPlayerStart* playerStart) const
{
	const FVector playerStartLoc = playerStart->GetActorLocation();

	// Iterate through all players in the level
	for (FConstPawnIterator iterator = GetWorld()->GetPawnIterator(); iterator; ++iterator)
	{
		ACharacter* otherPlayer = Cast<ACharacter>(*iterator);

		if (otherPlayer != NULL)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Magenta, TEXT("otherPlayer != NULL"));

			const FVector otherPlayerLoc = otherPlayer->GetActorLocation();

			// Check if a player is inside the valid radius of a playerstart
			if ((FMath::Abs(playerStartLoc.X - otherPlayerLoc.X) < playerStart->getValidSpawnRadius())
				&& (FMath::Abs(playerStartLoc.Y - otherPlayerLoc.Y) < playerStart->getValidSpawnRadius()))
			{
				//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Magenta, TEXT("isPlayerstartValid: return false"));

				return false;
			}
		}
	}

	return true;
}
