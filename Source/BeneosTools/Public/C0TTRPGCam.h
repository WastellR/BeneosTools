// Copyright (c) CORE Games, Ltd. All rights reserved.

#pragma once

#include <CoreMinimal.h>
#include <Camera/CameraActor.h>

#include "C0TTRPGCam.generated.h"

class AC0Grid;

UCLASS(meta = (DisplayName = "Beneos Cam"))
class BENEOSTOOLS_API AC0TTRPGCam : public ACameraActor
{
    GENERATED_BODY()

public:

    AC0TTRPGCam();

    virtual void OnConstruction(const FTransform& Transform) override;

    virtual FString GetDefaultActorLabel() const override;

protected:
    
    // Centre of grid and point that the camera is focused on
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

    // Camera's up vector will always be this
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools")
        FVector UpDirection;

public:

    // Hides/shows the grid
    UFUNCTION(CallInEditor, Category = "Beneos Tools")
        void ToggleGrid();

    // Centres the camera directly above the centre of the grid
    UFUNCTION(CallInEditor, Category = "Beneos Tools")
        void CentreCam();

private:

    // Updates the camera's aspect ratio + FOV based on grid Length and Width
    // and relative camera position.
    void UpdateCamera();

    TObjectPtr<UChildActorComponent> GridChildActor;
    AC0Grid* GetGridActor();

};
