// FPSGame created by Gard and Gorm

#pragma once

class FPSGAME_API FPSGUtility
{
public:
	FPSGUtility();
	~FPSGUtility();

	/**
	* Performs a general ray trace against the world using the channel COLLISION_INSTANT_HIT. It will return the first blocking hit
	* Checks against simple collisions (sphere, capsulate component etc, static mesh collisions)
	* @param ignoreActor The actor to ignore for this trace
	* @param world The world context. Just use GetWorld() from where the function is called
	* @param traceFromLocation The location where the trace is started
	* @param traceToLocation The location where the trace is ended
	* @param hitResult Contains information about the hit if the trace hit something
	* @return true if the trace hit something, false otherwise
	*/
	static bool rayTrace(AActor* ignoreActor, UWorld* world, const FVector& traceFromLocation, const FVector& traceToLocation, FHitResult& hitResult);

	/**
	* Performs a general ray trace against the world using the channel COLLISION_INSTANT_HIT. It will return the first blocking hit
	* Checks against simple collisions (sphere, capsulate component etc, static mesh collisions)
	* @param ignoreActors An array of actors to ignore for this trace
	* @param world The world context. Just use GetWorld() from where the function is called
	* @param traceFromLocation The location where the trace is started
	* @param traceToLocation The location where the trace is ended
	* @param hitResult Contains information about the hit if the trace hit something
	* @return true if the trace hit something, false otherwise
	*/
	static bool rayTrace(const TArray<AActor*>& ignoreActors, UWorld* world, const FVector& traceFromLocation, const FVector& traceToLocation, FHitResult& hitResult);

	/**
	* Performs a general ray trace against the world using the channel COLLISION_INSTANT_HIT_MULTI. Returns all overlaps and the first blocking hit found, if any
	* Checks against simple collisions (sphere, capsulate component etc, static mesh collisions)
	* @param ignoreActor The actor to ignore for this trace
	* @param world The world context. Just use GetWorld() from where the function is called
	* @param traceFromLocation The location where the trace is started
	* @param traceToLocation The location where the trace is ended
	* @param hitResults Contains information about all the overlap hits and block hit that the trace went through
	* @return true if the trace hit something, false otherwise
	*/
	static bool rayTrace(AActor* ignoreActors, UWorld* world, const FVector& traceFromLocation, const FVector& traceToLocation, TArray<FHitResult>& hitResults);

	/**
	* Performs a general ray trace against the world using the channel COLLISION_INSTANT_HIT_MULTI. Returns all overlaps and the first blocking hit found, if any
	* Checks against simple collisions (sphere, capsulate component etc, static mesh collisions)
	* @param ignoreActors An array of actors to ignore for this trace
	* @param world The world context. Just use GetWorld() from where the function is called
	* @param traceFromLocation The location where the trace is started
	* @param traceToLocation The location where the trace is ended
	* @param hitResults Contains information about all the overlap hits and block hit that the trace went through
	* @return true if the trace hit something, false otherwise
	*/
	static bool rayTrace(const TArray<AActor*>& ignoreActors, UWorld* world, const FVector& traceFromLocation, const FVector& traceToLocation, TArray<FHitResult>& hitResults);

	/**
	* Function to perform a ray trace against a single component. Checks for collisions on this component only
	* @param ignoreActor The actor to ignore for this trace
	* @param component The component to check the ray trace against
	* @param traceFromLocation The location where the trace is started
	* @param traceToLocation The location where the trace is ended
	* @param hitResult Contains information about the hit if the trace hit something
	* @return true if the trace hit something, false otherwise
	*/
	static bool rayTrace(AActor* ignoreActor, UPrimitiveComponent* component, const FVector& traceFromLocation, const FVector& traceToLocation, FHitResult& hitResult);

	/**
	* Function to perform a ray trace against a single component. Checks for collisions on this component only
	* @param ignoreActors An array of actors to ignore for this trace
	* @param component The component to check the ray trace against
	* @param traceFromLocation The location where the trace is started
	* @param traceToLocation The location where the trace is ended
	* @param hitResult Contains information about the hit if the trace hit something
	* @return true if the trace hit something, false otherwise
	*/
	static bool rayTrace(const TArray<AActor*>& ignoreActors, UPrimitiveComponent* component, const FVector& traceFromLocation, const FVector& traceToLocation, FHitResult& hitResult);

	/**
	* Function to create a UserWidget with the specified PlayerController as the owner
	* @param playerOwner The owner of this widget
	* @param widgetClass The UserWidget class to create an instance of
	* @param addToViewport Set to true if the created UserWidget should be added to the viewport (i.e. rendered on the screen)
	* @return UUserWidget The created UserWidget. NULL if not successfully created
	*/
	static UUserWidget* createUserWidget(APlayerController* playerOwner, UClass* widgetClass, bool addToViewport);

	/**
	* Function that destroys the specified UserWidget, remove it from the parent widget and viewport (screen) (Including setting it to NULL)
	* @param widgetToDestroy The widget to destroy
	*/
	static void destroyUserWidget(UUserWidget** widgetToDestroy);
};
