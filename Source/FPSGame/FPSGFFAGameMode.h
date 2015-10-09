// FPSGame created by Gard and Gorm

#pragma once

#include "FPSGGameMode.h"
#include "FPSGFFAGameMode.generated.h"

UCLASS()
class FPSGAME_API AFPSGFFAGameMode : public AFPSGGameMode
{
	GENERATED_BODY()
	
public:
	AFPSGFFAGameMode();

	virtual void Tick(float DeltaSeconds) override;

	virtual void StartPlay() override;

	/**
	* Initialize GameState data
	*/
	virtual void InitGameState() override;

	/**
	* Called when a players score is updated
	*/
	virtual void onScoreUpdate() override;

	/**
	* Finds the highest individual player score currently in the match
	* @return Returns the highest player score, or TNumericLimits<int32>::Min() if it failed to find a score
	*/
	virtual int32 findHighestScore() const override;

protected:

};
