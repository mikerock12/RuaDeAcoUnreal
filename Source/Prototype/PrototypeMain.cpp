#include "RDACombatWorld.h"
#include "raylib.h"

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

namespace
{
	namespace fs = std::filesystem;

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
	};

	struct StageVisualBank
	{
		Texture2D background{};
		Texture2D finisherProp1{};
		Texture2D finisherProp2{};
		Music music{};
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
		if (fs::exists(Path))
		{
			Sound S = LoadSound(Path.string().c_str());
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
		return Bank;
	}

	void PlayWorldSound(const SoundBank& Bank, RDA::SoundEvent Ev, RDA::FighterId ActiveFighter)
	{
		if (!Bank.audioReady) return;
		const size_t FIdx = static_cast<size_t>(ActiveFighter) % RDA::kFighterCount;
		switch (Ev)
		{
		case RDA::SoundEvent::Fight: PlaySound(Bank.fight); break;
		case RDA::SoundEvent::Swing: PlaySound(Bank.swing); break;
		case RDA::SoundEvent::HitLight: PlaySound(Bank.hitLight); break;
		case RDA::SoundEvent::HitHeavy: PlaySound(Bank.hitHeavy); break;
		case RDA::SoundEvent::Block: PlaySound(Bank.block); break;
		case RDA::SoundEvent::Jump: PlaySound(Bank.jump); break;
		case RDA::SoundEvent::Knockdown:
		case RDA::SoundEvent::GrabSlam: PlaySound(Bank.knockdown); break;
		case RDA::SoundEvent::GrabStart: PlaySound(Bank.hitLight); break;
		case RDA::SoundEvent::SpecialCast: PlaySound(Bank.specialSfx[FIdx]); break;
		case RDA::SoundEvent::SuperCast: PlaySound(Bank.superSfx[FIdx]); break;
		case RDA::SoundEvent::FinishReady: PlaySound(Bank.finalize); break;
		case RDA::SoundEvent::FinisherMonsterRoar:
		case RDA::SoundEvent::FinisherPotDrop:
		case RDA::SoundEvent::FinisherTrident: PlaySound(Bank.monsterRoar); break;
		case RDA::SoundEvent::FinisherSplash: PlaySound(Bank.hitHeavy); break;
		case RDA::SoundEvent::RoundEnd: PlaySound(Bank.roundEnd); break;
		case RDA::SoundEvent::MatchWin: PlaySound(Bank.matchWin); break;
		default: break;
		}
	}

	fs::path FindContentRoot()
	{
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
		if (!fs::exists(Path))
		{
			return Texture2D{};
		}
		Texture2D Tex = LoadTexture(Path.string().c_str());
		if (Tex.id != 0)
		{
			SetTextureFilter(Tex, TEXTURE_FILTER_POINT);
		}
		return Tex;
	}

	FighterVisualBank LoadFighterVisuals(const fs::path& ArtRoot, const RDA::FighterDef& Def)
	{
		FighterVisualBank Out{};
		Out.portrait = LoadPixelTexture(ArtRoot / "Portraits" / Def.portraitFile);
		const fs::path CharDir = ArtRoot / "Characters" / Def.assetFolder;

		for (const AnimSpec& Spec : kCommonSpecs)
		{
			SpriteSheet& Sheet = Out.sheets[static_cast<size_t>(Spec.id)];
			Sheet.frameSize = Def.spriteFrameSize;
			Sheet.fps = Spec.fps;
			Sheet.loop = Spec.loop;
			fs::path FilePath = CharDir / Spec.file;
			if (!fs::exists(FilePath))
			{
				FilePath = CharDir / "standing-heavy.png";
			}
			Sheet.texture = LoadPixelTexture(FilePath);
			if (Sheet.texture.id != 0 && Sheet.texture.height > 0)
			{
				Sheet.frameSize = Sheet.texture.height;
				Sheet.frames = std::max(1, Sheet.texture.width / std::max(1, Sheet.frameSize));
			}
		}

		// Map specials and super to fallback/custom sheets
		Out.sheets[static_cast<size_t>(RDA::AnimId::Special1)] = Out.sheets[static_cast<size_t>(RDA::AnimId::ForwardHeavy)];
		Out.sheets[static_cast<size_t>(RDA::AnimId::Special2)] = Out.sheets[static_cast<size_t>(RDA::AnimId::StandingHeavy)];
		Out.sheets[static_cast<size_t>(RDA::AnimId::Super)] = Out.sheets[static_cast<size_t>(RDA::AnimId::StandingHeavy)];
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
		if (std::fabs(RotationRad) > 0.01f)
		{
			Rectangle Dest{WorldX, WorldY - DestSize * 0.35f, DestSize, DestSize};
			Vector2 Origin{DestSize * 0.5f, DestSize * 0.5f};
			DrawTexturePro(Sheet.texture, Source, Dest, Origin, RotationRad * (180.0f / 3.14159265f), WHITE);
		}
		else
		{
			Rectangle Dest{
				std::round(WorldX - DestSize * 0.5f),
				std::round(WorldY - DestSize + static_cast<float>(SolePad) * Scale),
				DestSize,
				DestSize
			};
			DrawTexturePro(Sheet.texture, Source, Dest, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
		}
	}

	void DrawFighterOnStage(
		const FighterVisualBank& Bank,
		const RDA::FighterDef& Def,
		const RDA::FighterSnapshot& Snap,
		bool DrawBoxes)
	{
		if (!Snap.visible) return;
		DrawEllipse(static_cast<int>(Snap.x), RDA::kGroundY + 2, 26, 6, Color{0, 0, 0, 120});

		const SpriteSheet& Sheet = Bank.sheets[static_cast<size_t>(Snap.anim)];
		int Frame = 0;
		if (Sheet.frames > 1)
		{
			if (Snap.attacking && Snap.moveTotalFrames > 0)
			{
				RDA::MoveDef Move{};
				Move.totalFrames = Snap.moveTotalFrames;
				Move.hitbox.activeFrom = Snap.hitActiveFrom;
				Move.hitbox.activeTo = Snap.hitActiveTo;
				Frame = RDA::MoveAnimFrame(Move, Snap.moveFrame, Sheet.frames);
			}
			else
			{
				const int TicksPerFrame = std::max(1, 60 / std::max(1, Sheet.fps));
				Frame = Snap.stateFrame / TicksPerFrame;
				Frame = Sheet.loop ? (Frame % Sheet.frames) : std::min(Sheet.frames - 1, Frame);
			}
		}

		DrawSpriteSheet(Sheet, Snap.x, Snap.y, Snap.facing, Frame, Def.visualSolePad, Snap.victimRotation, 1.0f);

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
		const int PadX = 72;
		const int PadY = 294;
		DrawCircle(PadX, PadY, 42.0f, Color{12, 18, 36, 120});
		DrawCircleLines(PadX, PadY, 42.0f, Color{41, 217, 255, 160});
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
			{500, 268, "GRAB", kWhite, ActiveInput.grabPressed},
			{500, 320, "SUP", kDanger, ActiveInput.superPressed},
		};
		for (const BtnSpec& B : Btns)
		{
			DrawCircle(B.x, B.y, 16.0f, B.active ? ColorAlpha(B.col, 0.65f) : Color{12, 18, 36, 130});
			DrawCircleLines(B.x, B.y, 16.0f, B.col);
			const int Tw = MeasureText(B.label, 6);
			DrawText(B.label, B.x - Tw / 2, B.y - 3, 6, kWhite);
		}
	}

	void ReadTouchInput(RDA::InputFrame& OutP1, const Rectangle& DestViewport, bool& OutTouchDetected)
	{
		Vector2 Pointer{-1.0f, -1.0f};
		bool bPressed = false;
		bool bDown = false;

		if (GetTouchPointCount() > 0)
		{
			Pointer = GetTouchPosition(0);
			bDown = true;
			bPressed = IsGestureDetected(GESTURE_TAP) || IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
			OutTouchDetected = true;
		}
		else if (IsMouseButtonDown(MOUSE_LEFT_BUTTON))
		{
			Pointer = GetMousePosition();
			bDown = true;
			bPressed = IsMouseButtonPressed(MOUSE_LEFT_BUTTON);
		}

		if (!bDown || DestViewport.width <= 0.0f || DestViewport.height <= 0.0f) return;

		const float Lx = (Pointer.x - DestViewport.x) * (static_cast<float>(RDA::kLogicalWidth) / DestViewport.width);
		const float Ly = (Pointer.y - DestViewport.y) * (static_cast<float>(RDA::kLogicalHeight) / DestViewport.height);

		// D-pad area
		const float Dx = Lx - 72.0f;
		const float Dy = Ly - 294.0f;
		if (Dx * Dx + Dy * Dy <= 54.0f * 54.0f)
		{
			if (Dx < -10.0f) OutP1.left = true;
			if (Dx > 10.0f) OutP1.right = true;
			if (Dy < -14.0f) { OutP1.up = true; if (bPressed) OutP1.upPressed = true; }
			if (Dy > 14.0f) { OutP1.down = true; if (bPressed) OutP1.downPressed = true; }
		}

		auto HitBtn = [&](float Bx, float By) -> bool
		{
			const float Rx = Lx - Bx;
			const float Ry = Ly - By;
			return (Rx * Rx + Ry * Ry) <= 20.0f * 20.0f;
		};
		if (HitBtn(536.0f, 294.0f)) { OutP1.punch = true; if (bPressed) OutP1.punchPressed = true; }
		if (HitBtn(572.0f, 268.0f)) { OutP1.kick = true; if (bPressed) OutP1.kickPressed = true; }
		if (HitBtn(604.0f, 294.0f)) { OutP1.special = true; if (bPressed) OutP1.specialPressed = true; }
		if (HitBtn(572.0f, 320.0f)) { OutP1.block = true; if (bPressed) OutP1.blockPressed = true; }
		if (HitBtn(500.0f, 268.0f) && bPressed) { OutP1.grabPressed = true; OutP1.punchPressed = true; OutP1.kick = true; }
		if (HitBtn(500.0f, 320.0f) && bPressed) { OutP1.superPressed = true; OutP1.down = true; OutP1.specialPressed = true; }
	}

	RDA::InputFrame ReadPlayerOneInput(const Rectangle& DestViewport, bool& InOutTouchActive)
	{
		RDA::InputFrame Out{};
		Out.left = IsKeyDown(KEY_A);
		Out.right = IsKeyDown(KEY_D);
		Out.up = IsKeyDown(KEY_W);
		Out.down = IsKeyDown(KEY_S);
		Out.punch = IsKeyDown(KEY_F);
		Out.kick = IsKeyDown(KEY_G);
		Out.special = IsKeyDown(KEY_H);
		Out.block = IsKeyDown(KEY_R);

		Out.upPressed = IsKeyPressed(KEY_W);
		Out.downPressed = IsKeyPressed(KEY_S);
		Out.leftPressed = IsKeyPressed(KEY_A);
		Out.rightPressed = IsKeyPressed(KEY_D);
		Out.punchPressed = IsKeyPressed(KEY_F);
		Out.kickPressed = IsKeyPressed(KEY_G);
		Out.specialPressed = IsKeyPressed(KEY_H);
		Out.blockPressed = IsKeyPressed(KEY_R);
		Out.grabPressed = IsKeyPressed(KEY_T) || (IsKeyDown(KEY_F) && IsKeyPressed(KEY_G)) || (IsKeyPressed(KEY_F) && IsKeyDown(KEY_G));
		Out.superPressed = IsKeyPressed(KEY_Y);
		Out.pausePressed = IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_P);
		Out.confirmPressed = IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_SPACE);
		Out.debugPressed = IsKeyPressed(KEY_F1);

		if (IsGamepadAvailable(0))
		{
			const float AxisX = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_X);
			const float AxisY = GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y);
			Out.left = Out.left || AxisX < -0.35f || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_LEFT);
			Out.right = Out.right || AxisX > 0.35f || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_RIGHT);
			Out.up = Out.up || AxisY < -0.45f || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_UP);
			Out.down = Out.down || AxisY > 0.45f || IsGamepadButtonDown(0, GAMEPAD_BUTTON_LEFT_FACE_DOWN);
			Out.punch = Out.punch || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
			Out.kick = Out.kick || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_UP);
			Out.special = Out.special || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
			Out.block = Out.block || IsGamepadButtonDown(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);

			Out.upPressed = Out.upPressed || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_LEFT_FACE_UP);
			Out.punchPressed = Out.punchPressed || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_LEFT);
			Out.kickPressed = Out.kickPressed || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_UP);
			Out.specialPressed = Out.specialPressed || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT);
			Out.blockPressed = Out.blockPressed || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
			Out.grabPressed = Out.grabPressed || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_1);
			Out.superPressed = Out.superPressed || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_TRIGGER_2);
			Out.pausePressed = Out.pausePressed || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_MIDDLE_RIGHT);
			Out.confirmPressed = Out.confirmPressed || IsGamepadButtonPressed(0, GAMEPAD_BUTTON_RIGHT_FACE_DOWN);
		}

		ReadTouchInput(Out, DestViewport, InOutTouchActive);
		return Out;
	}

	RDA::InputFrame ReadPlayerTwoKeyboardInput()
	{
		RDA::InputFrame Out{};
		Out.left = IsKeyDown(KEY_LEFT);
		Out.right = IsKeyDown(KEY_RIGHT);
		Out.up = IsKeyDown(KEY_UP);
		Out.down = IsKeyDown(KEY_DOWN);
		Out.punch = IsKeyDown(KEY_J);
		Out.kick = IsKeyDown(KEY_K);
		Out.special = IsKeyDown(KEY_L);
		Out.block = IsKeyDown(KEY_U);

		Out.upPressed = IsKeyPressed(KEY_UP);
		Out.downPressed = IsKeyPressed(KEY_DOWN);
		Out.leftPressed = IsKeyPressed(KEY_LEFT);
		Out.rightPressed = IsKeyPressed(KEY_RIGHT);
		Out.punchPressed = IsKeyPressed(KEY_J);
		Out.kickPressed = IsKeyPressed(KEY_K);
		Out.specialPressed = IsKeyPressed(KEY_L);
		Out.blockPressed = IsKeyPressed(KEY_U);
		Out.grabPressed = IsKeyPressed(KEY_I) || (IsKeyDown(KEY_J) && IsKeyPressed(KEY_K)) || (IsKeyPressed(KEY_J) && IsKeyDown(KEY_K));
		Out.superPressed = IsKeyPressed(KEY_O);
		return Out;
	}
}

int main(int argc, char** argv)
{
	for (int i = 1; i < argc; ++i)
	{
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

	SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
	InitWindow(1280, 720, "Rua de Aco — Edicao Nativa C++20 (6 Lutadores / 3 Arenas)");
	SetExitKey(KEY_NULL);
	SetTargetFPS(60);
	InitAudioDevice();

	RenderTexture2D Canvas = LoadRenderTexture(RDA::kLogicalWidth, RDA::kLogicalHeight);
	SetTextureFilter(Canvas.texture, TEXTURE_FILTER_POINT);

	const fs::path ContentRoot = FindContentRoot();
	const fs::path ArtRoot = ContentRoot / "RuaDeAco" / "Art";
	const fs::path MusicRoot = ContentRoot / "RuaDeAco" / "Audio" / "Music";
	SoundBank Sounds = BuildSoundBank(ContentRoot);

	Texture2D LogoTex = LoadPixelTexture(ArtRoot / "UI" / "rua-de-aco-logo.png");
	std::array<FighterVisualBank, RDA::kFighterCount> FighterVisuals{};
	for (int i = 0; i < RDA::kFighterCount; ++i)
	{
		FighterVisuals[static_cast<size_t>(i)] = LoadFighterVisuals(ArtRoot, RDA::GetFighterDef(static_cast<RDA::FighterId>(i)));
	}

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
		if (IsAudioDeviceReady() && fs::exists(MusicPath))
		{
			SVis.music = LoadMusicStream(MusicPath.string().c_str());
			SVis.hasMusic = SVis.music.frameCount > 0;
			if (SVis.hasMusic)
			{
				SetMusicVolume(SVis.music, 0.35f);
			}
		}
	}

	Music MenuMusic{};
	bool bHasMenuMusic = false;
	if (IsAudioDeviceReady() && fs::exists(MusicRoot / "menu-principal.ogg"))
	{
		MenuMusic = LoadMusicStream((MusicRoot / "menu-principal.ogg").string().c_str());
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
	bool bShowTouchOverlay = false;
	bool bShowMoveList = false;
	bool bTrainingCpuActive = false;
	int GlobalTicks = 0;

	std::unique_ptr<RDA::CombatWorld> World;

	while (!WindowShouldClose())
	{
		GlobalTicks += 1;
		if (IsKeyPressed(KEY_F11) || (IsKeyDown(KEY_LEFT_ALT) && IsKeyPressed(KEY_ENTER)))
		{
			ToggleFullscreen();
		}
		if (IsKeyPressed(KEY_F4))
		{
			bShowTouchOverlay = !bShowTouchOverlay;
		}

		const float ScaleX = static_cast<float>(GetScreenWidth()) / static_cast<float>(RDA::kLogicalWidth);
		const float ScaleY = static_cast<float>(GetScreenHeight()) / static_cast<float>(RDA::kLogicalHeight);
		const float Scale = std::max(1.0f, std::min(ScaleX, ScaleY));
		const float DrawW = static_cast<float>(RDA::kLogicalWidth) * Scale;
		const float DrawH = static_cast<float>(RDA::kLogicalHeight) * Scale;
		const Rectangle DestViewport{
			std::floor((static_cast<float>(GetScreenWidth()) - DrawW) * 0.5f),
			std::floor((static_cast<float>(GetScreenHeight()) - DrawH) * 0.5f),
			DrawW,
			DrawH
		};

		const RDA::InputFrame P1 = ReadPlayerOneInput(DestViewport, bShowTouchOverlay);
		const RDA::InputFrame P2Key = ReadPlayerTwoKeyboardInput();

		if (Screen != AppScreen::Match && bHasMenuMusic)
		{
			UpdateMusicStream(MenuMusic);
		}

		if (Screen == AppScreen::MainMenu)
		{
			if (P1.upPressed || P2Key.upPressed) MenuCursor = (MenuCursor + 3) % 4;
			if (P1.downPressed || P2Key.downPressed) MenuCursor = (MenuCursor + 1) % 4;
			if (P1.leftPressed || P1.rightPressed)
			{
				const int NextDiff = (static_cast<int>(SelectedDifficulty) + 1) % 3;
				SelectedDifficulty = static_cast<RDA::AiDifficulty>(NextDiff);
			}
			if (P1.confirmPressed || P1.punchPressed)
			{
				if (MenuCursor == 3)
				{
					break;
				}
				SelectedMode = MenuCursor == 0 ? RDA::GameMode::VsCpu :
					(MenuCursor == 1 ? RDA::GameMode::LocalVersus : RDA::GameMode::Training);
				SelectStep = 0;
				Screen = AppScreen::CharacterSelect;
				PlayWorldSound(Sounds, RDA::SoundEvent::HitLight, RDA::FighterId::GutoBarba);
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
					World = std::make_unique<RDA::CombatWorld>(F1, F2, SelectedMode, Stage);
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

				World->Step(P1, P2);
				for (RDA::SoundEvent Ev : World->DrainSounds())
				{
					PlayWorldSound(Sounds, Ev, static_cast<RDA::FighterId>(SelectedP1));
				}
			}
		}

		BeginTextureMode(Canvas);
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

			DrawRectangle(160, 118, 320, 184, kPanel);
			DrawRectangleLinesEx(Rectangle{160, 118, 320, 184}, 1.0f, kCyan);

			const char* DiffNames[] = {"FACIL", "NORMAL", "DIFICIL"};
			const char* Items[] = {
				TextFormat("1. VS CPU (%s)", DiffNames[static_cast<int>(SelectedDifficulty)]),
				"2. VERSUS LOCAL (2 JOGADORES)",
				"3. MODO TREINAMENTO (FRAME DATA + BOXES)",
				"4. SAIR PARA O DESKTOP"
			};
			for (int i = 0; i < 4; ++i)
			{
				const bool bSel = MenuCursor == i;
				const int Y = 136 + i * 36;
				if (bSel)
				{
					DrawRectangle(174, Y - 4, 292, 24, Color{41, 217, 255, 45});
					DrawRectangleLinesEx(Rectangle{174.0f, static_cast<float>(Y - 4), 292.0f, 24.0f}, 1.0f, kYellow);
				}
				DrawText(Items[i], 186, Y + 2, 10, bSel ? kYellow : kWhite);
			}

			DrawText("W/S ou Setas: Navegar | A/D: Dificuldade CPU | ENTER/F: Confirmar | F4: Touch HUD | F11: Tela Cheia",
				38, 336, 10, kMuted);
		}
		else if (Screen == AppScreen::CharacterSelect)
		{
			DrawRectangle(0, 0, RDA::kLogicalWidth, RDA::kLogicalHeight, Color{8, 12, 26, 255});
			DrawText("SELECAO DE LUTADORES E ARENA", 172, 14, 18, kCyan);

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
			DrawText(TextFormat("Especiais: %s | %s", F1.ability1, F1.ability2), 34, 198, 8, kWhite);
			DrawText(TextFormat("Super (50 barra): %s", F1.ability3), 34, 212, 8, kYellow);
			DrawText("Agarrao Universal: F+G / T", 34, 226, 8, kGreen);

			DrawRectangle(328, 174, 286, 76, kPanel);
			DrawRectangleLinesEx(Rectangle{328, 174, 286, 76}, 1.0f, kPink);
			DrawText(TextFormat("%s: %s (%s)", SelectedMode == RDA::GameMode::VsCpu ? "CPU" : "P2", F2.name, F2.archetype), 336, 182, 10, kPink);
			DrawText(TextFormat("Especiais: %s | %s", F2.ability1, F2.ability2), 336, 198, 8, kWhite);
			DrawText(TextFormat("Super (50 barra): %s", F2.ability3), 336, 212, 8, kYellow);
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
				DrawText(SDef.subtitle, Sx + 8, Sy + 24, 8, kMuted);
			}

			const char* StepHint = SelectStep == 0 ? "PASSO 1/3: Escolha o Lutador P1 (A/D + ENTER/F)" :
				(SelectStep == 1 ? "PASSO 2/3: Escolha o Adversario (A/D + ENTER/F)" :
					"PASSO 3/3: Escolha a Arena (A/D + ENTER/F para iniciar a luta)");
			DrawText(StepHint, 26, 336, 10, kGreen);
		}
		else if (Screen == AppScreen::Match && World)
		{
			const RDA::WorldSnapshot Snap = World->Snapshot();
			const StageVisualBank& SVis = StageVisuals[static_cast<size_t>(Snap.stage)];

			if (SVis.background.id != 0)
			{
				DrawTexturePro(
					SVis.background,
					Rectangle{0, 0, static_cast<float>(SVis.background.width), static_cast<float>(SVis.background.height)},
					Rectangle{0, 0, static_cast<float>(RDA::kLogicalWidth), static_cast<float>(RDA::kLogicalHeight)},
					Vector2{0, 0}, 0.0f, WHITE);
			}
			DrawLine(RDA::kStageLeft, RDA::kGroundY, RDA::kStageRight, RDA::kGroundY, Color{41, 217, 255, 85});

			// Stage finisher visual choreography
			if (Snap.phase == RDA::CombatPhase::StageFinish)
			{
				if (Snap.stage == RDA::StageId::CaisDaCidade && SVis.finisherProp1.id != 0 && Snap.phaseFrame >= 110)
				{
					const int MonsterFrames = 12;
					const int FrameW = std::max(1, SVis.finisherProp1.width / MonsterFrames);
					const int MFrame = std::clamp((Snap.phaseFrame - 110) / 12, 0, MonsterFrames - 1);
					DrawTexturePro(
						SVis.finisherProp1,
						Rectangle{static_cast<float>(MFrame * FrameW), 0.0f, static_cast<float>(FrameW), static_cast<float>(SVis.finisherProp1.height)},
						Rectangle{260.0f, 110.0f, 180.0f, 180.0f},
						Vector2{0, 0}, 0.0f, WHITE);
				}
				else if (Snap.stage == RDA::StageId::CozinhaMacabra)
				{
					DrawCircle(442, 162, 34.0f, Color{92, 242, 155, 120});
					if (SVis.finisherProp2.id != 0 && Snap.phaseFrame >= 200)
					{
						DrawTexturePro(
							SVis.finisherProp2,
							Rectangle{0, 0, static_cast<float>(SVis.finisherProp2.width), static_cast<float>(SVis.finisherProp2.height)},
							Rectangle{418.0f, 128.0f, 48.0f, 48.0f},
							Vector2{0, 0}, 0.0f, WHITE);
					}
				}
				else if (Snap.stage == RDA::StageId::Sitio && SVis.finisherProp1.id != 0 && Snap.phaseFrame >= 60)
				{
					const int FFrameW = std::max(1, SVis.finisherProp1.width / 3);
					DrawTexturePro(
						SVis.finisherProp1,
						Rectangle{0, 0, static_cast<float>(FFrameW), static_cast<float>(SVis.finisherProp1.height)},
						Rectangle{296.0f, 136.0f, 64.0f, 80.0f},
						Vector2{0, 0}, 0.0f, WHITE);
				}
			}

			// Draw fighters
			for (int i = 0; i < 2; ++i)
			{
				const RDA::FighterSnapshot& FSnap = Snap.fighters[static_cast<size_t>(i)];
				const RDA::FighterDef& FDef = RDA::GetFighterDef(FSnap.id);
				const FighterVisualBank& FBank = FighterVisuals[static_cast<size_t>(FSnap.id)];
				DrawFighterOnStage(FBank, FDef, FSnap, Snap.debugBoxes);
			}

			// Draw active projectiles
			for (const RDA::ProjectileSnapshot& Proj : Snap.projectiles)
			{
				const Color PCol = Proj.ownerIndex == 0 ? kCyan : kPink;
				DrawCircle(static_cast<int>(Proj.x), static_cast<int>(Proj.y), Proj.active ? 10.0f : 6.0f, ColorAlpha(PCol, 0.85f));
				DrawCircleLines(static_cast<int>(Proj.x), static_cast<int>(Proj.y), 12.0f, kWhite);
				if (Snap.debugBoxes)
				{
					DrawRectangleLinesEx(
						Rectangle{Proj.worldBox.x, Proj.worldBox.y, Proj.worldBox.width, Proj.worldBox.height},
						1.0f, kYellow);
				}
			}

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
					TextFormat("F1: Hitboxes (%s) | F2: Reset Pos | F3: Dummy CPU (%s)",
						Snap.debugBoxes ? "ON" : "OFF",
						bTrainingCpuActive ? "ON" : "OFF"),
					188, 74, 8, kMuted);
			}

			if (Snap.banner && Snap.banner[0] != '\0')
			{
				const int BW = MeasureText(Snap.banner, 18);
				DrawRectangle((RDA::kLogicalWidth - BW) / 2 - 14, 122, BW + 28, 34, kPanel);
				DrawRectangleLines((RDA::kLogicalWidth - BW) / 2 - 14, 122, BW + 28, 34, kYellow);
				DrawText(Snap.banner, (RDA::kLogicalWidth - BW) / 2, 130, 18, kYellow);
			}

			if (Snap.paused)
			{
				DrawRectangle(140, 88, 360, 190, Color{8, 12, 26, 240});
				DrawRectangleLinesEx(Rectangle{140, 88, 360, 190}, 2.0f, kCyan);
				DrawText("JOGO PAUSADO", 252, 98, 16, kYellow);
				if (!bShowMoveList)
				{
					DrawText("ESC / P : Continuar a Luta", 164, 128, 10, kWhite);
					DrawText("M       : Ver Lista de Golpes dos Lutadores", 164, 148, 10, kCyan);
					DrawText("F1      : Alternar Visualizador de Hitboxes", 164, 168, 10, kWhite);
					DrawText("F4      : Alternar Controles Touch na Tela", 164, 188, 10, kWhite);
					DrawText("C       : Voltar para Selecao de Lutadores/Arena", 164, 208, 10, kYellow);
					DrawText("Q       : Voltar ao Menu Principal", 164, 228, 10, kPink);
				}
				else
				{
					const RDA::FighterDef& P1Def = RDA::GetFighterDef(F1.id);
					DrawText(TextFormat("LISTA DE GOLPES — %s", P1Def.name), 160, 122, 10, kCyan);
					DrawText("Ataque Fraco: F (P1) / J (P2) | Ataque Forte: G (P1) / K (P2)", 160, 140, 8, kWhite);
					DrawText("Agarrao Universal (Quebra Defesa): F+G simultaneo ou T (P1) / J+K ou I (P2)", 160, 156, 8, kGreen);
					DrawText(TextFormat("Especial 1 (H / L): %s", P1Def.ability1), 160, 172, 8, kYellow);
					DrawText(TextFormat("Especial 2 (Tras + H / L): %s", P1Def.ability2), 160, 188, 8, kYellow);
					DrawText(TextFormat("Super (50 Barra, Baixo+H ou Y): %s", P1Def.ability3), 160, 204, 8, kPink);
					DrawText("Finalizacao de Palco: Na janela FINALIZE!, aproxime-se e use F+G / T", 160, 222, 8, kCyan);
					DrawText("Pressione M para voltar ao menu de pausa", 160, 246, 8, kMuted);
				}
			}

			DrawTouchOverlay(bShowTouchOverlay, P1);
		}

		EndTextureMode();

		BeginDrawing();
		ClearBackground(BLACK);
		Rectangle Src{0.0f, 0.0f, static_cast<float>(Canvas.texture.width), -static_cast<float>(Canvas.texture.height)};
		DrawTexturePro(Canvas.texture, Src, DestViewport, Vector2{0.0f, 0.0f}, 0.0f, WHITE);
		EndDrawing();
	}

	if (bHasMenuMusic) UnloadMusicStream(MenuMusic);
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
		for (SpriteSheet& Sheet : FVis.sheets)
		{
			if (Sheet.texture.id != 0)
			{
				// Avoid double-unloading aliased textures
				const unsigned int UnloadedId = Sheet.texture.id;
				UnloadTexture(Sheet.texture);
				for (SpriteSheet& Other : FVis.sheets)
				{
					if (Other.texture.id == UnloadedId) Other.texture.id = 0;
				}
			}
		}
	}
	if (LogoTex.id != 0) UnloadTexture(LogoTex);
	Sounds.Unload();
	UnloadRenderTexture(Canvas);
	if (IsAudioDeviceReady()) CloseAudioDevice();
	CloseWindow();
	return 0;
}
