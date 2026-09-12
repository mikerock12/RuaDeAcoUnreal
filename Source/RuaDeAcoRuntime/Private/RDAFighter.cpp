#include "RDAFighter.h"

#include <algorithm>
#include <cmath>

namespace RDA
{
    Fighter::Fighter(const FighterDef& definition, float startX, int facingValue)
        : def(definition)
        , facing(facingValue < 0 ? -1 : 1)
    {
        x = startX;
        previousX = startX;
        health = def.stats.maxHealth;
    }

    void Fighter::BeginFrame(const InputFrame& in, float opponentX)
    {
        previousX = x;
        previousY = y;
        input = in;
        invulnerableFrames = std::max(0, invulnerableFrames - 1);

        if (hitStopFrames > 0)
        {
            hitStopFrames -= 1;
            frozen = true;
            return;
        }
        frozen = false;

        if (state == FighterState::Knockout || state == FighterState::Victory)
        {
            return;
        }

        if (state == FighterState::Landing)
        {
            return;
        }

        if (state == FighterState::HitStun)
        {
            x += velocityX;
            velocityX *= 0.88f;
            UpdateVertical();
            return;
        }

        if (state == FighterState::Knockdown || state == FighterState::WakeUp)
        {
            return;
        }

        if (blockStunFrames > 0)
        {
            Transition(input.down ? FighterState::BlockCrouching : FighterState::BlockStanding);
            x += velocityX;
            velocityX *= 0.82f;
            return;
        }

        if (activeMove)
        {
            ApplyMoveMotion();
            return;
        }

        if (y < kGroundY)
        {
            if (input.punchPressed)
            {
                StartMove(def.airPunch);
                ApplyMoveMotion();
                return;
            }
            if (input.kickPressed)
            {
                StartMove(def.airKick);
                ApplyMoveMotion();
                return;
            }
            x += airDriftX;
            UpdateVertical();
            if (y >= kGroundY)
            {
                airDriftX = 0.0f;
                Transition(FighterState::Landing);
            }
            else
            {
                Transition(velocityY < 0.0f ? FighterState::Jump : FighterState::Fall);
            }
            return;
        }

        if (input.block)
        {
            Transition(input.down ? FighterState::BlockCrouching : FighterState::BlockStanding);
            return;
        }

        if (input.upPressed)
        {
            const int horizontal = (input.right ? 1 : 0) - (input.left ? 1 : 0);
            const bool movingForward = horizontal == facing;
            const float jumpDrift = movingForward ? def.stats.jumpForwardSpeed : def.stats.jumpBackwardSpeed;
            airDriftX = static_cast<float>(horizontal) * jumpDrift;
            velocityY = -def.stats.jumpSpeed;
            Transition(FighterState::Jump);
            UpdateVertical();
            return;
        }

        if (const MoveDef* move = SelectMove())
        {
            StartMove(*move);
            return;
        }

        if (input.down)
        {
            Transition(FighterState::Crouch);
            return;
        }

        const int horizontal = (input.right ? 1 : 0) - (input.left ? 1 : 0);
        if (horizontal == 0)
        {
            Transition(FighterState::Idle);
            return;
        }

        const bool movingForward = horizontal == facing;
        const float speed = movingForward ? def.stats.walkSpeed : def.stats.backwardSpeed;
        x += static_cast<float>(horizontal) * speed;
        Transition(movingForward ? FighterState::WalkForward : FighterState::WalkBackward);

        if (opponentX != x)
        {
            facing = opponentX > x ? 1 : -1;
        }
    }

    void Fighter::CapturePose()
    {
        evaluatedHitboxes.clear();
        evaluatedHurtboxes.clear();

        if (activeMove
            && stateFrame >= activeMove->hitbox.activeFrom
            && stateFrame <= activeMove->hitbox.activeTo
            && !(activeMove->air && Grounded()))
        {
            evaluatedHitboxes.push_back(activeMove->hitbox);
        }

        if (CanReceiveHit())
        {
            const PoseBoxes& pose = CurrentPose();
            evaluatedHurtboxes.push_back(pose.head);
            evaluatedHurtboxes.push_back(pose.body);
            evaluatedHurtboxes.push_back(pose.legs);
            evaluatedPushbox = pose.pushbox;
        }
        else
        {
            evaluatedPushbox = CurrentPose().pushbox;
        }
    }

    void Fighter::FinishFrame()
    {
        ClampToStage();
        if (frozen)
        {
            return;
        }

        if (state == FighterState::Knockout || state == FighterState::Victory)
        {
            stateFrame += 1;
            return;
        }

        if (state == FighterState::HitStun)
        {
            hitStunFrames = std::max(0, hitStunFrames - 1);
            stateFrame += 1;
            if (hitStunFrames == 0)
            {
                if (knockdownPending)
                {
                    y = kGroundY;
                    velocityY = 0.0f;
                    Transition(FighterState::Knockdown);
                }
                else
                {
                    Transition(y < kGroundY ? FighterState::Fall : FighterState::Idle);
                }
            }
            return;
        }

        if (blockStunFrames > 0)
        {
            blockStunFrames -= 1;
            stateFrame += 1;
            if (blockStunFrames == 0)
            {
                Transition(FighterState::Idle);
            }
            return;
        }

        if (state == FighterState::Knockdown)
        {
            stateFrame += 1;
            if (stateFrame >= 42)
            {
                invulnerableFrames = 23;
                Transition(FighterState::WakeUp);
            }
            return;
        }

        if (state == FighterState::WakeUp)
        {
            stateFrame += 1;
            if (stateFrame >= kWakeUpFrames)
            {
                Transition(FighterState::Idle);
            }
            return;
        }

        if (state == FighterState::Landing)
        {
            stateFrame += 1;
            if (stateFrame >= kLandingFrames)
            {
                Transition(FighterState::Idle);
            }
            return;
        }

        if (activeMove)
        {
            stateFrame += 1;
            if (stateFrame >= activeMove->totalFrames)
            {
                activeMove = nullptr;
                Transition(y < kGroundY ? FighterState::Fall : FighterState::Idle);
            }
            return;
        }

        stateFrame += 1;
    }

    void Fighter::ApplyHit(const Hitbox& hitbox, int attackerFacing, bool blocked, int comboHits)
    {
        const int damage = CalculateDamage(hitbox.damage, hitbox.chipDamage, blocked, comboHits);
        health = std::max(0, health - damage);
        hitStopFrames = std::max(hitStopFrames, hitbox.hitStop);

        if (blocked)
        {
            blockStunFrames = hitbox.blockStun;
            Transition(input.down ? FighterState::BlockCrouching : FighterState::BlockStanding);
            velocityX = static_cast<float>(attackerFacing) * hitbox.knockbackX * 0.35f;
        }
        else
        {
            const bool counterHit = Attacking();
            activeMove = nullptr;
            hitStunFrames = CalculateHitStun(hitbox.hitStun, counterHit, comboHits);
            knockdownPending = hitbox.knockdown;
            airDriftX = 0.0f;
            velocityX = static_cast<float>(attackerFacing) * hitbox.knockbackX;
            velocityY = hitbox.knockbackY;
            Transition(FighterState::HitStun);
        }

        if (health <= 0)
        {
            Transition(FighterState::Knockout);
        }
    }

    void Fighter::AddHitStop(int frames)
    {
        hitStopFrames = std::max(hitStopFrames, frames);
    }

    void Fighter::SetMatchState(FighterState next)
    {
        activeMove = nullptr;
        velocityX = 0.0f;
        velocityY = 0.0f;
        airDriftX = 0.0f;
        hitStopFrames = 0;
        hitStunFrames = 0;
        blockStunFrames = 0;
        Transition(next);
    }

    void Fighter::ResetRound(float startX, int facingValue)
    {
        x = startX;
        previousX = startX;
        y = kGroundY;
        previousY = kGroundY;
        velocityX = 0.0f;
        velocityY = 0.0f;
        airDriftX = 0.0f;
        facing = facingValue < 0 ? -1 : 1;
        health = def.stats.maxHealth;
        invulnerableFrames = 0;
        hitStopFrames = 0;
        hitStunFrames = 0;
        blockStunFrames = 0;
        knockdownPending = false;
        frozen = false;
        hitRegistered = false;
        activeMove = nullptr;
        Transition(FighterState::Idle);
    }

    void Fighter::ResetMatch(float startX, int facingValue)
    {
        roundWins = 0;
        ResetRound(startX, facingValue);
    }

    void Fighter::ClampToStage()
    {
        x = std::clamp(x, kStageLeft, kStageRight);
    }

    FighterSnapshot Fighter::MakeSnapshot() const
    {
        FighterSnapshot snap;
        snap.id = def.id;
        snap.name = def.name;
        snap.x = x;
        snap.y = y;
        snap.previousX = previousX;
        snap.previousY = previousY;
        snap.airDriftX = airDriftX;
        snap.facing = facing;
        snap.state = state;
        snap.stateFrame = stateFrame;
        snap.health = health;
        snap.maxHealth = def.stats.maxHealth;
        snap.activeMoveId = activeMove ? activeMove->id : nullptr;
        snap.moveFrame = activeMove ? stateFrame : 0;
        snap.moveTotalFrames = activeMove ? activeMove->totalFrames : 1;
        snap.hitActiveFrom = activeMove ? activeMove->hitbox.activeFrom : 0;
        snap.hitActiveTo = activeMove ? activeMove->hitbox.activeTo : 0;
        snap.anim = ResolveAnim();
        snap.attacking = activeMove != nullptr;
        snap.pushboxWorld = ToWorld(evaluatedPushbox, x, y, facing);
        for (const Hurtbox& hurt : evaluatedHurtboxes)
        {
            snap.hurtboxesWorld.push_back(ToWorld(hurt.box, x, y, facing));
        }
        for (const Hitbox& hit : evaluatedHitboxes)
        {
            snap.hitboxesWorld.push_back(ToWorld(hit.box, x, y, facing));
        }
        return snap;
    }

    bool Fighter::CanReceiveHit() const
    {
        return invulnerableFrames <= 0
            && state != FighterState::Knockdown
            && state != FighterState::Knockout
            && state != FighterState::Victory;
    }

    bool Fighter::IsBlocking(HitLevel level) const
    {
        const bool guarding = state == FighterState::BlockStanding
            || state == FighterState::BlockCrouching
            || blockStunFrames > 0;
        if (!guarding || activeMove != nullptr)
        {
            return false;
        }
        const bool crouching = state == FighterState::BlockCrouching;
        if (level == HitLevel::Overhead) return !crouching;
        if (level == HitLevel::Low) return crouching;
        return true;
    }

    void Fighter::Transition(FighterState next)
    {
        if (state != next)
        {
            state = next;
            stateFrame = 0;
        }
    }

    void Fighter::StartMove(const MoveDef& move)
    {
        activeMove = &move;
        hitRegistered = false;
        Transition(move.state);
    }

    void Fighter::ApplyMoveMotion()
    {
        if (activeMove && activeMove->air && y < kGroundY)
        {
            x += airDriftX;
        }
        if (y < kGroundY || velocityY != 0.0f)
        {
            UpdateVertical();
        }
        if (activeMove && activeMove->air && y >= kGroundY && velocityY == 0.0f)
        {
            activeMove = nullptr;
            airDriftX = 0.0f;
            Transition(FighterState::Landing);
        }
    }

    void Fighter::UpdateVertical()
    {
        y += velocityY;
        velocityY += def.stats.gravity;
        if (y >= kGroundY)
        {
            y = kGroundY;
            velocityY = 0.0f;
        }
    }

    const PoseBoxes& Fighter::CurrentPose() const
    {
        if (!Grounded() || state == FighterState::Jump || state == FighterState::Fall)
        {
            return def.airborne;
        }
        if (state == FighterState::Crouch
            || state == FighterState::BlockCrouching
            || state == FighterState::Landing
            || (activeMove && (activeMove->animation == AnimId::CrouchLight || activeMove->animation == AnimId::CrouchHeavy)))
        {
            return def.crouching;
        }
        return def.standing;
    }

    AnimId Fighter::ResolveAnim() const
    {
        if (activeMove) return activeMove->animation;
        switch (state)
        {
        case FighterState::WalkForward: return AnimId::Walk;
        case FighterState::WalkBackward: return AnimId::WalkBackward;
        case FighterState::Jump:
        {
            const float relative = airDriftX * static_cast<float>(facing);
            if (relative > 0.01f) return AnimId::JumpForward;
            if (relative < -0.01f) return AnimId::JumpBackward;
            return AnimId::JumpNeutral;
        }
        case FighterState::Fall: return AnimId::Fall;
        case FighterState::Landing: return AnimId::Landing;
        case FighterState::Crouch: return AnimId::Crouch;
        case FighterState::BlockStanding: return AnimId::BlockStanding;
        case FighterState::BlockCrouching: return AnimId::BlockCrouching;
        case FighterState::HitStun: return AnimId::Hit;
        case FighterState::Knockdown: return AnimId::Knockdown;
        case FighterState::WakeUp: return AnimId::WakeUp;
        case FighterState::Knockout: return AnimId::Knockout;
        case FighterState::Victory: return AnimId::Victory;
        default: return AnimId::Idle;
        }
    }

    const MoveDef* Fighter::SelectMove() const
    {
        if (input.punchPressed)
        {
            return input.down ? &def.crouchPunch : &def.punch;
        }
        if (input.kickPressed)
        {
            return input.down ? &def.crouchKick : &def.kick;
        }
        return nullptr;
    }
}
