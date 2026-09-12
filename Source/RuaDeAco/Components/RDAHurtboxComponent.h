#pragma once

#include "CoreMinimal.h"
#include "Components/BoxComponent.h"
#include "RDAHurtboxComponent.generated.h"

UCLASS(ClassGroup = (RuaDeAco), meta = (BlueprintSpawnableComponent))
class RUADEACO_API URDAHurtboxComponent : public UBoxComponent
{
	GENERATED_BODY()

public:
	URDAHurtboxComponent();
};
