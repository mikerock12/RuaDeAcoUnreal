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
	namespace fs = std::filesystem;

	int gPassCount = 0;
	int gFailCount = 0;

	void Check(bool Cond, const char* Name, const char* Detail = "")
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

	fs::path FindGoldenFixturesPath()
	{
		fs::path Cur = fs::current_path();
		for (int i = 0; i < 6; ++i)
		{
			const fs::path Candidate = Cur / "Data" / "generated" / "golden_fixtures.json";
			if (fs::exists(Candidate)) return Candidate;
			if (!Cur.has_parent_path()) break;
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

		const char* ExpectedFighters[RDA::kFighterCount] = {
			"guto-barba",
			"rafa-mare",
			"noir-reflexo",
			"astro-riso",
			"dante-sinal",
			"leo-violeta"
		};

		for (int i = 0; i < RDA::kFighterCount; ++i)
		{
			const RDA::FighterDef* Def = RDA::FindFighterByCanonicalId(ExpectedFighters[i]);
			Check(Def != nullptr, "FighterInCatalog", ExpectedFighters[i]);
			Check(Content.find( std::string("\"id\": \"") + ExpectedFighters[i] + "\"") != std::string::npos,
				"FighterInGoldenJson", ExpectedFighters[i]);
		}

		// Verify canonical stats match golden_fixtures.json
		const RDA::FighterDef& Guto = RDA::GetGutoBarba();
		Check(Guto.stats.maxHealth == 1250 && Near(Guto.stats.walkSpeed, 2.47f) && Near(Guto.stats.jumpSpeed, 12.8f),
			"GutoCanonicalStats");

		const RDA::FighterDef& Rafa = RDA::GetRafaMare();
		Check(Rafa.stats.maxHealth == 1000 && Near(Rafa.stats.walkSpeed, 4.26f) && Near(Rafa.stats.jumpSpeed, 15.6f),
			"RafaCanonicalStats");

		const RDA::FighterDef& Noir = RDA::GetNoirReflexo();
		Check(Noir.stats.maxHealth == 980 && Near(Noir.stats.walkSpeed, 3.74f) && Near(Noir.stats.jumpSpeed, 14.4f),
			"NoirCanonicalStats");

		const RDA::FighterDef& Astro = RDA::GetAstroRiso();
		Check(Astro.stats.maxHealth == 930 && Near(Astro.stats.walkSpeed, 4.72f) && Near(Astro.stats.jumpSpeed, 15.4f),
			"AstroCanonicalStats");

		const RDA::FighterDef& Dante = RDA::GetDanteSinal();
		Check(Dante.stats.maxHealth == 960 && Near(Dante.stats.walkSpeed, 4.03f) && Near(Dante.stats.jumpSpeed, 14.8f),
			"DanteCanonicalStats");

		const RDA::FighterDef& Leo = RDA::GetLeoVioleta();
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
		for (int i = 0; i < 105; ++i) World.Step(Empty, Empty);
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
			if (World.Snapshot().grabActive) bSawGrabActive = true;
		}
		Check(bSawGrabActive, "UniversalGrabConnectedAndHeld");
		Check(World.fighters[1].health == InitialP2Health - 125, "UniversalGrabSlamDamage125");
		Check(World.fighters[1].state == RDA::FighterState::Knockdown || World.fighters[1].state == RDA::FighterState::WakeUp,
			"UniversalGrabLeavesVictimInKnockdown");
	}

	void TestProjectilesSpecialsAndSuper()
	{
		RDA::CombatWorld World(RDA::GetDanteSinal(), RDA::GetGutoBarba(), RDA::GameMode::Training, RDA::StageId::CozinhaMacabra);
		RDA::InputFrame Empty{};
		for (int i = 0; i < 105; ++i) World.Step(Empty, Empty);

		// Cast Chave Binaria projectile
		{
			RDA::InputFrame P1{};
			P1.special = true;
			P1.specialPressed = true;
			World.Step(P1, Empty);
		}
		bool bProjectileSpawned = false;
		for (int i = 0; i < 20; ++i)
		{
			World.Step(Empty, Empty);
			if (!World.Snapshot().projectiles.empty()) bProjectileSpawned = true;
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
		const RDA::StageId Stages[3] = {
			RDA::StageId::CaisDaCidade,
			RDA::StageId::CozinhaMacabra,
			RDA::StageId::Sitio
		};

		for (RDA::StageId Stage : Stages)
		{
			RDA::CombatWorld World(RDA::GetGutoBarba(), RDA::GetAstroRiso(), RDA::GameMode::LocalVersus, Stage);
			RDA::InputFrame Empty{};
			for (int i = 0; i < 105; ++i) World.Step(Empty, Empty);

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
			Check(World.Phase() == RDA::CombatPhase::FinishReady, "EnteredFinishReady", RDA::GetStageDef(Stage).canonicalId);

			// Execute Universal Grab during FinishReady
			{
				RDA::InputFrame P1{};
				P1.grabPressed = true;
				P1.punchPressed = true;
				P1.kick = true;
				World.Step(P1, Empty);
			}
			Check(World.Phase() == RDA::CombatPhase::StageFinish, "EnteredStageFinish", RDA::GetStageDef(Stage).canonicalId);

			for (int i = 0; i < RDA::kFinishEndFrame + 5; ++i)
			{
				World.Step(Empty, Empty);
			}
			const RDA::WorldSnapshot Snap = World.Snapshot();
			Check(Snap.phase == RDA::CombatPhase::MatchOver && Snap.finisherCompleted,
				"CompletedStageFinish", RDA::GetStageDef(Stage).canonicalId);
			Check(std::strcmp(Snap.banner, RDA::GetStageDef(Stage).finisherBanner) == 0,
				"StageFinisherBannerMatches", Snap.banner);
		}
	}

	void TestDeterminismAndBenchmark()
	{
		auto RunSim = [](int Steps) -> uint64_t
		{
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
		std::printf("BENCHMARK: 10000 ticks headless in %.2f ms (%.5f ms/tick) hash=%016llx\n",
			ElapsedMs, ElapsedMs / 10000.0, static_cast<unsigned long long>(BenchHash));
		Check(ElapsedMs < 250.0, "HeadlessBenchmarkUnderBudget");
	}
}

int main()
{
	TestCatalogAndGoldenFixturesJson();
	TestUniversalGrabAndTech();
	TestProjectilesSpecialsAndSuper();
	TestStageFinishersAllThreeArenas();
	TestDeterminismAndBenchmark();

	std::printf("RDA GOLDEN TESTS: %d passed, %d failed\n", gPassCount, gFailCount);
	return gFailCount == 0 ? 0 : 1;
}
