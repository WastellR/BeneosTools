// Copyright (c) CORE Games, Ltd. All rights reserved.

#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>

#include "C0Grid.generated.h"

UCLASS(config = BeneosTools, meta = (DisplayName = "Beneos Grid"))
class BENEOSTOOLS_API AC0Grid : public AActor
{
    GENERATED_BODY()

public:

    AC0Grid();

    virtual void OnConstruction(const FTransform& Transform) override;

    virtual FString GetDefaultActorLabel() const override;

    // Called whenever a property is changed in the editor
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;

protected:
    
    // Length in tiles
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools", meta = (ClampMin = 0.1f))
        float Length;

    // Width in tiles
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools", meta = (ClampMin = 0.1f))
        float Width;

    // Tile size in Unreal units (cm). Default is 152.4cm ~= 5ft
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

    void UpdateGrid();

    TObjectPtr<UStaticMeshComponent> PlaneMeshComponent;

    TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

public:

    void SetParameters(const float InLength, const float InWidth, 
        const float InTileSizeInCm, const float InLineWidth, const FColor& InGridColour,
        const float InGridRotation);

};
