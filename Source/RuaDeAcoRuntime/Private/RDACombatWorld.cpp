#include "RDACombatWorld.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace RDA
{
    CombatWorld::CombatWorld(const FighterDef& playerOne, const FighterDef& playerTwo)
        : fighters{Fighter(playerOne, kP1StartX, 1), Fighter(playerTwo, kP2StartX, -1)}
    {
        banner = "ROUND 1";
        bannerFrames = 90;
    }

    void CombatWorld::Step(const InputFrame& p1, const InputFrame& p2)
    {
        if (p1.debugPressed || p2.debugPressed)
        {
            debugBoxes = !debugBoxes;
        }
        if (paused)
        {
            return;
        }

        frame += 1;
        phaseFrame += 1;
        if (bannerFrames > 0)
        {
            bannerFrames -= 1;
        }

        switch (phase)
        {
        case CombatPhase::Intro:
            StepIntro();
            break;
        case CombatPhase::Active:
            StepActive(p1, p2);
            break;
        case CombatPhase::RoundOver:
            StepRoundOver();
            break;
        case CombatPhase::MatchOver:
            fighters[0].BeginFrame(InputFrame{}, fighters[1].X());
            fighters[1].BeginFrame(InputFrame{}, fighters[0].X());
            fighters[0].CapturePose();
            fighters[1].CapturePose();
            fighters[0].FinishFrame();
            fighters[1].FinishFrame();
            break;
        }
    }

    bool CombatWorld::TogglePause()
    {
        paused = !paused;
        return paused;
    }

    void CombatWorld::SetPaused(bool value)
    {
        paused = value;
    }

    void CombatWorld::ToggleDebugBoxes()
    {
        debugBoxes = !debugBoxes;
    }

    void CombatWorld::RestartMatch()
    {
        fighters[0].ResetMatch(kP1StartX, 1);
        fighters[1].ResetMatch(kP2StartX, -1);
        frame = 0;
        phase = CombatPhase::Intro;
        phaseFrame = 0;
        round = 1;
        timeFrames = kRoundTimeFrames;
        paused = false;
        roundDraw = false;
        roundWinner = -1;
        comboHits[0] = 0;
        comboHits[1] = 0;
        lastDamage = 0;
        banner = "ROUND 1";
        bannerFrames = 90;
    }

    WorldSnapshot CombatWorld::Snapshot() const
    {
        WorldSnapshot snap;
        snap.frame = frame;
        snap.phase = phase;
        snap.phaseFrame = phaseFrame;
        snap.round = round;
        snap.timeSeconds = std::max(0, (timeFrames + 59) / 60);
        snap.paused = paused;
        snap.debugBoxes = debugBoxes;
        snap.roundDraw = roundDraw;
        snap.roundWins[0] = fighters[0].RoundWins();
        snap.roundWins[1] = fighters[1].RoundWins();
        snap.fighters[0] = fighters[0].MakeSnapshot();
        snap.fighters[1] = fighters[1].MakeSnapshot();
        snap.banner = CurrentBanner();
        snap.lastDamage = lastDamage;
        return snap;
    }

    void CombatWorld::StepIntro()
    {
        UpdateFacing();
        fighters[0].BeginFrame(InputFrame{}, fighters[1].X());
        fighters[1].BeginFrame(InputFrame{}, fighters[0].X());
        fighters[0].CapturePose();
        fighters[1].CapturePose();
        fighters[0].FinishFrame();
        fighters[1].FinishFrame();

        if (phaseFrame == kIntroFightFrame)
        {
            banner = "FIGHT";
            bannerFrames = 45;
        }
        if (phaseFrame >= kIntroEndFrame)
        {
            phase = CombatPhase::Active;
            phaseFrame = 0;
            banner = "";
            bannerFrames = 0;
        }
    }

    void CombatWorld::StepActive(const InputFrame& p1, const InputFrame& p2)
    {
        UpdateFacing();
        fighters[0].BeginFrame(p1, fighters[1].X());
        fighters[1].BeginFrame(p2, fighters[0].X());
        fighters[0].CapturePose();
        fighters[1].CapturePose();
        ResolveHits();
        ResolvePushboxes();
        fighters[0].FinishFrame();
        fighters[1].FinishFrame();

        for (int i = 0; i < 2; ++i)
        {
            const Fighter& defender = fighters[i == 0 ? 1 : 0];
            if (defender.State() != FighterState::HitStun && defender.State() != FighterState::Knockdown)
            {
                comboHits[i] = 0;
            }
        }

        timeFrames = std::max(0, timeFrames - 1);
        if (fighters[0].Health() <= 0 || fighters[1].Health() <= 0 || timeFrames == 0)
        {
            const bool knockout = fighters[0].Health() <= 0 || fighters[1].Health() <= 0;
            const float r0 = static_cast<float>(fighters[0].Health()) / static_cast<float>(fighters[0].Def().stats.maxHealth);
            const float r1 = static_cast<float>(fighters[1].Health()) / static_cast<float>(fighters[1].Def().stats.maxHealth);
            int winner = -1;
            if (std::fabs(r0 - r1) >= 0.0001f)
            {
                winner = r0 > r1 ? 0 : 1;
            }
            FinishRound(winner, knockout);
        }
    }

    void CombatWorld::StepRoundOver()
    {
        fighters[0].BeginFrame(InputFrame{}, fighters[1].X());
        fighters[1].BeginFrame(InputFrame{}, fighters[0].X());
        fighters[0].CapturePose();
        fighters[1].CapturePose();
        fighters[0].FinishFrame();
        fighters[1].FinishFrame();

        if (phaseFrame < kRoundOverFrames)
        {
            return;
        }

        if (roundWinner >= 0 && fighters[roundWinner].RoundWins() >= kRoundsToWin)
        {
            phase = CombatPhase::MatchOver;
            phaseFrame = 0;
            banner = "VITORIA";
            bannerFrames = 240;
            return;
        }

        if (roundWinner >= 0)
        {
            round += 1;
        }
        timeFrames = kRoundTimeFrames;
        comboHits[0] = 0;
        comboHits[1] = 0;
        fighters[0].ResetRound(kP1StartX, 1);
        fighters[1].ResetRound(kP2StartX, -1);
        roundWinner = -1;
        roundDraw = false;
        phase = CombatPhase::Intro;
        phaseFrame = 0;
        std::snprintf(bannerBuffer, sizeof(bannerBuffer), "ROUND %d", round);
        banner = bannerBuffer;
        bannerFrames = 90;
    }

    void CombatWorld::UpdateFacing()
    {
        if (fighters[0].Grounded() && !fighters[0].Attacking())
        {
            fighters[0].SetFacing(fighters[0].X() <= fighters[1].X() ? 1 : -1);
        }
        if (fighters[1].Grounded() && !fighters[1].Attacking())
        {
            fighters[1].SetFacing(fighters[1].X() <= fighters[0].X() ? 1 : -1);
        }
    }

    void CombatWorld::ResolveHits()
    {
        auto findContact = [&](int attackerIndex) -> const Hitbox*
        {
            Fighter& attacker = fighters[attackerIndex];
            Fighter& defender = fighters[attackerIndex == 0 ? 1 : 0];
            if (!attacker.CanRegisterHit() || !defender.CanReceiveHit())
            {
                return nullptr;
            }
            for (const Hitbox& hit : attacker.EvaluatedHitboxes())
            {
                const Rect worldHit = ToWorld(hit.box, attacker.X(), attacker.Y(), attacker.Facing());
                for (const Hurtbox& hurt : defender.EvaluatedHurtboxes())
                {
                    const Rect worldHurt = ToWorld(hurt.box, defender.X(), defender.Y(), defender.Facing());
                    if (Intersects(worldHit, worldHurt))
                    {
                        return &hit;
                    }
                }
            }
            return nullptr;
        };

        const Hitbox* c0 = findContact(0);
        const Hitbox* c1 = findContact(1);
        auto apply = [&](int attackerIndex, const Hitbox* hit)
        {
            if (!hit) return;
            Fighter& attacker = fighters[attackerIndex];
            Fighter& defender = fighters[attackerIndex == 0 ? 1 : 0];
            if (!attacker.CanRegisterHit()) return;
            const bool blocked = defender.IsBlocking(hit->level);
            const int combo = defender.State() == FighterState::HitStun ? comboHits[attackerIndex] + 1 : 1;
            const int healthBefore = defender.Health();
            defender.ApplyHit(*hit, attacker.Facing(), blocked, combo);
            attacker.RegisterHit();
            attacker.AddHitStop(hit->hitStop);
            lastDamage = healthBefore - defender.Health();
            if (!blocked)
            {
                comboHits[attackerIndex] = combo;
            }
        };

        if (c0 && c1)
        {
            if (c0->priority > c1->priority)
            {
                apply(0, c0);
            }
            else if (c1->priority > c0->priority)
            {
                apply(1, c1);
            }
            else
            {
                apply(0, c0);
                apply(1, c1);
            }
        }
        else
        {
            apply(0, c0);
            apply(1, c1);
        }
    }

    void CombatWorld::ResolvePushboxes()
    {
        Fighter& one = fighters[0];
        Fighter& two = fighters[1];
        if (std::fabs(one.Y() - two.Y()) > 96.0f)
        {
            return;
        }
        const Rect oneBox = ToWorld(one.EvaluatedPushbox(), one.X(), one.Y(), one.Facing());
        const Rect twoBox = ToWorld(two.EvaluatedPushbox(), two.X(), two.Y(), two.Facing());
        const float overlap = HorizontalOverlap(oneBox, twoBox);
        if (overlap <= 0.0f || !Intersects(oneBox, twoBox))
        {
            return;
        }
        const bool oneIsLeft = one.X() <= two.X();
        const float totalWeight = one.Def().stats.weight + two.Def().stats.weight;
        const float moveOne = overlap * (two.Def().stats.weight / totalWeight);
        const float moveTwo = overlap * (one.Def().stats.weight / totalWeight);
        one.x += oneIsLeft ? -moveOne : moveOne;
        two.x += oneIsLeft ? moveTwo : -moveTwo;
        one.ClampToStage();
        two.ClampToStage();
    }

    void CombatWorld::FinishRound(int winner, bool knockout)
    {
        if (phase != CombatPhase::Active)
        {
            return;
        }
        roundWinner = winner;
        roundDraw = winner < 0;
        phase = CombatPhase::RoundOver;
        phaseFrame = 0;
        if (winner < 0)
        {
            banner = "EMPATE";
            bannerFrames = 120;
            return;
        }
        fighters[winner].AddRoundWin();
        fighters[winner].SetMatchState(FighterState::Victory);
        fighters[winner == 0 ? 1 : 0].SetMatchState(FighterState::Knockout);
        banner = knockout ? "KO" : "TEMPO";
        bannerFrames = 120;
    }

    const char* CombatWorld::CurrentBanner() const
    {
        if (paused) return "PAUSA";
        if (bannerFrames > 0) return banner;
        if (phase == CombatPhase::MatchOver) return "VITORIA";
        return "";
    }
}
