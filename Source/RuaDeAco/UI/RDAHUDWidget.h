#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "RDAHUDWidget.generated.h"

UCLASS()
class RUADEACO_API URDAHUDWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent, Category = "RuaDeAco")
	void SetFighterNames(const FText& PlayerOne, const FText& PlayerTwo);

	UFUNCTION(BlueprintImplementableEvent, Category = "RuaDeAco")
	void SetHealth(float PlayerOneRatio, float PlayerTwoRatio);

	UFUNCTION(BlueprintImplementableEvent, Category = "RuaDeAco")
	void SetTimer(int32 Seconds);

	UFUNCTION(BlueprintImplementableEvent, Category = "RuaDeAco")
	void SetRound(int32 Round, int32 WinsP1, int32 WinsP2);

	UFUNCTION(BlueprintImplementableEvent, Category = "RuaDeAco")
	void ShowBanner(const FText& Text);
};
