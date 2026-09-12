#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "RDAInputConfig.generated.h"

class UInputAction;
class UInputMappingContext;

UCLASS(BlueprintType)
class RUADEACO_API URDAInputConfig : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	TObjectPtr<UInputMappingContext> MappingContext;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	TObjectPtr<UInputAction> IA_Move;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	TObjectPtr<UInputAction> IA_Jump;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	TObjectPtr<UInputAction> IA_Crouch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	TObjectPtr<UInputAction> IA_Punch;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	TObjectPtr<UInputAction> IA_Kick;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	TObjectPtr<UInputAction> IA_Block;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	TObjectPtr<UInputAction> IA_Grab;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	TObjectPtr<UInputAction> IA_Special;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	TObjectPtr<UInputAction> IA_Pause;
};
