#pragma once

#include "RDATypes.h"

namespace RDA
{
	const FighterDef& GetGutoBarba();
	const FighterDef& GetRafaMare();
	const FighterDef& GetNoirReflexo();
	const FighterDef& GetAstroRiso();
	const FighterDef& GetDanteSinal();
	const FighterDef& GetLeoVioleta();
	const FighterDef& GetFighterDef(FighterId Id);
	const FighterDef* FindFighterByCanonicalId(const char* CanonicalId);

	const StageDef& GetStageDef(StageId Id);
	const StageDef* FindStageByCanonicalId(const char* CanonicalId);

	struct AppliedHit
	{
		int damage = 0;
		bool blocked = false;
		bool knockout = false;
		bool grabConnected = false;
		bool grabTeched = false;
		bool parryTriggered = false;
	};

	struct FighterSnapshot
	{
		FighterId id = FighterId::GutoBarba;
		const char* name = "";
		float x = 0.0f;
		float y = 0.0f;
		float velocityX = 0.0f;
		float velocityY = 0.0f;
		int facing = 1;
		FighterState state = FighterState::Idle;
		AnimId anim = AnimId::Idle;
		int stateFrame = 0;
		int health = 1000;
		int maxHealth = 1000;
		int meter = 0;
		int roundWins = 0;
		int comboCount = 0;
		bool grounded = true;
		bool attacking = false;
		const char* moveId = "";
		const char* moveLabel = "";
		int moveFrame = 0;
		int moveTotalFrames = 0;
		int hitActiveFrom = 0;
		int hitActiveTo = 0;
		int armorHits = 0;
		int parryFrames = 0;
		int damageReductionFrames = 0;
		int freezeFrames = 0;
		float victimRotation = 0.0f;
		bool visible = true;
		Rect pushboxWorld{};
		std::vector<Rect> hurtboxesWorld;
		std::vector<Rect> hitboxesWorld;
	};

	class FighterRuntime
	{
	public:
		FighterRuntime(const FighterDef& InDef, float StartX, int StartFacing);

		void ResetForRound(float StartX, int StartFacing);
		void ResetPosition(float StartX, int StartFacing);
		void BeginFrame(const InputFrame& Input, int WorldFrame);
		void FinishFrame();
		bool HasActiveHitbox(HitboxDef& OutHitbox, Rect& OutWorldRect) const;
		std::vector<Rect> CurrentHurtboxesWorld() const;
		Rect PushboxWorld() const;
		AppliedHit ReceiveHit(const HitboxDef& Hitbox, int AttackerFacing, bool bAllowGrabTech);
		void NotifyAttackConnected(bool bBlocked, const MoveDef& Move);
		void ForceState(FighterState NewState);
		void ForceMeter(int NewMeter);
		void EnterGrabbedState(FighterState VictimState, float AnchorX, float AnchorY, float RotationRad, int VictimFacing);
		void ReleaseFromGrab(float ThrowVelX, float ThrowVelY, bool bSlamToKnockdown);
		bool ConsumePendingProjectile(ProjectileDef& OutDef);
		FighterSnapshot Snapshot() const;

		const FighterDef* def = nullptr;
		float x = 0.0f;
		float y = static_cast<float>(kGroundY);
		float velocityX = 0.0f;
		float velocityY = 0.0f;
		float airDriftX = 0.0f;
		int facing = 1;
		FighterState state = FighterState::Idle;
		int stateFrame = 0;
		int health = 1000;
		int meter = 0;
		int roundWins = 0;
		bool grounded = true;

		const MoveDef* currentMove = nullptr;
		bool moveHasRegisteredContact = false;
		int hitStopFrames = 0;
		int stunFrames = 0;
		int knockdownFrames = 0;
		int freezeFrames = 0;
		int armorHits = 0;
		int parryFrames = 0;
		int parryRiposteDamage = 0;
		int damageReductionFrames = 0;
		float damageReductionMultiplier = 1.0f;
		float victimRotation = 0.0f;
		bool visible = true;

		int lastPunchFrame = -100;
		int lastKickFrame = -100;
		int lastDownFrame = -100;
		int lastForwardFrame = -100;
		int lastBackFrame = -100;

	private:
		bool TryStartMove(const InputFrame& Input, int WorldFrame);
		void StartMove(const MoveDef& Move);
		void ApplyHorizontalFriction();
		AnimId CurrentAnim() const;

		int pendingProjectileIndex = -1;
	};

	int MoveAnimFrame(const MoveDef& Move, int StateFrame, int SheetFrames);
}
