// Copyright (c) CORE Games, Ltd. All rights reserved.

#pragma once

#include <CoreMinimal.h>
#include <IDetailCustomization.h>

#include <Input/Reply.h>
#include <Internationalization/Text.h>

class FC0TorchCustomization : public IDetailCustomization
{

public:
	/** Makes a new instance of this detail layout class for a specific detail view requesting it */
	static TSharedRef<IDetailCustomization> MakeInstance();

	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

#pragma region Custom Details

protected:	

#pragma endregion Custom Details

};

