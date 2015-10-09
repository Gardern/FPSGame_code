// FPSGame created by Gard and Gorm

#pragma once

#include "FPSGWeapon.h"
#include "GameFramework/Character.h"
#include "Net/UnrealNetwork.h"

#include "FPSGPlayerController.h"
#include "FPSGProjectile.h"

#include "FPSGCharacter.generated.h"

UCLASS(Blueprintable)
class FPSGAME_API AFPSGCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	AFPSGCharacter();

	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	// Called after PreInitializeComponents and before BeginPlay
	virtual void PostInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	// Called when this Pawn is possessed. Only called on the server
	virtual void PossessedBy(AController * NewController) override;

	//Initialize values here based on the property values set in the editor
	virtual void PostInitProperties() override;

	// Called when this actor is about to be destroyed
	virtual void Destroyed() override;
	
	// Called every frame
	virtual void Tick( float DeltaSeconds ) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* InputComponent) override;

	virtual float TakeDamage(float damageAmount, struct FDamageEvent const& damageEvent, class AController* eventInstigator, class AActor* damageCauser) override;

	/**
	* Function called when the character falls out of the level (below variable "Kill Z" set in "World settings")
	* @param dmgType The type of damage received
	*/
	virtual void FellOutOfWorld(const class UDamageType & dmgType) override;

	/**
	* Handle pickups of a weapon
	* @param in_weapon The weapon we picked up. Either add this weapon to the inventory or take its ammunition //TODO Server ONLY?
	*/
	void manageWeapon(AFPSGWeapon* in_weapon);

	/**
	* Function to add extra health to this ACharacter
	*/
	void addExtraHealth(float extraHealth);

	/**
	* Function to initialize properties based on the properties that are set in the editor
	*/
	void initProperties();

	/**
	* Checks if there are any grenades left
	*/
	bool haveGrenades() const;

	/**
	* Decrement gerandes by one, first on local client,
	* then calls serverUseGrenade to do the same on the servers version
	*/
	void useGrenade();

	//Get functions
	AFPSGWeapon* getCurrentWeapon() const;
	float getMaxHealth() const;
	float getCurrentHealth() const;
	int32 getAmountOfGrenades() const;
	USkeletalMeshComponent* getFirstPersonMesh() const;
	FName getBoneHead() const;

	//Set functions
	void setCurrentHealth(float newHealth);
	void setAmountOfGrenades(int32 inAmountOfGrenades);

	/**
	* Different directions the player can move in
	*/
	UENUM(BlueprintType)
	enum class EMoveDirection : uint8
	{
		NONE,
		LEFT,
		RIGHT,
		FORWARD,
		BACKWARD
	};

	//Camera for first person view
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera)
	UCameraComponent* firstPersonCameraComponent;

	/**
	* Class variables exposed to blueprints
	*/
	//Stores which direction we are moving
	UPROPERTY(Replicated, BlueprintReadOnly, Category = Gameplay)
	EMoveDirection movementDirection;

	// The maximum health for this character
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = Health)
	float maxHealth;

	// The current character health
	UPROPERTY(Transient, Replicated, BlueprintReadOnly)
	float currentHealth;

	//Current weapon that is equipped
	UPROPERTY(Replicated, BlueprintReadOnly)
	AFPSGWeapon* currentWeapon;

	//How many grenades the character currently have
	UPROPERTY(Transient, BlueprintReadOnly)
	int32 amountOfGrenades;

private:
	/**
	* Functions that handles input
	*/
	void moveForward(float moveValue);
	void moveRight(float moveValue);
	void startJump();
	void stopJump();
	void startWeaponFire();
	void stopWeaponFire();
	void startThrowGrenade();
	void throwGrenade();
	void equipWeaponOne();
	void equipWeaponTwo();
	void equipWeaponThree();
	void equipWeaponFour();
	void equipWeaponFive();
	void equipWeaponSix();
	void equipWeaponSeven();
	void equipWeaponEight();
	void equipWeaponNine();
	void equipPreviousWeapon();
	void equipNextWeapon();

	/**
	* Called when playing the death animation on death
	* Initializes physics simulation and stops character movement
	*/
	void onDeathAnimation();

	/**
	* Server: Removes all weapons from the inventory and call Destroy on them
	*/
	void destroyInventory();

	/**
	* Checks if weapon in_weapon is in the weapon inventory
	* Uses AFPSGWeapon operator== to search for the weapon
	* @param in_weapon The weapon that we check if the inventory contains
	* @return true if it was found, false otherwise
	*/
	bool isInInventory(AFPSGWeapon* in_weapon);

	/**
	* Finds weapon in_weapon in the weapon inventory.
	* Uses AFPSGWeapon operator== to search for the weapon
	* @param in_weapon The weapon to find in the inventory
	* @return a pointer to the found weapon or NULL if it wasnt found
	*/
	AFPSGWeapon* findWeapon(AFPSGWeapon* in_weapon);

	/**
	* Finds the inventory index of weapon in_weapon
	* Uses AFPSGWeapon operator== to search for the weapon
	* @param in_weapon Find the index of this weapon
	* @return the index or INDEX_NONE if it wasnt found
	*/
	int32 findWeaponIndex(AFPSGWeapon* in_weapon);

	/**
	* Server: Adds weapon in_weapon to the weapon inventory.
	* @param in_weapon The weapon to add to the inventory
	*/
	void addWeapon(AFPSGWeapon* in_weapon);

	/**
	* Server: Removes weapon in_weapon from the weapon inventory.
	* @param in_weapon The weapon to remove from the inventory
	*/
	void removeWeapon(AFPSGWeapon* in_weapon);

	/**
	* Server: Spawns a new instance of weapon in_weapon.
	* @param in_weapon The weapon to spawn a new instance of
	* @param location The location to spawn the new weapon
	* @param rotation The rotation for the spawned weapon
	* @return a pointer to the spawned weapon
	*/
	AFPSGWeapon* spawnWeapon(AFPSGWeapon* in_weapon, const FVector& location, const FRotator& rotation);

	/**
	* Function called when the character dies
	* @param killer The player that killed this player
	*/
	void die(AFPSGPlayerController* killer);

	/**
	* Checks if the in game menu is open
	* @return true if the in game menu is open, false otherwise
	*/
	bool isInGameMenuOpen() const;

	//TODO Comment
	UFUNCTION()
	void onRep_DieAll();

	/**
	* Server function that handles unequip/equip of a new weapon from the inventory
	* in_currentWeaponIndex: Equips inventory weapon at this index. 
	*/
	UFUNCTION(Server, Reliable, WithValidation)
	void serverEquipWeapon(int32 in_currentWeaponIndex);
	void serverEquipWeapon_Implementation(int32 in_currentWeaponIndex);
	bool serverEquipWeapon_Validate(int32 in_currentWeaponIndex);

	/**
	* Server function that updates movement direction on the server
	* in_movementDirection: The new movement direction for the character (NONE, LEFT, RIGHT, FORWARD, BACKWARD)
	*/
	UFUNCTION(Server, Unreliable, WithValidation)
	void serverUpdateMovementDirection(uint8 in_movementDirection);
	void serverUpdateMovementDirection_Implementation(uint8 in_movementDirection);
	bool serverUpdateMovementDirection_Validate(uint8 in_movementDirection);

	/**
	* Server side function that spawns the grenade and initialize its velocity
	* @param spawnLocation The location to spawn the projectile
	* @param spawnRotation The rotation for the spawned projectile
	* @param launchDirection The direction to launch the grenade
	*/
	UFUNCTION(Server, Reliable, WithValidation)
	void serverThrowGrenade(const FVector& spawnLocation, const FRotator& spawnRotation, const FVector& launchDirection);
	void serverThrowGrenade_Implementation(const FVector& spawnLocation, const FRotator& spawnRotation, const FVector& launchDirection);
	bool serverThrowGrenade_Validate(const FVector& spawnLocation, const FRotator& spawnRotation, const FVector& launchDirection);

	/**
	* Function to decrement a grenade when used
	*/
	UFUNCTION(Server, Reliable, WithValidation)
	void serverUseGrenade();
	void serverUseGrenade_Implementation();
	bool serverUseGrenade_Validate();

	//true if we are currently firing our weapon
	bool isFiring;
	//true if FellOutOfWorld is called
	bool isFalling;

	//Max amount of grenades a character can have
	UPROPERTY(EditDefaultsOnly, Category = GrenadeData)
	int32 maxGrenades;

	//The first person view mesh, only visible to the player in first person
	UPROPERTY(VisibleDefaultsOnly, Category = Mesh)
	USkeletalMeshComponent* firstPersonMesh;

	//The animation that is played when the player dies
	UPROPERTY(EditDefaultsOnly, Category = Animation)
	UAnimMontage* deathAnimation;

	//The name of the socket where inventory weapons are attached
	UPROPERTY(EditDefaultsOnly, Category = SocketNames)
	FName inventoryWeaponSocket;

	//The name of the head bone in the characters physics asset
	UPROPERTY(EditDefaultsOnly, Category = BoneNames)
	FName boneHead;
	
	//Inventory that keeps the weapons
	UPROPERTY(Transient, Replicated)
	TArray<AFPSGWeapon*> weaponInventory;
	//TODO Currently no support for non default allocator with a TArray Property. However, try this in UE version 4.8
	//TArray<TSubclassOf<AFPSGWeapon>, TInlineAllocator<10> > weaponInventory;

	UPROPERTY(EditDefaultsOnly, Category = GrenadeData)
	TSubclassOf<AFPSGProjectile> grenade;

	//Calls the callback function onRep_DieAll on local client, all remote clients and server each time a player dies
	UPROPERTY(ReplicatedUsing = onRep_DieAll)
	bool dieCallback;
};
