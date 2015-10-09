// FPSGame created by Gard and Gorm

#pragma once

#include "GameFramework/HUD.h"
#include "FPSGHUD.generated.h"

/**
 * 
 */

UCLASS()
class FPSGAME_API AFPSGHUD : public AHUD
{
	GENERATED_BODY()

public:
	AFPSGHUD();
	
	// Called every frame to draw the HUD on screen
	virtual void DrawHUD() override;
	
private:
	// Function to draw the crosshair on screen
	void drawCrosshair();

	/**
	* Function to draw the healthbar on screen
	*/
	//void drawHealthbar();

	/**
	* Function to draw the ammo on screen
	*/
	//void drawAmmo();

	/**
	* Function to draw the respawn timer on screen
	*/
	//void drawRespawntimer();

	/**
	* Function to draw the amount of grenades on screen
	*/
	//void drawGrenades();

	// The crosshair texture	
	UTexture2D* crosshairTexture;
	// The healthbar texture
	//UTexture2D* healthbarTexture;

	// The font used for drawing the ammo text
	//UFont* ammoFont;
};
