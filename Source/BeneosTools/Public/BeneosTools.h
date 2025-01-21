// Copyright (c) CORE Games, Ltd. All rights reserved.

#pragma once

#include "Modules/ModuleManager.h"

class SC0TTRPGEditorViewport;
class AC0TTRPGCam;

class FBeneosToolsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	AC0TTRPGCam* SelectedCam;
	void OnActorSelectionChanged(const TArray<UObject*>& NewSelection, bool bIsSelection);
};
