// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGHUD.h"
#include "FPSGPlayerController.h"

AFPSGHUD::AFPSGHUD()
	: Super()
{
	// Set the crosshair texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> crosshairTextureObject(TEXT("Texture2D'/Game/FPSGameContent/Assets/HUD/crosshair.crosshair'"));
	if (crosshairTextureObject.Object != NULL)
	{
		crosshairTexture = crosshairTextureObject.Object;
	}

	/*
	// Set the healthbar texture
	static ConstructorHelpers::FObjectFinder<UTexture2D> healthbarTextureObject(TEXT("Texture2D'/Game/FPSGameContent/Assets/HUD/healthbar.healthbar'"));
	if (healthbarTextureObject.Object != NULL)
	{
		healthbarTexture = healthbarTextureObject.Object;
	}

	// Set the ammo text font
	static ConstructorHelpers::FObjectFinder<UFont> ammoFontObject(TEXT("/Engine/EngineFonts/RobotoDistanceField"));
	if (ammoFontObject.Object != NULL)
	{
		ammoFont = ammoFontObject.Object;
	}
	*/
}

void AFPSGHUD::DrawHUD()
{
	Super::DrawHUD();

	// Draw the crosshair
	drawCrosshair();
	// Draw the healthbar
	//drawHealthbar();
	// Draw ammo display
	//drawAmmo();
	// Draw grenades display
	//drawGrenades();
	// Draw respawn timer
	//drawRespawntimer();
}

void AFPSGHUD::drawCrosshair()
{
	AFPSGPlayerController* myPlayerController = Cast<AFPSGPlayerController>(GetOwningPlayerController());

	if (myPlayerController != NULL && myPlayerController->getIsAlive())
	{
		if (Canvas != NULL)
		{
			//We retrieve the center of the canvas
			const FVector2D center(Canvas->ClipX * 0.5f, Canvas->ClipY * 0.5f);

			//The center of the texture needs to align with the center of the canvas
			const FVector2D drawPosition((center.X - (crosshairTexture->GetSurfaceWidth() * 0.5)),
				(center.Y - (crosshairTexture->GetSurfaceHeight() * 0.5f)));

			//Now, we draw the crosshair
			FCanvasTileItem tileItem(drawPosition, crosshairTexture->Resource, FLinearColor::White);
			tileItem.BlendMode = SE_BLEND_Translucent;

			Canvas->DrawItem(tileItem);
		}
	}
}

/*
void AFPSGHUD::drawHealthbar()
{
	AFPSGPlayerController* myPlayerController = Cast<AFPSGPlayerController>(GetOwningPlayerController());

	if (myPlayerController != NULL && myPlayerController->getIsAlive())
	{
		int32 resX = Canvas->SizeX;
		int32 resY = Canvas->SizeY;

		float scaleUI = Canvas->ClipY / Canvas->ClipX;

		//Get the player so we can get the players health
		AFPSGCharacter* player = Cast<AFPSGCharacter>(myPlayerController->GetPawn());

		float health = 0.0f;

		if (player != NULL)
		{
			health = player->getCurrentHealth();
		}

		// Increase healthbar according to health
		float length = health * resX * 0.001f;

		FCanvasIcon healthbarIcon = UCanvas::MakeIcon(healthbarTexture, 0, 0, length, 20);
		
		// Draw the healthar
		Canvas->DrawIcon(healthbarIcon, (Canvas->SizeX) / 2.5, 20, scaleUI);
	}
}
*/

/*
void AFPSGHUD::drawAmmo()
{
	AFPSGPlayerController* myPlayerController = Cast<AFPSGPlayerController>(GetOwningPlayerController());

	if (myPlayerController != NULL && myPlayerController->getIsAlive())
	{
		int32 resX = Canvas->SizeX;
		int32 resY = Canvas->SizeY;

		//Get the player so we can get the players ammo
		AFPSGCharacter* player = Cast<AFPSGCharacter>(myPlayerController->GetPawn());

		FVector2D position = FVector2D(resX * 0.38f, resY * 0.42f);
		FVector2D scale = FVector2D(resX * 0.0008f, resY * 0.002f);
		int32 currentAmmo = 0.0f;
		int32 maxAmmo = 0.0f;

		if (player != NULL)
		{
			AFPSGWeapon* weapon = player->getCurrentWeapon();

			if (weapon != NULL)
			{
				currentAmmo = weapon->getCurrentAmmunition();
				maxAmmo = weapon->getMaxAmmunition();
			}
		}
		FString ammoText = FString::Printf(TEXT("Ammo: %d / %d"), currentAmmo, maxAmmo);

		// Draw the ammo on screen
		DrawText(ammoText, position, ammoFont, scale, FColor::Black);
	}
}

void AFPSGHUD::drawRespawntimer()
{
	//Get the player controller for this player
	AFPSGPlayerController* player = Cast<AFPSGPlayerController>(GetOwningPlayerController());

	if (player != NULL && !player->getIsAlive())
	{
		//GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Black, TEXT("drawRespawntimer player != NULL"));

		int32 resX = Canvas->SizeX;
		int32 resY = Canvas->SizeY;

		FVector2D position = FVector2D(resX * 0.002f, resY * 0.3f);
		FVector2D scale = FVector2D(resX * 0.002f, resY * 0.003f);

		FString respawnTimer = FString::Printf(TEXT("Respawning in: %f"), player->timeUntilRespawn());

		// Draw the respawn timer on screen
		DrawText(respawnTimer, position, ammoFont, scale, FColor::Black);
	}
}

void AFPSGHUD::drawGrenades()
{
	int32 resX = Canvas->SizeX;
	int32 resY = Canvas->SizeY;

	// Get the player so we can get the players grenades
	AFPSGCharacter* player = Cast<AFPSGCharacter>(GetOwningPawn());

	FVector2D position = FVector2D(resX * 0.38, resY * 0.30);
	FVector2D scale = FVector2D(resX * 0.0008, resY * 0.002);
	int32 amountOfGrenades = 0.0f;

	if (player != NULL)
	{
		amountOfGrenades = player->getAmountOfGrenades();
	}
	FString grenadesText = FString::Printf(TEXT("Grenades: %d"), amountOfGrenades);

	// Draw the grenades on screen
	DrawText(grenadesText, position, ammoFont, scale, FColor::Black);
}
*/