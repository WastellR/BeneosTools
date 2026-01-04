// Copyright (c) CORE Games, Ltd. All rights reserved.

#pragma once

#include <CoreMinimal.h>
#include <GameFramework/Actor.h>
#include <Components/ActorComponent.h>
#include <Components/SplineComponent.h>

#include "C0Torch.generated.h"

class UPointLightComponent;

UCLASS(config = BeneosTools, meta = (DisplayName = "Beneos Torch"))
class BENEOSTOOLS_API AC0Torch : public AActor
{
    GENERATED_BODY()

#pragma region Life Cycle

public:

    // Sets default values for this actor's properties
    AC0Torch();

    // Called every frame
    virtual void Tick(float DeltaTime) override;

protected:

    virtual void OnConstruction(const FTransform& Transform) override;    

#if WITH_EDITOR
    virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
    virtual bool ShouldTickIfViewportsOnly() const override;
#endif

    FString GetDefaultActorLabel() const override;

#pragma endregion Life Cycle

#pragma region Properties   

public:

    // Radius that the light will randomly move in
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Movement", meta = (ClampMin = 0.f))
        float MoveRadiusInCm;

	// After one loop, the light will return to the exact position + attenuation conditions it started at
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Movement", meta = (ClampMin = 0.f))
		float LoopDurationInSeconds;

    // Speed light moves at
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Movement", meta = (ClampMin = 0.f))
        float MoveSpeed;

    // Scale 0-1 of how smooth the path is. 0 is straight lines bouncing off the inside of the sphere.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Movement", meta = (ClampMin = 0.f, ClampMax = 1.f))
        float Curviness;

    // Draw a blue sphere around the extent of the move path's radius
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Movement")
        bool bDebugDrawRadius;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Intensity")
        bool bAnimateIntensity;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Intensity", meta = (ClampMin = 0.f))
        float IntensityMinInCandelas;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Intensity", meta = (ClampMin = 0.f))
        float IntensityMaxInCandelas;

    // How much to randomly vary the intensity by, clamped between 0 and 1 (+/- this * (max - min))
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Intensity", meta = (ClampMin = 0.f, ClampMax = 1.f))
        float IntensityVariance;

    // How much time it takes to go from max to minimum intensity
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Intensity", meta = (ClampMin = 0.f))
        float IntensityPeriodInSeconds;

    // How much to randomly vary the period by, clamped between 0 and 1 (+/- this * period)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Intensity", meta = (ClampMin = 0.f, ClampMax = 1.f))
        float IntensityPeriodVariance;

    // How smoothly to interpolate between values. 0 is a straight line, 1 a smooth curve.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Intensity", meta = (ClampMin = 0.f, ClampMax = 1.f))
        float IntensitySmoothness;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Attenuation")
        bool bAnimateAttenuation;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Attenuation", meta = (ClampMin = 0.f))
        float AttenuationRadiusMin;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Attenuation", meta = (ClampMin = 0.f))
        float AttenuationRadiusMax;

    // How much to randomly vary the Attenuation by, clamped between 0 and 1 (+/- this * (max - min))
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Attenuation", meta = (ClampMin = 0.f, ClampMax = 1.f))
        float AttenuationVariance;

    // How much time it takes to go from max to minimum Attenuation
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Attenuation", meta = (ClampMin = 0.f))
        float AttenuationPeriodInSeconds;

    // How much to randomly vary the period by, clamped between 0 and 1 (+/- this * period)
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Attenuation", meta = (ClampMin = 0.f, ClampMax = 1.f))
        float AttenuationPeriodVariance;

    // How smoothly to interpolate between values. 0 is a straight line, 1 a smooth curve.
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Attenuation", meta = (ClampMin = 0.f, ClampMax = 1.f))
        float AttenuationSmoothness;

    // Whether to animate the light colour
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Colour")
        bool bAnimateColours;

    // Light colours over time
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Colour", meta=(EditCondition = "bAnimateColours"))
        TObjectPtr<UCurveLinearColor> LightColours;

private:

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
        TObjectPtr<class USceneComponent> SceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
        TObjectPtr<class UPointLightComponent> PointLight;

    // Add the Spline Component
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
    TObjectPtr<class USplineComponent> SplineComponent;

    // Variable to track the spline's progress
    float LoopT;

    // Store previous light colour when colour animation is activated
    FLinearColor PrevLightColour;

#pragma endregion Properties

#pragma region Movement

private:

    const FVector RandomPointOnSphere();

    FVector PathA;
    FVector PathB;
    FVector PathC;
    FVector MidpointA;
    FVector MidpointB;
    void GenPath();
    float FirstHalfDuration;
    float TotalDuration;
    float PathT;

	TArray<FVector> Path;
	TArray<FVector> Midpoints;
	TArray<float> PathDurations;
	float CurSectionDuration;
	float ApproxCurveLength;
	void GenSplinePath();
	void GenLoopPath();
	void NextLoopPath();
	bool Temp;

	int32 PathIndex;

#pragma endregion Movement

#pragma region Intensity

    UPROPERTY()
        float PrevIntensity;

    float PeriodStart;
    float PeriodLength;
    float StartIntensity;
    float EndIntensity;
    bool NextPeriodIsMax;

    const float NextIntensity(const bool bMax, float Diff);

#pragma endregion Intensity

#pragma region Attenuation

    UPROPERTY()
        float PrevAttenuation;

    float AttPeriodStart;
    float AttPeriodLength;
    float AttStart;
    float AttEnd;
    bool AttNextPeriodIsMax;

    const float NextAttenuation(const bool bMax, float Diff);

#pragma endregion Attenuation

    bool bInitialized;

    void HandleDebugDraw();

};
