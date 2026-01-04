
    // Whether to animate the light colour
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Colour")
        bool bAnimateColours;

    // Light colours over time
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Beneos Tools|Colour", meta=(EditCondition = "bAnimateColours"))
        TObjectPtr<UCurveLinearColor> LightColours;

    // Store previous light colour when colour animation is activated
    FLinearColor PrevLightColour;
