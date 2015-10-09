// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGMainMenuGameMode.h"
#include "FPSGMainMenuPlayerController.h"
#include "FPSGGameSession.h"

AFPSGMainMenuGameMode::AFPSGMainMenuGameMode()
	: Super()
{
	PlayerControllerClass = AFPSGMainMenuPlayerController::StaticClass();
}

bool AFPSGMainMenuGameMode::ReadyToStartMatch_Implementation()
{
	//Should never start a match for this game mode
	return false;
}

TSubclassOf<AGameSession> AFPSGMainMenuGameMode::GetGameSessionClass() const
{
	return AFPSGGameSession::StaticClass();
}
