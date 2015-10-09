// FPSGame created by Gard and Gorm

#pragma once

#include "FPSGWeapon.h"
#include "FPSGInstantHitWeapon.generated.h"

UCLASS()
class FPSGAME_API AFPSGInstantHitWeapon : public AFPSGWeapon
{
	GENERATED_BODY()

public:
	AFPSGInstantHitWeapon();
	virtual ~AFPSGInstantHitWeapon();

	// Called after PreInitializeComponents and before BeginPlay
	virtual void PostInitializeComponents() override;

	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;

	//Initialize values here based on the property values set in the editor
	virtual void PostInitProperties() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void fire() override;

	virtual bool startFire() override;

	virtual bool stopFire() override;

protected:
	/**
	* Function to initialize properties based on the properties that are set in the editor
	*/
	virtual void initProperties() override;

private:
	/**
	* Fires an instant shot with tracing
	*/
	void instantFire();

	/**
	* Function that calculates the current spread
	*/
	void calculateCurrentSpread();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void serverProcessInstantHit(const FHitResult& impact, const FVector& shootDirection);
	void serverProcessInstantHit_Implementation(const FHitResult& impact, const FVector& shootDirection);
	bool serverProcessInstantHit_Validate(const FHitResult& impact, const FVector& shootDirection);

	//The timer handle for executing calculateCurrentSpread each incrementOverTime
	FTimerHandle calculateSpreadTimerHandle;

	//The current spread value when firing
	float currentSpread;

	//Is set to true if we use the weapons fire spread system
	bool useFireSpreadSystem;

	//The standard damage received by each individual shot
	UPROPERTY(EditDefaultsOnly, Category = InstantWeaponConfig)
	float hitDamage;

	//The damage (for head only) received by each individual shot
	UPROPERTY(EditDefaultsOnly, Category = InstantWeaponConfig)
	float hitHeadDamage;

	//The minimum spread for this weapon
	UPROPERTY(EditDefaultsOnly, Category = InstantWeaponConfig)
	float minSpread;

	//The maximum spread for this weapon
	UPROPERTY(EditDefaultsOnly, Category = InstantWeaponConfig)
	float maxSpread;

	//How much to increment the spread each "Increment Over Time"
	UPROPERTY(EditDefaultsOnly, Category = InstantWeaponConfig)
	float spreadIncrement;

	//How often (in seconds) to increment the spread by "Spread Increment"
	UPROPERTY(EditDefaultsOnly, Category = InstantWeaponConfig)
	float incrementOverTime;

	//The particle effect that gets played on impact
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* impactEffect;
};
