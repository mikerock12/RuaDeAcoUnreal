#pragma once

#include "RDAFighter.h"

namespace RDA
{
    class RDA_API CombatWorld
    {
    public:
        CombatWorld(const FighterDef& playerOne, const FighterDef& playerTwo);

        void Step(const InputFrame& p1, const InputFrame& p2);
        bool TogglePause();
        void SetPaused(bool value);
        void ToggleDebugBoxes();
        void RestartMatch();
        WorldSnapshot Snapshot() const;

        CombatPhase Phase() const { return phase; }
        bool Paused() const { return paused; }

    private:
        void StepIntro();
        void StepActive(const InputFrame& p1, const InputFrame& p2);
        void StepRoundOver();
        void UpdateFacing();
        void ResolveHits();
        void ResolvePushboxes();
        void FinishRound(int winner, bool knockout);
        const char* CurrentBanner() const;

        Fighter fighters[2];
        int frame = 0;
        CombatPhase phase = CombatPhase::Intro;
        int phaseFrame = 0;
        int round = 1;
        int timeFrames = kRoundTimeFrames;
        bool paused = false;
        bool debugBoxes = false;
        bool roundDraw = false;
        int roundWinner = -1;
        int comboHits[2] = {0, 0};
        int lastDamage = 0;
        const char* banner = "ROUND 1";
        char bannerBuffer[24] = "ROUND 1";
        int bannerFrames = 90;
    };
}
