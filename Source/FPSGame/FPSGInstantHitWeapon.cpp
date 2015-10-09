// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGInstantHitWeapon.h"
#include "FPSGCharacter.h"
#include "FPSGHUD.h"
#include "FPSGUtility.h"

AFPSGInstantHitWeapon::AFPSGInstantHitWeapon()
	: Super()
{
	currentSpread = 0.0f;
	useFireSpreadSystem = false;
	hitDamage = 0.0f;
	hitHeadDamage = 0.0f;
	minSpread = 0.0f;
	maxSpread = 0.0f;
	spreadIncrement = 0.0f;
	incrementOverTime = 0.0f;
	impactEffect = NULL;
}

AFPSGInstantHitWeapon::~AFPSGInstantHitWeapon()
{

}

void AFPSGInstantHitWeapon::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("AFPSGInstantHitWeapon::PostInitializeComponents: ") + getWeaponName() + TEXT(": ") + FString::FromInt(getCurrentAmmunition()));
	}
}

void AFPSGInstantHitWeapon::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	Super::EndPlay(endPlayReason);

	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("AFPSGInstantHitWeapon::EndPlay(): ") + getWeaponName() + TEXT(": ") + FString::FromInt(getCurrentAmmunition()));
	}
	
	GetWorldTimerManager().ClearTimer(calculateSpreadTimerHandle);
}

void AFPSGInstantHitWeapon::PostInitProperties()
{
	Super::PostInitProperties();

	initProperties();

	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Blue, TEXT("AFPSGInstantHitWeapon::PostInitProperties(): ") + getWeaponName() + TEXT("Currentspread: ") + FString::FromInt(currentSpread));
	}
}

#if WITH_EDITOR
void AFPSGInstantHitWeapon::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	initProperties();

	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Blue, TEXT("AFPSGWeapon::PostEditChangeProperty(): ") + getWeaponName() + TEXT(": ") + FString::FromInt(getCurrentAmmunition()));
	}
}
#endif

void AFPSGInstantHitWeapon::fire()
{
	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("AFPSGInstantHitWeapon::fire"));
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
			//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("AFPSGInstantHitWeapon::IsLocallyControlled"));

			//Fire an instant shot for each weapon shot
			for (int32 i = 0; i < numberOfShots; ++i)
			{
				//As long as we have ammunition left, we fire a instant shot
				if (haveAmmunition())
				{
					instantFire();
					useAmmunition();
				}
			}
		}
		
		fireCounter++;
		if (GetNetMode() != NM_DedicatedServer)
		{
			simulateFire();
		}
	}
}

bool AFPSGInstantHitWeapon::startFire()
{
	bool startedFiring = Super::startFire();
	
	if (useFireSpreadSystem && startedFiring)
	{
		//Clear the handle set in stopFire if its still active
		if (GetWorldTimerManager().IsTimerActive(calculateSpreadTimerHandle))
		{
			GetWorldTimerManager().ClearTimer(calculateSpreadTimerHandle);
		}

		//Set the handler when firing
		GetWorldTimerManager().SetTimer(calculateSpreadTimerHandle, this, &AFPSGInstantHitWeapon::calculateCurrentSpread, incrementOverTime, true, 0.0f);
	}
	return startedFiring;
}

bool AFPSGInstantHitWeapon::stopFire()
{
	bool stoppedFiring = Super::stopFire();

	if (holdToFire)
	{
		if (useFireSpreadSystem && stoppedFiring)
		{
			//Clear the handle set in startFire if its still active
			if (GetWorldTimerManager().IsTimerActive(calculateSpreadTimerHandle))
			{
				GetWorldTimerManager().ClearTimer(calculateSpreadTimerHandle);
			}

			//Set the handler again when firing has stopped to decrease spread
			GetWorldTimerManager().SetTimer(calculateSpreadTimerHandle, this, &AFPSGInstantHitWeapon::calculateCurrentSpread, incrementOverTime, true, 0.0f);
		}
	}
	return stoppedFiring;
}

void AFPSGInstantHitWeapon::initProperties()
{
	Super::initProperties();

	if (holdToFire && minSpread >= 0.0f && maxSpread > 0.0f && spreadIncrement > 0.0f && incrementOverTime > 0.0f)
	{
		useFireSpreadSystem = true;
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "useFireSpreadSystem");
	}

	currentSpread = minSpread;
}

void AFPSGInstantHitWeapon::instantFire()
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("AFPSGInstantHitWeapon::instantFire"));

	const int32 randomSeed = FMath::Rand();
	FRandomStream randomGenerator(randomSeed);
	const float spreadCone = FMath::DegreesToRadians(currentSpread * 0.5f);

	//Setup trace start location, direction and end location
	const FVector startCameraTrace = myOwner->firstPersonCameraComponent->GetComponentLocation();
	const FVector lookCameraDirection = myOwner->firstPersonCameraComponent->GetForwardVector();
	const FVector shootCameraDirection = randomGenerator.VRandCone(lookCameraDirection, spreadCone, spreadCone);
	const FVector endCameraTrace = startCameraTrace + shootCameraDirection * range;

	//Perform a line/ray trace into the world from the camera (Will go right through the crosshair/middle of screen) (when not using spread)
	TArray<AActor*> traceIgnoreActors;
	traceIgnoreActors.Reserve(2);
	traceIgnoreActors.Emplace(Instigator);
	traceIgnoreActors.Emplace(this);
	TArray<FHitResult> worldTraceHitResults;
	bool worldTraceHit = FPSGUtility::rayTrace(traceIgnoreActors, GetWorld(), startCameraTrace, endCameraTrace, worldTraceHitResults);

	//DEBUG
	if (fireDebug)
	{
		UWorld* world = GetWorld();
		if (world != NULL)
		{
			const FVector startWeaponLineLoc = weaponMeshComponent->GetSocketLocation(muzzleSocket);
			if (worldTraceHitResults.Num() > 0)
			{
				DrawDebugLine(world, startWeaponLineLoc, worldTraceHitResults[0].Location, FColor::Blue, true, 5, 10.0f);
				for (int32 i = 0; i < worldTraceHitResults.Num(); ++i)
				{
					DrawDebugSphere(world, worldTraceHitResults[i].Location, 5, 20, FColor::Green, true, 5, 10.0f);
				}
			}
			else
			{
				DrawDebugLine(world, startWeaponLineLoc, endCameraTrace, FColor::Blue, true, 5, 10.0f);
			}
		}
	}

	//We hit something (Either at least one block or overlap)
	if (worldTraceHit || worldTraceHitResults.Num() > 0)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Hit something"));

		//Loop through all our hits
		for (int32 i = 0; i < worldTraceHitResults.Num(); ++i)
		{
			AFPSGCharacter* otherPlayer = Cast<AFPSGCharacter>(worldTraceHitResults[i].GetActor());

			//We hit a player
			if (otherPlayer != NULL)
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Hit character capsule on player ") + FString::FromInt(i));

				//Execute an additional ray trace against the players mesh to check for more precise collision
				FHitResult meshTraceHitResult;
				bool meshTraceHit = FPSGUtility::rayTrace(traceIgnoreActors, otherPlayer->GetMesh(), startCameraTrace, endCameraTrace, meshTraceHitResult);

				//We hit the players mesh
				if (meshTraceHit)
				{
					//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Hit character mesh on player ") + FString::FromInt(i));
					
					//Process player hit on server
					serverProcessInstantHit(meshTraceHitResult, shootCameraDirection);

					//We hit a player, so dont need to check any more potential hits
					break;
				}
			}
			else
			{
				//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("Hit something in the world on hit ") + FString::FromInt(i));

				//We hit something in the world, process hit on server
				serverProcessInstantHit(worldTraceHitResults[i], shootCameraDirection);
			}
		}
	}
}

void AFPSGInstantHitWeapon::calculateCurrentSpread()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "AFPSGInstantHitWeapon::calculateCurrentSpread");

	//Increment spread when firing
	if (firingState == EWeaponState::FIRING)
	{
		if (currentSpread < maxSpread)
		{
			currentSpread += spreadIncrement;
		}
		else
		{
			//Fire spread is at its maximum, therefore clear timer so it stops executing this function
			GetWorldTimerManager().ClearTimer(calculateSpreadTimerHandle);
		}
		currentSpread = FMath::Min(currentSpread, maxSpread);
	}
	else if (firingState == EWeaponState::IDLE) //Decrement spread when not firing
	{
		if (currentSpread > minSpread)
		{
			currentSpread -= spreadIncrement;
		}
		else
		{
			//Fire spread is at its minimum, therefore clear timer so it stops executing this function
			GetWorldTimerManager().ClearTimer(calculateSpreadTimerHandle);
		}
		currentSpread = FMath::Max(currentSpread, minSpread);
	}

	//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, "currentSpread: " + FString::SanitizeFloat(currentSpread));
}

void AFPSGInstantHitWeapon::serverProcessInstantHit_Implementation(const FHitResult& impact, const FVector& shootDirection)
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, TEXT("AFPSGInstantHitWeapon::serverProcessInstantHit_Implementation"));

	AFPSGCharacter* otherPlayer = Cast<AFPSGCharacter>(impact.GetActor());
	
	//We hit a player
	if (otherPlayer != NULL)
	{
		AFPSGPlayerController* thisPlayerController = Cast<AFPSGPlayerController>(myOwner->GetController());
		if (thisPlayerController == NULL)
		{
			return;
		}

		//Set up point damage for a bullet hit
		FPointDamageEvent pointDamage;
		pointDamage.DamageTypeClass = UDamageType::StaticClass();
		pointDamage.HitInfo = impact;
		pointDamage.ShotDirection = shootDirection;

		//If we hit the head, use head damage
		if (impact.BoneName == otherPlayer->getBoneHead())
		{
			pointDamage.Damage = hitHeadDamage;
		}
		else
		{
			pointDamage.Damage = hitDamage;
		}

		//Apply damage to the player we hit
		otherPlayer->TakeDamage(pointDamage.Damage, pointDamage, thisPlayerController, this);
	}
}

bool AFPSGInstantHitWeapon::serverProcessInstantHit_Validate(const FHitResult& impact, const FVector& shootDirection)
{
	return true;
}
