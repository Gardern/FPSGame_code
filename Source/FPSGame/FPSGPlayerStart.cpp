// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGPlayerStart.h"

// Sets default values
AFPSGPlayerStart::AFPSGPlayerStart()
	: Super()
{
	validSpawnRadius = 1300.0f;
}

float AFPSGPlayerStart::getValidSpawnRadius() const
{
	return validSpawnRadius;
}
