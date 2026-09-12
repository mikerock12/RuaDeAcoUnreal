#include "RDACombatWorld.h"

#include "raylib.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <memory>
#include <string>
#include <vector>

using namespace RDA;

namespace
{
    constexpr int kAnimCount = 23;
    constexpr float kStickDeadzone = 0.35f;

    const Color kInk = {2, 4, 10, 255};
    const Color kNavy = {9, 21, 42, 255};
    const Color kPanel = {13, 23, 40, 230};
    const Color kCyan = {32, 200, 229, 255};
    const Color kCyanLight = {154, 247, 255, 255};
    const Color kPink = {212, 71, 113, 255};
    const Color kGold = {232, 184, 74, 255};
    const Color kIvory = {243, 231, 189, 255};
    const Color kMuted = {113, 134, 154, 255};
    const Color kDanger = {217, 67, 80, 255};
    const Color kSteel = {82, 107, 130, 255};

    struct AnimSheet
    {
        Texture2D texture{};
        int frameSize = 256;
        int frames = 4;
        int fps = 6;
        bool loop = false;
        bool loaded = false;
    };

    struct AnimSpec
    {
        AnimId id;
        const char* file;
        int fps;
        bool loop;
    };

    struct FighterArt
    {
        AnimSheet sheets[kAnimCount];
    };

    const AnimSpec kGutoSpecs[] = {
        {AnimId::Idle, "idle.png", 5, true},
        {AnimId::Walk, "corrida.png", 7, true},
        {AnimId::WalkBackward, "walk-backward.png", 6, true},
        {AnimId::JumpNeutral, "jump-neutral.png", 6, false},
        {AnimId::JumpForward, "jump-forward.png", 6, false},
        {AnimId::JumpBackward, "jump-backward.png", 6, false},
        {AnimId::Fall, "fall.png", 6, false},
        {AnimId::Landing, "landing.png", 8, false},
        {AnimId::Crouch, "crouch.png", 5, false},
        {AnimId::StandingLight, "standing-light.png", 9, false},
        {AnimId::StandingHeavy, "standing-heavy.png", 7, false},
        {AnimId::ForwardHeavy, "forward-heavy.png", 7, false},
        {AnimId::CrouchLight, "crouch-light.png", 9, false},
        {AnimId::CrouchHeavy, "crouch-heavy.png", 7, false},
        {AnimId::AirLight, "air-light-neutral.png", 9, false},
        {AnimId::AirHeavy, "air-heavy-neutral.png", 7, false},
        {AnimId::BlockStanding, "block-standing.png", 7, false},
        {AnimId::BlockCrouching, "block-crouching.png", 7, false},
        {AnimId::Hit, "hit.png", 10, false},
        {AnimId::Knockdown, "knockdown.png", 6, false},
        {AnimId::WakeUp, "wake-up.png", 7, false},
        {AnimId::Knockout, "knockout.png", 5, false},
        {AnimId::Victory, "victory.png", 6, true},
    };

    const AnimSpec kRafaSpecs[] = {
        {AnimId::Idle, "idle.png", 6, true},
        {AnimId::Walk, "corrida.png", 10, true},
        {AnimId::WalkBackward, "walk-backward.png", 8, true},
        {AnimId::JumpNeutral, "jump-neutral.png", 8, false},
        {AnimId::JumpForward, "jump-forward.png", 8, false},
        {AnimId::JumpBackward, "jump-backward.png", 8, false},
        {AnimId::Fall, "fall.png", 8, false},
        {AnimId::Landing, "landing.png", 12, false},
        {AnimId::Crouch, "crouch.png", 6, false},
        {AnimId::StandingLight, "standing-light.png", 12, false},
        {AnimId::StandingHeavy, "standing-heavy.png", 9, false},
        {AnimId::ForwardHeavy, "forward-heavy.png", 9, false},
        {AnimId::CrouchLight, "crouch-light.png", 12, false},
        {AnimId::CrouchHeavy, "crouch-heavy.png", 9, false},
        {AnimId::AirLight, "air-light-neutral.png", 12, false},
        {AnimId::AirHeavy, "air-heavy-neutral.png", 9, false},
        {AnimId::BlockStanding, "block-standing.png", 8, false},
        {AnimId::BlockCrouching, "block-crouching.png", 8, false},
        {AnimId::Hit, "hit.png", 10, false},
        {AnimId::Knockdown, "knockdown.png", 7, false},
        {AnimId::WakeUp, "wake-up.png", 9, false},
        {AnimId::Knockout, "knockout.png", 6, false},
        {AnimId::Victory, "victory.png", 7, true},
    };

    std::filesystem::path ExecutableDir()
    {
        const char* dir = GetApplicationDirectory();
        if (dir && dir[0] != '\0')
        {
            return std::filesystem::path(dir);
        }
        return std::filesystem::current_path();
    }

    std::filesystem::path FindContent()
    {
        const std::filesystem::path exe = ExecutableDir();
        const std::filesystem::path candidates[] = {
            exe / "Content",
            exe.parent_path() / "Content",
            exe.parent_path().parent_path() / "Content",
            exe.parent_path().parent_path().parent_path() / "Content",
            std::filesystem::current_path() / "Content",
            std::filesystem::path("D:/PROJETOS/RuaDeAcoUnreal/Content"),
        };
        for (const auto& candidate : candidates)
        {
            if (std::filesystem::exists(candidate / "RuaDeAco" / "Art" / "Characters" / "GutoBarba" / "idle.png"))
            {
                return candidate;
            }
        }
        return {};
    }

    int AnimIndex(AnimId id)
    {
        return static_cast<int>(id);
    }

    void LoadFighterArt(FighterArt& art, const std::filesystem::path& folder, int frameSize, const AnimSpec* specs, int specCount)
    {
        for (int i = 0; i < specCount; ++i)
        {
            const AnimSpec& spec = specs[i];
            const int index = AnimIndex(spec.id);
            const std::filesystem::path file = folder / spec.file;
            AnimSheet& sheet = art.sheets[index];
            sheet.frameSize = frameSize;
            sheet.fps = spec.fps;
            sheet.loop = spec.loop;
            if (!std::filesystem::exists(file))
            {
                continue;
            }
            Image image = LoadImage(file.string().c_str());
            if (image.data == nullptr)
            {
                continue;
            }
            sheet.frames = (std::max)(1, image.width / (std::max)(1, frameSize));
            sheet.texture = LoadTextureFromImage(image);
            UnloadImage(image);
            if (sheet.texture.id != 0)
            {
                SetTextureFilter(sheet.texture, TEXTURE_FILTER_POINT);
                sheet.loaded = true;
            }
        }
    }

    int ResolveFrame(const AnimSheet& sheet, const FighterSnapshot& snap)
    {
        if (!sheet.loaded || sheet.frames <= 1)
        {
            return 0;
        }
        if (snap.attacking)
        {
            MoveDef move;
            move.totalFrames = std::max(1, snap.moveTotalFrames);
            move.hitbox.activeFrom = snap.hitActiveFrom;
            move.hitbox.activeTo = snap.hitActiveTo;
            return MoveAnimFrame(move, snap.moveFrame, sheet.frames);
        }
        const int ticks = (std::max)(1, 60 / (std::max)(1, sheet.fps));
        const int frame = snap.stateFrame / ticks;
        return sheet.loop ? frame % sheet.frames : (std::min)(sheet.frames - 1, frame);
    }

    void DrawFighterSprite(const FighterArt& art, const FighterSnapshot& snap, const FighterDef& def, float alpha)
    {
        const float worldX = snap.previousX + (snap.x - snap.previousX) * alpha;
        const float worldY = snap.previousY + (snap.y - snap.previousY) * alpha;
        const float drawX = std::round(worldX);
        const float drawY = std::round(worldY + static_cast<float>(def.visualSolePad));

        DrawEllipse(
            static_cast<int>(drawX),
            static_cast<int>(kGroundY + 6.0f),
            28,
            7,
            Fade(BLACK, 0.40f));

        const AnimSheet& sheet = art.sheets[AnimIndex(snap.anim)];
        if (!sheet.loaded)
        {
            const Color fallback = snap.id == FighterId::GutoBarba ? Color{97, 72, 142, 255} : Color{18, 125, 168, 255};
            DrawRectangle(static_cast<int>(drawX) - 16, static_cast<int>(drawY) - 64, 32, 64, fallback);
            return;
        }

        const int frame = ResolveFrame(sheet, snap);
        const float size = static_cast<float>(sheet.frameSize);
        Rectangle src = {
            static_cast<float>(frame * sheet.frameSize),
            0.0f,
            snap.facing < 0 ? -size : size,
            size
        };
        Rectangle dst = {drawX, drawY, size, size};
        Vector2 origin = {size * 0.5f, size};
        DrawTexturePro(sheet.texture, src, dst, origin, 0.0f, WHITE);
    }

    void DrawDebugBoxes(const FighterSnapshot& snap)
    {
        DrawRectangleLinesEx(
            {snap.pushboxWorld.x, snap.pushboxWorld.y, snap.pushboxWorld.width, snap.pushboxWorld.height},
            1.0f,
            Fade(SKYBLUE, 0.85f));
        for (const Rect& hurt : snap.hurtboxesWorld)
        {
            DrawRectangleLinesEx({hurt.x, hurt.y, hurt.width, hurt.height}, 1.0f, Fade(LIME, 0.9f));
        }
        for (const Rect& hit : snap.hitboxesWorld)
        {
            DrawRectangleRec({hit.x, hit.y, hit.width, hit.height}, Fade(RED, 0.35f));
            DrawRectangleLinesEx({hit.x, hit.y, hit.width, hit.height}, 1.0f, RED);
        }
    }

    void DrawPixelText(const Font& font, const char* text, int x, int y, int size, Color color)
    {
        DrawTextEx(font, text, {static_cast<float>(x), static_cast<float>(y)}, static_cast<float>(size), 1.0f, color);
    }

    void DrawHealthBar(int x, int y, int width, int height, float ratio, Color fill, bool flip)
    {
        DrawRectangle(x - 1, y - 1, width + 2, height + 2, kSteel);
        DrawRectangle(x, y, width, height, Color{20, 28, 42, 255});
        const int filled = static_cast<int>(std::round(std::clamp(ratio, 0.0f, 1.0f) * static_cast<float>(width)));
        if (filled > 0)
        {
            const int barX = flip ? x + width - filled : x;
            DrawRectangle(barX, y, filled, height, fill);
        }
    }

    void DrawHud(const Font& font, const WorldSnapshot& snap)
    {
        DrawRectangle(0, 0, kLogicalWidth, 52, Color{5, 8, 18, 210});
        DrawRectangle(0, 50, kLogicalWidth, 1, kCyan);

        const FighterSnapshot& p1 = snap.fighters[0];
        const FighterSnapshot& p2 = snap.fighters[1];
        const float r1 = p1.maxHealth > 0 ? static_cast<float>(p1.health) / static_cast<float>(p1.maxHealth) : 0.0f;
        const float r2 = p2.maxHealth > 0 ? static_cast<float>(p2.health) / static_cast<float>(p2.maxHealth) : 0.0f;

        DrawRectangle(8, 8, 18, 18, kCyan);
        DrawRectangleLines(8, 8, 18, 18, kIvory);
        DrawPixelText(font, p1.name, 30, 6, 8, kIvory);
        DrawHealthBar(30, 18, 196, 8, r1, r1 < 0.25f ? kDanger : kCyan, false);

        DrawRectangle(kLogicalWidth - 26, 8, 18, 18, kPink);
        DrawRectangleLines(kLogicalWidth - 26, 8, 18, 18, kIvory);
        const int p2NameWidth = static_cast<int>(MeasureTextEx(font, p2.name, 8, 1).x);
        DrawPixelText(font, p2.name, kLogicalWidth - 30 - p2NameWidth, 6, 8, kIvory);
        DrawHealthBar(kLogicalWidth - 226, 18, 196, 8, r2, r2 < 0.25f ? kDanger : kPink, true);

        char timer[8];
        std::snprintf(timer, sizeof(timer), "%02d", snap.timeSeconds);
        const int timerWidth = static_cast<int>(MeasureTextEx(font, timer, 16, 1).x);
        DrawPixelText(font, timer, (kLogicalWidth - timerWidth) / 2, 6, 16, kGold);

        for (int i = 0; i < kRoundsToWin; ++i)
        {
            const bool won = i < snap.roundWins[0];
            DrawCircle(300 - i * 10, 36, 3, won ? kCyan : Color{40, 50, 70, 255});
            const bool won2 = i < snap.roundWins[1];
            DrawCircle(340 + i * 10, 36, 3, won2 ? kPink : Color{40, 50, 70, 255});
        }

        char roundLabel[12];
        std::snprintf(roundLabel, sizeof(roundLabel), "R%d", snap.round);
        const int roundWidth = static_cast<int>(MeasureTextEx(font, roundLabel, 8, 1).x);
        DrawPixelText(font, roundLabel, (kLogicalWidth - roundWidth) / 2, 30, 8, kMuted);

        if (snap.banner && snap.banner[0] != '\0')
        {
            const int size = (std::strcmp(snap.banner, "KO") == 0) ? 32 : 24;
            const int width = static_cast<int>(MeasureTextEx(font, snap.banner, static_cast<float>(size), 1).x);
            const int x = (kLogicalWidth - width) / 2;
            const int y = 120;
            DrawRectangle(x - 12, y - 8, width + 24, size + 16, Color{5, 8, 18, 180});
            DrawPixelText(font, snap.banner, x, y, size, snap.banner[0] == 'K' ? kDanger : kGold);
        }

        if (snap.phase == CombatPhase::MatchOver)
        {
            const char* winner = snap.roundWins[0] >= kRoundsToWin ? p1.name : p2.name;
            char line[48];
            std::snprintf(line, sizeof(line), "%s VENCEU", winner);
            const int width = static_cast<int>(MeasureTextEx(font, line, 12, 1).x);
            DrawPixelText(font, line, (kLogicalWidth - width) / 2, 168, 12, kIvory);
            const char* hint = "ENTER: menu   R: revanche";
            const int hintW = static_cast<int>(MeasureTextEx(font, hint, 8, 1).x);
            DrawPixelText(font, hint, (kLogicalWidth - hintW) / 2, 190, 8, kMuted);
        }

        DrawPixelText(font, "P1 WASD F soco G chute R defesa", 8, kLogicalHeight - 18, 8, Fade(kMuted, 0.9f));
        DrawPixelText(font, "P2 SETAS J/K/U", kLogicalWidth - 150, kLogicalHeight - 18, 8, Fade(kMuted, 0.9f));
    }

    bool StickLeft(int pad)
    {
        return IsGamepadAvailable(pad)
            && (GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_X) < -kStickDeadzone
                || IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_LEFT));
    }

    bool StickRight(int pad)
    {
        return IsGamepadAvailable(pad)
            && (GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_X) > kStickDeadzone
                || IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_RIGHT));
    }

    bool StickUp(int pad)
    {
        return IsGamepadAvailable(pad)
            && (GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_Y) < -kStickDeadzone
                || IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_UP));
    }

    bool StickDown(int pad)
    {
        return IsGamepadAvailable(pad)
            && (GetGamepadAxisMovement(pad, GAMEPAD_AXIS_LEFT_Y) > kStickDeadzone
                || IsGamepadButtonDown(pad, GAMEPAD_BUTTON_LEFT_FACE_DOWN));
    }

    InputFrame SamplePlayer(int player)
    {
        InputFrame input;
        const int pad = player;
        if (player == 0)
        {
            input.left = IsKeyDown(KEY_A) || StickLeft(pad);
            input.right = IsKeyDown(KEY_D) || StickRight(pad);
            input.up = IsKeyDown(KEY_W) || StickUp(pad);
            input.down = IsKeyDown(KEY_S) || StickDown(pad);
            input.punch = IsKeyDown(KEY_F) || (IsGamepadAvailable(pad) && IsGamepadButtonDown(pad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));
            input.kick = IsKeyDown(KEY_G) || (IsGamepadAvailable(pad) && IsGamepadButtonDown(pad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT));
            input.block = IsKeyDown(KEY_R) || (IsGamepadAvailable(pad) && IsGamepadButtonDown(pad, GAMEPAD_BUTTON_RIGHT_FACE_UP));
            input.special = IsKeyDown(KEY_H) || (IsGamepadAvailable(pad) && IsGamepadButtonDown(pad, GAMEPAD_BUTTON_RIGHT_FACE_LEFT));
            input.upPressed = IsKeyPressed(KEY_W)
                || (IsGamepadAvailable(pad) && IsGamepadButtonPressed(pad, GAMEPAD_BUTTON_LEFT_FACE_UP));
            input.punchPressed = IsKeyPressed(KEY_F) || (IsGamepadAvailable(pad) && IsGamepadButtonPressed(pad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));
            input.kickPressed = IsKeyPressed(KEY_G) || (IsGamepadAvailable(pad) && IsGamepadButtonPressed(pad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT));
            input.blockPressed = IsKeyPressed(KEY_R) || (IsGamepadAvailable(pad) && IsGamepadButtonPressed(pad, GAMEPAD_BUTTON_RIGHT_FACE_UP));
        }
        else
        {
            input.left = IsKeyDown(KEY_LEFT) || StickLeft(pad);
            input.right = IsKeyDown(KEY_RIGHT) || StickRight(pad);
            input.up = IsKeyDown(KEY_UP) || StickUp(pad);
            input.down = IsKeyDown(KEY_DOWN) || StickDown(pad);
            input.punch = IsKeyDown(KEY_J) || (IsGamepadAvailable(pad) && IsGamepadButtonDown(pad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));
            input.kick = IsKeyDown(KEY_K) || (IsGamepadAvailable(pad) && IsGamepadButtonDown(pad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT));
            input.block = IsKeyDown(KEY_U) || (IsGamepadAvailable(pad) && IsGamepadButtonDown(pad, GAMEPAD_BUTTON_RIGHT_FACE_UP));
            input.special = IsKeyDown(KEY_L) || (IsGamepadAvailable(pad) && IsGamepadButtonPressed(pad, GAMEPAD_BUTTON_RIGHT_FACE_LEFT));
            input.upPressed = IsKeyPressed(KEY_UP) || (IsGamepadAvailable(pad) && IsGamepadButtonPressed(pad, GAMEPAD_BUTTON_LEFT_FACE_UP));
            input.punchPressed = IsKeyPressed(KEY_J) || (IsGamepadAvailable(pad) && IsGamepadButtonPressed(pad, GAMEPAD_BUTTON_RIGHT_FACE_DOWN));
            input.kickPressed = IsKeyPressed(KEY_K) || (IsGamepadAvailable(pad) && IsGamepadButtonPressed(pad, GAMEPAD_BUTTON_RIGHT_FACE_RIGHT));
            input.blockPressed = IsKeyPressed(KEY_U) || (IsGamepadAvailable(pad) && IsGamepadButtonPressed(pad, GAMEPAD_BUTTON_RIGHT_FACE_UP));
        }

        input.pause = IsKeyDown(KEY_ESCAPE) || (IsGamepadAvailable(player) && IsGamepadButtonDown(player, GAMEPAD_BUTTON_MIDDLE_RIGHT));
        input.pausePressed = IsKeyPressed(KEY_ESCAPE) || (IsGamepadAvailable(player) && IsGamepadButtonPressed(player, GAMEPAD_BUTTON_MIDDLE_RIGHT));
        input.confirm = IsKeyDown(KEY_ENTER);
        input.confirmPressed = IsKeyPressed(KEY_ENTER);
        input.debugPressed = IsKeyPressed(KEY_F1);
        input.restartPressed = IsKeyPressed(KEY_N);
        return input;
    }

    void DrawTitle(const Font& font, const Font& fontLarge, const Texture2D& stage, const FighterArt& gutoArt, const FighterArt& rafaArt, int ticks)
    {
        if (stage.id != 0)
        {
            DrawTexturePro(
                stage,
                {0, 0, static_cast<float>(stage.width), static_cast<float>(stage.height)},
                {0, 0, static_cast<float>(kLogicalWidth), static_cast<float>(kLogicalHeight)},
                {0, 0},
                0.0f,
                WHITE);
        }
        else
        {
            DrawRectangle(0, 0, kLogicalWidth, kLogicalHeight, kNavy);
        }
        DrawRectangle(0, 0, kLogicalWidth, kLogicalHeight, Color{2, 6, 16, 120});
        DrawRectangle(0, 0, kLogicalWidth, 70, Color{5, 8, 18, 220});
        DrawRectangle(0, kLogicalHeight - 70, kLogicalWidth, 70, Color{5, 8, 18, 220});

        const char* title = "RUA DE ACO";
        const int titleW = static_cast<int>(MeasureTextEx(fontLarge, title, 24, 1).x);
        DrawPixelText(fontLarge, title, (kLogicalWidth - titleW) / 2, 18, 24, kCyanLight);
        const char* sub = "UNREAL C++  /  PROTOTIPO 0.1";
        const int subW = static_cast<int>(MeasureTextEx(font, sub, 8, 1).x);
        DrawPixelText(font, sub, (kLogicalWidth - subW) / 2, 48, 8, kGold);

        FighterSnapshot guto{};
        guto.id = FighterId::GutoBarba;
        guto.x = 188;
        guto.y = kGroundY;
        guto.previousX = 188;
        guto.previousY = kGroundY;
        guto.facing = 1;
        guto.stateFrame = ticks;
        guto.anim = AnimId::Idle;
        FighterSnapshot rafa = guto;
        rafa.id = FighterId::RafaMare;
        rafa.x = 452;
        rafa.previousX = 452;
        rafa.facing = -1;
        DrawFighterSprite(gutoArt, guto, GetGutoBarba(), 1.0f);
        DrawFighterSprite(rafaArt, rafa, GetRafaMare(), 1.0f);

        DrawPixelText(font, "GUTO BARBA", 130, 250, 8, kIvory);
        DrawPixelText(font, "RAFA MARE", 400, 250, 8, kIvory);

        const char* start = (ticks / 40) % 2 == 0 ? "ENTER PARA LUTAR" : "";
        const int startW = static_cast<int>(MeasureTextEx(font, "ENTER PARA LUTAR", 12, 1).x);
        DrawPixelText(font, start, (kLogicalWidth - startW) / 2, 280, 12, kGold);

        DrawPixelText(font, "P1: WASD mover  F soco  G chute  R defesa", 70, 312, 8, kMuted);
        DrawPixelText(font, "P2: setas mover  J soco  K chute  U defesa", 70, 326, 8, kMuted);
        DrawPixelText(font, "F1 hitboxes   ESC pausa   gamepad suportado", 70, 340, 8, kMuted);
    }
}

int main()
{
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(kPresentationWidth, kPresentationHeight, "Rua de Aco Unreal C++  -  Prototipo 0.1");
    InitAudioDevice();
    SetTargetFPS(60);
    SetWindowMinSize(kLogicalWidth, kLogicalHeight);

    const std::filesystem::path content = FindContent();
    if (content.empty())
    {
        while (!WindowShouldClose())
        {
            BeginDrawing();
            ClearBackground(kInk);
            DrawText("Content/ nao encontrado.", 40, 40, 20, RED);
            DrawText("Coloque a pasta Content ao lado do executavel.", 40, 70, 18, RAYWHITE);
            EndDrawing();
        }
        CloseAudioDevice();
        CloseWindow();
        return 1;
    }

    const std::filesystem::path artRoot = content / "RuaDeAco" / "Art";
    FighterArt gutoArt{};
    FighterArt rafaArt{};
    LoadFighterArt(gutoArt, artRoot / "Characters" / "GutoBarba", 288, kGutoSpecs, static_cast<int>(sizeof(kGutoSpecs) / sizeof(kGutoSpecs[0])));
    LoadFighterArt(rafaArt, artRoot / "Characters" / "RafaMare", 256, kRafaSpecs, static_cast<int>(sizeof(kRafaSpecs) / sizeof(kRafaSpecs[0])));

    Texture2D stage{};
    const std::filesystem::path stagePath = artRoot / "Environments" / "CaisDaCidade" / "background.png";
    if (std::filesystem::exists(stagePath))
    {
        stage = LoadTexture(stagePath.string().c_str());
        SetTextureFilter(stage, TEXTURE_FILTER_POINT);
    }

    const std::filesystem::path fontPath = content / "RuaDeAco" / "Fonts" / "PressStart2P-Regular.ttf";
    Font font = LoadFontEx(fontPath.string().c_str(), 16, nullptr, 0);
    Font fontLarge = LoadFontEx(fontPath.string().c_str(), 32, nullptr, 0);
    SetTextureFilter(font.texture, TEXTURE_FILTER_POINT);
    SetTextureFilter(fontLarge.texture, TEXTURE_FILTER_POINT);

    Music music{};
    bool musicLoaded = false;
    const std::filesystem::path musicPath = content / "RuaDeAco" / "Audio" / "Music" / "cais-da-cidade.ogg";
    if (std::filesystem::exists(musicPath))
    {
        music = LoadMusicStream(musicPath.string().c_str());
        music.looping = true;
        SetMusicVolume(music, 0.42f);
        PlayMusicStream(music);
        musicLoaded = true;
    }

    RenderTexture2D canvas = LoadRenderTexture(kLogicalWidth, kLogicalHeight);
    SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);

    enum class Screen { Title, Fight };
    Screen screen = Screen::Title;
    std::unique_ptr<CombatWorld> world;
    int titleTicks = 0;
    bool prevP1Jump = false;
    bool prevP2Jump = false;

    while (!WindowShouldClose())
    {
        if (musicLoaded)
        {
            UpdateMusicStream(music);
        }
        if (IsKeyPressed(KEY_F11))
        {
            ToggleFullscreen();
        }

        InputFrame p1 = SamplePlayer(0);
        InputFrame p2 = SamplePlayer(1);

        const bool p1StickJump = IsGamepadAvailable(0) && GetGamepadAxisMovement(0, GAMEPAD_AXIS_LEFT_Y) < -kStickDeadzone;
        const bool p2StickJump = IsGamepadAvailable(1) && GetGamepadAxisMovement(1, GAMEPAD_AXIS_LEFT_Y) < -kStickDeadzone;
        if (p1StickJump && !prevP1Jump) p1.upPressed = true;
        if (p2StickJump && !prevP2Jump) p2.upPressed = true;
        prevP1Jump = p1StickJump;
        prevP2Jump = p2StickJump;

        if (screen == Screen::Title)
        {
            titleTicks += 1;
            if (p1.confirmPressed || p1.punchPressed || p2.confirmPressed)
            {
                world = std::make_unique<CombatWorld>(GetGutoBarba(), GetRafaMare());
                screen = Screen::Fight;
            }
        }
        else if (world)
        {
            if (p1.pausePressed || p2.pausePressed)
            {
                world->TogglePause();
            }
            if (world->Phase() == CombatPhase::MatchOver)
            {
                if (p1.confirmPressed || p2.confirmPressed)
                {
                    screen = Screen::Title;
                    world.reset();
                }
                else if (IsKeyPressed(KEY_R))
                {
                    world->RestartMatch();
                }
            }
            else
            {
                world->Step(p1, p2);
            }
        }

        BeginTextureMode(canvas);
        ClearBackground(kInk);
        if (stage.id != 0)
        {
            DrawTexturePro(
                stage,
                {0, 0, static_cast<float>(stage.width), static_cast<float>(stage.height)},
                {0, 0, static_cast<float>(kLogicalWidth), static_cast<float>(kLogicalHeight)},
                {0, 0},
                0.0f,
                WHITE);
        }
        else
        {
            DrawRectangle(0, 0, kLogicalWidth, kLogicalHeight, kNavy);
            DrawRectangle(0, static_cast<int>(kGroundY), kLogicalWidth, kLogicalHeight - static_cast<int>(kGroundY), Color{18, 28, 46, 255});
        }

        if (screen == Screen::Title)
        {
            DrawTitle(font, fontLarge, stage, gutoArt, rafaArt, titleTicks);
        }
        else if (world)
        {
            const WorldSnapshot snap = world->Snapshot();
            DrawFighterSprite(gutoArt, snap.fighters[0], GetGutoBarba(), 1.0f);
            DrawFighterSprite(rafaArt, snap.fighters[1], GetRafaMare(), 1.0f);
            if (snap.debugBoxes)
            {
                DrawDebugBoxes(snap.fighters[0]);
                DrawDebugBoxes(snap.fighters[1]);
            }
            DrawHud(font, snap);
            if (snap.paused)
            {
                DrawRectangle(0, 0, kLogicalWidth, kLogicalHeight, Color{0, 0, 0, 120});
                const char* pause = "PAUSA";
                const int w = static_cast<int>(MeasureTextEx(fontLarge, pause, 24, 1).x);
                DrawPixelText(fontLarge, pause, (kLogicalWidth - w) / 2, 140, 24, kIvory);
                DrawPixelText(font, "ESC continua    F1 hitboxes", 190, 180, 8, kMuted);
            }
        }
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        const float scale = (std::min)(
            static_cast<float>(GetScreenWidth()) / static_cast<float>(kLogicalWidth),
            static_cast<float>(GetScreenHeight()) / static_cast<float>(kLogicalHeight));
        const int integerScale = (std::max)(1, static_cast<int>(std::floor(scale)));
        const float usedScale = static_cast<float>(integerScale);
        const float drawW = kLogicalWidth * usedScale;
        const float drawH = kLogicalHeight * usedScale;
        const float drawX = (GetScreenWidth() - drawW) * 0.5f;
        const float drawY = (GetScreenHeight() - drawH) * 0.5f;
        DrawTexturePro(
            canvas.texture,
            {0, 0, static_cast<float>(kLogicalWidth), -static_cast<float>(kLogicalHeight)},
            {drawX, drawY, drawW, drawH},
            {0, 0},
            0.0f,
            WHITE);
        EndDrawing();
    }

    if (musicLoaded) UnloadMusicStream(music);
    UnloadRenderTexture(canvas);
    if (stage.id != 0) UnloadTexture(stage);
    UnloadFont(font);
    UnloadFont(fontLarge);
    for (int i = 0; i < kAnimCount; ++i)
    {
        if (gutoArt.sheets[i].loaded) UnloadTexture(gutoArt.sheets[i].texture);
        if (rafaArt.sheets[i].loaded) UnloadTexture(rafaArt.sheets[i].texture);
    }
    CloseAudioDevice();
    CloseWindow();
    return 0;
}
