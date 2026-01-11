// Copyright (c) CORE Games, Ltd. All rights reserved.

#include "C0TTRPGCineCam.h"

#include <CineCameraComponent.h>
#include <Kismet/KismetMathLibrary.h>
#include "C0Grid.h"

AC0TTRPGCineCam::AC0TTRPGCineCam(const FObjectInitializer& ObjectInitializer) :
    Super(ObjectInitializer),
    ParentGrid(nullptr),
    AdjustmentMode(EC0AdjustmentMode::Automatic),
    AspectRatio(0.f),
    FOV(0.f),
    TopOverlayMargin(0.f),
    BottomOverlayMargin(0.f),
    RightOverlayMargin(0.f),
    LeftOverlayMargin(0.f),
    GridMarginInTiles(0.f),
    bDebugDrawLineToFocalPoint(false),
    bShowPIP(true),
    bInitialized(false)
{
    SetActorRotation(FRotator(-90.f, 0.f, 0.f));

    // Create margin meshes
    const FString MaterialPath = TEXT("/BeneosTools/M_MarginMaterial.M_MarginMaterial");
    UMaterial* Material = Cast<UMaterial>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MaterialPath));
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
    TopMarginComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("TopMarginComponent"));
    TopMarginComponent->SetupAttachment(RootComponent);
    TopMarginComponent->SetVisibility(false);
    TopMarginComponent->SetMaterial(0, Material);
    TopMarginComponent->SetStaticMesh(PlaneMesh.Object);
    TopMarginComponent->SetUsingAbsoluteRotation(false);
    TopMarginComponent->SetCastShadow(false);
    TopMarginComponent->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
    TopMarginComponent->SetRenderCustomDepth(false);
    TopMarginComponent->bSelectable = false;
    BottomMarginComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("BottomMarginComponent"));
    BottomMarginComponent->SetupAttachment(RootComponent);
    BottomMarginComponent->SetVisibility(false);
    BottomMarginComponent->SetMaterial(0, Material);
    BottomMarginComponent->SetStaticMesh(PlaneMesh.Object);
    BottomMarginComponent->SetUsingAbsoluteRotation(false);
    BottomMarginComponent->SetCastShadow(false);
    BottomMarginComponent->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
    BottomMarginComponent->SetRenderCustomDepth(false);
    BottomMarginComponent->bSelectable = false;
    RightMarginComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("RightMarginComponent"));
    RightMarginComponent->SetupAttachment(RootComponent);
    RightMarginComponent->SetVisibility(false);
    RightMarginComponent->SetMaterial(0, Material);
    RightMarginComponent->SetStaticMesh(PlaneMesh.Object);
    RightMarginComponent->SetUsingAbsoluteRotation(false);
    RightMarginComponent->SetCastShadow(false);
    RightMarginComponent->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
    RightMarginComponent->SetRenderCustomDepth(false);
    RightMarginComponent->bSelectable = false;
    LeftMarginComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LeftMarginComponent"));
    LeftMarginComponent->SetupAttachment(RootComponent);
    LeftMarginComponent->SetVisibility(false);
    LeftMarginComponent->SetMaterial(0, Material);
    LeftMarginComponent->SetStaticMesh(PlaneMesh.Object);
    LeftMarginComponent->SetUsingAbsoluteRotation(false);
    LeftMarginComponent->SetCastShadow(false);
    LeftMarginComponent->SetRelativeRotation(FRotator(90.f, 0.f, 0.f));
    LeftMarginComponent->SetRenderCustomDepth(false);
    LeftMarginComponent->bSelectable = false;
}

void AC0TTRPGCineCam::OnConstruction(const FTransform& Transform)
{
    // Call initialization only the first time the actor is created in editor
    if (!bInitialized)
    {
        bInitialized = true;
        PrevAdjustmentMode = AdjustmentMode;
        PrevParentGrid = ParentGrid;
        if (ParentGrid)
        {
            ParentGrid->ChildCineCameras.Add(this);
        }
    }

    Super::OnConstruction(Transform);

    HandleDebugDraw();
    
    UpdateCamera();
}

void AC0TTRPGCineCam::PostEditMove(bool bFinished)
{
    Super::PostEditMove(bFinished);

    UpdateCamera();
}

void AC0TTRPGCineCam::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    
    if (PropertyChangedEvent.Property)
    {
        // If the ShowPIP property gets changed, re-select this actor 
        // so that the change gets applied immediately
        if (PropertyChangedEvent.Property->GetName() == "bShowPIP")
        {
            if (GEditor)
            {
                GEditor->SelectActor(this, true, true);
            }
        }
        if (PropertyChangedEvent.Property->GetName() == "ParentGrid")
        {
            if (PrevParentGrid)
            {
                PrevParentGrid->ChildCineCameras.Remove(this);
            }
            if (ParentGrid)
            {
                ParentGrid->ChildCineCameras.Add(this);
            }
            PrevParentGrid = ParentGrid;
        }
    }
}

FString AC0TTRPGCineCam::GetDefaultActorLabel() const
{
    return TEXT("Beneos Cam");
}

void AC0TTRPGCineCam::UpdateCamera()
{
    if (!ParentGrid)
        return;

    // Constrain camera to ParentGrid grid's z-axis
    FVector PointToLine = GetActorLocation() - ParentGrid->GetActorLocation();
    FVector Dir = PointToLine;
    Dir.Normalize();
    if (Dir != ParentGrid->GetActorUpVector())
    {
        float ProjectionLength = FVector::DotProduct(PointToLine, ParentGrid->GetActorUpVector());
        FVector ProjectionVector = ParentGrid->GetActorUpVector() * ProjectionLength;
        FVector ProjectedPoint = ParentGrid->GetActorLocation() + ProjectionVector;
        SetActorLocation(ProjectedPoint);
        if (GEditor)
        {
            GEditor->SetPivot(ProjectedPoint, false, true, true);
        }
    }

    // Find camera rotation
    FVector Forward = (ParentGrid->GetActorLocation() - GetActorLocation()).GetSafeNormal();
    FVector RotatedUpDir = ParentGrid->GetActorRightVector().RotateAngleAxis(-90.f, ParentGrid->GetActorUpVector());
    FVector Right = FVector::CrossProduct(RotatedUpDir, Forward).GetSafeNormal();
    FVector AdjustedUp = FVector::CrossProduct(Forward, Right);
    FMatrix RotationMatrix = FMatrix(Forward, Right, AdjustedUp, FVector::ZeroVector);
    SetActorRotation(RotationMatrix.Rotator());    

    const float Margin = GridMarginInTiles * 2.f;
    const float AutoAspectRatio = (ParentGrid->Width + Margin) / (ParentGrid->Length + Margin);
    const float CamDist = FVector::Dist(ParentGrid->GetActorLocation(), GetActorLocation());
    const float HalfGridWidth = (ParentGrid->Width + Margin) * ParentGrid->TileSizeInCm / 2.f;
    const float AutoFOV = FMath::RadiansToDegrees(2 * FMath::Atan(HalfGridWidth / CamDist));
    // Adjust FOV + aspect ratio based on mode
    if (AdjustmentMode != PrevAdjustmentMode)
    {
        if (AdjustmentMode == EC0AdjustmentMode::Automatic)
        {
            AspectRatio = 0.f;
            FOV = 0.f;
        }
        else if (AdjustmentMode == EC0AdjustmentMode::Additional &&
            PrevAdjustmentMode == EC0AdjustmentMode::Manual)
        {            
            AspectRatio -= AutoAspectRatio;
            FOV -= AutoFOV;
        }
        else if (AdjustmentMode == EC0AdjustmentMode::Manual)
        {
            if (PrevAdjustmentMode == EC0AdjustmentMode::Automatic)
            {
                AspectRatio = AutoAspectRatio;
                FOV = AutoFOV;
            }
            else if (PrevAdjustmentMode == EC0AdjustmentMode::Additional)
            {
                AspectRatio += AutoAspectRatio;
                FOV += AutoFOV;
            }
        }

        PrevAdjustmentMode = AdjustmentMode;
    }

    float FinalAspectRatio = AspectRatio;
    float FinalFOV = FOV;
    if (AdjustmentMode == EC0AdjustmentMode::Automatic)
    {
        FinalAspectRatio = AutoAspectRatio;
        FinalFOV = AutoFOV;
    }
    else if (AdjustmentMode == EC0AdjustmentMode::Additional)
    {
        FinalAspectRatio = AutoAspectRatio + AspectRatio;
        FinalFOV = AutoFOV + FOV;
    }
    else if (AdjustmentMode == EC0AdjustmentMode::Manual)
    {
        FinalAspectRatio = AspectRatio;
        FinalFOV = FOV;
    }
    
    if (UCineCameraComponent* Camera = GetCineCameraComponent())
    {
        const float MagicNumber = 153.f;
        const float M = (MagicNumber / CamDist) * Camera->CurrentFocalLength;
        Camera->Filmback.SensorWidth = (ParentGrid->Width + Margin) * M;
        Camera->Filmback.SensorHeight = (ParentGrid->Length + Margin) * M;
    }

    // Update margin overlays
    TopMarginComponent->SetVisibility(TopOverlayMargin > 0.f);
    BottomMarginComponent->SetVisibility(BottomOverlayMargin > 0.f);
    RightMarginComponent->SetVisibility(RightOverlayMargin > 0.f);
    LeftMarginComponent->SetVisibility(LeftOverlayMargin > 0.f);
    // Distance of overlay panels from cam centre
    const float OverlayDist = 50.f;
    const float TileSizeInUnits = ParentGrid->TileSizeInCm / 100.f;
    for (int32 i = 0; i < 4; i++)
    {
        UStaticMeshComponent* MarginComponent = TopMarginComponent;
        FVector Location;
        FVector Scale;
        if (i == 0)
        {
            MarginComponent = TopMarginComponent;
            Location = ParentGrid->GetActorLocation() + ParentGrid->GetActorForwardVector() * ((ParentGrid->Length - TopOverlayMargin) / 2) * ParentGrid->TileSizeInCm;
            Scale = FVector(TopOverlayMargin * TileSizeInUnits, ParentGrid->Width * TileSizeInUnits, 1.f);
        }
        if (i == 1)
        {
            MarginComponent = BottomMarginComponent;
            Location = ParentGrid->GetActorLocation() - ParentGrid->GetActorForwardVector() * ((ParentGrid->Length - BottomOverlayMargin) / 2) * ParentGrid->TileSizeInCm;
            Scale = FVector(BottomOverlayMargin * TileSizeInUnits, ParentGrid->Width * TileSizeInUnits, 1.f);
        }
        else if (i == 2)
        {
            MarginComponent = RightMarginComponent;
            Location = ParentGrid->GetActorLocation() + ParentGrid->GetActorRightVector() * ((ParentGrid->Width - RightOverlayMargin) / 2) * ParentGrid->TileSizeInCm;
            Scale = FVector(ParentGrid->Length * TileSizeInUnits, RightOverlayMargin * TileSizeInUnits, 1.f);
        }
        else if (i == 3)
        {
            Location = ParentGrid->GetActorLocation() - ParentGrid->GetActorRightVector() * ((ParentGrid->Width - LeftOverlayMargin) / 2) * ParentGrid->TileSizeInCm;
            Scale = FVector(ParentGrid->Length * TileSizeInUnits, LeftOverlayMargin * TileSizeInUnits, 1.f);
            MarginComponent = LeftMarginComponent;
        }

        const FVector VectorToCam = GetActorLocation() - Location;
        const float DistToCam = VectorToCam.Length();
        const float MoveRatio = (DistToCam - OverlayDist) / DistToCam;
        Location += (VectorToCam * MoveRatio);
        Scale *= 1.f - MoveRatio;
        MarginComponent->SetWorldLocation(Location);        
        MarginComponent->SetUsingAbsoluteScale(false);
        MarginComponent->SetWorldScale3D(Scale);        
    }
}

#pragma region Editor Functions

void AC0TTRPGCineCam::ToggleGridVisibility()
{
    if (ParentGrid)
    {
        ParentGrid->ToggleVisibility();
    }
}

void AC0TTRPGCineCam::ApplyGlobally()
{
    if (ParentGrid)
    {
        ParentGrid->ApplyGlobally();
    }
}

#pragma endregion Editor Functions

#pragma region PIP

void AC0TTRPGCineCam::OnSelectActor(const bool bInPrevShowPIP)
{
    bPrevShowPIP = bInPrevShowPIP;
    SetShowPIP(bShowPIP);
}

void AC0TTRPGCineCam::OnDeselectActor()
{
    ULevelEditorViewportSettings* ViewportSettings = GetMutableDefault<ULevelEditorViewportSettings>();
    if (ViewportSettings)
    {
        SetShowPIP(bPrevShowPIP);
    }
}

void AC0TTRPGCineCam::SetShowPIP(const bool bShow)
{
    ULevelEditorViewportSettings* ViewportSettings = GetMutableDefault<ULevelEditorViewportSettings>();
    if (ViewportSettings)
    {
        ViewportSettings->bPreviewSelectedCameras = bShow;
        ViewportSettings->SaveConfig();
    }
}

#pragma endregion PIP

void AC0TTRPGCineCam::HandleDebugDraw()
{
    if (bDebugDrawLineToFocalPoint)
    {
        DrawDebugLine(GetWorld(), GetActorLocation(), ParentGrid->GetActorLocation(), FColor::Red, false, 1.f);
        FTimerHandle Handle;
        GetWorld()->GetTimerManager().SetTimer(Handle, this, &AC0TTRPGCineCam::HandleDebugDraw, 1.0f, false);
    }
}
