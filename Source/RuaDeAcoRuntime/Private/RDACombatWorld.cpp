#include "RDACombatWorld.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace RDA
{
	CombatWorld::CombatWorld(
		const FighterDef& PlayerOne,
		const FighterDef& PlayerTwo,
		GameMode InMode,
		StageId InStage)
		: fighters{FighterRuntime(PlayerOne, 188.0f, 1), FighterRuntime(PlayerTwo, 452.0f, -1)}
		, mode(InMode)
		, stage(InStage)
	{
		if (mode == GameMode::Training)
		{
			fighters[0].ForceMeter(kMaxMeter);
			fighters[1].ForceMeter(kMaxMeter);
		}
	}

	void CombatWorld::RestartMatch()
	{
		frame = 0;
		phase = CombatPhase::Intro;
		phaseFrame = 0;
		round = 1;
		timeFrames = kRoundTimeFrames;
		paused = false;
		roundDraw = false;
		roundWinner = -1;
		matchWinner = -1;
		finisherWinner = -1;
		finisherCompleted = false;
		lastDamage = 0;
		comboCounts = {0, 0};
		comboTimers = {0, 0};
		projectiles.clear();
		activeGrab = ActiveGrabState{};
		fighters[0].ResetForRound(188.0f, 1);
		fighters[1].ResetForRound(452.0f, -1);
		fighters[0].roundWins = 0;
		fighters[1].roundWins = 0;
		fighters[0].meter = mode == GameMode::Training ? kMaxMeter : 0;
		fighters[1].meter = mode == GameMode::Training ? kMaxMeter : 0;
	}

	void CombatWorld::ResetTrainingPositions()
	{
		if (mode != GameMode::Training) return;
		activeGrab = ActiveGrabState{};
		projectiles.clear();
		fighters[0].ResetForRound(220.0f, 1);
		fighters[1].ResetForRound(420.0f, -1);
		fighters[0].ForceMeter(kMaxMeter);
		fighters[1].ForceMeter(kMaxMeter);
		comboCounts = {0, 0};
		comboTimers = {0, 0};
		lastDamage = 0;
	}

	void CombatWorld::TogglePause()
	{
		paused = !paused;
	}

	void CombatWorld::ToggleDebugBoxes()
	{
		debugBoxes = !debugBoxes;
	}

	std::vector<SoundEvent> CombatWorld::DrainSounds()
	{
		std::vector<SoundEvent> Out;
		Out.swap(soundEvents);
		return Out;
	}

	void CombatWorld::Step(const InputFrame& P1Input, const InputFrame& P2Input)
	{
		if (P1Input.debugPressed || P2Input.debugPressed)
		{
			debugBoxes = !debugBoxes;
		}
		if (paused)
		{
			return;
		}

		frame += 1;
		phaseFrame += 1;

		if (phase == CombatPhase::Intro)
		{
			StepIntro();
			return;
		}
		if (phase == CombatPhase::FinishReady)
		{
			StepFinishReady(P1Input, P2Input);
			return;
		}
		if (phase == CombatPhase::StageFinish)
		{
			StepStageFinish();
			return;
		}
		if (phase == CombatPhase::RoundOver)
		{
			StepRoundOver();
			return;
		}
		if (phase == CombatPhase::MatchOver)
		{
			fighters[0].FinishFrame();
			fighters[1].FinishFrame();
			return;
		}

		StepActive(P1Input, P2Input);
	}

	void CombatWorld::StepIntro()
	{
		InputFrame Empty{};
		fighters[0].facing = fighters[0].x <= fighters[1].x ? 1 : -1;
		fighters[1].facing = fighters[1].x <= fighters[0].x ? 1 : -1;
		fighters[0].BeginFrame(Empty, frame);
		fighters[1].BeginFrame(Empty, frame);
		fighters[0].FinishFrame();
		fighters[1].FinishFrame();

		if (phaseFrame == 60)
		{
			soundEvents.push_back(SoundEvent::Fight);
		}
		if (phaseFrame >= 105)
		{
			phase = CombatPhase::Active;
			phaseFrame = 0;
		}
	}

	void CombatWorld::StepActive(const InputFrame& P1Input, const InputFrame& P2Input)
	{
		FighterRuntime& One = fighters[0];
		FighterRuntime& Two = fighters[1];

		if (One.grounded && !One.currentMove && !activeGrab.active)
		{
			One.facing = One.x <= Two.x ? 1 : -1;
		}
		if (Two.grounded && !Two.currentMove && !activeGrab.active)
		{
			Two.facing = Two.x <= One.x ? 1 : -1;
		}

		const MoveDef* PrevMove0 = One.currentMove;
		const MoveDef* PrevMove1 = Two.currentMove;
		const bool WasGrounded0 = One.grounded;
		const bool WasGrounded1 = Two.grounded;

		One.BeginFrame(P1Input, frame);
		Two.BeginFrame(P2Input, frame);

		if (One.currentMove && One.currentMove != PrevMove0)
		{
			soundEvents.push_back(One.currentMove->isSuper ? SoundEvent::SuperCast :
				One.currentMove->isSpecial ? SoundEvent::SpecialCast : SoundEvent::Swing);
		}
		if (Two.currentMove && Two.currentMove != PrevMove1)
		{
			soundEvents.push_back(Two.currentMove->isSuper ? SoundEvent::SuperCast :
				Two.currentMove->isSpecial ? SoundEvent::SpecialCast : SoundEvent::Swing);
		}
		if (WasGrounded0 && !One.grounded && One.state == FighterState::Jump)
		{
			soundEvents.push_back(SoundEvent::Jump);
		}
		if (WasGrounded1 && !Two.grounded && Two.state == FighterState::Jump)
		{
			soundEvents.push_back(SoundEvent::Jump);
		}

		SpawnProjectiles(0);
		SpawnProjectiles(1);
		UpdateProjectiles();
		ResolveHits(P1Input, P2Input);
		ResolvePushboxes();

		One.FinishFrame();
		Two.FinishFrame();
		UpdateActiveGrab();

		for (int i = 0; i < 2; ++i)
		{
			if (comboTimers[static_cast<size_t>(i)] > 0)
			{
				comboTimers[static_cast<size_t>(i)] -= 1;
				if (comboTimers[static_cast<size_t>(i)] == 0)
				{
					comboCounts[static_cast<size_t>(i)] = 0;
				}
			}
		}

		if (mode == GameMode::Training)
		{
			One.ForceMeter(kMaxMeter);
			Two.ForceMeter(kMaxMeter);
			if (One.health <= 150) One.health = One.def->stats.maxHealth;
			if (Two.health <= 150 && Two.state == FighterState::Idle) Two.health = Two.def->stats.maxHealth;
			return;
		}

		timeFrames = std::max(0, timeFrames - 1);
		if (One.health <= 0 || Two.health <= 0 || timeFrames == 0)
		{
			const bool bKnockout = One.health <= 0 || Two.health <= 0;
			const float R1 = static_cast<float>(One.health) / static_cast<float>(One.def->stats.maxHealth);
			const float R2 = static_cast<float>(Two.health) / static_cast<float>(Two.def->stats.maxHealth);
			int Winner = -1;
			if (std::fabs(R1 - R2) >= 0.0001f)
			{
				Winner = R1 > R2 ? 0 : 1;
			}
			FinishRound(Winner, bKnockout);
		}
	}

	void CombatWorld::SpawnProjectiles(int OwnerIndex)
	{
		FighterRuntime& Owner = fighters[static_cast<size_t>(OwnerIndex)];
		FighterRuntime& Target = fighters[static_cast<size_t>(1 - OwnerIndex)];
		ProjectileDef Def{};
		if (!Owner.ConsumePendingProjectile(Def))
		{
			return;
		}

		// Max 1 active projectile of the same id per owner
		projectiles.erase(
			std::remove_if(projectiles.begin(), projectiles.end(), [&](const ProjectileRuntime& P) {
				return P.ownerIndex == OwnerIndex && std::strcmp(P.def.id, Def.id) == 0;
			}),
			projectiles.end());

		ProjectileRuntime Proj{};
		Proj.runtimeId = nextProjectileId++;
		Proj.ownerIndex = OwnerIndex;
		Proj.def = Def;
		Proj.facing = Owner.facing;
		if (Def.targetSnapshot)
		{
			Proj.x = Target.x;
			Proj.y = static_cast<float>(kGroundY);
			Proj.velocityX = 0.0f;
		}
		else
		{
			Proj.x = Owner.x + static_cast<float>(Owner.facing) * Def.offsetX;
			Proj.y = Owner.y + Def.offsetY;
			Proj.velocityX = static_cast<float>(Owner.facing) * Def.velocityX;
		}
		Proj.life = Def.lifeFrames;
		Proj.ageFrames = 0;
		projectiles.push_back(Proj);
	}

	void CombatWorld::UpdateProjectiles()
	{
		for (auto It = projectiles.begin(); It != projectiles.end();)
		{
			It->x += It->velocityX;
			It->ageFrames += 1;
			It->life -= 1;
			if (It->life <= 0 || It->x < static_cast<float>(kStageLeft - 80) || It->x > static_cast<float>(kStageRight + 80))
			{
				It = projectiles.erase(It);
			}
			else
			{
				++It;
			}
		}
	}

	void CombatWorld::ResolveHits(const InputFrame& P1Input, const InputFrame& P2Input)
	{
		FighterRuntime& One = fighters[0];
		FighterRuntime& Two = fighters[1];

		auto CheckContact = [](const FighterRuntime& Attacker, const FighterRuntime& Defender, HitboxDef& OutHit) -> bool
		{
			Rect HitWorld{};
			if (!Attacker.HasActiveHitbox(OutHit, HitWorld))
			{
				return false;
			}
			for (const Rect& Hurt : Defender.CurrentHurtboxesWorld())
			{
				if (Intersects(HitWorld, Hurt))
				{
					return true;
				}
			}
			return false;
		};

		HitboxDef Hit01{};
		HitboxDef Hit10{};
		const bool bContact01 = CheckContact(One, Two, Hit01);
		const bool bContact10 = CheckContact(Two, One, Hit10);

		const bool bP1Tech = (P1Input.punchPressed && P1Input.kick) || (P1Input.kickPressed && P1Input.punch) || P1Input.grabPressed;
		const bool bP2Tech = (P2Input.punchPressed && P2Input.kick) || (P2Input.kickPressed && P2Input.punch) || P2Input.grabPressed;

		auto ApplyStrikeOrThrow = [this](int AttackerIdx, const HitboxDef& Hit, bool bDefenderTech)
		{
			FighterRuntime& Attacker = fighters[static_cast<size_t>(AttackerIdx)];
			FighterRuntime& Defender = fighters[static_cast<size_t>(1 - AttackerIdx)];
			const MoveDef* Move = Attacker.currentMove;
			const AppliedHit Result = Defender.ReceiveHit(Hit, Attacker.facing, bDefenderTech);

			if (Result.grabTeched)
			{
				Attacker.currentMove = nullptr;
				Attacker.state = FighterState::HitStun;
				Attacker.stunFrames = 10;
				soundEvents.push_back(SoundEvent::Block);
				return;
			}

			if (Result.grabConnected && Move && Move->isGrab)
			{
				Attacker.NotifyAttackConnected(false, *Move);
				activeGrab.active = true;
				activeGrab.attackerIndex = AttackerIdx;
				activeGrab.defenderIndex = 1 - AttackerIdx;
				activeGrab.move = *Move;
				Defender.EnterGrabbedState(
					FighterState::GrabbedFront,
					Attacker.x + static_cast<float>(Attacker.facing) * 44.0f,
					static_cast<float>(kGroundY),
					0.0f,
					-Attacker.facing);
				soundEvents.push_back(SoundEvent::GrabStart);
				return;
			}

			if (Result.parryTriggered)
			{
				Attacker.ReceiveHit(
					HitboxDef{Rect{0, -50, 40, 40}, HitLevel::Mid, HitKind::Strike, 1, 1, Defender.parryRiposteDamage, 0, 22, 12, 10, 5, 4.2f, -2.0f, true},
					Defender.facing,
					false);
				soundEvents.push_back(SoundEvent::HitHeavy);
				return;
			}

			if (Move)
			{
				Attacker.NotifyAttackConnected(Result.blocked, *Move);
			}
			if (!Result.blocked && Result.damage > 0)
			{
				lastDamage = Result.damage;
				comboCounts[static_cast<size_t>(AttackerIdx)] += 1;
				comboTimers[static_cast<size_t>(AttackerIdx)] = 45;
			}
			if (Result.blocked)
			{
				soundEvents.push_back(SoundEvent::Block);
			}
			else if (Result.knockout || Hit.knockdown)
			{
				soundEvents.push_back(SoundEvent::Knockdown);
			}
			else
			{
				soundEvents.push_back(Hit.damage >= 90 ? SoundEvent::HitHeavy : SoundEvent::HitLight);
			}
		};

		if (bContact01 && bContact10)
		{
			if (Hit01.priority >= Hit10.priority)
			{
				ApplyStrikeOrThrow(0, Hit01, bP2Tech);
			}
			if (Hit10.priority >= Hit01.priority && !activeGrab.active)
			{
				ApplyStrikeOrThrow(1, Hit10, bP1Tech);
			}
		}
		else if (bContact01)
		{
			ApplyStrikeOrThrow(0, Hit01, bP2Tech);
		}
		else if (bContact10)
		{
			ApplyStrikeOrThrow(1, Hit10, bP1Tech);
		}

		// Resolve projectile contacts
		for (auto It = projectiles.begin(); It != projectiles.end();)
		{
			if (It->ageFrames < It->def.armingFrames)
			{
				++It;
				continue;
			}
			FighterRuntime& Defender = fighters[static_cast<size_t>(1 - It->ownerIndex)];
			if (It->def.hitbox.crouchAvoidable && Defender.state == FighterState::Crouch)
			{
				++It;
				continue;
			}
			if (It->def.hitbox.airAvoidable && !Defender.grounded)
			{
				++It;
				continue;
			}
			const Rect ProjRect = ToWorldRect(It->def.hitbox.rect, It->x, It->y, It->facing);
			bool bHit = false;
			for (const Rect& Hurt : Defender.CurrentHurtboxesWorld())
			{
				if (Intersects(ProjRect, Hurt))
				{
					bHit = true;
					break;
				}
			}
			if (bHit)
			{
				const AppliedHit Res = Defender.ReceiveHit(It->def.hitbox, It->facing, false);
				if (!Res.blocked && Res.damage > 0)
				{
					lastDamage = Res.damage;
					comboCounts[static_cast<size_t>(It->ownerIndex)] += 1;
					comboTimers[static_cast<size_t>(It->ownerIndex)] = 45;
				}
				soundEvents.push_back(Res.blocked ? SoundEvent::Block : SoundEvent::HitHeavy);
				It = projectiles.erase(It);
			}
			else
			{
				++It;
			}
		}
	}

	void CombatWorld::UpdateActiveGrab()
	{
		if (!activeGrab.active)
		{
			return;
		}
		FighterRuntime& Attacker = fighters[static_cast<size_t>(activeGrab.attackerIndex)];
		FighterRuntime& Defender = fighters[static_cast<size_t>(activeGrab.defenderIndex)];

		if (!Attacker.currentMove || !Attacker.currentMove->isGrab)
		{
			activeGrab.active = false;
			if (Defender.state == FighterState::GrabbedFront || Defender.state == FighterState::GrabbedLifted)
			{
				Defender.ReleaseFromGrab(0.0f, 0.0f, true);
			}
			return;
		}

		const int F = Attacker.stateFrame;
		if (F < activeGrab.move.holdStartFrame)
		{
			Defender.EnterGrabbedState(
				FighterState::GrabbedFront,
				Attacker.x + static_cast<float>(Attacker.facing) * 44.0f,
				static_cast<float>(kGroundY),
				0.0f,
				-Attacker.facing);
		}
		else if (F < activeGrab.move.releaseFrame)
		{
			const float T = static_cast<float>(F - activeGrab.move.holdStartFrame) /
				static_cast<float>(std::max(1, activeGrab.move.releaseFrame - activeGrab.move.holdStartFrame));
			const float ArcY = -std::sin(T * 3.14159265f) * 72.0f;
			const float OffsetX = 44.0f + T * 16.0f;
			Defender.EnterGrabbedState(
				FighterState::GrabbedLifted,
				Attacker.x + static_cast<float>(Attacker.facing) * OffsetX,
				static_cast<float>(kGroundY) + ArcY,
				T * 1.57f * static_cast<float>(Attacker.facing),
				-Attacker.facing);
		}
		else
		{
			const int GrabDamage = activeGrab.move.hitbox.damage;
			Defender.health = std::max(0, Defender.health - GrabDamage);
			lastDamage = GrabDamage;
			comboCounts[static_cast<size_t>(activeGrab.attackerIndex)] += 1;
			comboTimers[static_cast<size_t>(activeGrab.attackerIndex)] = 45;
			Defender.ReleaseFromGrab(static_cast<float>(Attacker.facing) * 2.0f, 0.0f, true);
			if (Defender.health <= 0)
			{
				Defender.state = FighterState::Knockout;
				Defender.stateFrame = 0;
			}
			activeGrab.active = false;
			soundEvents.push_back(SoundEvent::GrabSlam);
		}
	}

	void CombatWorld::ResolvePushboxes()
	{
		if (activeGrab.active) return;
		FighterRuntime& One = fighters[0];
		FighterRuntime& Two = fighters[1];
		if (!One.grounded || !Two.grounded)
		{
			return;
		}

		const Rect A = One.PushboxWorld();
		const Rect B = Two.PushboxWorld();
		if (!Intersects(A, B))
		{
			return;
		}
		const float Overlap = HorizontalOverlap(A, B);
		if (Overlap <= 0.0f)
		{
			return;
		}

		const float Push = Overlap * 0.5f;
		const float Dir = One.x <= Two.x ? 1.0f : -1.0f;
		One.x = std::clamp(One.x - Dir * Push, static_cast<float>(kStageLeft), static_cast<float>(kStageRight));
		Two.x = std::clamp(Two.x + Dir * Push, static_cast<float>(kStageLeft), static_cast<float>(kStageRight));
	}

	void CombatWorld::FinishRound(int WinnerIndex, bool bKnockout)
	{
		activeGrab = ActiveGrabState{};
		projectiles.clear();
		roundWinner = WinnerIndex;
		roundDraw = WinnerIndex < 0;
		soundEvents.push_back(SoundEvent::RoundEnd);

		if (WinnerIndex >= 0)
		{
			fighters[static_cast<size_t>(WinnerIndex)].roundWins += 1;
			const int LoserIndex = 1 - WinnerIndex;
			if (!bKnockout)
			{
				fighters[static_cast<size_t>(LoserIndex)].ForceState(FighterState::Knockdown);
			}

			if (fighters[static_cast<size_t>(WinnerIndex)].roundWins >= kRoundsToWin)
			{
				matchWinner = WinnerIndex;
				finisherWinner = WinnerIndex;
				phase = CombatPhase::FinishReady;
				phaseFrame = 0;
				fighters[static_cast<size_t>(WinnerIndex)].ForceState(FighterState::Idle);
				fighters[static_cast<size_t>(WinnerIndex)].hitStopFrames = 0;
				fighters[static_cast<size_t>(LoserIndex)].ForceState(FighterState::Knockdown);
				fighters[static_cast<size_t>(LoserIndex)].hitStopFrames = 0;
				soundEvents.push_back(SoundEvent::FinishReady);
				return;
			}

			fighters[static_cast<size_t>(WinnerIndex)].ForceState(FighterState::Victory);
		}

		phase = CombatPhase::RoundOver;
		phaseFrame = 0;
	}

	void CombatWorld::StepFinishReady(const InputFrame& P1Input, const InputFrame& P2Input)
	{
		if (finisherWinner < 0)
		{
			phase = CombatPhase::MatchOver;
			phaseFrame = 0;
			return;
		}

		FighterRuntime& Winner = fighters[static_cast<size_t>(finisherWinner)];
		FighterRuntime& Loser = fighters[static_cast<size_t>(1 - finisherWinner)];
		const InputFrame& WinInput = finisherWinner == 0 ? P1Input : P2Input;
		InputFrame Empty{};

		Winner.facing = Winner.x <= Loser.x ? 1 : -1;
		Loser.facing = -Winner.facing;
		Winner.BeginFrame(WinInput, frame);
		Loser.BeginFrame(Empty, frame);
		Winner.FinishFrame();
		Loser.FinishFrame();

		const float Distance = std::fabs(Winner.x - Loser.x);
		const bool bGrabAttempt = Winner.currentMove && Winner.currentMove->isGrab;
		if (bGrabAttempt && Distance <= 135.0f)
		{
			phase = CombatPhase::StageFinish;
			phaseFrame = 0;
			soundEvents.push_back(SoundEvent::GrabStart);
			return;
		}

		if (phaseFrame >= kFinishWindowFrames)
		{
			Winner.ForceState(FighterState::Victory);
			Loser.ForceState(FighterState::Knockout);
			phase = CombatPhase::MatchOver;
			phaseFrame = 0;
			soundEvents.push_back(SoundEvent::MatchWin);
		}
	}

	void CombatWorld::StepStageFinish()
	{
		if (finisherWinner < 0)
		{
			phase = CombatPhase::MatchOver;
			return;
		}
		FighterRuntime& Winner = fighters[static_cast<size_t>(finisherWinner)];
		FighterRuntime& Loser = fighters[static_cast<size_t>(1 - finisherWinner)];

		if (phaseFrame < kFinishThrowFrame)
		{
			const float Lift = static_cast<float>(phaseFrame) / static_cast<float>(kFinishThrowFrame);
			Loser.EnterGrabbedState(
				FighterState::GrabbedLifted,
				Winner.x + static_cast<float>(Winner.facing) * 32.0f,
				static_cast<float>(kGroundY) - 42.0f - Lift * 46.0f,
				1.57f * static_cast<float>(Winner.facing),
				-Winner.facing);
		}
		else
		{
			const float T = std::clamp(
				static_cast<float>(phaseFrame - kFinishThrowFrame) / 52.0f,
				0.0f,
				1.0f);
			float TargetX = std::clamp(Winner.x + static_cast<float>(Winner.facing) * 175.0f, 150.0f, 490.0f);
			float TargetY = 256.0f;
			if (stage == StageId::CozinhaMacabra)
			{
				TargetX = 442.0f;
				TargetY = 152.0f;
			}
			else if (stage == StageId::Sitio)
			{
				TargetX = 328.0f;
				TargetY = 165.0f;
			}
			const float CurX = Winner.x + (TargetX - Winner.x) * T;
			const float CurY = (static_cast<float>(kGroundY) - 88.0f) * (1.0f - T) + TargetY * T - 80.0f * 4.0f * T * (1.0f - T);
			Loser.EnterGrabbedState(
				FighterState::GrabbedLifted,
				CurX,
				CurY,
				T * 6.28f * static_cast<float>(Winner.facing),
				-Winner.facing);
			if (phaseFrame >= 178)
			{
				Loser.visible = false;
			}
		}

		if (phaseFrame == kFinishThrowFrame)
		{
			if (stage == StageId::CaisDaCidade) soundEvents.push_back(SoundEvent::FinisherMonsterRoar);
			else if (stage == StageId::CozinhaMacabra) soundEvents.push_back(SoundEvent::FinisherPotDrop);
			else soundEvents.push_back(SoundEvent::FinisherTrident);
		}
		if (phaseFrame == 112 && stage == StageId::CaisDaCidade)
		{
			soundEvents.push_back(SoundEvent::FinisherSplash);
		}

		if (phaseFrame >= kFinishEndFrame)
		{
			finisherCompleted = true;
			Winner.ForceState(FighterState::Victory);
			phase = CombatPhase::MatchOver;
			phaseFrame = 0;
			soundEvents.push_back(SoundEvent::MatchWin);
		}
	}

	void CombatWorld::StepRoundOver()
	{
		fighters[0].FinishFrame();
		fighters[1].FinishFrame();
		if (phaseFrame >= 150)
		{
			StartNextRound();
		}
	}

	void CombatWorld::StartNextRound()
	{
		round += 1;
		phase = CombatPhase::Intro;
		phaseFrame = 0;
		timeFrames = kRoundTimeFrames;
		roundDraw = false;
		roundWinner = -1;
		projectiles.clear();
		activeGrab = ActiveGrabState{};
		fighters[0].ResetForRound(188.0f, 1);
		fighters[1].ResetForRound(452.0f, -1);
	}

	WorldSnapshot CombatWorld::Snapshot() const
	{
		WorldSnapshot Out{};
		Out.frame = frame;
		Out.phase = phase;
		Out.phaseFrame = phaseFrame;
		Out.round = round;
		Out.timeSeconds = std::max(0, (timeFrames + 59) / 60);
		Out.paused = paused;
		Out.debugBoxes = debugBoxes;
		Out.roundDraw = roundDraw;
		Out.roundWinner = roundWinner;
		Out.matchWinner = matchWinner;
		Out.finisherWinner = finisherWinner;
		Out.finisherCompleted = finisherCompleted;
		Out.stage = stage;
		Out.mode = mode;
		Out.fighters = {fighters[0].Snapshot(), fighters[1].Snapshot()};
		Out.fighters[0].comboCount = comboCounts[0];
		Out.fighters[1].comboCount = comboCounts[1];
		Out.grabActive = activeGrab.active;
		Out.lastDamage = lastDamage;

		for (const ProjectileRuntime& P : projectiles)
		{
			Out.projectiles.push_back(ProjectileSnapshot{
				P.runtimeId,
				P.ownerIndex,
				P.def.id,
				P.x,
				P.y,
				P.facing,
				ToWorldRect(P.def.hitbox.rect, P.x, P.y, P.facing),
				P.ageFrames >= P.def.armingFrames
			});
		}

		if (paused)
		{
			Out.banner = "PAUSADO";
		}
		else if (phase == CombatPhase::Intro)
		{
			Out.banner = phaseFrame < 60 ? "PREPARE-SE" : "LUTE!";
		}
		else if (phase == CombatPhase::FinishReady)
		{
			Out.banner = "FINALIZE! (AGARRAO F+G / J+K)";
		}
		else if (phase == CombatPhase::StageFinish)
		{
			Out.banner = phaseFrame >= 210 ? GetStageDef(stage).finisherBanner : "FINALIZACAO DE PALCO!";
		}
		else if (phase == CombatPhase::RoundOver)
		{
			Out.banner = roundDraw ? "EMPATE" : "NOCAUTE!";
		}
		else if (phase == CombatPhase::MatchOver)
		{
			if (finisherCompleted)
			{
				Out.banner = GetStageDef(stage).finisherBanner;
			}
			else if (matchWinner == 0)
			{
				Out.banner = "JOGADOR 1 VENCE!";
			}
			else if (matchWinner == 1)
			{
				Out.banner = "JOGADOR 2 VENCE!";
			}
			else
			{
				Out.banner = "FIM DE LUTA";
			}
		}

		return Out;
	}

	uint64_t CombatWorld::ComputeStateHash() const
	{
		uint64_t Hash = 0xcbf29ce484222325ULL;
		auto MixInt = [&Hash](int64_t Val)
		{
			for (int i = 0; i < 8; ++i)
			{
				const uint8_t Byte = static_cast<uint8_t>(( static_cast<uint64_t>(Val) >> (i * 8)) & 0xFFU);
				Hash ^= Byte;
				Hash *= 0x100000001b3ULL;
			}
		};

		MixInt(frame);
		MixInt(static_cast<int>(phase));
		MixInt(phaseFrame);
		MixInt(round);
		MixInt(timeFrames);
		MixInt(static_cast<int>(stage));
		MixInt(static_cast<int>(mode));
		for (const FighterRuntime& F : fighters)
		{
			MixInt(static_cast<int>(F.def->id));
			MixInt(static_cast<int64_t>(std::llround(F.x * 1000.0f)));
			MixInt(static_cast<int64_t>(std::llround(F.y * 1000.0f)));
			MixInt(static_cast<int64_t>(std::llround(F.velocityX * 1000.0f)));
			MixInt(static_cast<int64_t>(std::llround(F.velocityY * 1000.0f)));
			MixInt(F.facing);
			MixInt(static_cast<int>(F.state));
			MixInt(F.stateFrame);
			MixInt(F.health);
			MixInt(F.meter);
			MixInt(F.roundWins);
			MixInt(F.grounded ? 1 : 0);
		}
		MixInt(static_cast<int64_t>(projectiles.size()));
		for (const ProjectileRuntime& P : projectiles)
		{
			MixInt(P.ownerIndex);
			MixInt(static_cast<int64_t>(std::llround(P.x * 1000.0f)));
			MixInt(static_cast<int64_t>(std::llround(P.y * 1000.0f)));
			MixInt(P.life);
		}
		return Hash;
	}

	InputFrame BuildCpuInput(const CombatWorld& World, int CpuIndex, AiDifficulty Difficulty)
	{
		InputFrame Out{};
		const WorldSnapshot Snap = World.Snapshot();
		if (Snap.paused || (Snap.phase != CombatPhase::Active && Snap.phase != CombatPhase::FinishReady))
		{
			return Out;
		}

		const int OpponentIndex = 1 - CpuIndex;
		const FighterSnapshot& Self = Snap.fighters[static_cast<size_t>(CpuIndex)];
		const FighterSnapshot& Foe = Snap.fighters[static_cast<size_t>(OpponentIndex)];
		const float Dx = Foe.x - Self.x;
		const float AbsDx = std::fabs(Dx);
		const int Cadence = Difficulty == AiDifficulty::Hard ? 9 : (Difficulty == AiDifficulty::Normal ? 15 : 24);
		const int Tick = (Snap.frame + CpuIndex * 13) % Cadence;

		// During FinishReady, walk up to defeated opponent and execute Universal Grab!
		if (Snap.phase == CombatPhase::FinishReady && Snap.finisherWinner == CpuIndex)
		{
			if (AbsDx > 90.0f)
			{
				Out.left = Dx < 0.0f;
				Out.right = Dx > 0.0f;
			}
			else if (Tick == 0)
			{
				Out.grabPressed = true;
				Out.punchPressed = true;
				Out.kick = true;
			}
			return Out;
		}

		// React to enemy projectiles by jumping or blocking
		for (const ProjectileSnapshot& P : Snap.projectiles)
		{
			if (P.ownerIndex != CpuIndex && std::fabs(P.x - Self.x) < 95.0f)
			{
				if (Difficulty == AiDifficulty::Hard && Self.grounded)
				{
					Out.up = true;
					Out.upPressed = true;
				}
				else
				{
					Out.block = true;
				}
				return Out;
			}
		}

		if (Foe.attacking && AbsDx < 105.0f && Tick < (Cadence / 3))
		{
			Out.block = true;
			Out.down = (Snap.frame % 2) == 0;
			return Out;
		}

		if (AbsDx > 110.0f)
		{
			Out.left = Dx < 0.0f;
			Out.right = Dx > 0.0f;
			if (Tick == 0 && AbsDx > 165.0f)
			{
				Out.special = true;
				Out.specialPressed = true;
			}
			return Out;
		}

		if (Self.meter >= 50 && Tick == 0 && Difficulty != AiDifficulty::Easy)
		{
			Out.superPressed = true;
			Out.down = true;
			Out.special = true;
			Out.specialPressed = true;
			return Out;
		}

		if (AbsDx < 62.0f && Tick == 2 && (Foe.state == FighterState::BlockStanding || Foe.state == FighterState::BlockCrouching))
		{
			Out.grabPressed = true;
			Out.punchPressed = true;
			Out.kick = true;
			return Out;
		}

		if (Tick == 0)
		{
			Out.punch = true;
			Out.punchPressed = true;
		}
		else if (Tick == (Cadence / 2))
		{
			Out.kick = true;
			Out.kickPressed = true;
			Out.left = Dx < 0.0f;
			Out.right = Dx > 0.0f;
		}
		else if (Tick == (Cadence - 2))
		{
			Out.down = true;
			Out.kick = true;
			Out.kickPressed = true;
		}
		return Out;
	}
}
