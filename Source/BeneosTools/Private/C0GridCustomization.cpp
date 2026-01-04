// Copyright (c) CORE Games, Ltd. All rights reserved.

#include "C0GridCustomization.h"

#include <DetailCategoryBuilder.h>
#include <DetailLayoutBuilder.h>
#include <DetailWidgetRow.h>
#include <Widgets/Input/SButton.h>

#include "C0Grid.h"

#define LOCTEXT_NAMESPACE "GridCustomization"

TSharedRef<IDetailCustomization> FC0GridCustomization::MakeInstance()
{
	return MakeShareable(new FC0GridCustomization);
}

void FC0GridCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{	
	// Move category to top of details panel
	IDetailCategoryBuilder& ToolsCategory = DetailBuilder.EditCategory("Beneos Tools");
	ToolsCategory.SetSortOrder(0);
}
