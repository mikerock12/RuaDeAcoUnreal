#include "RDATypes.h"

namespace RDA
{
    static Hitbox MakeHit(
        const char* id,
        int from,
        int to,
        float left,
        float top,
        float right,
        float bottom,
        HitLevel level,
        int damage,
        int chip,
        int hitStun,
        int blockStun,
        int hitStop,
        int priority,
        float kbX,
        float kbY,
        bool knockdown = false)
    {
        Hitbox h;
        h.id = id;
        h.activeFrom = from;
        h.activeTo = to;
        h.box = MakeRect(left, top, right, bottom);
        h.level = level;
        h.damage = damage;
        h.chipDamage = chip;
        h.hitStun = hitStun;
        h.blockStun = blockStun;
        h.hitStop = hitStop;
        h.priority = priority;
        h.knockbackX = kbX;
        h.knockbackY = kbY;
        h.knockdown = knockdown;
        return h;
    }

    const FighterDef& GetGutoBarba()
    {
        static const FighterDef def = []()
        {
            FighterDef d;
            d.id = FighterId::GutoBarba;
            d.name = "GUTO BARBA";
            d.archetype = "TANK / GRAPPLER";
            d.stats.maxHealth = 1250;
            d.stats.walkSpeed = 1.75f;
            d.stats.backwardSpeed = 1.45f;
            d.stats.jumpSpeed = 12.8f;
            d.stats.jumpForwardSpeed = 1.9f;
            d.stats.jumpBackwardSpeed = 1.7f;
            d.stats.gravity = 0.86f;
            d.stats.weight = 1.28f;
            d.standing = MakePose(-31, -208, 31, -158, -46, -170, 46, -73, -55, -82, 55, 0, -35, -160, 35, 0);
            d.crouching = MakePose(-34, -182, 34, -136, -51, -145, 51, -58, -62, -68, 62, 0, -43, -122, 43, 0);
            d.airborne = MakePose(-30, -207, 30, -158, -45, -168, 45, -72, -53, -81, 53, -5, -34, -154, 34, -20);
            d.spriteFrameSize = 288;
            d.visualSolePad = 6;
            d.idleFps = 5;
            d.walkFps = 7;

            d.punch = {"elbow", "Cotovelada", FighterState::LightAttack, AnimId::StandingLight, 20, false,
                MakeHit("guto-elbow", 6, 9, 79, -176, 113, -149, HitLevel::Mid, 66, 2, 15, 10, 7, 2, 2.2f, 0.0f)};
            d.kick = {"frontKick", "Chute frontal", FighterState::KickAttack, AnimId::CrouchLight, 29, false,
                MakeHit("guto-front-kick", 10, 14, 52, -118, 110, -56, HitLevel::Low, 82, 4, 17, 12, 8, 3, 3.3f, 0.0f)};
            d.crouchPunch = d.kick;
            d.crouchKick = {"rasteiraUrso", "Rasteira do Urso", FighterState::KickAttack, AnimId::CrouchHeavy, 36, false,
                MakeHit("guto-sweep", 14, 18, 67, -71, 125, -20, HitLevel::Low, 118, 7, 24, 15, 10, 3, 3.4f, -3.4f, true)};
            d.airPunch = {"jumpLightNeutral", "Soco aereo", FighterState::LightAttack, AnimId::AirLight, 24, true,
                MakeHit("guto-air-light", 6, 15, 35, -173, 69, -119, HitLevel::Overhead, 72, 4, 15, 10, 6, 2, 2.2f, 0.0f)};
            d.airKick = {"jumpHeavyNeutral", "Chute aereo", FighterState::HeavyAttack, AnimId::AirHeavy, 28, true,
                MakeHit("guto-air-kick", 8, 17, 67, -144, 125, -84, HitLevel::Overhead, 120, 8, 20, 13, 9, 3, 3.6f, -2.5f)};
            return d;
        }();
        return def;
    }

    const FighterDef& GetRafaMare()
    {
        static const FighterDef def = []()
        {
            FighterDef d;
            d.id = FighterId::RafaMare;
            d.name = "RAFA MARE";
            d.archetype = "AGILE / RUSHDOWN";
            d.stats.maxHealth = 1000;
            d.stats.walkSpeed = 3.05f;
            d.stats.backwardSpeed = 2.5f;
            d.stats.jumpSpeed = 15.6f;
            d.stats.jumpForwardSpeed = 3.3f;
            d.stats.jumpBackwardSpeed = 2.9f;
            d.stats.gravity = 0.92f;
            d.stats.weight = 0.9f;
            d.standing = MakePose(-24, -172, 24, -128, -35, -138, 35, -58, -45, -66, 45, 0, -25, -132, 25, 0);
            d.crouching = MakePose(-27, -143, 27, -104, -43, -112, 43, -45, -54, -52, 54, 0, -34, -96, 34, 0);
            d.airborne = MakePose(-24, -168, 24, -126, -34, -134, 34, -58, -40, -66, 40, -4, -23, -124, 23, -18);
            d.spriteFrameSize = 256;
            d.visualSolePad = 6;
            d.idleFps = 6;
            d.walkFps = 10;

            d.punch = {"lightPunch", "Soco rapido", FighterState::LightAttack, AnimId::StandingLight, 15, false,
                MakeHit("rafa-jab", 4, 6, 50, -144, 84, -124, HitLevel::High, 45, 0, 11, 8, 5, 1, 1.6f, 0.0f)};
            d.kick = {"highKick", "Chute alto", FighterState::KickAttack, AnimId::ForwardHeavy, 32, false,
                MakeHit("rafa-high", 12, 16, 58, -127, 95, -101, HitLevel::Overhead, 102, 6, 20, 14, 9, 3, 3.8f, -3.6f)};
            d.crouchPunch = {"lowKick", "Chute baixo", FighterState::KickAttack, AnimId::CrouchLight, 23, false,
                MakeHit("rafa-low", 7, 10, 59, -115, 93, -94, HitLevel::Low, 58, 2, 14, 10, 6, 1, 2.1f, 0.0f)};
            d.crouchKick = {"rasteira", "Rasteira", FighterState::KickAttack, AnimId::CrouchHeavy, 31, false,
                MakeHit("rafa-sweep", 11, 15, 77, -41, 113, -11, HitLevel::Low, 92, 5, 22, 14, 8, 2, 3.0f, -3.0f, true)};
            d.airPunch = {"jumpLightNeutral", "Soco aereo", FighterState::LightAttack, AnimId::AirLight, 22, true,
                MakeHit("rafa-air-light", 5, 14, 10, -136, 44, -92, HitLevel::Overhead, 58, 3, 13, 9, 5, 2, 1.8f, 0.0f)};
            d.airKick = {"jumpHeavyNeutral", "Chute aereo", FighterState::HeavyAttack, AnimId::AirHeavy, 26, true,
                MakeHit("rafa-air-kick", 7, 16, 21, -121, 56, -74, HitLevel::Overhead, 96, 6, 18, 12, 8, 3, 3.2f, -2.0f)};
            return d;
        }();
        return def;
    }

    const FighterDef& GetFighterDef(FighterId id)
    {
        return id == FighterId::RafaMare ? GetRafaMare() : GetGutoBarba();
    }
}
