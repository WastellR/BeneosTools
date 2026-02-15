// Copyright (c) CORE Games, Ltd. All rights reserved.

#include "C0ChaseToolCustomization.h"

#include <DetailCategoryBuilder.h>
#include <DetailLayoutBuilder.h>
#include <DetailWidgetRow.h>
#include <IDetailGroup.h>
#include <Widgets/Input/SButton.h>
#include <Widgets/Input/SVectorInputBox.h>

#include "C0ChaseTool.h"

#define LOCTEXT_NAMESPACE "ChaseToolCustomization"

TSharedRef<IDetailCustomization> FC0ChaseToolCustomization::MakeInstance()
{
	return MakeShareable(new FC0ChaseToolCustomization);
}

void FC0ChaseToolCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{	
	CachedDetailBuilder = &DetailBuilder;
	TArray<TWeakObjectPtr<UObject>> Objects;
	DetailBuilder.GetObjectsBeingCustomized(Objects);

	if (Objects.Num() != 1)
	{
		return;
	}

	EditedActor = Cast<AC0ChaseTool>(Objects[0].Get());
	TWeakPtr<FC0ChaseToolCustomization> WeakThisPtr = SharedThis(this);
	// Update UI with event broadcast
	EditedActor->EditorUIUpdate.AddLambda([WeakThisPtr]()
		{
			if (TSharedPtr<FC0ChaseToolCustomization> Pinned = WeakThisPtr.Pin())
			{
				Pinned->RefreshDetails();
			}
		});

	// Move category to top of details panel
	IDetailCategoryBuilder& ToolsCategory = DetailBuilder.EditCategory("Beneos Tools");
	ToolsCategory.SetSortOrder(0);
	ToolsCategory.AddCustomRow(FText::FromString("CopyButtons"))
		.NameContent().HAlign(HAlign_Center).MinDesiredWidth(50.f)
		[
			SNew(SButton)
				.Text(FText::FromString("Copy A to B"))
				.OnClicked(this, &FC0ChaseToolCustomization::OnAToBButtonClicked)				
		]
		.ValueContent().HAlign(HAlign_Center).MinDesiredWidth(50.f)
		[
			SNew(SButton)
				.Text(FText::FromString("Copy B to A"))
				.OnClicked(this, &FC0ChaseToolCustomization::OnBToAButtonClicked)				
		];

	TSharedPtr<IPropertyHandle> PropertyHandle = DetailBuilder.GetProperty(
			GET_MEMBER_NAME_CHECKED(AC0ChaseTool, LoopDuration)
		);
	PropertyHandle->MarkHiddenByCustomization();
	ToolsCategory.AddCustomRow(FText::FromString("Loop Duration"))
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			PropertyHandle->CreatePropertyValueWidget()
		];

	PropertyHandle = DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(AC0ChaseTool, PathDistanceInCm)
	);
	PropertyHandle->MarkHiddenByCustomization();
	ToolsCategory.AddCustomRow(FText::FromString("Path Distance"))
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			PropertyHandle->CreatePropertyValueWidget()
		];

	
	ToolsCategory.AddCustomRow(FText::FromString("MovementSpeed"))
		.NameContent()
		[
			SNew(STextBlock)
				.Text(FText::FromString("Movement Speed"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.ToolTipText(FText::FromString("Speed camera is moving at to get from start to finish in the set loop time"))
		]
		.ValueContent()
		[
			SNew(STextBlock)
				.Text(EditedActor->GetSpeedText())
				.Font(IDetailLayoutBuilder::GetDetailFont())			
		];

	PropertyHandle = DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(AC0ChaseTool, bBoxAGrabContents)
	);
	PropertyHandle->MarkHiddenByCustomization();
	ToolsCategory.AddCustomRow(FText::FromString("BoxA Grab Contents"))
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			PropertyHandle->CreatePropertyValueWidget()
		];

	PropertyHandle = DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(AC0ChaseTool, bBoxBGrabContents)
	);
	PropertyHandle->MarkHiddenByCustomization();
	ToolsCategory.AddCustomRow(FText::FromString("BoxB Grab Contents"))
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			PropertyHandle->CreatePropertyValueWidget()
		];

	// Preview category
	IDetailGroup& PreviewGroup = ToolsCategory.AddGroup(FName("Preview"), FText::FromString("Camera Movement Preview"));
	
	PropertyHandle = DetailBuilder.GetProperty(
			GET_MEMBER_NAME_CHECKED(AC0ChaseTool, Camera)
		);
	PropertyHandle->MarkHiddenByCustomization();
	PreviewGroup.AddWidgetRow()
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			PropertyHandle->CreatePropertyValueWidget()
		];
	PropertyHandle = DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(AC0ChaseTool, CameraHeight)
	);
	PropertyHandle->MarkHiddenByCustomization();
	PreviewGroup.AddWidgetRow()
		.NameContent()
		[
			PropertyHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			PropertyHandle->CreatePropertyValueWidget()
		];
	
	PreviewGroup.AddWidgetRow()
		.NameContent()
		[
			SNew(STextBlock)
				.Text(FText::FromString("Preview Animation"))
				.Font(IDetailLayoutBuilder::GetDetailFont())
				.ToolTipText(FText::FromString("Current position of camera along chase path (between 0 and 1)"))
		]
		.ValueContent()
		[
			SNew(SButton)
				.Text(this, &FC0ChaseToolCustomization::GetPreviewButtonText)
				.OnClicked(this, &FC0ChaseToolCustomization::OnPreviewButtonClicked)
				.HAlign(HAlign_Center)
		];
	ChasePositionHandle = DetailBuilder.GetProperty(
		GET_MEMBER_NAME_CHECKED(AC0ChaseTool, CameraChasePosition)
	);
	ChasePositionHandle->MarkHiddenByCustomization();
	PreviewGroup.AddWidgetRow()
		.NameContent()
		[			
			SNew(STextBlock)
			.Text(FText::FromString("Current Position"))
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.ToolTipText(FText::FromString("Current position of camera along chase path (between 0 and 1)"))
		]
		.ValueContent()
		[
			ChasePositionHandle->CreatePropertyValueWidget()
		]
		.IsEnabled(!EditedActor->IsRunningChasePreview());

	TSharedPtr<IPropertyHandle> ChasePauseHandle =
		DetailBuilder.GetProperty(
			GET_MEMBER_NAME_CHECKED(AC0ChaseTool, CameraChasePause)
		);
	ChasePauseHandle->MarkHiddenByCustomization();
	PreviewGroup.AddWidgetRow()
		.NameContent()
		[
			ChasePauseHandle->CreatePropertyNameWidget()
		]
		.ValueContent()
		[
			ChasePauseHandle->CreatePropertyValueWidget()
		];
}

void FC0ChaseToolCustomization::RefreshDetails()
{
	CachedDetailBuilder->ForceRefreshDetails();
}

FReply FC0ChaseToolCustomization::OnPreviewButtonClicked()
{
	if (EditedActor.IsValid())
	{
		EditedActor->Modify();               // undo support
		EditedActor->ToggleChasePreview();
	}
	CachedDetailBuilder->ForceRefreshDetails();
	return FReply::Handled();
}

FText FC0ChaseToolCustomization::GetPreviewButtonText() const
{
	bool bIsRunning = true;
	if (EditedActor.IsValid())
	{
		bIsRunning = EditedActor->IsRunningChasePreview();
	}	

	return FText::FromString(bIsRunning ? "Stop" : "Start");
}

FReply FC0ChaseToolCustomization::OnAToBButtonClicked()
{
	if (EditedActor.IsValid())
	{
		EditedActor->CopyAToB();
	}
	return FReply::Handled();
}

FReply FC0ChaseToolCustomization::OnBToAButtonClicked()
{
	if (EditedActor.IsValid())
	{
		EditedActor->CopyBToA();
	}
	return FReply::Handled();
}

bool FC0ChaseToolCustomization::IsChasePositionEnabled() const
{
	if (EditedActor.IsValid())
	{
		return !EditedActor->IsRunningChasePreview();
	}
	return true;
}
