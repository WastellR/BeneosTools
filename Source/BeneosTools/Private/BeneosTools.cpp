// Copyright (c) CORE Games, Ltd. All rights reserved.

#include "BeneosTools.h"

#define LOCTEXT_NAMESPACE "FBeneosToolsModule"


#include "C0TTRPGCam.h"
#include "C0TTRPGCamCustomization.h"
void FBeneosToolsModule::StartupModule()
{
    SelectedCam = nullptr;
    FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");    
    LevelEditorModule.OnActorSelectionChanged().AddRaw(this, &FBeneosToolsModule::OnActorSelectionChanged);

    // Register details customization
    FPropertyEditorModule& PropertyEditorModule = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    // Leave out the suffix "A/U/F" when specifying the name of your class in the first param
    PropertyEditorModule.RegisterCustomClassLayout("C0TTRPGCam", FOnGetDetailCustomizationInstance::CreateStatic(&FC0TTRPGCamCustomization::MakeInstance));

void FBeneosToolsModule::OnActorSelectionChanged(const TArray<UObject*>& NewSelection, bool bIsSelection)
{
    // Since we have to override a global setting in order to change whether the PIP
    // is displayed, this tries to keep the global setting unchanged by switching back
    // to whatever it was previously when the camera is deselected.
    bool bPrevShowPIP;
    if (SelectedCam)
    {
        SelectedCam->OnDeselectActor();
        bPrevShowPIP = SelectedCam->GetPrevShowPIP();
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
    for (UObject* SelectedObject : NewSelection)
    {
        if (AC0TTRPGCam* NextSelectedCam = Cast<AC0TTRPGCam>(SelectedObject))
        {
            SelectedCam = NextSelectedCam;
            SelectedCam->OnSelectActor(bPrevShowPIP);
        }
    }
}
void FBeneosToolsModule::ShutdownModule()
{
    if (SelectedCam)
    {
        ULevelEditorViewportSettings* ViewportSettings = GetMutableDefault<ULevelEditorViewportSettings>();
        if (ViewportSettings)
        {
            ViewportSettings->bPreviewSelectedCameras = SelectedCam->GetPrevShowPIP();
            ViewportSettings->SaveConfig();
        }
    }
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FBeneosToolsModule, BeneosTools)