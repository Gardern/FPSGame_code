// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGCharacter.h"
#include "FPSGWeapon.h"
#include "FPSGInstantHitWeapon.h"
#include "FPSGProjectileWeapon.h"
#include "FPSGGameMode.h"
#include "FPSGPlayerState.h"
#include "Net/UnrealNetwork.h"

AFPSGCharacter::AFPSGCharacter()
	: Super()
{
	bReplicates = true;

 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	GetCapsuleComponent()->bGenerateOverlapEvents = true;

	//Initialize the first person camera component
	firstPersonCameraComponent = CreateDefaultSubobject<UCameraComponent>(TEXT("FPSGFirstPersonCamera"));
	firstPersonCameraComponent->AttachParent = GetCapsuleComponent();

	FVector characterViewLocation;
	FRotator characterViewRotation;
	GetActorEyesViewPoint(characterViewLocation, characterViewRotation);

	//Position the camera and use the view rotation of the pawn that we get from input
	firstPersonCameraComponent->RelativeLocation = characterViewLocation;
	firstPersonCameraComponent->RelativeRotation = characterViewRotation;
	firstPersonCameraComponent->bUsePawnControlRotation = true;

	//Initialize the first person mesh component
	firstPersonMesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("FPSGFirstPersonMesh"));
	firstPersonMesh->SetOnlyOwnerSee(true);
	firstPersonMesh->AttachParent = firstPersonCameraComponent;
	firstPersonMesh->bCastDynamicShadow = false;
	firstPersonMesh->CastShadow = false;

	//The regular body mesh should not be visible to the player
	GetMesh()->SetOwnerNoSee(true);

	movementDirection = EMoveDirection::NONE;
	maxHealth = 100.0f;
	currentHealth = 100.0f;
	currentWeapon = NULL;
	isFiring = false;
	isFalling = false;
	amountOfGrenades = 0;
	maxGrenades = 0;
	deathAnimation = NULL;
	inventoryWeaponSocket = "";
	boneHead = "";
	//Reserve memory for at least 10 elements (weapons)
	weaponInventory.Reserve(10);
	dieCallback = false;
}

//Place all variables that should be replicated in here
void AFPSGCharacter::GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(AFPSGCharacter, movementDirection);
	DOREPLIFETIME(AFPSGCharacter, dieCallback);
	DOREPLIFETIME_CONDITION(AFPSGCharacter, currentWeapon, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFPSGCharacter, weaponInventory, COND_OwnerOnly);
	DOREPLIFETIME_CONDITION(AFPSGCharacter, currentHealth, COND_OwnerOnly);
}

void AFPSGCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	currentHealth = maxHealth;

	UWorld* world = GetWorld();
	if (world != NULL)
	{
		AFPSGGameMode* gameMode = world->GetAuthGameMode<AFPSGGameMode>();

		//Add our start weapon(s) that the gamemode defines (server only)
		if (gameMode != NULL && Role == ROLE_Authority)
		{
			for (const TSubclassOf<AFPSGWeapon>& weapon : gameMode->getStartWeapons())
			{
				//Spawn and add the weapon to the inventory
				manageWeapon(weapon.GetDefaultObject());
			}
			//Equip the first weapon
			equipWeaponOne();
		}
	}
}

// Called when the game starts or when spawned
void AFPSGCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Blue, TEXT("AFPSGCharacter::BeginPlay()"));
	}
}

void AFPSGCharacter::PossessedBy(AController * NewController)
{
	Super::PossessedBy(NewController);

	GEngine->AddOnScreenDebugMessage(-1, 20.0f, FColor::Yellow, TEXT("AFPSGCharacter::PossessedBy"));

	AFPSGPlayerController* myPlayerController = Cast<AFPSGPlayerController>(GetController());

	if (myPlayerController != NULL)
	{
		//Set to true, and when replicated to clients, will call the callback function onRep_SpawnOwnerOnly in player controller
		myPlayerController->setSpawnCallback(true);

		//We are the server, but just to be sure
		if (Role == ROLE_Authority)
		{
			//Server have to manually call the onRep_SpawnOwnerOnly callback
			myPlayerController->onRep_SpawnOwnerOnly();
		}
	}
}

void AFPSGCharacter::PostInitProperties()
{
	Super::PostInitProperties();

	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Black, TEXT("AFPSGCharacter::PostInitProperties"));
	}

	initProperties();
}

void AFPSGCharacter::Destroyed()
{
	Super::Destroyed();

	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Red, "AFPSGCharacter::Destroyed()");

	destroyInventory();
}

// Called every frame
void AFPSGCharacter::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );

	//The characters velocity magnitude is zero if the character is standing still
	if (GetVelocity().Size() == 0.0f)
	{
		if (movementDirection != EMoveDirection::NONE)
		{
			serverUpdateMovementDirection(static_cast<uint8>(EMoveDirection::NONE));
		}
	}
}

// Called to bind functionality to input
void AFPSGCharacter::SetupPlayerInputComponent(class UInputComponent* InputComponent)
{
	Super::SetupPlayerInputComponent(InputComponent);

	InputComponent->BindAxis("MoveForward", this, &AFPSGCharacter::moveForward);
	InputComponent->BindAxis("MoveRight", this, &AFPSGCharacter::moveRight);

	InputComponent->BindAxis("Turn", this, &AFPSGCharacter::AddControllerYawInput);
	InputComponent->BindAxis("LookUp", this, &AFPSGCharacter::AddControllerPitchInput);

	InputComponent->BindAction("Jump", IE_Pressed, this, &AFPSGCharacter::startJump);
	InputComponent->BindAction("Jump", IE_Released, this, &AFPSGCharacter::stopJump);

	InputComponent->BindAction("Fire", IE_Pressed, this, &AFPSGCharacter::startWeaponFire);
	InputComponent->BindAction("Fire", IE_Released, this, &AFPSGCharacter::stopWeaponFire);
	InputComponent->BindAction("ThrowGrenade", IE_Pressed, this, &AFPSGCharacter::startThrowGrenade);
	InputComponent->BindAction("EquipWeaponOne", IE_Pressed, this, &AFPSGCharacter::equipWeaponOne);
	InputComponent->BindAction("EquipWeaponTwo", IE_Pressed, this, &AFPSGCharacter::equipWeaponTwo);
	InputComponent->BindAction("EquipWeaponThree", IE_Pressed, this, &AFPSGCharacter::equipWeaponThree);
	InputComponent->BindAction("EquipWeaponFour", IE_Pressed, this, &AFPSGCharacter::equipWeaponFour);
	InputComponent->BindAction("EquipWeaponFive", IE_Pressed, this, &AFPSGCharacter::equipWeaponFive);
	InputComponent->BindAction("EquipWeaponSix", IE_Pressed, this, &AFPSGCharacter::equipWeaponSix);
	InputComponent->BindAction("EquipWeaponSeven", IE_Pressed, this, &AFPSGCharacter::equipWeaponSeven);
	InputComponent->BindAction("EquipWeaponEight", IE_Pressed, this, &AFPSGCharacter::equipWeaponEight);
	InputComponent->BindAction("equipWeaponNine", IE_Pressed, this, &AFPSGCharacter::equipWeaponNine);
	InputComponent->BindAction("EquipPreviousWeapon", IE_Pressed, this, &AFPSGCharacter::equipPreviousWeapon);
	InputComponent->BindAction("EquipNextWeapon", IE_Pressed, this, &AFPSGCharacter::equipNextWeapon);}

float AFPSGCharacter::TakeDamage(float damageAmount, struct FDamageEvent const& damageEvent, class AController* eventInstigator, class AActor* damageCauser)
{
	//eventInstigator Player that killed this player

	float actualDamageAmount = Super::TakeDamage(damageAmount, damageEvent, eventInstigator, damageCauser);

	//GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Red, "AFPSGCharacter::TakeDamage: " + FString::FromInt(actualDamageAmount));

	if (currentHealth > 0.0f && actualDamageAmount > 0.0f)
	{
		currentHealth -= actualDamageAmount;

		if (currentHealth <= 0.0f)
		{
			//Correct the actual damage in case health is less than zero
			actualDamageAmount += currentHealth;
			currentHealth = 0.0f;

			//Call this here to make sure currentHealth is replicated immediately, probably dont need it
			ForceNetUpdate();

			// Kill character
			die(Cast<AFPSGPlayerController>(eventInstigator));
		}
		//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, "Take Damage Health" + FString::SanitizeFloat(currentHealth));
	}

	return actualDamageAmount;
}

void AFPSGCharacter::FellOutOfWorld(const class UDamageType & dmgType)
{
	if (!isFalling)
	{
		if (GEngine != NULL)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 2.0f, FColor::Yellow, TEXT("AFPSGCharacter::FellOutOfWorld"));
		}

		isFalling = true;
		die(NULL);
	}
}

void AFPSGCharacter::manageWeapon(AFPSGWeapon* in_weapon)
{
	bool weaponFound = isInInventory(in_weapon);

	//This weapon was found in the inventory, only add its ammunition
	if (weaponFound)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Red, "weaponFound");

		AFPSGWeapon* weapon = findWeapon(in_weapon);

		if (weapon != NULL)
		{
			//GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Red, "Add ammunition");
			weapon->setCurrentAmmunition(weapon->getCurrentAmmunition() + in_weapon->getCurrentAmmunition());
		}
	}
	else //Otherwise, add the weapon
	{
		//GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Red, "weapon NOT found");

		//Spawn the weapon
		AFPSGWeapon* spawnedWeapon = spawnWeapon(in_weapon, FVector::ZeroVector, FRotator::ZeroRotator);

		//Disable the weapons collision component, attach to character and set to invisible, then add it to the inventory
		if (spawnedWeapon != NULL)
		{
			spawnedWeapon->getBoxCollisionComponent()->UnregisterComponent();
			spawnedWeapon->AttachRootComponentTo(GetMesh(), inventoryWeaponSocket, EAttachLocation::SnapToTarget);
			spawnedWeapon->SetActorHiddenInGame(true);
			addWeapon(spawnedWeapon);
		}
	}
}

void AFPSGCharacter::addExtraHealth(float extraHealth)
{
	// debug
	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red, "Player had " + FString::FromInt(currentHealth) + " health before powerup");
	}

	float newHealth = currentHealth + extraHealth;
	newHealth = FMath::Min(newHealth, maxHealth);
	setCurrentHealth(newHealth);

	// debug
	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red, "Player has " + FString::FromInt(currentHealth) + " health after powerup");
	}
}

void AFPSGCharacter::initProperties()
{
	//maxGrenades is set in the blueprint class in the editor
	setAmountOfGrenades(maxGrenades);
}

bool AFPSGCharacter::haveGrenades() const
{
	return (amountOfGrenades > 0);
}

void AFPSGCharacter::useGrenade()
{
	//First, we use grenade locally
	--amountOfGrenades;

	// Then, the server uses the grenade
	if (Role < ROLE_Authority)
	{
		serverUseGrenade();
	}
}

AFPSGWeapon* AFPSGCharacter::getCurrentWeapon() const
{
	return currentWeapon;
}

float AFPSGCharacter::getMaxHealth() const
{
	return maxHealth;
}

float AFPSGCharacter::getCurrentHealth() const
{
	return currentHealth;
}

int32 AFPSGCharacter::getAmountOfGrenades() const
{
	return amountOfGrenades;
}

USkeletalMeshComponent* AFPSGCharacter::getFirstPersonMesh() const
{
	return firstPersonMesh;
}

FName AFPSGCharacter::getBoneHead() const
{
	return boneHead;
}

void AFPSGCharacter::setCurrentHealth(float newHealth)
{
	currentHealth = newHealth;
}

void AFPSGCharacter::setAmountOfGrenades(int32 inAmountOfGrenades)
{
	amountOfGrenades = inAmountOfGrenades;

	if (amountOfGrenades > maxGrenades)
	{
		amountOfGrenades = maxGrenades;
	}
}

void AFPSGCharacter::moveForward(float moveValue)
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	if (Controller != NULL && moveValue != 0.0f)
	{
		//Retrieve the viewing/aiming direction of the controlled pawn
		FRotator rotation = Controller->GetControlRotation();

		//We want to limit the moving to the XY plane
		if (GetCharacterMovement()->IsMovingOnGround() || GetCharacterMovement()->IsFalling())
		{
			rotation.Pitch = 0.0f;
		}

		//Move the character
		const FVector moveDirection = FRotationMatrix(rotation).GetScaledAxis(EAxis::X);
		AddMovementInput(moveDirection, moveValue);

		if (moveValue == 1.0f)
		{
			if (movementDirection != EMoveDirection::FORWARD)
			{
				serverUpdateMovementDirection(static_cast<uint8>(EMoveDirection::FORWARD));
			}
		}
		if (moveValue == -1.0f)
		{
			if (movementDirection != EMoveDirection::BACKWARD)
			{
				serverUpdateMovementDirection(static_cast<uint8>(EMoveDirection::BACKWARD));
			}
		}
	}
}

void AFPSGCharacter::moveRight(float moveValue)
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	if (Controller != NULL && moveValue != 0.0f)
	{
		//Retrieve the viewing/aiming direction of the controlled pawn
		FRotator rotation = Controller->GetControlRotation();

		//Move the character
		const FVector moveDirection = FRotationMatrix(rotation).GetScaledAxis(EAxis::Y);
		AddMovementInput(moveDirection, moveValue);

		if (moveValue == 1.0f)
		{
			if (movementDirection != EMoveDirection::RIGHT)
			{
				serverUpdateMovementDirection(static_cast<uint8>(EMoveDirection::RIGHT));
			}
		}
		if (moveValue == -1.0f)
		{
			if (movementDirection != EMoveDirection::LEFT)
			{
				serverUpdateMovementDirection(static_cast<uint8>(EMoveDirection::LEFT));
			}
		}
	}
}

void AFPSGCharacter::startJump()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	bPressedJump = true;
}

void AFPSGCharacter::stopJump()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	bPressedJump = false;
}

void AFPSGCharacter::startWeaponFire()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("AFPSGCharacter::startFire"));

	if (currentWeapon != NULL && !isFiring)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("AFPSGCharacter::startFire"));
		isFiring = true;
		currentWeapon->startFire();
	}
}

void AFPSGCharacter::stopWeaponFire()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("AFPSGCharacter::stopFire"));

	if (currentWeapon != NULL && isFiring)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Red, TEXT("AFPSGCharacter::stopFire2"));
		isFiring = false;
		currentWeapon->stopFire();
	}
}

void AFPSGCharacter::startThrowGrenade()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;
	
	//GEngine->AddOnScreenDebugMessage(-1, 4.0f, FColor::Yellow, TEXT("AFPSGCharacter::startThrowGrenade"));

	if (haveGrenades())
	{
		throwGrenade();
	}
}

void AFPSGCharacter::throwGrenade()
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("AFPSGCharacter::throwGrenade"));

	// Get the spawn location and rotation (These are sent to the server, all projectiles use the same)
	const FVector spawnLocation = firstPersonCameraComponent->GetComponentLocation();
	const FRotator spawnRotation = firstPersonCameraComponent->RelativeRotation;

	// Get the camera direction
	const FVector lookCameraDirection = firstPersonCameraComponent->GetForwardVector();

	// Call server function to throw the grenade
	serverThrowGrenade(spawnLocation, spawnRotation, lookCameraDirection);

	// Decrement the amount of grenades by 1
	useGrenade();
}


void AFPSGCharacter::equipWeaponOne()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	if (weaponInventory.IsValidIndex(0))
	{
		serverEquipWeapon(0);
	}
}

void AFPSGCharacter::equipWeaponTwo()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	if (weaponInventory.IsValidIndex(1))
	{
		serverEquipWeapon(1);
	}
}

void AFPSGCharacter::equipWeaponThree()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	if (weaponInventory.IsValidIndex(2))
	{
		serverEquipWeapon(2);
	}
}

void AFPSGCharacter::equipWeaponFour()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	if (weaponInventory.IsValidIndex(3))
	{
		serverEquipWeapon(3);
	}
}

void AFPSGCharacter::equipWeaponFive()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	if (weaponInventory.IsValidIndex(4))
	{
		serverEquipWeapon(4);
	}
}

void AFPSGCharacter::equipWeaponSix()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	if (weaponInventory.IsValidIndex(5))
	{
		serverEquipWeapon(5);
	}
}

void AFPSGCharacter::equipWeaponSeven()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	if (weaponInventory.IsValidIndex(6))
	{
		serverEquipWeapon(6);
	}
}

void AFPSGCharacter::equipWeaponEight()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	if (weaponInventory.IsValidIndex(7))
	{
		serverEquipWeapon(7);
	}
}

void AFPSGCharacter::equipWeaponNine()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	if (weaponInventory.IsValidIndex(8))
	{
		serverEquipWeapon(8);
	}
}

void AFPSGCharacter::equipPreviousWeapon()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	int32 tempIndex = findWeaponIndex(currentWeapon);

	if (tempIndex == INDEX_NONE)
	{
		if (weaponInventory.IsValidIndex(0))
		{
			serverEquipWeapon(0);
		}
	}
	else
	{
		--tempIndex;

		if (weaponInventory.IsValidIndex(tempIndex))
		{
			serverEquipWeapon(tempIndex);
		}
	}
}

void AFPSGCharacter::equipNextWeapon()
{
	//Dont execute this input if the in game menu is open
	if (isInGameMenuOpen()) return;

	int32 tempIndex = findWeaponIndex(currentWeapon);

	if (tempIndex == INDEX_NONE)
	{
		if (weaponInventory.IsValidIndex(0))
		{
			serverEquipWeapon(0);
		}
	}
	else
	{
		++tempIndex;

		if (weaponInventory.IsValidIndex(tempIndex))
		{
			serverEquipWeapon(tempIndex);
		}
	}
}

void AFPSGCharacter::onDeathAnimation()
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("AFPSGCharacter::onDeathAnimation"));

	//Initialize physics simulation
	if (GetMesh() != NULL && GetMesh()->GetPhysicsAsset() != NULL)
	{
		GetMesh()->SetAllBodiesSimulatePhysics(true);
		GetMesh()->SetSimulatePhysics(true);
		GetMesh()->WakeAllRigidBodies();
		GetMesh()->bBlendPhysics = true;
	}

	//Disable character movement
	if (GetCharacterMovement() != NULL)
	{
		GetCharacterMovement()->StopMovementImmediately();
		GetCharacterMovement()->DisableMovement();
		GetCharacterMovement()->SetComponentTickEnabled(false);
	}

	//Calls Destroy after 10 seconds
	SetLifeSpan(10.0f);
}

void AFPSGCharacter::destroyInventory()
{
	//Only server should destroy the inventory
	if (Role == ROLE_Authority)
	{
		for (int32 i = weaponInventory.Num() - 1; i >= 0; --i)
		{
			if (weaponInventory.IsValidIndex(i))
			{
				AFPSGWeapon* weapon = weaponInventory[i];
				if (weapon != NULL)
				{
					removeWeapon(weapon);
					weapon->Destroy();
				}
			}
		}
	}
}

bool AFPSGCharacter::isInInventory(AFPSGWeapon* in_weapon)
{
	bool weaponFound = false;

	if (in_weapon == NULL)
	{
		return weaponFound;
	}

	for (const AFPSGWeapon* weapon : weaponInventory)
	{
		if (weapon != NULL)
		{
			if (*in_weapon == *weapon)
			{
				weaponFound = true;
				break;
			}
		}
	}

	return weaponFound;
}

AFPSGWeapon* AFPSGCharacter::findWeapon(AFPSGWeapon* in_weapon)
{
	if (in_weapon == NULL)
	{
		return NULL;
	}

	for (AFPSGWeapon* weapon : weaponInventory)
	{
		if (weapon != NULL)
		{
			if (*in_weapon == *weapon)
			{
				return weapon;
			}
		}
	}

	return NULL;
}

int32 AFPSGCharacter::findWeaponIndex(AFPSGWeapon* in_weapon)
{
	int32 weaponIndex = INDEX_NONE;

	if (in_weapon == NULL)
	{
		return weaponIndex;
	}

	for (int32 i = 0; i < weaponInventory.Num(); ++i)
	{
		if (weaponInventory.IsValidIndex(i))
		{
			if (weaponInventory[i] != NULL)
			{
				if (*in_weapon == *weaponInventory[i])
				{
					weaponIndex = i;
					break;
				}
			}
		}
	}

	return weaponIndex;
}

void AFPSGCharacter::addWeapon(AFPSGWeapon* in_weapon)
{
	//Only server should add the weapon
	if (Role == ROLE_Authority)
	{
		if (in_weapon != NULL)
		{
			//Add the weapon and make sure it has the correct amount of ammunition
			int32 addedWeaponIndex = weaponInventory.AddUnique(in_weapon);
		}
	}
}

void AFPSGCharacter::removeWeapon(AFPSGWeapon* in_weapon)
{
	//Only server should remove the weapon
	if (Role == ROLE_Authority)
	{
		if (in_weapon != NULL)
		{
			weaponInventory.RemoveSingle(in_weapon);
		}
	}
}

AFPSGWeapon* AFPSGCharacter::spawnWeapon(AFPSGWeapon* in_weapon, const FVector& location, const FRotator& rotation)
{
	if (Role == ROLE_Authority && in_weapon != NULL)
	{
		UWorld* world = GetWorld();
		if (world != NULL)
		{
			FActorSpawnParameters spawnParameters;
			spawnParameters.Owner = this;
			spawnParameters.Instigator = this;
			spawnParameters.bNoCollisionFail = true;

			AFPSGWeapon* spawnedWeapon = world->SpawnActor<AFPSGWeapon>(in_weapon->GetClass(), location, rotation, spawnParameters);
			spawnedWeapon->setMyOwner(Cast<AFPSGCharacter>(spawnedWeapon->GetOwner()));

			return spawnedWeapon;
		}
	}
	return NULL;
}

void AFPSGCharacter::die(AFPSGPlayerController* killer)
{
	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("AFPSGCharacter::die"));
	}

	//Destroy the player inventory
	destroyInventory();

	AFPSGPlayerController* myPlayerController = Cast<AFPSGPlayerController>(GetController());

	if (myPlayerController != NULL)
	{
		//Set to true, and when replicated to clients, will call the callback function onRep_DieOwnerOnly in player controller
		myPlayerController->setDieCallback(true);

		//We are the server, but just to be sure
		if (Role == ROLE_Authority)
		{
			//Server have to manually call the onRep_DieOwnerOnly callback
			myPlayerController->onRep_DieOwnerOnly();

			AFPSGGameMode* gameMode = GetWorld() ? GetWorld()->GetAuthGameMode<AFPSGGameMode>() : NULL;

			if (gameMode != NULL)
			{
				gameMode->notifyKill(killer, myPlayerController);
			}
		}
	}

	//Set to true, and when replicated to clients, will call the callback function onRep_DieAll
	dieCallback = true;

	//We are the server, but just to be sure
	if (Role == ROLE_Authority)
	{
		//Server have to manually call the onRep_DieAll callback
		onRep_DieAll();
	}

	//Calls our controller PawnPendingDestroy function
	DetachFromControllerPendingDestroy();
}

bool AFPSGCharacter::isInGameMenuOpen() const
{
	//Dont execute this input if the in game menu is open
	AFPSGPlayerController * myController = Cast<AFPSGPlayerController>(Controller);
	if (myController != NULL)
	{
		return myController->isInGameMenuOpen();
	}
	return false;
}

void AFPSGCharacter::onRep_DieAll()
{
	//GEngine->AddOnScreenDebugMessage(-1, 5.0f, FColor::Magenta, "AFPSGCharacter::onRep_Die");

	if (dieCallback)
	{
		bReplicateMovement = false;
		bTearOff = true;

		//Disable capsule collision
		if (GetCapsuleComponent() != NULL)
		{
			GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);
			GetCapsuleComponent()->SetCollisionResponseToAllChannels(ECR_Ignore);
		}

		//TODO Should stop all animation montages here
		//In case any is currently being played (For eksample weapon fire, reload etc)
		//Currently only have a die animation montage, so dont need to do this

		//Play the death animation
		float deathAnimationLength = PlayAnimMontage(deathAnimation, 1.0f, NAME_None);

		//If the death animation was successfully played
		if (deathAnimationLength > 0.0f)
		{
			FTimerHandle onDeathTimerHandle;
			GetWorldTimerManager().SetTimer(onDeathTimerHandle, this, &AFPSGCharacter::onDeathAnimation, FMath::Min(0.1f, deathAnimationLength), false);
			//GetWorldTimerManager().SetTimer(onDeathTimerHandle, this, &AFPSGCharacter::onDeathAnimation, deathAnimationLength / 2.0f, false);
		}
		else
		{
			onDeathAnimation();
		}
	}
}

void AFPSGCharacter::serverEquipWeapon_Implementation(int32 in_currentWeaponIndex)
{
	if (weaponInventory[in_currentWeaponIndex] != NULL)
	{
		//Detach previous weapon
		if (currentWeapon != NULL)
		{
			//If previous weapon is attached to firstPersonMesh, then detach it and attach it to the inventory socket
			if (currentWeapon->IsAttachedTo(firstPersonMesh->GetOwner()))
			{
				currentWeapon->DetachRootComponentFromParent(true);
				currentWeapon->AttachRootComponentTo(GetMesh(), inventoryWeaponSocket, EAttachLocation::SnapToTarget);
				currentWeapon->SetActorHiddenInGame(true);
			}
		}

		//Retrieve the new weapon from the inventory
		currentWeapon = weaponInventory[in_currentWeaponIndex];

		//Attach the new weapon
		if (currentWeapon != NULL)
		{
			//If current weapon is attached to the inventory socket, detach it and then attach it to the equipped weapon socket
			if (currentWeapon->IsAttachedTo(GetMesh()->GetOwner()))
			{
				currentWeapon->DetachRootComponentFromParent(true);
				currentWeapon->AttachRootComponentTo(firstPersonMesh, currentWeapon->getEquipAtSocket(), EAttachLocation::SnapToTarget);
				currentWeapon->SetActorHiddenInGame(false);
			}
		}
	}
}

bool AFPSGCharacter::serverEquipWeapon_Validate(int32 in_currentWeaponIndex)
{
	return true;
}

void AFPSGCharacter::serverUpdateMovementDirection_Implementation(uint8 in_movementDirection)
{
	movementDirection = static_cast<EMoveDirection>(in_movementDirection);
}

bool AFPSGCharacter::serverUpdateMovementDirection_Validate(uint8 in_movementDirection)
{
	return true;
}

void AFPSGCharacter::serverThrowGrenade_Implementation(const FVector& spawnLocation, const FRotator& spawnRotation, const FVector& launchDirection)
{
	//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("AFPSGCharacter::serverThrowGrenade_Implementation"));

	UWorld* world = GetWorld();

	if (grenade != NULL && world != NULL)
	{
		//Set up spawn parameters and spawn the projectile
		FActorSpawnParameters spawnParameters;
		spawnParameters.Owner = this;
		spawnParameters.Instigator = this;

		AFPSGProjectile* spawnedProjectile = world->SpawnActor<AFPSGProjectile>(grenade, spawnLocation, spawnRotation, spawnParameters);

		//Initialize its controller owner and velocity
		if (spawnedProjectile != NULL)
		{
			spawnedProjectile->setMyOwnerController(Cast<AFPSGPlayerController>(Controller));

			FVector direction = launchDirection;
			spawnedProjectile->initVelocity(&direction);
		}
	}
}

bool AFPSGCharacter::serverThrowGrenade_Validate(const FVector& spawnLocation, const FRotator& spawnRotation, const FVector& launchDirection)
{
	return true;
}

void AFPSGCharacter::serverUseGrenade_Implementation()
{
	useGrenade();
}

bool AFPSGCharacter::serverUseGrenade_Validate()
{
	return true;
}
