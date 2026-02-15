// Copyright (c) CORE Games, Ltd. All rights reserved.

#include "C0ChaseTool.h"

#include <Components/BoxComponent.h>
#include <Landscape.h>
#include <LandscapeStreamingProxy.h>
#include <Engine/OverlapResult.h>
#include <UnrealEdGlobals.h>
#include <Editor/UnrealEdEngine.h>
#include <Selection.h>
#include <Camera/CameraActor.h>

#include "C0Grid.h"
#include "C0BoxComponent.h"

AC0ChaseTool::AC0ChaseTool() :
	BoxSize(FVector(500.f)),
	LoopDuration(10.f),
	PathDistanceInCm(1000.f),
	bDrawCameraPath(true),
	PathColour(FColor::White),

	CameraHeight(5000.f),
	CameraChasePosition(0.f),
	CameraChasePause(3.f),

	bBoxAGrabContents(true),
	bBoxBGrabContents(true),
	bMoveHasStarted(false),
	bMoveBoxBHasStarted(false)
{
	PrimaryActorTick.bCanEverTick = true;	

	BoxA = CreateDefaultSubobject<UC0BoxComponent>(TEXT("BoxA"));
	BoxB = CreateDefaultSubobject<UC0BoxComponent>(TEXT("BoxB"));
	SetRootComponent(BoxA);
	BoxB->AttachToComponent(BoxA, FAttachmentTransformRules::KeepRelativeTransform);

	// Box components setup
	BoxA->SetBoxExtent(BoxSize);
	BoxA->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxA->SetCollisionObjectType(ECC_WorldDynamic);
	BoxA->SetCollisionResponseToAllChannels(ECR_Overlap);
	BoxA->SetGenerateOverlapEvents(true);
	BoxA->OnPostEditChangeProperty.AddUObject(
		this,
		&AC0ChaseTool::OnBoxAEditChangedProperty
	);

	BoxB->SetRelativeLocation(FVector(1000.f, 0.f, 0.f));
	BoxB->SetBoxExtent(BoxSize);
	BoxB->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	BoxB->SetCollisionObjectType(ECC_WorldDynamic);
	BoxB->SetCollisionResponseToAllChannels(ECR_Overlap);
	BoxB->SetGenerateOverlapEvents(true);
	BoxB->OnPostEditComponentMove.AddUObject(
		this,
		&AC0ChaseTool::OnBoxBEditComponentMoved
	);
	BoxB->OnPostEditChangeProperty.AddUObject(
		this,
		&AC0ChaseTool::OnBoxBEditChangedProperty
	);
}

void AC0ChaseTool::PostActorCreated()
{
	Super::PostActorCreated();

	StartMoveLocation = GetActorLocation();
	BoxBStartMoveLocation = BoxB->GetComponentLocation();
}

FString AC0ChaseTool::GetDefaultActorLabel() const
{
	return TEXT("BeneosChaseTool");
}

void AC0ChaseTool::PostEditMove(bool bFinished)
{
	Super::PostEditMove(bFinished);

	FVector Delta = GetActorLocation() - StartMoveLocation;

	if (bFinished)
	{
		StartMoveLocation = GetActorLocation();
		BoxBStartMoveLocation = BoxB->GetComponentLocation();
		bMoveHasStarted = false;
	}
	else if (bBoxAGrabContents)
	{
		if (!bMoveHasStarted)
		{
			// Get Box A overlapping actors
			TArray<FOverlapResult> Overlaps;
			GetBoxOverlappingActors(Overlaps, *BoxA);

			BoxAGrabActorLocations.Empty();
			for (const FOverlapResult& Overlap : Overlaps)
			{
				AActor* Actor = Overlap.GetActor();
				if (IsGrabbable(Actor))
				{
					BoxAGrabActorLocations.Add(Actor, Actor->GetActorLocation());
				}
			}
		}

		for (TPair<AActor*, FVector>& Pair : BoxAGrabActorLocations)
		{
			if (Pair.Key && IsValid(Pair.Key))
			{
				Pair.Key->SetActorLocation(Pair.Value + Delta);
			}
		}		
		bMoveHasStarted = true;
	}
}

void AC0ChaseTool::OnBoxAEditChangedProperty(FName PropertyName, FName MemberPropertyName)
{	
	if (MemberPropertyName == FName("BoxExtent"))
	{
		BoxSize = BoxA->GetUnscaledBoxExtent();
	}
}

void AC0ChaseTool::OnBoxBEditComponentMoved(bool bFinished)
{
	FVector Delta = BoxB->GetComponentLocation() - BoxBStartMoveLocation;

	if (bFinished)
	{
		BoxBStartMoveLocation = BoxB->GetComponentLocation();
		bMoveBoxBHasStarted = false;
		PathDistanceInCm = (BoxB->GetComponentLocation() - BoxA->GetComponentLocation()).Length();
	}
	else if (bBoxBGrabContents)
	{		
		if (!bMoveBoxBHasStarted)
		{
			// Get Box A overlapping actors
			TArray<FOverlapResult> Overlaps;
			GetBoxOverlappingActors(Overlaps, *BoxB);

			BoxBGrabActorLocations.Empty();
			for (const FOverlapResult& Overlap : Overlaps)
			{
				AActor* Actor = Overlap.GetActor();
				if (IsGrabbable(Actor))
				{
					BoxBGrabActorLocations.Add(Actor, Actor->GetActorLocation());
				}
			}
		}

		for (TPair<AActor*, FVector>& Pair : BoxBGrabActorLocations)
		{
			if (Pair.Key && IsValid(Pair.Key))
			{
				Pair.Key->SetActorLocation(Pair.Value + Delta);
			}
		}
		bMoveBoxBHasStarted = true;
	}
}

void AC0ChaseTool::OnBoxBEditChangedProperty(FName PropertyName, FName MemberPropertyName)
{
	if (MemberPropertyName == USceneComponent::GetRelativeLocationPropertyName())
	{
		PathDistanceInCm = (BoxB->GetComponentLocation() - BoxA->GetComponentLocation()).Length();
	}
}

// Called whenever a property is changed in the editor
void AC0ChaseTool::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property->GetFName();
	FProperty* MemberPropertyThatChanged = PropertyChangedEvent.MemberProperty;
	const FName MemberPropertyName = MemberPropertyThatChanged != NULL ? MemberPropertyThatChanged->GetFName() : NAME_None;	
	static const FName Name_RelativeLocation = USceneComponent::GetRelativeLocationPropertyName();
	// If location changed
	if (MemberPropertyName == Name_RelativeLocation)
	{
		FVector Delta = GetActorLocation() - StartMoveLocation;
		TArray<FOverlapResult> OverlapsA;
		TArray<FOverlapResult> OverlapsB;
		if (bBoxAGrabContents)
		{			
			GetBoxOverlappingActors(OverlapsA, *BoxA, Delta);			
		}
		if (bBoxBGrabContents)
		{
			GetBoxOverlappingActors(OverlapsB, *BoxB, Delta);
		}
		OverlapsA += OverlapsB;
		for (const FOverlapResult& Overlap : OverlapsA)
		{
			AActor* Actor = Overlap.GetActor();
			if (!IsGrabbable(Actor))
				continue;

			Actor->SetActorLocation(Actor->GetActorLocation() + Delta);
		}

		StartMoveLocation = GetActorLocation();
	}
	if (PropertyName == FName("Camera") || PropertyName == FName("CameraHeight") || PropertyName == FName("CameraChasePosition"))
	{
		if (Camera && IsValid(Camera))
		{
			Camera->SetActorLocation(GetCameraPos());
		}
	}
	if (PropertyName == FName("LoopDuration") || PropertyName == FName("PathDistanceInCm"))
	{
		if (bIsRunningChasePreview)
		{
			ChasePreviewStart = GetWorld()->GetTimeSeconds();
		}
		// Update speed display
		EditorUIUpdate.Broadcast();
	}
	if (PropertyName == FName("PathDistanceInCm"))
	{
		FVector Direction = BoxB->GetComponentLocation() - BoxA->GetComponentLocation();
		Direction.Normalize();
		BoxB->SetWorldLocation(BoxA->GetComponentLocation() + Direction * PathDistanceInCm);
	}
	if (MemberPropertyName == FName("BoxSize"))
	{
		BoxA->SetBoxExtent(BoxSize);
		BoxB->SetBoxExtent(BoxSize);
	}
}

void AC0ChaseTool::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (bIsRunningChasePreview)
	{
		float TimeSinceStart = GetWorld()->GetTimeSeconds() - ChasePreviewStart;
		const float TotalLoop = LoopDuration + CameraChasePause;
		if (TimeSinceStart >= TotalLoop)
		{
			TimeSinceStart -= TotalLoop;
			ChasePreviewStart += TotalLoop;
		}
		
		CameraChasePosition = FMath::Min(TimeSinceStart / LoopDuration, 1.f);
		if (Camera && IsValid(Camera))
		{
			Camera->SetActorLocation(GetCameraPos());
		}
	}
	if (IsActorOrSelectionParentSelected() && bDrawCameraPath)
	{
		DrawDebugLine(GetWorld(), BoxA->GetComponentLocation(), BoxB->GetComponentLocation(), PathColour);
	}
}

FText AC0ChaseTool::GetSpeedText() const
{
	const float SpeedCmS = PathDistanceInCm / LoopDuration;
	const float SpeedKmH = SpeedCmS * 0.036f;
	return FText::FromString(FString::Printf(TEXT("%.2f km/h"), SpeedKmH));
}

void AC0ChaseTool::ToggleChasePreview()
{
	bIsRunningChasePreview = !bIsRunningChasePreview;
	if (bIsRunningChasePreview)
	{
		ChasePreviewStart = GetWorld()->GetTimeSeconds();
	}
}

const FVector AC0ChaseTool::GetCameraPos() const
{
	return BoxA->GetComponentLocation() 
		+ (CameraChasePosition * (BoxB->GetComponentLocation() - BoxA->GetComponentLocation()))
		+ (FVector::UpVector * CameraHeight);
}

void AC0ChaseTool::CopyAToB()
{
	TArray<FOverlapResult> OverlapsA;
	GetBoxOverlappingActors(OverlapsA, *BoxA);
	TArray<FOverlapResult> OverlapsB;
	GetBoxOverlappingActors(OverlapsB, *BoxB);
	TArray<AActor*> DeleteActors;
	for (FOverlapResult& Overlap : OverlapsB)
	{
		AActor* Actor = Overlap.GetActor();
		if (IsGrabbable(Actor))
		{
			DeleteActors.Add(Actor);
		}		
	}
	GUnrealEd->DeleteActors(DeleteActors, GetWorld(), GUnrealEd->GetSelectedActors()->GetElementSelectionSet());

	for (FOverlapResult& Overlap : OverlapsA)
	{
		AActor* Actor = Overlap.GetActor();
		if (!IsGrabbable(Actor))
			continue;

		TArray<AActor*> NewActors;
		GUnrealEd->DuplicateActors({ Actor }, NewActors, GetWorld()->GetCurrentLevel(), BoxB->GetComponentLocation() - BoxA->GetComponentLocation());
	}
}

void AC0ChaseTool::CopyBToA()
{
	TArray<FOverlapResult> OverlapsA;
	GetBoxOverlappingActors(OverlapsA, *BoxA);
	TArray<FOverlapResult> OverlapsB;
	GetBoxOverlappingActors(OverlapsB, *BoxB);
	TArray<AActor*> DeleteActors;
	for (FOverlapResult& Overlap : OverlapsA)
	{
		AActor* Actor = Overlap.GetActor();
		if (IsGrabbable(Actor))
		{
			DeleteActors.Add(Actor);
		}
	}
	GUnrealEd->DeleteActors(DeleteActors, GetWorld(), GUnrealEd->GetSelectedActors()->GetElementSelectionSet());

	for (FOverlapResult& Overlap : OverlapsB)
	{
		AActor* Actor = Overlap.GetActor();
		if (!IsGrabbable(Actor))
			continue;

		TArray<AActor*> NewActors;
		GUnrealEd->DuplicateActors({ Actor }, NewActors, GetWorld()->GetCurrentLevel(), BoxA->GetComponentLocation() - BoxB->GetComponentLocation());
	}
}

void AC0ChaseTool::GetBoxOverlappingActors(TArray<FOverlapResult>& OverlapsOut, const UBoxComponent& Box, FVector BoxDelta)
{
	FCollisionQueryParams Params(
		SCENE_QUERY_STAT(EditorBoxOverlap),
		false,
		GetOwner()
	);
	FCollisionObjectQueryParams ObjectParams;
	ObjectParams.AddObjectTypesToQuery(ECC_WorldStatic);
	ObjectParams.AddObjectTypesToQuery(ECC_WorldDynamic);
	ObjectParams.AddObjectTypesToQuery(ECC_Pawn);

	const FVector BoxExtent = Box.GetScaledBoxExtent();
	FTransform BoxTransform = Box.GetComponentTransform();
	BoxTransform.SetLocation(BoxTransform.GetLocation() - BoxDelta);

	bool bHit = GetWorld()->OverlapMultiByObjectType(
		OverlapsOut,
		BoxTransform.GetLocation(),
		BoxTransform.GetRotation(),
		ObjectParams,
		FCollisionShape::MakeBox(BoxExtent),
		Params
	);
}

const bool AC0ChaseTool::IsGrabbable(AActor* Actor)
{
	return !(Actor == this 
		|| Actor->IsA<ALandscape>() 
		|| Actor->IsA<ALandscapeProxy>() 
		|| Actor->IsA<ALandscapeStreamingProxy>() 
		|| Actor->IsA<AC0Grid>());
}
