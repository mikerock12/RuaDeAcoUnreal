#include "RDACombatWorld.h"
#include <jni.h>
#include <memory>

namespace
{
	std::unique_ptr<RDA::CombatWorld> gWorld;
	int gPrevMask = 0;

	RDA::InputFrame DecodeMask(int Mask, int PrevMask)
	{
		RDA::InputFrame In{};
		In.left = (Mask & (1 << 0)) != 0;
		In.right = (Mask & (1 << 1)) != 0;
		In.up = (Mask & (1 << 2)) != 0;
		In.down = (Mask & (1 << 3)) != 0;
		In.punch = (Mask & (1 << 4)) != 0;
		In.kick = (Mask & (1 << 5)) != 0;
		In.special = (Mask & (1 << 6)) != 0;
		In.block = (Mask & (1 << 7)) != 0;

		const int NewlyPressed = Mask & ~PrevMask;
		In.leftPressed = (NewlyPressed & (1 << 0)) != 0;
		In.rightPressed = (NewlyPressed & (1 << 1)) != 0;
		In.upPressed = (NewlyPressed & (1 << 2)) != 0;
		In.downPressed = (NewlyPressed & (1 << 3)) != 0;
		In.punchPressed = (NewlyPressed & (1 << 4)) != 0;
		In.kickPressed = (NewlyPressed & (1 << 5)) != 0;
		In.specialPressed = (NewlyPressed & (1 << 6)) != 0;
		In.blockPressed = (NewlyPressed & (1 << 7)) != 0;
		In.grabPressed = (NewlyPressed & (1 << 8)) != 0 || (In.punch && In.kick && (In.punchPressed || In.kickPressed));
		In.superPressed = (NewlyPressed & (1 << 9)) != 0;
		In.pausePressed = (NewlyPressed & (1 << 10)) != 0;
		return In;
	}
}

extern "C" JNIEXPORT void JNICALL
Java_com_smellsliketech_ruadeaco_MainActivity_nativeInitMatch(
	JNIEnv*, jobject, jint p1Id, jint p2Id, jint modeId, jint stageId)
{
	const RDA::FighterDef& P1 = RDA::GetFighterDef(static_cast<RDA::FighterId>(p1Id));
	const RDA::FighterDef& P2 = RDA::GetFighterDef(static_cast<RDA::FighterId>(p2Id));
	const RDA::GameMode Mode = static_cast<RDA::GameMode>(modeId);
	const RDA::StageId Stage = static_cast<RDA::StageId>(stageId);
	gWorld = std::make_unique<RDA::CombatWorld>(P1, P2, Mode, Stage);
	gPrevMask = 0;
}

extern "C" JNIEXPORT void JNICALL
Java_com_smellsliketech_ruadeaco_MainActivity_nativeStep(
	JNIEnv* env, jobject, jint p1Mask, jint difficulty, jfloatArray outState)
{
	if (!gWorld)
	{
		gWorld = std::make_unique<RDA::CombatWorld>(
			RDA::GetGutoBarba(), RDA::GetRafaMare(), RDA::GameMode::VsCpu, RDA::StageId::CaisDaCidade);
	}

	const RDA::InputFrame P1 = DecodeMask(p1Mask, gPrevMask);
	gPrevMask = p1Mask;
	if (P1.pausePressed)
	{
		gWorld->TogglePause();
	}

	const RDA::InputFrame P2 = RDA::BuildCpuInput(*gWorld, 1, static_cast<RDA::AiDifficulty>(difficulty));
	gWorld->Step(P1, P2);
	gWorld->DrainSounds();

	const RDA::WorldSnapshot Snap = gWorld->Snapshot();
	jfloat Buf[24] = {
		static_cast<jfloat>(Snap.frame),
		static_cast<jfloat>(Snap.phase),
		static_cast<jfloat>(Snap.phaseFrame),
		static_cast<jfloat>(Snap.round),
		static_cast<jfloat>(Snap.timeSeconds),
		Snap.paused ? 1.0f : 0.0f,
		static_cast<jfloat>(Snap.stage),
		Snap.fighters[0].x,
		Snap.fighters[0].y,
		static_cast<jfloat>(Snap.fighters[0].facing),
		static_cast<jfloat>(Snap.fighters[0].anim),
		static_cast<jfloat>(Snap.fighters[0].stateFrame),
		static_cast<jfloat>(Snap.fighters[0].health),
		static_cast<jfloat>(Snap.fighters[0].maxHealth),
		static_cast<jfloat>(Snap.fighters[0].meter),
		static_cast<jfloat>(Snap.fighters[0].roundWins),
		Snap.fighters[1].x,
		Snap.fighters[1].y,
		static_cast<jfloat>(Snap.fighters[1].facing),
		static_cast<jfloat>(Snap.fighters[1].anim),
		static_cast<jfloat>(Snap.fighters[1].stateFrame),
		static_cast<jfloat>(Snap.fighters[1].health),
		static_cast<jfloat>(Snap.fighters[1].maxHealth),
		static_cast<jfloat>(Snap.fighters[1].meter),
	};
	env->SetFloatArrayRegion(outState, 0, 24, Buf);
}
