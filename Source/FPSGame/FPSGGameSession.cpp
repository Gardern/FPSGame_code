// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGGameSession.h"
#include "FPSGMainMenuPlayerController.h"
#include "FPSGPlayerController.h"

namespace
{
	const FString customMatchKeyword("Custom");
}

AFPSGGameSession::AFPSGGameSession()
	: Super()
{
	hostSettings = NULL;
	searchSettings = NULL;
}

void AFPSGGameSession::HandleMatchHasStarted()
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("AFPSGGameSession::HandleMatchHasStarted"));

	//TODO Set SessionName here to GameSessionName before calling Super::HandleMatchHasStarted()

	//Super::HandleMatchHasStarted();

	//Start the session when match has started
	startSession(GameSessionName);
}

void AFPSGGameSession::HandleMatchHasEnded()
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("AFPSGGameSession::HandleMatchHasEnded"));

	//Super::HandleMatchHasEnded();

	//End the session when match has ended
	endSession(GameSessionName);
}

bool AFPSGGameSession::hostSession(TSharedPtr<FUniqueNetId> userId, const FString& startURL, FName sessionName, const FString& gameType, const FString& mapName, bool isLanMatch, bool isPresence, int32 maxNumberOfPLayers)
{
	bool success = false;
	travelURL = startURL;

	//Retrieves the online subsystem we are using
	IOnlineSubsystem* const onlineSubsystem = IOnlineSubsystem::Get();

	if (onlineSubsystem != NULL)
	{
		MaxPlayers = maxNumberOfPLayers;
		
		IOnlineSessionPtr sessions = onlineSubsystem->GetSessionInterface();
		if (sessions.IsValid() && userId.IsValid())
		{
			//Set up different host settings for this session
			hostSettings = MakeShareable(new FOnlineSessionSettings());
			hostSettings->NumPublicConnections = MaxPlayers;
			if (hostSettings->NumPublicConnections < 0)
			{
				hostSettings->NumPublicConnections = 0;
			}
			hostSettings->NumPrivateConnections = 0;
			hostSettings->bIsLANMatch = isLanMatch;
			hostSettings->bShouldAdvertise = true;
			hostSettings->bAllowJoinInProgress = true;
			hostSettings->bAllowInvites = true;
			hostSettings->bUsesPresence = isPresence;
			hostSettings->bAllowJoinViaPresence = true;
			hostSettings->bAllowJoinViaPresenceFriendsOnly = false;

			hostSettings->Set(SETTING_MAXPLAYERS, MaxPlayers, EOnlineDataAdvertisementType::ViaOnlineService);
			hostSettings->Set(SETTING_GAMEMODE, gameType, EOnlineDataAdvertisementType::ViaOnlineService);
			hostSettings->Set(SETTING_MAPNAME, mapName, EOnlineDataAdvertisementType::ViaOnlineService);
			hostSettings->Set(SETTING_MATCHING_HOPPER, FString("TeamDeathmatch"), EOnlineDataAdvertisementType::DontAdvertise);
			hostSettings->Set(SETTING_MATCHING_TIMEOUT, 120.0f, EOnlineDataAdvertisementType::ViaOnlineService);
			hostSettings->Set(SETTING_SESSION_TEMPLATE_NAME, FString("GameSession"), EOnlineDataAdvertisementType::DontAdvertise);
			hostSettings->Set(SEARCH_KEYWORDS, customMatchKeyword, EOnlineDataAdvertisementType::ViaOnlineService);
			
			//Start the delegate and create a new session
			onCreateSessionCompleteDelegateHandle = sessions->AddOnCreateSessionCompleteDelegate_Handle(FOnCreateSessionCompleteDelegate::CreateUObject(this, &AFPSGGameSession::onCreateSessionComplete));
			success = sessions->CreateSession(*userId, sessionName, *hostSettings);
			if (success)
			{
				GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("CreateSession success"));
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("CreateSession NOT success"));
			}
		}
	}
	return success;
}

bool AFPSGGameSession::findSessions(TSharedPtr<FUniqueNetId> userId, FName sessionName, bool isLanMatch, bool isPresence)
{
	bool success = false;

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (sessions.IsValid() && userId.IsValid())
	{
		//Set up the search settings that are used when searching for a session
		searchSettings = MakeShareable(new FOnlineSessionSearch());
		searchSettings->bIsLanQuery = isLanMatch;
		searchSettings->MaxSearchResults = 10;
		searchSettings->PingBucketSize = 50;

		if (isPresence)
		{
			searchSettings->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);
		}

		searchSettings->QuerySettings.Set(SEARCH_KEYWORDS, customMatchKeyword, EOnlineComparisonOp::Equals);

		TSharedRef<FOnlineSessionSearch> searchSettingsRef = searchSettings.ToSharedRef();

		//Start the delegate and try to find any running sessions
		onFindSessionsCompleteDelegateHandle = sessions->AddOnFindSessionsCompleteDelegate_Handle(FOnFindSessionsCompleteDelegate::CreateUObject(this, &AFPSGGameSession::onFindSessionsComplete));
		success = sessions->FindSessions(*userId, searchSettingsRef);
		if (success)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("FindSessions success"));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("FindSessions NOT success"));
		}
	}
	else
	{
		onFindSessionsComplete(false);
	}
	return success;
}

bool AFPSGGameSession::joinSession(TSharedPtr<FUniqueNetId> userId, FName sessionName, int32 sessionIndex)
{
	bool success = false;

	//Only join the session if the index to the session is valid
	if (searchSettings->SearchResults.IsValidIndex(sessionIndex))
	{
		IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
		IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

		if (sessions.IsValid() && userId.IsValid())
		{
			//Start the delegate and join the session
			onJoinSessionCompleteDelegateHandle = sessions->AddOnJoinSessionCompleteDelegate_Handle(FOnJoinSessionCompleteDelegate::CreateUObject(this, &AFPSGGameSession::onJoinSessionComplete));
			success = sessions->JoinSession(*userId, sessionName, searchSettings->SearchResults[sessionIndex]);
			if (success)
			{
				GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("JoinSession success"));
			}
			else
			{
				GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("JoinSession NOT success"));
			}
		}
	}
	return success;
}

bool AFPSGGameSession::startSession(FName sessionName)
{
	bool success = false;

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (sessions.IsValid())
	{
		//Start delegate and start the session
		onStartSessionCompleteDelegateHandle = sessions->AddOnStartSessionCompleteDelegate_Handle(FOnStartSessionCompleteDelegate::CreateUObject(this, &AFPSGGameSession::onStartSessionComplete));
		success = sessions->StartSession(sessionName);
		if (success)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("StartSession success"));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("StartSession NOT success"));
		}
	}
	return success;
}

bool AFPSGGameSession::endSession(FName sessionName)
{
	bool success = false;

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (sessions.IsValid())
	{
		UWorld* world = GetWorld();
		if (world != NULL)
		{
			//Notify all remote clients to end their session
			for (FConstPlayerControllerIterator iter = world->GetPlayerControllerIterator(); iter; ++iter)
			{
				AFPSGPlayerController* clientPlayer = Cast<AFPSGPlayerController>(*iter);
				if (clientPlayer != NULL && !clientPlayer->IsLocalPlayerController())
				{
					clientPlayer->clientEndSession();
				}
			}
		}
			
		//End the session for server
		success = sessions->EndSession(sessionName);
		if (success)
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("EndSession success"));
		}
		else
		{
			GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("EndSession NOT success"));
		}
	}
	return success;
}

bool AFPSGGameSession::endAndDestroySession(FName sessionName)
{
	GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("FPSGGameSession::endAndDestroySession"));

	bool success = false;

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (sessions.IsValid())
	{
		//Get the state this session is in
		EOnlineSessionState::Type sessionState = sessions->GetSessionState(sessionName);

		//If we are in the progress of starting, we cant end or destroy the session
		if (sessionState == EOnlineSessionState::Starting) return false;

		UWorld* world = GetWorld();
		if (world == NULL) return false;

		//If the session is currently in progress, we should end it
		if (sessionState == EOnlineSessionState::InProgress)
		{
			GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("FPSGGameSession::endAndDestroySession EndSession"));

			//Notify any clients that are connected to end the session
			for (FConstPlayerControllerIterator iter = world->GetPlayerControllerIterator(); iter; ++iter)
			{
				AFPSGPlayerController* clientPlayer = Cast<AFPSGPlayerController>(*iter);
				if (clientPlayer != NULL && !clientPlayer->IsLocalPlayerController())
				{
					clientPlayer->clientEndSession();
				}
			}

			//End the session for server
			onEndSessionCompleteDelegateHandle = sessions->AddOnEndSessionCompleteDelegate_Handle(FOnEndSessionCompleteDelegate::CreateUObject(this, &AFPSGGameSession::onEndSessionComplete));
			success = sessions->EndSession(sessionName);
		}
		else if (sessionState == EOnlineSessionState::Ending) //If the session is ending, just wait for it to end
		{
			GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("FPSGGameSession::endAndDestroySession Ending"));

			onEndSessionCompleteDelegateHandle = sessions->AddOnEndSessionCompleteDelegate_Handle(FOnEndSessionCompleteDelegate::CreateUObject(this, &AFPSGGameSession::onEndSessionComplete));
		}
		else if (sessionState == EOnlineSessionState::Ended || sessionState == EOnlineSessionState::Pending) //If the session has been ended, we can destroy it
		{
			GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("FPSGGameSession::endAndDestroySession DestroySession"));

			//Notify any clients that are connected to destroy the session
			for (FConstPlayerControllerIterator iter = world->GetPlayerControllerIterator(); iter; ++iter)
			{
				//Both main menu player controllers and normal player controllers have the ability to destroy its session
				//Therefore, check which one it is
				if ((*iter)->IsA(AFPSGPlayerController::StaticClass()))
				{
					AFPSGPlayerController* clientPlayer = Cast<AFPSGPlayerController>(*iter);
					if (clientPlayer != NULL && !clientPlayer->IsLocalPlayerController())
					{
						clientPlayer->clientDestroySession();
					}
				}
				else if ((*iter)->IsA(AFPSGMainMenuPlayerController::StaticClass()))
				{
					AFPSGMainMenuPlayerController* clientPlayer = Cast<AFPSGMainMenuPlayerController>(*iter);
					if (clientPlayer != NULL && !clientPlayer->IsLocalPlayerController())
					{
						clientPlayer->clientDestroySession();
					}
				}
			}

			//Destroy the session for server
			onDestroySessionCompleteDelegateHandle = sessions->AddOnDestroySessionCompleteDelegate_Handle(FOnDestroySessionCompleteDelegate::CreateUObject(this, &AFPSGGameSession::onDestroySessionComplete));
			success = sessions->DestroySession(sessionName);
		}
	}
	
	return success;
}

void AFPSGGameSession::onCreateSessionComplete(FName sessionName, bool successful) const
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("onCreateSessionComplete Success: ") + FString::FromInt(successful));

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (sessions.IsValid())
	{
		//Clear the delegate
		sessions->ClearOnCreateSessionCompleteDelegate_Handle(onCreateSessionCompleteDelegateHandle);

		EOnJoinSessionCompleteResult::Type result = successful ? EOnJoinSessionCompleteResult::Success : EOnJoinSessionCompleteResult::UnknownError;

		UGameInstance* gameInstance = GetGameInstance();
		if (gameInstance != NULL)
		{
			AFPSGMainMenuPlayerController* mainMenuPlayerController =
				gameInstance->GetLocalPlayers()[0] ? Cast<AFPSGMainMenuPlayerController>(gameInstance->GetLocalPlayers()[0]->PlayerController) : NULL;

			//Notify the player that the create session request has finished
			if (mainMenuPlayerController != NULL)
			{
				if (result == EOnJoinSessionCompleteResult::Success)
				{
					mainMenuPlayerController->onHostGameCompleteSuccess();
				}
				else
				{
					GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("NetworkErrors, CreateSessionFailed"));
					UE_LOG(LogTemp, Error, TEXT("NetworkErrors, CreateSessionFailed"));
				}
				mainMenuPlayerController->onHostGameComplete();
			}
		}
	}
}

void AFPSGGameSession::onDestroySessionComplete(FName sessionName, bool successful)
{
	GEngine->AddOnScreenDebugMessage(-1, 40.0f, FColor::Cyan, TEXT("onDestroySessionComplete Success: ") + FString::FromInt(successful));

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (sessions.IsValid())
	{
		//Clear the delegate
		sessions->ClearOnDestroySessionCompleteDelegate_Handle(onDestroySessionCompleteDelegateHandle);
	}

	endAndDestroySession(GameSessionName);

	if (successful)
	{
		hostSettings = NULL;
		ReturnToMainMenuHost();
	}
}

void AFPSGGameSession::onStartSessionComplete(FName sessionName, bool successful) const
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("onStartSessionComplete Success: ") + FString::FromInt(successful));

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (sessions.IsValid())
	{
		//Clear the delegate
		sessions->ClearOnStartSessionCompleteDelegate_Handle(onStartSessionCompleteDelegateHandle);
	}
	
	UWorld* world = GetWorld();
	if (successful && world != NULL)
	{
		//If any clients are connected to host at match startup, notify them that the match has started, and let them call StartSession
		for (FConstPlayerControllerIterator iter = world->GetPlayerControllerIterator(); iter; ++iter)
		{
			AFPSGPlayerController* clientPlayer = Cast<AFPSGPlayerController>(*iter);
			if (clientPlayer != NULL && !clientPlayer->IsLocalPlayerController())
			{
				clientPlayer->clientStartSession();
			}
		}
	}
}

void AFPSGGameSession::onEndSessionComplete(FName sessionName, bool successful)
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("onEndSessionComplete Success: ") + FString::FromInt(successful));

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (sessions.IsValid())
	{
		//Clear the delegate
		sessions->ClearOnEndSessionCompleteDelegate_Handle(onEndSessionCompleteDelegateHandle);
	}
	
	endAndDestroySession(GameSessionName);
}

void AFPSGGameSession::onFindSessionsComplete(bool successful) const
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("onFindSessionsComplete Success: ") + FString::FromInt(successful));

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (sessions.IsValid())
	{
		//Clear the delegate
		sessions->ClearOnFindSessionsCompleteDelegate_Handle(onFindSessionsCompleteDelegateHandle);

		UGameInstance* gameInstance = GetGameInstance();
		if (gameInstance != NULL)
		{
			AFPSGMainMenuPlayerController* mainMenuPlayerController =
				gameInstance->GetLocalPlayers()[0] ? Cast<AFPSGMainMenuPlayerController>(gameInstance->GetLocalPlayers()[0]->PlayerController) : NULL;

			//Notify the player that the find sessions query has completed
			if (mainMenuPlayerController != NULL)
			{
				if (successful)
				{
					mainMenuPlayerController->onSearchForGamesCompleteSuccess();
				}
				mainMenuPlayerController->onSearchForGamesComplete();
			}
		}
	}
}

void AFPSGGameSession::onJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result)
{
	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("onJoinSessionComplete Success: ") + FString::FromInt(static_cast<int32>(result)));

	IOnlineSubsystem* onlineSubsystem = IOnlineSubsystem::Get();
	IOnlineSessionPtr sessions = onlineSubsystem != NULL ? onlineSubsystem->GetSessionInterface() : NULL;

	if (sessions.IsValid())
	{
		//Clear the delegate
		sessions->ClearOnJoinSessionCompleteDelegate_Handle(onJoinSessionCompleteDelegateHandle);
			
		UGameInstance* gameInstance = GetGameInstance();
		if (gameInstance != NULL)
		{
			AFPSGMainMenuPlayerController* mainMenuPlayerController =
				gameInstance->GetLocalPlayers()[0] ? Cast<AFPSGMainMenuPlayerController>(gameInstance->GetLocalPlayers()[0]->PlayerController) : NULL;

			//Notify the player that the join session request has completed
			if (mainMenuPlayerController != NULL)
			{
				if (result == EOnJoinSessionCompleteResult::Success)
				{
					mainMenuPlayerController->onJoinGameCompleteSuccess();

					//We havent traveled to the session yet so this player is the only player in its world (using 0 as controller id should therefore be ok)
					TravelToSession(0, GameSessionName);
				}
				else
				{
					//TODO: EOnJoinSessionCompleteResult does not seem completed yet (JoinSession only returns Success or UnknownError). These errors are therefore useless.
					//TODO (NOT PRIORITY) - Print to UMG HUD
					FText returnReason;
					switch (result)
					{
					case EOnJoinSessionCompleteResult::SessionIsFull:
						returnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Game session is full.");
						break;
					case EOnJoinSessionCompleteResult::SessionDoesNotExist:
						returnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Game session no longer exists.");
						break;
					case EOnJoinSessionCompleteResult::CouldNotRetrieveAddress:
						returnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Could not retrieve the servers address.");
						break;
					case EOnJoinSessionCompleteResult::AlreadyInSession:
						returnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Already connected to the session.");
						break;
					default:
						returnReason = NSLOCTEXT("NetworkErrors", "JoinSessionFailed", "Join failed (Unknown error)");
						break;
					}
					GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Cyan, TEXT("NetworkErrors, JoinSessionFailed: ") + returnReason.ToString());
				}
				mainMenuPlayerController->onJoinGameComplete();
			}
		}
	}
}
