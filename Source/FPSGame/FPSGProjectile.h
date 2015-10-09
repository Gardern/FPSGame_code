// FPSGame created by Gard and Gorm

#pragma once

#include "GameFramework/Actor.h"

#include "FPSGPlayerController.h"

#include "FPSGProjectile.generated.h"

UCLASS()
class FPSGAME_API AFPSGProjectile : public AActor
{
	GENERATED_BODY()
	
public:	
	// Sets default values for this actor's properties
	AFPSGProjectile();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	// Initial setup
	virtual void PostInitializeComponents() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	/**
	* Initialize the velocity of this projectile in the shoot direction
	* @param direction The direction to initialize the velocity in (Normalizes the direction vector if its not already normalized)
	*/
	void initVelocity(FVector* direction);

	/**
	* Apply damage and other stuff when the projectile explodes
	* @param hitResult Information about the hit
	*/
	UFUNCTION()
	void onExplode(const FHitResult& hitResult);

	void setMyOwnerController(AFPSGPlayerController* inMyOwnerController);
	
private:
	/**
	* Function to render the explosion effect for the projectile
	* @param impact Information about the hit
	*/
	void renderExplosionEffect(const FHitResult& impact);

	/**
	* Disable the projectile
	*/
	void disable();

	/**
	* Function runs on client to render explosion effect when bExploded is replicated
	*/
	UFUNCTION()
	void onRep_Exploded();

	//The player controller that owns this projectile. This represents the controller of the character that shot it and is responsible for its damage
	//NOTE: This points to the same as the characters Controller variable, just casted down to AFPSGPlayerController
	//NOTE: This pointer is only available on the server as its not replicated and not set locally.
	AFPSGPlayerController* myOwnerController;

	//The projectiles movement component
	UPROPERTY(VisibleAnywhere, Category = Movement)
	UProjectileMovementComponent* movementComponent;

	//The sphere collision component
	UPROPERTY(VisibleAnywhere, Category = Collision)
	USphereComponent* sphereCollisionComponent;

	//The projectiles mesh component
	UPROPERTY(VisibleAnywhere, Category = Config)
	UStaticMeshComponent* projectileMeshComponent;

	//The projectiles particle component
	UPROPERTY(VisibleAnywhere, Category = Particle)
	UParticleSystemComponent* projectileParticleComponent;

	//set to true to debug projectile explosion radius (USED ONLY FOR DEBUG)
	UPROPERTY(EditDefaultsOnly, Category = ProjectileData)
	bool radiusDebug;

	//The maximum amount of damage, i.e. the damage at the location of the projectile explosion
	UPROPERTY(EditDefaultsOnly, Category = ProjectileData)
	float maxDamage;

	//The minimum amount of damage
	UPROPERTY(EditDefaultsOnly, Category = ProjectileData)
	float minDamage;

	//Radius of the max damage area
	UPROPERTY(EditDefaultsOnly, Category = ProjectileData)
	float innerRadius;

	//Radius of the minimum damage area
	UPROPERTY(EditDefaultsOnly, Category = ProjectileData)
	float outerRadius;

	//The damage falloff exponent from innerRadius to outerRadius (1.0 for linear increase from minDamage to maxDamage)
	UPROPERTY(EditDefaultsOnly, Category = ProjectileData)
	float damageFalloff;

	//True when projectile explodes
	UPROPERTY(Transient, ReplicatedUsing = onRep_Exploded)
	bool bExploded;

	//The particle effect that gets rendered on explosion
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* impactEffect;
};
