// Copyright (c) CORE Games, Ltd. All rights reserved.

#include "BeneosTools.h"

#define LOCTEXT_NAMESPACE "FBeneosToolsModule"


#include "C0CamGrid.h"
#include "C0TTRPGCamCustomization.h"
#include "C0TTRPGCineCamCustomization.h"
#include "C0GridCustomization.h"
#include "C0TorchCustomization.h"
void FBeneosToolsModule::StartupModule()
{
    SelectedGrid = nullptr;
    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");    
    LevelEditorModule.OnActorSelectionChanged().AddRaw(this, &FBeneosToolsModule::OnActorSelectionChanged);

    // Register details customization
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    // Leave out the suffix "A/U/F" when specifying the name of your class in the first param
    PropertyEditorModule.RegisterCustomClassLayout("C0TTRPGCam", FOnGetDetailCustomizationInstance::CreateStatic(&FC0TTRPGCamCustomization::MakeInstance));
    PropertyEditorModule.RegisterCustomClassLayout("C0TTRPGCineCam", FOnGetDetailCustomizationInstance::CreateStatic(&FC0TTRPGCineCamCustomization::MakeInstance));
    PropertyEditorModule.RegisterCustomClassLayout("C0Grid", FOnGetDetailCustomizationInstance::CreateStatic(&FC0GridCustomization::MakeInstance));
    PropertyEditorModule.RegisterCustomClassLayout("C0Torch", FOnGetDetailCustomizationInstance::CreateStatic(&FC0TorchCustomization::MakeInstance));
}

void FBeneosToolsModule::OnActorSelectionChanged(const TArray<UObject*>& NewSelection, bool bIsSelection)
{
    // Since we have to override a global setting in order to change whether the PIP
    // is displayed, this tries to keep the global setting unchanged by switching back
    // to whatever it was previously when the camera is deselected.
    bool bPrevShowPIP;
    if (SelectedGrid)
    {
        SelectedGrid->OnDeselectActor();
        bPrevShowPIP = SelectedGrid->GetPrevShowPIP();
    }
    else
    {
        ULevelEditorViewportSettings* ViewportSettings = GetMutableDefault<ULevelEditorViewportSettings>();
        if (ViewportSettings)
        {
            bPrevShowPIP = ViewportSettings->bPreviewSelectedCameras;
        }
    }
    SelectedGrid = nullptr;
    for (UObject* SelectedObject : NewSelection)
    {
        if (AC0CamGrid* NextSelectedGrid = Cast<AC0CamGrid>(SelectedObject))
        {
            SelectedGrid = NextSelectedGrid;
            SelectedGrid->OnSelectActor(bPrevShowPIP);
        }
    }
}
void FBeneosToolsModule::ShutdownModule()
{
    FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(BeneosEditorTabId);

    if (SelectedGrid)
    {
        ULevelEditorViewportSettings* ViewportSettings = GetMutableDefault<ULevelEditorViewportSettings>();
        if (ViewportSettings)
        {
            ViewportSettings->bPreviewSelectedCameras = SelectedGrid->GetPrevShowPIP();
            ViewportSettings->SaveConfig();
        }
    }
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBeneosToolsModule, BeneosTools)