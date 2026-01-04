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

private:

    void LoadGridMaterial(const bool bRedo = false);

public:
    
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
    // Render on top
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools")
        bool bRenderOnTop;

    void UpdateGrid();
    // Enable Snapping
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Snapping")
        bool bEnableSnapping;

    // Use selected actor as snapping origin
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Snapping", meta = (EditCondition = "bEnableSnapping"))
        bool bUseActorSnapOrigin;

    // Snap Origin Actor
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Snapping", meta = (EditCondition = "bEnableSnapping && bUseActorSnapOrigin"))
        TObjectPtr<AActor> SnapOriginActor;

    // Snap Origin Vector
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Snapping", meta = (EditCondition = "bEnableSnapping && !bUseActorSnapOrigin"))
        FVector SnapOriginVector;

    // Snap increment
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Snapping", meta = (EditCondition = "bEnableSnapping"))
        float SnapIncrement;

    // Enable to halve the snap increment
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Snapping", meta = (EditCondition = "bEnableSnapping"))
        bool bHalveSnapIncrement;

private:

    // Variables used to snap while dragging
    
    // Actor position on previous movement frame
    float PrevMoveX;
    float PrevMoveY;
    // Actor position *before* drag movement started
    float StartMoveX;
    float StartMoveY;
    // Total amount that the gizmo has been moved
    float GizmoMoveX;
    float GizmoMoveY;
    // True if was dragging in the previous frame
    bool bWasDragging;

    void SnapToIncrement();

public:

    TObjectPtr<UStaticMeshComponent> PlaneMeshComponent;

    TObjectPtr<UMaterialInstanceDynamic> DynamicMaterial;

};
