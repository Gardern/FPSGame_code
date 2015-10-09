// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGProjectile.h"
#include "FPSGCharacter.h"

// Sets default values
AFPSGProjectile::AFPSGProjectile()
	: Super()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;
	bReplicateMovement = true;
	bCanBeDamaged = false;

	sphereCollisionComponent = CreateDefaultSubobject<USphereComponent>(TEXT("FPSGProjectileCollision"));
	sphereCollisionComponent->SetNotifyRigidBodyCollision(true);

	RootComponent = sphereCollisionComponent;

	projectileMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FPSGProjectileMesh"));
	projectileMeshComponent->AttachTo(RootComponent);

	movementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("FPSGProjectileMovement"));
	movementComponent->UpdatedComponent = sphereCollisionComponent;
	movementComponent->bRotationFollowsVelocity = true;
	movementComponent->OnProjectileStop.AddDynamic(this, &AFPSGProjectile::onExplode);

	projectileParticleComponent = CreateDefaultSubobject<UParticleSystemComponent>(TEXT("FPSGProjectileParticle"));
	projectileParticleComponent->bAutoDestroy = false;
	projectileParticleComponent->AttachParent = RootComponent;

	myOwnerController = NULL;
	radiusDebug = false;
	maxDamage = 0.0f;
	minDamage = 0.0f;
	innerRadius = 0.0f;
	outerRadius = 0.0f;
	damageFalloff = 0.0f;
	bExploded = false;
	impactEffect = NULL;
}

void AFPSGProjectile::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSGProjectile, bExploded);
}

void AFPSGProjectile::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Black, TEXT("AFPSGProjectile::PostInitializeComponents"));

	sphereCollisionComponent->MoveIgnoreActors.Add(GetInstigator());
}

// Called every frame
void AFPSGProjectile::Tick(float DeltaTime)
{
	Super::Tick( DeltaTime );
}

void AFPSGProjectile::initVelocity(FVector* direction)
{
	if (movementComponent != NULL && direction != NULL)
	{
		//Normalize direction if its not already normalized
		if (!direction->IsNormalized())
		{
			*direction = direction->GetSafeNormal();
		}

		//Set the projectiles velocity
		movementComponent->Velocity = (*direction) * (movementComponent->InitialSpeed);
	}
}

void AFPSGProjectile::onExplode(const FHitResult& hitResult)
{
	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Blue, TEXT("AFPSGProjectile::onExplode"));
	
	// Run on server to apply damage
	if (Role == ROLE_Authority)
	{
		if (myOwnerController != NULL)
		{
			//Apply radial damage (Will call TakeDamage function for all Actors inside the damage radius)
			UGameplayStatics::ApplyRadialDamageWithFalloff(this, maxDamage, minDamage, hitResult.Location, innerRadius, outerRadius, damageFalloff,
				UDamageType::StaticClass(), TArray<AActor*>(), this, myOwnerController);
		}
	}

	// Run on server to render explosion effect and disable projectile
	if (Role == ROLE_Authority && !bExploded)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Blue, TEXT("Server before renderExplosionEffect and disable"));

		renderExplosionEffect(hitResult);
		disable();
	}

	//DEBUG radius
	if (radiusDebug)
	{
		UWorld* world = GetWorld();
		if (world != NULL)
		{
			DrawDebugSphere(world, hitResult.Location, innerRadius, 25, FColor::Blue, true, 2, 10.0f);
			DrawDebugSphere(world, hitResult.Location, outerRadius, 30, FColor::Red, true, 2, 10.0f);
		}
	}
}

void AFPSGProjectile::setMyOwnerController(AFPSGPlayerController* inMyOwnerController)
{
	myOwnerController = inMyOwnerController;
}

void AFPSGProjectile::renderExplosionEffect(const FHitResult& impact)
{
	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Blue, TEXT("renderExplosionEffect"));

	if (projectileParticleComponent)
	{
		projectileParticleComponent->Deactivate();
	}
	
	// Render the explosion effect
	if (GetNetMode() != NM_DedicatedServer)
	{
		UGameplayStatics::SpawnEmitterAtLocation(this, impactEffect, impact.ImpactPoint);
	}

	bExploded = true;
}

void AFPSGProjectile::disable()
{
	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Blue, TEXT("disable"));

	movementComponent->StopMovementImmediately();

	// Set the projectile to invisible
	SetActorHiddenInGame(true);

	// Wait to destroy the projectile to make sure clients render the explosion effect first
	SetLifeSpan(2.0f);
}

void AFPSGProjectile::onRep_Exploded()
{
	//GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Blue, TEXT("onRep_Exploded"));

	FHitResult impact;
	impact.ImpactPoint = GetActorLocation();

	renderExplosionEffect(impact);
}
