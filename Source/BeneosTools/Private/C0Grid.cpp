// Copyright (c) CORE Games, Ltd. All rights reserved.

#include "C0Grid.h"

#include <Kismet/GameplayStatics.h>
#include "C0TTRPGCam.h"
#include "C0TTRPGCineCam.h"

AC0Grid::AC0Grid() :
    Length(10),
    Width(10),
    TileSizeInCm(152.4),
    LineWidth(0.05f),
    GridColour(FColor(255, 255, 255, 51)),
    bRenderOnTop(true),

    // Snapping
    bEnableSnapping(false),
    bUseActorSnapOrigin(true),
    SnapOriginActor(nullptr),
    SnapOriginVector(FVector::ZeroVector),
    SnapIncrement(152.4),
    bHalveSnapIncrement(false),
    PrevMoveX(0.f),
    PrevMoveY(0.f),
    GizmoMoveX(0.f),
    GizmoMoveY(0.f),
    bWasDragging(false)
{
    PlaneMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMeshComponent"));
    RootComponent = PlaneMeshComponent;

    // Load the default plane mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
    if (PlaneMesh.Succeeded())
    {
        PlaneMeshComponent->SetStaticMesh(PlaneMesh.Object);
		PlaneMeshComponent->SetCollisionEnabled(ECollisionEnabled::NoCollision);
    }
    LoadGridMaterial();

    FVector ActorLocation = GetActorLocation();
    StartMoveX = ActorLocation.X;
    StartMoveY = ActorLocation.Y;
}

void AC0Grid::OnConstruction(const FTransform& Transform)
{
    LoadGridMaterial();
    UpdateGrid();
}

FString AC0Grid::GetDefaultActorLabel() const
{
    return TEXT("BeneosGrid");
}

void AC0Grid::PostEditMove(bool bFinished)
{
    Super::PostEditMove(bFinished);

    // Handle snapping
    if (bEnableSnapping)
    {
        FVector NewLocation = GetActorLocation();
        FVector Origin = SnapOriginVector;
        float Increment = bHalveSnapIncrement ? SnapIncrement / 2.f : SnapIncrement;
        if (bUseActorSnapOrigin && SnapOriginActor)
        {
            Origin = SnapOriginActor->GetActorLocation();
        }
        if (!bWasDragging)
        {            
            PrevMoveX = NewLocation.X;
            PrevMoveY = NewLocation.Y;
        }
        bWasDragging = true;

        const float HalfIncrement = Increment / 2.f;
        float X = NewLocation.X - Origin.X;
        GizmoMoveX += X - PrevMoveX;
        float Remainder = FMath::Modulo(GizmoMoveX, Increment);
        X = StartMoveX + GizmoMoveX - Remainder;
        if (FMath::Abs(Remainder) > HalfIncrement)
        {
            X += Increment * FMath::Sign(Remainder);            
        }
        PrevMoveX = X;

        float Y = NewLocation.Y - Origin.Y;
        GizmoMoveY += Y - PrevMoveY;
        Remainder = FMath::Modulo(GizmoMoveY, Increment);
        Y = StartMoveY + GizmoMoveY - Remainder;
        if (FMath::Abs(Remainder) > HalfIncrement)
        {
            Y += Increment * FMath::Sign(Remainder);
        }
        PrevMoveY = Y;

        NewLocation.X = Origin.X + X;
        NewLocation.Y = Origin.Y + Y;
        SetActorLocation(NewLocation);
    }

    if (bFinished)
    {
        // Reset snapping variables
        GizmoMoveX = 0.f;
        GizmoMoveY = 0.f;
        bWasDragging = false;
        FVector NewLocation = GetActorLocation();
        StartMoveX = NewLocation.X;
        StartMoveY = NewLocation.Y;

        for (AC0TTRPGCam* Camera : ChildCameras)
        {
            Camera->UpdateCamera();
        }
        for (AC0TTRPGCineCam* CineCamera : ChildCineCameras)
        {
            CineCamera->UpdateCamera();
        }
    }
}

void AC0Grid::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.Property)
    {
        const FName PropertyName = PropertyChangedEvent.Property->GetFName();
        FProperty* MemberPropertyThatChanged = PropertyChangedEvent.MemberProperty;
        const FName MemberPropertyName = MemberPropertyThatChanged != NULL ? MemberPropertyThatChanged->GetFName() : NAME_None;
        if(PropertyName == GET_MEMBER_NAME_CHECKED(AC0Grid, Length) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(AC0Grid, Width) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(AC0Grid, TileSizeInCm) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(AC0Grid, LineWidth) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(AC0Grid, GridColour) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(AC0Grid, bRenderOnTop))
        {
            UpdateGrid();
        }
        else if (PropertyName == GET_MEMBER_NAME_CHECKED(AC0Grid, bEnableSnapping) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(AC0Grid, bUseActorSnapOrigin) ||
            PropertyName == GET_MEMBER_NAME_CHECKED(AC0Grid, SnapOriginActor) ||
            MemberPropertyName == "SnapOriginVector")
        {
            if(bEnableSnapping)
                SnapToIncrement();
        }        
        static const FName Name_RelativeLocation = USceneComponent::GetRelativeLocationPropertyName();
        // If location changed
        if (MemberPropertyName == Name_RelativeLocation)
        {
            SnapToIncrement();            
        }
    }
}

void AC0Grid::LoadGridMaterial(const bool bRedo)
{
    if (!DynamicMaterial ||
        PlaneMeshComponent->GetMaterial(0)->GetFullName().Contains(FString("WorldGrid"))
        || bRedo)
    {
        // Load custom grid material
        const FString MaterialPath = TEXT("/BeneosTools/M_GridMaterial.M_GridMaterial");
        UMaterial* Material = Cast<UMaterial>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MaterialPath));
        Material->bDisableDepthTest = bRenderOnTop;

        //PlaneMeshComponent->SetMaterial(0, Material);
        DynamicMaterial = UMaterialInstanceDynamic::Create(Material, nullptr, FName());
        PlaneMeshComponent->SetMaterial(0, DynamicMaterial);       
    }
}

void AC0Grid::SnapToIncrement()
{
    FVector NewLocation = GetActorLocation();
    FVector Origin = SnapOriginVector;
    float Increment = bHalveSnapIncrement ? SnapIncrement / 2.f : SnapIncrement;
    if (bUseActorSnapOrigin && SnapOriginActor)
    {
        Origin = SnapOriginActor->GetActorLocation();
    }
    const float HalfIncrement = Increment / 2.f;

    float X = NewLocation.X - Origin.X;
    float Remainder = FMath::Modulo(X, Increment);
    X -= Remainder;
    if (FMath::Abs(Remainder) > HalfIncrement)
    {
        X += Increment * FMath::Sign(Remainder);
    }

    float Y = NewLocation.Y - Origin.Y;
    Remainder = FMath::Modulo(Y, Increment);
    Y -= Remainder;
    if (FMath::Abs(Remainder) > HalfIncrement)
    {
        Y += Increment * FMath::Sign(Remainder);
    }

    NewLocation.X = Origin.X + X;
    NewLocation.Y = Origin.Y + Y;
    SetActorLocation(NewLocation);
    StartMoveX = NewLocation.X;
    StartMoveY = NewLocation.Y;
}

void AC0Grid::UpdateGrid()
{
    if (DynamicMaterial)
    {
        static bool bLastRenderOnTop = true;
        if (bLastRenderOnTop != bRenderOnTop)
        {
            LoadGridMaterial(true);
        }
        bLastRenderOnTop = bRenderOnTop;

        DynamicMaterial->SetScalarParameterValue("GridWidth", Width);
        DynamicMaterial->SetScalarParameterValue("GridLength", Length);
        DynamicMaterial->SetScalarParameterValue("LineWidth", LineWidth);
        DynamicMaterial->SetVectorParameterValue("GridColour", GridColour);
    }    
    SetActorScale3D(FVector(Length * TileSizeInCm / 100.f, Width * TileSizeInCm / 100.f, 1.f));
    for (AC0TTRPGCam* Camera : ChildCameras)
    {
        Camera->UpdateCamera();
    }
    for (AC0TTRPGCineCam* CineCamera : ChildCineCameras)
    {
        CineCamera->UpdateCamera();
    }
}

void AC0Grid::SetGridVisibility(const bool bVisible)
{
    RootComponent->SetVisibility(bVisible, false);
}

void AC0Grid::ToggleVisibility()
{
    RootComponent->SetVisibility(!RootComponent->GetVisibleFlag(), false);
}

void AC0Grid::ApplyGlobally()
{
    TArray<AActor*> OutActors;
    UGameplayStatics::GetAllActorsOfClass(GetWorld(), AC0Grid::StaticClass(), OutActors);
    for (AActor* Actor : OutActors)
    {
        if (Actor == this)
            continue;

        AC0Grid* GridActor = Cast<AC0Grid>(Actor);
        GridActor->GridColour = GridColour;
        GridActor->LineWidth = LineWidth;
        GridActor->SetGridVisibility(RootComponent->GetVisibleFlag());
        GridActor->UpdateGrid();
    }
}
