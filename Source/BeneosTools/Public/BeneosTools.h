// Copyright (c) CORE Games, Ltd. All rights reserved.

#pragma once

#include "Modules/ModuleManager.h"

class SC0TTRPGEditorViewport;
class AC0TTRPGCam;
class AC0TTRPGCineCam;

class FBeneosToolsModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;

	AC0TTRPGCam* SelectedCam;
	AC0TTRPGCineCam* SelectedCineCam;
	void OnActorSelectionChanged(const TArray<UObject*>& NewSelection, bool bIsSelection);
};
