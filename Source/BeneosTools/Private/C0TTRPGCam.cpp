// Copyright (c) CORE Games, Ltd. All rights reserved.

#include "C0TTRPGCam.h"

#include <Camera/CameraComponent.h>

#include <Kismet/KismetMathLibrary.h>
#include "C0Grid.h"

#pragma region Utility


bool FindLineIntersection(
    const FVector2D& Point1, const FVector2D& Dir1,
    const FVector2D& Point2, const FVector2D& Dir2,
    FVector2D& IntersectionPoint)
{
    const float Determinant = Dir1.X * Dir2.Y - Dir1.Y * Dir2.X;
    // If the determinant is zero, the lines are parallel
    if (FMath::IsNearlyZero(Determinant))
    {
        return false;
    }

    const float t = ((Point2.X - Point1.X) * Dir2.Y - (Point2.Y - Point1.Y) * Dir2.X) / Determinant;
    IntersectionPoint = Point1 + t * Dir1;

    return true;
}

FVector2D ProjectPointOntoLine(const FVector2D& Point, const FVector2D& LinePoint, const FVector2D& LineDirection)
{
    FVector2D NormalizedDirection = LineDirection.GetSafeNormal();
    FVector2D LineToPoint = Point - LinePoint;
    float ProjectionLength = FVector2D::DotProduct(LineToPoint, NormalizedDirection);
    FVector2D ProjectionPoint = LinePoint + NormalizedDirection * ProjectionLength;

    return ProjectionPoint;
}

#pragma endregion Utility

AC0TTRPGCam::AC0TTRPGCam() :
    GridCentre(FVector::Zero()),
    GridMarginInTiles(0.1f),
    Length(10),
    Width(10),
    TileSizeInCm(152.4),
    LineWidth(0.05f),
    GridColour(FColor(255, 255, 255, 51)),
    GridRotation(0.f),
    UpDirection(FVector(1.f, 0.f, 0.f)),
    AdjustmentMode(EC0AdjustmentMode::Automatic),
    AspectRatio(0.f),
    FOV(0.f),
    bDebugDrawLineToFocalPoint(false),
    bInitialized(false)
{
    GridChildActor = CreateDefaultSubobject<UChildActorComponent>(TEXT("GridChild"));
    GridChildActor->SetupAttachment(RootComponent);
    GridChildActor->SetUsingAbsoluteLocation(true);
    GridChildActor->SetUsingAbsoluteRotation(true);
    GridChildActor->SetUsingAbsoluteScale(true);

    SetActorRotation(FRotator(-90.f, 0.f, 0.f));

    PrimaryActorTick.bCanEverTick = true;
    PrimaryActorTick.bTickEvenWhenPaused = true;
}

void AC0TTRPGCam::OnConstruction(const FTransform& Transform)
{
    // Call initialization only the first time the actor is created in editor
    if (!bInitialized)
    {
        bInitialized = true;
        PrevAdjustmentMode = AdjustmentMode;
    }

    Super::OnConstruction(Transform);

    GridChildActor->SetChildActorClass(AC0Grid::StaticClass());
    GridChildActor->CreateChildActor();
    UpdateCamera();
}

FString AC0TTRPGCam::GetDefaultActorLabel() const
{
    return TEXT("BeneosCam");
}

void AC0TTRPGCam::ToggleGrid()
{
    // Open custom cam viewport

    /*FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    FBeneosToolsModule& BeneosModule = FModuleManager::LoadModuleChecked<FBeneosToolsModule>("BeneosTools");
    LevelEditorModule.GetLevelEditorTabManager()->TryInvokeTab(BeneosModule.BeneosEditorTabId);
    BeneosModule.ViewportWidget->ViewportClient->CameraActor = this;*/

    GridChildActor->SetVisibility(!GridChildActor->GetVisibleFlag(), true);
}

void AC0TTRPGCam::CentreCam()
{
    FVector Location = GetActorLocation();
    Location.X = GridCentre.X;
    Location.Y = GridCentre.Y;
    SetActorLocation(Location);    

    UpdateCamera();
}

void AC0TTRPGCam::UpdateCamera()
{
    const float Margin = GridMarginInTiles * 2.f;
    const float AutoAspectRatio = (Width + Margin) / (Length + Margin);
    const float HalfGridWidth = (Width + Margin) * TileSizeInCm / 2.f;
    const float HalfGridLength = (Length + Margin) * TileSizeInCm / 2.f;
    float FrameWidth = HalfGridWidth;
    float FrameLength = HalfGridLength;
    float GridMinX = GridCentre.X - HalfGridWidth;
    float GridMaxX = GridCentre.X + HalfGridWidth;
    float GridMinY = GridCentre.Y - HalfGridLength;
    float GridMaxY = GridCentre.Y + HalfGridLength;
    
    FVector AdjustedCentre = GridCentre;
    FVector ProjectedCamera = FVector(GetActorLocation().X, GetActorLocation().Y, GridCentre.Z);
    FVector CameraOffset = ProjectedCamera - GridCentre;
    // Find camera focus location if it's tilted off-centre
    if (GetActorLocation().X != GridCentre.X || GetActorLocation().Y != GridCentre.Y)
    {        
        FVector2D Centre2D(GridCentre.X, GridCentre.Y);        
        FVector UpDir = FVector(1.f, 0.f, 0.f);
        FVector RightDir = FVector(0.f, 1.f, 0.f);
        UpDir = UpDir.RotateAngleAxis(GridRotation, FVector::UpVector);
        RightDir = RightDir.RotateAngleAxis(GridRotation, FVector::UpVector);
        // Find positions of grid corners
        const FVector TopLeft = GridCentre + (UpDir * HalfGridLength) + (RightDir * HalfGridWidth);
        const FVector TopRight = GridCentre + (UpDir * HalfGridLength) - (RightDir * HalfGridWidth);
        const FVector BottomLeft = GridCentre - (UpDir * HalfGridLength) + (RightDir * HalfGridWidth);
        const FVector BottomRight = GridCentre - (UpDir * HalfGridLength) - (RightDir * HalfGridWidth);
        // Project camera position onto central axes
        const FVector2D XProjection = ProjectPointOntoLine(FVector2D(ProjectedCamera), Centre2D, FVector2D(RightDir));
        const FVector2D YProjection = ProjectPointOntoLine(FVector2D(ProjectedCamera), Centre2D, FVector2D(UpDir));
        const float XOffsetRatio = FVector2D::DistSquared(Centre2D, XProjection) / HalfGridWidth;
        const float YOffsetRatio = FVector2D::DistSquared(Centre2D, YProjection) / HalfGridLength;
        // Find world space position of desired edges of frame
        FVector IntersectA, IntersectB;
        FVector2D Point1 = FVector2D(TopLeft.X, TopLeft.Y);
        FVector2D Point2 = FVector2D(BottomRight.X, BottomRight.Y);
        FVector2D Dir;
        if (XOffsetRatio < YOffsetRatio)
        {
            Dir = FVector2D(TopLeft.X - TopRight.X, TopLeft.Y - TopRight.Y);
            Dir.Normalize();
        }
        else
        {
            Dir = FVector2D(TopLeft.X - BottomLeft.X, TopLeft.Y - BottomLeft.Y);
            Dir.Normalize();
        }
        FVector2D IntersectPoint;
        FVector2D CamDir(CameraOffset.X, CameraOffset.Y);
        CamDir.Normalize();
        FindLineIntersection(Point1, Dir, Centre2D, CamDir, IntersectPoint);
        IntersectA = FVector(IntersectPoint.X, IntersectPoint.Y, GridCentre.Z);
        FindLineIntersection(Point2, Dir, Centre2D, CamDir, IntersectPoint);
        IntersectB = FVector(IntersectPoint.X, IntersectPoint.Y, GridCentre.Z);

        //DrawDebugLine(GetWorld(), GetActorLocation(), IntersectA, FColor::Red, false, 10.f);
        //DrawDebugLine(GetWorld(), GetActorLocation(), IntersectB, FColor::Blue, false, 10.f);
        // Find direction to new centre
        FVector FrameEdgeDirA = IntersectA - GetActorLocation();
        FrameEdgeDirA.Normalize();
        FVector FrameEdgeDirB = IntersectB - GetActorLocation();
        FrameEdgeDirB.Normalize();
        const FVector NewCentreDir = (FrameEdgeDirA + FrameEdgeDirB) / 2.f;
        float Denominator = FVector::DotProduct(NewCentreDir, FVector::UpVector);
        float t = FVector::DotProduct((GridCentre - GetActorLocation()), FVector::UpVector) / Denominator;        
        AdjustedCentre = GetActorLocation() + t * NewCentreDir;

        // Adjust FOV calc
        float EdgeDistA = FVector::Distance(AdjustedCentre, IntersectA);
        float EdgeDistB = FVector::Distance(AdjustedCentre, IntersectB);
        FrameLength = FMath::Max(EdgeDistA, EdgeDistB);
    }

    // Find camera rotation
    FVector Forward = (AdjustedCentre - GetActorLocation()).GetSafeNormal();
    FVector RotatedUpDir = UpDirection.RotateAngleAxis(GridRotation, FVector::UpVector);
    FVector Right = FVector::CrossProduct(RotatedUpDir, Forward).GetSafeNormal();
    FVector AdjustedUp = FVector::CrossProduct(Forward, Right);
    FMatrix RotationMatrix = FMatrix(Forward, Right, AdjustedUp, FVector::ZeroVector);
    SetActorRotation(RotationMatrix.Rotator());
    CameraFocusPoint = AdjustedCentre;
    HandleDebugDraw();
    
    GridChildActor->SetWorldLocationAndRotation(GridCentre, FRotator());    
    GetGridActor()->SetParameters(Length, Width, TileSizeInCm, LineWidth, GridColour, GridRotation);
    
    // Calc FOV
    const float CamDist = FVector::Dist(GridCentre, GetActorLocation());
    const float AutoFOV = FMath::RadiansToDegrees(2 * FMath::Atan(FrameWidth / CamDist));

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

    if (UCameraComponent* Component = GetCameraComponent())
    {        
        Component->SetAspectRatio(FinalAspectRatio);
        Component->SetFieldOfView(FinalFOV);
    }
}

void AC0TTRPGCam::HandleDebugDraw()
{
    if (bDebugDrawLineToFocalPoint)
    {
        DrawDebugLine(GetWorld(), GetActorLocation(), CameraFocusPoint, FColor::Red, false, 1.f);
        FTimerHandle Handle;
        GetWorld()->GetTimerManager().SetTimer(Handle, this, &AC0TTRPGCam::HandleDebugDraw, 1.0f, false);
    }
}

AC0Grid* AC0TTRPGCam::GetGridActor()
{    
    return Cast<AC0Grid>(GridChildActor->GetChildActor());    
}
