#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RDARoundWidget.generated.h"

UCLASS()
class RUADEACO_API URDARoundWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "RuaDeAco")
	void SetRoundState(int32 Round, int32 WinsP1, int32 WinsP2);
};
