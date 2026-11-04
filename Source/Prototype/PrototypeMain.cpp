#include "RDACombatWorld.h"
#include "raylib.h"
#if defined(_WIN32)
#include "WindowsBranding.h"
#include "OnlineWindow.h"
#endif
#include "CanonicalPresentation.h"
#include "RDAStageVisuals.h"
#include "RDAPlatformAssets.h"
#include "GamePreferences.h"
#include "MobileServices.h"
#if defined(PLATFORM_ANDROID)
extern "C" JNIEXPORT void JNICALL Java_com_smellsliketech_ruadeaco_MainActivity_nativeSetPaused(JNIEnv*,jclass,jboolean paused){SLTMobile::SetBackground(paused);}
#endif
#include "rlgl.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

namespace
{
	namespace fs = std::filesystem;
	SLTPrefs::Preferences* Preferences=nullptr;
    float CameraTrauma=0;
    bool AssetLoading=false;
    double LastLoadingPaint=0;

	constexpr Color kInk{5, 7, 17, 255};
	constexpr Color kPanel{12, 18, 36, 235};
	constexpr Color kCyan{41, 217, 255, 255};
	constexpr Color kPink{246, 64, 112, 255};
	constexpr Color kYellow{255, 213, 92, 255};
	constexpr Color kWhite{247, 242, 208, 255};
	constexpr Color kMuted{115, 130, 155, 255};
	constexpr Color kDanger{228, 63, 90, 255};
	constexpr Color kGreen{92, 242, 155, 255};

	enum class AppScreen
	{
		MainMenu = 0,
		CharacterSelect,
		Match,
	};

	struct SpriteSheet
	{
		Texture2D texture{};
		int frameSize = 256;
		int frames = 1;
		int fps = 8;
		bool loop = false;
		float offsetX = 0, offsetY = 0, sole = 6;
	};

	struct AnimSpec
	{
		RDA::AnimId id;
		const char* file;
		int fps;
		bool loop;
	};

	constexpr AnimSpec kCommonSpecs[] = {
		{RDA::AnimId::Idle, "idle.png", 6, true},
		{RDA::AnimId::Walk, "corrida.png", 9, true},
		{RDA::AnimId::WalkBackward, "walk-backward.png", 7, true},
		{RDA::AnimId::JumpNeutral, "jump-neutral.png", 7, false},
		{RDA::AnimId::JumpForward, "jump-forward.png", 7, false},
		{RDA::AnimId::JumpBackward, "jump-backward.png", 7, false},
		{RDA::AnimId::Fall, "fall.png", 7, false},
		{RDA::AnimId::Landing, "landing.png", 10, false},
		{RDA::AnimId::Crouch, "crouch.png", 6, false},
		{RDA::AnimId::StandingLight, "standing-light.png", 11, false},
		{RDA::AnimId::StandingHeavy, "standing-heavy.png", 8, false},
		{RDA::AnimId::ForwardLight, "forward-light.png", 10, false},
		{RDA::AnimId::ForwardHeavy, "forward-heavy.png", 8, false},
		{RDA::AnimId::CrouchLight, "crouch-light.png", 10, false},
		{RDA::AnimId::CrouchHeavy, "crouch-heavy.png", 8, false},
		{RDA::AnimId::AirLight, "air-light-neutral.png", 10, false},
		{RDA::AnimId::AirHeavy, "air-heavy-neutral.png", 8, false},
		{RDA::AnimId::BlockStanding, "block-standing.png", 7, false},
		{RDA::AnimId::BlockCrouching, "block-crouching.png", 7, false},
		{RDA::AnimId::Hit, "hit.png", 10, false},
		{RDA::AnimId::Knockdown, "knockdown.png", 6, false},
		{RDA::AnimId::WakeUp, "wake-up.png", 8, false},
		{RDA::AnimId::Knockout, "knockout.png", 5, false},
		{RDA::AnimId::Victory, "victory.png", 6, true},
		{RDA::AnimId::UniversalGrab, "universal-grab-v2.png", 12, false},
		{RDA::AnimId::GrabbedFront, "grabbed-front.png", 8, false},
		{RDA::AnimId::GrabbedLifted, "grabbed-lifted.png", 8, false},
		{RDA::AnimId::Thrown, "thrown.png", 8, false},
		{RDA::AnimId::Frozen, "frozen.png", 6, true},
	};

	struct FighterVisualBank
	{
		Texture2D portrait{};
		std::array<SpriteSheet, static_cast<size_t>(RDA::AnimId::Count)> sheets{};
		std::unordered_map<std::string, SpriteSheet> animations;
		std::vector<std::pair<CanonicalVisual::Effect, Texture2D>> effects;
	};

	struct StageVisualBank
	{
		Texture2D background{};
		Texture2D finisherProp1{};
		Texture2D finisherProp2{};
		Music music{};
		std::vector<unsigned char> musicData;
		bool hasMusic = false;
	};

	struct SoundBank
	{
		bool audioReady = false;
		Sound fight{};
		Sound swing{};
		Sound hitLight{};
		Sound hitHeavy{};
		Sound block{};
		Sound jump{};
		Sound knockdown{};
		Sound roundEnd{};
		Sound matchWin{};
		Sound finalize{};
		Sound monsterRoar{};
		std::array<Sound, RDA::kFighterCount> specialSfx{};
		std::array<Sound, RDA::kFighterCount> superSfx{};
		std::unordered_map<std::string,std::pair<Sound,Sound>> moveSounds;
        std::unordered_map<std::string,std::pair<float,float>> moveGains;

		void Unload()
		{
			if (!audioReady) return;
			UnloadSound(fight);
			UnloadSound(swing);
			UnloadSound(hitLight);
			UnloadSound(hitHeavy);
			UnloadSound(block);
			UnloadSound(jump);
			UnloadSound(knockdown);
			UnloadSound(roundEnd);
			UnloadSound(matchWin);
			UnloadSound(finalize);
			UnloadSound(monsterRoar);
			for (auto& Entry : moveSounds) { UnloadSound(Entry.second.first); UnloadSound(Entry.second.second); }
			for (int i = 0; i < RDA::kFighterCount; ++i)
			{
				UnloadSound(specialSfx[static_cast<size_t>(i)]);
				UnloadSound(superSfx[static_cast<size_t>(i)]);
			}
		}
	};

	Sound MakeTone(float FreqStart, float FreqEnd, float Seconds, float Volume, float NoiseMix = 0.0f)
	{
		constexpr int SampleRate = 22050;
		const int SampleCount = std::max(1, static_cast<int>(Seconds * SampleRate));
		std::vector<short> Pcm(static_cast<size_t>(SampleCount));
		float Phase = 0.0f;
		unsigned int Seed = 0x13579BDFU;

		for (int i = 0; i < SampleCount; ++i)
		{
			const float T = static_cast<float>(i) / static_cast<float>(SampleCount);
			const float Env = (1.0f - T) * (1.0f - T);
			const float Freq = FreqStart + (FreqEnd - FreqStart) * T;
			Phase += (2.0f * 3.14159265f * Freq) / static_cast<float>(SampleRate);
			const float Square = std::sin(Phase) >= 0.0f ? 0.8f : -0.8f;
			Seed = Seed * 1664525U + 1013904223U;
			const float Noise = (static_cast<float>((Seed >> 16) & 0x7FFFU) / 16384.0f) - 1.0f;
			const float Sample = ((1.0f - NoiseMix) * Square + NoiseMix * Noise) * Env * Volume;
			Pcm[static_cast<size_t>(i)] = static_cast<short>(std::clamp(Sample, -1.0f, 1.0f) * 32000.0f);
		}

		Wave WaveData{};
		WaveData.frameCount = static_cast<unsigned int>(SampleCount);
		WaveData.sampleRate = SampleRate;
		WaveData.sampleSize = 16;
		WaveData.channels = 1;
		WaveData.data = MemAlloc(static_cast<unsigned int>(SampleCount * sizeof(short)));
		std::memcpy(WaveData.data, Pcm.data(), static_cast<size_t>(SampleCount) * sizeof(short));
		Sound Out = LoadSoundFromWave(WaveData);
		UnloadWave(WaveData);
		return Out;
	}

	Sound LoadSfxOrFallback(const fs::path& Path, float FreqStart, float FreqEnd, float Seconds, float Volume, float NoiseMix = 0.0f)
	{
		if (RDAAssets::Exists(Path))
		{
			Sound S = LoadSound(Path.generic_string().c_str());
			if (S.frameCount > 0)
			{
				return S;
			}
		}
		return MakeTone(FreqStart, FreqEnd, Seconds, Volume, NoiseMix);
	}

	SoundBank BuildSoundBank(const fs::path& ContentRoot)
	{
		SoundBank Bank{};
		if (!IsAudioDeviceReady())
		{
			return Bank;
		}
		Bank.audioReady = true;
		const fs::path SfxRoot = ContentRoot / "RuaDeAco" / "Audio" / "Sfx";

		Bank.fight = LoadSfxOrFallback(SfxRoot / "locutor" / "fight.ogg", 520.0f, 780.0f, 0.22f, 0.35f, 0.10f);
		Bank.swing = LoadSfxOrFallback(SfxRoot / "combate" / "golpe-ar.ogg", 320.0f, 150.0f, 0.07f, 0.20f, 0.45f);
		Bank.hitLight = LoadSfxOrFallback(SfxRoot / "combate" / "soco-leve.ogg", 260.0f, 95.0f, 0.09f, 0.40f, 0.35f);
		Bank.hitHeavy = LoadSfxOrFallback(SfxRoot / "combate" / "soco-forte.ogg", 160.0f, 52.0f, 0.16f, 0.50f, 0.45f);
		Bank.block = LoadSfxOrFallback(SfxRoot / "combate" / "bloqueio.ogg", 680.0f, 420.0f, 0.07f, 0.28f, 0.15f);
		Bank.jump = MakeTone(210.0f, 460.0f, 0.08f, 0.18f, 0.05f);
		Bank.knockdown = LoadSfxOrFallback(SfxRoot / "combate" / "ko-impacto.ogg", 110.0f, 38.0f, 0.24f, 0.52f, 0.55f);
		Bank.roundEnd = LoadSfxOrFallback(SfxRoot / "locutor" / "ko.ogg", 440.0f, 220.0f, 0.32f, 0.35f, 0.10f);
		Bank.matchWin = LoadSfxOrFallback(SfxRoot / "locutor" / "vitoria.ogg", 330.0f, 660.0f, 0.40f, 0.38f, 0.05f);
		Bank.finalize = LoadSfxOrFallback(SfxRoot / "locutor" / "finalize.ogg", 300.0f, 580.0f, 0.30f, 0.40f, 0.15f);
		Bank.monsterRoar = LoadSfxOrFallback(SfxRoot / "finalizacao" / "monstro-rugido.ogg", 95.0f, 42.0f, 0.45f, 0.55f, 0.60f);

		for (int i = 0; i < RDA::kFighterCount; ++i)
		{
			const RDA::FighterDef& Def = RDA::GetFighterDef(static_cast<RDA::FighterId>(i));
			Bank.specialSfx[static_cast<size_t>(i)] = LoadSfxOrFallback(
				SfxRoot / "especiais" / Def.specialSfxFile, 380.0f, 620.0f, 0.18f, 0.38f, 0.20f);
			Bank.superSfx[static_cast<size_t>(i)] = LoadSfxOrFallback(
				SfxRoot / "especiais" / Def.superSfxFile, 240.0f, 760.0f, 0.32f, 0.48f, 0.25f);
		}
		for (const auto& Cue:CanonicalVisual::sounds)
		{
			Sound Voice=LoadSfxOrFallback(SfxRoot/Cue.voice,380,620,0.18f,0.38f,0.2f);
			Sound Effect=LoadSfxOrFallback(SfxRoot/Cue.effect,240,760,0.32f,0.48f,0.25f);
			SetSoundVolume(Voice,Cue.voiceGain); SetSoundVolume(Effect,Cue.effectGain); SetSoundPitch(Effect,Cue.rate);
			Bank.moveSounds.emplace(std::string(Cue.fighter)+"/"+Cue.move,std::make_pair(Voice,Effect));
            Bank.moveGains.emplace(std::string(Cue.fighter)+"/"+Cue.move,std::make_pair(Cue.voiceGain,Cue.effectGain));
		}
		return Bank;
	}

	void PlayWorldSound(const SoundBank& Bank, RDA::SoundEvent Ev, RDA::FighterId ActiveFighter)
	{
        if(Ev==RDA::SoundEvent::HitHeavy||Ev==RDA::SoundEvent::GrabSlam) {
            CameraTrauma=.20f;
            if(Preferences&&Preferences->value.vibration)SLTMobile::Call("vibrateCombat");
        }

		if (!Bank.audioReady) return;
		const size_t FIdx = static_cast<size_t>(ActiveFighter) % RDA::kFighterCount;
		switch (Ev)
		{
		case RDA::SoundEvent::Fight: (SetSoundVolume(Bank.fight,Preferences->value.voice),PlaySound(Bank.fight)); break;
		case RDA::SoundEvent::Swing: (SetSoundVolume(Bank.swing,Preferences->value.effects),PlaySound(Bank.swing)); break;
		case RDA::SoundEvent::HitLight: (SetSoundVolume(Bank.hitLight,Preferences->value.effects),PlaySound(Bank.hitLight)); break;
		case RDA::SoundEvent::HitHeavy: (SetSoundVolume(Bank.hitHeavy,Preferences->value.effects),PlaySound(Bank.hitHeavy)); break;
		case RDA::SoundEvent::Block: (SetSoundVolume(Bank.block,Preferences->value.effects),PlaySound(Bank.block)); break;
		case RDA::SoundEvent::Jump: (SetSoundVolume(Bank.jump,Preferences->value.effects),PlaySound(Bank.jump)); break;
		case RDA::SoundEvent::Knockdown:
		case RDA::SoundEvent::GrabSlam: (SetSoundVolume(Bank.knockdown,Preferences->value.effects),PlaySound(Bank.knockdown)); break;
		case RDA::SoundEvent::GrabStart: (SetSoundVolume(Bank.hitLight,Preferences->value.effects),PlaySound(Bank.hitLight)); break;
		case RDA::SoundEvent::SpecialCast: (SetSoundVolume(Bank.specialSfx[FIdx],Preferences->value.effects),PlaySound(Bank.specialSfx[FIdx])); break;
		case RDA::SoundEvent::SuperCast: (SetSoundVolume(Bank.superSfx[FIdx],Preferences->value.effects),PlaySound(Bank.superSfx[FIdx])); break;
		case RDA::SoundEvent::FinishReady: (SetSoundVolume(Bank.finalize,Preferences->value.voice),PlaySound(Bank.finalize)); break;
		case RDA::SoundEvent::FinisherMonsterRoar:
		case RDA::SoundEvent::FinisherPotDrop:
		case RDA::SoundEvent::FinisherTrident: (SetSoundVolume(Bank.monsterRoar,Preferences->value.effects),PlaySound(Bank.monsterRoar)); break;
		case RDA::SoundEvent::FinisherSplash: (SetSoundVolume(Bank.hitHeavy,Preferences->value.effects),PlaySound(Bank.hitHeavy)); break;
		case RDA::SoundEvent::RoundEnd: (SetSoundVolume(Bank.roundEnd,Preferences->value.voice),PlaySound(Bank.roundEnd)); break;
		case RDA::SoundEvent::MatchWin: (SetSoundVolume(Bank.matchWin,Preferences->value.voice),PlaySound(Bank.matchWin)); break;
		default: break;
		}
	}
	void PlayMoveSound(const SoundBank& Bank, const RDA::SoundCue& Cue, const RDA::CombatWorld& World)
	{
		if (!Bank.audioReady || Cue.event==RDA::SoundEvent::Swing) return;
		const std::string Key=std::string(World.fighters[static_cast<size_t>(Cue.fighterIndex)].def->canonicalId)+"/"+Cue.moveId;
		auto Found=Bank.moveSounds.find(Key);
		if(Found!=Bank.moveSounds.end()) { auto Gains=Bank.moveGains.at(Key);SetSoundVolume(Found->second.first,Preferences->value.voice*Gains.first);SetSoundVolume(Found->second.second,Preferences->value.effects*Gains.second);PlaySound(Found->second.first); PlaySound(Found->second.second); }
	}

	fs::path FindContentRoot()
	{
#if defined(PLATFORM_ANDROID)
		return fs::path("Content");
#endif
		fs::path Cur = fs::current_path();
		for (int Depth = 0; Depth < 6; ++Depth)
		{
			if (fs::exists(Cur / "Content" / "RuaDeAco"))
			{
				return Cur / "Content";
			}
			if (fs::exists(Cur / "RuaDeAco" / "Art"))
			{
				return Cur;
			}
			if (!Cur.has_parent_path()) break;
			Cur = Cur.parent_path();
		}
		if (const char* AppDir = GetApplicationDirectory())
		{
			fs::path ExeDir(AppDir);
			for (int Depth = 0; Depth < 6; ++Depth)
			{
				if (fs::exists(ExeDir / "Content" / "RuaDeAco"))
				{
					return ExeDir / "Content";
				}
				if (fs::exists(ExeDir / "RuaDeAco" / "Art"))
				{
					return ExeDir;
				}
				if (!ExeDir.has_parent_path()) break;
				ExeDir = ExeDir.parent_path();
			}
		}
		return fs::path("Content");
	}

	Texture2D LoadPixelTexture(const fs::path& Path)
	{
		if (!RDAAssets::Exists(Path))
		{
			return Texture2D{};
		}
		Texture2D Tex = LoadTexture(Path.generic_string().c_str());
        if(AssetLoading && GetTime()-LastLoadingPaint>.045) {
            BeginDrawing();ClearBackground(kInk);DrawText("CARREGANDO LUTADORES...",GetScreenWidth()/2-155,GetScreenHeight()/2,22,kCyan);EndDrawing();LastLoadingPaint=GetTime();
        }
		if (Tex.id != 0)
		{
			SetTextureFilter(Tex, TEXTURE_FILTER_POINT);
		}
		return Tex;
	}

	FighterVisualBank LoadFighterVisuals(const fs::path& ArtRoot, const RDA::FighterDef& Def, bool LoadAnimations = true)
	{
		FighterVisualBank Out{};
		Out.portrait = LoadPixelTexture(ArtRoot / "Portraits" / Def.portraitFile);
		if (!LoadAnimations) return Out;
		const fs::path CharDir = ArtRoot / "Characters" / Def.assetFolder;

		for (const auto& Spec : CanonicalVisual::animations)
		{
			if (std::strcmp(Spec.fighter, Def.canonicalId) != 0) continue;
			SpriteSheet& Sheet = Out.animations[Spec.id];
			Sheet.frameSize = Spec.size;
			Sheet.fps = Spec.fps;
			Sheet.loop = Spec.loop;
			Sheet.frames = Spec.frames;
			Sheet.offsetX = Spec.x;
			Sheet.offsetY = Spec.y;
			Sheet.sole = Spec.sole;
			Sheet.texture = LoadPixelTexture(CharDir / Spec.file);
			if (Sheet.texture.id == 0) TraceLog(LOG_ERROR, "Missing canonical animation: %s/%s", Def.canonicalId, Spec.file);
		}
		const char* Names[] = {"idle", "walk", "walkBackward", "jumpNeutral", "jumpForward", "jumpBackward", "fall", "landing", "crouch", "standingLight", "standingHeavy", "forwardLight", "forwardHeavy", "crouchLight", "crouchHeavy", "airLightNeutral", "airHeavyNeutral", "blockStanding", "blockCrouching", "hit", "knockdown", "wakeUp", "knockout", "victory", "universalGrab", "grabbedFront", "grabbedLifted", "thrown", "frozen", "special1", "special2", "special3"};
		for (size_t i = 0; i < static_cast<size_t>(RDA::AnimId::Count); ++i) Out.sheets[i] = Out.animations.at(Names[i]);
		for (const auto& Effect : CanonicalVisual::effects)
			if (std::strcmp(Effect.fighter, Def.canonicalId) == 0) Out.effects.emplace_back(Effect, LoadPixelTexture(CharDir / Effect.file));
		TraceLog(LOG_INFO,"RDA_ASSETS: %s animations=%d",Def.canonicalId,static_cast<int>(Out.animations.size()));
		return Out;
	}

	void DrawSpriteSheet(const SpriteSheet& Sheet, float WorldX, float WorldY, int Facing, int Frame, int SolePad, float RotationRad = 0.0f, float Scale = 1.0f)
	{
		if (Sheet.texture.id == 0) return;
		const int ClampedFrame = std::clamp(Frame, 0, std::max(0, Sheet.frames - 1));
		const float Size = static_cast<float>(Sheet.frameSize);
		Rectangle Source{
			static_cast<float>(ClampedFrame * Sheet.frameSize),
			0.0f,
			Facing >= 0 ? Size : -Size,
			Size
		};
		const float DestSize = Size * Scale;
		Rectangle Dest{std::round(WorldX), std::round(WorldY + static_cast<float>(SolePad) * Scale), DestSize, DestSize};
		DrawTexturePro(Sheet.texture, Source, Dest, Vector2{DestSize * 0.5f, DestSize}, RotationRad * (180.0f / 3.14159265f), WHITE);
	}

	void DrawFighterOnStage(
		const FighterVisualBank& Bank,
		const RDA::FighterDef& Def,
		const RDA::FighterSnapshot& Snap,
		bool DrawBoxes)
	{
		if (!Snap.visible) return;
		const int StateFrame=Snap.poseStateFrame;
		const char* MoveId=Snap.poseMoveId;
		const bool Attacking=MoveId && MoveId[0];
		DrawEllipse(static_cast<int>(Snap.x), RDA::kGroundY + 2, 26, 6, Color{0, 0, 0, 120});

		const SpriteSheet* SelectedSheet = &Bank.sheets[static_cast<size_t>(Snap.poseAnim)];
		if (Attacking && Bank.animations.count(Snap.poseMoveAnimationKey)) SelectedSheet = &Bank.animations.at(Snap.poseMoveAnimationKey);
		int LocalFrame = StateFrame, PhaseFrames = 0, ExplicitFrame = -1;
		for (const auto& Phase : CanonicalVisual::phases)
		{
			if (std::strcmp(Phase.fighter, Def.canonicalId) == 0 && std::strcmp(Phase.move, MoveId) == 0 && StateFrame >= Phase.from && StateFrame <= Phase.to)
			{
				SelectedSheet = &Bank.animations.at(Phase.animation);
				LocalFrame = StateFrame - Phase.from;
				PhaseFrames = Phase.to - Phase.from + 1;
				ExplicitFrame = Phase.frame;
			}
		}
		const SpriteSheet& Sheet = *SelectedSheet;
		int Frame = 0;
		if (Sheet.frames > 1)
		{
			if (ExplicitFrame >= 0) Frame = ExplicitFrame;
			else if (PhaseFrames > 0) Frame = std::min(Sheet.frames - 1, LocalFrame * Sheet.frames / PhaseFrames);
			else if (Attacking && Snap.poseMoveTotalFrames > 0)
			{
				RDA::MoveDef Move{};
				Move.totalFrames = Snap.poseMoveTotalFrames;
				Move.hitbox.activeFrom = Snap.poseHitActiveFrom;
				Move.hitbox.activeTo = Snap.poseHitActiveTo;
				Frame = RDA::MoveAnimFrame(Move, StateFrame, Sheet.frames);
			}
			else
			{
				const int TicksPerFrame = std::max(1, static_cast<int>(std::round(60.0f / std::max(1, Sheet.fps))));
				Frame = StateFrame / TicksPerFrame;
				Frame = Sheet.loop ? (Frame % Sheet.frames) : std::min(Sheet.frames - 1, Frame);
			}
		}
		if (Snap.poseState == RDA::FighterState::Dizzy) Frame = (StateFrame / 12) % std::min(2, Sheet.frames);
		if (Snap.victimPoseFrame >= 0) Frame = Snap.victimPoseFrame;
		if (std::strcmp(MoveId, "universalGrab") == 0)
		{
			const int Starts[] = {0,3,6,10,14,18,23,29,35,40,46,58};
			Frame = 0; for(int i=1;i<12;++i) if(StateFrame>=Starts[i]) Frame=i;
			if (!Snap.poseMoveConnected && StateFrame>9) Frame=11;
		}

		if (Snap.cutBelowY >= 0) BeginScissorMode(0,0,RDA::kLogicalWidth,static_cast<int>(Snap.cutBelowY));
		DrawSpriteSheet(Sheet, Snap.x + Sheet.offsetX * Snap.facing, Snap.y + Sheet.offsetY, Snap.facing, Frame, static_cast<int>(Sheet.sole), Snap.victimRotation, Snap.victimScale);
		if (Snap.cutBelowY >= 0) EndScissorMode();
		for (const auto& Entry : Bank.effects)
		{
			const auto& E = Entry.first;
			if (!Entry.second.id || std::strcmp(E.usage, "attached") != 0 || std::strcmp(E.move, MoveId) != 0 || StateFrame < E.from || StateFrame > E.to) continue;
			const int Ef = std::min(E.frames - 1, std::max(0, StateFrame - E.from) / std::max(1, 60 / E.fps));
			DrawTexturePro(Entry.second, Rectangle{static_cast<float>(Ef * E.size), 0, static_cast<float>(E.size * Snap.facing), static_cast<float>(E.size)}, Rectangle{Snap.x + E.x * Snap.facing, Snap.y + Sheet.sole + E.y, E.size * E.scale, E.size * E.scale}, Vector2{E.size * E.scale / 2, E.size * E.scale / 2}, 0, WHITE);
		}

		if (DrawBoxes)
		{
			DrawRectangleLinesEx(
				Rectangle{Snap.pushboxWorld.x, Snap.pushboxWorld.y, Snap.pushboxWorld.width, Snap.pushboxWorld.height},
				1.0f, Color{90, 180, 255, 210});
			for (const RDA::Rect& Hurt : Snap.hurtboxesWorld)
			{
				DrawRectangleLinesEx(Rectangle{Hurt.x, Hurt.y, Hurt.width, Hurt.height}, 1.0f, Color{80, 255, 120, 210});
			}
			for (const RDA::Rect& Hit : Snap.hitboxesWorld)
			{
				DrawRectangleRec(Rectangle{Hit.x, Hit.y, Hit.width, Hit.height}, Color{255, 64, 96, 110});
				DrawRectangleLinesEx(Rectangle{Hit.x, Hit.y, Hit.width, Hit.height}, 1.0f, Color{255, 64, 96, 245});
			}
		}
	}

	void DrawTouchOverlay(bool bShowTouch, const RDA::InputFrame& ActiveInput)
	{
		if (!bShowTouch) return;
		// Left radial D-pad in 640x360 logical space
		const Vector2 Pad=Preferences->ControlPoint({72,294},640,360);
        const int PadX=static_cast<int>(Pad.x),PadY=static_cast<int>(Pad.y);
		DrawCircle(PadX, PadY, 42.0f*Preferences->value.touchSize, ColorAlpha(kPanel,Preferences->value.touchOpacity));
		DrawCircleLines(PadX, PadY, 42.0f*Preferences->value.touchSize, ColorAlpha(kCyan,Preferences->value.touchOpacity));
		DrawText("L", PadX - 32, PadY - 4, 8, ActiveInput.left ? kYellow : kMuted);
		DrawText("R", PadX + 26, PadY - 4, 8, ActiveInput.right ? kYellow : kMuted);
		DrawText("U", PadX - 3, PadY - 34, 8, ActiveInput.up ? kYellow : kMuted);
		DrawText("D", PadX - 3, PadY + 26, 8, ActiveInput.down ? kYellow : kMuted);

		// Right action diamond + Grab + Super
		struct BtnSpec { int x; int y; const char* label; Color col; bool active; };
		const BtnSpec Btns[] = {
			{536, 294, "L", kCyan, ActiveInput.punch},
			{572, 268, "H", kPink, ActiveInput.kick},
			{604, 294, "S", kYellow, ActiveInput.special},
			{572, 320, "B", kGreen, ActiveInput.block},
			{500, 268, "AG", kWhite, ActiveInput.grabPressed},
			{500, 320, "SUP", kDanger, ActiveInput.superPressed},
		};
		for (const BtnSpec& B : Btns)
		{
            Vector2 Center=Preferences->ControlPoint({static_cast<float>(B.x),static_cast<float>(B.y)},640,360);
			DrawCircle(static_cast<int>(Center.x), static_cast<int>(Center.y), 16.0f*Preferences->value.touchSize, B.active ? ColorAlpha(B.col, 0.65f) : ColorAlpha(kPanel,Preferences->value.touchOpacity));
			DrawCircleLines(static_cast<int>(Center.x), static_cast<int>(Center.y),16.0f*Preferences->value.touchSize,B.col);
			const int Tw = MeasureText(B.label, 6);
			DrawText(B.label,static_cast<int>(Center.x)-Tw/2,static_cast<int>(Center.y)-3,6,kWhite);
		}
	}

	Vector2 PointerLogical(const Rectangle& Viewport, Vector2 P)
	{
		return {(P.x - Viewport.x) * RDA::kLogicalWidth / Viewport.width, (P.y - Viewport.y) * RDA::kLogicalHeight / Viewport.height};
	}
	void MergeCombatEdges(RDA::InputFrame& To, const RDA::InputFrame& From)
	{
		To.upPressed|=From.upPressed; To.downPressed|=From.downPressed; To.leftPressed|=From.leftPressed; To.rightPressed|=From.rightPressed;
		To.punchPressed|=From.punchPressed; To.kickPressed|=From.kickPressed; To.specialPressed|=From.specialPressed; To.blockPressed|=From.blockPressed;
		To.grabPressed|=From.grabPressed; To.superPressed|=From.superPressed; To.debugPressed|=From.debugPressed;
	}
	void ClearCombatEdges(RDA::InputFrame& Input)
	{
		Input.upPressed=Input.downPressed=Input.leftPressed=Input.rightPressed=false;
		Input.punchPressed=Input.kickPressed=Input.specialPressed=Input.blockPressed=false;
		Input.grabPressed=Input.superPressed=Input.debugPressed=false;
	}

	void ReadTouchInput(RDA::InputFrame& OutP1, const Rectangle& DestViewport, bool& OutTouchDetected, bool AllowGameplayControls)
	{
		static RDA::InputFrame Previous{};
		static bool PreviousGrab = false, PreviousSuper = false;
		RDA::InputFrame Touch{};
		bool Grab = false, Super = false;
		const int Count = GetTouchPointCount();
		if (Count > 0) OutTouchDetected = true;
		// Menu targets can overlap the invisible joystick/buttons. Reset touch
		// edges outside a match instead of interpreting a stage tap as direction.
		const int Points = AllowGameplayControls ? (Count > 0 ? Count : (IsMouseButtonDown(MOUSE_LEFT_BUTTON) ? 1 : 0)) : 0;
		for (int i = 0; i < Points; ++i)
		{
			Vector2 P = Preferences->ControlInput(PointerLogical(DestViewport, Count > 0 ? GetTouchPosition(i) : GetMousePosition()),640,360);
			if (P.x < 0 || P.x > RDA::kLogicalWidth || P.y < 0 || P.y > RDA::kLogicalHeight) continue;
			const float Dx = P.x - 72, Dy = P.y - 294;
			if (Dx * Dx + Dy * Dy <= 54 * 54*Preferences->value.touchSize*Preferences->value.touchSize)
			{
				Touch.left |= Dx < -10; Touch.right |= Dx > 10;
				Touch.up |= Dy < -14; Touch.down |= Dy > 14;
			}
			auto Hit = [&](float X, float Y) { return (P.x-X)*(P.x-X)+(P.y-Y)*(P.y-Y) <= 20*20*Preferences->value.touchSize*Preferences->value.touchSize; };
			Touch.punch |= Hit(536,294); Touch.kick |= Hit(572,268);
			Touch.special |= Hit(604,294); Touch.block |= Hit(572,320);
			Grab |= Hit(500,268); Super |= Hit(500,320);
		}
		OutP1.left |= Touch.left; OutP1.right |= Touch.right; OutP1.up |= Touch.up; OutP1.down |= Touch.down;
		OutP1.punch |= Touch.punch; OutP1.kick |= Touch.kick; OutP1.special |= Touch.special; OutP1.block |= Touch.block;
		OutP1.leftPressed |= Touch.left && !Previous.left; OutP1.rightPressed |= Touch.right && !Previous.right;
		OutP1.upPressed |= Touch.up && !Previous.up; OutP1.downPressed |= Touch.down && !Previous.down;
		OutP1.punchPressed |= Touch.punch && !Previous.punch; OutP1.kickPressed |= Touch.kick && !Previous.kick;
		OutP1.specialPressed |= Touch.special && !Previous.special; OutP1.blockPressed |= Touch.block && !Previous.block;
		OutP1.grabPressed |= Grab && !PreviousGrab; OutP1.superPressed |= Super && !PreviousSuper;
		Previous = Touch; PreviousGrab = Grab; PreviousSuper = Super;
	}

	void ReadGamepadInput(RDA::InputFrame& Out, int Pad)
	{
		if (IsGamepadAvailable(Pad))
		{
			const float AxisX = GetGamepadAxisMovement(Pad, GAMEPAD_AXIS_LEFT_X);
			const float AxisY = GetGamepadAxisMovement(Pad, GAMEPAD_AXIS_LEFT_Y);
			Out.left = Out.left || AxisX < -Preferences->value.deadzone || IsGamepadButtonDown(Pad, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
			Out.right = Out.right || AxisX > Preferences->value.deadzone || IsGamepadButtonDown(Pad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
			Out.up = Out.up || AxisY < -Preferences->value.deadzone || IsGamepadButtonDown(Pad, GAMEPAD_BUTTON_LEFT_FACE_UP);
			Out.down = Out.down || AxisY > Preferences->value.deadzone || IsGamepadButtonDown(Pad, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
			Out.punch = Out.punch || IsGamepadButtonDown(Pad, Preferences->value.buttons[0]);
			Out.kick = Out.kick || IsGamepadButtonDown(Pad, Preferences->value.buttons[1]);
			Out.special = Out.special || IsGamepadButtonDown(Pad, Preferences->value.buttons[2]);
			Out.block = Out.block || IsGamepadButtonDown(Pad, Preferences->value.buttons[3]);

			Out.upPressed = Out.upPressed || IsGamepadButtonPressed(Pad, GAMEPAD_BUTTON_LEFT_FACE_UP);
			Out.punchPressed = Out.punchPressed || IsGamepadButtonPressed(Pad, Preferences->value.buttons[0]);
			Out.kickPressed = Out.kickPressed || IsGamepadButtonPressed(Pad, Preferences->value.buttons[1]);
			Out.specialPressed = Out.specialPressed || IsGamepadButtonPressed(Pad, Preferences->value.buttons[2]);
			Out.blockPressed = Out.blockPressed || IsGamepadButtonPressed(Pad, Preferences->value.buttons[3]);
			Out.grabPressed = Out.grabPressed || IsGamepadButtonPressed(Pad, Preferences->value.buttons[4]);
			Out.superPressed = Out.superPressed || IsGamepadButtonPressed(Pad, Preferences->value.buttons[5]);
			Out.pausePressed = Out.pausePressed || IsGamepadButtonPressed(Pad, GAMEPAD_BUTTON_MIDDLE_RIGHT);
			Out.confirmPressed = Out.confirmPressed || IsGamepadButtonPressed(Pad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
		}
		static std::array<RDA::InputFrame,2> Previous{};
		const auto& Last=Previous[static_cast<size_t>(Pad)];
		Out.upPressed|=Out.up&&!Last.up; Out.downPressed|=Out.down&&!Last.down;
		Out.leftPressed|=Out.left&&!Last.left; Out.rightPressed|=Out.right&&!Last.right;
		Previous[static_cast<size_t>(Pad)]=Out;
	}

	RDA::InputFrame ReadPlayerOneInput(const Rectangle& DestViewport, bool& InOutTouchActive, bool AllowGameplayTouch)
	{
		RDA::InputFrame Out{};
		Out.left = Preferences->Down(0,0);
		Out.right = Preferences->Down(0,1);
		Out.up = Preferences->Down(0,2);
		Out.down = Preferences->Down(0,3);
		Out.punch = Preferences->Down(0,4);
		Out.kick = Preferences->Down(0,5);
		Out.special = Preferences->Down(0,6);
		Out.block = Preferences->Down(0,7);

		Out.upPressed = Preferences->Pressed(0,2);
		Out.downPressed = Preferences->Pressed(0,3);
		Out.leftPressed = Preferences->Pressed(0,0);
		Out.rightPressed = Preferences->Pressed(0,1);
		Out.punchPressed = Preferences->Pressed(0,4);
		Out.kickPressed = Preferences->Pressed(0,5);
		Out.specialPressed = Preferences->Pressed(0,6);
		Out.blockPressed = Preferences->Pressed(0,7);
		Out.grabPressed = Preferences->Pressed(0,8) || (Preferences->Down(0,4) && Preferences->Pressed(0,5)) || (Preferences->Pressed(0,4) && Preferences->Down(0,5));
		Out.superPressed = Preferences->Pressed(0,9);
		Out.pausePressed = IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P);
		Out.confirmPressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
		Out.debugPressed = IsKeyPressed(KEY_F1);

		ReadGamepadInput(Out,0);

		ReadTouchInput(Out, DestViewport, InOutTouchActive, AllowGameplayTouch);
		return Out;
	}

	RDA::InputFrame ReadPlayerTwoKeyboardInput()
	{
		RDA::InputFrame Out{};
		Out.left = Preferences->Down(1,0);
		Out.right = Preferences->Down(1,1);
		Out.up = Preferences->Down(1,2);
		Out.down = Preferences->Down(1,3);
		Out.punch = Preferences->Down(1,4);
		Out.kick = Preferences->Down(1,5);
		Out.special = Preferences->Down(1,6);
		Out.block = Preferences->Down(1,7);

		Out.upPressed = Preferences->Pressed(1,2);
		Out.downPressed = Preferences->Pressed(1,3);
		Out.leftPressed = Preferences->Pressed(1,0);
		Out.rightPressed = Preferences->Pressed(1,1);
		Out.punchPressed = Preferences->Pressed(1,4);
		Out.kickPressed = Preferences->Pressed(1,5);
		Out.specialPressed = Preferences->Pressed(1,6);
		Out.blockPressed = Preferences->Pressed(1,7);
		Out.grabPressed = Preferences->Pressed(1,8) || (Preferences->Down(1,4) && Preferences->Pressed(1,5)) || (Preferences->Pressed(1,4) && Preferences->Down(1,5));
		Out.superPressed = Preferences->Pressed(1,9);
		ReadGamepadInput(Out,1);
		return Out;
	}
}

int main(int argc, char** argv)
{
	std::string RenderSmokeDirectory;
    std::string SettingsSmokeDirectory;
	for (int i = 1; i < argc; ++i)
	{
#if defined(_WIN32)
        if(std::strcmp(argv[i],"--online-qa-port")==0&&i+1<argc)return RDAOnline::Smoke(std::atoi(argv[++i]));
#endif
		if(std::strcmp(argv[i],"--settings-smoke")==0 && i+1<argc)SettingsSmokeDirectory=argv[++i];
		if (std::strcmp(argv[i], "--render-smoke") == 0 && i + 1 < argc) RenderSmokeDirectory = argv[++i];
		if (std::strcmp(argv[i], "--headless-smoke") == 0)
		{
			RDA::CombatWorld SmokeWorld(
				RDA::GetGutoBarba(),
				RDA::GetDanteSinal(),
				RDA::GameMode::VsCpu,
				RDA::StageId::CaisDaCidade);
			for (int Step = 0; Step < 360; ++Step)
			{
				const RDA::InputFrame P1 = RDA::BuildCpuInput(SmokeWorld, 0, RDA::AiDifficulty::Hard);
				const RDA::InputFrame P2 = RDA::BuildCpuInput(SmokeWorld, 1, RDA::AiDifficulty::Normal);
				SmokeWorld.Step(P1, P2);
			}
			std::printf("OK RuaDeAco headless-smoke hash=%016llx\n",
				static_cast<unsigned long long>(SmokeWorld.ComputeStateHash()));
			return 0;
		}
	}

	SetConfigFlags(RenderSmokeDirectory.empty() && SettingsSmokeDirectory.empty() ? FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT : FLAG_WINDOW_HIDDEN);
#if defined(PLATFORM_ANDROID)
	// Use the physical EGL surface. raylib 5.5 otherwise scales its requested
	// window in addition to our own centered 640x360 framebuffer viewport.
	const Vector2 PhysicalSurface=SLTMobile::InitialSurfaceSize();
    InitWindow(static_cast<int>(PhysicalSurface.x),static_cast<int>(PhysicalSurface.y), "Rua de Aco");
#else
	InitWindow(1280, 720, "Rua de Aco");
#endif
	RDAAssets::Initialize();
#if defined(PLATFORM_ANDROID)
    fs::path SettingsPath=fs::path(GetAndroidApp()->activity->internalDataPath)/"preferences.cfg";
#else
    const char* LocalData=std::getenv("LOCALAPPDATA");
    fs::path SettingsPath=fs::path(LocalData?LocalData:GetApplicationDirectory())/"SmellsLikeTech/RuaDeAco/preferences.cfg";
#endif
    SLTPrefs::Preferences Settings(SettingsPath,true); Preferences=&Settings;
    auto LaunchOnline=[&](){
#if defined(_WIN32)
        RDAOnline::Open(GetWindowHandle(),Settings.value.master,Settings.value.music,Settings.value.effects,Settings.value.touchOpacity);
#elif defined(PLATFORM_ANDROID)
        SLTMobile::OpenOnline(Settings.value.master,Settings.value.music,Settings.value.effects,Settings.value.touchOpacity);
#endif
    };
    if(!RenderSmokeDirectory.empty()) { Settings.Defaults();Settings.value.quality=0; }
    Settings.ApplyWindow(!RenderSmokeDirectory.empty()||!SettingsSmokeDirectory.empty());

	SetExitKey(KEY_NULL);
#if defined(_WIN32)
	const auto IconBytes = SLTBranding::ApplicationIconPng();
	Image AppIcon = LoadImageFromMemory(".png", IconBytes.data(), static_cast<int>(IconBytes.size()));
	if (AppIcon.data) { SetWindowIcon(AppIcon); UnloadImage(AppIcon); TraceLog(LOG_INFO, "RDA_APP_ICON: embedded icon applied"); }
	else TraceLog(LOG_ERROR, "RDA_APP_ICON: embedded icon missing or invalid");
#endif
	SetTargetFPS(RenderSmokeDirectory.empty() ? Settings.value.fps : 0);
	InitAudioDevice();

	RenderTexture2D Canvas = LoadRenderTexture(RDA::kLogicalWidth*Settings.RenderScale(), RDA::kLogicalHeight*Settings.RenderScale());
	SetTextureFilter(Canvas.texture, TEXTURE_FILTER_POINT);

	const fs::path ContentRoot = FindContentRoot();
	const fs::path ArtRoot = ContentRoot / "RuaDeAco" / "Art";
	const fs::path MusicRoot = ContentRoot / "RuaDeAco" / "Audio" / "Music";
	RDAStageVisuals::StageRenderer StageRenderer;
	if (!StageRenderer.Load(ArtRoot)) { TraceLog(LOG_ERROR,"Canonical stage assets incomplete"); return 5; }
	SoundBank Sounds = BuildSoundBank(ContentRoot);

	Texture2D LogoTex = LoadPixelTexture(ArtRoot / "UI" / "rua-de-aco-logo.png");
	std::array<FighterVisualBank, RDA::kFighterCount> FighterVisuals{};
	for (int i = 0; i < RDA::kFighterCount; ++i)
	{
		FighterVisuals[static_cast<size_t>(i)] = LoadFighterVisuals(ArtRoot, RDA::GetFighterDef(static_cast<RDA::FighterId>(i)), false);
	}
	auto PrepareFighterVisuals = [&](int One, int Two)
	{
        AssetLoading=true;
		for (int i=0;i<RDA::kFighterCount;++i)
		{
			auto& Bank = FighterVisuals[static_cast<size_t>(i)];
			if (i==One || i==Two)
			{
				if (Bank.animations.empty()) { if (Bank.portrait.id) UnloadTexture(Bank.portrait); Bank=LoadFighterVisuals(ArtRoot,RDA::GetFighterDef(static_cast<RDA::FighterId>(i))); }
			}
			else
			{
				for (auto& Entry:Bank.animations) if (Entry.second.texture.id) UnloadTexture(Entry.second.texture);
				for (auto& Entry:Bank.effects) if (Entry.second.id) UnloadTexture(Entry.second);
				Bank.animations.clear(); Bank.effects.clear();
			}
		}
        AssetLoading=false;
	};

	std::array<StageVisualBank, RDA::kStageCount> StageVisuals{};
	for (int i = 0; i < RDA::kStageCount; ++i)
	{
		const RDA::StageDef& SDef = RDA::GetStageDef(static_cast<RDA::StageId>(i));
		StageVisualBank& SVis = StageVisuals[static_cast<size_t>(i)];
		const fs::path StageDir = ArtRoot / "Environments" / SDef.folder;
		SVis.background = LoadPixelTexture(StageDir / "background.png");
		if (SDef.id == RDA::StageId::CaisDaCidade)
		{
			SVis.finisherProp1 = LoadPixelTexture(StageDir / "monster-finisher-v2.png");
			SVis.finisherProp2 = LoadPixelTexture(StageDir / "splash.png");
		}
		else if (SDef.id == RDA::StageId::CozinhaMacabra)
		{
			SVis.finisherProp1 = LoadPixelTexture(StageDir / "witch.png");
			SVis.finisherProp2 = LoadPixelTexture(StageDir / "skull-bones.png");
		}
		else if (SDef.id == RDA::StageId::Sitio)
		{
			SVis.finisherProp1 = LoadPixelTexture(StageDir / "masked-farmer.png");
			SVis.finisherProp2 = LoadPixelTexture(StageDir / "trident.png");
		}

		const fs::path MusicPath = MusicRoot / SDef.musicFile;
		if (IsAudioDeviceReady() && RDAAssets::Exists(MusicPath))
		{
			SVis.music = RDAAssets::LoadMusic(MusicPath,SVis.musicData);
			SVis.hasMusic = SVis.music.frameCount > 0;
			if (SVis.hasMusic)
			{
				SetMusicVolume(SVis.music, 0.35f);
			}
		}
	}

	Music MenuMusic{};
	std::vector<unsigned char> MenuMusicData;
	bool bHasMenuMusic = false;
	if (IsAudioDeviceReady() && RDAAssets::Exists(MusicRoot / "menu-principal.ogg"))
	{
		MenuMusic = RDAAssets::LoadMusic(MusicRoot / "menu-principal.ogg",MenuMusicData);
		bHasMenuMusic = MenuMusic.frameCount > 0;
		if (bHasMenuMusic)
		{
			SetMusicVolume(MenuMusic, 0.35f);
			PlayMusicStream(MenuMusic);
		}
	}

	AppScreen Screen = AppScreen::MainMenu;
	int MenuCursor = 0;
	RDA::GameMode SelectedMode = RDA::GameMode::VsCpu;
	RDA::AiDifficulty SelectedDifficulty = RDA::AiDifficulty::Normal;
	int SelectedP1 = 0; // Guto Barba
	int SelectedP2 = 1; // Rafa Mare
	int SelectedStage = 0; // Cais da Cidade
	int SelectStep = 0; // 0 = P1, 1 = P2, 2 = Arena
	bool bShowTouchOverlay =
#if defined(PLATFORM_ANDROID)
		true;
#else
		false;
#endif
	bool bShowMoveList = false;
	bool bTrainingCpuActive = false;
	int GlobalTicks = 0;

	std::unique_ptr<RDA::CombatWorld> World;
	if (!RenderSmokeDirectory.empty()) fs::create_directories(RenderSmokeDirectory);
    if(!SettingsSmokeDirectory.empty())fs::create_directories(SettingsSmokeDirectory);
	AppScreen LoggedScreen = static_cast<AppScreen>(-1);
	bool LoggedPause = false;
	double SimulationAccumulator = 0;
	RDA::InputFrame PendingP1{}, PendingP2{};
	int SmokeFinisher = -1, LastSmokeFinisherFrame = -1;

	while (!WindowShouldClose())
	{
        const bool SettingsWasOpen=Settings.open;
        Settings.Update();
        if(Settings.changed) {
            if(Settings.windowChanged)Settings.ApplyWindow(!RenderSmokeDirectory.empty()||!SettingsSmokeDirectory.empty());
            if(Canvas.texture.width!=RDA::kLogicalWidth*Settings.RenderScale()) { UnloadRenderTexture(Canvas);Canvas=LoadRenderTexture(RDA::kLogicalWidth*Settings.RenderScale(),RDA::kLogicalHeight*Settings.RenderScale());SetTextureFilter(Canvas.texture,TEXTURE_FILTER_POINT); }
        }
        #if defined(_WIN32)
        SetMasterVolume(RDAOnline::Active()?0:Settings.value.master);
        if(RenderSmokeDirectory.empty())SetTargetFPS(RDAOnline::Active()?15:Settings.value.fps);
#else
        SetMasterVolume(!SLTMobile::Background&&IsWindowFocused()?Settings.value.master:0);
#endif
        if(bHasMenuMusic)SetMusicVolume(MenuMusic,Settings.value.music);
        for(auto& Stage:StageVisuals)if(Stage.hasMusic)SetMusicVolume(Stage.music,Settings.value.music);
        bShowTouchOverlay=Settings.value.touch;
        const bool AndroidPause=SLTPrefs::Mobile&&SLTMobile::PauseRequested.exchange(false);
        if(World && !World->IsPaused() && (AndroidPause || ((SLTPrefs::Mobile||Settings.value.autoPause) && !IsWindowFocused()) || Settings.open))World->TogglePause();
		GlobalTicks += 1;
        if(!SettingsSmokeDirectory.empty())Settings.Show((GlobalTicks-1)/2);
		CameraTrauma=std::max(0.f,CameraTrauma-GetFrameTime());
		const double FrameSeconds = RenderSmokeDirectory.empty() ? std::min(GetFrameTime(),0.1f) : 1.0/60.0;
		if (Screen != AppScreen::Match) { SimulationAccumulator=0; PendingP1={}; PendingP2={}; }
		if (!RenderSmokeDirectory.empty())
		{
			if (GlobalTicks == 2) Screen = AppScreen::CharacterSelect;
			if (GlobalTicks >= 3 && GlobalTicks < 1083 && (GlobalTicks - 3) % 180 == 0)
			{
				const int Index = (GlobalTicks - 3) / 180;
				if (Index >= RDA::kFighterCount) break;
				SelectedP1 = Index; SelectedP2 = (Index + 1) % RDA::kFighterCount; SelectedStage = Index % RDA::kStageCount;
				SelectedMode = RDA::GameMode::LocalVersus;
				PrepareFighterVisuals(SelectedP1,SelectedP2);
				World = std::make_unique<RDA::CombatWorld>(RDA::GetFighterDef(static_cast<RDA::FighterId>(SelectedP1)), RDA::GetFighterDef(static_cast<RDA::FighterId>(SelectedP2)), SelectedMode, static_cast<RDA::StageId>(SelectedStage));
				Screen = AppScreen::Match;
			}
			if (GlobalTicks >= 1083 && (GlobalTicks-1083)%320 == 0)
			{
				SmokeFinisher=(GlobalTicks-1083)/320; LastSmokeFinisherFrame=-1;
				if (SmokeFinisher>=RDA::kStageCount) break;
				SelectedP1=0; SelectedP2=3; SelectedStage=SmokeFinisher; SelectedMode=RDA::GameMode::LocalVersus;
				PrepareFighterVisuals(SelectedP1,SelectedP2);
				World=std::make_unique<RDA::CombatWorld>(RDA::GetGutoBarba(),RDA::GetAstroRiso(),SelectedMode,static_cast<RDA::StageId>(SelectedStage));
				RDA::InputFrame Empty{}, Punch{}, Grab{};
				for(int i=0;i<105;++i) World->Step(Empty,Empty);
				World->fighters[0].roundWins=1; World->fighters[0].x=300; World->fighters[1].x=330; World->fighters[1].health=10;
				Punch.punch=Punch.punchPressed=true; World->Step(Punch,Empty);
				for(int i=0;i<15 && World->Phase()==RDA::CombatPhase::Active;++i) World->Step(Empty,Empty);
				Grab.grabPressed=Grab.punchPressed=Grab.kick=true; World->Step(Grab,Empty);
				for(int i=0;i<12 && World->Phase()==RDA::CombatPhase::FinishReady;++i) World->Step(Empty,Empty);
				if (World->Phase()!=RDA::CombatPhase::StageFinish) { TraceLog(LOG_ERROR,"Render smoke did not enter canonical finisher"); return 6; }
				SimulationAccumulator=0; Screen=AppScreen::Match;
			}
		}
		if (!SLTPrefs::Mobile && !SettingsWasOpen && (IsKeyPressed(KEY_F11) || (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER))))
		{
			ToggleFullscreen();
		}
		if (!SLTPrefs::Mobile && !SettingsWasOpen && IsKeyPressed(KEY_F4))
		{
			Settings.value.touch = !Settings.value.touch; Settings.Save();
		}

		const float ScaleX = static_cast<float>(GetScreenWidth()) / static_cast<float>(RDA::kLogicalWidth);
		const float ScaleY = static_cast<float>(GetScreenHeight()) / static_cast<float>(RDA::kLogicalHeight);
		const float RawScale = std::max(0.01f, std::min(ScaleX, ScaleY));
        const float Scale=Settings.value.integerScale&&RawScale>=1?std::floor(RawScale):RawScale;
		const float DrawW = static_cast<float>(RDA::kLogicalWidth) * Scale;
		const float DrawH = static_cast<float>(RDA::kLogicalHeight) * Scale;
		const Rectangle DestViewport{
			std::floor((static_cast<float>(GetScreenWidth()) - DrawW) * 0.5f),
			std::floor((static_cast<float>(GetScreenHeight()) - DrawH) * 0.5f),
			DrawW,
			DrawH
		};

		RDA::InputFrame P1 = ReadPlayerOneInput(DestViewport, bShowTouchOverlay, Screen == AppScreen::Match && !Settings.open && World && !World->IsPaused());
        if(SettingsWasOpen||Settings.open)P1={};
		P1.pausePressed |= !SettingsWasOpen && !Settings.open && IsKeyPressed(KEY_BACK);
		static bool PauseWasDown=false;
		bool PauseDown=false;
		for(int i=0;i<GetTouchPointCount();++i) { auto P=PointerLogical(DestViewport,GetTouchPosition(i)); PauseDown|=CheckCollisionPointRec(P,Rectangle{590,0,50,46}); }
		if(Screen==AppScreen::Match && !SettingsWasOpen && !Settings.open) P1.pausePressed|=PauseDown&&!PauseWasDown;
		PauseWasDown=PauseDown;
		static bool PointerWasDown = false;
		const bool PointerDown = GetTouchPointCount() > 0 || IsMouseButtonDown(MOUSE_LEFT_BUTTON);
		const bool PointerTap = PointerDown && !PointerWasDown && !SettingsWasOpen && !Settings.open;
		PointerWasDown = PointerDown;
		const Vector2 Pointer = PointerLogical(DestViewport, GetTouchPointCount() > 0 ? GetTouchPosition(0) : GetMousePosition());
		auto Tap = [&](float X, float Y, float W, float H) { return PointerTap && CheckCollisionPointRec(Pointer, Rectangle{X,Y,W,H}); };
        if((Screen!=AppScreen::Match && Tap(548,326,88,30)) || (!SLTPrefs::Mobile&&!SettingsWasOpen&&!Settings.open&&IsKeyPressed(KEY_F10)))Settings.Show();
        if(Screen==AppScreen::Match && World && World->IsPaused() && Tap(160,182,320,20))Settings.Show();
        if(Screen==AppScreen::Match && World && SelectedMode==RDA::GameMode::Training && !Settings.open) {
            if(Tap(180,87,85,23))World->ToggleDebugBoxes();
            if(Tap(270,87,85,23))World->ResetTrainingPositions();
            if(Tap(360,87,90,23))bTrainingCpuActive=!bTrainingCpuActive;
        }
		if (Screen == AppScreen::MainMenu)
		{
			for (int i=0; i<6; ++i) if (Tap(64.f+(i%2)*272,142.f+(i/2)*56,240,44)) { MenuCursor=i; P1.confirmPressed=true; }
			if (Tap(160,304,320,26)) SelectedDifficulty=static_cast<RDA::AiDifficulty>((static_cast<int>(SelectedDifficulty)+1)%3);
		}
		else if (Screen == AppScreen::CharacterSelect)
		{
			for (int i=0;i<RDA::kFighterCount;++i) if (Tap(26.0f+i*98,46,90,118)) { if (SelectStep==0) SelectedP1=i; else { SelectedP2=i; SelectStep=1; } P1.confirmPressed=true; }
			for (int i=0;i<RDA::kStageCount;++i) if (SelectStep==2 && Tap(26.0f+i*198,276,190,48)) { SelectedStage=i; P1.confirmPressed=true; }
			if (Tap(0,0,90,34)) P1.pausePressed=true;
		}
		else if (Screen == AppScreen::Match && World)
		{
			if (Tap(590,0,50,46)) P1.pausePressed=true;
			if (World->IsPaused())
			{
				if (Tap(160,122,320,20)) P1.pausePressed=true;
				if (Tap(160,142,320,20)) bShowMoveList=!bShowMoveList;
				if (Tap(160,162,320,20)) P1.debugPressed=true;
				if (Tap(160,202,320,20)) { Screen=AppScreen::CharacterSelect; SelectStep=0; }
				if (Tap(160,222,320,20)) Screen=AppScreen::MainMenu;
			}
		}
		RDA::InputFrame P2Key = ReadPlayerTwoKeyboardInput();
        if(Settings.open||SettingsWasOpen)P2Key={};
		if (Screen != LoggedScreen) { TraceLog(LOG_INFO,"RDA_SCREEN: %d",static_cast<int>(Screen)); LoggedScreen=Screen; }
		if (World && World->IsPaused()!=LoggedPause) { LoggedPause=World->IsPaused(); TraceLog(LOG_INFO,"RDA_PAUSE: %d",LoggedPause?1:0); }

		if (Screen != AppScreen::Match && bHasMenuMusic)
		{
			if (!IsMusicStreamPlaying(MenuMusic)) PlayMusicStream(MenuMusic);
			UpdateMusicStream(MenuMusic);
		}
		else if (bHasMenuMusic && IsMusicStreamPlaying(MenuMusic)) StopMusicStream(MenuMusic);
		if (Screen != AppScreen::Match) for (auto& StageVisual : StageVisuals) if(StageVisual.hasMusic && IsMusicStreamPlaying(StageVisual.music)) StopMusicStream(StageVisual.music);

        if(!Settings.open && !SettingsWasOpen) {
		if (Screen == AppScreen::MainMenu)
		{
			if (P1.upPressed || P2Key.upPressed) MenuCursor = (MenuCursor + 5) % 6;
			if (P1.downPressed || P2Key.downPressed) MenuCursor = (MenuCursor + 1) % 6;
			if (P1.leftPressed || P1.rightPressed)
			{
				const int NextDiff = (static_cast<int>(SelectedDifficulty) + 1) % 3;
				SelectedDifficulty = static_cast<RDA::AiDifficulty>(NextDiff);
			}
			if (P1.confirmPressed || P1.punchPressed)
			{
				if (MenuCursor == 5)
				{
					break;
				}
				if(MenuCursor==3)LaunchOnline();
                else if(MenuCursor==4)Settings.Show();
                else {
				SelectedMode = MenuCursor == 0 ? RDA::GameMode::VsCpu :
					(MenuCursor == 1 ? RDA::GameMode::LocalVersus : RDA::GameMode::Training);
				SelectStep = 0;
				Screen = AppScreen::CharacterSelect;
				PlayWorldSound(Sounds, RDA::SoundEvent::HitLight, RDA::FighterId::GutoBarba);
                }
			}
		}
		else if (Screen == AppScreen::CharacterSelect)
		{
			if (P1.pausePressed)
			{
				if (SelectStep > 0) SelectStep -= 1;
				else Screen = AppScreen::MainMenu;
			}
			else if (SelectStep == 0)
			{
				if (P1.leftPressed) SelectedP1 = (SelectedP1 + RDA::kFighterCount - 1) % RDA::kFighterCount;
				if (P1.rightPressed) SelectedP1 = (SelectedP1 + 1) % RDA::kFighterCount;
				if (P1.confirmPressed || P1.punchPressed)
				{
					SelectStep = 1;
					PlayWorldSound(Sounds, RDA::SoundEvent::HitLight, static_cast<RDA::FighterId>(SelectedP1));
				}
			}
			else if (SelectStep == 1)
			{
				if (P1.leftPressed || P2Key.leftPressed) SelectedP2 = (SelectedP2 + RDA::kFighterCount - 1) % RDA::kFighterCount;
				if (P1.rightPressed || P2Key.rightPressed) SelectedP2 = (SelectedP2 + 1) % RDA::kFighterCount;
				if (P1.confirmPressed || P1.punchPressed || P2Key.punchPressed)
				{
					SelectStep = 2;
					PlayWorldSound(Sounds, RDA::SoundEvent::HitLight, static_cast<RDA::FighterId>(SelectedP2));
				}
			}
			else if (SelectStep == 2)
			{
				if (P1.leftPressed || P2Key.leftPressed) SelectedStage = (SelectedStage + RDA::kStageCount - 1) % RDA::kStageCount;
				if (P1.rightPressed || P2Key.rightPressed) SelectedStage = (SelectedStage + 1) % RDA::kStageCount;
				if (P1.confirmPressed || P1.punchPressed)
				{
					const RDA::FighterDef& F1 = RDA::GetFighterDef(static_cast<RDA::FighterId>(SelectedP1));
					const RDA::FighterDef& F2 = RDA::GetFighterDef(static_cast<RDA::FighterId>(SelectedP2));
					const RDA::StageId Stage = static_cast<RDA::StageId>(SelectedStage);
					PrepareFighterVisuals(SelectedP1,SelectedP2);
					World = std::make_unique<RDA::CombatWorld>(F1, F2, SelectedMode, Stage);
					TraceLog(LOG_INFO,"RDA_MATCH_STAGE: %d p1=%d p2=%d",static_cast<int>(World->Stage()),SelectedP1,SelectedP2);
					bShowMoveList = false;
					bTrainingCpuActive = false;
					if (StageVisuals[static_cast<size_t>(SelectedStage)].hasMusic)
					{
						PlayMusicStream(StageVisuals[static_cast<size_t>(SelectedStage)].music);
					}
					Screen = AppScreen::Match;
				}
			}
		}
		else if (Screen == AppScreen::Match && World)
		{
			StageVisualBank& SVis = StageVisuals[static_cast<size_t>(SelectedStage)];
			if (SVis.hasMusic)
			{
				UpdateMusicStream(SVis.music);
			}

			if (P1.pausePressed)
			{
				if (bShowMoveList) bShowMoveList = false;
				else World->TogglePause();
			}

			if (World->IsPaused())
			{
				SimulationAccumulator=0; PendingP1={}; PendingP2={};
				if (P1.debugPressed) World->ToggleDebugBoxes();
				if (IsKeyPressed(KEY_M)) bShowMoveList = !bShowMoveList;
				if (IsKeyPressed(KEY_C))
				{
					Screen = AppScreen::CharacterSelect;
					SelectStep = 0;
				}
				if (IsKeyPressed(KEY_Q))
				{
					Screen = AppScreen::MainMenu;
				}
			}
			else
			{
				if (SelectedMode == RDA::GameMode::Training)
				{
					if (IsKeyPressed(KEY_F2)) World->ResetTrainingPositions();
					if (IsKeyPressed(KEY_F3)) bTrainingCpuActive = !bTrainingCpuActive;
				}

				RDA::InputFrame P2 = P2Key;
				if (SelectedMode == RDA::GameMode::VsCpu || (SelectedMode == RDA::GameMode::Training && bTrainingCpuActive))
				{
					P2 = RDA::BuildCpuInput(*World, 1, SelectedDifficulty);
				}

				if (World->Phase() == RDA::CombatPhase::MatchOver)
				{
					if (P1.confirmPressed || P1.punchPressed)
					{
						World->RestartMatch();
					}
					if (IsKeyPressed(KEY_C))
					{
						Screen = AppScreen::CharacterSelect;
						SelectStep = 0;
					}
				}

				MergeCombatEdges(PendingP1,P1); MergeCombatEdges(PendingP2,P2Key);
				SimulationAccumulator += FrameSeconds;
				while (SimulationAccumulator + 1e-9 >= 1.0/60.0)
				{
					RDA::InputFrame TickP1=P1, TickP2=P2Key;
					ClearCombatEdges(TickP1); ClearCombatEdges(TickP2);
					MergeCombatEdges(TickP1,PendingP1); MergeCombatEdges(TickP2,PendingP2);
					if (SelectedMode == RDA::GameMode::VsCpu || (SelectedMode == RDA::GameMode::Training && bTrainingCpuActive)) TickP2=RDA::BuildCpuInput(*World,1,SelectedDifficulty);
					World->Step(TickP1,TickP2);
					PendingP1={}; PendingP2={};
					SimulationAccumulator -= 1.0/60.0;
				}
				for (const auto& Cue : World->DrainSoundCues()) PlayMoveSound(Sounds,Cue,*World);
				for (RDA::SoundEvent Ev : World->DrainSounds())
				{
					if (Ev==RDA::SoundEvent::SpecialCast || Ev==RDA::SoundEvent::SuperCast) continue;
					PlayWorldSound(Sounds, Ev, static_cast<RDA::FighterId>(SelectedP1));
				}
			}
		}

        }
		BeginTextureMode(Canvas);
        rlPushMatrix();rlScalef(static_cast<float>(Settings.RenderScale()),static_cast<float>(Settings.RenderScale()),1);
		ClearBackground(kInk);

		if (Screen == AppScreen::MainMenu)
		{
			const StageVisualBank& BgStage = StageVisuals[0];
			if (BgStage.background.id != 0)
			{
				DrawTexturePro(
					BgStage.background,
					Rectangle{0, 0, static_cast<float>(BgStage.background.width), static_cast<float>(BgStage.background.height)},
					Rectangle{0, 0, static_cast<float>(RDA::kLogicalWidth), static_cast<float>(RDA::kLogicalHeight)},
					Vector2{0, 0}, 0.0f, WHITE);
				DrawRectangle(0, 0, RDA::kLogicalWidth, RDA::kLogicalHeight, Color{4, 8, 20, 175});
			}

			if (LogoTex.id != 0)
			{
				const float Lw = 180.0f;
				const float Lh = Lw * (static_cast<float>(LogoTex.height) / static_cast<float>(std::max(1, LogoTex.width)));
				DrawTexturePro(
					LogoTex,
					Rectangle{0, 0, static_cast<float>(LogoTex.width), static_cast<float>(LogoTex.height)},
					Rectangle{static_cast<float>(RDA::kLogicalWidth) * 0.5f - Lw * 0.5f, 14.0f, Lw, Lh},
					Vector2{0, 0}, 0.0f, WHITE);
			}
			else
			{
				DrawText("RUA DE ACO", 218, 28, 30, kCyan);
			}

            DrawRectangle(52,130,536,184,kPanel);
			const char* DiffNames[] = {"FACIL", "NORMAL", "DIFICIL"};
			const char* Items[] = {
				TextFormat("1. VS CPU (%s)", DiffNames[static_cast<int>(SelectedDifficulty)]),
				"2. VERSUS LOCAL (2 JOGADORES)",
				"3. TREINAMENTO",
				"4. JOGAR ONLINE",
                "5. CONFIGURACOES",
                SLTPrefs::Mobile?"6. FECHAR JOGO":"6. SAIR"
			};
            for(int i=0;i<6;i++) {
                int x=64+(i%2)*272,y=142+(i/2)*56;
                DrawRectangle(x,y,240,44,MenuCursor==i?Color{40,63,87,250}:Color{16,25,44,245});
                DrawRectangleLines(x,y,240,44,MenuCursor==i?kYellow:kCyan);
                int font=10;DrawText(Items[i],x+12,y+17,font,MenuCursor==i?kYellow:kWhite);
            }

			DrawText(SLTPrefs::Mobile?"TOQUE PARA ESCOLHER O MODO | CONFIGURACOES: TELA, AUDIO E CONTROLES":"Setas: navegar | Enter: confirmar | Configuracoes: tela, audio e controles",
				38, 336, 10, kMuted);
			if (bShowTouchOverlay) DrawText("TOQUE PARA ESCOLHER | TOQUE AQUI: DIFICULDADE", 166, 310, 8, kCyan);
		}
		else if (Screen == AppScreen::CharacterSelect)
		{
			DrawRectangle(0, 0, RDA::kLogicalWidth, RDA::kLogicalHeight, Color{8, 12, 26, 255});
			DrawText("SELECAO DE LUTADORES E ARENA", 172, 14, 18, kCyan);
			if (bShowTouchOverlay) DrawText("< VOLTAR", 8, 14, 10, kWhite);

			for (int i = 0; i < RDA::kFighterCount; ++i)
			{
				const RDA::FighterDef& FDef = RDA::GetFighterDef(static_cast<RDA::FighterId>(i));
				const FighterVisualBank& FVis = FighterVisuals[static_cast<size_t>(i)];
				const int X = 26 + i * 98;
				const int Y = 46;
				DrawRectangle(X, Y, 90, 118, kPanel);
				if (FVis.portrait.id != 0)
				{
					DrawTexturePro(
						FVis.portrait,
						Rectangle{0, 0, static_cast<float>(FVis.portrait.width), static_cast<float>(FVis.portrait.height)},
						Rectangle{static_cast<float>(X + 5), static_cast<float>(Y + 5), 80.0f, 80.0f},
						Vector2{0, 0}, 0.0f, WHITE);
				}
				DrawText(FDef.name, X + 6, Y + 90, 8, kWhite);
				DrawText(TextFormat("HP %d", FDef.stats.maxHealth), X + 6, Y + 102, 8, kMuted);

				if (SelectedP1 == i)
				{
					DrawRectangleLinesEx(Rectangle{static_cast<float>(X - 2), static_cast<float>(Y - 2), 94.0f, 122.0f}, 2.0f, kCyan);
					DrawText("P1", X + 6, Y + 6, 10, kCyan);
				}
				if (SelectedP2 == i)
				{
					DrawRectangleLinesEx(Rectangle{static_cast<float>(X - 4), static_cast<float>(Y - 4), 98.0f, 126.0f}, 2.0f, kPink);
					DrawText(SelectedMode == RDA::GameMode::VsCpu ? "CPU" : "P2", X + 62, Y + 6, 10, kPink);
				}
			}

			// Detailed fighter card for current cursor
			const RDA::FighterDef& F1 = RDA::GetFighterDef(static_cast<RDA::FighterId>(SelectedP1));
			const RDA::FighterDef& F2 = RDA::GetFighterDef(static_cast<RDA::FighterId>(SelectedP2));
			DrawRectangle(26, 174, 286, 76, kPanel);
			DrawRectangleLinesEx(Rectangle{26, 174, 286, 76}, 1.0f, kCyan);
			DrawText(TextFormat("P1: %s (%s)", F1.name, F1.archetype), 34, 182, 10, kCyan);
			DrawText(TextFormat("Especiais: %s | %s", F1.special1.label, F1.special2.label), 34, 198, 8, kWhite);
			DrawText(TextFormat("Super (50 barra): %s", F1.superMove.label), 34, 212, 8, kYellow);
			DrawText(SLTPrefs::Mobile?"Agarrar: toque AG | Super: toque SUP":"Agarrar e Super: veja suas teclas em Configuracoes", 34, 226, 8, kGreen);

			DrawRectangle(328, 174, 286, 76, kPanel);
			DrawRectangleLinesEx(Rectangle{328, 174, 286, 76}, 1.0f, kPink);
			DrawText(TextFormat("%s: %s (%s)", SelectedMode == RDA::GameMode::VsCpu ? "CPU" : "P2", F2.name, F2.archetype), 336, 182, 10, kPink);
			DrawText(TextFormat("Especiais: %s | %s", F2.special1.label, F2.special2.label), 336, 198, 8, kWhite);
			DrawText(TextFormat("Super (50 barra): %s", F2.superMove.label), 336, 212, 8, kYellow);
			DrawText("Agarrao Universal: J+K / I", 336, 226, 8, kGreen);

			// Arena selector
			DrawText("ARENA (PALCO COM FINALIZACAO PROPRIA):", 26, 260, 10, kYellow);
			for (int s = 0; s < RDA::kStageCount; ++s)
			{
				const RDA::StageDef& SDef = RDA::GetStageDef(static_cast<RDA::StageId>(s));
				const int Sx = 26 + s * 198;
				const int Sy = 276;
				const bool bStageSel = SelectedStage == s;
				DrawRectangle(Sx, Sy, 190, 48, kPanel);
				DrawRectangleLinesEx(
					Rectangle{static_cast<float>(Sx), static_cast<float>(Sy), 190.0f, 48.0f},
					bStageSel ? 2.0f : 1.0f,
					bStageSel ? kYellow : kMuted);
				DrawText(SDef.name, Sx + 8, Sy + 8, 10, bStageSel ? kYellow : kWhite);
				DrawText(TextSubtext(SDef.subtitle,0,27), Sx + 8, Sy + 24, 8, kMuted);
			}

			const char* StepHint = SelectStep == 0 ? "PASSO 1/3: Escolha o Lutador P1 (A/D + ENTER/F)" :
				(SelectStep == 1 ? "PASSO 2/3: Escolha o Adversario (A/D + ENTER/F)" :
					"PASSO 3/3: Escolha a Arena (A/D + ENTER/F para iniciar a luta)");
			DrawText(StepHint, 26, 336, 10, kGreen);
		}
		else if (Screen == AppScreen::Match && World)
		{
			const RDA::WorldSnapshot Snap = World->Snapshot();
			StageRenderer.decorativeEffects=Settings.value.particles;
            StageRenderer.Update(static_cast<float>(FrameSeconds),Snap);
			StageRenderer.DrawBack(Snap);
			const bool VictimInStage = RDAStageVisuals::StageRenderer::VictimInStageLayer(Snap) && Snap.finisherWinner >= 0;
			if (VictimInStage)
			{
				const auto& Victim = Snap.fighters[static_cast<size_t>(1-Snap.finisherWinner)];
				DrawFighterOnStage(FighterVisuals[static_cast<size_t>(Victim.id)],RDA::GetFighterDef(Victim.id),Victim,Snap.debugBoxes);
			}
			StageRenderer.DrawFront(Snap);

			// Draw fighters
			const bool ReverseDraw = Snap.fighters[0].victimInFront || (Snap.fighters[1].state == RDA::FighterState::GrabbedFront && !Snap.fighters[1].victimInFront) || (Snap.fighters[1].state == RDA::FighterState::GrabbedLifted && !Snap.fighters[1].victimInFront);
			for (int DrawIndex = 0; DrawIndex < 2; ++DrawIndex)
			{
				const int i = ReverseDraw ? 1 - DrawIndex : DrawIndex;
				if (VictimInStage && i != Snap.finisherWinner) continue;
				const RDA::FighterSnapshot& FSnap = Snap.fighters[static_cast<size_t>(i)];
				const RDA::FighterDef& FDef = RDA::GetFighterDef(FSnap.id);
				const FighterVisualBank& FBank = FighterVisuals[static_cast<size_t>(FSnap.id)];
				DrawFighterOnStage(FBank, FDef, FSnap, Snap.debugBoxes);
			}

			// Draw active projectiles
			for (const RDA::ProjectileSnapshot& Proj : Snap.projectiles)
			{
				const auto& Owner = Snap.fighters[static_cast<size_t>(Proj.ownerIndex)];
				const auto& Bank = FighterVisuals[static_cast<size_t>(Owner.id)];
				bool Drawn = false;
				for (const auto& Entry : Bank.effects) if (std::strcmp(Entry.first.usage,"projectile")==0 && std::strcmp(Entry.first.move,Proj.sourceMoveId)==0 && Entry.second.id)
				{
					const auto& E=Entry.first; int Ef=(Proj.ageFrames/std::max(1,60/E.fps))%E.frames;
					DrawTexturePro(Entry.second, Rectangle{static_cast<float>(Ef*E.size),0,static_cast<float>(E.size*Proj.facing),static_cast<float>(E.size)}, Rectangle{Proj.x,Proj.y,E.size*E.scale,E.size*E.scale}, Vector2{E.size*E.scale/2,E.size*E.scale/2},0,WHITE);
					Drawn=true; break;
				}
				const Color PCol = Proj.ownerIndex == 0 ? kCyan : kPink;
				if (!Drawn) DrawCircle(static_cast<int>(Proj.x), static_cast<int>(Proj.y), Proj.active ? 10.0f : 6.0f, ColorAlpha(PCol, 0.85f));
				if (Snap.debugBoxes)
				{
					DrawRectangleLinesEx(
						Rectangle{Proj.worldBox.x, Proj.worldBox.y, Proj.worldBox.width, Proj.worldBox.height},
						1.0f, kYellow);
				}
			}

			StageRenderer.DrawOverlay(Snap);
			// Top HUD
			DrawRectangle(0, 0, RDA::kLogicalWidth, 54, kPanel);
			DrawLine(0, 54, RDA::kLogicalWidth, 54, Color{41, 217, 255, 95});

			const auto& F1 = Snap.fighters[0];
			const auto& F2 = Snap.fighters[1];
			const float R1 = F1.maxHealth > 0 ? static_cast<float>(F1.health) / static_cast<float>(F1.maxHealth) : 0.0f;
			const float R2 = F2.maxHealth > 0 ? static_cast<float>(F2.health) / static_cast<float>(F2.maxHealth) : 0.0f;

			DrawText(F1.name, 20, 6, 10, kCyan);
			DrawRectangle(20, 20, 230, 11, Color{24, 32, 52, 255});
			DrawRectangle(20, 20, static_cast<int>(230.0f * R1), 11, R1 < 0.25f ? kDanger : kCyan);
			DrawRectangleLines(20, 20, 230, 11, kWhite);
			DrawRectangle(20, 35, 150, 5, Color{20, 26, 42, 255});
			DrawRectangle(20, 35, static_cast<int>(1.5f * F1.meter), 5, F1.meter >= 50 ? kYellow : kGreen);

			const int Name2W = MeasureText(F2.name, 10);
			DrawText(F2.name, RDA::kLogicalWidth - 20 - Name2W, 6, 10, kPink);
			DrawRectangle(RDA::kLogicalWidth - 250, 20, 230, 11, Color{24, 32, 52, 255});
			const int W2 = static_cast<int>(230.0f * R2);
			DrawRectangle(RDA::kLogicalWidth - 20 - W2, 20, W2, 11, R2 < 0.25f ? kDanger : kPink);
			DrawRectangleLines(RDA::kLogicalWidth - 250, 20, 230, 11, kWhite);
			DrawRectangle(RDA::kLogicalWidth - 170, 35, 150, 5, Color{20, 26, 42, 255});
			const int M2 = static_cast<int>(1.5f * F2.meter);
			DrawRectangle(RDA::kLogicalWidth - 20 - M2, 35, M2, 5, F2.meter >= 50 ? kYellow : kGreen);

			for (int i = 0; i < RDA::kRoundsToWin; ++i)
			{
				DrawCircle(186 + i * 12, 38, 4.0f, i < F1.roundWins ? kYellow : kMuted);
				DrawCircle(RDA::kLogicalWidth - 186 - i * 12, 38, 4.0f, i < F2.roundWins ? kYellow : kMuted);
			}

			const char* TimeTxt = TextFormat("%02d", Snap.timeSeconds);
			const int TimeW = MeasureText(TimeTxt, 20);
			DrawText(TimeTxt, (RDA::kLogicalWidth - TimeW) / 2, 10, 20, kYellow);
			const char* RoundTxt = TextFormat("R%d", Snap.round);
			DrawText(RoundTxt, (RDA::kLogicalWidth - MeasureText(RoundTxt, 10)) / 2, 34, 10, kMuted);

			if (F1.comboCount >= 2)
			{
				DrawText(TextFormat("%d HITS!", F1.comboCount), 22, 62, 12, kYellow);
			}
			if (F2.comboCount >= 2)
			{
				DrawText(TextFormat("%d HITS!", F2.comboCount), RDA::kLogicalWidth - 96, 62, 12, kYellow);
			}

			// Training frame data overlay
			if (SelectedMode == RDA::GameMode::Training)
			{
				DrawRectangle(180, 58, 280, 28, Color{12, 18, 36, 195});
				DrawText(
					TextFormat("TREINO | Dano: %d | P1 Move: %s (%d/%df)",
						Snap.lastDamage,
						F1.attacking ? F1.moveId : "Idle",
						F1.moveFrame,
						F1.moveTotalFrames),
					188, 63, 8, kCyan);
				DrawText(
					TextFormat("Caixas (%s) | Resetar posicao | CPU (%s)",
						Snap.debugBoxes ? "ON" : "OFF",
						bTrainingCpuActive ? "ON" : "OFF"),
					188, 74, 8, kMuted);
			}

            if(SelectedMode==RDA::GameMode::Training) {
                DrawRectangle(180,87,85,23,kPanel);DrawText("CAIXAS",192,94,9,kCyan);
                DrawRectangle(270,87,85,23,kPanel);DrawText("RESET",285,94,9,kWhite);
                DrawRectangle(360,87,90,23,kPanel);DrawText("CPU ON/OFF",367,94,8,kYellow);
            }
			std::string DisplayBanner = Snap.banner ? Snap.banner : "";
			if (Snap.phase==RDA::CombatPhase::Intro) DisplayBanner=Snap.phaseFrame<60 ? "ROUND "+std::to_string(Snap.round) : "FIGHT";
			else if (Snap.phase==RDA::CombatPhase::FinishReady) DisplayBanner="FINALIZE!  FRACO + FORTE  "+std::to_string(std::max(0,(480-Snap.phaseFrame+59)/60));
			else if (Snap.phase==RDA::CombatPhase::StageFinish)
			{
				const int Threshold=Snap.stage==RDA::StageId::CozinhaMacabra?230:Snap.stage==RDA::StageId::Sitio?282:238;
				DisplayBanner=Snap.phaseFrame<Threshold?"":Snap.stage==RDA::StageId::CozinhaMacabra?"O JANTAR ESTA SERVIDO":Snap.stage==RDA::StageId::Sitio?"A COLHEITA ESTA FEITA":"O CAIS COBRA SUA ALMA";
			}
			else if (Snap.phase==RDA::CombatPhase::MatchOver && Snap.matchWinner>=0) DisplayBanner=std::string(Snap.fighters[static_cast<size_t>(Snap.matchWinner)].name)+" CAMPEAO";
			if (!DisplayBanner.empty() && !Snap.paused)
			{
				const int BW = MeasureText(DisplayBanner.c_str(), 18);
				const int Y=(Snap.phase==RDA::CombatPhase::FinishReady || Snap.phase==RDA::CombatPhase::StageFinish)?61:139;
				DrawRectangle((RDA::kLogicalWidth - BW) / 2 - 14, Y, BW + 28, 34, kPanel);
				DrawRectangleLines((RDA::kLogicalWidth - BW) / 2 - 14, Y, BW + 28, 34, kYellow);
				DrawText(DisplayBanner.c_str(), (RDA::kLogicalWidth - BW) / 2, Y+8, 18, kYellow);
			}
			if(Snap.phase==RDA::CombatPhase::MatchOver) DrawText("LEVE / CONFIRMAR: REVANCHE | PAUSA: MENU",182,342,8,kWhite);

			if (Snap.paused)
			{
				DrawRectangle(140, 88, 360, 190, Color{8, 12, 26, 240});
				DrawRectangleLinesEx(Rectangle{140, 88, 360, 190}, 2.0f, kCyan);
				DrawText("JOGO PAUSADO", 252, 98, 16, kYellow);
				if (!bShowMoveList)
				{
					DrawText("CONTINUAR A LUTA", 164, 128, 10, kWhite);
					DrawText("LISTA DE GOLPES", 164, 148, 10, kCyan);
					DrawText("ALTERNAR CAIXAS DE COLISAO", 164, 168, 10, kWhite);
					DrawText("CONFIGURACOES: TELA, AUDIO E CONTROLES",164,188,10,kWhite);
					DrawText("ESCOLHER LUTADORES E ARENA", 164, 208, 10, kYellow);
					DrawText("VOLTAR AO MENU PRINCIPAL", 164, 228, 10, kPink);
				}
				else
				{
					const RDA::FighterDef& P1Def = RDA::GetFighterDef(F1.id);
					DrawText(TextFormat("LISTA DE GOLPES — %s", P1Def.name), 160, 122, 10, kCyan);
					DrawText("LEVE / FORTE / DEFESA: botoes na tela ou seu controle",160,138,8,kWhite);
					DrawText("AGARRAR: AG | SUPER: SUP | Veja suas teclas nas configuracoes",160,151,8,kGreen);
					int Row=0;
					for(const auto& Move:P1Def.canonicalMoves) if((Move.isSpecial || Move.isSuper) && Row<3)
					{
						std::string Command;
						const char* Digits[]={"5","2","3","1","6","4","8"};
						for(int D:Move.command.directions) if(D>=0 && D<7) Command+=Digits[D];
						if(Command.empty()) Command="NEUTRO";
						Command+=" + ESPECIAL";
						DrawText(TextFormat("%s%s",Move.label,Move.isSuper?" [50 BARRA]":""),160,168+Row*24,8,Move.isSuper?kPink:kYellow);
						DrawText(Command.c_str(),160,179+Row*24,8,kWhite); ++Row;
					}
					DrawText("236 = BAIXO > BAIXO+FRENTE > FRENTE | 214 = PARA TRAS",160,243,8,kMuted);
					DrawText("FINALIZE: perto + AGARRAR | Toque na lista para voltar",160,256,8,kCyan);
				}
			}

			DrawTouchOverlay(bShowTouchOverlay, P1);
			if (bShowTouchOverlay) { DrawRectangle(590,0,50,40,kPanel); DrawText("II",608,12,16,kWhite); }
		}

        if(Screen!=AppScreen::Match){DrawRectangle(548,326,88,30,kPanel);DrawText("CONFIGURAR",555,337,10,kCyan);}
        rlDrawRenderBatchActive();rlPopMatrix();
		EndTextureMode();
		const int FinisherFrame = !RenderSmokeDirectory.empty() && World ? World->Snapshot().visualFinisher.frame : -1;
		const bool CaptureFinisher = SmokeFinisher>=0 && FinisherFrame!=LastSmokeFinisherFrame && (FinisherFrame==34 || FinisherFrame==72 || FinisherFrame==112 || FinisherFrame==144 || FinisherFrame==190 || FinisherFrame==260 || FinisherFrame==300);
		if (!RenderSmokeDirectory.empty() && (GlobalTicks <= 2 || (GlobalTicks<1083 && (GlobalTicks-3)%180 == 120) || CaptureFinisher))
		{
			Image Capture = LoadImageFromTexture(Canvas.texture); ImageFlipVertical(&Capture);
			const std::string Name = CaptureFinisher ? std::string("finisher-")+RDA::GetStageDef(static_cast<RDA::StageId>(SelectedStage)).canonicalId+"-"+std::to_string(FinisherFrame)+".png" : GlobalTicks==1 ? "main-menu.png" : GlobalTicks==2 ? "character-select.png" : std::string("native-")+RDA::GetFighterDef(static_cast<RDA::FighterId>(SelectedP1)).canonicalId+".png";
			const bool Written = ExportImage(Capture,(fs::path(RenderSmokeDirectory)/Name).string().c_str()); UnloadImage(Capture);
			if (!Written) return 4;
			if (CaptureFinisher) LastSmokeFinisherFrame=FinisherFrame;
		}

		BeginDrawing();
		ClearBackground(BLACK);
		Rectangle Src{0.0f, 0.0f, static_cast<float>(Canvas.texture.width), -static_cast<float>(Canvas.texture.height)};
		Rectangle PresentationViewport=DestViewport;
        if(Settings.value.shake && Screen==AppScreen::Match && World && !World->IsPaused())PresentationViewport.x+=std::sin(GetTime()*70)*CameraTrauma*18;
        DrawTexturePro(Canvas.texture, Src, PresentationViewport, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
        if(Settings.value.brightness<1)DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),Fade(BLACK,1-Settings.value.brightness));
        else if(Settings.value.brightness>1)DrawRectangle(0,0,GetScreenWidth(),GetScreenHeight(),Fade(WHITE,(Settings.value.brightness-1)*.45f));
        if(Settings.value.showFps)DrawFPS(8,GetScreenHeight()-22);

        Settings.Draw();
        #if defined(PLATFORM_ANDROID)
        if(GlobalTicks<=3)TraceLog(LOG_INFO,"RDA_PRESENTED_FRAME: %d screen=%dx%d render=%dx%d viewport=%.0fx%.0f focused=%d",GlobalTicks,GetScreenWidth(),GetScreenHeight(),GetRenderWidth(),GetRenderHeight(),DestViewport.width,DestViewport.height,IsWindowFocused());
        #endif
		EndDrawing();
        if(!SettingsSmokeDirectory.empty() && GlobalTicks%2==0){Image capture=LoadImageFromScreen();std::string file=(fs::path(SettingsSmokeDirectory)/("settings-"+std::to_string((GlobalTicks-1)/2)+".png")).string();bool saved=ExportImage(capture,file.c_str());UnloadImage(capture);if(!saved)return 9;if(GlobalTicks>=10)break;}

	}

	if (bHasMenuMusic) UnloadMusicStream(MenuMusic);
	StageRenderer.Unload();
	for (StageVisualBank& SVis : StageVisuals)
	{
		if (SVis.background.id != 0) UnloadTexture(SVis.background);
		if (SVis.finisherProp1.id != 0) UnloadTexture(SVis.finisherProp1);
		if (SVis.finisherProp2.id != 0) UnloadTexture(SVis.finisherProp2);
		if (SVis.hasMusic) UnloadMusicStream(SVis.music);
	}
	for (FighterVisualBank& FVis : FighterVisuals)
	{
		if (FVis.portrait.id != 0) UnloadTexture(FVis.portrait);
		for (auto& Entry : FVis.animations) if (Entry.second.texture.id) UnloadTexture(Entry.second.texture);
		for (auto& Entry : FVis.effects) if (Entry.second.id) UnloadTexture(Entry.second);
	}
	if (LogoTex.id != 0) UnloadTexture(LogoTex);
	Sounds.Unload();
	UnloadRenderTexture(Canvas);
	if (IsAudioDeviceReady()) CloseAudioDevice();
#if defined(_WIN32)
    RDAOnline::Close();
#endif
	CloseWindow();
	return 0;
}
