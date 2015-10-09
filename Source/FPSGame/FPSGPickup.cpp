// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGPickup.h"
#include "FPSGCharacter.h"

// Sets default values
AFPSGPickup::AFPSGPickup()
	: Super()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	respawnTime = 0.0f;
}

AFPSGPickup::~AFPSGPickup()
{

}

// Called when the game starts or when spawned
void AFPSGPickup::BeginPlay()
{
	Super::BeginPlay();	
}

void AFPSGPickup::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	Super::EndPlay(endPlayReason);

	GetWorldTimerManager().ClearTimer(respawnTimerHandle);
}

// Called every frame
void AFPSGPickup::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

void AFPSGPickup::onOverlap(class AActor* otherActor, class UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult)
{
	// Only character should be able to overlap with a pickup
	if (otherActor->IsA(AFPSGCharacter::StaticClass()))
	{
		//GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Red, TEXT("otherActor is AFPSGCharacter"));

		// Picked up the powerup
		onPickedUp(otherActor);

		// Remove powerup by setting it to invisible and disable collision
		remove();

		// debug
		if (GEngine != NULL)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red, "AFPSGPickup::onOverlap");
		}
	}
}

void AFPSGPickup::onPickedUp(class AActor* otherActor)
{

}

void AFPSGPickup::remove()
{
	// Clear timer just in case (Nothing to clear first time)
	GetWorldTimerManager().ClearTimer(respawnTimerHandle);

	// Set timer to respawn pickup
	GetWorldTimerManager().SetTimer(respawnTimerHandle, this, &AFPSGPickup::spawn, respawnTime, false);

	// Remove the pickup by setting it to invisible and disable collision
	SetActorHiddenInGame(true);
	SetActorEnableCollision(false);

	// debug
	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red, "AFPSGPickup::remove");
	}
}

void AFPSGPickup::spawn()
{
	// Spawn the pickup by setting it to visible and enable collision
	SetActorHiddenInGame(false);
	SetActorEnableCollision(true);

	// debug
	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red, "AFPSGPickup::spawn");
	}
}
