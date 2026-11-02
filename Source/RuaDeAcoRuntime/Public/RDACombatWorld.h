#pragma once

#include "RDAFighter.h"

namespace RDA
{
	enum class SoundEvent : uint8_t
	{
		None = 0,
		Fight,
		Swing,
		HitLight,
		HitHeavy,
		Block,
		Jump,
		Knockdown,
		RoundEnd,
		MatchWin,
		GrabStart,
		GrabSlam,
		SpecialCast,
		SuperCast,
		FinishReady,
		FinisherMonsterRoar,
		FinisherSplash,
		FinisherPotDrop,
		FinisherTrident,
	};

	struct ProjectileRuntime
	{
		int runtimeId = 0;
		int ownerIndex = 0;
		ProjectileDef def{};
		float x = 0.0f;
		float y = 0.0f;
		float velocityX = 0.0f;
		int facing = 1;
		int life = 0;
		int ageFrames = 0;
	};

	struct ProjectileSnapshot
	{
		int runtimeId = 0;
		int ownerIndex = 0;
		const char* id = "";
		float x = 0.0f;
		float y = 0.0f;
		int facing = 1;
		Rect worldBox{};
		bool active = true;
	};

	struct ActiveGrabState
	{
		bool active = false;
		int attackerIndex = 0;
		int defenderIndex = 1;
		MoveDef move{};
	};

	struct WorldSnapshot
	{
		int frame = 0;
		CombatPhase phase = CombatPhase::Intro;
		int phaseFrame = 0;
		int round = 1;
		int timeSeconds = 99;
		bool paused = false;
		bool debugBoxes = false;
		bool roundDraw = false;
		int roundWinner = -1;
		int matchWinner = -1;
		int finisherWinner = -1;
		bool finisherCompleted = false;
		StageId stage = StageId::CaisDaCidade;
		GameMode mode = GameMode::VsCpu;
		const char* banner = "";
		std::array<FighterSnapshot, 2> fighters{};
		std::vector<ProjectileSnapshot> projectiles;
		bool grabActive = false;
		int lastDamage = 0;
	};

	class CombatWorld
	{
	public:
		CombatWorld(
			const FighterDef& PlayerOne,
			const FighterDef& PlayerTwo,
			GameMode InMode = GameMode::LocalVersus,
			StageId InStage = StageId::CaisDaCidade);

		void Step(const InputFrame& P1Input, const InputFrame& P2Input);
		void TogglePause();
		void ToggleDebugBoxes();
		void RestartMatch();
		void ResetTrainingPositions();
		WorldSnapshot Snapshot() const;
		std::vector<SoundEvent> DrainSounds();
		uint64_t ComputeStateHash() const;

		CombatPhase Phase() const { return phase; }
		bool IsPaused() const { return paused; }
		StageId Stage() const { return stage; }
		GameMode Mode() const { return mode; }

		std::array<FighterRuntime, 2> fighters;

	private:
		void StepIntro();
		void StepActive(const InputFrame& P1Input, const InputFrame& P2Input);
		void StepFinishReady(const InputFrame& P1Input, const InputFrame& P2Input);
		void StepStageFinish();
		void StepRoundOver();
		void SpawnProjectiles(int OwnerIndex);
		void UpdateProjectiles();
		void ResolveHits(const InputFrame& P1Input, const InputFrame& P2Input);
		void UpdateActiveGrab();
		void ResolvePushboxes();
		void FinishRound(int WinnerIndex, bool bKnockout);
		void StartNextRound();

		GameMode mode = GameMode::LocalVersus;
		StageId stage = StageId::CaisDaCidade;
		int frame = 0;
		CombatPhase phase = CombatPhase::Intro;
		int phaseFrame = 0;
		int round = 1;
		int timeFrames = kRoundTimeFrames;
		bool paused = false;
		bool debugBoxes = false;
		bool roundDraw = false;
		int roundWinner = -1;
		int matchWinner = -1;
		int finisherWinner = -1;
		bool finisherCompleted = false;
		int lastDamage = 0;
		int nextProjectileId = 1;
		std::array<int, 2> comboCounts{0, 0};
		std::array<int, 2> comboTimers{0, 0};
		std::vector<ProjectileRuntime> projectiles;
		ActiveGrabState activeGrab{};
		std::vector<SoundEvent> soundEvents;
	};

	InputFrame BuildCpuInput(const CombatWorld& World, int CpuIndex, AiDifficulty Difficulty = AiDifficulty::Normal);
}
