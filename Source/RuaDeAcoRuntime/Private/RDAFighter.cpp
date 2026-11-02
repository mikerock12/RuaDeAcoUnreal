#include "RDAFighter.h"
#include <algorithm>
#include <cmath>

namespace RDA
{
	FighterRuntime::FighterRuntime(const FighterDef& InDef, float StartX, int StartFacing)
		: def(&InDef)
	{
		ResetForRound(StartX, StartFacing);
		roundWins = 0;
	}

	void FighterRuntime::ResetForRound(float StartX, int StartFacing)
	{
		ResetPosition(StartX, StartFacing);
		health = def->stats.maxHealth;
	}

	void FighterRuntime::ResetPosition(float StartX, int StartFacing)
	{
		x = StartX;
		y = static_cast<float>(kGroundY);
		velocityX = 0.0f;
		velocityY = 0.0f;
		airDriftX = 0.0f;
		facing = StartFacing >= 0 ? 1 : -1;
		state = FighterState::Idle;
		stateFrame = 0;
		grounded = true;
		currentMove = nullptr;
		moveHasRegisteredContact = false;
		hitStopFrames = 0;
		stunFrames = 0;
		knockdownFrames = 0;
		freezeFrames = 0;
		armorHits = 0;
		parryFrames = 0;
		parryRiposteDamage = 0;
		damageReductionFrames = 0;
		damageReductionMultiplier = 1.0f;
		victimRotation = 0.0f;
		visible = true;
		pendingProjectileIndex = -1;
		lastPunchFrame = -100;
		lastKickFrame = -100;
		lastDownFrame = -100;
		lastForwardFrame = -100;
		lastBackFrame = -100;
	}

	void FighterRuntime::ForceState(FighterState NewState)
	{
		currentMove = nullptr;
		moveHasRegisteredContact = false;
		velocityX = 0.0f;
		state = NewState;
		stateFrame = 0;
	}

	void FighterRuntime::ForceMeter(int NewMeter)
	{
		meter = std::clamp(NewMeter, 0, kMaxMeter);
	}

	void FighterRuntime::EnterGrabbedState(FighterState VictimState, float AnchorX, float AnchorY, float RotationRad, int VictimFacing)
	{
		currentMove = nullptr;
		moveHasRegisteredContact = false;
		velocityX = 0.0f;
		velocityY = 0.0f;
		airDriftX = 0.0f;
		x = std::clamp(AnchorX, static_cast<float>(kStageLeft), static_cast<float>(kStageRight));
		y = std::min(static_cast<float>(kGroundY), AnchorY);
		grounded = y >= static_cast<float>(kGroundY) - 0.5f;
		facing = VictimFacing >= 0 ? 1 : -1;
		victimRotation = RotationRad;
		state = VictimState;
	}

	void FighterRuntime::ReleaseFromGrab(float ThrowVelX, float ThrowVelY, bool bSlamToKnockdown)
	{
		victimRotation = 0.0f;
		currentMove = nullptr;
		if (bSlamToKnockdown)
		{
			y = static_cast<float>(kGroundY);
			grounded = true;
			velocityX = ThrowVelX;
			velocityY = 0.0f;
			state = FighterState::Knockdown;
			stateFrame = 0;
			knockdownFrames = 26;
		}
		else
		{
			grounded = false;
			velocityX = ThrowVelX;
			velocityY = ThrowVelY;
			state = FighterState::Thrown;
			stateFrame = 0;
		}
	}

	bool FighterRuntime::ConsumePendingProjectile(ProjectileDef& OutDef)
	{
		if (pendingProjectileIndex < 0 || pendingProjectileIndex >= def->projectileCount)
		{
			return false;
		}
		OutDef = def->projectiles[static_cast<size_t>(pendingProjectileIndex)];
		pendingProjectileIndex = -1;
		return true;
	}

	void FighterRuntime::BeginFrame(const InputFrame& Input, int WorldFrame)
	{
		if (parryFrames > 0) parryFrames -= 1;
		if (damageReductionFrames > 0)
		{
			damageReductionFrames -= 1;
			if (damageReductionFrames == 0) damageReductionMultiplier = 1.0f;
		}

		if (Input.punchPressed) lastPunchFrame = WorldFrame;
		if (Input.kickPressed) lastKickFrame = WorldFrame;
		if (Input.downPressed || Input.down) lastDownFrame = WorldFrame;
		const bool bHoldingForward = (facing > 0 && Input.right) || (facing < 0 && Input.left);
		const bool bHoldingBack = (facing > 0 && Input.left) || (facing < 0 && Input.right);
		if (bHoldingForward) lastForwardFrame = WorldFrame;
		if (bHoldingBack) lastBackFrame = WorldFrame;

		if (hitStopFrames > 0)
		{
			return;
		}

		if (state == FighterState::Knockout || state == FighterState::Victory)
		{
			ApplyHorizontalFriction();
			return;
		}

		if (state == FighterState::GrabbedFront || state == FighterState::GrabbedLifted)
		{
			return;
		}

		if (freezeFrames > 0)
		{
			freezeFrames -= 1;
			state = FighterState::Frozen;
			ApplyHorizontalFriction();
			if (freezeFrames == 0)
			{
				state = FighterState::Idle;
				stateFrame = 0;
			}
			return;
		}

		if (state == FighterState::Thrown)
		{
			if (grounded)
			{
				state = FighterState::Knockdown;
				stateFrame = 0;
				knockdownFrames = 24;
			}
			return;
		}

		if (state == FighterState::Knockdown)
		{
			ApplyHorizontalFriction();
			if (grounded && stateFrame >= knockdownFrames)
			{
				state = FighterState::WakeUp;
				stateFrame = 0;
			}
			return;
		}

		if (state == FighterState::WakeUp)
		{
			velocityX = 0.0f;
			if (stateFrame >= kWakeUpFrames)
			{
				state = FighterState::Idle;
				stateFrame = 0;
			}
			return;
		}

		if (state == FighterState::HitStun || state == FighterState::BlockStanding || state == FighterState::BlockCrouching)
		{
			ApplyHorizontalFriction();
			if (stunFrames > 0)
			{
				stunFrames -= 1;
				if (stunFrames == 0)
				{
					state = Input.down && grounded ? FighterState::Crouch : FighterState::Idle;
					stateFrame = 0;
				}
				return;
			}
		}

		if (currentMove)
		{
			if (currentMove->grantArmorFrame >= 0 && stateFrame == currentMove->grantArmorFrame)
			{
				armorHits = currentMove->grantArmorHits;
			}
			if (currentMove->grantParryFrame >= 0 && stateFrame == currentMove->grantParryFrame)
			{
				parryFrames = currentMove->grantParryDuration;
				parryRiposteDamage = currentMove->grantParryDamage;
			}
			if (currentMove->grantDamageReductionFrame >= 0 && stateFrame == currentMove->grantDamageReductionFrame)
			{
				damageReductionFrames = currentMove->grantDamageReductionDuration;
				damageReductionMultiplier = currentMove->grantDamageReductionMultiplier;
			}
			if (currentMove->spawnProjectileFrame >= 0 && stateFrame == currentMove->spawnProjectileFrame)
			{
				pendingProjectileIndex = currentMove->projectileIndex;
			}

			if (stateFrame >= currentMove->totalFrames)
			{
				currentMove = nullptr;
				moveHasRegisteredContact = false;
				armorHits = 0;
				state = grounded ? (Input.down ? FighterState::Crouch : FighterState::Idle) : FighterState::Fall;
				stateFrame = 0;
			}
			else
			{
				if (currentMove->moveTo > 0 && stateFrame >= currentMove->moveFrom && stateFrame <= currentMove->moveTo)
				{
					velocityX = currentMove->moveVelX * static_cast<float>(facing);
				}
				else if (grounded)
				{
					velocityX = 0.0f;
				}
				return;
			}
		}

		if (state == FighterState::Landing)
		{
			velocityX = 0.0f;
			if (TryStartMove(Input, WorldFrame))
			{
				return;
			}
			if (stateFrame >= kLandingFrames)
			{
				state = Input.down ? FighterState::Crouch : FighterState::Idle;
				stateFrame = 0;
			}
			return;
		}

		if (TryStartMove(Input, WorldFrame))
		{
			return;
		}

		const int MoveDir = (Input.right ? 1 : 0) - (Input.left ? 1 : 0);
		const bool bHoldingBackward = MoveDir != 0 && MoveDir != facing;

		if ((Input.block || (bHoldingBackward && Input.down)) && grounded && Input.block)
		{
			velocityX = 0.0f;
			state = Input.down ? FighterState::BlockCrouching : FighterState::BlockStanding;
			return;
		}

		if (Input.upPressed && grounded)
		{
			grounded = false;
			velocityY = -def->stats.jumpSpeed;
			if (MoveDir == 0)
			{
				airDriftX = 0.0f;
			}
			else
			{
				const float Speed = MoveDir == facing ? def->stats.jumpForwardSpeed : def->stats.jumpBackwardSpeed;
				airDriftX = static_cast<float>(MoveDir) * Speed;
			}
			velocityX = airDriftX;
			state = FighterState::Jump;
			stateFrame = 0;
			return;
		}

		if (!grounded)
		{
			velocityX = airDriftX;
			state = velocityY < 0.0f ? FighterState::Jump : FighterState::Fall;
			return;
		}

		if (Input.down)
		{
			velocityX = 0.0f;
			state = FighterState::Crouch;
			return;
		}

		if (MoveDir == 0)
		{
			velocityX = 0.0f;
			state = FighterState::Idle;
			return;
		}

		const bool Forward = MoveDir == facing;
		velocityX = static_cast<float>(MoveDir) * (Forward ? def->stats.walkSpeed : def->stats.backwardSpeed);
		state = Forward ? FighterState::WalkForward : FighterState::WalkBackward;
	}

	bool FighterRuntime::TryStartMove(const InputFrame& Input, int WorldFrame)
	{
		if (!grounded)
		{
			if (Input.kickPressed)
			{
				StartMove(def->airHeavy);
				return true;
			}
			if (Input.punchPressed)
			{
				StartMove(def->airLight);
				return true;
			}
			return false;
		}

		// Universal Grab: simultaneous Light + Heavy (within 3 frames) or explicit grab button
		const bool bSimultaneousGrab =
			(Input.punchPressed && Input.kick) ||
			(Input.kickPressed && Input.punch) ||
			(std::abs(lastPunchFrame - lastKickFrame) <= 3 && (Input.punchPressed || Input.kickPressed));
		if (Input.grabPressed || bSimultaneousGrab)
		{
			StartMove(def->universalGrab);
			return true;
		}

		// Super move: Down + Special or Super button with >= 50 meter
		if ((Input.superPressed || (Input.specialPressed && Input.down)) && meter >= def->superMove.meterCost)
		{
			meter = std::max(0, meter - def->superMove.meterCost);
			StartMove(def->superMove);
			return true;
		}

		// Specials: Special button (Forward/Neutral -> Special1, Back/Motion -> Special2)
		if (Input.specialPressed)
		{
			const bool bBackSpecial = (facing > 0 && Input.left) || (facing < 0 && Input.right) ||
				(WorldFrame - lastBackFrame <= 10);
			StartMove(bBackSpecial ? def->special2 : def->special1);
			return true;
		}

		const int MoveDir = (Input.right ? 1 : 0) - (Input.left ? 1 : 0);
		const bool Forward = MoveDir == facing;

		if (Input.down)
		{
			if (Input.kickPressed)
			{
				StartMove(def->crouchHeavy);
				return true;
			}
			if (Input.punchPressed)
			{
				StartMove(def->crouchLight);
				return true;
			}
			return false;
		}

		if (Forward && Input.kickPressed)
		{
			StartMove(def->forwardHeavy);
			return true;
		}

		if (Forward && Input.punchPressed)
		{
			StartMove(def->forwardLight);
			return true;
		}

		if (Input.kickPressed)
		{
			StartMove(def->standingHeavy);
			return true;
		}

		if (Input.punchPressed)
		{
			StartMove(def->standingLight);
			return true;
		}

		return false;
	}

	void FighterRuntime::StartMove(const MoveDef& Move)
	{
		currentMove = &Move;
		moveHasRegisteredContact = false;
		state = Move.state;
		stateFrame = 0;
		if (grounded)
		{
			velocityX = 0.0f;
		}
	}

	void FighterRuntime::FinishFrame()
	{
		if (hitStopFrames > 0)
		{
			hitStopFrames -= 1;
			return;
		}

		if (state == FighterState::GrabbedFront || state == FighterState::GrabbedLifted)
		{
			stateFrame += 1;
			return;
		}

		x += velocityX;
		y += velocityY;

		if (!grounded)
		{
			velocityY += def->stats.gravity;
			if (y >= static_cast<float>(kGroundY))
			{
				y = static_cast<float>(kGroundY);
				velocityY = 0.0f;
				airDriftX = 0.0f;
				grounded = true;
				if (currentMove && currentMove->air)
				{
					currentMove = nullptr;
					moveHasRegisteredContact = false;
				}
				if (state == FighterState::Thrown)
				{
					state = FighterState::Knockdown;
					stateFrame = 0;
					knockdownFrames = 24;
				}
				else if (state != FighterState::Knockdown && state != FighterState::Knockout)
				{
					state = FighterState::Landing;
					stateFrame = 0;
				}
				x = std::clamp(x, static_cast<float>(kStageLeft), static_cast<float>(kStageRight));
				return;
			}
		}

		x = std::clamp(x, static_cast<float>(kStageLeft), static_cast<float>(kStageRight));
		stateFrame += 1;
	}

	bool FighterRuntime::HasActiveHitbox(HitboxDef& OutHitbox, Rect& OutWorldRect) const
	{
		if (!currentMove || moveHasRegisteredContact)
		{
			return false;
		}
		if (stateFrame < currentMove->hitbox.activeFrom || stateFrame > currentMove->hitbox.activeTo)
		{
			return false;
		}
		OutHitbox = currentMove->hitbox;
		OutWorldRect = ToWorldRect(OutHitbox.rect, x, y, facing);
		return true;
	}

	std::vector<Rect> FighterRuntime::CurrentHurtboxesWorld() const
	{
		std::vector<Rect> Out;
		if (state == FighterState::WakeUp || state == FighterState::Knockout ||
			state == FighterState::GrabbedFront || state == FighterState::GrabbedLifted)
		{
			return Out;
		}

		const bool Crouching = state == FighterState::Crouch
			|| state == FighterState::BlockCrouching
			|| (currentMove && currentMove->crouch);

		if (Crouching)
		{
			Out.reserve(def->crouchingHurtboxes.size());
			for (const Rect& Box : def->crouchingHurtboxes)
			{
				Out.push_back(ToWorldRect(Box, x, y, facing));
			}
		}
		else
		{
			Out.reserve(def->standingHurtboxes.size());
			for (const Rect& Box : def->standingHurtboxes)
			{
				Out.push_back(ToWorldRect(Box, x, y, facing));
			}
		}
		return Out;
	}

	Rect FighterRuntime::PushboxWorld() const
	{
		return ToWorldRect(def->stats.pushbox, x, y, facing);
	}

	AppliedHit FighterRuntime::ReceiveHit(const HitboxDef& Hitbox, int AttackerFacing, bool bAllowGrabTech)
	{
		if (Hitbox.kind == HitKind::Throw)
		{
			if (!grounded || state == FighterState::HitStun || state == FighterState::Knockdown || state == FighterState::WakeUp)
			{
				return AppliedHit{};
			}
			if (bAllowGrabTech)
			{
				velocityX = static_cast<float>(AttackerFacing) * 3.2f;
				stunFrames = 10;
				state = FighterState::BlockStanding;
				stateFrame = 0;
				return AppliedHit{0, false, false, false, true, false};
			}
			currentMove = nullptr;
			moveHasRegisteredContact = false;
			hitStopFrames = Hitbox.hitStop;
			return AppliedHit{0, false, false, true, false, false};
		}

		if (parryFrames > 0 && Hitbox.level != HitLevel::Low)
		{
			parryFrames = 0;
			hitStopFrames = 8;
			meter = std::min(kMaxMeter, meter + 18);
			return AppliedHit{0, true, false, false, false, true};
		}

		const bool Blocking = state == FighterState::BlockStanding || state == FighterState::BlockCrouching;
		const bool LowBlock = state == FighterState::BlockCrouching;
		const bool CanBlock = Blocking
			&& !(Hitbox.level == HitLevel::Low && !LowBlock)
			&& !(Hitbox.level == HitLevel::Overhead && LowBlock);

		if (CanBlock)
		{
			const int Chip = std::max(0, static_cast<int>( std::round(static_cast<float>(Hitbox.chipDamage) * damageReductionMultiplier)));
			health = std::max(1, health - Chip);
			stunFrames = Hitbox.blockStun;
			hitStopFrames = Hitbox.hitStop;
			velocityX = static_cast<float>(AttackerFacing) * Hitbox.knockbackX * 0.55f;
			meter = std::min(kMaxMeter, meter + 4);
			stateFrame = 0;
			return AppliedHit{Chip, true, false, false, false, false};
		}

		const int FinalDamage = std::max(1, static_cast<int>(std::round(static_cast<float>(Hitbox.damage) * damageReductionMultiplier)));
		health = std::max(0, health - FinalDamage);
		hitStopFrames = Hitbox.hitStop;
		meter = std::min(kMaxMeter, meter + 6);

		if (armorHits > 0 && health > 0 && !Hitbox.knockdown)
		{
			armorHits -= 1;
			return AppliedHit{FinalDamage, false, false, false, false, false};
		}

		currentMove = nullptr;
		moveHasRegisteredContact = false;
		velocityX = static_cast<float>(AttackerFacing) * (Hitbox.knockbackX / std::max(0.5f, def->stats.weight));

		if (health <= 0)
		{
			state = FighterState::Knockout;
			stateFrame = 0;
			velocityY = -4.0f;
			grounded = false;
			return AppliedHit{FinalDamage, false, true, false, false, false};
		}

		if (Hitbox.freezeFrames > 0)
		{
			freezeFrames = Hitbox.freezeFrames;
			state = FighterState::Frozen;
			stateFrame = 0;
			return AppliedHit{FinalDamage, false, false, false, false, false};
		}

		if (Hitbox.knockdown || !grounded)
		{
			state = FighterState::Knockdown;
			stateFrame = 0;
			knockdownFrames = 26;
			if (Hitbox.knockbackY < 0.0f)
			{
				velocityY = Hitbox.knockbackY;
				grounded = false;
			}
		}
		else
		{
			state = FighterState::HitStun;
			stateFrame = 0;
			stunFrames = Hitbox.hitStun;
		}

		return AppliedHit{FinalDamage, false, false, false, false, false};
	}

	void FighterRuntime::NotifyAttackConnected(bool bBlocked, const MoveDef& Move)
	{
		moveHasRegisteredContact = true;
		hitStopFrames = Move.hitbox.hitStop;
		meter = std::min(kMaxMeter, meter + (bBlocked ? Move.meterGainOnBlock : Move.meterGainOnHit));
	}

	void FighterRuntime::ApplyHorizontalFriction()
	{
		if (!grounded)
		{
			return;
		}
		velocityX *= 0.78f;
		if (std::fabs(velocityX) < 0.08f)
		{
			velocityX = 0.0f;
		}
	}

	AnimId FighterRuntime::CurrentAnim() const
	{
		if (currentMove)
		{
			return currentMove->anim;
		}
		switch (state)
		{
		case FighterState::WalkForward: return AnimId::Walk;
		case FighterState::WalkBackward: return AnimId::WalkBackward;
		case FighterState::Jump:
			if (airDriftX * static_cast<float>(facing) > 0.1f) return AnimId::JumpForward;
			if (airDriftX * static_cast<float>(facing) < -0.1f) return AnimId::JumpBackward;
			return AnimId::JumpNeutral;
		case FighterState::Fall: return AnimId::Fall;
		case FighterState::Landing: return AnimId::Landing;
		case FighterState::Crouch: return AnimId::Crouch;
		case FighterState::BlockStanding: return AnimId::BlockStanding;
		case FighterState::BlockCrouching: return AnimId::BlockCrouching;
		case FighterState::HitStun: return AnimId::Hit;
		case FighterState::Knockdown: return AnimId::Knockdown;
		case FighterState::WakeUp: return AnimId::WakeUp;
		case FighterState::GrabbedFront: return AnimId::GrabbedFront;
		case FighterState::GrabbedLifted: return AnimId::GrabbedLifted;
		case FighterState::Thrown: return AnimId::Thrown;
		case FighterState::Frozen: return AnimId::Frozen;
		case FighterState::Victory: return AnimId::Victory;
		case FighterState::Knockout: return AnimId::Knockout;
		default: return AnimId::Idle;
		}
	}

	FighterSnapshot FighterRuntime::Snapshot() const
	{
		FighterSnapshot Out{};
		Out.id = def->id;
		Out.name = def->name;
		Out.x = x;
		Out.y = y;
		Out.velocityX = velocityX;
		Out.velocityY = velocityY;
		Out.facing = facing;
		Out.state = state;
		Out.anim = CurrentAnim();
		Out.stateFrame = stateFrame;
		Out.health = health;
		Out.maxHealth = def->stats.maxHealth;
		Out.meter = meter;
		Out.roundWins = roundWins;
		Out.grounded = grounded;
		Out.attacking = currentMove != nullptr;
		Out.moveId = currentMove ? currentMove->id : "";
		Out.moveLabel = currentMove ? currentMove->label : "";
		Out.moveFrame = stateFrame;
		Out.moveTotalFrames = currentMove ? currentMove->totalFrames : 0;
		Out.hitActiveFrom = currentMove ? currentMove->hitbox.activeFrom : 0;
		Out.hitActiveTo = currentMove ? currentMove->hitbox.activeTo : 0;
		Out.armorHits = armorHits;
		Out.parryFrames = parryFrames;
		Out.damageReductionFrames = damageReductionFrames;
		Out.freezeFrames = freezeFrames;
		Out.victimRotation = victimRotation;
		Out.visible = visible;
		Out.pushboxWorld = PushboxWorld();
		Out.hurtboxesWorld = CurrentHurtboxesWorld();

		HitboxDef Hit{};
		Rect WorldHit{};
		if (HasActiveHitbox(Hit, WorldHit))
		{
			Out.hitboxesWorld.push_back(WorldHit);
		}
		return Out;
	}

	int MoveAnimFrame(const MoveDef& Move, int StateFrame, int SheetFrames)
	{
		if (SheetFrames <= 1)
		{
			return 0;
		}
		const int Clamped = std::clamp(StateFrame, 0, std::max(1, Move.totalFrames));
		const int Frame = (Clamped * SheetFrames) / std::max(1, Move.totalFrames);
		return std::clamp(Frame, 0, SheetFrames - 1);
	}
}
