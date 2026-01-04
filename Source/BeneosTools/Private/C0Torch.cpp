
	bAnimateColours(false),
	// Load default light colours
	const FString ColoursPath = TEXT("/BeneosTools/C_TorchColoursExample.C_TorchColoursExample");
	if (UCurveLinearColor* DefaultColours = Cast<UCurveLinearColor>(StaticLoadObject(UCurveLinearColor::StaticClass(), nullptr, *ColoursPath)))
	{
		LightColours = DefaultColours;
	}
		// Interpolate light colour
		if (bAnimateColours)
		{
			PointLight->SetLightColor(LightColours->GetClampedLinearColorValue(T));
		}
		else if (PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(AC0Torch, bAnimateColours))
		{
			if (bAnimateColours)
			{
				PrevLightColour = PointLight->LightColor;
			}
			else
			{
				PointLight->SetLightColor(PrevLightColour);
			}
		}
