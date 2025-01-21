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

UCLASS(meta = (DisplayName = "Beneos Cam"))
class BENEOSTOOLS_API AC0TTRPGCam : public ACameraActor
{
    GENERATED_BODY()

public:

    AC0TTRPGCam();

    virtual void OnConstruction(const FTransform& Transform) override;

    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

    virtual FString GetDefaultActorLabel() const override;

protected:
    
    // Centre of grid and point that the camera is focused on, in world space
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beneos Tools")
        FVector GridCentre;

    // Margin between the grid edge and the frame of the camera
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools")
        float GridMarginInTiles;

    // Length in tiles
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools", meta = (ClampMin = 0.1f))
        float Length;

    // Width in tiles
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools", meta = (ClampMin = 0.1f))
        float Width;

    // Tile size in Unreal units (cm). Default is 152.4 ~= 5ft
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools")
        float TileSizeInCm;

    // Width of grid lines as % of tile width
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools", meta = (ClampMin = 0.01f, ClampMax = 1.f))
        float LineWidth;

    // Grid Colour
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools")
        FColor GridColour;

    // Grid Rotation in degrees around Z axis
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools")
        float GridRotation;

    // Camera's up vector will always be this
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools")
        FVector UpDirection;

    // Sets auto-update behaviour of cam's FOV + Aspect Ratio
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools")
        EC0AdjustmentMode AdjustmentMode;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools", meta = (EditCondition = "AdjustmentMode != EC0AdjustmentMode::Automatic", ClampMin = "0.1", ClampMax = "100.0"))
        float AspectRatio;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools", meta = (EditCondition = "AdjustmentMode != EC0AdjustmentMode::Automatic", UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg))
        float FOV;

    // Tick to draw a line between the camera and its focal point
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beneos Tools")
        bool bDebugDrawLineToFocalPoint;

    // Toggles whether this camera shows a picture-in-picture preview 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beneos Tools")
        bool bShowPIP;

public:

    // Hides/shows the grid
    UFUNCTION(CallInEditor, Category = "Beneos Tools")
        void ToggleGrid();

    // Centres the camera directly above the centre of the grid
    UFUNCTION(CallInEditor, Category = "Beneos Tools")
        void CentreCam();
    
    void OnSelectActor(const bool bInPrevShowPIP);
    void OnDeselectActor();
    bool GetPrevShowPIP() { return bPrevShowPIP; }

private:

    // Updates the camera's aspect ratio + FOV based on grid Length and Width
    // and relative camera position.
    void UpdateCamera();

    void HandleDebugDraw();

    TObjectPtr<UChildActorComponent> GridChildActor;
    AC0Grid* GetGridActor();

    void SetShowPIP(const bool bShow);

    bool bInitialized;

    EC0AdjustmentMode PrevAdjustmentMode;

    FVector CameraFocusPoint;

    // Value of ULevelEditorViewportSettings::bPreviewSelectedCameras setting before selected
    bool bPrevShowPIP;

};
