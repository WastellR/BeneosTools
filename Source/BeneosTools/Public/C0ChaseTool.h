// Copyright (c) CORE Games, Ltd. All rights reserved.

#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>

#include "C0ChaseTool.generated.h"

class UBoxComponent;
class UC0BoxComponent;
class ACameraActor;

DECLARE_MULTICAST_DELEGATE(FOnEditorUIUpdate);

/*
	Two boxes, that can copy all actors in box A and paste them 
	all in box B.
	- Boxes can be used to grab and move contained actors
	- Boxes are separate components, so can be moved together or independently
	
	A camera object can be selected to be controlled by this actor.

*/
UCLASS(config = BeneosTools, meta = (DisplayName = "Beneos Chase Tool"))
class BENEOSTOOLS_API AC0ChaseTool : public AActor
{
    GENERATED_BODY()

public:

	AC0ChaseTool();

    virtual FString GetDefaultActorLabel() const override;

	virtual void PostActorCreated() override;

    virtual void PostEditMove(bool bFinished) override;

    // Called whenever a property is changed in the editor
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
	virtual bool ShouldTickIfViewportsOnly() const override { return true; };

	virtual void Tick(float DeltaSeconds) override;

	// UI customization event
	FOnEditorUIUpdate EditorUIUpdate;

#pragma region Chase

public:	

	// Dimensions of each box (always kept the same as each other)
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Category = "Beneos Tools"))
		FVector BoxSize;

	// Camera loop duration in seconds 
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Units="Seconds"))
		float LoopDuration;

	// Length between the centre of the two boxes in cm
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float PathDistanceInCm;	

	// Draws a line between the box centres when selected
	UPROPERTY()
		bool bDrawCameraPath;

	// Camera path colour
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (Category = "Beneos Tools", EditCondition = "bDrawCameraPath", DisplayName = "Draw Camera Path"))
		FColor PathColour;

	FText GetSpeedText() const;

#pragma region Preview

	void ToggleChasePreview();

	// Camera actor to use for chase preview
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		TObjectPtr<ACameraActor> Camera;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		float CameraHeight;

	// Current position of camera along chase path (between 0 and 1)
	UPROPERTY(EditAnywhere, Interp)
		float CameraChasePosition;

	UFUNCTION(CallInEditor)
		void SetCameraChasePosition(float NewPosition);

	// Length of time to pause at end of chase preview
	UPROPERTY(EditAnywhere, meta=(Units="Seconds"))
		float CameraChasePause;

#pragma endregion Preview

private:

	UPROPERTY()
		bool bIsRunningChasePreview;
	float ChasePreviewStart;

	const FVector GetCameraPos() const;

public:

	bool IsRunningChasePreview() const { return bIsRunningChasePreview; }

#pragma endregion Chase

#pragma region Box A

public:

	// Move contained actors along with Box A
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName = "BoxA Grab Contents"))
		bool bBoxAGrabContents;

	// Copy actors from A to B
	void CopyAToB();

	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		TObjectPtr<UC0BoxComponent> BoxA;

private:

	UFUNCTION()
		void OnBoxAEditChangedProperty(FName PropertyName, FName MemberPropertyName);

#pragma endregion Box A

#pragma region Box B

public:

	// Move contained actors along with Box B
	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (DisplayName="BoxB Grab Contents"))
		bool bBoxBGrabContents;

	// Copy actors from B to A
	void CopyBToA();

	// Use subclass so delegate fires on PostEditComponentMove
	UPROPERTY(VisibleDefaultsOnly, BlueprintReadOnly)
		TObjectPtr<UC0BoxComponent> BoxB;

private:

	UFUNCTION()
		void OnBoxBEditComponentMoved(bool bFinished);

	UFUNCTION()
		void OnBoxBEditChangedProperty(FName PropertyName, FName MemberPropertyName);

#pragma endregion BoxB

private:

	TMap<AActor*, FVector> BoxAGrabActorLocations;	
	TMap<AActor*, FVector> BoxBGrabActorLocations;

	UPROPERTY()
		FVector StartMoveLocation;
	UPROPERTY()
		FVector BoxBStartMoveLocation;

	bool bMoveHasStarted;
	bool bMoveBoxBHasStarted;

	void GetBoxOverlappingActors(TArray<FOverlapResult>& OverlapsOut, const UBoxComponent& Box, FVector BoxDelta = FVector::ZeroVector);
	const bool IsGrabbable(AActor* Actor);
};
