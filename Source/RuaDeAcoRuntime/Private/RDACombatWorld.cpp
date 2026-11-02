#include "RDACombatWorld.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace RDA
{
#include "RDAStageFinisher.inc"
CombatWorld::CombatWorld(const FighterDef &PlayerOne, const FighterDef &PlayerTwo, GameMode InMode, StageId InStage)
    : fighters{FighterRuntime(PlayerOne, 188.0f, 1), FighterRuntime(PlayerTwo, 452.0f, -1)}, mode(InMode),
      stage(InStage)
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
    if (mode != GameMode::Training)
        return;
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

std::vector<SoundCue> CombatWorld::DrainSoundCues()
{
    std::vector<SoundCue> result;
    result.swap(soundCues);
    return result;
}

void CombatWorld::Step(const InputFrame &P1Input, const InputFrame &P2Input)
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
        InputFrame Empty{};
        fighters[0].BeginFrame(Empty, frame);
        fighters[1].BeginFrame(Empty, frame);
        fighters[0].CaptureCollisionPose();
        fighters[1].CaptureCollisionPose();
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
    fighters[0].CaptureCollisionPose();
    fighters[1].CaptureCollisionPose();
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

void CombatWorld::StepActive(const InputFrame &P1Input, const InputFrame &P2Input)
{
    FighterRuntime &One = fighters[0];
    FighterRuntime &Two = fighters[1];

    if (One.grounded && !(One.currentMove && One.currentMove->lockFacing))
    {
        One.facing = One.x <= Two.x ? 1 : -1;
    }
    if (Two.grounded && !(Two.currentMove && Two.currentMove->lockFacing))
    {
        Two.facing = Two.x <= One.x ? 1 : -1;
    }

    const MoveDef *PrevMove0 = One.currentMove;
    const MoveDef *PrevMove1 = Two.currentMove;
    const bool WasGrounded0 = One.grounded;
    const bool WasGrounded1 = Two.grounded;

    One.BeginFrame(P1Input, frame);
    Two.BeginFrame(P2Input, frame);
    One.CaptureCollisionPose();
    Two.CaptureCollisionPose();

    if (One.currentMove && One.currentMove != PrevMove0)
    {
        if (One.currentMove->isSuper)
        {
            One.hitStopFrames = std::max(One.hitStopFrames, 30);
            Two.hitStopFrames = std::max(Two.hitStopFrames, 30);
        }
        const SoundEvent event = One.currentMove->isSuper     ? SoundEvent::SuperCast
                                 : One.currentMove->isSpecial ? SoundEvent::SpecialCast
                                                              : SoundEvent::Swing;
        soundEvents.push_back(event);
        soundCues.push_back(SoundCue{event, 0, One.currentMove->id});
    }
    if (Two.currentMove && Two.currentMove != PrevMove1)
    {
        if (Two.currentMove->isSuper)
        {
            One.hitStopFrames = std::max(One.hitStopFrames, 30);
            Two.hitStopFrames = std::max(Two.hitStopFrames, 30);
        }
        const SoundEvent event = Two.currentMove->isSuper     ? SoundEvent::SuperCast
                                 : Two.currentMove->isSpecial ? SoundEvent::SpecialCast
                                                              : SoundEvent::Swing;
        soundEvents.push_back(event);
        soundCues.push_back(SoundCue{event, 1, Two.currentMove->id});
    }
    if (WasGrounded0 && !One.grounded && One.state == FighterState::Jump)
    {
        soundEvents.push_back(SoundEvent::Jump);
    }
    if (WasGrounded1 && !Two.grounded && Two.state == FighterState::Jump)
    {
        soundEvents.push_back(SoundEvent::Jump);
    }

    One.ConsumeMoveEvents();
    Two.ConsumeMoveEvents();
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
        const auto defenderState = fighters[1 - i].state;
        if (defenderState != FighterState::HitStun && defenderState != FighterState::Thrown &&
            defenderState != FighterState::Knockdown)
        {
            comboTimers[i] = comboCounts[i] = 0;
            continue;
        }
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
    FighterRuntime &Owner = fighters[static_cast<size_t>(OwnerIndex)];
    FighterRuntime &Target = fighters[static_cast<size_t>(1 - OwnerIndex)];
    ProjectileDef Def{};
    if (!Owner.ConsumePendingProjectile(Def))
    {
        return;
    }

    if (Def.maxActivePerOwner >= 0 && std::count_if(projectiles.begin(), projectiles.end(), [&](const auto &p) {
                                          return p.ownerIndex == OwnerIndex && std::strcmp(p.def.id, Def.id) == 0;
                                      }) >= Def.maxActivePerOwner)
        return;

    ProjectileRuntime Proj{};
    Proj.runtimeId = nextProjectileId++;
    Proj.ownerIndex = OwnerIndex;
    Proj.def = Def;
    Proj.sourceMove = Owner.currentMove;
    Proj.facing = Owner.facing;
    if (Def.targetSnapshot)
    {
        Proj.x = Target.x + Def.offsetX;
        Proj.y = Target.y + Def.offsetY;
        Proj.velocityX = Owner.facing * Def.velocityX;
    }
    else
    {
        Proj.x = Owner.x + static_cast<float>(Owner.facing) * Def.offsetX;
        Proj.y = static_cast<float>(kGroundY) + Def.offsetY;
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
        if (It->life <= 0 || It->x <= static_cast<float>(kStageLeft - 30) ||
            It->x >= static_cast<float>(kStageRight + 30))
        {
            It = projectiles.erase(It);
        }
        else
        {
            ++It;
        }
    }
}

void CombatWorld::ResolveHits(const InputFrame &, const InputFrame &)
{
    struct Contact
    {
        bool found = false;
        HitboxDef h{};
    };
    auto find = [&](int ai) {
        Contact c;
        const auto &a = fighters[ai];
        const auto &d = fighters[1 - ai];
        if (a.grabbed || d.grabbed)
            return c;
        auto boxes = a.evaluatedHitboxes;
        std::stable_sort(boxes.begin(), boxes.end(), [](auto &a, auto &b) { return a.priority > b.priority; });
        for (const auto &h : boxes)
        {
            if (std::find(a.registeredHitIds.begin(), a.registeredHitIds.end(), h.id) != a.registeredHitIds.end() ||
                !d.CanReceive(h))
                continue;
            Rect wr = ToWorldRect(h.rect, a.x, a.y, a.facing);
            for (const auto &hr : d.CurrentHurtboxesWorld())
                if (Intersects(wr, hr))
                {
                    c.found = true;
                    c.h = h;
                    return c;
                }
        }
        return c;
    };
    auto outcome = [&](int ai, const HitboxDef &h, const MoveDef *m, const AppliedHit &res, int depth) {
        auto &a = fighters[ai];
        auto &d = fighters[1 - ai];
        lastDamage = res.damage;
        if (res.armored)
        {
            comboCounts[ai] = comboTimers[ai] = 0;
            soundEvents.push_back(SoundEvent::Block);
            return;
        }
        if (m)
            a.ForceMeter(a.meter + (res.blocked ? m->meterGainOnBlock : m->meterGainOnHit));
        d.ForceMeter(d.meter + static_cast<int>(std::ceil(res.damage * 0.055f)));
        if (res.blocked)
        {
            comboCounts[ai] = comboTimers[ai] = 0;
            soundEvents.push_back(SoundEvent::Block);
        }
        else
        {
            comboCounts[ai] = depth;
            comboTimers[ai] = 90;
            soundEvents.push_back(h.damage >= 90 ? SoundEvent::HitHeavy : SoundEvent::HitLight);
        }
    };
    auto parry = [&](int di) {
        auto &d = fighters[di];
        auto &a = fighters[1 - di];
        int damage = d.parryRiposteDamage;
        d.parryFrames = d.parryRiposteDamage = 0;
        if (damage <= 0)
            return;
        HitboxDef h{};
        h.damage = damage;
        h.hitStun = 24;
        h.hitStop = 10;
        h.priority = 99;
        h.knockbackX = 5;
        h.knockbackY = -2;
        const auto res = a.ReceiveHit(h, d.facing, false, mode == GameMode::Training, 1,
                                      d.offensiveDebuffFrames > 0 ? d.offensiveDebuffMultiplier : 1);
        d.hitStopFrames = std::max(d.hitStopFrames, 10);
        d.ForceMeter(d.meter + 12);
        lastDamage = res.damage;
        soundEvents.push_back(SoundEvent::HitHeavy);
    };
    auto apply = [&](int ai, const HitboxDef &h) {
        auto &a = fighters[ai];
        auto &d = fighters[1 - ai];
        if (a.grabbed || d.grabbed)
            return;
        const MoveDef *m = a.currentMove;
        if (std::find(a.registeredHitIds.begin(), a.registeredHitIds.end(), h.id) != a.registeredHitIds.end())
            return;
        a.RegisterHit(h);
        if (d.parryFrames > 0 && h.kind != HitKind::Throw && h.level != HitLevel::Low)
        {
            a.moveConnected = 2;
            parry(1 - ai);
            return;
        }
        int depth = d.state == FighterState::HitStun ? comboCounts[ai] + 1 : 1;
        if (h.kind == HitKind::Throw && m && m->isGrab)
        {
            a.moveConnected = 1;
            a.hitStopFrames = std::max(a.hitStopFrames, h.hitStop);
            d.hitStopFrames = std::max(d.hitStopFrames, h.hitStop);
            activeGrab.active = true;
            activeGrab.attackerIndex = ai;
            activeGrab.defenderIndex = 1 - ai;
            activeGrab.move = *m;
            activeGrab.hitbox = h;
            activeGrab.comboDepth = depth;
            d.grabbed = true;
            UpdateActiveGrab();
            soundEvents.push_back(SoundEvent::GrabStart);
            return;
        }
        const auto res = d.ReceiveHit(h, a.facing, false, mode == GameMode::Training, depth,
                                      a.offensiveDebuffFrames > 0 ? a.offensiveDebuffMultiplier : 1);
        a.moveConnected = res.blocked ? 2 : 1;
        a.hitStopFrames = std::max(a.hitStopFrames, h.hitStop);
        outcome(ai, h, m, res, depth);
    };
    Contact one = find(0), two = find(1);
    if (one.found && two.found)
    {
        if (one.h.priority >= two.h.priority)
            apply(0, one.h);
        if (two.h.priority >= one.h.priority)
            apply(1, two.h);
    }
    else if (one.found)
        apply(0, one.h);
    else if (two.found)
        apply(1, two.h);
    for (auto it = projectiles.begin(); it != projectiles.end();)
    {
        auto &d = fighters[1 - it->ownerIndex];
        if (it->ageFrames < it->def.armingFrames || !d.CanReceive(it->def.hitbox))
        {
            ++it;
            continue;
        }
        Rect wr = ToWorldRect(it->def.hitbox.rect, it->x, it->y, it->facing);
        bool hit = false;
        for (const auto &hr : d.CurrentHurtboxesWorld())
            if (Intersects(wr, hr))
            {
                hit = true;
                break;
            }
        if (!hit)
        {
            ++it;
            continue;
        }
        if (d.parryFrames > 0 && it->def.hitbox.kind != HitKind::Throw && it->def.hitbox.level != HitLevel::Low)
            parry(1 - it->ownerIndex);
        else
        {
            auto &a = fighters[it->ownerIndex];
            int depth = d.state == FighterState::HitStun ? comboCounts[it->ownerIndex] + 1 : 1;
            auto res = d.ReceiveHit(it->def.hitbox, it->facing, false, mode == GameMode::Training, depth,
                                    a.offensiveDebuffFrames > 0 ? a.offensiveDebuffMultiplier : 1);
            a.hitStopFrames = std::max(a.hitStopFrames, it->def.hitbox.hitStop);
            outcome(it->ownerIndex, it->def.hitbox, it->sourceMove, res, depth);
        }
        it = projectiles.erase(it);
    }
}

void CombatWorld::UpdateActiveGrab()
{
    if (!activeGrab.active)
        return;
    auto &a = fighters[activeGrab.attackerIndex];
    auto &d = fighters[activeGrab.defenderIndex];
    const auto &m = activeGrab.move;
    if (!a.currentMove || std::strcmp(a.currentMove->id, m.id) != 0 || a.state == FighterState::Knockout)
    {
        activeGrab.active = false;
        d.grabbed = false;
        d.ForceState(d.y < kGroundY ? FighterState::Fall : FighterState::Idle);
        return;
    }
    int f = a.stateFrame;
    if (f >= m.releaseFrame)
    {
        activeGrab.active = false;
        const auto res =
            d.ReceiveHit(activeGrab.hitbox, a.facing, false, mode == GameMode::Training, activeGrab.comboDepth);
        a.hitStopFrames = std::max(a.hitStopFrames, activeGrab.hitbox.hitStop);
        a.ForceMeter(a.meter + m.meterGainOnHit);
        d.ForceMeter(d.meter + static_cast<int>(std::ceil(res.damage * 0.055f)));
        lastDamage = res.damage;
        comboCounts[activeGrab.attackerIndex] = activeGrab.comboDepth;
        comboTimers[activeGrab.attackerIndex] = 90;
        if (!res.knockout)
            d.ReleaseFromGrab(a.facing * m.throwVelocityX, m.throwVelocityY, m.slam);
        else
            d.grabbed = false;
        soundEvents.push_back(SoundEvent::GrabSlam);
        return;
    }
    const auto &poses = m.grabPoses[static_cast<int>(d.def->id)];
    if (!poses.empty())
    {
        const auto &p = poses[std::clamp(f, 0, static_cast<int>(poses.size()) - 1)];
        d.EnterGrabbedState(p.state, a.x + a.facing * p.x, a.y + p.y, a.facing * p.rotation, -a.facing);
        d.victimPoseFrame = p.poseFrame;
        d.victimInFront = m.id == std::string("universalGrab") && f >= 35;
    }
    else
        d.EnterGrabbedState(f < m.holdStartFrame ? FighterState::GrabbedFront : FighterState::GrabbedLifted,
                            a.x + a.facing * 44, a.y, 0, -a.facing);
}

void CombatWorld::ResolvePushboxes()
{
    if (activeGrab.active)
        return;
    FighterRuntime &One = fighters[0];
    FighterRuntime &Two = fighters[1];
    if (One.grabbed || Two.grabbed || One.state == FighterState::Thrown || Two.state == FighterState::Thrown ||
        std::fabs(One.y - Two.y) > 96)
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

    const float TotalWeight = One.def->stats.weight + Two.def->stats.weight;
    const float Dir = One.x <= Two.x ? 1.0f : -1.0f;
    One.x = std::clamp(One.x - Dir * Overlap * Two.def->stats.weight / TotalWeight, static_cast<float>(kStageLeft),
                       static_cast<float>(kStageRight));
    Two.x = std::clamp(Two.x + Dir * Overlap * One.def->stats.weight / TotalWeight, static_cast<float>(kStageLeft),
                       static_cast<float>(kStageRight));
}

void CombatWorld::FinishRound(int winner, bool)
{
    if (phase != CombatPhase::Active)
        return;
    activeGrab = {};
    projectiles.clear();
    roundWinner = winner;
    roundDraw = winner < 0;
    phase = CombatPhase::RoundOver;
    phaseFrame = 0;
    soundEvents.push_back(SoundEvent::RoundEnd);
    if (winner < 0)
    {
        fighters[0].ForceState(fighters[0].health <= 0 || fighters[1].health <= 0 ? FighterState::Knockout
                                                                                  : FighterState::Idle);
        fighters[1].ForceState(fighters[0].health <= 0 || fighters[1].health <= 0 ? FighterState::Knockout
                                                                                  : FighterState::Idle);
        return;
    }
    auto &a = fighters[winner];
    auto &d = fighters[1 - winner];
    ++a.roundWins;
    if (mode != GameMode::Training && a.roundWins >= kRoundsToWin)
    {
        matchWinner = finisherWinner = winner;
        phase = CombatPhase::FinishReady;
        a.ForceState(FighterState::Idle);
        d.ForceState(FighterState::Dizzy);
        soundEvents.push_back(SoundEvent::FinishReady);
        return;
    }
    a.ForceState(FighterState::Victory);
    d.ForceState(FighterState::Knockout);
}
void CombatWorld::CompleteFinish()
{
    projectiles.clear();
    finisherCompleted = phase == CombatPhase::StageFinish;
    fighters[finisherWinner].ForceState(FighterState::Victory);
    fighters[1 - finisherWinner].ForceState(FighterState::Knockout);
    phase = CombatPhase::MatchOver;
    phaseFrame = phaseFrame >= kFinishEndFrame ? kFinishEndFrame : 0;
    soundEvents.push_back(SoundEvent::MatchWin);
}

void CombatWorld::StepFinishReady(const InputFrame &p1, const InputFrame &p2)
{
    if (finisherWinner < 0)
    {
        phase = CombatPhase::MatchOver;
        return;
    }
    auto &a = fighters[finisherWinner];
    auto &d = fighters[1 - finisherWinner];
    InputFrame empty{};
    if (phaseFrame >= kFinishWindowFrames)
    {
        phaseFrame = 0;
        CompleteFinish();
        return;
    }
    if (!(a.currentMove && a.currentMove->lockFacing) && a.grounded)
        a.facing = a.x <= d.x ? 1 : -1;
    a.BeginFrame(finisherWinner == 0 ? p1 : p2, frame);
    d.BeginFrame(empty, frame);
    a.CaptureCollisionPose();
    d.CaptureCollisionPose();
    auto boxes = a.evaluatedHitboxes;
    std::stable_sort(boxes.begin(), boxes.end(), [](auto &a, auto &b) { return a.priority > b.priority; });
    for (const auto &h : boxes)
    {
        if (!d.CanReceive(h))
            continue;
        Rect wr = ToWorldRect(h.rect, a.x, a.y, a.facing);
        bool contact = false;
        for (const auto &hr : d.CurrentHurtboxesWorld())
            if (Intersects(wr, hr))
            {
                contact = true;
                break;
            }
        if (!contact)
            continue;
        if (h.kind == HitKind::Throw)
        {
            a.ForceState(FighterState::Idle);
            d.ForceState(FighterState::Dizzy);
            projectiles.clear();
            phase = CombatPhase::StageFinish;
            phaseFrame = 0;
            StepStageFinish();
            soundEvents.push_back(SoundEvent::GrabStart);
        }
        else
        {
            phaseFrame = 0;
            CompleteFinish();
        }
        return;
    }
    a.ConsumeMoveEvents();
    SpawnProjectiles(finisherWinner);
    UpdateProjectiles();
    int oldHealth = d.health;
    ResolveHits(empty, empty);
    if (d.state != FighterState::Dizzy || d.health < oldHealth)
    {
        projectiles.clear();
        phaseFrame = 0;
        CompleteFinish();
        return;
    }
    ResolvePushboxes();
    a.FinishFrame();
    d.FinishFrame();
}

void CombatWorld::StepStageFinish()
{
    if (finisherWinner < 0)
    {
        phase = CombatPhase::MatchOver;
        return;
    }
    auto &a = fighters[finisherWinner];
    auto &d = fighters[1 - finisherWinner];
    int f = phaseFrame;
    if (stage == StageId::Sitio && f >= 84 && f < 132 && std::fabs(a.x - 327.5f) < 150)
        a.x = std::clamp(a.x - a.facing * (f % 2 == 0 ? 2.0f : 1.0f), float(kStageLeft), float(kStageRight));
    auto p = StageVictim(stage, f, a, d);
    d.EnterGrabbedState(p.state, p.x, p.y, p.rotation, -a.facing);
    d.victimPoseFrame = p.pose;
    d.victimScale = p.scale;
    d.cutBelowY = p.cut;
    d.visible = p.visible;
    d.victimInFront = p.front;
    a.CaptureCollisionPose();
    d.CaptureCollisionPose();
    if (stage == StageId::CaisDaCidade && f == 60)
        soundEvents.push_back(SoundEvent::FinisherMonsterRoar);
    if (stage == StageId::CaisDaCidade && f == 112)
        soundEvents.push_back(SoundEvent::FinisherSplash);
    if (stage == StageId::CozinhaMacabra && f == 108)
        soundEvents.push_back(SoundEvent::FinisherPotDrop);
    if (stage == StageId::Sitio && f == 112)
        soundEvents.push_back(SoundEvent::FinisherTrident);
    if (f >= kFinishEndFrame)
        CompleteFinish();
}

void CombatWorld::StepRoundOver()
{
    InputFrame Empty{};
    fighters[0].BeginFrame(Empty, frame);
    fighters[1].BeginFrame(Empty, frame);
    fighters[0].CaptureCollisionPose();
    fighters[1].CaptureCollisionPose();
    fighters[0].FinishFrame();
    fighters[1].FinishFrame();
    if (phaseFrame >= 150)
    {
        StartNextRound();
    }
}

void CombatWorld::StartNextRound()
{
    if (roundWinner >= 0)
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
    comboCounts = {0, 0};
    comboTimers = {0, 0};
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
    if (finisherWinner >= 0 && (phase == CombatPhase::StageFinish || finisherCompleted))
    {
        const int f = std::min(kFinishEndFrame, phaseFrame);
        Out.visualFinisher = StageVisual(stage, f, fighters[finisherWinner]);
        auto &a = Out.fighters[finisherWinner];
        if (f < 84)
        {
            a.poseState = FighterState::HeavyAttack;
            a.poseAnim = AnimId::UniversalGrab;
            a.poseStateFrame = FinisherAttackerFrame(f);
            a.poseMoveId = "universalGrab";
            a.poseMoveAnimationKey = "universalGrab";
            a.poseMoveConnected = true;
            a.poseMoveTotalFrames = fighters[finisherWinner].def->universalGrab.totalFrames;
            a.poseHitActiveFrom = fighters[finisherWinner].def->universalGrab.visualImpactFrom;
            a.poseHitActiveTo = fighters[finisherWinner].def->universalGrab.visualImpactTo;
        }
        else if (stage == StageId::Sitio && f >= 84 && f < 132 && std::fabs(a.x - 327.5f) < 150)
        {
            a.poseState = FighterState::WalkBackward;
            a.poseAnim = AnimId::WalkBackward;
            a.poseStateFrame = f - 84;
            a.poseMoveId = "";
            a.poseMoveAnimationKey = "";
            a.poseMoveTotalFrames = 0;
        }
    }

    for (const ProjectileRuntime &P : projectiles)
    {
        Out.projectiles.push_back(ProjectileSnapshot{
            P.runtimeId, P.ownerIndex, P.def.id, P.x, P.y, P.facing, ToWorldRect(P.def.hitbox.rect, P.x, P.y, P.facing),
            P.ageFrames >= P.def.armingFrames, P.sourceMove ? P.sourceMove->id : "", P.ageFrames, P.def.armingFrames});
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
    auto MixInt = [&Hash](int64_t Val) {
        for (int i = 0; i < 8; ++i)
        {
            const uint8_t Byte = static_cast<uint8_t>((static_cast<uint64_t>(Val) >> (i * 8)) & 0xFFU);
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
    for (const FighterRuntime &F : fighters)
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
    for (const ProjectileRuntime &P : projectiles)
    {
        MixInt(P.ownerIndex);
        MixInt(static_cast<int64_t>(std::llround(P.x * 1000.0f)));
        MixInt(static_cast<int64_t>(std::llround(P.y * 1000.0f)));
        MixInt(P.life);
    }
    return Hash;
}

// Planejador da referência src/ai/CpuController.ts; RNG local injetável/determinístico.
InputFrame BuildCpuInput(const CombatWorld &world, int index, AiDifficulty difficulty)
{
    if (index < 0 || index > 1)
        return {};
    auto &state = world.cpuControllers[index];
    const WorldSnapshot snapshot = world.Snapshot();
    const auto &definition = *world.fighters[index].def;
    if (state.definition != &definition || state.difficulty != difficulty || snapshot.frame < state.lastFrame)
    {
        state = {};
        state.definition = &definition;
        state.difficulty = difficulty;
        state.randomState = 0x9e3779b9U ^ (uint32_t(definition.id) + 1) * 0x85ebca6bU ^ uint32_t(index) * 0xc2b2ae35U;
    }
    if (state.lastFrame == snapshot.frame)
        return state.cachedInput;
    state.lastFrame = snapshot.frame;
    auto emit = [&](InputFrame current) {
        const auto &previous = state.previousActions;
        current.leftPressed = current.left && !previous.left;
        current.rightPressed = current.right && !previous.right;
        current.upPressed = current.up && !previous.up;
        current.downPressed = current.down && !previous.down;
        current.punchPressed = current.punch && !previous.punch;
        current.kickPressed = current.kick && !previous.kick;
        current.specialPressed = current.special && !previous.special;
        current.blockPressed = current.block && !previous.block;
        state.previousActions = current;
        state.cachedInput = current;
        return current;
    };
    auto resetPlanning = [&]() {
        state.observations.clear();
        state.plan.clear();
        state.decisionCooldown = 0;
    };
    auto random = [&]() {
        uint32_t &value = state.randomState;
        value ^= value << 13;
        value ^= value >> 17;
        value ^= value << 5;
        return (value & 0xffffff) / 16777216.0f;
    };
    auto direction = [](int token, bool towardRight) {
        InputFrame input{};
        if (token == 1 || token == 2 || token == 3)
            input.down = true;
        if (token == 2 || token == 4)
        {
            input.right = towardRight;
            input.left = !towardRight;
        }
        if (token == 3 || token == 5)
        {
            input.left = towardRight;
            input.right = !towardRight;
        }
        if (token == 6)
            input.up = true;
        return input;
    };
    auto queueMove = [&](const MoveDef &move, bool towardRight) {
        for (int token : move.command.directions)
            state.plan.push_back({3, direction(token, towardRight)});
        InputFrame final =
            move.command.directions.empty() ? InputFrame{} : direction(move.command.directions.back(), towardRight);
        final.punch = move.command.buttons & 16;
        final.kick = move.command.buttons & 32;
        final.special = move.command.buttons & 64;
        final.block = move.command.buttons & 128;
        state.plan.push_back({1, final});
        state.plan.push_back({8, {}});
    };
    const auto &own = snapshot.fighters[index];
    const auto &rival = snapshot.fighters[1 - index];
    if (snapshot.phase == CombatPhase::FinishReady && snapshot.finisherWinner == index)
    {
        resetPlanning();
        InputFrame input{};
        if (snapshot.phaseFrame >= 60)
        {
            if (std::fabs(own.x - rival.x) > 74)
            {
                input.right = rival.x > own.x;
                input.left = !input.right;
            }
            else if (snapshot.phaseFrame % 30 < 5)
                input.punch = input.kick = true;
        }
        return emit(input);
    }
    if (snapshot.paused || snapshot.phase != CombatPhase::Active)
    {
        resetPlanning();
        return emit({});
    }
    struct Config
    {
        int reaction, decision;
        float guard, special, jump, mistake;
    };
    const Config config = difficulty == AiDifficulty::Easy   ? Config{18, 12, 0.2f, 0.08f, 0.08f, 0.15f}
                          : difficulty == AiDifficulty::Hard ? Config{6, 4, 0.65f, 0.25f, 0.14f, 0.02f}
                                                             : Config{10, 7, 0.45f, 0.16f, 0.11f, 0.06f};
    state.observations.push_back(snapshot);
    while (state.observations.size() > size_t(config.reaction + 2))
        state.observations.erase(state.observations.begin());
    if (state.observations.size() <= size_t(config.reaction))
        return emit({});
    if (own.state == FighterState::LightAttack || own.state == FighterState::HeavyAttack ||
        own.state == FighterState::KickAttack || own.state == FighterState::SpecialAttack ||
        own.state == FighterState::HitStun || own.state == FighterState::Knockdown ||
        own.state == FighterState::WakeUp || own.state == FighterState::Victory || own.state == FighterState::Knockout)
        return emit({});
    state.decisionCooldown = std::max(0, state.decisionCooldown - 1);
    if (state.plan.empty() && state.decisionCooldown == 0)
    {
        const auto &observed = state.observations.front();
        const auto &self = observed.fighters[index];
        const auto &opponent = observed.fighters[1 - index];
        float distance = std::fabs(self.x - opponent.x);
        bool towardRight = opponent.x > self.x;
        auto approach = [&](float jumpChance) {
            InputFrame input{};
            input.right = towardRight;
            input.left = !towardRight;
            if (random() < jumpChance)
            {
                input.up = true;
                state.plan.push_back({1, input});
                input.up = false;
                state.plan.push_back({16, input});
            }
            else
                state.plan.push_back({10, input});
        };
        auto choose = [&]() {
            if (random() < config.mistake)
            {
                state.plan.push_back({8, {}});
                return;
            }
            if (opponent.attacking && distance < 105 && random() < config.guard)
            {
                InputFrame guard{};
                guard.block = true;
                guard.down = random() < 0.35f;
                state.plan.push_back({12, guard});
                return;
            }
            if (distance > 145)
            {
                approach(config.jump);
                return;
            }
            const bool canSuper = self.meter >= 100;
            if (canSuper || random() < config.special)
            {
                const MoveDef *candidate = nullptr;
                for (const auto &move : definition.canonicalMoves)
                    if (move.isSpecial && move.isSuper == canSuper && (distance < 60 || !move.isGrab) &&
                        (!candidate || move.command.priority > candidate->command.priority))
                        candidate = &move;
                if (candidate)
                {
                    queueMove(*candidate, towardRight);
                    return;
                }
            }
            const float preferredRange = definition.cpuPreferredRange;
            if (distance > preferredRange)
            {
                approach(config.jump * 0.5f);
                return;
            }
            const MoveDef *throwMove = nullptr;
            for (const auto &move : definition.canonicalMoves)
                if (move.isGrab && !move.isSuper)
                {
                    throwMove = &move;
                    break;
                }
            if (distance < 48 && throwMove && random() < 0.28f)
            {
                queueMove(*throwMove, towardRight);
                return;
            }
            if (distance < 36 && random() < 0.22f)
            {
                InputFrame away{};
                away.left = towardRight;
                away.right = !towardRight;
                state.plan.push_back({10, away});
                return;
            }
            const bool heavy = random() < 0.42f, low = random() < 0.25f;
            InputFrame attack{};
            if (low)
                attack.down = true;
            else if (heavy && random() < 0.35f)
            {
                attack.right = towardRight;
                attack.left = !towardRight;
            }
            attack.kick = heavy;
            attack.punch = !heavy;
            state.plan.push_back({1, attack});
            state.plan.push_back({5, {}});
        };
        choose();
        state.decisionCooldown = config.decision;
    }
    InputFrame actions{};
    if (!state.plan.empty())
    {
        actions = state.plan.front().actions;
        if (--state.plan.front().frames <= 0)
            state.plan.erase(state.plan.begin());
    }
    return emit(actions);
}
} // namespace RDA
