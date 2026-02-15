// Copyright (c) CORE Games, Ltd. All rights reserved.

#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "C0BoxComponent.generated.h"

DECLARE_MULTICAST_DELEGATE_OneParam(FOnPostEditComponentMove, bool /* bFinished */);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnPostEditChangeProperty, FName /* PropertyName */, FName /* MemberPropertyName */);

/**
  Subclassed UBoxComponent that fires a delegate on PostEditComponentMove
 */
UCLASS(meta = (BlueprintSpawnableComponent))
class BENEOSTOOLS_API UC0BoxComponent : public UBoxComponent
{
	GENERATED_BODY()

public:

	FOnPostEditComponentMove OnPostEditComponentMove;
	FOnPostEditChangeProperty OnPostEditChangeProperty;

#if WITH_EDITOR
	virtual void PostEditComponentMove(bool bFinished) override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
};
