// FPSGame created by Gard and Gorm

#pragma once

#include "FPSGWeapon.h"
#include "FPSGProjectile.h"
#include "FPSGCharacter.h"
#include "FPSGProjectileWeapon.generated.h"

UCLASS()
class FPSGAME_API AFPSGProjectileWeapon : public AFPSGWeapon
{
	GENERATED_BODY()

public:
	AFPSGProjectileWeapon();
	virtual ~AFPSGProjectileWeapon();

	virtual void fire() override;

private:
	/**
	* Function that performs all calculations related to firing a projectile
	* @param launchDirections Array to add all the launch directions to
	*/
	void projectileFire(TArray<FVector>& launchDirections);

	/**
	* Server side function that spawns the projectiles and initialize its velocity based on the calculations in projectileFire
	* @param spawnLocation The location to spawn the projectile
	* @param spawnRotation The rotation for the spawned projectile
	* @param launchDirections The array of directions to launch all the projectiles
	*/
	UFUNCTION(Server, Reliable, WithValidation)
	void serverProjectileFire(const FVector& spawnLocation, const FRotator& spawnRotation, const TArray<FVector>& launchDirections);
	void serverProjectileFire_Implementation(const FVector& spawnLocation, const FRotator& spawnRotation, const TArray<FVector>& launchDirections);
	bool serverProjectileFire_Validate(const FVector& spawnLocation, const FRotator& spawnRotation, const TArray<FVector>& launchDirections);

	//The projectile to use for this weapon
	UPROPERTY(EditDefaultsOnly, Category = ProjectileWeaponConfig)
	TSubclassOf<AFPSGProjectile> projectile;

	//The amount of spread this weapon makes
	UPROPERTY(EditDefaultsOnly, Category = ProjectileWeaponConfig)
	float spread;
};
