#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "RDAPlayerController.generated.h"

UCLASS()
class RUADEACO_API ARDAPlayerController : public APlayerController
{
	GENERATED_BODY()

public:
	ARDAPlayerController();
	virtual void BeginPlay() override;
};
