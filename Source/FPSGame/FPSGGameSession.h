// FPSGame created by Gard and Gorm

#pragma once

#include "FPSGOnlineGameSettings.h"
#include "Online.h"

#include "GameFramework/GameSession.h"
#include "FPSGGameSession.generated.h"

UCLASS()
class FPSGAME_API AFPSGGameSession : public AGameSession
{
	GENERATED_BODY()

public:
	AFPSGGameSession();

	/**
	* Called when the match has started
	*/
	virtual void HandleMatchHasStarted() override;

	/**
	* Called when the match has ended
	*/
	virtual void HandleMatchHasEnded() override;

	/**
	* Used to host a new online session
	* @param userId The user ID of the player that initiated the host request
	* @param startURL The URL to use for ServerTravel
	* @param sessionName Name of session
	* @param gameType Gametype to use for this session
	* @param mapName Map to use for this session
	* @param isLanMatch true if this is a lan match
	* @param isPresence true if this is a presence session
	* @param maxNumberOfPLayers Max number of players accepted by this session
	* @return true if successful, false otherwise
	*/
	bool hostSession(TSharedPtr<FUniqueNetId> userId, const FString& startURL, FName sessionName, const FString& gameType, 
			const FString& mapName, bool isLanMatch, bool isPresence, int32 maxNumberOfPLayers);

	/**
	* Used to find online sessions
	* @param userId The user ID of the player that initiated the search request
	* @param sessionName Name of session
	* @param isLanMatch true if this is a lan match
	* @param isPresence true if this is a presence session
	* @return true if successful, false otherwise
	*/
	bool findSessions(TSharedPtr<FUniqueNetId> userId, FName sessionName, bool isLanMatch, bool isPresence);

	/**
	* Used to join a online session
	* @param userId The user ID of the player that initiated the join request
	* @param sessionName Name of session
	* @param sessionIndex The index to the session to join
	* @return true if successful, false otherwise
	*/
	bool joinSession(TSharedPtr<FUniqueNetId> userId, FName sessionName, int32 sessionIndex);

	/**
	* Used to start a online session
	* @param sessionName Name of session
	* @return true if successful, false otherwise
	*/
	bool startSession(FName sessionName);

	/**
	* Used to end a online session
	* @param sessionName Name of session
	* @return true if successful, false otherwise
	*/
	bool endSession(FName sessionName);

	/**
	* Used to end and destroy an online session
	* Primarily used to destroy a session, but guarantees it is ended before destroying
	* Fails if the session is currently in the progress of starting
	* @param sessionName Name of session
	* @return true if successful, false otherwise
	*/
	bool endAndDestroySession(FName sessionName);

	//Current host settings
	TSharedPtr<FOnlineSessionSettings> hostSettings;

	//Current search settings
	TSharedPtr<FOnlineSessionSearch> searchSettings;

	//The URL to use for any travel the host performs
	FString travelURL;

private:
	/**
	* Delegate executed when a session create request has completed
	* @param sessionName the session name this callback is for
	* @param successful true if the async operation completed succesfully, false otherwise
	*/
	void onCreateSessionComplete(FName sessionName, bool successful) const;

	/**
	* Delegate executed when a session destroy request has completed
	* @param sessionName the session name this callback is for
	* @param successful true if the async operation completed succesfully, false otherwise
	*/
	void onDestroySessionComplete(FName sessionName, bool successful);

	/**
	* Delegate executed when a session start request has completed
	* @param sessionName the session name this callback is for
	* @param successful true if the async operation completed succesfully, false otherwise
	*/
	void onStartSessionComplete(FName sessionName, bool successful) const;

	/**
	* Delegate executed when a session end request has completed
	* @param sessionName the session name this callback is for
	* @param successful true if the async operation completed succesfully, false otherwise
	*/
	void onEndSessionComplete(FName sessionName, bool successful);

	/**
	* Delegate executed when a find sessions query has completed
	* @param successful true if the async operation completed succesfully, false otherwise
	*/
	void onFindSessionsComplete(bool successful) const;

	/**
	* Delegate executed when a session join request has completed
	* @param sessionName the session name this callback is for
	* @param result The result of the async operation
	*/
	void onJoinSessionComplete(FName sessionName, EOnJoinSessionCompleteResult::Type result);
	
	/**
	* Delegate handles to different registered delegates
	*/
	FDelegateHandle onCreateSessionCompleteDelegateHandle;
	FDelegateHandle onDestroySessionCompleteDelegateHandle;
	FDelegateHandle onStartSessionCompleteDelegateHandle;
	FDelegateHandle onEndSessionCompleteDelegateHandle;
	FDelegateHandle onFindSessionsCompleteDelegateHandle;
	FDelegateHandle onJoinSessionCompleteDelegateHandle;
};
