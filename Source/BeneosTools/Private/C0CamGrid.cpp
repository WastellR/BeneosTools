// Copyright (c) CORE Games, Ltd. All rights reserved.

#include "C0CamGrid.h"
#include "C0TTRPGCam.h"
#include "C0ChildCamComponent.h"

AC0CamGrid::AC0CamGrid() :        
    AdjustmentMode(EC0AdjustmentMode::Automatic),
    AspectRatio(0.f),
    FOV(0.f),
    GridMarginInTiles(0.f),
    bDebugDrawLineToFocalPoint(false),
    bShowPIP(true),
    bInitialized(false)
{
    BeneosCam = CreateDefaultSubobject<UC0ChildCamComponent>(TEXT("CamChild"));
    BeneosCam->SetupAttachment(RootComponent);
    BeneosCam->SetUsingAbsoluteLocation(false);
    BeneosCam->SetUsingAbsoluteRotation(false);
    BeneosCam->SetUsingAbsoluteScale(true);
    BeneosCam->SetRelativeLocationAndRotation(FVector(0.f, 0.f, 500.f), FRotator(-90.f, 0.f, 0.f));
}

void AC0CamGrid::OnConstruction(const FTransform& Transform)
{
    // Call initialization only the first time the actor is created in editor
    if (!bInitialized)
    {
        bInitialized = true;
        PrevAdjustmentMode = AdjustmentMode;
    }

    Super::OnConstruction(Transform);

    BeneosCam->SetChildActorClass(AC0TTRPGCam::StaticClass());
    BeneosCam->CreateChildActor();
    AC0TTRPGCam* Cam = Cast<AC0TTRPGCam>(BeneosCam->GetChildActor());
    Cam->SetParent(this);

    HandleDebugDraw();
}

FString AC0CamGrid::GetDefaultActorLabel() const
{
    return TEXT("Beneos Cam Grid");
}

void AC0CamGrid::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.Property)
    {
        if (PropertyChangedEvent.Property->GetName() == "bShowPIP")
        {
            if (GEditor)
            {
                GEditor->SelectActor(this, true, true);
            }
        }
    }
}

void AC0CamGrid::PostEditMove(bool bFinished)
{
    Super::PostEditMove(bFinished);

    BeneosCam->bMoveUpdate = bFinished;
}

void AC0CamGrid::ToggleGrid()
{
    RootComponent->SetVisibility(!RootComponent->GetVisibleFlag(), false);
}

#pragma region PIP

void AC0CamGrid::OnSelectActor(const bool bInPrevShowPIP)
{
    bPrevShowPIP = bInPrevShowPIP;
    SetShowPIP(bShowPIP);
}

void AC0CamGrid::OnDeselectActor()
{
    ULevelEditorViewportSettings* ViewportSettings = GetMutableDefault<ULevelEditorViewportSettings>();
    if (ViewportSettings)
    {
        SetShowPIP(bPrevShowPIP);
    }
}

void AC0CamGrid::SetShowPIP(const bool bShow)
{
    ULevelEditorViewportSettings* ViewportSettings = GetMutableDefault<ULevelEditorViewportSettings>();
    if (ViewportSettings)
    {
        ViewportSettings->bPreviewSelectedCameras = bShow;
        ViewportSettings->SaveConfig();
    }
}

#pragma endregion PIP

void AC0CamGrid::HandleDebugDraw()
{
    if (bDebugDrawLineToFocalPoint)
    {
        DrawDebugLine(GetWorld(), GetActorLocation(), BeneosCam->GetComponentLocation(), FColor::Red, false, 1.f);
        FTimerHandle Handle;
        GetWorld()->GetTimerManager().SetTimer(Handle, this, &AC0CamGrid::HandleDebugDraw, 1.0f, false);
    }
}
