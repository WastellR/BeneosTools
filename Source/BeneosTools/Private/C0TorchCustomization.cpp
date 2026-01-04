// Copyright (c) CORE Games, Ltd. All rights reserved.

#include "C0TorchCustomization.h"

#include <DetailCategoryBuilder.h>
#include <DetailLayoutBuilder.h>
#include <DetailWidgetRow.h>
#include <Widgets/Input/SButton.h>

#include "C0Torch.h"

#define LOCTEXT_NAMESPACE "TorchCustomization"

TSharedRef<IDetailCustomization> FC0TorchCustomization::MakeInstance()
{
	return MakeShareable(new FC0TorchCustomization);
}

void FC0TorchCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{	
	// Move category to top of details panel
	IDetailCategoryBuilder& ToolsCategory = DetailBuilder.EditCategory("Beneos Tools");
	ToolsCategory.SetSortOrder(0);
}
