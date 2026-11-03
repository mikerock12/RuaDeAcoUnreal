#pragma once

#include "RDAFighter.h"

namespace RDA
{
enum class SoundEvent : uint8_t
{
    None = 0,
    Fight,
    Swing,
    HitLight,
    HitHeavy,
    Block,
    Jump,
    Knockdown,
    RoundEnd,
    MatchWin,
    GrabStart,
    GrabSlam,
    SpecialCast,
    SuperCast,
    FinishReady,
    FinisherMonsterRoar,
    FinisherSplash,
    FinisherPotDrop,
    FinisherTrident,
};

struct SoundCue
{
    SoundEvent event = SoundEvent::None;
    int fighterIndex = -1;
    const char *moveId = "";
};

struct ProjectileRuntime
{
    int runtimeId = 0;
    int ownerIndex = 0;
    ProjectileDef def{};
    float x = 0.0f;
    float y = 0.0f;
    float velocityX = 0.0f;
    int facing = 1;
    int life = 0;
    int ageFrames = 0;
    const MoveDef *sourceMove = nullptr;
};

struct ProjectileSnapshot
{
    int runtimeId = 0;
    int ownerIndex = 0;
    const char *id = "";
    float x = 0.0f;
    float y = 0.0f;
    int facing = 1;
    Rect worldBox{};
    bool active = true;
    const char *sourceMoveId = "";
    int ageFrames = 0;
    int armingFrames = 0;
};

struct FinisherVisualSnapshot
{
    bool active = false;
    int frame = 0;
    int attackerFrame = 0;
    bool monsterVisible = false;
    int monsterFrame = 0;
    float monsterX = 0, monsterY = 0, monsterScale = 1.35f, monsterReveal = 0;
    bool monsterFlip = false;
    float mouthX = 0, mouthY = 0;
    float doors = 0;
    bool farmerVisible = false;
    int farmerFrame = 0, farmerFacing = 1;
    float farmerX = 0, farmerFeetY = 0;
    float tridentX = 0, tridentBaseY = 0, tridentTipX = 0, tridentTipY = 0;
    float shadow = 0, blood = 0;
    bool bonesVisible = false;
};

struct ActiveGrabState
{
    bool active = false;
    int attackerIndex = 0;
    int defenderIndex = 1;
    MoveDef move{};
    HitboxDef hitbox{};
    int comboDepth = 1;
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
    int roundWinner = -1;
    int matchWinner = -1;
    int finisherWinner = -1;
    bool finisherCompleted = false;
    StageId stage = StageId::CaisDaCidade;
    GameMode mode = GameMode::VsCpu;
    const char *banner = "";
    std::array<FighterSnapshot, 2> fighters{};
    std::vector<ProjectileSnapshot> projectiles;
    bool grabActive = false;
    int lastDamage = 0;
    FinisherVisualSnapshot visualFinisher{};
};

struct CpuPlanStep
{
    int frames = 1;
    InputFrame actions{};
};
struct CpuControllerState
{
    int lastFrame = -1;
    const FighterDef *definition = nullptr;
    AiDifficulty difficulty = AiDifficulty::Normal;
    uint32_t randomState = 1;
    std::vector<WorldSnapshot> observations;
    std::vector<CpuPlanStep> plan;
    InputFrame previousActions{}, cachedInput{};
    int decisionCooldown = 0;
};

class CombatWorld
{
  public:
    CombatWorld(const FighterDef &PlayerOne, const FighterDef &PlayerTwo, GameMode InMode = GameMode::LocalVersus,
                StageId InStage = StageId::CaisDaCidade);

    void Step(const InputFrame &P1Input, const InputFrame &P2Input);
    void TogglePause();
    void ToggleDebugBoxes();
    void RestartMatch();
    void ResetTrainingPositions();
    WorldSnapshot Snapshot() const;
    std::vector<SoundEvent> DrainSounds();
    std::vector<SoundCue> DrainSoundCues();
    uint64_t ComputeStateHash() const;

    CombatPhase Phase() const
    {
        return phase;
    }
    bool IsPaused() const
    {
        return paused;
    }
    StageId Stage() const
    {
        return stage;
    }
    GameMode Mode() const
    {
        return mode;
    }

    std::array<FighterRuntime, 2> fighters;

  private:
    friend InputFrame BuildCpuInput(const CombatWorld &, int, AiDifficulty);
    mutable std::array<CpuControllerState, 2> cpuControllers{};
    void StepIntro();
    void StepActive(const InputFrame &P1Input, const InputFrame &P2Input);
    void StepFinishReady(const InputFrame &P1Input, const InputFrame &P2Input);
    void StepStageFinish();
    void StepRoundOver();
    void CompleteFinish();
    void SpawnProjectiles(int OwnerIndex);
    void UpdateProjectiles();
    void ResolveHits(const InputFrame &P1Input, const InputFrame &P2Input);
    void UpdateActiveGrab();
    void ResolvePushboxes();
    void FinishRound(int WinnerIndex, bool bKnockout);
    void StartNextRound();

    GameMode mode = GameMode::LocalVersus;
    StageId stage = StageId::CaisDaCidade;
    int frame = 0;
    CombatPhase phase = CombatPhase::Intro;
    int phaseFrame = 0;
    int round = 1;
    int timeFrames = kRoundTimeFrames;
    bool paused = false;
    bool debugBoxes = false;
    bool roundDraw = false;
    int roundWinner = -1;
    int matchWinner = -1;
    int finisherWinner = -1;
    bool finisherCompleted = false;
    int lastDamage = 0;
    int nextProjectileId = 1;
    std::array<int, 2> comboCounts{0, 0};
    std::array<int, 2> comboTimers{0, 0};
    std::vector<ProjectileRuntime> projectiles;
    ActiveGrabState activeGrab{};
    std::vector<SoundEvent> soundEvents;
    std::vector<SoundCue> soundCues;
};

InputFrame BuildCpuInput(const CombatWorld &World, int CpuIndex, AiDifficulty Difficulty = AiDifficulty::Normal);
} // namespace RDA
