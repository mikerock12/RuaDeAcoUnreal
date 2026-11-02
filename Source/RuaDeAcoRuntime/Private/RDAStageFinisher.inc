// Port das funções puras src/combat/stageFinisher.ts. Sem dependência gráfica.
#include "../../../Data/generated/RDAGrabGeometry.inc"
namespace
{
constexpr float kPi = 3.14159265358979323846f;
float Clamp01(float v)
{
    return std::clamp(v, 0.0f, 1.0f);
}
float Ease(float v)
{
    float t = Clamp01(v);
    return t * t * (3 - 2 * t);
}
float Mix(float a, float b, float t)
{
    return a + (b - a) * t;
}
float Wobble(int f, int p = 16)
{
    int t = f % p;
    return t < p / 2 ? t - p / 4.0f : p * 0.75f - t;
}
struct FinishPose
{
    FighterState state = FighterState::GrabbedLifted;
    int pose = 7;
    float x = 0, y = 0, rotation = 0, scale = 1;
    bool visible = true, front = true;
    float cut = -1;
};
struct Hold
{
    int pose;
    float tilt, x, y;
};
Hold FlightHold(int victim, int reaction = 0)
{
    int pose = reaction % 24 < 12 ? 7 : 6;
    return {pose, kLiftTilts[victim][pose], kLiftHolds[victim][pose][0], kLiftHolds[victim][pose][1]};
}
FinishPose PlaceHold(Hold h, float hx, float hy, float angle, int facing, float scale)
{
    float r = angle - h.tilt, c = std::cos(r), s = std::sin(r);
    FinishPose p;
    p.pose = h.pose;
    p.rotation = facing * r;
    p.scale = scale;
    p.x = hx + facing * (h.x * c + h.y * s) * scale;
    p.y = hy + (h.x * s - h.y * c) * scale;
    return p;
}
int FinisherAttackerFrame(int f)
{
    if (f < 12)
        return 6 + f / 3;
    if (f < 30)
        return 10 + (f - 12) * 13 / 18;
    if (f < 46)
        return 23;
    if (f < 54)
        return 29;
    if (f < 60)
        return 35;
    return std::min(65, 40 + int(std::floor((f - 60) * 1.1f)));
}

FinisherVisualSnapshot StageVisual(StageId stage, int f, const FighterRuntime &a)
{
    FinisherVisualSnapshot out;
    out.active = true;
    out.frame = f;
    out.attackerFrame = FinisherAttackerFrame(f);
    const int facing = a.facing;
    const float origin = a.x;
    if (stage == StageId::CaisDaCidade)
    {
        static const int keys[][2] = {{116, 0}, {124, 1}, {134, 2}, {144, 3}, {152, 4}, {160, 5},  {166, 6},
                                      {170, 7}, {183, 8}, {194, 9}, {207, 8}, {221, 9}, {240, 10}, {262, 11}};
        static const float mouths[][2] = {{56, -57}, {33, -115}, {24, -114}, {33, -111}, {20, -132}, {100, -82},
                                          {61, -82}, {58, -82},  {24, -115}, {24, -113}, {47, -59},  {68, -31}};
        for (const auto &key : keys)
            if (f >= key[0])
                out.monsterFrame = key[1];
        out.monsterReveal = Ease((f - 116) / 34.0f) * (1 - Ease((f - 244) / 45.0f));
        const float lunge = Ease((f - 152) / 14.0f) * (1 - Ease((f - 183) / 15.0f));
        const float chew = f >= 160 && f < 238 ? Wobble(f, 14) * 0.55f : 0;
        out.monsterX = std::clamp(origin + facing * 185.0f, 150.0f, 490.0f) - facing * (14 * lunge + chew);
        out.monsterY = 262 + (1 - out.monsterReveal) * 160 * out.monsterScale;
        out.monsterFlip = facing == 1;
        out.monsterVisible = out.monsterReveal > 0;
        out.mouthX = out.monsterX - facing * mouths[out.monsterFrame][0] * out.monsterScale;
        out.mouthY = out.monsterY + mouths[out.monsterFrame][1] * out.monsterScale;
    }
    if (stage == StageId::Sitio)
    {
        out.doors = Ease((f - 30) / 50.0f) * (1 - Ease((f - 244) / 38.0f));
        out.farmerFacing = origin <= 327.5f ? -1 : 1;
        out.farmerFrame = f < 112 ? (f >= 96 ? 1 : 0) : (f < 150 ? 2 : 1);
        const float hands[][2] = {{8, -41}, {8, -53}, {8, -38}};
        const float brace = f >= 112 && f < 150 ? 4 : 0, advance = Ease((f - 62) / 24.0f),
                    retreat = Ease((f - 232) / 36.0f);
        const float recoil = f >= 112 && f < 128 ? Wobble(f - 112, 8) * 0.5f : 0;
        out.farmerX = 327.5f + out.farmerFacing * (advance * 5 - retreat * 7 + recoil);
        out.farmerFeetY = 217 + retreat * 4;
        out.farmerVisible = f >= 62 && f < 282;
        out.tridentX = out.farmerX + out.farmerFacing * hands[out.farmerFrame][0];
        const float gripY = out.farmerFeetY + hands[out.farmerFrame][1] + brace;
        out.tridentBaseY = gripY + 26;
        out.tridentTipX = out.tridentX;
        out.tridentTipY = gripY - 22;
        out.shadow = Ease((f - 232) / 26.0f);
        out.blood = f >= 112 ? 1 - Ease((f - 112) / 40.0f) : 0;
    }
    out.bonesVisible = stage == StageId::CozinhaMacabra && f >= 210;
    return out;
}
std::pair<float, float> Hand(int attacker, int f)
{
    static const int starts[] = {0, 3, 6, 10, 14, 18, 23, 29, 35, 40, 46, 58};
    int i = 0;
    for (int j = 1; j < 12; ++j)
        if (f >= starts[j])
            i = j;
    int next = std::min(11, i + 1);
    float t = starts[next] == starts[i] ? 0 : Ease(float(f - starts[i]) / (starts[next] - starts[i]));
    return {Mix(kGrabHands[attacker][i][0], kGrabHands[attacker][next][0], t),
            Mix(kGrabHands[attacker][i][1], kGrabHands[attacker][next][1], t)};
}
FinishPose Flight(Hold hold, int f, int start, int end, float fx, float fy, float tx, float ty, float height, int spins,
                  float endAngle, float endScale, int facing)
{
    float t = Clamp01(float(f - start) / (end - start));
    float scale = Mix(1, endScale, Ease(t));
    float angle = Mix(kPi / 2, endAngle + spins * kPi * 2, t);
    return PlaceHold(hold, Mix(fx, tx, t), Mix(fy, ty, t) - height * 4 * t * (1 - t), angle, facing, scale);
}
FinishPose StageVictim(StageId stage, int f, const FighterRuntime &attacker, const FighterRuntime &victim)
{
    int ai = int(attacker.def->id), vi = int(victim.def->id), face = attacker.facing;
    float origin = attacker.x;
    if (f < 60)
    {
        const auto &poses = attacker.def->universalGrab.grabPoses[vi];
        const auto &h = poses[FinisherAttackerFrame(f)];
        FinishPose p;
        p.state = h.state;
        p.pose = h.poseFrame;
        p.x = origin + face * h.x;
        p.y = kGroundY + h.y;
        p.rotation = face * h.rotation;
        p.front = FinisherAttackerFrame(f) >= 35;
        return p;
    }
    Hold hold = FlightHold(vi);
    auto hand = Hand(ai, FinisherAttackerFrame(59));
    float fromX = origin + face * hand.first, fromY = kGroundY + hand.second;
    if (stage == StageId::CaisDaCidade)
    {
        float base = std::clamp(origin + face * 185.0f, 150.0f, 490.0f);
        if (f < 112)
            return Flight(hold, f, 60, 112, fromX, fromY, base - face * 10, 256, 92, 1, kPi / 2, 0.8f, face);
        static const int keys[][2] = {{116, 0}, {124, 1}, {134, 2}, {144, 3}, {152, 4}, {160, 5},  {166, 6},
                                      {170, 7}, {183, 8}, {194, 9}, {207, 8}, {221, 9}, {240, 10}, {262, 11}};
        static const float mouths[][2] = {{56, -57}, {33, -115}, {24, -114}, {33, -111}, {20, -132}, {100, -82},
                                          {61, -82}, {58, -82},  {24, -115}, {24, -113}, {47, -59},  {68, -31}};
        int pose = 0;
        for (const auto &key : keys)
            if (f >= key[0])
                pose = key[1];
        float reveal = Ease((f - 116) / 34.0f) * (1 - Ease((f - 244) / 45.0f));
        float lunge = Ease((f - 152) / 14.0f) * (1 - Ease((f - 183) / 15.0f));
        float chew = f >= 160 && f < 238 ? Wobble(f, 14) * 0.55f : 0;
        float mx = base - face * (14 * lunge + chew) - face * mouths[pose][0] * 1.35f;
        float my = 262 + (1 - reveal) * 160 * 1.35f + mouths[pose][1] * 1.35f;
        auto p = PlaceHold(hold, mx, my, kPi / 2 + Wobble(f, 12) * 0.03f, face, 0.8f);
        p.visible = f >= 134 && f < 160;
        return p;
    }
    if (stage == StageId::CozinhaMacabra)
    {
        if (f >= 108)
            hold = FlightHold(vi, f);
        if (f < 108)
            return Flight(hold, f, 60, 108, fromX, fromY, 442, 148, 76, 0, kPi, 0.55f, face);
        float sink = Ease((f - 108) / 70.0f), kick = Wobble(f, 12) * 0.035f * (1 - sink);
        auto p = PlaceHold(hold, 442 + Wobble(f, 16) * 0.5f, 148 + sink * 78, kPi + kick, face, 0.55f);
        p.visible = f < 178;
        p.cut = 160;
        return p;
    }
    int farmerFacing = origin <= 327.5f ? -1 : 1;
    int fp = f < 112 ? (f >= 96 ? 1 : 0) : (f < 150 ? 2 : 1);
    static const float farmerHands[][2] = {{8, -41}, {8, -53}, {8, -38}};
    float brace = f >= 112 && f < 150 ? 4 : 0, advance = Ease((f - 62) / 24.0f), retreat = Ease((f - 232) / 36.0f);
    float recoil = f >= 112 && f < 128 ? Wobble(f - 112, 8) * 0.5f : 0;
    float farmerX = 327.5f + farmerFacing * (advance * 5 - retreat * 7 + recoil), feetY = 217 + retreat * 4;
    float tx = farmerX + farmerFacing * farmerHands[fp][0], ty = feetY + farmerHands[fp][1] + brace - 22 + 4;
    if (f < 112)
        return Flight(hold, f, 60, 112, fromX, fromY, tx, ty, 70, 1, kPi / 2, 0.42f, face);
    float twitch = f < 150 ? Wobble(f, 8) * 0.03f : 0, sag = Ease((f - 150) / 50.0f);
    auto p = PlaceHold(hold, tx, ty + (f < 150 ? Wobble(f, 8) * 0.25f : 0) + sag * 3, kPi / 2 + sag * 0.22f + twitch,
                       face, 0.42f);
    p.visible = f < 282;
    return p;
}
} // namespace
