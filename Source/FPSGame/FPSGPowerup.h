// FPSGame created by Gard and Gorm

#pragma once

#include "GameFramework/Actor.h"

#include "FPSGPickup.h"

#include "FPSGPowerup.generated.h"

UCLASS()
class FPSGAME_API AFPSGPowerup : public AFPSGPickup
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSGPowerup();
	virtual ~AFPSGPowerup();

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

protected:
	/**
	* Function to do something determined by this AFPSGPowerup actor when picked up
	* @param otherActor The actor that picked up the AFPSGPowerup
	*/
	virtual void onPickedUp(class AActor* otherActor) override;

	// The amount this powerup updates
	UPROPERTY(EditDefaultsOnly, Category = UpdateAmount)
	float updateAmount;

private:
	// The collision component for this powerup
	UPROPERTY(VisibleAnywhere, Category = Collision)
	USphereComponent* collisionComp;

	// The mesh component for this powerup
	UPROPERTY(VisibleAnywhere, Category = Config)
	UStaticMeshComponent* powerupMeshComponent;
};
