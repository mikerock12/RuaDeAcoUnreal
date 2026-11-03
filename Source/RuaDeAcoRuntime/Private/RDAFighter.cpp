#include "RDAFighter.h"
#include <algorithm>
#include <cmath>
#include <cstring>

namespace RDA
{
void FighterRuntime::Transition(FighterState s)
{
    if (state != s)
    {
        state = s;
        stateFrame = 0;
    }
}
void FighterRuntime::UpdateVertical()
{
    y += velocityY;
    velocityY += def->stats.gravity;
    if (y >= kGroundY)
    {
        y = kGroundY;
        velocityY = 0;
    }
    grounded = y >= kGroundY;
}
static int HeldMask(const InputFrame &i)
{
    return (i.left ? 1 : 0) | (i.right ? 2 : 0) | (i.up ? 4 : 0) | (i.down ? 8 : 0) | (i.punch ? 16 : 0) |
           (i.kick ? 32 : 0) | (i.special ? 64 : 0) | (i.block ? 128 : 0);
}
static int PressedMask(const InputFrame &i)
{
    return (i.leftPressed ? 1 : 0) | (i.rightPressed ? 2 : 0) | (i.upPressed ? 4 : 0) | (i.downPressed ? 8 : 0) |
           (i.punchPressed ? 16 : 0) | (i.kickPressed ? 32 : 0) | (i.specialPressed ? 64 : 0) |
           (i.blockPressed ? 128 : 0);
}
bool FighterRuntime::MatchCommand(const CommandDef &c, int wf) const
{
    int hm = HeldMask(frameInput), pm = PressedMask(frameInput);
    if (!c.buttons)
        return false;
    if (c.simultaneous && ((hm & c.buttons) != c.buttons || !(pm & c.buttons)))
        return false;
    int activation = -1;
    for (int b = 4; b < 8; ++b)
    {
        int mask = 1 << b;
        if (!(c.buttons & mask))
            continue;
        int recent = -1;
        for (auto it = buttonSamples.rbegin(); it != buttonSamples.rend(); ++it)
            if (it->first == mask && wf - it->second <= c.bufferFrames)
            {
                recent = it->second;
                break;
            }
        if (pm & mask)
            recent = wf;
        if (!(hm & mask) && recent < 0)
            return false;
        if (recent >= 0)
            activation = std::max(activation, recent);
    }
    if (activation < 0 || wf - activation > c.bufferFrames)
        return false;
    int si = static_cast<int>(directionSamples.size()) - 1, previous = activation;
    for (auto it = c.directions.rbegin(); it != c.directions.rend(); ++it)
    {
        int found = -1;
        while (si >= 0)
        {
            auto s = directionSamples[si--];
            if (s.second > activation)
                continue;
            if (s.first == *it || (c.directions.size() == 1 && *it == 1 && (s.first == 2 || s.first == 3)))
            {
                found = s.second;
                break;
            }
        }
        if (found < 0 || previous - found > c.maxGapFrames)
            return false;
        previous = found;
    }
    return true;
}
void FighterRuntime::ApplyMoveMotion(const MoveDef &m)
{
    for (const auto &a : m.movement)
        if (stateFrame >= a.from && stateFrame <= a.to)
        {
            x += a.x * facing * (passiveFrames > 0 ? def->passiveSpeed : 1);
            if (a.setY)
                velocityY = a.y;
            break;
        }
    if (m.air && y < kGroundY)
        x += airDriftX * (passiveFrames > 0 ? def->passiveSpeed : 1);
    if (y < kGroundY || velocityY != 0)
        UpdateVertical();
    if (m.air && y >= kGroundY && velocityY == 0)
    {
        currentMove = nullptr;
        airDriftX = 0;
        Transition(FighterState::Landing);
    }
}
void FighterRuntime::ConsumeMoveEvents()
{
    if (!currentMove || frozen ||
        std::find(emittedEvents.begin(), emittedEvents.end(), stateFrame) != emittedEvents.end())
        return;
    for (const auto &e : currentMove->events)
    {
        if (e.frame != stateFrame)
            continue;
        if (e.type == 0)
            pendingProjectileIndex = e.value;
        if (e.type == 1)
            armorHits = e.value;
        if (e.type == 2)
            armorHits = 0;
        if (e.type == 3)
            passiveFrames = e.duration;
        if (e.type == 4)
        {
            parryFrames = e.duration;
            parryRiposteDamage = e.value;
        }
        if (e.type == 5 && damageReductionFrames == 0 && damageReductionCooldownFrames == 0)
        {
            damageReductionFrames = e.duration;
            damageReductionMultiplier = e.multiplier;
            damageReductionCooldownFrames = std::max(e.duration, e.cooldown);
        }
    }
    emittedEvents.push_back(stateFrame);
}
int FighterRuntime::PoseKind() const
{
    if (state == FighterState::Landing)
        return 3;
    if (y < kGroundY)
        return 2;
    if (state == FighterState::Crouch || state == FighterState::BlockCrouching || (currentMove && currentMove->crouch))
        return 1;
    return 0;
}
std::vector<HitboxDef> FighterRuntime::ActiveHitboxes() const
{
    std::vector<HitboxDef> out;
    if (!currentMove || (currentMove->air && y >= kGroundY))
        return out;
    for (const auto &h : currentMove->hitboxes)
        if (stateFrame >= h.activeFrom && stateFrame <= h.activeTo)
            out.push_back(h);
    return out;
}
void FighterRuntime::CaptureCollisionPose()
{
    evaluatedState = state;
    evaluatedAnim = CurrentAnim();
    evaluatedStateFrame = stateFrame;
    evaluatedMove = currentMove;
    evaluatedMoveConnected = moveConnected;
    evaluatedHitboxes = ActiveHitboxes();
    evaluatedHurtboxes.clear();
    if (invulnerableFrames == 0 && state != FighterState::Knockdown && state != FighterState::Knockout &&
        state != FighterState::Thrown && !grabbed && state != FighterState::GrabbedFront &&
        state != FighterState::GrabbedLifted && state != FighterState::Frozen)
        evaluatedHurtboxes = def->poseHurtboxes[PoseKind()];
    evaluatedPushbox = def->posePushboxes[PoseKind()];
    poseCaptured = true;
}
void FighterRuntime::RegisterHit(const HitboxDef &h)
{
    registeredHitIds.push_back(h.id);
    moveHasRegisteredContact = true;
}
bool FighterRuntime::CanReceive(const HitboxDef &h) const
{
    if (invulnerableFrames > 0 || state == FighterState::Knockdown || state == FighterState::Knockout ||
        state == FighterState::Thrown || grabbed || state == FighterState::GrabbedFront ||
        state == FighterState::GrabbedLifted || state == FighterState::Frozen)
        return false;
    int p = PoseKind();
    if (h.crouchAvoidable && p == 1)
        return false;
    if (h.airAvoidable && (p == 2 || p == 3))
        return false;
    return true;
}
FighterRuntime::FighterRuntime(const FighterDef &InDef, float StartX, int StartFacing) : def(&InDef)
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
    victimPoseFrame = -1;
    victimScale = 1;
    cutBelowY = -1;
    victimInFront = false;
    pendingProjectileIndex = -1;
    lastPunchFrame = -100;
    lastKickFrame = -100;
    lastDownFrame = -100;
    lastForwardFrame = -100;
    lastBackFrame = -100;
    invulnerableFrames = passiveFrames = damageReductionCooldownFrames = offensiveDebuffFrames = damageTowardPassive =
        blockStunFrames = 0;
    offensiveDebuffMultiplier = 1;
    moveConnected = 0;
    frozen = grabbed = knockdownPending = jumpQueued = poseCaptured = false;
    directionSamples.clear();
    buttonSamples.clear();
    registeredHitIds.clear();
    emittedEvents.clear();
    frameInput = {};
}

void FighterRuntime::ForceState(FighterState NewState)
{
    currentMove = nullptr;
    moveHasRegisteredContact = false;
    velocityX = 0.0f;
    y = kGroundY;
    velocityY = airDriftX = 0;
    grounded = true;
    grabbed = frozen = jumpQueued = knockdownPending = false;
    hitStopFrames = stunFrames = blockStunFrames = invulnerableFrames = armorHits = 0;
    victimRotation = 0;
    victimPoseFrame = -1;
    moveConnected = 0;
    directionSamples.clear();
    buttonSamples.clear();
    Transition(NewState);
}

void FighterRuntime::ForceMeter(int NewMeter)
{
    meter = std::clamp(NewMeter, 0, kMaxMeter);
}

void FighterRuntime::EnterGrabbedState(FighterState VictimState, float AnchorX, float AnchorY, float RotationRad,
                                       int VictimFacing)
{
    grabbed = true;
    armorHits = blockStunFrames = stunFrames = 0;
    knockdownPending = false;
    currentMove = nullptr;
    moveHasRegisteredContact = false;
    velocityX = 0.0f;
    velocityY = 0.0f;
    airDriftX = 0.0f;
    x = AnchorX;
    y = AnchorY;
    grounded = y >= static_cast<float>(kGroundY) - 0.5f;
    facing = VictimFacing >= 0 ? 1 : -1;
    victimRotation = RotationRad;
    Transition(VictimState);
}

void FighterRuntime::ReleaseFromGrab(float ThrowVelX, float ThrowVelY, bool bSlamToKnockdown)
{
    grabbed = false;
    victimRotation = 0.0f;
    currentMove = nullptr;
    victimPoseFrame = -1;
    if (bSlamToKnockdown)
    {
        y = static_cast<float>(kGroundY);
        grounded = true;
        velocityX = ThrowVelX;
        velocityY = 0.0f;
        state = FighterState::Knockdown;
        stateFrame = 0;
        stunFrames = 0;
        knockdownPending = false;
        victimPoseFrame = 3;
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

bool FighterRuntime::ConsumePendingProjectile(ProjectileDef &OutDef)
{
    if (pendingProjectileIndex < 0 || pendingProjectileIndex >= def->projectileCount)
    {
        return false;
    }
    OutDef = def->projectiles[static_cast<size_t>(pendingProjectileIndex)];
    pendingProjectileIndex = -1;
    return true;
}

void FighterRuntime::BeginFrame(const InputFrame &in, int wf)
{
    frameInput = in;
    poseCaptured = false;
    int token = 0;
    bool fw = facing > 0 ? in.right : in.left, bk = facing > 0 ? in.left : in.right;
    if (in.up)
        token = 6;
    else if (in.down && fw)
        token = 2;
    else if (in.down && bk)
        token = 3;
    else if (in.down)
        token = 1;
    else if (fw)
        token = 4;
    else if (bk)
        token = 5;
    directionSamples.push_back({token, wf});
    int pm = PressedMask(in);
    for (int b = 4; b < 8; ++b)
        if (pm & (1 << b))
            buttonSamples.push_back({1 << b, wf});
    std::erase_if(directionSamples, [wf](auto s) { return s.second < wf - 45; });
    std::erase_if(buttonSamples, [wf](auto s) { return s.second < wf - 45; });
    invulnerableFrames = std::max(0, invulnerableFrames - 1);
    passiveFrames = std::max(0, passiveFrames - 1);
    if (damageReductionFrames > 0 && --damageReductionFrames == 0)
        damageReductionMultiplier = 1;
    damageReductionCooldownFrames = std::max(0, damageReductionCooldownFrames - 1);
    if (parryFrames > 0 && --parryFrames == 0)
        parryRiposteDamage = 0;
    if (offensiveDebuffFrames > 0 && --offensiveDebuffFrames == 0)
        offensiveDebuffMultiplier = 1;
    freezeFrames = std::max(0, freezeFrames - 1);
    if (hitStopFrames > 0)
    {
        --hitStopFrames;
        frozen = true;
        return;
    }
    frozen = false;
    if (state == FighterState::Knockout || state == FighterState::Victory || state == FighterState::Dizzy)
        return;
    if (grabbed || state == FighterState::GrabbedFront || state == FighterState::GrabbedLifted ||
        state == FighterState::Frozen)
        return;
    if (state == FighterState::Landing)
    {
        if (in.upPressed || in.up)
            jumpQueued = true;
        return;
    }
    if (state == FighterState::HitStun || state == FighterState::Thrown)
    {
        x += velocityX;
        velocityX *= 0.88f;
        UpdateVertical();
        return;
    }
    if (state == FighterState::Knockdown || state == FighterState::WakeUp)
        return;
    if (blockStunFrames > 0)
    {
        Transition(in.down ? FighterState::BlockCrouching : FighterState::BlockStanding);
        x += velocityX;
        velocityX *= 0.82f;
        return;
    }
    if (currentMove && !currentMove->air && stateFrame <= 3 && moveConnected == 0 &&
        currentMove->command.buttons != 48 && in.punch && in.kick && (in.punchPressed || in.kickPressed))
    {
        StartMove(def->universalGrab);
        return;
    }
    if (currentMove)
    {
        // Cancel commands are selected without mutating the active move until eligible.
        for (const auto &c : currentMove->cancels)
            if (stateFrame >= c.from && stateFrame <= c.to && (c.condition == 0 || c.condition == moveConnected))
            {
                const MoveDef *requested = nullptr;
                for (const auto &m : def->canonicalMoves)
                    if (!m.air && MatchCommand(m.command, wf) &&
                        (!requested || m.command.priority > requested->command.priority))
                        requested = &m;
                if (requested && meter >= requested->meterCost &&
                    std::find(c.into.begin(), c.into.end(), requested->id) != c.into.end())
                {
                    StartMove(*requested);
                    return;
                }
            }
        ApplyMoveMotion(*currentMove);
        return;
    }
    if (y < kGroundY)
    {
        int h = (in.right ? 1 : 0) - (in.left ? 1 : 0);
        if (h)
        {
            float target = h * (h == facing ? def->stats.jumpForwardSpeed : def->stats.jumpBackwardSpeed);
            airDriftX = airDriftX * 0.72f + target * 0.28f;
        }
        if (TryStartMove(in, wf))
        {
            ApplyMoveMotion(*currentMove);
            return;
        }
        x += airDriftX * (passiveFrames > 0 ? def->passiveSpeed : 1);
        UpdateVertical();
        if (y >= kGroundY)
        {
            airDriftX = 0;
            Transition(FighterState::Landing);
        }
        else
            Transition(velocityY < 0 ? FighterState::Jump : FighterState::Fall);
        return;
    }
    if (in.upPressed || jumpQueued)
    {
        jumpQueued = false;
        int h = (in.right ? 1 : 0) - (in.left ? 1 : 0);
        airDriftX = h * (h == facing ? def->stats.jumpForwardSpeed : def->stats.jumpBackwardSpeed);
        velocityY = -def->stats.jumpSpeed;
        Transition(FighterState::Jump);
        UpdateVertical();
        return;
    }
    if (in.block)
    {
        Transition(in.down ? FighterState::BlockCrouching : FighterState::BlockStanding);
        return;
    }
    if (TryStartMove(in, wf))
        return;
    if (in.down)
    {
        Transition(FighterState::Crouch);
        return;
    }
    int h = (in.right ? 1 : 0) - (in.left ? 1 : 0);
    if (!h)
    {
        Transition(FighterState::Idle);
        return;
    }
    x += h * (h == facing ? def->stats.walkSpeed : def->stats.backwardSpeed) *
         (passiveFrames > 0 ? def->passiveSpeed : 1);
    Transition(h == facing ? FighterState::WalkForward : FighterState::WalkBackward);
}

bool FighterRuntime::TryStartMove(const InputFrame &in, int wf)
{
    if (in.grabPressed && y >= kGroundY)
    {
        StartMove(def->universalGrab);
        return true;
    }
    if (in.superPressed && y >= kGroundY && meter >= def->superMove.meterCost)
    {
        StartMove(def->superMove);
        return true;
    }
    int trajectory = airDriftX == 0 ? 0 : (airDriftX * facing > 0 ? 1 : 2);
    const MoveDef *best = nullptr;
    for (const auto &m : def->canonicalMoves)
    {
        if (m.air != (y < kGroundY) || (m.trajectory >= 0 && m.trajectory != trajectory) ||
            !MatchCommand(m.command, wf))
            continue;
        if (!best || m.command.priority > best->command.priority)
            best = &m;
    }
    if (!best || meter < best->meterCost)
        return false;
    StartMove(*best);
    return true;
}

void FighterRuntime::StartMove(const MoveDef &m)
{
    currentMove = &m;
    armorHits = 0;
    meter = std::clamp(meter - m.meterCost, 0, kMaxMeter);
    registeredHitIds.clear();
    emittedEvents.clear();
    moveConnected = 0;
    moveHasRegisteredContact = false;
    Transition(m.state);
}

void FighterRuntime::FinishFrame()
{
    x = std::clamp(x, float(kStageLeft), float(kStageRight));
    grounded = y >= kGroundY;
    if (frozen)
        return;
    if (state == FighterState::Knockout || state == FighterState::Victory || state == FighterState::Dizzy ||
        state == FighterState::GrabbedFront || state == FighterState::GrabbedLifted || state == FighterState::Frozen)
    {
        ++stateFrame;
        return;
    }
    if (state == FighterState::HitStun || state == FighterState::Thrown)
    {
        stunFrames = std::max(0, stunFrames - 1);
        ++stateFrame;
        if (stunFrames == 0)
        {
            if (knockdownPending)
            {
                y = kGroundY;
                velocityY = 0;
                grounded = true;
                Transition(FighterState::Knockdown);
            }
            else
                Transition(y < kGroundY ? FighterState::Fall : FighterState::Idle);
        }
        return;
    }
    if (blockStunFrames > 0)
    {
        --blockStunFrames;
        ++stateFrame;
        if (blockStunFrames == 0)
            Transition(FighterState::Idle);
        return;
    }
    if (state == FighterState::Knockdown)
    {
        if (++stateFrame >= 42)
        {
            invulnerableFrames = 23;
            victimPoseFrame = -1;
            Transition(FighterState::WakeUp);
        }
        return;
    }
    if (state == FighterState::WakeUp)
    {
        if (++stateFrame >= kWakeUpFrames)
            Transition(FighterState::Idle);
        return;
    }
    if (state == FighterState::Landing)
    {
        if (++stateFrame >= kLandingFrames)
            Transition(FighterState::Idle);
        return;
    }
    if (currentMove)
    {
        const MoveDef &m = *currentMove;
        ++stateFrame;
        int activeTo = -1;
        for (const auto &h : m.hitboxes)
            activeTo = std::max(activeTo, h.activeTo);
        if (m.isGrab && m.whiffRecoveryFrame >= 0 && moveConnected == 0 && stateFrame > activeTo &&
            stateFrame < m.whiffRecoveryFrame)
            stateFrame = m.whiffRecoveryFrame;
        if (stateFrame >= m.totalFrames)
        {
            currentMove = nullptr;
            armorHits = 0;
            Transition(y < kGroundY ? FighterState::Fall : FighterState::Idle);
        }
        return;
    }
    ++stateFrame;
}

bool FighterRuntime::HasActiveHitbox(HitboxDef &out, Rect &wr) const
{
    auto hs = poseCaptured ? evaluatedHitboxes : ActiveHitboxes();
    std::stable_sort(hs.begin(), hs.end(), [](auto &a, auto &b) { return a.priority > b.priority; });
    for (const auto &h : hs)
        if (std::find(registeredHitIds.begin(), registeredHitIds.end(), h.id) == registeredHitIds.end())
        {
            out = h;
            wr = ToWorldRect(h.rect, x, y, facing);
            return true;
        }
    return false;
}
std::vector<Rect> FighterRuntime::CurrentHurtboxesWorld() const
{
    std::vector<Rect> out;
    const auto &boxes = poseCaptured ? evaluatedHurtboxes : def->poseHurtboxes[PoseKind()];
    for (const auto &b : boxes)
        out.push_back(ToWorldRect(b, x, y, facing));
    return out;
}
Rect FighterRuntime::PushboxWorld() const
{
    return ToWorldRect(poseCaptured ? evaluatedPushbox : def->posePushboxes[PoseKind()], x, y, facing);
}

AppliedHit FighterRuntime::ReceiveHit(const HitboxDef &h, int af, bool /*tech*/, bool infinite, int depth,
                                      float attackMultiplier)
{
    if (h.kind == HitKind::Throw && !grabbed)
        return AppliedHit{0, false, false, true};
    if (parryFrames > 0 && h.kind != HitKind::Throw && h.level != HitLevel::Low)
        return AppliedHit{0, false, false, false, false, true};
    if (armorHits > 0 && h.kind != HitKind::Throw)
    {
        --armorHits;
        hitStopFrames = std::max(hitStopFrames, h.hitStop);
        AppliedHit r;
        r.armored = true;
        return r;
    }
    bool guarding =
        state == FighterState::BlockStanding || state == FighterState::BlockCrouching || blockStunFrames > 0;
    bool low = state == FighterState::BlockCrouching;
    bool block = guarding && !currentMove && h.kind != HitKind::Throw && !(h.level == HitLevel::Low && !low) &&
                 !(h.level == HitLevel::Overhead && low);
    float scale = block ? 1 : std::max(0.35f, 1 - std::max(0, depth - 1) * 0.08f);
    int damage =
        std::max(0, int(std::floor((block ? h.chipDamage : h.damage) * scale *
                                       (damageReductionFrames > 0 ? damageReductionMultiplier : 1) * attackMultiplier +
                                   0.5f)));
    if (!infinite)
        health = std::max(0, health - damage);
    if (!block && damage > 0 && def->passiveThreshold > 0)
    {
        damageTowardPassive += damage;
        if (damageTowardPassive >= def->passiveThreshold)
        {
            damageTowardPassive = 0;
            passiveFrames = def->passiveDuration;
        }
    }
    hitStopFrames = std::max(hitStopFrames, h.hitStop);
    if (block)
    {
        blockStunFrames = h.blockStun;
        Transition(frameInput.down ? FighterState::BlockCrouching : FighterState::BlockStanding);
        velocityX = af * h.knockbackX * 0.35f;
    }
    else
    {
        bool counter = currentMove != nullptr;
        grabbed = false;
        victimRotation = 0;
        victimPoseFrame = -1;
        currentMove = nullptr;
        armorHits = 0;
        stunFrames = std::max(0, h.hitStun + (counter ? 3 : 0) - std::min(4, std::max(0, depth - 1)));
        knockdownPending = h.knockdown;
        freezeFrames = std::max(freezeFrames, h.freezeFrames);
        airDriftX = 0;
        velocityX = af * h.knockbackX;
        velocityY = h.knockbackY;
        Transition(FighterState::HitStun);
        if (damage > 0 && h.offensiveDebuffFrames > 0)
        {
            offensiveDebuffFrames = h.offensiveDebuffFrames;
            offensiveDebuffMultiplier = h.offensiveDebuffMultiplier;
        }
    }
    if (health <= 0)
        Transition(FighterState::Knockout);
    return AppliedHit{damage, block, health <= 0};
}
void FighterRuntime::NotifyAttackConnected(bool blocked, const MoveDef &m)
{
    moveConnected = blocked ? 2 : 1;
    hitStopFrames = std::max(hitStopFrames, m.hitbox.hitStop);
    meter = std::clamp(meter + (blocked ? m.meterGainOnBlock : m.meterGainOnHit), 0, kMaxMeter);
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
    case FighterState::WalkForward:
        return AnimId::Walk;
    case FighterState::WalkBackward:
        return AnimId::WalkBackward;
    case FighterState::Jump:
        if (airDriftX * static_cast<float>(facing) > 0.1f)
            return AnimId::JumpForward;
        if (airDriftX * static_cast<float>(facing) < -0.1f)
            return AnimId::JumpBackward;
        return AnimId::JumpNeutral;
    case FighterState::Fall:
        return AnimId::Fall;
    case FighterState::Landing:
        return AnimId::Landing;
    case FighterState::Crouch:
        return AnimId::Crouch;
    case FighterState::BlockStanding:
        return AnimId::BlockStanding;
    case FighterState::BlockCrouching:
        return AnimId::BlockCrouching;
    case FighterState::HitStun:
        return AnimId::Hit;
    case FighterState::Knockdown:
        return AnimId::Knockdown;
    case FighterState::WakeUp:
        return AnimId::WakeUp;
    case FighterState::GrabbedFront:
        return AnimId::GrabbedFront;
    case FighterState::GrabbedLifted:
        return AnimId::GrabbedLifted;
    case FighterState::Thrown:
        return AnimId::Thrown;
    case FighterState::Frozen:
        return AnimId::Frozen;
    case FighterState::Victory:
        return AnimId::Victory;
    case FighterState::Knockout:
        return AnimId::Knockout;
    default:
        return AnimId::Idle;
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
    Out.grounded = y >= kGroundY;
    Out.attacking = currentMove != nullptr;
    Out.moveConnected = moveConnected != 0;
    const MoveDef *visualMove = poseCaptured ? evaluatedMove : currentMove;
    Out.poseState = poseCaptured ? evaluatedState : state;
    Out.poseAnim = poseCaptured ? evaluatedAnim : CurrentAnim();
    Out.poseStateFrame = poseCaptured ? evaluatedStateFrame : stateFrame;
    Out.poseMoveId = visualMove ? visualMove->id : "";
    Out.poseMoveAnimationKey = visualMove ? visualMove->animationKey : "";
    Out.poseMoveConnected = (poseCaptured ? evaluatedMoveConnected : moveConnected) != 0;
    Out.poseMoveTotalFrames = visualMove ? visualMove->totalFrames : 0;
    Out.poseHitActiveFrom = visualMove ? visualMove->visualImpactFrom : 0;
    Out.poseHitActiveTo = visualMove ? visualMove->visualImpactTo : 0;
    Out.moveId = currentMove ? currentMove->id : "";
    Out.moveLabel = currentMove ? currentMove->label : "";
    Out.moveAnimationKey = currentMove ? currentMove->animationKey : "";
    Out.moveFrame = stateFrame;
    Out.moveTotalFrames = currentMove ? currentMove->totalFrames : 0;
    Out.hitActiveFrom = currentMove ? currentMove->visualImpactFrom : 0;
    Out.hitActiveTo = currentMove ? currentMove->visualImpactTo : 0;
    Out.armorHits = armorHits;
    Out.parryFrames = parryFrames;
    Out.damageReductionFrames = damageReductionFrames;
    Out.freezeFrames = freezeFrames;
    Out.victimRotation = victimRotation;
    Out.visible = visible;
    Out.victimPoseFrame = victimPoseFrame;
    Out.victimScale = victimScale;
    Out.cutBelowY = cutBelowY;
    Out.victimInFront = victimInFront;
    Out.pushboxWorld = PushboxWorld();
    Out.hurtboxesWorld = CurrentHurtboxesWorld();

    for (const auto &hit : poseCaptured ? evaluatedHitboxes : ActiveHitboxes())
        Out.hitboxesWorld.push_back(ToWorldRect(hit.rect, x, y, facing));
    return Out;
}

int MoveAnimFrame(const MoveDef &Move, int StateFrame, int SheetFrames)
{
    if (SheetFrames <= 1)
    {
        return 0;
    }
    const int elapsed = std::clamp(StateFrame, 0, std::max(0, Move.totalFrames - 1));
    if (SheetFrames != 4)
        return std::min(SheetFrames - 1, elapsed * SheetFrames / std::max(1, Move.totalFrames));
    const int from = Move.visualImpactFrom >= 0 ? Move.visualImpactFrom : Move.hitbox.activeFrom;
    const int to = Move.visualImpactTo >= 0 ? Move.visualImpactTo : Move.hitbox.activeTo;
    if (elapsed >= from && elapsed <= to)
        return 2;
    if (elapsed > to)
        return 3;
    return elapsed < std::max(1, from / 2) ? 0 : 1;
}
} // namespace RDA
