// Copyright (c) CORE Games, Ltd. All rights reserved.

#pragma once

#include <CoreMinimal.h>
#include <Camera/CameraActor.h>

#include "C0TTRPGCam.generated.h"

UENUM(BlueprintType)
enum class EC0AdjustmentMode : uint8
{
    E_NONE		UMETA(DisplayName = "NONE", Hidden),

    // AR and FOV are calculated automatically, depending on camera position
    Automatic	UMETA(DisplayName = "Automatic"),
    // AR and FOV are calculated based on cam position, plus additional adjustable value
    Additional	UMETA(DisplayName = "Additional"),
    // AR and FOV are not automatically adjusted
    Manual		UMETA(DisplayName = "Manual"),

    E_NUM		UMETA(Hidden)
};

class AC0Grid;

UCLASS(config = BeneosTools, meta = (DisplayName = "Beneos Cam"))
class BENEOSTOOLS_API AC0TTRPGCam : public ACameraActor
{
    GENERATED_BODY()

public:

    AC0TTRPGCam();

    virtual void OnConstruction(const FTransform& Transform) override;

    virtual void PostEditMove(bool bFinished) override;

    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

    FString GetDefaultActorLabel() const override;

    // Updates the camera's aspect ratio + FOV based on grid Length and Width
    // and relative camera position.
    void UpdateCamera();

#pragma region Properties

protected:

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beneos Tools")
        TObjectPtr<AC0Grid> ParentGrid;

    // Sets auto-update behaviour of cam's FOV + Aspect Ratio
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Advanced")
        EC0AdjustmentMode AdjustmentMode;

        EC0AdjustmentMode PrevAdjustmentMode;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Advanced", meta = (EditCondition = "AdjustmentMode != EC0AdjustmentMode::Automatic", ClampMin = "0.1", ClampMax = "100.0"))
        float AspectRatio;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Advanced", meta = (EditCondition = "AdjustmentMode != EC0AdjustmentMode::Automatic", UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg))
        float FOV;

    // Overlay from the edge of the screen to be covered with black, in tiles
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Margin")
        float TopOverlayMargin;

    // Overlay from the edge of the screen to be covered with black, in tiles
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Margin")
        float BottomOverlayMargin;

    // Overlay from the edge of the screen to be covered with black, in tiles
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Margin")
        float RightOverlayMargin;

    // Overlay from the edge of the screen to be covered with black, in tiles
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Margin")
        float LeftOverlayMargin;

    // Margin between the grid edge and the frame of the camera
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Advanced")
        float GridMarginInTiles;

    // Camera's up vector will always be this
    /*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools")
        FVector UpDirection;*/

        // Tick to draw a line between the camera and its focal point
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beneos Tools|Advanced")
        bool bDebugDrawLineToFocalPoint;

    // Toggles whether this camera shows a picture-in-picture preview 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beneos Tools|Advanced")
        bool bShowPIP;

#pragma endregion Properties

#pragma region Editor Functions

    // Hides/shows the parent grid
    UFUNCTION(CallInEditor, Category = "Beneos Tools")
        void ToggleGridVisibility();

    // Applies the visual settings of the parent grid (colour, line width, visibility) to all grids in the scene
    UFUNCTION(CallInEditor, Category = "Beneos Tools")
        void ApplyGlobally();

#pragma endregion Editor Functions

#pragma region PIP

public:

    // Handles PIP disabling
    void OnSelectActor(const bool bInPrevShowPIP);
    void OnDeselectActor();
    bool GetPrevShowPIP() { return bPrevShowPIP; }

private:

    // Value of ULevelEditorViewportSettings::bPreviewSelectedCameras setting before selected
    bool bPrevShowPIP;
    void SetShowPIP(const bool bShow);

#pragma endregion PIP

#pragma region Margin

    TObjectPtr<UStaticMeshComponent> TopMarginComponent;
    TObjectPtr<UStaticMeshComponent> BottomMarginComponent;
    TObjectPtr<UStaticMeshComponent> RightMarginComponent;
    TObjectPtr<UStaticMeshComponent> LeftMarginComponent;

#pragma endregion Margin

private:

    void HandleDebugDraw();

    bool bInitialized;

    AC0Grid* PrevParentGrid;

};

#pragma region Utility

bool FindLineIntersection(
    const FVector2D& Point1, const FVector2D& Dir1,
    const FVector2D& Point2, const FVector2D& Dir2,
    FVector2D& IntersectionPoint);

FVector2D ProjectPointOntoLine(const FVector2D& Point, const FVector2D& LinePoint, const FVector2D& LineDirection);

#pragma endregion Utility