// Copyright (c) CORE Games, Ltd. All rights reserved.

#pragma once

#include "C0Grid.h"
#include "C0TTRPGCam.h"

#include "C0CamGrid.generated.h"

class UC0ChildCamComponent;

UCLASS(config = BeneosTools, meta = (DisplayName = "Beneos Cam Grid"))
class BENEOSTOOLS_API AC0CamGrid : public AC0Grid
{
    GENERATED_BODY()

public:

    AC0CamGrid();

    virtual void OnConstruction(const FTransform& Transform) override;

    virtual FString GetDefaultActorLabel() const override;

    //// Called whenever a property is changed in the editor
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

    virtual void PostEditMove(bool bFinished) override;

private:

    bool bInitialized;

#pragma region Properties

public:

    // Sets auto-update behaviour of cam's FOV + Aspect Ratio
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools")
        EC0AdjustmentMode AdjustmentMode;

    EC0AdjustmentMode PrevAdjustmentMode;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools", meta = (EditCondition = "AdjustmentMode != EC0AdjustmentMode::Automatic", ClampMin = "0.1", ClampMax = "100.0"))
        float AspectRatio;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools", meta = (EditCondition = "AdjustmentMode != EC0AdjustmentMode::Automatic", UIMin = "5.0", UIMax = "170", ClampMin = "0.001", ClampMax = "360.0", Units = deg))
        float FOV;

    // Margin between the grid edge and the frame of the camera
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools")
        float GridMarginInTiles;

    // Camera's up vector will always be this
    /*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools")
        FVector UpDirection;*/

    // Tick to draw a line between the camera and its focal point
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beneos Tools")
        bool bDebugDrawLineToFocalPoint;

    // Toggles whether this camera shows a picture-in-picture preview 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beneos Tools")
        bool bShowPIP;

    UPROPERTY(EditAnywhere)
        TObjectPtr<UC0ChildCamComponent> BeneosCam;

    // Hides/shows the grid
    UFUNCTION(CallInEditor, Category = "Beneos Tools")
        void ToggleGrid();

#pragma endregion Properties

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

private:

    void HandleDebugDraw();

};
