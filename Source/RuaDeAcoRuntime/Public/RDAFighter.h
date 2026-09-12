#pragma once

#include "RDATypes.h"

namespace RDA
{
    class Fighter
    {
    public:
        Fighter(const FighterDef& definition, float startX, int facing);

        void BeginFrame(const InputFrame& input, float opponentX);
        void CapturePose();
        void FinishFrame();
        void ApplyHit(const Hitbox& hitbox, int attackerFacing, bool blocked, int comboHits);
        void AddHitStop(int frames);
        void SetMatchState(FighterState state);
        void ResetRound(float startX, int facing);
        void ResetMatch(float startX, int facing);
        void ClampToStage();

        FighterSnapshot MakeSnapshot() const;

        const FighterDef& Def() const { return def; }
        FighterId Id() const { return def.id; }
        float X() const { return x; }
        float Y() const { return y; }
        int Facing() const { return facing; }
        FighterState State() const { return state; }
        int Health() const { return health; }
        int RoundWins() const { return roundWins; }
        void AddRoundWin() { roundWins += 1; }
        bool Grounded() const { return y >= kGroundY - 0.01f; }
        bool Frozen() const { return frozen; }
        bool Attacking() const { return activeMove != nullptr; }
        const MoveDef* CurrentMove() const { return activeMove; }
        const std::vector<Hitbox>& EvaluatedHitboxes() const { return evaluatedHitboxes; }
        const std::vector<Hurtbox>& EvaluatedHurtboxes() const { return evaluatedHurtboxes; }
        const Rect& EvaluatedPushbox() const { return evaluatedPushbox; }
        bool CanRegisterHit() const { return !hitRegistered; }
        void RegisterHit() { hitRegistered = true; }
        bool CanReceiveHit() const;
        bool IsBlocking(HitLevel level) const;
        void SetFacing(int value) { facing = value < 0 ? -1 : 1; }

        float x = 0.0f;
        float y = kGroundY;

    private:
        void Transition(FighterState next);
        void StartMove(const MoveDef& move);
        void ApplyMoveMotion();
        void UpdateVertical();
        const PoseBoxes& CurrentPose() const;
        AnimId ResolveAnim() const;
        const MoveDef* SelectMove() const;

        const FighterDef& def;
        float previousX = 0.0f;
        float previousY = kGroundY;
        float velocityX = 0.0f;
        float velocityY = 0.0f;
        float airDriftX = 0.0f;
        int facing = 1;
        FighterState state = FighterState::Idle;
        int stateFrame = 0;
        int health = 1000;
        int roundWins = 0;
        int hitStopFrames = 0;
        int hitStunFrames = 0;
        int blockStunFrames = 0;
        int invulnerableFrames = 0;
        bool knockdownPending = false;
        bool frozen = false;
        bool hitRegistered = false;
        const MoveDef* activeMove = nullptr;
        InputFrame input{};
        std::vector<Hitbox> evaluatedHitboxes;
        std::vector<Hurtbox> evaluatedHurtboxes;
        Rect evaluatedPushbox{};
    };
}
