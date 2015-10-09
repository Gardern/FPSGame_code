// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGWeapon.h"
#include "FPSGCharacter.h"

// Sets default values
AFPSGWeapon::AFPSGWeapon()
	: Super()
{
	weaponMeshComponent = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSGWeaponMesh"));
	weaponMeshComponent->bSelfShadowOnly = true;
	RootComponent = weaponMeshComponent;

	maxAmmunition = 0;
	currentAmmunition = 0;
	firingState = EWeaponState::IDLE;
	weaponName = "";
	timeBetweenShots = 0.0f;
	range = 0.0f;
	numberOfShots = 0.0f;
	holdToFire = false;
	fireDebug = false;
	muzzleSocket = "";
	muzzleEffect = NULL;
	equipAtSocket = "";
	myOwner = NULL;
	fireCounter = 0;
}

AFPSGWeapon::~AFPSGWeapon()
{

}

bool AFPSGWeapon::operator==(const AFPSGWeapon& weapon) const
{
	return weaponName == weapon.weaponName;
}

//Place all variables that should be replicated in here
void AFPSGWeapon::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	DOREPLIFETIME_CONDITION(AFPSGWeapon, currentAmmunition, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFPSGWeapon, myOwner, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFPSGWeapon, fireCounter, COND_SkipOwner);
}

void AFPSGWeapon::PreInitializeComponents()
{
	Super::PreInitializeComponents();

	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("AFPSGWeapon::PreInitializeComponents(): ") + getWeaponName() + TEXT(": ") + FString::FromInt(getCurrentAmmunition()));
	}
	
	//boxCollisionComponent is created on the server, which means it should point to NULL on clients
	if (Role == ROLE_Authority)
	{
		boxCollisionComponent = NewObject<UBoxComponent>(this, UBoxComponent::StaticClass(), TEXT("FPSGWeaponCollision"));

		if (boxCollisionComponent != NULL)
		{
			boxCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AFPSGWeapon::onOverlap);
			boxCollisionComponent->bGenerateOverlapEvents = true;
			boxCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			boxCollisionComponent->SetCollisionObjectType(COLLISION_PICKUP);
			boxCollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
			boxCollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
			boxCollisionComponent->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
			boxCollisionComponent->AttachTo(RootComponent);
			boxCollisionComponent->RegisterComponent();

			boxCollisionComponent->bHiddenInGame = false;
		}

		/*
		boxCollisionComponent = ConstructObject<UBoxComponent>(UBoxComponent::StaticClass(), this, TEXT("FPSGWeaponCollision"));

		if (boxCollisionComponent != NULL)
		{
			boxCollisionComponent->OnComponentBeginOverlap.AddDynamic(this, &AFPSGWeapon::onOverlap);
			boxCollisionComponent->bGenerateOverlapEvents = true;
			boxCollisionComponent->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
			boxCollisionComponent->SetCollisionObjectType(COLLISION_PICKUP);
			boxCollisionComponent->SetCollisionResponseToAllChannels(ECR_Ignore);
			boxCollisionComponent->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
			boxCollisionComponent->SetCollisionResponseToChannel(COLLISION_PROJECTILE, ECR_Block);
			boxCollisionComponent->AttachTo(RootComponent);
			boxCollisionComponent->RegisterComponent();

			boxCollisionComponent->bHiddenInGame = false;
		}
		*/
	}
}

// Called when the game starts or when spawned
void AFPSGWeapon::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Blue, TEXT("AFPSGWeapon::BeginPlay(): ") + getWeaponName() + TEXT(": ") + FString::FromInt(getCurrentAmmunition()));
	}
}

void AFPSGWeapon::EndPlay(const EEndPlayReason::Type endPlayReason)
{
	Super::EndPlay(endPlayReason);

	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Green, TEXT("AFPSGWeapon::EndPlay(): ") + getWeaponName() + TEXT(": ") + FString::FromInt(getCurrentAmmunition()));
	}

	GetWorldTimerManager().ClearTimer(singleFireTimerHandle);
	GetWorldTimerManager().ClearTimer(multiFireTimerHandle);
	GetWorldTimerManager().ClearTimer(invokeMultiFireTimerHandle);
}

void AFPSGWeapon::PostInitProperties()
{
	Super::PostInitProperties();
	
	initProperties();
	
	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 4.f, FColor::Blue, TEXT("AFPSGWeapon::PostInitProperties(): ") + getWeaponName() + TEXT(": ") + FString::FromInt(getCurrentAmmunition()));
	}
}

#if WITH_EDITOR
void AFPSGWeapon::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	initProperties();

	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 6.f, FColor::Blue, TEXT("AFPSGWeapon::PostEditChangeProperty(): ") + getWeaponName() + TEXT(": ") + FString::FromInt(getCurrentAmmunition()));
	}
}
#endif

// Called every frame
void AFPSGWeapon::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void AFPSGWeapon::fire()
{

}

bool AFPSGWeapon::startFire()
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("AFPSGWeapon::startFire"));

	bool startedFiring = false;

	//No need to start firing if we dont have ammunition
	if (haveAmmunition())
	{
		//Fire at interval "timeBetweenShots" if this is a hold down to shoot weapon
		if (holdToFire)
		{
			if (!GetWorldTimerManager().IsTimerActive(invokeMultiFireTimerHandle) && timeBetweenShots > 0.0f)
			{
				//Clear timer just in case
				GetWorldTimerManager().ClearTimer(invokeMultiFireTimerHandle);

				GetWorldTimerManager().SetTimer(invokeMultiFireTimerHandle, timeBetweenShots, false);

				GetWorldTimerManager().SetTimer(multiFireTimerHandle, this, &AFPSGWeapon::fire, timeBetweenShots, true, 0.0f);

				firingState = EWeaponState::FIRING;
				startedFiring = true;
			}
		}
		else //Otherwise just be able to fire a single time each "timeBetweenShots"
		{
			//If the timer is not active, we can fire. we also activate it again
			if (!GetWorldTimerManager().IsTimerActive(singleFireTimerHandle) && timeBetweenShots > 0.0f)
			{
				fire();

				//Clear timer just in case
				GetWorldTimerManager().ClearTimer(singleFireTimerHandle);

				GetWorldTimerManager().SetTimer(singleFireTimerHandle, timeBetweenShots, false);

				firingState = EWeaponState::FIRING;
				startedFiring = true;
			}
		}
	}
	return startedFiring;
}

bool AFPSGWeapon::stopFire()
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("AFPSGWeapon::stopFire"));

	bool stoppedFiring = true;

	if (Role < ROLE_Authority)
	{
		serverStopFire();
	}

	firingState = EWeaponState::IDLE;
	fireCounter = 0;

	if (holdToFire)
	{
		GetWorldTimerManager().ClearTimer(multiFireTimerHandle);
	}
	return stoppedFiring;
}

UBoxComponent* AFPSGWeapon::getBoxCollisionComponent() const
{
	return boxCollisionComponent;
}

FString AFPSGWeapon::getWeaponName() const
{
	return weaponName;
}

const int32 AFPSGWeapon::getMaxAmmunition() const
{
	return maxAmmunition;
}

float AFPSGWeapon::getTimeBetweenShots() const
{
	return timeBetweenShots;
}

float AFPSGWeapon::getRange() const
{
	return range;
}

float AFPSGWeapon::getNumberOfShots() const
{
	return numberOfShots;
}

int32 AFPSGWeapon::getCurrentAmmunition() const
{
	return currentAmmunition;
}

UParticleSystem* AFPSGWeapon::getMuzzleEffect() const
{
	return muzzleEffect;
}

FName AFPSGWeapon::getEquipAtSocket() const
{
	return equipAtSocket;
}

void AFPSGWeapon::setCurrentAmmunition(int32 inCurrentAmmunition)
{
	currentAmmunition = inCurrentAmmunition;

	if (currentAmmunition > maxAmmunition)
	{
		currentAmmunition = maxAmmunition;
	}
}

void AFPSGWeapon::setMyOwner(class AFPSGCharacter* inOwner)
{
	myOwner = inOwner;
}

void AFPSGWeapon::onPickedUp(class AActor* otherActor)
{
	GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Red, "AFPSGWeapon::onPickedUp");

	/*
	* Overlaps on AFPSGWeapon will only be executed on the server since only the server owns the boxCollisionComponent
	*/

	//Just return if we are a client (should not happen, but checks just in case)
	if (Role < ROLE_Authority && GetNetMode() == NM_Client)
	{
		return;
	}
	
	// Get the character that picked up the weapon
	AFPSGCharacter* theCharacter = Cast<AFPSGCharacter>(otherActor);

	if (theCharacter != NULL)
	{
		if (GEngine != NULL)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Red, TEXT("AFPSGWeapon::onPickedUp"));
		}
		theCharacter->manageWeapon(this);
	}
}

void AFPSGWeapon::initProperties()
{
	//maxAmmunition is set in the blueprint class in the editor
	currentAmmunition = maxAmmunition;
}

bool AFPSGWeapon::haveAmmunition() const
{
	return (currentAmmunition > 0);
}

void AFPSGWeapon::useAmmunition()
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("AFPSGWeapon::useAmmunition"));

	//First, we use ammunition locally
	--currentAmmunition;
	
	//Then if we are a client, we let the server do the same on its own version
	//The reason that we first updated it on local client too is:
	//If we let only the server update its version, and there is a long delay until currentAmmunition gets replicated to the client, then
	//haveAmmunition checks on the client may succeed in the meanwhile meaning we can still shoot even if we should not have any ammunition.
	if (Role < ROLE_Authority)
	{
		serverUseAmmunition();
	}
}
void AFPSGWeapon::simulateFire()
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("AFPSGWeapon::simulateFire"));

	playEffects();
}

void AFPSGWeapon::onRep_fireCounter()
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("AFPSGWeapon::onRep_fireCounter, fireCounter: ") + FString::FromInt(fireCounter));

	if (fireCounter > 0)
	{
		simulateFire();
	}
}

void AFPSGWeapon::serverFire_Implementation()
{
	fire();
}

bool AFPSGWeapon::serverFire_Validate()
{
	return true;
}

void AFPSGWeapon::serverStopFire_Implementation()
{
	//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("AFPSGWeapon::serverStopFire_Implementation"));
	
	fireCounter = 0;
}

bool AFPSGWeapon::serverStopFire_Validate()
{
	return true;
}

void AFPSGWeapon::playEffects()
{
	//Spawn the muzzle particle effect
	UGameplayStatics::SpawnEmitterAttached(muzzleEffect, weaponMeshComponent, muzzleSocket);
}

void AFPSGWeapon::serverUseAmmunition_Implementation()
{
	useAmmunition();
}

bool AFPSGWeapon::serverUseAmmunition_Validate()
{
	return true;
}
