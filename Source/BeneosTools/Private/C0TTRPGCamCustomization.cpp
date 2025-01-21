// Copyright (c) CORE Games, Ltd. All rights reserved.

#include "C0TTRPGCamCustomization.h"

#include <DetailCategoryBuilder.h>
#include <DetailLayoutBuilder.h>
#include <DetailWidgetRow.h>
#include <Widgets/Input/SButton.h>

#define LOCTEXT_NAMESPACE "TTRPGCamCustomization"

TSharedRef<IDetailCustomization> FC0TTRPGCamCustomization::MakeInstance()
{
	return MakeShareable(new FC0TTRPGCamCustomization);
}

void FC0TTRPGCamCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{	
	// Move category to top of details panel
	IDetailCategoryBuilder& ToolsCategory = DetailBuilder.EditCategory("Beneos Tools");
	ToolsCategory.SetSortOrder(0);
}
