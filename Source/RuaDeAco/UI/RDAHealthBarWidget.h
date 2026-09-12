#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RDAHealthBarWidget.generated.h"

UCLASS()
class RUADEACO_API URDAHealthBarWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "RuaDeAco")
	void SetRatio(float Ratio);
};
