#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "RDATypes.h"
#include "RDAGameMode.generated.h"

namespace RDA
{
	struct WorldSnapshot;
	struct InputFrame;
	class CombatWorld;
}

UCLASS()
class RUADEACO_API ARDAGameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	ARDAGameMode();
	virtual ~ARDAGameMode() override;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaSeconds) override;
	virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;

	bool IsTitleScreen() const { return bTitleScreen; }
	bool TryGetSnapshot(RDA::WorldSnapshot& OutSnapshot) const;
	int32 GetTitleTicks() const { return TitleTicks; }

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	int32 RoundsToWin = 2;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "RuaDeAco")
	int32 RoundTimeSeconds = 99;

private:
	void StartMatchRuntime();
	void ReadInputs(RDA::InputFrame& OutP1, RDA::InputFrame& OutP2) const;

	bool bTitleScreen = true;
	int32 TitleTicks = 0;
	float StepAccumulator = 0.0f;
	RDA::CombatWorld* FightWorld = nullptr;
};
