#pragma once

// Camadas/atores decorativos portados de CaisStageView, KitchenStageView,
// SitioStageView e respectivos *Ambience.ts. O RNG é isolado do combate.
#include "RDACombatWorld.h"
#include "raylib.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <filesystem>
#include <string>
#include <unordered_map>
#include <vector>

namespace RDAStageVisuals
{
class StageRenderer
{
  public:
    bool decorativeEffects=true;
    bool Load(const std::filesystem::path &artRoot)
    {
        Unload();
        bool ready = true;
        auto load = [&](const char *group, const char *folder, const char *name) {
            const auto path = artRoot / "Environments" / folder / (std::string(name) + ".png");
            Texture2D texture = LoadTexture(path.generic_string().c_str());
            if (texture.id)
                SetTextureFilter(texture, TEXTURE_FILTER_POINT);
            else
            {
                TraceLog(LOG_ERROR, "RDA_STAGE_ASSET_MISSING: %s", path.generic_string().c_str());
                ready = false;
            }
            textures[std::string(group) + "/" + name] = texture;
        };
        for (const char *name : {"background", "moon", "ufo", "witch", "ship", "water-0", "water-1", "water-2",
                                 "monster", "monster-finisher-v2", "fire", "splash"})
            load("cais", "CaisDaCidade", name);
        for (const char *name : {"background", "witch", "bat", "rat", "skull-bones"})
            load("kitchen", "CozinhaMacabra", name);
        for (const char *name : {"background", "hen", "duck", "snake", "lizard", "masked-farmer", "trident"})
            load("sitio", "Sitio", name);
        for (int i = 0; i < 3; ++i)
            SetTextureWrap(T("cais/water-" + std::to_string(i)), TEXTURE_WRAP_REPEAT);
        Reset();
        return ready;
    }
    void Unload()
    {
        for (auto &entry : textures)
            if (entry.second.id)
                UnloadTexture(entry.second);
        textures.clear();
    }
    StageRenderer() = default;
    StageRenderer(const StageRenderer &) = delete;
    StageRenderer &operator=(const StageRenderer &) = delete;

    // dt em segundos. Pausa e finalização não avançam os ecossistemas.
    void Update(float dt, const RDA::WorldSnapshot &snapshot)
    {
        if (!std::isfinite(dt) || dt <= 0 || snapshot.paused)
            return;
        if (snapshot.frame < lastFrame || snapshot.stage != lastStage)
            Reset();
        lastFrame = snapshot.frame;
        lastStage = snapshot.stage;
        const float ms = std::min(dt * 1000.0f, 100.0f);
        if (snapshot.visualFinisher.active)
            return;
        if (snapshot.stage == RDA::StageId::CaisDaCidade)
        {
            UpdateCais(ms, snapshot.fighters[0].x, snapshot.fighters[1].x);
            float target =
                std::clamp((320 - (snapshot.fighters[0].x + snapshot.fighters[1].x) * 0.5f) * 0.025f, -3.0f, 3.0f);
            skyOffset += (target - skyOffset) * std::min(1.0f, ms / 240);
        }
        else if (snapshot.stage == RDA::StageId::CozinhaMacabra)
            UpdateKitchen(ms);
        else
            UpdateFarm(ms);
    }

    // DrawBack -> vítima na camada do cenário -> DrawFront -> lutadores
    // em primeiro plano -> DrawOverlay. Antes do impacto a vítima também
    // fica em primeiro plano: use VictimInStageLayer para decidir.
    static bool VictimInStageLayer(const RDA::WorldSnapshot &s)
    {
        if (!s.visualFinisher.active)
            return false;
        const int threshold = s.stage == RDA::StageId::CozinhaMacabra ? 108 : s.stage == RDA::StageId::Sitio ? 112 : 60;
        return s.visualFinisher.frame >= threshold;
    }
    void DrawBack(const RDA::WorldSnapshot &s) const
    {
        if (s.stage == RDA::StageId::CaisDaCidade)
            DrawCaisBack(s);
        else if (s.stage == RDA::StageId::CozinhaMacabra)
            DrawKitchenBack(s);
        else
            DrawFarmBack(s);
    }
    void DrawFront(const RDA::WorldSnapshot &s) const
    {
        if (s.stage == RDA::StageId::CaisDaCidade)
            DrawCaisFront(s);
        else if (s.stage == RDA::StageId::CozinhaMacabra)
            DrawKitchenFront(s);
        else
            DrawFarmFront(s);
    }
    void DrawOverlay(const RDA::WorldSnapshot &s) const
    {
        const auto &v = s.visualFinisher;
        if (!v.active)
            return;
        int f = v.frame;
        const float fade = std::min(1.0f, f / 24.0f);
        Rect(0, 0, 640, 8, 0x02040a, fade * 0.82f);
        Rect(0, 352, 640, 8, 0x02040a, fade * 0.82f);
        int impact = s.stage == RDA::StageId::CozinhaMacabra ? 108 : s.stage == RDA::StageId::Sitio ? 112 : 112;
        float x = s.stage == RDA::StageId::CozinhaMacabra ? 442
                  : s.stage == RDA::StageId::Sitio        ? v.tridentTipX
                                                          : v.monsterX;
        float y = s.stage == RDA::StageId::CozinhaMacabra ? 152 : s.stage == RDA::StageId::Sitio ? v.tridentTipY : 258;
        unsigned color = s.stage == RDA::StageId::CozinhaMacabra ? 0xf5b36b
                         : s.stage == RDA::StageId::Sitio        ? 0xffd78a
                                                                 : 0x9ae4f2;
        int age = f - impact;
        if (age >= 0 && age < 36)
        {
            float p = age / 36.0f;
            for (int i = 0; i < 24; ++i)
            {
                float a = i * 2.399f, spread = (10 + i % 6 * 7) * p;
                Rect(x + std::cos(a) * spread, y + std::sin(a) * spread * 0.6f + p * p * 22, 2, 2,
                     i % 3 ? color : 0xf5eee0, (1 - p) * 0.75f);
            }
        }
        if (s.stage == RDA::StageId::CozinhaMacabra && age >= 0)
            for (int i = 0; i < 12; ++i)
            {
                float p = ((age + i * 11) % 84) / 84.0f;
                Rect(x - 24 + i % 6 * 9 + std::sin(p * 4 + i) * 4, y - 8 - p * 62, 3 + std::round(p * 4), 3,
                     i % 2 ? 0xe7c9a2 : 0x9c9988, (1 - p) * 0.22f);
            }
        if (s.stage == RDA::StageId::Sitio && f >= 34 && f < 282)
            for (int i = 0; i < 10; ++i)
            {
                float p = ((f + i * 17) % 100) / 100.0f;
                Rect(288 + i % 5 * 15, 224 - p * 90, 1, 2, 0xe0ba78, std::sin(p * Pi) * 0.25f);
            }
        for (int beat : s.stage == RDA::StageId::CaisDaCidade ? std::vector<int>{160, 184, 211}
                        : s.stage == RDA::StageId::Sitio      ? std::vector<int>{112, 282}
                                                              : std::vector<int>{108})
            if (f >= beat && f < beat + 8)
                Rect(0, 8, 640, 344, s.stage == RDA::StageId::CaisDaCidade ? 0x6b1631 : 0xffd49b,
                     (1 - (f - beat) / 8.0f) * 0.12f);
    }

  private:
    static constexpr float Pi = 3.14159265358979323846f;
    std::unordered_map<std::string, Texture2D> textures;
    unsigned rng = 0x51a7e123U;
    int lastFrame = 0;
    RDA::StageId lastStage = RDA::StageId::CaisDaCidade;
    float skyOffset = 0;
    struct Pose
    {
        bool visible = false;
        float x = 0, y = 0, scale = 1, alpha = 1;
        int frame = 0, direction = 1;
    };
    Pose ufo, witch, monster, ship, cannon;
    float caisTime = 0, eventTime = 0, idleRemaining = 8000, targetX = 320;
    int caisEvent = -1, lastEvent = -1, eventDirection = 1, targetSlot = 0;
    std::vector<int> eventBag;
    float monsterReveal = 0, beamStrength = 0, fireStrength = 0, muzzleFlash = 0, impactStrength = 0;
    struct KitchenActor
    {
        bool active = false;
        float age = 0, duration = 1, x = 0, y = 0, startX = 0, endX = 0, lane = 0, phase = 0;
        int frame = 0;
    };
    std::array<KitchenActor, 5> bats{};
    std::array<KitchenActor, 3> rats{};
    float kitchenTime = 0, nextBats = 1800, nextRat = 900;
    enum class Action
    {
        Walk,
        Lay,
        Eat,
        Challenge,
        Fight,
        Retreat
    };
    struct Animal
    {
        int species = 0;
        float x = 0, y = 0;
        int direction = 1;
        Action action = Action::Walk;
        float timer = 0, layIn = 0;
        bool laid = false;
        int egg = -1, frame = 0;
    };
    struct Egg
    {
        bool active = false;
        float x = 0, y = 0, age = 0;
        int eater = 2;
    };
    std::array<Animal, 6> animals{};
    std::array<Egg, 6> eggs{};
    float farmTime = 0, nextFight = 18000, meetingX = 320;
    bool meeting = false;
    int eggsLaid = 0;
    Texture2D T(const std::string &name) const
    {
        auto it = textures.find(name);
        return it == textures.end() ? Texture2D{} : it->second;
    }
    static Color C(unsigned rgb, float alpha = 1)
    {
        return Color{static_cast<unsigned char>(rgb >> 16), static_cast<unsigned char>(rgb >> 8),
                     static_cast<unsigned char>(rgb), static_cast<unsigned char>(std::clamp(alpha, 0.0f, 1.0f) * 255)};
    }
    static void Rect(float x, float y, float w, float h, unsigned color, float alpha = 1)
    {
        DrawRectangle(int(std::round(x)), int(std::round(y)), int(std::round(w)), int(std::round(h)), C(color, alpha));
    }
    static void Ellipse(float x, float y, float w, float h, unsigned color, float alpha = 1)
    {
        DrawEllipse(int(std::round(x)), int(std::round(y)), w * 0.5f, h * 0.5f, C(color, alpha));
    }
    static void Ring(float x, float y, float w, float h, unsigned color, float alpha = 1)
    {
        DrawEllipseLines(int(std::round(x)), int(std::round(y)), w * 0.5f, h * 0.5f, C(color, alpha));
    }
    static float Ease(float value)
    {
        float t = std::clamp(value, 0.0f, 1.0f);
        return t * t * (3 - 2 * t);
    }
    static float Mix(float a, float b, float t)
    {
        return a + (b - a) * t;
    }
    float Random()
    {
        rng ^= rng << 13;
        rng ^= rng >> 17;
        rng ^= rng << 5;
        return (rng & 0xffffff) / 16777216.0f;
    }
    float Between(float a, float b)
    {
        return a + Random() * (b - a);
    }
    static float Approach(float x, float target, float step)
    {
        return x + (target > x ? 1.0f : target < x ? -1.0f : 0) * std::min(std::fabs(target - x), step);
    }
    void Reset()
    {
        skyOffset = 0;
        caisTime = eventTime = 0;
        caisEvent = lastEvent = -1;
        eventBag.clear();
        idleRemaining = Between(6000, 10000);
        ufo = {};
        witch = {};
        monster = {};
        ship = {};
        cannon = {};
        monsterReveal = beamStrength = fireStrength = muzzleFlash = impactStrength = 0;
        kitchenTime = 0;
        nextBats = 1800;
        nextRat = 900;
        bats = {};
        rats = {};
        farmTime = 0;
        nextFight = 18000;
        meeting = false;
        eggsLaid = 0;
        eggs = {};
        animals = {Animal{0, 180, 259, 1, Action::Walk, 0, 2000},
                   Animal{0, 458, 277, -1, Action::Walk, 0, 8000},
                   Animal{1, 56, 246, 1},
                   Animal{1, 598, 246, -1},
                   Animal{2, 44, 273, 1},
                   Animal{3, 590, 281, -1}};
    }
    void Sprite(const char *name, float x, float y, int frames = 1, int frame = 0, float ax = 0.5f, float ay = 0.5f,
                float scale = 1, bool flip = false, float alpha = 1, float crop = -1) const
    {
        Texture2D t = T(name);
        if (!t.id)
            return;
        float fw = float(t.width) / frames, fh = float(t.height);
        frame = std::clamp(frame, 0, frames - 1);
        float h = crop < 0 ? fh : std::clamp(crop, 0.0f, fh);
        if (h <= 0)
            return;
        DrawTexturePro(
            t, Rectangle{frame * fw, 0, flip ? -fw : fw, h},
            Rectangle{std::round(x - ax * fw * scale), std::round(y - ay * fh * scale), fw * scale, h * scale},
            Vector2{0, 0}, 0, C(0xffffff, alpha));
    }
    void Background(const char *name, float offsetY = 0) const
    {
        auto t = T(name);
        if (t.id)
            DrawTexturePro(t, Rectangle{0, 0, float(t.width), float(t.height)}, Rectangle{0, offsetY, 640, 360},
                           Vector2{0, 0}, 0, WHITE);
    }
    void Actor(const char *name, const Pose &p, int frames = 1, bool feet = false) const
    {
        if (p.visible)
            Sprite(name, p.x, p.y, frames, p.frame, 0.5f, feet ? 1.0f : 0.5f, p.scale, p.direction < 0, p.alpha);
    }

    void UpdateCais(float ms, float p1, float p2)
    {
        caisTime += ms;
        if (caisEvent < 0)
        {
            idleRemaining -= ms;
            if (idleRemaining > 0)
                return;
            if (eventBag.empty())
            {
                eventBag = {0, 1, 2, 3, 4};
                for (int i = 4; i > 0; --i)
                    std::swap(eventBag[i], eventBag[int(Random() * (i + 1))]);
                if (eventBag.back() == lastEvent)
                    std::swap(eventBag[0], eventBag.back());
            }
            caisEvent = eventBag.back();
            eventBag.pop_back();
            eventTime = 0;
            eventDirection = Random() < 0.5f ? 1 : -1;
            targetSlot = Random() < 0.5f ? 0 : 1;
            targetX = caisEvent == 1   ? std::clamp(targetSlot == 0 ? p1 : p2, 90.0f, 550.0f)
                      : caisEvent == 4 ? (eventDirection == 1 ? 400.0f : 240.0f)
                                       : Between(290, 360);
        }
        else
            eventTime += ms;
        const float durations[] = {20000, 20000, 22000, 22000, 42000};
        ufo.visible = witch.visible = monster.visible = ship.visible = cannon.visible = false;
        monsterReveal = beamStrength = fireStrength = muzzleFlash = impactStrength = 0;
        if (eventTime >= durations[caisEvent])
        {
            lastEvent = caisEvent;
            caisEvent = -1;
            idleRemaining = Between(9000, 17000);
            return;
        }
        float t = eventTime, p = t / durations[caisEvent];
        auto travel = [&](float a, float b) { return eventDirection == 1 ? Mix(a, b, p) : Mix(b, a, p); };
        if (caisEvent == 0)
        {
            ufo = {true, travel(-60, 700), 96 + std::sin(p * Pi * 2) * 4, 0.3f, 0.9f};
        }
        else if (caisEvent == 1)
        {
            if (t > 6000 && t < 13000)
                targetX =
                    std::clamp(targetX + std::clamp((targetSlot == 0 ? p1 : p2) - targetX, -ms * 0.012f, ms * 0.012f),
                               90.0f, 550.0f);
            float arrival = Ease(t / 6000), exit = Ease((t - 13000) / 7000);
            ufo = {true, Mix(eventDirection == 1 ? -70.0f : 710.0f, targetX, arrival),
                   Mix(Mix(80, 142, arrival), -110, exit) + std::sin(t / 1100) * 2,
                   Mix(Mix(0.3f, 1, arrival), 0.14f, exit), 1 - Ease((t - 18000) / 2000)};
            beamStrength = Ease((t - 5500) / 2500) * (1 - Ease((t - 13500) / 2000));
        }
        else if (caisEvent == 2)
        {
            witch = {true, travel(-70, 710), 126 + std::sin(p * Pi * 2) * 9, 0.65f, 0.9f, 0, eventDirection};
        }
        else
        {
            bool battle = caisEvent == 4;
            float rise = battle ? 9000.0f : 0, sink = battle ? 29000.0f : 16000.0f,
                  fireStart = battle ? 17000.0f : 9000.0f, fireEnd = battle ? 20500.0f : 13000.0f;
            monsterReveal = Ease((t - rise) / 6000) * (1 - Ease((t - sink) / 6000));
            fireStrength = Ease((t - fireStart) / 700) * (1 - Ease((t - fireEnd) / 900));
            monster = {monsterReveal > 0,
                       targetX,
                       251,
                       0.9f,
                       1,
                       fireStrength > 0 ? 2 : t > fireStart - 1600 && t < fireEnd ? 1 : 0,
                       eventDirection};
            if (battle)
            {
                float x = Mix(Mix(-150, 150, Ease(t / 10000)), 790, Ease((t - 31000) / 11000));
                ship = {true, eventDirection == 1 ? x : 640 - x, 249 + std::sin(t / 1700), 0.9f, 1, 0, eventDirection};
                for (int fired : {20000, 23500, 27000})
                {
                    float age = t - fired;
                    muzzleFlash = std::max(muzzleFlash, age >= 0 && age < 220 ? 1 - age / 220 : 0);
                    if (age >= 0 && age < 1600)
                    {
                        float q = age / 1600;
                        cannon = {true, Mix(ship.x + eventDirection * 55, monster.x - eventDirection * 7, q),
                                  Mix(ship.y - 26, monster.y - 66, q) - std::sin(q * Pi) * 20};
                    }
                    if (age >= 1600 && age < 2350)
                    {
                        impactStrength = 1 - (age - 1600) / 750;
                        monster.frame = 3;
                    }
                }
            }
        }
    }
    void UpdateKitchen(float ms)
    {
        kitchenTime += ms;
        nextBats -= ms;
        nextRat -= ms;
        if (nextBats <= 0)
        {
            int count = 2 + int(Random() * 4);
            for (int i = 0; i < count; ++i)
            {
                auto it = std::find_if(bats.begin(), bats.end(), [](const auto &a) { return !a.active; });
                if (it == bats.end())
                    break;
                *it = {true,
                       -i * 180.0f,
                       Between(4600, 7200),
                       139,
                       142,
                       139,
                       Random() < 0.2f ? -28.0f : 668.0f,
                       0,
                       Between(0, Pi * 2),
                       0};
            }
            nextBats = Between(8000, 14000);
        }
        if (nextRat <= 0)
        {
            auto it = std::find_if(rats.begin(), rats.end(), [](const auto &a) { return !a.active; });
            if (it != rats.end())
            {
                bool right = Random() < 0.5f;
                float lanes[] = {249, 262, 278};
                float lane = lanes[int(Random() * 3)];
                *it = {true,
                       0,
                       Between(6500, 10000),
                       right ? -30.0f : 670.0f,
                       lane,
                       right ? -30.0f : 670.0f,
                       right ? 670.0f : -30.0f,
                       lane};
            }
            nextRat = Between(4000, 8500);
        }
        for (auto &a : bats)
            if (a.active)
            {
                a.age += ms;
                if (a.age >= a.duration)
                {
                    a.active = false;
                    continue;
                }
                float p = std::max(0.0f, a.age) / a.duration;
                a.x = std::round(Mix(a.startX, a.endX, p));
                a.y = std::round(142 - std::sin(p * Pi / 2) * 46 +
                                 std::sin(p * Pi * 5 + a.phase) * 10 * std::min(1.0f, p * 8));
                a.frame = int(std::floor(std::max(0.0f, a.age) / 100)) % 4;
            }
        for (auto &a : rats)
            if (a.active)
            {
                a.age += ms;
                if (a.age >= a.duration)
                {
                    a.active = false;
                    continue;
                }
                a.x = std::round(Mix(a.startX, a.endX, a.age / a.duration));
                a.y = a.lane;
                a.frame = int(a.age / 110) % 4;
            }
    }
    static void Walk(Animal &a)
    {
        a.action = Action::Walk;
        a.timer = 0;
        a.egg = -1;
    }
    static float Speed(int species)
    {
        const float speeds[] = {18, 23, 36, 43};
        return speeds[species];
    }
    static void Patrol(Animal &a, float seconds)
    {
        a.x += a.direction * Speed(a.species) * seconds;
        if (a.x >= 620)
        {
            a.x = 620;
            a.direction = -1;
        }
        if (a.x <= 20)
        {
            a.x = 20;
            a.direction = 1;
        }
    }
    void UpdateFarm(float ms)
    {
        farmTime += ms;
        nextFight -= ms;
        float seconds = ms / 1000;
        for (auto &egg : eggs)
            if (egg.active)
            {
                egg.age += ms;
                if (egg.age > 60000)
                    egg.active = false;
            }
        auto &snake = animals[4];
        auto &lizard = animals[5];
        if (nextFight <= 0 && !meeting && snake.action == Action::Walk && lizard.action == Action::Walk)
        {
            meeting = true;
            meetingX = std::clamp((snake.x + lizard.x) * 0.5f, 140.0f, 500.0f);
            snake.action = lizard.action = Action::Challenge;
            snake.egg = lizard.egg = -1;
        }
        for (auto &a : animals)
        {
            a.timer += ms;
            if (a.action == Action::Lay)
            {
                a.frame = 3;
                if (!a.laid && a.timer >= 450)
                {
                    auto it = std::find_if(eggs.begin(), eggs.end(), [](const auto &e) { return !e.active; });
                    if (it != eggs.end())
                    {
                        *it = {true, a.x - a.direction * 9, a.y, 0, eggsLaid % 2 == 0 ? 2 : 3};
                        ++eggsLaid;
                    }
                    a.laid = true;
                }
                if (a.timer >= 1150)
                    Walk(a);
                continue;
            }
            if (a.action == Action::Fight)
            {
                a.frame = 3;
                if (a.timer >= 1350)
                {
                    a.action = Action::Retreat;
                    a.timer = 0;
                    a.direction = a.species == 2 ? -1 : 1;
                }
                continue;
            }
            if (a.action == Action::Eat)
            {
                a.frame = int(a.timer / 180) % 3;
                if (a.timer >= 1100)
                    Walk(a);
                continue;
            }
            if (a.action == Action::Retreat)
            {
                a.x += a.direction * Speed(a.species) * seconds;
                a.frame = int(a.timer / 130) % 3;
                if (a.timer >= 1400)
                    Walk(a);
                continue;
            }
            if (a.action == Action::Challenge && meeting)
            {
                a.direction = a.species == 2 ? 1 : -1;
                a.x = Approach(a.x, meetingX + (a.species == 2 ? -15 : 15), Speed(a.species) * seconds);
                a.y = Approach(a.y, 267, 32 * seconds);
            }
            else if (a.species >= 2)
            {
                if (a.egg < 0 || !eggs[a.egg].active || eggs[a.egg].eater != a.species)
                {
                    a.egg = -1;
                    for (int i = 0; i < 6; ++i)
                        if (eggs[i].active && eggs[i].eater == a.species)
                        {
                            a.egg = i;
                            break;
                        }
                }
                if (a.egg >= 0)
                {
                    auto &egg = eggs[a.egg];
                    a.direction = egg.x >= a.x ? 1 : -1;
                    a.x = Approach(a.x, egg.x - a.direction * 18, Speed(a.species) * seconds);
                    a.y = Approach(a.y, egg.y, 32 * seconds);
                    if (std::fabs(egg.x - a.x) <= 20 && std::fabs(egg.y - a.y) < 3)
                    {
                        egg.active = false;
                        a.egg = -1;
                        a.action = Action::Eat;
                        a.timer = 0;
                    }
                }
                else
                    Patrol(a, seconds);
            }
            else
            {
                Patrol(a, seconds);
                if (a.species == 0)
                {
                    a.layIn -= ms;
                    if (a.layIn <= 0)
                    {
                        a.action = Action::Lay;
                        a.timer = 0;
                        a.laid = false;
                        a.layIn = Between(12000, 20000);
                    }
                }
            }
            a.frame = int(farmTime / (a.species == 1 ? 155 : 130)) % (a.species == 1 ? 4 : 3);
        }
        if (meeting && snake.action == Action::Challenge && lizard.action == Action::Challenge &&
            std::fabs(snake.x - (meetingX - 15)) < 1 && std::fabs(lizard.x - (meetingX + 15)) < 1 &&
            std::fabs(snake.y - 267) < 1 && std::fabs(lizard.y - 267) < 1)
        {
            snake.action = lizard.action = Action::Fight;
            snake.timer = lizard.timer = 0;
            snake.frame = lizard.frame = 3;
            meeting = false;
            nextFight = Between(22000, 36000);
        }
    }

    void DrawCaisBack(const RDA::WorldSnapshot &s) const
    {
        Rect(0, 0, 640, 360, 0x040d20);
        Background("cais/background", 13);
        float moonX = 430 + std::round(skyOffset);
        Ellipse(moonX, 96, 96, 96, 0xa9daf1, 0.008f);
        Ellipse(moonX, 96, 80, 80, 0xa9daf1, 0.012f);
        Ellipse(moonX, 96, 66, 66, 0xa9daf1, 0.018f);
        if (!s.visualFinisher.active && caisEvent == 0)
            Actor("cais/ufo", ufo);
        Sprite("cais/moon", moonX, 96);
        if (!s.visualFinisher.active)
            Actor("cais/witch", witch);
        for (int i = 0; i < 3; ++i)
        {
            auto t = T("cais/water-" + std::to_string(i));
            float offset = std::round(std::sin(caisTime / (3100 + i * 900) + i) * (2 + i));
            if (t.id)
                DrawTexturePro(t, Rectangle{offset, 0, 512, 14}, Rectangle{64, float(209 + i * 15), 512, 14},
                               Vector2{0, 0}, 0, C(0xffffff, 0.16f + (1 + std::sin(caisTime / 2400 + i)) * 0.035f));
        }
        for (int i = 0; i < 12; ++i)
            Rect(moonX + std::sin(caisTime / 2400 + i * 2.4f) * (4 + i * 0.65f), 206 + i * 4.5f, 5 + i * 1.8f, 1,
                 0xb5e5ee, 0.06f + (1 + std::sin(caisTime / 1700 + i)) * 0.055f);
        const auto &f = s.visualFinisher;
        if (f.active)
        {
            if (f.monsterVisible)
                Sprite("cais/monster-finisher-v2", f.monsterX, f.monsterY, 12, f.monsterFrame, 0.5f, 1, f.monsterScale,
                       f.monsterFlip, 1, std::ceil(192 - (1 - f.monsterReveal) * 160));
            Rect(0, 0, 640, 360, 0x081021, std::min(1.0f, f.frame / 24.0f) * 0.1f);
            Rect(0, 0, 640, 360, 0x080016, 0.2f + (f.frame >= 112 ? 0.12f : 0));
        }
        else
        {
            Actor("cais/ship", ship, 1, true);
            if (monster.visible)
                Sprite("cais/monster", monster.x, monster.y + (1 - monsterReveal) * 144 * monster.scale, 4,
                       monster.frame, 0.5f, 1, monster.scale, monster.direction < 0, 1, std::ceil(144 * monsterReveal));
            if (fireStrength > 0)
            {
                Ellipse(monster.x, 253, 76, 10, 0xfba649, fireStrength * 0.12f);
                auto t = T("cais/fire");
                float x = monster.x + monster.direction * 12 * monster.scale, y = monster.y - 106 * monster.scale;
                if (t.id)
                    DrawTexturePro(
                        t, Rectangle{float((int(caisTime / 160) % 4) * 64), 0, 64, 96},
                        Rectangle{x - 64 * 0.9f / 2, y - 96 * 0.9f * fireStrength, 64 * 0.9f, 96 * 0.9f * fireStrength},
                        Vector2{0, 0}, 0, C(0xffffff, fireStrength * 0.95f));
            }
            if (monsterReveal > 0 && monsterReveal < 0.99f)
                Sprite("cais/splash", monster.x, monster.y + 5, 4, int(caisTime / 240) % 4, 0.5f, 1, 1, false,
                       0.4f + std::sin(monsterReveal * Pi) * 0.4f);
            if (caisEvent == 1)
                Actor("cais/ufo", ufo);
            if (beamStrength > 0)
            {
                float x = std::round(ufo.x), top = std::round(ufo.y + 15 * ufo.scale);
                DrawTriangle(Vector2{x, top}, Vector2{x - 57, 304}, Vector2{x + 57, 304},
                             C(0x72dae8, beamStrength * 0.085f));
                DrawTriangle(Vector2{x, top}, Vector2{x - 23, 304}, Vector2{x + 23, 304},
                             C(0xbeffff, beamStrength * 0.06f));
                Ellipse(x, 304, 115, 16, 0x75dce9, beamStrength * 0.1f);
                for (int i = 0; i < 8; ++i)
                {
                    float p = std::fmod(caisTime / 4300 + i / 8.0f, 1.0f);
                    Rect(x + std::sin(i * 5 + caisTime / 1600) * (1 - p) * 46, 304 - p * (304 - top), 1, 2, 0xc0ffff,
                         beamStrength * (1 - p) * 0.35f);
                }
            }
            if (cannon.visible)
            {
                DrawCircle(int(std::round(cannon.x)), int(std::round(cannon.y)), 3, C(0x02050a));
                Rect(cannon.x - 1, cannon.y - 2, 2, 1, 0x97a9b5);
            }
            if (muzzleFlash > 0)
            {
                float x = ship.x + ship.direction * 55, y = ship.y - 26;
                DrawCircle(int(x), int(y), 6, C(0xf4a249, muzzleFlash));
                Rect(x - 2, y - 2, 4, 4, 0xffefbc, muzzleFlash);
            }
            if (impactStrength > 0)
                for (int i = 0; i < 6; ++i)
                {
                    float radius = (1 - impactStrength) * 17;
                    Rect(monster.x - ship.direction * 7 + std::cos(i * 1.05f) * radius,
                         monster.y - 66 + std::sin(i * 1.05f) * radius, 2, 2, i % 2 ? 0xf6cc73 : 0x9abdc7,
                         impactStrength * 0.75f);
                }
        }
    }
    void DrawCaisFront(const RDA::WorldSnapshot &s) const
    {
        const auto &v = s.visualFinisher;
        if (!v.active)
            return;
        int f = v.frame;
        float wx = std::round(v.monsterX), wy = 250;
        if ((f >= 112 && f < 155) || (f >= 240 && f < 282))
            Sprite("cais/splash", wx, wy + 4, 4, (f / 6) % 4, 0.5f, 1, 1.5f, false, 0.9f);
        if (f >= 160 && f < 238)
            for (int i = 0; i < 9; ++i)
            {
                float p = ((f - 160 + i * 7) % 24) / 24.0f;
                Rect(v.mouthX + std::sin(i * 4.0f) * p * 42, v.mouthY + p * p * 80, 2, 3, i % 2 ? 0x8c1736 : 0xc4424f,
                     (1 - p) * 0.85f);
            }
        if (f >= 112)
        {
            for (int ring = 0; ring < 4; ++ring)
            {
                float p = ((f - 112 + ring * 13) % 72) / 72.0f;
                Ring(wx, wy, 40 + p * 180, 5 + p * 20, ring % 2 ? 0xc7eff3 : 0x77bfd0, (1 - p) * 0.45f);
            }
            if (f < 150 || (f > 248 && f < 280))
                for (int i = 0; i < 18; ++i)
                {
                    float p = ((f + i * 3) % 26) / 26.0f;
                    Rect(wx + std::cos(i * 2.4f) * p * 90, wy - 4 - 56 * 4 * p * (1 - p), 2, 3,
                         i % 2 ? 0xc4f3ef : 0x55a5bf, (1 - p) * 0.8f);
                }
        }
    }
    void DrawKitchenBack(const RDA::WorldSnapshot &s) const
    {
        Background("kitchen/background");
        bool active = s.visualFinisher.active;
        int f = s.visualFinisher.frame;
        bool cooking = active && f >= 108;
        Sprite("kitchen/witch", 408, 163, 4, active && f > 0 ? (f / 6) % 4 : int(kitchenTime / 220) % 4);
        if (cooking)
        {
            Ellipse(442, 152, 62, 42, 0x24160e);
            Ellipse(442, 151, 52, 34, 0x3a2416);
            Ellipse(444, 149, 36, 20, 0x4c3020);
            for (int i = 0; i < 7; ++i)
            {
                float p = ((f * 3 + i * 37) % 90) / 90.0f;
                Rect(442 + std::cos(i * 2.1f) * 18 * (1 - p * 0.4f), 162 - p * 14, 2 + i % 2, 2,
                     i % 2 ? 0x7d5330 : 0xa5743f, (1 - p) * 0.9f);
            }
        }
        for (int i = 0; i < 6; ++i)
        {
            float clock = active && f > 0 ? f * 16.0f : kitchenTime, boil = cooking ? 1.6f : 1;
            float p = std::fmod(clock + i * 430, 2600.0f) / 2600;
            Rect(443 + std::sin(p * 6 + i) * 7 * boil, 148 - p * 30 * boil, 2 + std::floor(p * 4),
                 3 + std::floor(p * 5), cooking ? 0xd9d2c3 : 0xa8bf78, (1 - p) * (cooking ? 0.5f : 0.28f));
        }
        if (!cooking)
        {
            for (const auto &a : bats)
                if (a.active && a.age >= 0)
                    Sprite("kitchen/bat", a.x, a.y, 4, a.frame, 0.5f, 0.5f, 1, a.endX < a.startX);
            for (const auto &a : rats)
                if (a.active)
                    Sprite("kitchen/rat", a.x, a.y, 4, a.frame, 0.5f, 0.5f, 1, a.endX < a.startX);
        }
    }
    void DrawKitchenFront(const RDA::WorldSnapshot &s) const
    {
        if (!s.visualFinisher.active || s.visualFinisher.frame < 108)
            return;
        int f = s.visualFinisher.frame;
        int shift = f % 16, spoon = shift < 8 ? shift - 4 : 12 - shift;
        if (f >= 210)
            Sprite("kitchen/skull-bones", 442.0f + spoon, 156.0f + f % 2, 1, 0, 0.5f, 0.6f);
        for (int offset = -1; offset <= 1; ++offset)
            Ring(442, 152 + float(offset), 58, 40, 0x161616);
        Ring(442, 150, 46, 30, 0x3a3a3a);
        DrawLineEx(Vector2{424, 122}, Vector2{442.0f + spoon * 2, 154}, 3, C(0x6b3e22));
        Rect(440.0f + spoon * 2, 152, 5, 3, 0x8d5a32);
        int age = f - 108;
        if (age < 26)
            for (int i = 0; i < 14; ++i)
            {
                float t = age / 26.0f;
                Rect(442 + std::cos(i * 2.4f) * (10 + i % 5 * 8) * t,
                     144 - std::fabs(std::sin(i * 1.7f)) * 34 * t + 60 * t * t, 2, 3, i % 2 ? 0xc99658 : 0x79502c,
                     1 - t);
            }
    }
    void DrawFarmBack(const RDA::WorldSnapshot &s) const
    {
        Background("sitio/background");
        const auto &v = s.visualFinisher;
        if (v.active && v.doors > 0)
            Rect(271, 129, 113, 87, 0x120c08);
        if (v.active && v.farmerVisible)
            Sprite("sitio/masked-farmer", v.farmerX, v.farmerFeetY, 3, v.farmerFrame, 0.5f, 1, 1, v.farmerFacing < 0);
    }
    void DrawFarmAmbience() const
    {
        if(!decorativeEffects)return;
        for (const auto &egg : eggs)
            if (egg.active)
            {
                Rect(egg.x - 2, egg.y - 7, 4, 7, 0x76553b);
                Rect(egg.x - 3, egg.y - 5, 6, 4, 0x76553b);
                Rect(egg.x - 1, egg.y - 6, 2, 5, 0xffefc0);
                Rect(egg.x - 2, egg.y - 4, 4, 3, 0xffefc0);
                Rect(egg.x - 1, egg.y - 5, 1, 3, 0xffffff);
            }
        static const char *names[] = {"sitio/hen", "sitio/duck", "sitio/snake", "sitio/lizard"};
        for (const auto &a : animals)
        {
            float tussle = a.action == Action::Fight ? std::round(std::sin(a.timer / 70) * 2) : 0,
                  bob = a.action == Action::Eat ? std::round(std::sin(a.timer / 100)) : 0;
            Sprite(names[a.species], std::round(a.x) + tussle, std::round(a.y) + bob, 4, a.frame, 0.5f, 1, 1,
                   a.direction < 0);
        }
        if (animals[4].action == Action::Fight)
            for (int i = 0; i < 5; ++i)
            {
                float p = std::fmod(animals[4].timer + i * 170, 650.0f) / 650;
                Rect((animals[4].x + animals[5].x) * 0.5f + std::sin(i * 2 + p * 5) * 13, animals[4].y - 3 - p * 12, 3,
                     2, 0xffd18a, (1 - p) * 0.65f);
            }
    }
    void DrawFarmFront(const RDA::WorldSnapshot &s) const
    {
        const auto &v = s.visualFinisher;
        if (!v.active)
        {
            DrawFarmAmbience();
            return;
        }
        int f = v.frame;
        if (v.farmerVisible)
            Sprite("sitio/trident", v.tridentX, v.tridentBaseY, 1, 0, 0.5f, 1);
        if (f >= 112 && f < 282)
        {
            int age = f - 112;
            float tx = v.tridentTipX, ty = v.tridentTipY;
            if (v.blood > 0)
                for (int i = 0; i < 18; ++i)
                {
                    float t = std::min(1.0f, age / 20.0f);
                    Rect(tx + std::cos(i * 1.9f) * (8 + i % 5 * 6) * t,
                         ty - std::fabs(std::sin(i * 2.3f)) * 16 * t + 22 * t * t, 2 + i % 2, 2 + i % 2,
                         i % 3 ? 0x8c1a1a : 0xc4303a, v.blood * 0.95f);
                }
            float pool = std::min(1.0f, age / 30.0f);
            Rect(tx - 3, ty + 2, 6, 2, 0x7a1414, 0.9f * pool);
            Rect(tx - 2, ty + 4, 4, 1, 0x7a1414, 0.9f * pool);
            for (int i = 0; i < 4; ++i)
                if (age >= i * 23)
                {
                    float p = ((age + i * 23) % 64) / 64.0f;
                    Rect(tx - 1 + i % 2, ty + 6 + p * 26, 1, 2 + i % 2, 0x7a1414, 0.9f);
                }
        }
        Rect(271, 129, 113, 87, 0x070503, v.shadow * 0.94f);
        if (v.doors > 0)
        {
            auto t = T("sitio/background");
            if (t.id)
            {
                float width = std::max(0.06f, 1 - v.doors * 0.94f), dark = std::round(255 - v.doors * 190) / 255;
                auto tint = C(0xffffff);
                tint.r = tint.g = tint.b = static_cast<unsigned char>(dark * 255);
                DrawTexturePro(t, Rectangle{271, 129, 56, 87}, Rectangle{271, 129, 56 * width, 87}, Vector2{0, 0}, 0,
                               tint);
                DrawTexturePro(t, Rectangle{327, 129, 57, 87}, Rectangle{384 - 57 * width, 129, 57 * width, 87},
                               Vector2{0, 0}, 0, tint);
            }
        }
        DrawFarmAmbience();
    }
};
} // namespace RDAStageVisuals
