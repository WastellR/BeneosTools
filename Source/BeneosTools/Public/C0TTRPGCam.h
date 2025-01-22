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

class AC0CamGrid;

UCLASS(NotPlaceable, meta = (DisplayName = "Beneos Cam"))
class BENEOSTOOLS_API AC0TTRPGCam : public ACameraActor
{
    GENERATED_BODY()

public:

    AC0TTRPGCam();

    virtual void OnConstruction(const FTransform& Transform) override;

    void PostEditComponentMove(bool bFinished);

    virtual FString GetDefaultActorLabel() const override;

    void SetParent(AC0CamGrid* InParent);

    // Updates the camera's aspect ratio + FOV based on grid Length and Width
    // and relative camera position.
    void UpdateCamera();

protected:

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

    // Camera's up vector will always be this
    /*UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools")
        FVector UpDirection;*/

    // Toggles whether this camera shows a picture-in-picture preview 
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Beneos Tools")
        bool bShowPIP;

private:

    void HandleDebugDraw();

    FVector CameraFocusPoint;

    AC0CamGrid* Parent;

};
