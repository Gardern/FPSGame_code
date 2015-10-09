// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGProjectileWeapon.h"
#include "FPSGCharacter.h"
#include "FPSGUtility.h"

AFPSGProjectileWeapon::AFPSGProjectileWeapon()
	: Super()
{
	spread = 0.0f;
}

AFPSGProjectileWeapon::~AFPSGProjectileWeapon()
{

}

void AFPSGProjectileWeapon::fire()
{
	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("AFPSGProjectileWeapon::fire"));
	}

	if (Role < ROLE_Authority)
	{
		serverFire();
	}

	//Only fire if we have ammunition
	if (haveAmmunition())
	{
		//Executed only by local player
		if (myOwner != NULL && myOwner->IsLocallyControlled())
		{
			TArray<FVector> launchDirections;
			launchDirections.Reserve(numberOfShots);

			//Set the spawn location and rotation (These are sent to the server, all projectiles use the same)
			const FVector spawnLocation = weaponMeshComponent->GetSocketLocation(muzzleSocket);
			const FRotator spawnRotation = weaponMeshComponent->GetSocketRotation(muzzleSocket);

			projectileFire(launchDirections);
			serverProjectileFire(spawnLocation, spawnRotation, launchDirections);
		}

		fireCounter++;
		if (GetNetMode() != NM_DedicatedServer)
		{
			simulateFire();
		}
	}
}

void AFPSGProjectileWeapon::projectileFire(TArray<FVector>& launchDirections)
{
	//Fire a projectile for each weapon shot
	for (int32 i = 0; i < numberOfShots; ++i)
	{
		//As long as we have ammunition left, perform all calculations for this projectile, and lower the ammunition
		if (haveAmmunition())
		{
			const FVector spawnLocation = weaponMeshComponent->GetSocketLocation(muzzleSocket);

			const int32 randomSeed = FMath::Rand();
			FRandomStream randomGenerator(randomSeed);
			const float spreadCone = FMath::DegreesToRadians(spread * 0.5f);

			//Setup trace start location, direction and end location
			const FVector startCameraTrace = myOwner->firstPersonCameraComponent->GetComponentLocation();
			const FVector lookCameraDirection = myOwner->firstPersonCameraComponent->GetForwardVector();
			const FVector shootCameraDirection = randomGenerator.VRandCone(lookCameraDirection, spreadCone, spreadCone);
			const FVector endCameraTrace = startCameraTrace + shootCameraDirection * range;
				
			//Perform the line/ray trace from the camera (Will go right through the crosshair/middle of screen) (when not using spread)
			TArray<AActor*> standardTraceIgnoreActors;
			standardTraceIgnoreActors.Reserve(2);
			standardTraceIgnoreActors.Emplace(Instigator);
			standardTraceIgnoreActors.Emplace(this);
			FHitResult standardTraceHitResult;
			bool standardTraceHit = FPSGUtility::rayTrace(standardTraceIgnoreActors, GetWorld(), startCameraTrace, endCameraTrace, standardTraceHitResult);

			//Calculate the launch direction from the weapon to the camera trace hit location
			//If we hit something, use the hit location, if not use the location at the trace end
			FVector launchDirection;
			if (standardTraceHitResult.bBlockingHit && standardTraceHit)
			{
				launchDirection = standardTraceHitResult.Location - spawnLocation;
			}
			else
			{
				launchDirection = standardTraceHitResult.TraceEnd - spawnLocation;
			}

			//Normalize the launch direction, and add it to the array of launch directions (will be sent to the server)
			if (launchDirection.Normalize())
			{
				launchDirections.Emplace(launchDirection);
			}
			useAmmunition();
		}
	}
}

void AFPSGProjectileWeapon::serverProjectileFire_Implementation(const FVector& spawnLocation, const FRotator& spawnRotation, const TArray<FVector>& launchDirections)
{
	UWorld* world = GetWorld();

	if (projectile != NULL && world != NULL)
	{
		//Iterate through all the projectiles launch directions that we got from the client, and spawn them
		for (const FVector& launchDir : launchDirections)
		{
			//Set up spawn parameters and spawn the projectile
			FActorSpawnParameters spawnParameters;
			spawnParameters.Owner = this;
			spawnParameters.Instigator = myOwner;

			AFPSGProjectile* spawnedProjectile = world->SpawnActor<AFPSGProjectile>(projectile, spawnLocation, spawnRotation, spawnParameters);

			//Initialize its controller owner and velocity
			if (spawnedProjectile != NULL)
			{
				spawnedProjectile->setMyOwnerController(Cast<AFPSGPlayerController>(myOwner->Controller));

				FVector direction = launchDir;
				spawnedProjectile->initVelocity(&direction);
			}
		}
	}
}

bool AFPSGProjectileWeapon::serverProjectileFire_Validate(const FVector& spawnLocation, const FRotator& spawnRotation, const TArray<FVector>& launchDirections)
{
	return true;
}
