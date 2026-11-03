#include "RDACombatWorld.h"

#include <chrono>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

namespace
{
using namespace RDA;
#include "../Data/generated/RDACanonicalReplays.inc"
namespace fs = std::filesystem;

int gPassCount = 0;
int gFailCount = 0;

void Check(bool Cond, const char *Name, const char *Detail = "")
{
    if (Cond)
    {
        gPassCount += 1;
    }
    else
    {
        gFailCount += 1;
        std::fprintf(stderr, "FAIL: %s (%s)\n", Name, Detail);
    }
}

bool Near(float A, float B, float Eps = 0.01f)
{
    return std::fabs(A - B) <= Eps;
}

InputFrame ReplayDecode(int held, int pressed)
{
    InputFrame i{};
    i.left = held & 1;
    i.right = held & 2;
    i.up = held & 4;
    i.down = held & 8;
    i.punch = held & 16;
    i.kick = held & 32;
    i.special = held & 64;
    i.block = held & 128;
    i.leftPressed = pressed & 1;
    i.rightPressed = pressed & 2;
    i.upPressed = pressed & 4;
    i.downPressed = pressed & 8;
    i.punchPressed = pressed & 16;
    i.kickPressed = pressed & 32;
    i.specialPressed = pressed & 64;
    i.blockPressed = pressed & 128;
    return i;
}
void RunCanonicalReplay(const char *name, const FighterDef &one, const FighterDef &two, GameMode mode, StageId stage,
                        const ReplayStep *expected, const ReplayInput *inputs, int steps)
{
    CombatWorld w(one, two, mode, stage);
    int mismatches = 0;
    for (int p = 0; p < 2; ++p)
    {
        w.fighters[p].x = expected[0].fighters[p].x;
        w.fighters[p].health = expected[0].fighters[p].health;
        w.fighters[p].roundWins = expected[0].fighters[p].roundWins;
    }
    for (int step = 0; step <= steps; ++step)
    {
        if (step)
        {
            auto i = inputs[step - 1];
            w.Step(ReplayDecode(i.h1, i.p1), ReplayDecode(i.h2, i.p2));
        }
        const auto s = w.Snapshot();
        const auto &e = expected[step];
        bool good =
            s.frame == e.frame && s.phase == e.phase && s.phaseFrame == e.phaseFrame &&
            s.projectiles.size() == e.projectiles &&
            std::strcmp(s.grabActive ? w.fighters[w.fighters[0].grabbed ? 1 : 0].currentMove->id : "", e.grab) == 0;
        for (int p = 0; p < 2; ++p)
        {
            const auto &a = s.fighters[p];
            const auto &b = e.fighters[p];
            bool fg = Near(a.x, b.x, 0.002f) && Near(a.y, b.y, 0.002f) && Near(a.velocityX, b.vx, 0.002f) &&
                      Near(a.velocityY, b.vy, 0.002f) && a.facing == b.facing && a.state == b.state &&
                      a.stateFrame == b.stateFrame && a.health == b.health && a.meter == b.meter &&
                      a.roundWins == b.roundWins && a.grounded == b.grounded && std::strcmp(a.moveId, b.move) == 0;
            const auto &f = w.fighters[p];
            float mechanics[] = {float(f.armorHits),
                                 float(f.passiveFrames),
                                 float(f.invulnerableFrames),
                                 float(f.damageReductionFrames),
                                 f.damageReductionMultiplier,
                                 float(f.damageReductionCooldownFrames),
                                 float(f.parryFrames),
                                 float(f.parryRiposteDamage),
                                 float(f.offensiveDebuffFrames),
                                 f.offensiveDebuffMultiplier,
                                 float(f.freezeFrames),
                                 f.victimRotation,
                                 float(f.victimPoseFrame)};
            for (int k = 0; k < 13; ++k)
                if (!Near(mechanics[k], b.mechanics[k], 0.002f))
                {
                    fg = false;
                    if (mismatches < 8)
                        std::fprintf(stderr, "REPLAY %s step=%d P%d mechanics[%d]=%.6f expected=%.6f\n", name, step,
                                     p + 1, k, mechanics[k], b.mechanics[k]);
                }
            if (!fg && mismatches < 8)
                std::fprintf(
                    stderr,
                    "REPLAY %s step=%d P%d actual pos=(%.5f,%.5f) vel=(%.5f,%.5f) facing=%d state=%d/%d hp=%d meter=%d "
                    "move=%s; expected pos=(%.5f,%.5f) vel=(%.5f,%.5f) facing=%d state=%d/%d hp=%d meter=%d move=%s\n",
                    name, step, p + 1, a.x, a.y, a.velocityX, a.velocityY, a.facing, int(a.state), a.stateFrame,
                    a.health, a.meter, a.moveId, b.x, b.y, b.vx, b.vy, b.facing, int(b.state), b.stateFrame, b.health,
                    b.meter, b.move);
            good = good && fg;
        }
        if (!good)
            ++mismatches;
    }
    std::printf("CANONICAL REPLAY %s steps=%d divergences=%d\n", name, steps, mismatches);
    Check(mismatches == 0, "CanonicalReplay", name);
}

fs::path FindGoldenFixturesPath()
{
    fs::path Cur = fs::current_path();
    for (int i = 0; i < 6; ++i)
    {
        const fs::path Candidate = Cur / "Data" / "generated" / "golden_fixtures.json";
        if (fs::exists(Candidate))
            return Candidate;
        if (!Cur.has_parent_path())
            break;
        Cur = Cur.parent_path();
    }
    return fs::path("Data/generated/golden_fixtures.json");
}

void TestCatalogAndGoldenFixturesJson()
{
    const fs::path JsonPath = FindGoldenFixturesPath();
    Check(fs::exists(JsonPath), "GoldenFixturesExists", JsonPath.string().c_str());

    std::ifstream In(JsonPath);
    std::ostringstream Buffer;
    Buffer << In.rdbuf();
    const std::string Content = Buffer.str();
    Check(Content.size() > 1000, "GoldenFixturesNonEmpty");

    const char *ExpectedFighters[RDA::kFighterCount] = {"guto-barba", "rafa-mare",   "noir-reflexo",
                                                        "astro-riso", "dante-sinal", "leo-violeta"};

    for (int i = 0; i < RDA::kFighterCount; ++i)
    {
        const RDA::FighterDef *Def = RDA::FindFighterByCanonicalId(ExpectedFighters[i]);
        Check(Def != nullptr, "FighterInCatalog", ExpectedFighters[i]);
        Check(Content.find(std::string("\"id\": \"") + ExpectedFighters[i] + "\"") != std::string::npos,
              "FighterInGoldenJson", ExpectedFighters[i]);
    }

    // Verify canonical stats match golden_fixtures.json
    const RDA::FighterDef &Guto = RDA::GetGutoBarba();
    Check(Guto.stats.maxHealth == 1250 && Near(Guto.stats.walkSpeed, 2.47f) && Near(Guto.stats.jumpSpeed, 12.8f),
          "GutoCanonicalStats");

    const RDA::FighterDef &Rafa = RDA::GetRafaMare();
    Check(Rafa.stats.maxHealth == 1000 && Near(Rafa.stats.walkSpeed, 4.26f) && Near(Rafa.stats.jumpSpeed, 15.6f),
          "RafaCanonicalStats");

    const RDA::FighterDef &Noir = RDA::GetNoirReflexo();
    Check(Noir.stats.maxHealth == 980 && Near(Noir.stats.walkSpeed, 3.74f) && Near(Noir.stats.jumpSpeed, 14.4f),
          "NoirCanonicalStats");

    const RDA::FighterDef &Astro = RDA::GetAstroRiso();
    Check(Astro.stats.maxHealth == 930 && Near(Astro.stats.walkSpeed, 4.72f) && Near(Astro.stats.jumpSpeed, 15.4f),
          "AstroCanonicalStats");

    const RDA::FighterDef &Dante = RDA::GetDanteSinal();
    Check(Dante.stats.maxHealth == 960 && Near(Dante.stats.walkSpeed, 4.03f) && Near(Dante.stats.jumpSpeed, 14.8f),
          "DanteCanonicalStats");

    const RDA::FighterDef &Leo = RDA::GetLeoVioleta();
    Check(Leo.stats.maxHealth == 1050 && Near(Leo.stats.walkSpeed, 4.08f) && Near(Leo.stats.jumpSpeed, 14.4f),
          "LeoCanonicalStats");

    // Verify all 3 stages
    Check(RDA::FindStageByCanonicalId("cais-da-cidade") != nullptr, "StageCais");
    Check(RDA::FindStageByCanonicalId("cozinha-macabra") != nullptr, "StageCozinha");
    Check(RDA::FindStageByCanonicalId("sitio") != nullptr, "StageSitio");
}

void TestUniversalGrabAndTech()
{
    RDA::CombatWorld World(RDA::GetDanteSinal(), RDA::GetLeoVioleta(), RDA::GameMode::LocalVersus, RDA::StageId::Sitio);
    RDA::InputFrame Empty{};
    for (int i = 0; i < 105; ++i)
        World.Step(Empty, Empty);
    Check(World.Phase() == RDA::CombatPhase::Active, "IntroToActive");

    // Walk fighters close together
    for (int i = 0; i < 45; ++i)
    {
        RDA::InputFrame P1{};
        RDA::InputFrame P2{};
        P1.right = true;
        P2.left = true;
        World.Step(P1, P2);
    }

    const int InitialP2Health = World.fighters[1].health;
    // Trigger universal grab with simultaneous punch + kick
    {
        RDA::InputFrame P1{};
        P1.punch = true;
        P1.kick = true;
        P1.punchPressed = true;
        P1.kickPressed = true;
        World.Step(P1, Empty);
    }
    Check(World.fighters[0].currentMove != nullptr && World.fighters[0].currentMove->isGrab, "UniversalGrabStarted");

    bool bSawGrabActive = false;
    for (int i = 0; i < 60; ++i)
    {
        World.Step(Empty, Empty);
        if (World.Snapshot().grabActive)
            bSawGrabActive = true;
    }
    Check(bSawGrabActive, "UniversalGrabConnectedAndHeld");
    Check(World.fighters[1].health == InitialP2Health - 125, "UniversalGrabSlamDamage125");
    Check(World.fighters[1].state == RDA::FighterState::Knockdown ||
              World.fighters[1].state == RDA::FighterState::WakeUp,
          "UniversalGrabLeavesVictimInKnockdown");
}

void TestProjectilesSpecialsAndSuper()
{
    RDA::CombatWorld World(RDA::GetDanteSinal(), RDA::GetGutoBarba(), RDA::GameMode::Training,
                           RDA::StageId::CozinhaMacabra);
    RDA::InputFrame Empty{};
    for (int i = 0; i < 105; ++i)
        World.Step(Empty, Empty);

    // Chave Binaria: baixo, baixo-tras, tras + especial (fonte TS).
    {
        RDA::InputFrame P1{};
        P1.down = true;
        P1.downPressed = true;
        World.Step(P1, Empty);
        P1.left = true;
        P1.leftPressed = true;
        World.Step(P1, Empty);
        P1.down = false;
        P1.downPressed = false;
        P1.leftPressed = false;
        P1.special = true;
        P1.specialPressed = true;
        World.Step(P1, Empty);
    }
    bool bProjectileSpawned = false;
    for (int i = 0; i < 20; ++i)
    {
        World.Step(Empty, Empty);
        if (!World.Snapshot().projectiles.empty())
            bProjectileSpawned = true;
    }
    Check(bProjectileSpawned, "DanteProjectileSpawned");

    // Cast Super (Ponto Final)
    World.ResetTrainingPositions();
    {
        RDA::InputFrame P1{};
        P1.down = true;
        P1.special = true;
        P1.specialPressed = true;
        P1.superPressed = true;
        World.Step(P1, Empty);
    }
    Check(World.fighters[0].currentMove != nullptr && World.fighters[0].currentMove->isSuper, "SuperStarted");
}

void TestStageFinishersAllThreeArenas()
{
    const RDA::StageId Stages[3] = {RDA::StageId::CaisDaCidade, RDA::StageId::CozinhaMacabra, RDA::StageId::Sitio};

    for (RDA::StageId Stage : Stages)
    {
        RDA::CombatWorld World(RDA::GetGutoBarba(), RDA::GetAstroRiso(), RDA::GameMode::LocalVersus, Stage);
        RDA::InputFrame Empty{};
        for (int i = 0; i < 105; ++i)
            World.Step(Empty, Empty);

        // Set P1 at 1 round win and P2 at 1 HP, then land a light attack to win the match and enter FinishReady
        World.fighters[0].roundWins = 1;
        World.fighters[0].x = 300.0f;
        World.fighters[1].x = 330.0f;
        World.fighters[1].health = 10;

        {
            RDA::InputFrame P1{};
            P1.punch = true;
            P1.punchPressed = true;
            World.Step(P1, Empty);
        }
        for (int i = 0; i < 15 && World.Phase() == RDA::CombatPhase::Active; ++i)
        {
            World.Step(Empty, Empty);
        }
        Check(World.Phase() == RDA::CombatPhase::FinishReady, "EnteredFinishReady",
              RDA::GetStageDef(Stage).canonicalId);

        // Execute Universal Grab during FinishReady
        {
            RDA::InputFrame P1{};
            P1.grabPressed = true;
            P1.punchPressed = true;
            P1.kick = true;
            World.Step(P1, Empty);
        }
        // A captura é avaliada pela janela ativa canônica (startup de 6f).
        for (int f = 0; f < 12 && World.Phase() == RDA::CombatPhase::FinishReady; ++f)
            World.Step(Empty, Empty);
        Check(World.Phase() == RDA::CombatPhase::StageFinish, "EnteredStageFinish",
              RDA::GetStageDef(Stage).canonicalId);

        for (int i = 0; i < RDA::kFinishEndFrame + 5; ++i)
        {
            World.Step(Empty, Empty);
        }
        const RDA::WorldSnapshot Snap = World.Snapshot();
        Check(Snap.phase == RDA::CombatPhase::MatchOver && Snap.finisherCompleted, "CompletedStageFinish",
              RDA::GetStageDef(Stage).canonicalId);
        Check(std::strcmp(Snap.banner, RDA::GetStageDef(Stage).finisherBanner) == 0, "StageFinisherBannerMatches",
              Snap.banner);
    }
}

void TestDeterminismAndBenchmark()
{
    auto RunSim = [](int Steps) -> uint64_t {
        RDA::CombatWorld W(RDA::GetRafaMare(), RDA::GetNoirReflexo(), RDA::GameMode::VsCpu, RDA::StageId::CaisDaCidade);
        for (int s = 0; s < Steps; ++s)
        {
            const RDA::InputFrame P1 = RDA::BuildCpuInput(W, 0, RDA::AiDifficulty::Hard);
            const RDA::InputFrame P2 = RDA::BuildCpuInput(W, 1, RDA::AiDifficulty::Normal);
            W.Step(P1, P2);
        }
        return W.ComputeStateHash();
    };

    const uint64_t HashA = RunSim(600);
    const uint64_t HashB = RunSim(600);
    Check(HashA == HashB && HashA != 0, "Deterministic600FramesHash");

    const auto Start = std::chrono::steady_clock::now();
    const uint64_t BenchHash = RunSim(10000);
    const auto End = std::chrono::steady_clock::now();
    const double ElapsedMs = std::chrono::duration<double, std::milli>(End - Start).count();
    std::printf("BENCHMARK: 10000 ticks headless in %.2f ms (%.5f ms/tick) hash=%016llx\n", ElapsedMs,
                ElapsedMs / 10000.0, static_cast<unsigned long long>(BenchHash));
    Check(ElapsedMs < 250.0, "HeadlessBenchmarkUnderBudget");
}

void TestAttackSoundOwnership()
{
    CombatWorld world(GetGutoBarba(), GetRafaMare(), GameMode::Training);
    InputFrame empty{};
    for (int frame = 0; frame < 105; ++frame)
        world.Step(empty, empty);
    world.DrainSounds();
    world.DrainSoundCues();
    InputFrame p1{}, p2{};
    p1.punch = p1.punchPressed = true;
    p2.special = p2.specialPressed = true;
    world.Step(p1, p2);
    const uint64_t hash = world.ComputeStateHash();
    auto cues = world.DrainSoundCues();
    auto legacy = world.DrainSounds();
    bool valid = cues.size() == 2 && legacy.size() == 2;
    if (valid)
        valid = cues[0].fighterIndex == 0 && cues[0].event == SoundEvent::Swing &&
                std::strcmp(cues[0].moveId, "elbow") == 0 && cues[1].fighterIndex == 1 &&
                cues[1].event == SoundEvent::SpecialCast && std::strcmp(cues[1].moveId, "maoDaMare") == 0;
    valid = valid && world.DrainSoundCues().empty() && world.ComputeStateHash() == hash;
    for (int frame = 0; frame < 100; ++frame)
        world.Step(empty, empty);
    world.DrainSoundCues();
    p2 = {};
    p2.superPressed = true;
    world.Step(empty, p2);
    cues = world.DrainSoundCues();
    valid = valid && cues.size() == 1 && cues[0].event == SoundEvent::SuperCast && cues[0].fighterIndex == 1 &&
            std::strcmp(cues[0].moveId, "chuteRessaca") == 0;
    Check(valid, "AttackSoundOwnershipAndIndependentDrain");
}

void TestCpuCanonicalSpecialCommands()
{
    bool all = true;
    for (int id = 0; id < kFighterCount; ++id)
        for (bool super : {false, true})
        {
            CombatWorld world(GetAstroRiso(), GetFighterDef(static_cast<FighterId>(id)),
                              super ? GameMode::Training : GameMode::LocalVersus);
            world.fighters[0].x = 300;
            world.fighters[1].x = 350;
            InputFrame idle{};
            bool special = false, usedShortcut = false, cached = true;
            for (int step = 0; step < 2400 && !special; ++step)
            {
                const InputFrame input = BuildCpuInput(world, 1, AiDifficulty::Hard);
                const InputFrame repeated = BuildCpuInput(world, 1, AiDifficulty::Hard);
                cached = cached && input.special == repeated.special &&
                         input.specialPressed == repeated.specialPressed && input.left == repeated.left &&
                         input.down == repeated.down;
                usedShortcut = usedShortcut || input.superPressed || input.grabPressed;
                world.Step(idle, input);
                const auto *move = world.fighters[1].currentMove;
                special = move && move->isSpecial && move->isSuper == super;
            }
            std::printf("CPU CANONICAL COMMAND fighter=%s type=%s started=%s shortcut=%s cached=%s\n",
                        GetFighterDef(static_cast<FighterId>(id)).canonicalId, super ? "super" : "special",
                        special ? "yes" : "no", usedShortcut ? "yes" : "no", cached ? "yes" : "no");
            all = all && special && !usedShortcut && cached;
        }
    Check(all, "CpuSixFightersStartCanonicalSpecialsAndSupersWithoutShortcuts");
}
} // namespace

int main(int argc, char **argv)
{
    if (argc > 1 && std::strcmp(argv[1], "--parity") == 0)
    {
        std::ifstream fixture(FindGoldenFixturesPath(), std::ios::binary);
        uint64_t fnv = 0xcbf29ce484222325ULL;
        char byte;
        while (fixture.get(byte))
        {
            fnv ^= static_cast<unsigned char>(byte);
            fnv *= 0x100000001b3ULL;
        }
        Check(fnv == kFixtureFnv, "CompiledReplayMatchesFixture",
              "Execute python Scripts/generate-canonical-data.py e recompile");
        TestCanonicalReplays();
        return gFailCount ? 1 : 0;
    }
    TestCatalogAndGoldenFixturesJson();
    TestUniversalGrabAndTech();
    TestProjectilesSpecialsAndSuper();
    TestStageFinishersAllThreeArenas();
    TestDeterminismAndBenchmark();
    TestAttackSoundOwnership();
    TestCpuCanonicalSpecialCommands();

    std::printf("RDA CORE TESTS: %d passed, %d failed\n", gPassCount, gFailCount);
    return gFailCount == 0 ? 0 : 1;
}
