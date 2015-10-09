// FPSGame created by Gard and Gorm

#include "FPSGame.h"
#include "FPSGUtility.h"

FPSGUtility::FPSGUtility()
{
}

FPSGUtility::~FPSGUtility()
{
}

bool FPSGUtility::rayTrace(AActor* ignoreActor, UWorld* world, 
	const FVector& traceFromLocation, const FVector& traceToLocation, FHitResult& hitResult)
{
	bool rayTraceHit = false;

	if (ignoreActor != NULL)
	{
		static FName rayTraceTag = FName(TEXT("GeneralRayTrace"));

		//Sets parameters for the trace
		FCollisionQueryParams traceParameters(rayTraceTag, false, ignoreActor);
		traceParameters.bTraceAsyncScene = true;
		traceParameters.bReturnPhysicalMaterial = true;
		traceParameters.AddIgnoredActor(ignoreActor);

		hitResult = FHitResult(ForceInit);

		if (world != NULL)
		{
			//Performs the actual ray/line trace
			rayTraceHit = world->LineTraceSingleByChannel(hitResult, traceFromLocation, traceToLocation, COLLISION_INSTANT_HIT, traceParameters);
		}
	}

	return rayTraceHit;
}

bool FPSGUtility::rayTrace(const TArray<AActor*>& ignoreActors, UWorld* world,
	const FVector& traceFromLocation, const FVector& traceToLocation, FHitResult& hitResult)
{
	bool rayTraceHit = false;
	
	//If there are any actors to ignore
	if (ignoreActors.Num() > 0)
	{
		static FName rayTraceTag = FName(TEXT("GeneralRayTrace"));

		//Sets parameters for the trace
		FCollisionQueryParams traceParameters(rayTraceTag, false, ignoreActors[0]);
		traceParameters.bTraceAsyncScene = true;
		traceParameters.bReturnPhysicalMaterial = true;
		traceParameters.AddIgnoredActors(ignoreActors);
		
		hitResult = FHitResult(ForceInit);
		
		if (world != NULL)
		{
			//Performs the actual ray/line trace
			rayTraceHit = world->LineTraceSingleByChannel(hitResult, traceFromLocation, traceToLocation, COLLISION_INSTANT_HIT, traceParameters);
		}
	}

	return rayTraceHit;
}

bool FPSGUtility::rayTrace(AActor* ignoreActor, UWorld* world,
	const FVector& traceFromLocation, const FVector& traceToLocation, TArray<FHitResult>& hitResults)
{
	bool rayTraceHit = false;

	if (ignoreActor != NULL)
	{
		static FName rayTraceTag = FName(TEXT("GeneralMultiRayTrace"));

		//Sets parameters for the trace
		FCollisionQueryParams traceParameters(rayTraceTag, false, ignoreActor);
		traceParameters.bTraceAsyncScene = true;
		traceParameters.bReturnPhysicalMaterial = true;
		traceParameters.AddIgnoredActor(ignoreActor);

		if (world != NULL)
		{
			//Performs the actual ray/line trace
			rayTraceHit = world->LineTraceMultiByChannel(hitResults, traceFromLocation, traceToLocation, COLLISION_INSTANT_HIT_MULTI, traceParameters);
		}
	}

	return rayTraceHit;
}

bool FPSGUtility::rayTrace(const TArray<AActor*>& ignoreActors, UWorld* world,
	const FVector& traceFromLocation, const FVector& traceToLocation, TArray<FHitResult>& hitResults)
{
	bool rayTraceHit = false;

	//If there are any actors to ignore
	if (ignoreActors.Num() > 0)
	{
		static FName rayTraceTag = FName(TEXT("GeneralMultiRayTrace"));

		//Sets parameters for the trace
		FCollisionQueryParams traceParameters(rayTraceTag, false, ignoreActors[0]);
		traceParameters.bTraceAsyncScene = true;
		traceParameters.bReturnPhysicalMaterial = true;
		traceParameters.AddIgnoredActors(ignoreActors);

		if (world != NULL)
		{
			//Performs the actual ray/line trace
			rayTraceHit = world->LineTraceMultiByChannel(hitResults, traceFromLocation, traceToLocation, COLLISION_INSTANT_HIT_MULTI, traceParameters);
		}
	}

	return rayTraceHit;
}

bool FPSGUtility::rayTrace(AActor* ignoreActor, UPrimitiveComponent* component,
	const FVector& traceFromLocation, const FVector& traceToLocation, FHitResult& hitResult)
{
	bool rayTraceHit = false;

	if (ignoreActor != NULL)
	{
		static FName rayTraceTag = FName(TEXT("ComponentRayTrace"));

		//Sets parameters for the trace
		FCollisionQueryParams traceParameters(rayTraceTag, false, ignoreActor);
		traceParameters.bTraceAsyncScene = true;
		traceParameters.bReturnPhysicalMaterial = true;
		traceParameters.AddIgnoredActor(ignoreActor);

		hitResult = FHitResult(ForceInit);

		if (component != NULL)
		{
			//Performs the actual ray/line trace
			rayTraceHit = component->LineTraceComponent(hitResult, traceFromLocation, traceToLocation, traceParameters);
		}
	}

	return rayTraceHit;
}

bool FPSGUtility::rayTrace(const TArray<AActor*>& ignoreActors, UPrimitiveComponent* component,
	const FVector& traceFromLocation, const FVector& traceToLocation, FHitResult& hitResult)
{
	bool rayTraceHit = false;
	
	//If there are any actors to ignore
	if (ignoreActors.Num() > 0)
	{
		static FName rayTraceTag = FName(TEXT("ComponentRayTrace"));

		//Sets parameters for the trace
		FCollisionQueryParams traceParameters(rayTraceTag, false, ignoreActors[0]);
		traceParameters.bTraceAsyncScene = true;
		traceParameters.bReturnPhysicalMaterial = true;
		traceParameters.AddIgnoredActors(ignoreActors);
		
		hitResult = FHitResult(ForceInit);
		
		if (component != NULL)
		{
			//Performs the actual ray/line trace
			rayTraceHit = component->LineTraceComponent(hitResult, traceFromLocation, traceToLocation, traceParameters);
		}
	}

	return rayTraceHit;
}

UUserWidget* FPSGUtility::createUserWidget(APlayerController* playerOwner, UClass* widgetClass, bool addToViewport)
{
	UUserWidget* result = NULL;

	//Create a instance of the widget widgetClass
	if (widgetClass != NULL)
	{
		result = CreateWidget<UUserWidget>(playerOwner, widgetClass);

		if (addToViewport)
		{
			//Add the UserWidget to the screen
			if (result != NULL && !result->IsInViewport())
			{
				result->AddToViewport();
			}
		}
	}
	return result;
}

void FPSGUtility::destroyUserWidget(UUserWidget** widgetToDestroy)
{
	//Removes the UserWidget from the parent widget, (and screen if added to the viewport), then destroy it
	if (widgetToDestroy != NULL)
	{
		if ((*widgetToDestroy) != NULL)
		{
			(*widgetToDestroy)->RemoveFromParent();
			(*widgetToDestroy) = NULL;
		}
	}
}
