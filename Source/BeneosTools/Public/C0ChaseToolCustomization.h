// Copyright (c) CORE Games, Ltd. All rights reserved.

#pragma once

#include <CoreMinimal.h>
#include <IDetailCustomization.h>

#include <Input/Reply.h>
#include <Internationalization/Text.h>

class AC0ChaseTool;

class FC0ChaseToolCustomization : public IDetailCustomization
{

public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

	void RefreshDetails();

#pragma region Custom Details

protected:

	TObjectPtr<IDetailLayoutBuilder> CachedDetailBuilder;
	TWeakObjectPtr<AC0ChaseTool> EditedActor;

	FReply OnPreviewButtonClicked();
	FText GetPreviewButtonText() const;

	FReply OnAToBButtonClicked();
	FReply OnBToAButtonClicked();

	TSharedPtr<IPropertyHandle> ChasePositionHandle;
	bool IsChasePositionEnabled() const;

#pragma endregion Custom Details

};

