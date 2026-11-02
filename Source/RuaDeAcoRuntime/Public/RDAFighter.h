#pragma once

#include "RDATypes.h"

namespace RDA
{
const FighterDef &GetGutoBarba();
const FighterDef &GetRafaMare();
const FighterDef &GetNoirReflexo();
const FighterDef &GetAstroRiso();
const FighterDef &GetDanteSinal();
const FighterDef &GetLeoVioleta();
const FighterDef &GetFighterDef(FighterId Id);
const FighterDef *FindFighterByCanonicalId(const char *CanonicalId);

const StageDef &GetStageDef(StageId Id);
const StageDef *FindStageByCanonicalId(const char *CanonicalId);

struct AppliedHit
{
    int damage = 0;
    bool blocked = false;
    bool knockout = false;
    bool grabConnected = false;
    bool grabTeched = false;
    bool parryTriggered = false;
    bool armored = false;
};

struct FighterSnapshot
{
    FighterId id = FighterId::GutoBarba;
    const char *name = "";
    float x = 0.0f;
    float y = 0.0f;
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    int facing = 1;
    FighterState state = FighterState::Idle;
    AnimId anim = AnimId::Idle;
    int stateFrame = 0;
    int health = 1000;
    int maxHealth = 1000;
    int meter = 0;
    int roundWins = 0;
    int comboCount = 0;
    bool grounded = true;
    bool attacking = false;
    bool moveConnected = false;
    FighterState poseState = FighterState::Idle;
    AnimId poseAnim = AnimId::Idle;
    int poseStateFrame = 0;
    const char *poseMoveId = "";
    const char *poseMoveAnimationKey = "";
    bool poseMoveConnected = false;
    int poseMoveTotalFrames = 0;
    int poseHitActiveFrom = 0;
    int poseHitActiveTo = 0;
    const char *moveId = "";
    const char *moveLabel = "";
    const char *moveAnimationKey = "";
    int moveFrame = 0;
    int moveTotalFrames = 0;
    int hitActiveFrom = 0;
    int hitActiveTo = 0;
    int armorHits = 0;
    int parryFrames = 0;
    int damageReductionFrames = 0;
    int freezeFrames = 0;
    float victimRotation = 0.0f;
    bool visible = true;
    int victimPoseFrame = -1;
    float victimScale = 1;
    float cutBelowY = -1;
    bool victimInFront = false;
    Rect pushboxWorld{};
    std::vector<Rect> hurtboxesWorld;
    std::vector<Rect> hitboxesWorld;
};

class FighterRuntime
{
  public:
    FighterRuntime(const FighterDef &InDef, float StartX, int StartFacing);

    void ResetForRound(float StartX, int StartFacing);
    void ResetPosition(float StartX, int StartFacing);
    void BeginFrame(const InputFrame &Input, int WorldFrame);
    void FinishFrame();
    bool HasActiveHitbox(HitboxDef &OutHitbox, Rect &OutWorldRect) const;
    std::vector<Rect> CurrentHurtboxesWorld() const;
    Rect PushboxWorld() const;
    AppliedHit ReceiveHit(const HitboxDef &Hitbox, int AttackerFacing, bool bAllowGrabTech, bool InfiniteHealth = false,
                          int ComboDepth = 1, float AttackMultiplier = 1);
    void NotifyAttackConnected(bool bBlocked, const MoveDef &Move);
    void ForceState(FighterState NewState);
    void ForceMeter(int NewMeter);
    void EnterGrabbedState(FighterState VictimState, float AnchorX, float AnchorY, float RotationRad, int VictimFacing);
    void ReleaseFromGrab(float ThrowVelX, float ThrowVelY, bool bSlamToKnockdown);
    bool ConsumePendingProjectile(ProjectileDef &OutDef);
    FighterSnapshot Snapshot() const;
    std::vector<HitboxDef> ActiveHitboxes() const;
    bool CanReceive(const HitboxDef &Hitbox) const;
    void CaptureCollisionPose();
    void RegisterHit(const HitboxDef &Hitbox);
    void ConsumeMoveEvents();

    const FighterDef *def = nullptr;
    float x = 0.0f;
    float y = static_cast<float>(kGroundY);
    float velocityX = 0.0f;
    float velocityY = 0.0f;
    float airDriftX = 0.0f;
    int facing = 1;
    FighterState state = FighterState::Idle;
    int stateFrame = 0;
    int health = 1000;
    int meter = 0;
    int roundWins = 0;
    bool grounded = true;

    const MoveDef *currentMove = nullptr;
    bool moveHasRegisteredContact = false;
    int hitStopFrames = 0;
    int stunFrames = 0;
    int knockdownFrames = 0;
    int freezeFrames = 0;
    int armorHits = 0;
    int parryFrames = 0;
    int parryRiposteDamage = 0;
    int damageReductionFrames = 0;
    float damageReductionMultiplier = 1.0f;
    float victimRotation = 0.0f;
    bool visible = true;
    int victimPoseFrame = -1;
    float victimScale = 1;
    float cutBelowY = -1;
    bool victimInFront = false;

    int lastPunchFrame = -100;
    int lastKickFrame = -100;
    int lastDownFrame = -100;
    int lastForwardFrame = -100;
    int lastBackFrame = -100;
    int invulnerableFrames = 0;
    int blockStunFrames = 0;
    int passiveFrames = 0;
    int damageReductionCooldownFrames = 0;
    int offensiveDebuffFrames = 0;
    float offensiveDebuffMultiplier = 1;
    int damageTowardPassive = 0;
    int moveConnected = 0;
    bool frozen = false;
    bool grabbed = false;
    bool knockdownPending = false;
    bool jumpQueued = false;
    InputFrame frameInput{};
    std::vector<std::pair<int, int>> directionSamples;
    std::vector<std::pair<int, int>> buttonSamples;
    std::vector<std::string> registeredHitIds;
    std::vector<int> emittedEvents;
    std::vector<Rect> evaluatedHurtboxes;
    std::vector<HitboxDef> evaluatedHitboxes;
    Rect evaluatedPushbox{};
    bool poseCaptured = false;
    FighterState evaluatedState = FighterState::Idle;
    AnimId evaluatedAnim = AnimId::Idle;
    int evaluatedStateFrame = 0;
    const MoveDef *evaluatedMove = nullptr;
    int evaluatedMoveConnected = 0;

  private:
    bool TryStartMove(const InputFrame &Input, int WorldFrame);
    void StartMove(const MoveDef &Move);
    void ApplyHorizontalFriction();
    void Transition(FighterState NewState);
    void UpdateVertical();
    void ApplyMoveMotion(const MoveDef &Move);
    int PoseKind() const;
    bool MatchCommand(const CommandDef &Command, int Frame) const;
    AnimId CurrentAnim() const;

    int pendingProjectileIndex = -1;
};

int MoveAnimFrame(const MoveDef &Move, int StateFrame, int SheetFrames);
} // namespace RDA
