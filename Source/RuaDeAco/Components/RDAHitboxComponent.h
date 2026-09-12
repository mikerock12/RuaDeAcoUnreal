#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "RDAHitboxComponent.generated.h"

UCLASS(ClassGroup = (RuaDeAco), meta = (BlueprintSpawnableComponent))
class RUADEACO_API URDAHitboxComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	URDAHitboxComponent();

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	bool bActive = false;
};
