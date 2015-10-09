// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGGameInstance.h"
#include "FPSGMainMenuPlayerController.h"
#include "Online.h"

UFPSGGameInstance::UFPSGGameInstance()
	: Super()
{
	numRegisteredPlayersAtSessionStart = 0;
}

void UFPSGGameInstance::endGame(const FString& mapName, bool isLanMatch) const
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("UFPSGGameInstance::endGame"));

	UWorld* world = GetWorld();
	AFPSGGameSession* gameSession = getGameSession();
	if (world != NULL && gameSession != NULL)
	{
		FString startURL = FString::Printf(TEXT("/Game/FPSGameContent/Levels/%s?game=%s%s"), *mapName, TEXT("?listen"), isLanMatch ? TEXT("?bIsLanMatch") : TEXT(""));

		gameSession->travelURL = startURL;

		GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("ServerTravel URL: ") + gameSession->travelURL);

		world->ServerTravel(gameSession->travelURL);
	}
}

bool UFPSGGameInstance::hostGame(const FString& mapName, const FString& gameType, bool isLanMatch, int32 maxNumberOfPlayers)
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("UFPSGGameInstance::hostGame"));

	FString startURL = FString::Printf(TEXT("/Game/FPSGameContent/Levels/%s?game=%s%s"), *mapName, TEXT("?listen"), isLanMatch ? TEXT("?bIsLanMatch") : TEXT(""));

	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, "StartURL: " + startURL);

	ULocalPlayer* localPlayer = LocalPlayers[0];
	AFPSGGameSession* const gameSession = getGameSession();
	bool success = false;

	//Calls game session to host a game session
	if (localPlayer != NULL && gameSession != NULL)
	{
		success = gameSession->hostSession(localPlayer->GetPreferredUniqueNetId(), startURL, GameSessionName, gameType, mapName, isLanMatch, true, maxNumberOfPlayers);
	}
	return success;
}

bool UFPSGGameInstance::findGames(bool isLanMatch)
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("UFPSGGameInstance::findGames"));

	ULocalPlayer* localPlayer = LocalPlayers[0];
	AFPSGMainMenuPlayerController* localPlayerController = localPlayer != NULL ? Cast<AFPSGMainMenuPlayerController>(localPlayer->PlayerController) : NULL;
	AFPSGGameSession* const gameSession = getGameSession();
	bool success = false;

	if (localPlayer != NULL && localPlayerController != NULL && gameSession != NULL)
	{
		if (!localPlayerController->searchingForServers)
		{
			localPlayerController->searchingForServers = true;

			//Calls game session to find all available game sessions that matches this search
			success = gameSession->findSessions(localPlayer->GetPreferredUniqueNetId(), GameSessionName, isLanMatch, true);
		}
	}
	return success;
}

bool UFPSGGameInstance::joinGame(int32 serverIndex)
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("UFPSGGameInstance::joinGame"));

	ULocalPlayer* localPlayer = LocalPlayers[0];
	AFPSGGameSession* const gameSession = getGameSession();
	bool success = false;

	//Calls game session to join a session
	if (localPlayer != NULL && gameSession != NULL)
	{
		success = gameSession->joinSession(localPlayer->GetPreferredUniqueNetId(), GameSessionName, serverIndex);
	}
	return success;
}

bool UFPSGGameInstance::startGame(const FString& mapName, bool isLanMatch)
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("UFPSGGameInstance::startGame"));

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (sessions.IsValid())
	{
		//First, we want to check if there actually are any registered players to this session before continuing
		FNamedOnlineSession* onlineSession = sessions->GetNamedSession(GameSessionName);
		if (onlineSession != NULL)
		{
			if (onlineSession->RegisteredPlayers.Num() <= 0) return false;

			//TODO: NOT PRIORITY - Update game instance numRegisteredPlayersAtSessionStart dynamically
			//If some players connects/disconnects while loading game, Gamemode->ReadyToStartMatch may not work properly
			//Decrease this when a player disconnects
			//Make it impossible to join matches that are in the process of starting
			//Override GameSession->RegisterPlayer and UnregisterPlayer and increase/decrease it when those functions are called
			numRegisteredPlayersAtSessionStart = onlineSession->RegisteredPlayers.Num();

			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("UFPSGGameInstance::startGame - RegisteredPlayers: " + FString::FromInt(numRegisteredPlayersAtSessionStart)));
		}

		//Also, only continue if the session has been created or ended
		EOnlineSessionState::Type sessionState = sessions->GetSessionState(GameSessionName);
		if (sessionState != EOnlineSessionState::Type::Pending && sessionState != EOnlineSessionState::Type::Ended) return false;
	}

	//Second, notify all players that the match is about to start, then let the server start it
	UWorld* world = GetWorld();
	if (world != NULL)
	{
		//Now, notify all connected clients that we are about to start a match
		for (FConstPlayerControllerIterator iter = world->GetPlayerControllerIterator(); iter; ++iter)
		{
			AFPSGMainMenuPlayerController* clientPlayer = Cast<AFPSGMainMenuPlayerController>(*iter);
			if (clientPlayer != NULL)
			{
				clientPlayer->clientPreStartOnlineMatch(); //Should send acknowledgement back to server when completed before server start match? (Can mark clients as completed)
			}
		}

		FString startURL = FString::Printf(TEXT("/Game/FPSGameContent/Levels/%s?game=%s%s"), *mapName, TEXT("?listen"), isLanMatch ? TEXT("?bIsLanMatch") : TEXT(""));

		//Let the server start and travel to the selected map (Clients are brought with it)
		AFPSGGameSession* gameSession = getGameSession();
		if (gameSession != NULL)
		{
			gameSession->travelURL = startURL;

			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("ServerTravel URL: ") + gameSession->travelURL);

			world->ServerTravel(gameSession->travelURL);
		}
	}
	return true;
}

/*
void UFPSGGameInstance::setMatchSettings(const FString& mapName, const FString& gameType, bool isLanMatch, int32 maxNumberOfPlayers, bool isPresence)
{

}

void UFPSGGameInstance::setDefaultMatchSettings()
{

}
*/

AFPSGGameSession* UFPSGGameInstance::getGameSession() const
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
