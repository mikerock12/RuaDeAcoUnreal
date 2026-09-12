#pragma once

#include <cstdint>
#include <string>
#include <vector>

#if defined(RUADEACORUNTIME_API)
#define RDA_API RUADEACORUNTIME_API
#else
#define RDA_API
#endif

namespace RDA
{
    constexpr int kLogicalWidth = 640;
    constexpr int kLogicalHeight = 360;
    constexpr int kPresentationWidth = 1280;
    constexpr int kPresentationHeight = 720;
    constexpr float kGroundY = 304.0f;
    constexpr float kStageLeft = 36.0f;
    constexpr float kStageRight = 604.0f;
    constexpr float kFixedStepSeconds = 1.0f / 60.0f;
    constexpr int kRoundTimeFrames = 99 * 60;
    constexpr int kRoundsToWin = 2;
    constexpr int kLandingFrames = 6;
    constexpr int kWakeUpFrames = 22;
    constexpr int kIntroFightFrame = 60;
    constexpr int kIntroEndFrame = 105;
    constexpr int kRoundOverFrames = 150;
    constexpr float kP1StartX = 188.0f;
    constexpr float kP2StartX = 452.0f;

    enum class FighterId : uint8_t
    {
        GutoBarba,
        RafaMare
    };

    enum class FighterState : uint8_t
    {
        Idle,
        WalkForward,
        WalkBackward,
        Jump,
        Fall,
        Landing,
        Crouch,
        BlockStanding,
        BlockCrouching,
        LightAttack,
        HeavyAttack,
        KickAttack,
        HitStun,
        Knockdown,
        WakeUp,
        Victory,
        Knockout
    };

    enum class CombatPhase : uint8_t
    {
        Intro,
        Active,
        RoundOver,
        MatchOver
    };

    enum class HitLevel : uint8_t
    {
        High,
        Mid,
        Low,
        Overhead
    };

    enum class HurtRegion : uint8_t
    {
        Head,
        Body,
        Legs
    };

    enum class AnimId : uint8_t
    {
        Idle,
        Walk,
        WalkBackward,
        JumpNeutral,
        JumpForward,
        JumpBackward,
        Fall,
        Landing,
        Crouch,
        StandingLight,
        StandingHeavy,
        ForwardHeavy,
        CrouchLight,
        CrouchHeavy,
        AirLight,
        AirHeavy,
        BlockStanding,
        BlockCrouching,
        Hit,
        Knockdown,
        WakeUp,
        Knockout,
        Victory
    };

    struct Rect
    {
        float x = 0.0f;
        float y = 0.0f;
        float width = 0.0f;
        float height = 0.0f;
    };

    struct Hurtbox
    {
        Rect box;
        HurtRegion region = HurtRegion::Body;
    };

    struct Hitbox
    {
        const char* id = "";
        int activeFrom = 0;
        int activeTo = 0;
        Rect box;
        HitLevel level = HitLevel::Mid;
        int damage = 0;
        int chipDamage = 0;
        int hitStun = 0;
        int blockStun = 0;
        int hitStop = 0;
        int priority = 1;
        float knockbackX = 0.0f;
        float knockbackY = 0.0f;
        bool knockdown = false;
    };

    struct MoveDef
    {
        const char* id = "";
        const char* label = "";
        FighterState state = FighterState::LightAttack;
        AnimId animation = AnimId::StandingLight;
        int totalFrames = 1;
        bool air = false;
        Hitbox hitbox;
    };

    struct PoseBoxes
    {
        Hurtbox head;
        Hurtbox body;
        Hurtbox legs;
        Rect pushbox;
    };

    struct FighterStats
    {
        int maxHealth = 1000;
        float walkSpeed = 2.0f;
        float backwardSpeed = 1.7f;
        float jumpSpeed = 14.0f;
        float jumpForwardSpeed = 2.4f;
        float jumpBackwardSpeed = 2.1f;
        float gravity = 0.84f;
        float weight = 1.0f;
    };

    struct FighterDef
    {
        FighterId id = FighterId::GutoBarba;
        const char* name = "";
        const char* archetype = "";
        FighterStats stats;
        PoseBoxes standing;
        PoseBoxes crouching;
        PoseBoxes airborne;
        MoveDef punch;
        MoveDef kick;
        MoveDef crouchPunch;
        MoveDef crouchKick;
        MoveDef airPunch;
        MoveDef airKick;
        int spriteFrameSize = 256;
        int visualSolePad = 6;
        int idleFps = 6;
        int walkFps = 8;
    };

    struct InputFrame
    {
        bool left = false;
        bool right = false;
        bool up = false;
        bool down = false;
        bool punch = false;
        bool kick = false;
        bool block = false;
        bool special = false;
        bool pause = false;
        bool confirm = false;

        bool upPressed = false;
        bool punchPressed = false;
        bool kickPressed = false;
        bool blockPressed = false;
        bool pausePressed = false;
        bool confirmPressed = false;
        bool debugPressed = false;
        bool restartPressed = false;
    };

    struct FighterSnapshot
    {
        FighterId id = FighterId::GutoBarba;
        const char* name = "";
        float x = 0.0f;
        float y = 0.0f;
        float previousX = 0.0f;
        float previousY = 0.0f;
        float airDriftX = 0.0f;
        int facing = 1;
        FighterState state = FighterState::Idle;
        int stateFrame = 0;
        int health = 0;
        int maxHealth = 1000;
        const char* activeMoveId = nullptr;
        int moveFrame = 0;
        int moveTotalFrames = 1;
        int hitActiveFrom = 0;
        int hitActiveTo = 0;
        AnimId anim = AnimId::Idle;
        bool attacking = false;
        Rect pushboxWorld{};
        std::vector<Rect> hurtboxesWorld;
        std::vector<Rect> hitboxesWorld;
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
        int roundWins[2] = {0, 0};
        FighterSnapshot fighters[2];
        const char* banner = "";
        int lastDamage = 0;
    };

    inline Rect MakeRect(float left, float top, float right, float bottom)
    {
        Rect r;
        r.x = left;
        r.y = top;
        r.width = right - left;
        r.height = bottom - top;
        return r;
    }

    inline Hurtbox MakeHurt(HurtRegion region, float left, float top, float right, float bottom)
    {
        Hurtbox h;
        h.box = MakeRect(left, top, right, bottom);
        h.region = region;
        return h;
    }

    inline PoseBoxes MakePose(
        float hx0, float hy0, float hx1, float hy1,
        float bx0, float by0, float bx1, float by1,
        float lx0, float ly0, float lx1, float ly1,
        float px0, float py0, float px1, float py1)
    {
        PoseBoxes p;
        p.head = MakeHurt(HurtRegion::Head, hx0, hy0, hx1, hy1);
        p.body = MakeHurt(HurtRegion::Body, bx0, by0, bx1, by1);
        p.legs = MakeHurt(HurtRegion::Legs, lx0, ly0, lx1, ly1);
        p.pushbox = MakeRect(px0, py0, px1, py1);
        return p;
    }

    inline bool Intersects(const Rect& a, const Rect& b)
    {
        return a.x < b.x + b.width
            && a.x + a.width > b.x
            && a.y < b.y + b.height
            && a.y + a.height > b.y;
    }

    inline float HorizontalOverlap(const Rect& a, const Rect& b)
    {
        const float left = a.x > b.x ? a.x : b.x;
        const float rightA = a.x + a.width;
        const float rightB = b.x + b.width;
        const float right = rightA < rightB ? rightA : rightB;
        const float overlap = right - left;
        return overlap > 0.0f ? overlap : 0.0f;
    }

    inline Rect ToWorld(const Rect& local, float ownerX, float ownerY, int facing)
    {
        Rect world;
        world.x = facing == 1 ? ownerX + local.x : ownerX - local.x - local.width;
        world.y = ownerY + local.y;
        world.width = local.width;
        world.height = local.height;
        return world;
    }

    inline int CalculateDamage(int baseDamage, int chipDamage, bool blocked, int comboHits)
    {
        const int source = blocked ? chipDamage : baseDamage;
        const float scaling = blocked ? 1.0f : (1.0f - (comboHits > 1 ? (comboHits - 1) * 0.08f : 0.0f));
        const float clamped = scaling < 0.35f ? 0.35f : scaling;
        const int value = static_cast<int>(source * clamped + 0.5f);
        return value < 0 ? 0 : value;
    }

    inline int CalculateHitStun(int baseFrames, bool counterHit, int comboHits)
    {
        const int counterBonus = counterHit ? 3 : 0;
        const int comboReduction = comboHits > 1 ? (comboHits - 1 > 4 ? 4 : comboHits - 1) : 0;
        const int value = baseFrames + counterBonus - comboReduction;
        return value < 0 ? 0 : value;
    }

    inline int MoveAnimFrame(const MoveDef& move, int elapsed, int spriteFrames = 4)
    {
        if (spriteFrames != 4)
        {
            const int idx = elapsed * spriteFrames / (move.totalFrames > 0 ? move.totalFrames : 1);
            return idx < spriteFrames ? idx : spriteFrames - 1;
        }
        if (elapsed >= move.hitbox.activeFrom && elapsed <= move.hitbox.activeTo) return 2;
        if (elapsed > move.hitbox.activeTo) return 3;
        const int prep = move.hitbox.activeFrom / 2;
        return elapsed < (prep < 1 ? 1 : prep) ? 0 : 1;
    }

    RDA_API const FighterDef& GetGutoBarba();
    RDA_API const FighterDef& GetRafaMare();
    RDA_API const FighterDef& GetFighterDef(FighterId id);
}
