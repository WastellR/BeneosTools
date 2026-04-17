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
    bHalveSnapIncrement(false)
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

    if (bFinished)
    {
		SnapToIncrement();

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
	if (!bEnableSnapping)
		return;

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
