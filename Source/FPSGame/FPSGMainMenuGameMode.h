// FPSGame created by Gard and Gorm

#pragma once

#include "GameFramework/GameMode.h"
#include "FPSGMainMenuGameMode.generated.h"

UCLASS()
class FPSGAME_API AFPSGMainMenuGameMode : public AGameMode
{
	GENERATED_BODY()

public:
	AFPSGMainMenuGameMode();

	virtual bool ReadyToStartMatch_Implementation() override;

	/** 
	* Returns the game session class to use
	*/
	virtual TSubclassOf<AGameSession> GetGameSessionClass() const override;
};
