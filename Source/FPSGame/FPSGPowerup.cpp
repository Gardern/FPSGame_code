// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGPowerup.h"

#include"Engine.h"

#include "FPSGCharacter.h"
#include "FPSGGameMode.h"

// Sets default values
AFPSGPowerup::AFPSGPowerup()
	: Super()
{
	collisionComp = CreateDefaultSubobject<USphereComponent>(TEXT("FPSGPowerupCollision"));
	collisionComp->InitSphereRadius(60.0f);
	collisionComp->OnComponentBeginOverlap.AddDynamic(this, &AFPSGPowerup::onOverlap);
	collisionComp->bGenerateOverlapEvents = true;
	RootComponent = collisionComp;

	powerupMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("FPSGPowerupMesh"));
	powerupMeshComponent->AttachTo(RootComponent);

	updateAmount = 0.0f;
}

AFPSGPowerup::~AFPSGPowerup()
{

}

// Called when the game starts or when spawned
void AFPSGPowerup::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void AFPSGPowerup::Tick( float DeltaTime )
{
	Super::Tick( DeltaTime );
}

void AFPSGPowerup::onPickedUp(class AActor* otherActor)
{
	//GEngine->AddOnScreenDebugMessage(-1, 6.0f, FColor::Red, TEXT("AFPSGPowerup::onPickedUp"));

	// debug
	if (GEngine != NULL)
	{
		//GEngine->AddOnScreenDebugMessage(-1, 8.0f, FColor::Red, "Player picked up powerup with " + FString::FromInt(updateAmount) + " extra health");
	}

	// Get the player that picked up the powerup
	AFPSGCharacter* player = Cast<AFPSGCharacter>(otherActor);

	if (player != NULL)
	{
		player->addExtraHealth(updateAmount);
	}
}
