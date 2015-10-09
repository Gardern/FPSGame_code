// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGMainMenuPlayerController.h"
#include "FPSGUtility.h"
#include "FPSGGameInstance.h"
#include "FPSGGameState.h"

AFPSGMainMenuPlayerController::AFPSGMainMenuPlayerController()
	: Super()
{
	HUDJoinGameMenu = NULL;
	HUDLobbyMenu = NULL;
	searchingForServers = false;
	currentServerIndex = -1;
	finishedSearchingForGames = false;
}

void AFPSGMainMenuPlayerController::BeginPlay()
{
	Super::BeginPlay();
}

void AFPSGMainMenuPlayerController::Destroyed()
{
	Super::Destroyed();

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "AFPSGMainMenuPlayerController::Destroyed()");

	//Make sure all widgets that belongs to this player controller are destroyed
	FPSGUtility::destroyUserWidget(&HUDJoinGameMenu);
	FPSGUtility::destroyUserWidget(&HUDLobbyMenu);
}

void AFPSGMainMenuPlayerController::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
}

void AFPSGMainMenuPlayerController::onHostGameComplete()
{

}

void AFPSGMainMenuPlayerController::onHostGameCompleteSuccess()
{
	UWorld* world = GetWorld();
	AFPSGGameSession* gameSession = getGameSession();

	if (world != NULL && gameSession != NULL)
	{
		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("onCreateSessionComplete: ServerTravel"));
		UE_LOG(LogTemp, Log, TEXT("onCreateSessionComplete: ServerTravel"));

		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("ServerTravel URL: ") + gameSession->travelURL);

		world->ServerTravel(gameSession->travelURL);
	}
}

void AFPSGMainMenuPlayerController::onSearchForGamesComplete()
{
	searchingForServers = false;
}

void AFPSGMainMenuPlayerController::onSearchForGamesCompleteSuccess()
{
	AFPSGGameSession* gameSession = getGameSession();

	if (gameSession != NULL && gameSession->searchSettings.IsValid())
	{
		if (gameSession->searchSettings->SearchState == EOnlineAsyncTaskState::Type::Done)
		{
			//Remove all existing game list elements
			gameListData.RemoveAt(0, gameListData.Num(), false);

			//Retrive settings information for all the search results
			for (const FOnlineSessionSearchResult& searchResult : gameSession->searchSettings->SearchResults)
			{
				int32 maxPlayers;
				FString gameType, mapName;
				searchResult.Session.SessionSettings.Get(SETTING_MAXPLAYERS, maxPlayers);
				searchResult.Session.SessionSettings.Get(SETTING_GAMEMODE, gameType);
				searchResult.Session.SessionSettings.Get(SETTING_MAPNAME, mapName);

				int32 pingInMs = searchResult.PingInMs;
				int32 currentPlayers = maxPlayers - searchResult.Session.NumOpenPublicConnections;
				bool isLANMatch = searchResult.Session.SessionSettings.bIsLANMatch;

				//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, "pingInMs: " + FString::FromInt(pingInMs));
				//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, "isLANMatch: " + FString::FromInt(isLANMatch));
				//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, "maxPlayers: " + FString::FromInt(maxPlayers));
				//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, "gameType: " + gameType);
				//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, "mapName: " + mapName);

				//Create a single game list element and add it to the game list data
				FGameListElement gameListElement;
				gameListElement.gameType = gameType;
				gameListElement.mapName = mapName;
				gameListElement.currentPlayers = currentPlayers;
				gameListElement.maxPlayers = maxPlayers;
				gameListElement.ping = pingInMs;
				gameListElement.matchType = isLANMatch ? "LAN" : "Online";

				gameListData.Add(gameListElement);
			}
			//Marks that we have finished searching.
			//NOTE: The game list can now be initialized based on the game list data. This is done in the BPWid_FPSGJoinGameMenu widget (Tick function)
			finishedSearchingForGames = true;
		}
	}
}

void AFPSGMainMenuPlayerController::onJoinGameComplete()
{

}

void AFPSGMainMenuPlayerController::onJoinGameCompleteSuccess()
{

}

bool AFPSGMainMenuPlayerController::destroySession(FName sessionName) const
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
			GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("AFPSGMainMenuPlayerController::destroySession success"));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("AFPSGMainMenuPlayerController::destroySession NOT success"));
		}
	}
	return success;
}

void AFPSGMainMenuPlayerController::ClientReturnToMainMenu_Implementation(const FString& ReturnReason)
{
	GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("AFPSGMainMenuPlayerController::ClientReturnToMainMenu_Implementation"));

	FString URL = FString::Printf(TEXT("/Game/FPSGameContent/Levels/%s"), *GAMEENTRY_MAP_NAME.ToString());

	GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("ClientTravel URL: ") + URL);

	ClientTravel(URL, TRAVEL_Absolute);
}

void AFPSGMainMenuPlayerController::onClickLeave()
{
	//GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, "AFPSGMainMenuPlayerController::onClickLeave");

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (!sessions.IsValid()) return;

	//Retrieve the session state
	EOnlineSessionState::Type sessionState = EOnlineSessionState::Type::NoSession;

	if (Role == ROLE_Authority && GetNetMode() < NM_Client)
	{
		sessionState = sessions->GetSessionState(GameSessionName);
	}
	else
	{
		sessionState = PlayerState != NULL ? sessions->GetSessionState(PlayerState->SessionName) : EOnlineSessionState::Type::NoSession;
	}

	//Only attempt to leave the game if the session is pending or has been ended
	if (sessionState == EOnlineSessionState::Type::Pending || sessionState == EOnlineSessionState::Type::Ended)
	{
		//Check if it is a server or client that wants to quit to the main menu
		if (Role == ROLE_Authority && GetNetMode() < NM_Client)
		{
			//Destroy the session and return each player to the main menu
			AFPSGGameSession* gameSession = getGameSession();
			if (gameSession != NULL)
			{
				gameSession->endAndDestroySession(GameSessionName);
			}
		}
		else
		{
			//Client will destroy its session locally, and then return to the main menu
			clientDestroySession();
			ClientReturnToMainMenu_Implementation("");
		}
	}
}

void AFPSGMainMenuPlayerController::clientPreStartOnlineMatch_Implementation()
{
	FPSGUtility::destroyUserWidget(&HUDLobbyMenu);
}

bool AFPSGMainMenuPlayerController::clientPreStartOnlineMatch_Validate()
{
	return true;
}

void AFPSGMainMenuPlayerController::clientDestroySession_Implementation() const
{
	GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("AFPSGMainMenuPlayerController::clientDestroySession_Implementation"));

	if (PlayerState != NULL)
	{
		//Destroy the session
		destroySession(PlayerState->SessionName);
	}
}

bool AFPSGMainMenuPlayerController::clientDestroySession_Validate() const
{
	return true;
}

AFPSGGameSession* AFPSGMainMenuPlayerController::getGameSession() const
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
