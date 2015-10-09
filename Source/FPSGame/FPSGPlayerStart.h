// FPSGame created by Gard and Gorm

#pragma once

#include "GameFramework/PlayerStart.h"
#include "FPSGPlayerStart.generated.h"

UCLASS()
class FPSGAME_API AFPSGPlayerStart : public APlayerStart
{
	GENERATED_BODY()
	
public:
	// Sets default values for this actor's properties
	AFPSGPlayerStart();

	// Get functions
	float getValidSpawnRadius() const;

protected:
	float validSpawnRadius; // Valid to spawn at this playerstart if no pawn is within the radius
};
