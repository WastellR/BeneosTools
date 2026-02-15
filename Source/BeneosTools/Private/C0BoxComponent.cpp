// Copyright (c) CORE Games, Ltd. All rights reserved.

#include "C0BoxComponent.h"

#if WITH_EDITOR
void UC0BoxComponent::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);

	OnPostEditComponentMove.Broadcast(bFinished);
}

void UC0BoxComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	const FName PropertyName = PropertyChangedEvent.Property->GetFName();
	FProperty* MemberPropertyThatChanged = PropertyChangedEvent.MemberProperty;
	const FName MemberPropertyName = MemberPropertyThatChanged != NULL ? MemberPropertyThatChanged->GetFName() : NAME_None;

	OnPostEditChangeProperty.Broadcast(PropertyName, MemberPropertyName);
}
#endif
