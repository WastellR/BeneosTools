// Copyright (c) CORE Games, Ltd. All rights reserved.

#include "BeneosTools.h"

#define LOCTEXT_NAMESPACE "FBeneosToolsModule"

#include <LevelEditor.h>

#include "C0TTRPGCam.h"
#include "C0TTRPGCineCam.h"
#include "C0TTRPGCamCustomization.h"
#include "C0TTRPGCineCamCustomization.h"
#include "C0GridCustomization.h"
#include "C0TorchCustomization.h"
#include "C0ChaseToolCustomization.h"

void FBeneosToolsModule::StartupModule()
{
    SelectedCam = nullptr;
    SelectedCineCam = nullptr;
    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
    LevelEditorModule.OnActorSelectionChanged().AddRaw(this, &FBeneosToolsModule::OnActorSelectionChanged);

    // Register details customization
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    // Leave out the suffix "A/U/F" when specifying the name of your class in the first param
    PropertyEditorModule.RegisterCustomClassLayout("C0TTRPGCam", FOnGetDetailCustomizationInstance::CreateStatic(&FC0TTRPGCamCustomization::MakeInstance));
    PropertyEditorModule.RegisterCustomClassLayout("C0TTRPGCineCam", FOnGetDetailCustomizationInstance::CreateStatic(&FC0TTRPGCineCamCustomization::MakeInstance));
    PropertyEditorModule.RegisterCustomClassLayout("C0Grid", FOnGetDetailCustomizationInstance::CreateStatic(&FC0GridCustomization::MakeInstance));
    PropertyEditorModule.RegisterCustomClassLayout("C0Torch", FOnGetDetailCustomizationInstance::CreateStatic(&FC0TorchCustomization::MakeInstance));
	PropertyEditorModule.RegisterCustomClassLayout("C0ChaseTool", FOnGetDetailCustomizationInstance::CreateStatic(&FC0ChaseToolCustomization::MakeInstance));
}

void FBeneosToolsModule::OnActorSelectionChanged(const TArray<UObject*>& NewSelection, bool bIsSelection)
{
    // Since we have to override a global setting in order to change whether the PIP
    // is displayed, this tries to keep the global setting unchanged by switching back
    // to whatever it was previously when the camera is deselected.
    bool bPrevShowPIP = false;

    if (SelectedCam)
    {
        SelectedCam->OnDeselectActor();
        bPrevShowPIP = SelectedCam->GetPrevShowPIP();
    }
    else if (SelectedCineCam)
    {
        SelectedCineCam->OnDeselectActor();
        bPrevShowPIP = SelectedCineCam->GetPrevShowPIP();
    }
    else
    {
        ULevelEditorViewportSettings* ViewportSettings = GetMutableDefault<ULevelEditorViewportSettings>();
        if (ViewportSettings)
        {
            bPrevShowPIP = ViewportSettings->bPreviewSelectedCameras;
        }
    }

    SelectedCam = nullptr;
    SelectedCineCam = nullptr;
    for (UObject* SelectedObject : NewSelection)
    {
        if (AC0TTRPGCam* NextSelectedCam = Cast<AC0TTRPGCam>(SelectedObject))
        {
            SelectedCam = NextSelectedCam;
            SelectedCam->OnSelectActor(bPrevShowPIP);
            break;
        }
        else if (AC0TTRPGCineCam* NextSelectedCineCam = Cast<AC0TTRPGCineCam>(SelectedObject))
        {
            SelectedCineCam = NextSelectedCineCam;
            SelectedCineCam->OnSelectActor(bPrevShowPIP);
            break;
        }
    }
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBeneosToolsModule, BeneosTools)