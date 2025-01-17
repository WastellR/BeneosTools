// Copyright (c) CORE Games, Ltd. All rights reserved.

#include "C0Grid.h"

AC0Grid::AC0Grid() :
    Length(10),
    Width(10),
    TileSizeInCm(152.4),
    LineWidth(0.05f),
    GridColour(FColor(255, 255, 255, 51))
{
    PlaneMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("PlaneMeshComponent"));
    RootComponent = PlaneMeshComponent;

    // Load the default plane mesh
    static ConstructorHelpers::FObjectFinder<UStaticMesh> PlaneMesh(TEXT("/Engine/BasicShapes/Plane.Plane"));
    // Load custom grid material
    const FString MaterialPath = TEXT("/BeneosTools/M_GridMaterial.M_GridMaterial");
    UMaterial* Material = Cast<UMaterial>(StaticLoadObject(UMaterialInterface::StaticClass(), nullptr, *MaterialPath));
    if (PlaneMesh.Succeeded())
    {
        PlaneMeshComponent->SetStaticMesh(PlaneMesh.Object);
        //PlaneMeshComponent->SetMaterial(0, Material);
        DynamicMaterial = UMaterialInstanceDynamic::Create(Material, nullptr, FName()); 
        PlaneMeshComponent->SetMaterial(0, DynamicMaterial);
    }
}

void AC0Grid::OnConstruction(const FTransform& Transform)
{
    UpdateGrid();
}

FString AC0Grid::GetDefaultActorLabel() const
{
    return TEXT("BeneosGrid");
}

void AC0Grid::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);

    if (PropertyChangedEvent.Property &&
        (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AC0Grid, Length) ||
            PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AC0Grid, Width) ||
            PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AC0Grid, TileSizeInCm) ||
            PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AC0Grid, LineWidth) ||
            PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AC0Grid, GridColour)))
    {
        UpdateGrid();
    }
}

void AC0Grid::UpdateGrid()
{
    if (DynamicMaterial)
    {
        DynamicMaterial->SetScalarParameterValue("GridWidth", Width);
        DynamicMaterial->SetScalarParameterValue("GridLength", Length);
        DynamicMaterial->SetScalarParameterValue("LineWidth", LineWidth);
        DynamicMaterial->SetVectorParameterValue("GridColour", GridColour);
    }
    SetActorScale3D(FVector(Length * TileSizeInCm / 100.f, Width * TileSizeInCm / 100.f, 1.f));
}

void AC0Grid::SetParameters(const float InLength, const float InWidth,
    const float InTileSizeInCm, const float InLineWidth, const FColor& InGridColour)
{
    Length = InLength;
    Width = InWidth;
    TileSizeInCm = InTileSizeInCm;
    LineWidth = InLineWidth;
    GridColour = InGridColour;
    UpdateGrid();
}
