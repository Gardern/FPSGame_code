// FPSGame created by Gard and Gorm

#pragma once

#include "GameFramework/Actor.h"
#include "FPSGPickup.generated.h"

UCLASS()
class FPSGAME_API AFPSGPickup : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSGPickup();
	virtual ~AFPSGPickup();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	/**
	* Function that is called when an AActor overlaps with this AFPSGPickup actor
	* @param otherActor The actor that overlaps with the AFPSGPickup
	*/
	UFUNCTION()
	virtual void onOverlap(class AActor* otherActor, class UPrimitiveComponent* otherComp, int32 otherBodyIndex, bool bFromSweep, const FHitResult& sweepResult);

protected:
	/**
	* Function to do something determined by this AFPSGPickup actor when picked up
	* @param otherActor The actor that picked up the AFPSGPickup
	*/
	virtual void onPickedUp(class AActor* otherActor);

	/**
	* Function to remove the AFPSGPickup from the map
	*/
	virtual void remove();

	/**
	* Function to respawn the AFPSGPickup on the map
	*/
	virtual void spawn();

	// The handle for the respawn timer
	FTimerHandle respawnTimerHandle;

	// The time it takes for this pickup to respawn
	UPROPERTY(EditDefaultsOnly, Category = RespawnTime)
	float respawnTime;
};
