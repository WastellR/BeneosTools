// Copyright (c) CORE Games, Ltd. All rights reserved.

#include "C0TTRPGCam.h"

#include <Camera/CameraComponent.h>

#include <Kismet/KismetMathLibrary.h>
#include "C0CamGrid.h"

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
    GridMarginInTiles(0.f),
    Length(10),
    Width(10),
    TileSizeInCm(152.4)
{
    SetActorRotation(FRotator(-90.f, 0.f, 0.f));
}

void AC0TTRPGCam::OnConstruction(const FTransform& Transform)
{
    Super::OnConstruction(Transform);
    
    UpdateCamera();
}

void AC0TTRPGCam::PostEditComponentMove(bool bFinished)
{
    if (Parent)
    {
        FVector PointToLine = GetActorLocation() - Parent->GetActorLocation();
        FVector Dir = PointToLine;
        Dir.Normalize();
        if (Dir != Parent->GetActorUpVector())
        {
            float ProjectionLength = FVector::DotProduct(PointToLine, Parent->GetActorUpVector());
            FVector ProjectionVector = Parent->GetActorUpVector() * ProjectionLength;
            FVector ProjectedPoint = Parent->GetActorLocation() + ProjectionVector;
            SetActorLocation(ProjectedPoint);
            if (GEditor)
            {
                GEditor->SetPivot(ProjectedPoint, false, true, true);
            }
        }
    }

    UpdateCamera();
}

FString AC0TTRPGCam::GetDefaultActorLabel() const
{
    return TEXT("Beneos Cam");
}

void AC0TTRPGCam::SetParent(AC0CamGrid* InParent)
{
    Parent = InParent;
    Length = Parent->Length;
    Width = Parent->Width;
    TileSizeInCm = Parent->TileSizeInCm;   
    GridMarginInTiles = Parent->GridMarginInTiles;
    UpdateCamera();
}

void AC0TTRPGCam::UpdateCamera()
{
    if (!Parent)
        return;

    const float Margin = GridMarginInTiles * 2.f;
    const float AutoAspectRatio = (Width + Margin) / (Length + Margin);
    const float CamDist = FVector::Dist(Parent->GetActorLocation(), GetActorLocation());
    const float HalfGridWidth = (Width + Margin) * TileSizeInCm / 2.f;
    const float AutoFOV = FMath::RadiansToDegrees(2 * FMath::Atan(HalfGridWidth / CamDist));
    // Adjust FOV + aspect ratio based on mode
    if (Parent->AdjustmentMode != Parent->PrevAdjustmentMode)
    {
        if (Parent->AdjustmentMode == EC0AdjustmentMode::Automatic)
        {
            Parent->AspectRatio = 0.f;
            Parent->FOV = 0.f;
        }
        else if (Parent->AdjustmentMode == EC0AdjustmentMode::Additional &&
            Parent->PrevAdjustmentMode == EC0AdjustmentMode::Manual)
        {            
            Parent->AspectRatio -= AutoAspectRatio;
            Parent->FOV -= AutoFOV;
        }
        else if (Parent->AdjustmentMode == EC0AdjustmentMode::Manual)
        {
            if (Parent->PrevAdjustmentMode == EC0AdjustmentMode::Automatic)
            {
                Parent->AspectRatio = AutoAspectRatio;
                Parent->FOV = AutoFOV;
            }
            else if (Parent->PrevAdjustmentMode == EC0AdjustmentMode::Additional)
            {
                Parent->AspectRatio += AutoAspectRatio;
                Parent->FOV += AutoFOV;
            }
        }

        Parent->PrevAdjustmentMode = Parent->AdjustmentMode;
    }

    float FinalAspectRatio = Parent->AspectRatio;
    float FinalFOV = Parent->FOV;
    if (Parent->AdjustmentMode == EC0AdjustmentMode::Automatic)
    {
        FinalAspectRatio = AutoAspectRatio;
        FinalFOV = AutoFOV;
    }
    else if (Parent->AdjustmentMode == EC0AdjustmentMode::Additional)
    {
        FinalAspectRatio = AutoAspectRatio + Parent->AspectRatio;
        FinalFOV = AutoFOV + Parent->FOV;
    }
    else if (Parent->AdjustmentMode == EC0AdjustmentMode::Manual)
    {
        FinalAspectRatio = Parent->AspectRatio;
        FinalFOV = Parent->FOV;
    }

    if (UCameraComponent* Component = GetCameraComponent())
    {
        Component->SetAspectRatio(FinalAspectRatio);
        Component->SetFieldOfView(FinalFOV);
    }
}
