#include "C0Torch.h"

#include <Components/PointLightComponent.h>
#include <Kismet/KismetMathLibrary.h>
#include <Math/UnrealMathUtility.h>
#include <Components/SplineComponent.h>
#include <Curves/CurveLinearColor.h>

// Sets default values
AC0Torch::AC0Torch() :
	MoveRadiusInCm(100.0f),
	LoopDurationInSeconds(0.f),
	MoveSpeed(5.f),
	Curviness(0.5f),
	PathA(FVector::ZeroVector),
	bDebugDrawRadius(false),

	bAnimateIntensity(false),
	IntensityMinInCandelas(4.f),
	IntensityMaxInCandelas(12.f),
	IntensityVariance(0.f),
	IntensityPeriodInSeconds(1.f),
	IntensityPeriodVariance(0.f),
	IntensitySmoothness(0.f),

	bAnimateAttenuation(false),
	AttenuationRadiusMin(800.f),
	AttenuationRadiusMax(1000.f),
	AttenuationVariance(0.f),
	AttenuationPeriodInSeconds(1.f),
	AttenuationPeriodVariance(0.f),
	AttenuationSmoothness(0.f),

	bAnimateColours(false),

	PeriodStart(0.f),
	PeriodLength(0.f),
	StartIntensity(-1.f),
	NextPeriodIsMax(false),
	bInitialized(false)
{
    // Set this actor to call Tick() every frame
    PrimaryActorTick.bCanEverTick = true;

    // Create the Point Light Component
	SceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("SceneComponent"));
	RootComponent = SceneComponent;
    PointLight = CreateDefaultSubobject<UPointLightComponent>(TEXT("PointLight"));
	PointLight->SetupAttachment(RootComponent);

    // Create the Spline Component
    SplineComponent = CreateDefaultSubobject<USplineComponent>(TEXT("SplineComponent"));
    SplineComponent->SetupAttachment(RootComponent);
	SplineComponent->bDrawDebug = false;

	// Load default light colours
	const FString ColoursPath = TEXT("/BeneosTools/C_TorchColoursExample.C_TorchColoursExample");
	if (UCurveLinearColor* DefaultColours = Cast<UCurveLinearColor>(StaticLoadObject(UCurveLinearColor::StaticClass(), nullptr, *ColoursPath)))
	{
		LightColours = DefaultColours;
	}
}

FVector Bezier(const FVector& A, const FVector& B, const FVector& C, const float G, const float T)
{
	const FVector M = A + (C - A) * G;
	return A * ((1 - T) * (1 - T)) + M * (2 * (1 - T) * T) + B * (T * T);
}

// Called every frame
void AC0Torch::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	// Comment out the existing path moving code
	/*
	if (MoveRadiusInCm > 0.f && MoveSpeed > 0.f)
	{		
		if (LoopDurationInSeconds > 0.f)
		{
			PathT += DeltaTime;			
			if (PathT >= CurSectionDuration)
			{
				PathT -= CurSectionDuration;
				NextLoopPath();
				Temp = false;
			}
			float T = PathT / CurSectionDuration;						
			// Smooth out movement by averaging straight line + bezier curve
			FVector PointA = PathA + MidpointA / 2.f;
			FVector PointB = PathB + MidpointB / 2.f;
			FVector X = Bezier(PointA, PointB, PathA + PathB / 2.f, Curviness, T);			

			FVector P = FMath::Lerp(PathA, PathB, T);
			FVector P2 = FMath::Lerp(MidpointA, MidpointB, T);
			FVector P3 = FMath::Lerp(P2, P, FMath::Abs(T - 0.5f) * 2.f);
			FVector P4 = Bezier(PathA, PathB, PathC, Curviness, T);

			FVector P5 = (P + X) / 2.f;
			PointLight->SetRelativeLocation(P5);
			if (T >= 0.5f && !Temp)
			{
				Temp = true;
				//DrawDebugLine(GetWorld(), GetActorLocation() + P, GetActorLocation() + PathC, FColor::Cyan, false, CurSectionDuration / 2.f);
			}
		}
	}
	else
	{
		PointLight->SetRelativeLocation(FVector::ZeroVector);
	}
	*/
	
	if (LoopDurationInSeconds > 0.f)
	{
		LoopT += DeltaTime;
		if (LoopT > LoopDurationInSeconds)
		{
			LoopT -= LoopDurationInSeconds;
		}
		const float T = LoopT / LoopDurationInSeconds;
		// Spline-based movement
		if (MoveRadiusInCm > 0.f && MoveSpeed > 0.f)
		{			
			FVector SplineLocation = SplineComponent->GetLocationAtTime(T, ESplineCoordinateSpace::Local, true);
			PointLight->SetRelativeLocation(SplineLocation);
		}
		// Interpolate light colour
		if (bAnimateColours)
		{
			PointLight->SetLightColor(LightColours->GetClampedLinearColorValue(T));
		}
	}
	else if (MoveRadiusInCm > 0.f && MoveSpeed > 0.f)
	{
		// Old non-loop movement
		PathT += DeltaTime * MoveSpeed;
		if (PathT >= FirstHalfDuration)
		{
			GenPath();
		}
		float T = PathT / FirstHalfDuration;
		float T2 = PathT / TotalDuration;
		FVector LinearPoint = FMath::Lerp(PathA, PathB, T);
		FVector CurvedPoint = FMath::Lerp(
			LinearPoint,
			FMath::Lerp(PathB, PathC, T),
			T2);
		FVector PointOnCurve = FMath::Lerp(LinearPoint, CurvedPoint, Curviness);
		PointLight->SetRelativeLocation(PointOnCurve);
	}
	else
	{
		PointLight->SetRelativeLocation(FVector::ZeroVector);
	}

	if (bAnimateIntensity)
	{
		const float RemainingTime = LoopDurationInSeconds - LoopT;
		float TimeSinceStart = GetWorld()->GetTimeSeconds() - PeriodStart;
		if (TimeSinceStart >= PeriodLength)
		{
			TimeSinceStart = 0.f;
			PeriodStart = GetWorld()->GetTimeSeconds();			
			PeriodLength = FMath::RandRange(-IntensityPeriodVariance, IntensityPeriodVariance) * IntensityPeriodInSeconds
				+ IntensityPeriodInSeconds;
			if(LoopDurationInSeconds > 0.f && RemainingTime < PeriodLength)
			{
				PeriodLength = RemainingTime;
				StartIntensity = EndIntensity;
				EndIntensity = FMath::Max(0.f, (IntensityMinInCandelas + IntensityMaxInCandelas) / 2.f);
			}
			else{
				const float ClampedMin = FMath::Max(0.f, IntensityMinInCandelas);
				const float ClampedMax = FMath::Max(ClampedMin, IntensityMaxInCandelas);
				const float Diff = ClampedMax - ClampedMin;
				if (StartIntensity < 0.f)
				{
					StartIntensity = FMath::Max(0.f, (IntensityMinInCandelas + IntensityMaxInCandelas) / 2.f);
				}
				else
				{
					StartIntensity = EndIntensity;
				}

				EndIntensity = NextIntensity(NextPeriodIsMax, Diff);
				NextPeriodIsMax = !NextPeriodIsMax;
			}
		}

		const float LinearT = TimeSinceStart / PeriodLength;
		float SmoothT = 1.f - (FMath::Sin(PI * (LinearT - PI / 2.f)) + 1.f) / 2.f;
		const float T = FMath::Lerp(LinearT, SmoothT, IntensitySmoothness);
		PointLight->SetIntensity(FMath::Lerp(StartIntensity, EndIntensity, T));
	}

	if (bAnimateAttenuation)
	{
		const float RemainingTime = LoopDurationInSeconds - LoopT;
		float TimeSinceStart = GetWorld()->GetTimeSeconds() - AttPeriodStart;
		if (TimeSinceStart >= AttPeriodLength)
		{
			TimeSinceStart = 0.f;
			AttPeriodStart = GetWorld()->GetTimeSeconds();
			AttPeriodLength = FMath::RandRange(-AttenuationPeriodVariance, AttenuationPeriodVariance) * AttenuationPeriodInSeconds
				+ AttenuationPeriodInSeconds;
			
			if (LoopDurationInSeconds > 0.f && RemainingTime < AttPeriodLength)
			{
				AttPeriodLength = RemainingTime;
				AttStart = AttEnd;
				AttEnd = FMath::Max(0.f, (AttenuationRadiusMin + AttenuationRadiusMax) / 2.f);
			}
			else
			{
				const float ClampedMin = FMath::Max(0.f, AttenuationRadiusMin);
				const float ClampedMax = FMath::Max(ClampedMin, AttenuationRadiusMax);
				const float Diff = ClampedMax - ClampedMin;
				if (AttStart < 0.f)
				{
					AttStart = FMath::Max(0.f, (AttenuationRadiusMin + AttenuationRadiusMax) / 2.f);
				}
				else
				{
					AttStart = AttEnd;
				}

				AttEnd = NextAttenuation(AttNextPeriodIsMax, Diff);
				AttNextPeriodIsMax = !AttNextPeriodIsMax;
			}
		}

		const float LinearT = TimeSinceStart / AttPeriodLength;
		float SmoothT = 1.f - (FMath::Sin(PI * (LinearT - PI / 2.f)) + 1.f) / 2.f;
		const float T = FMath::Lerp(LinearT, SmoothT, AttenuationSmoothness);
		const float Radius = FMath::Lerp(AttStart, AttEnd, T);
		PointLight->SetAttenuationRadius(Radius);
	}	
}

void AC0Torch::OnConstruction(const FTransform& Transform)
{	
	Super::OnConstruction(Transform);

	HandleDebugDraw();
	if (MoveRadiusInCm == 0.f || MoveSpeed == 0.f)
	{
		PointLight->SetRelativeLocation(FVector::ZeroVector);
	}
}

void AC0Torch::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property)
	{
		if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AC0Torch, bAnimateIntensity))
		{
			if (bAnimateIntensity)
			{
				PrevIntensity = PointLight->Intensity;
			}
			else
			{
				PointLight->SetIntensity(PrevIntensity);
			}
		}
		else if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AC0Torch, bAnimateAttenuation))
		{
			if (bAnimateAttenuation)
			{
				PrevAttenuation = PointLight->AttenuationRadius;
			}
			else
			{
				PointLight->SetAttenuationRadius(PrevAttenuation);
			}
		}
		else if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AC0Torch, bAnimateColours))
		{
			if (bAnimateColours)
			{
				PrevLightColour = PointLight->LightColor;
			}
			else
			{
				PointLight->SetLightColor(PrevLightColour);
			}
		}
	}
	if (LoopDurationInSeconds > 0.f)
	{
		LoopT = 0.f;
		GenSplinePath();
	}
}

bool AC0Torch::ShouldTickIfViewportsOnly() const
{
	return true;
}

FString AC0Torch::GetDefaultActorLabel() const
{
	return TEXT("Beneos Torch");
}

const FVector AC0Torch::RandomPointOnSphere()
{
    // Generate a random vector of length MoveRadiusInCm
	return FMath::VRand() * MoveRadiusInCm;
}

void AC0Torch::GenPath()
{
    if (PathA.IsZero())
    {
        PathA = RandomPointOnSphere();
        PathB = RandomPointOnSphere();
    }
	else
	{
		PathA = PointLight->GetRelativeLocation();
		PathB = PathC;
	}
    PathC = RandomPointOnSphere();
	FVector TangentA = (PathB - PathA).GetSafeNormal();
	FVector TangentB = (PathC - PathB).GetSafeNormal();
	FVector Cross = FVector::CrossProduct(TangentA, TangentB);
	const float FirstHalfLength = FVector::Dist(PathA, PathB);
	const float TotalLength = FVector::Dist(PathB, PathC) + FirstHalfLength;
	TangentA = TangentA.RotateAngleAxis(90.f * Curviness, Cross);
	TangentB = TangentB.RotateAngleAxis(-90.f * Curviness, Cross);
    //TotalPathLength = FirstHalfLength + FVector::Dist(PathB, PathC);
	FirstHalfDuration = FirstHalfLength / MoveSpeed;
	TotalDuration = TotalLength / MoveSpeed;
    PathT = 0.f;
}

void AC0Torch::GenLoopPath()
{
	const float MoveSpeedCmS = MoveSpeed * 10.f;
	PathA = RandomPointOnSphere();
	PathB = RandomPointOnSphere();
	PathC = RandomPointOnSphere();
	float PathDuration = 0.f;
	Path.Empty();
	PathDurations.Empty();
	Path.Add(PathA);

	// If the chosen settings are too small to find a loop, 
	// Make a manual one and print a warning to the screen
	float FirstHalfLength = FVector::Dist(PathA, PathB);
	FirstHalfDuration = FirstHalfLength / MoveSpeedCmS;
	if (FirstHalfDuration > LoopDurationInSeconds)
	{
		const float DesiredLength = (LoopDurationInSeconds * MoveSpeedCmS) / 2.f;
		PathB = PathA - PathB.GetSafeNormal() * DesiredLength;
		Path.Add(PathB);
		PathDurations.Add(LoopDurationInSeconds / 2.f);
		PathDurations.Add(LoopDurationInSeconds / 2.f);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 10.0f, FColor::Yellow, TEXT("Warning: Speed and loop length create a very short loop. Will probably look weird."));
		}
	}
	else
	{
		while (PathDuration < LoopDurationInSeconds)
		{
			FirstHalfLength = FVector::Dist(PathA, PathB);
			FirstHalfDuration = FirstHalfLength / MoveSpeedCmS;

			Path.Add(PathB);
			PathDurations.Add(FirstHalfDuration);
			PathDuration += FirstHalfDuration;
			PathA = PathB;
			PathB = PathC;
			PathC = RandomPointOnSphere();
		}
		Path.RemoveAt(Path.Num() - 1);
		PathDurations.RemoveAt(PathDurations.Num() - 1);
		PathDuration -= FirstHalfDuration;

		float RemainingDistance = (LoopDurationInSeconds - PathDuration) * MoveSpeedCmS;
		float LastDistance = FVector::Dist(Path.Last(), Path[0]);

		// Remove last section of the path until the closed loop is less than the target loop duration
		while (LastDistance > RemainingDistance && Path.Num() > 1)
		{
			const float SectionLength = FVector::Dist(Path.Last(), Path[Path.Num() - 2]);
			const float SectionDuration = SectionLength / MoveSpeedCmS;
			PathDuration -= SectionDuration;
			Path.RemoveAt(Path.Num() - 1);
			PathDurations.RemoveAt(PathDurations.Num() - 1);

			RemainingDistance = (LoopDurationInSeconds - PathDuration) * MoveSpeedCmS;
			LastDistance = FVector::Dist(Path.Last(), Path[0]);
		}

		// Now find a point between the last to bring up the path up to the required length	
		const float LengthC = RemainingDistance - LastDistance;
		// cos(C) = 1 - c^2 / 2a^2
		const float AngleC = FMath::Acos(1.f - ((LengthC * LengthC) / (2 * LastDistance * LastDistance)));
		FVector Direction = Path[0] - Path.Last();
		Direction = Direction.RotateAngleAxis(FMath::RadiansToDegrees(AngleC), FMath::VRand());
		Path.Add(Path.Last() + Direction);
		PathDurations.Add(LastDistance / MoveSpeedCmS);
		PathDuration += (LastDistance / MoveSpeedCmS);
		PathDurations.Add(LengthC / MoveSpeedCmS);
		PathDuration += LengthC / MoveSpeedCmS;

		Midpoints.Empty();
		for (int32 i = 0; i < Path.Num(); i++)
		{
			PathA = Path[i == 0 ? Path.Num() - 1 : i - 1];
			PathB = Path[i];
			PathC = Path[(i + 1) % Path.Num()];
			Midpoints.Add(Bezier(PathA, PathB, PathC, Curviness, 0.5f));
		}
	}

	PathIndex = 0;
	PathT = 0.f;
	NextLoopPath();
	for (int i = 0; i < Path.Num(); i++)
	{
		FVector P = Path[i];
		FVector P2 = Path[(i + 1) % Path.Num()];
		DrawDebugLine(GetWorld(), GetActorLocation() + P, GetActorLocation() + P2, FColor::Red, false, 10.f);
	}
}

void AC0Torch::NextLoopPath()
{
	if (PathIndex >= Path.Num())
	{
		PathIndex = 0;
	}
	PathA = Path[PathIndex];
	PathB = Path[(PathIndex + 1) % Path.Num()];
	PathC = Path[(PathIndex + 2) % Path.Num()];
	MidpointA = Midpoints[PathIndex];
	MidpointB = Midpoints[(PathIndex + 1) % Path.Num()];
	// Make the curve midpoint somewhere near the surface of the sphere
	//PathC = (PathA + PathB) / 2.f;
	//PathC = PathC.RotateAngleAxis(FMath::RandRange(0.f, 5.f), FMath::VRand());
	//PathC = PathC.GetSafeNormal() * MoveRadiusInCm * FMath::RandRange(0.8f, 1.f);
	CurSectionDuration = PathDurations[PathIndex];
	/*DrawDebugLine(GetWorld(), GetActorLocation() + PathA, GetActorLocation() + PathC, FColor::Green, false, CurSectionDuration);
	DrawDebugLine(GetWorld(), GetActorLocation() + PathB, GetActorLocation() + PathC, FColor::Green, false, CurSectionDuration);*/
	
	// Calculate approx curve length
	float AB = FVector::Dist(PathA, PathB);
	float BC = FVector::Dist(PathB, PathC);
	float AC = FVector::Dist(PathA, PathC);
	ApproxCurveLength = AB + BC + 0.5f * (AB + BC - AC);

	PathIndex++;
}

const float AC0Torch::NextIntensity(const bool bMax, float Diff)
{
	float Variance = FMath::RandRange(-IntensityVariance, IntensityVariance)* Diff;
	if (bMax)
	{
		return FMath::Max(0.f, IntensityMaxInCandelas + Variance);
	}
	else
	{
		return FMath::Max(0.f, IntensityMinInCandelas + Variance);
	}
}

const float AC0Torch::NextAttenuation(const bool bMax, float Diff)
{
	float Variance = FMath::RandRange(-AttenuationVariance, AttenuationVariance) * Diff / 2.f;
	if (bMax)
	{
		return FMath::Max(0.f, AttenuationRadiusMax + Variance);
	}
	else
	{
		return FMath::Max(0.f, AttenuationRadiusMin + Variance);
	}
}

void AC0Torch::HandleDebugDraw()
{
	if (bDebugDrawRadius)
	{
		DrawDebugSphere(GetWorld(), GetActorLocation(), MoveRadiusInCm, 16, FColor::Blue, false, 1.f);
		//DrawDebugLine(GetWorld(), GetActorLocation() + PathA, GetActorLocation() + PathB, FColor::Red, false, 1.f);
		//DrawDebugLine(GetWorld(), GetActorLocation() + PathB, GetActorLocation() + PathC, FColor::Green, false, 1.f);
		for (int i = 0; i < Path.Num(); i++)
		{
			FVector P = Path[i];
			FVector P2 = Path[(i + 1) % Path.Num()];
			FVector M = Midpoints[i];			
			DrawDebugLine(GetWorld(), GetActorLocation() + P, GetActorLocation() + P2, FColor::Red, false, 1.f);
			DrawDebugLine(GetWorld(), GetActorLocation() + M, GetActorLocation() + P, FColor::Green, false, 1.f);
		}
		FTimerHandle Handle;
		GetWorld()->GetTimerManager().SetTimer(Handle, this, &AC0Torch::HandleDebugDraw, 1.0f, false);
	}
}

void AC0Torch::GenSplinePath()
{
    SplineComponent->ClearSplinePoints();

    // Calculate the number of points based on move speed and loop duration
    const float BasePoints = 2.0f; // Base number of points
    const float SpeedFactor = FMath::Clamp(MoveSpeed / 10.0f, 0.5f, 2.0f); // Adjust factor based on speed
    const int32 NumPoints = FMath::RoundToInt(BasePoints * SpeedFactor * LoopDurationInSeconds);

    FVector PreviousPoint = FVector::ZeroVector;
    for (int32 i = 0; i < NumPoints; ++i)
    {
        FVector Point = RandomPointOnSphere();
        SplineComponent->AddSplinePoint(Point, ESplineCoordinateSpace::Local);

        if (i > 0)
        {
            // Calculate tangent based on curviness
            FVector Tangent = (Point - PreviousPoint) * Curviness;
            SplineComponent->SetTangentAtSplinePoint(i - 1, Tangent, ESplineCoordinateSpace::Local);
        }

        PreviousPoint = Point;
    }

    // Set tangent for the last point to close the loop
    if (NumPoints > 1)
    {
        FVector FirstPoint = SplineComponent->GetLocationAtSplinePoint(0, ESplineCoordinateSpace::Local);
        FVector LastPoint = SplineComponent->GetLocationAtSplinePoint(NumPoints - 1, ESplineCoordinateSpace::Local);
        FVector Tangent = (FirstPoint - LastPoint) * Curviness;
        SplineComponent->SetTangentAtSplinePoint(NumPoints - 1, Tangent, ESplineCoordinateSpace::Local);
    }

    SplineComponent->SetClosedLoop(true);
}