// FPSGame created by Gard and Gorm

#pragma once

#include "GameFramework/Actor.h"

#include "FPSGPickup.h"

#include "FPSGWeapon.generated.h"

UCLASS()
class FPSGAME_API AFPSGWeapon : public AFPSGPickup
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AFPSGWeapon();
	virtual ~AFPSGWeapon();
	
	bool operator==(const AFPSGWeapon& weapon) const;
	
	virtual void GetLifetimeReplicatedProps(TArray< FLifetimeProperty > & OutLifetimeProps) const override;

	// Called before gameplay begins
	virtual void PreInitializeComponents() override;

	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	virtual void EndPlay(const EEndPlayReason::Type endPlayReason) override;

	//Initialize values here based on the property values set in the editor
	virtual void PostInitProperties() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	// Called every frame
	virtual void Tick(float DeltaSeconds) override;

	/**
	* Fire the weapon
	*/
	virtual void fire();

	/**
	* Start firing of a weapon
	* @return true if it successfully started firing
	*/
	virtual bool startFire();
	
	/**
	* Stop firing of a weapon
	* @return true if it successfully stopped firing
	*/
	virtual bool stopFire();

	//Get functions
	UBoxComponent* getBoxCollisionComponent() const;
	FString getWeaponName() const;
	const int32 getMaxAmmunition() const;
	float getTimeBetweenShots() const;
	float getRange() const;
	float getNumberOfShots() const;
	int32 getCurrentAmmunition() const;
	UParticleSystem* getMuzzleEffect() const;
	FName getEquipAtSocket() const;

	//Set functions
	void setCurrentAmmunition(int32 inCurrentAmmunition);
	void setMyOwner(class AFPSGCharacter* inOwner);

	/*
	* Class variables exposed to blueprints
	*/
	//Max amount of ammunition this weapon can have
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = WeaponConfig)
	int32 maxAmmunition;

	//How much ammunition this weapon currently have
	UPROPERTY(VisibleDefaultsOnly, Replicated, Transient, BlueprintReadOnly, Category = WeaponConfig)
	int32 currentAmmunition;

protected:
	/**
	* Function to do something determined by this AFPSGWeapon actor when picked up
	* @param otherActor The actor that picked up the AFPSGWeapon
	*/
	virtual void onPickedUp(class AActor* otherActor) override;

	/**
	* Function to initialize properties based on the properties that are set in the editor
	*/
	virtual void initProperties();

	/**
	* Checks if there are any ammunition left
	*/
	bool haveAmmunition() const;

	/**
	* Decrement ammunition by one, first on local client,
	* then calls serverUseAmmunition to do the same on the servers version
	*/
	void useAmmunition();

	/**
	* Handles all visual aspects of firing (effects, ADD Later: animations, sounds etc). Is called on all connections
	*/
	void simulateFire();

	UFUNCTION()
	void onRep_fireCounter();

	UFUNCTION(Server, Reliable, WithValidation)
	void serverFire();
	void serverFire_Implementation();
	bool serverFire_Validate();

	UFUNCTION(Server, Reliable, WithValidation)
	void serverStopFire();
	void serverStopFire_Implementation();
	bool serverStopFire_Validate();

	UENUM(BlueprintType)
	enum class EWeaponState : uint8
	{
		IDLE,
		FIRING
	};

	//Stores the current fire state
	EWeaponState firingState;

	//The timer handle for single fire weapons to only trigger shooting at specific intervals
	FTimerHandle singleFireTimerHandle;

	//The timer handle for multi fire weapons to perform continuous firing
	FTimerHandle multiFireTimerHandle;

	//The collision component
	UPROPERTY(VisibleAnywhere, Category = Collision)
	//Created on the server in PreInitializeComponents. Will therefore point to NULL on clients
	UBoxComponent* boxCollisionComponent;

	//The skeletal mesh component
	UPROPERTY(VisibleAnywhere, Category = Config)
	USkeletalMeshComponent* weaponMeshComponent;

	//Name of this weapon
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
	FString weaponName;

	//The time you have to wait before you can shoot again (in seconds)
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
	float timeBetweenShots;
	
	//The range of this weapon
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
	float range;

	//How many shots this weapon fires each time
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
	float numberOfShots;

	//If true, fire by holding down the fire button
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
	bool holdToFire;

	//set to true to debug weapon fire (USED ONLY FOR DEBUG)
	UPROPERTY(EditDefaultsOnly, Category = WeaponConfig)
	bool fireDebug;

	UPROPERTY(EditDefaultsOnly, Category = SocketNames)
	FName muzzleSocket;
	
	//The particle effect that gets played when shooting
	UPROPERTY(EditDefaultsOnly, Category = Effects)
	UParticleSystem* muzzleEffect;

	//The name of the socket where this weapon is attached when equipped
	UPROPERTY(EditDefaultsOnly, Category = SocketNames)
	FName equipAtSocket;

	//The owner of this weapon. (NOTE: This points to the same as the Actors Owner variable, just casted down to AFPSGCharacter)
	UPROPERTY(Replicated, Transient)
	class AFPSGCharacter* myOwner;

	//Incremented each time we fire (By local player and server)
	UPROPERTY(Transient, ReplicatedUsing=onRep_fireCounter)
	int32 fireCounter;

private:
	/**
	* Play effects related to a weapon
	*/
	void playEffects();

	UFUNCTION(Server, Reliable, WithValidation)
	void serverUseAmmunition();
	void serverUseAmmunition_Implementation();
	bool serverUseAmmunition_Validate();

	//A helper timer for multi fire weapons to decide when to invoke the multiFireTimerHandle
	FTimerHandle invokeMultiFireTimerHandle;
};
