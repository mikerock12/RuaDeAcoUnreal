#pragma once

#include <array>
#include <cstdint>
#include <string>
#include <vector>

namespace RDA
{
	inline constexpr int kLogicalWidth = 640;
	inline constexpr int kLogicalHeight = 360;
	inline constexpr int kGroundY = 304;
	inline constexpr int kStageLeft = 36;
	inline constexpr int kStageRight = 604;
	inline constexpr int kRoundTimeFrames = 99 * 60;
	inline constexpr int kRoundsToWin = 2;
	inline constexpr int kMaxMeter = 100;
	inline constexpr int kLandingFrames = 3;
	inline constexpr int kWakeUpFrames = 22;
	inline constexpr int kFinishWindowFrames = 480;
	inline constexpr int kFinishThrowFrame = 60;
	inline constexpr int kFinishEndFrame = 300;
	inline constexpr float kFixedStepSeconds = 1.0f / 60.0f;

	enum class FighterId : uint8_t
	{
		GutoBarba = 0,
		RafaMare = 1,
		NoirReflexo = 2,
		AstroRiso = 3,
		DanteSinal = 4,
		LeoVioleta = 5,
	};

	inline constexpr int kFighterCount = 6;

	enum class StageId : uint8_t
	{
		CaisDaCidade = 0,
		CozinhaMacabra = 1,
		Sitio = 2,
	};

	inline constexpr int kStageCount = 3;

	enum class GameMode : uint8_t
	{
		VsCpu = 0,
		LocalVersus = 1,
		Training = 2,
	};

	enum class AiDifficulty : uint8_t
	{
		Easy = 0,
		Normal = 1,
		Hard = 2,
	};

	enum class FighterState : uint8_t
	{
		Idle = 0,
		WalkForward,
		WalkBackward,
		Jump,
		Fall,
		Landing,
		Crouch,
		BlockStanding,
		BlockCrouching,
		LightAttack,
		HeavyAttack,
		KickAttack,
		SpecialAttack,
		HitStun,
		Knockdown,
		WakeUp,
		GrabbedFront,
		GrabbedLifted,
		Thrown,
		Frozen,
		Victory,
		Knockout,
	};

	enum class CombatPhase : uint8_t
	{
		Intro = 0,
		Active,
		FinishReady,
		StageFinish,
		RoundOver,
		MatchOver,
	};

	enum class HitLevel : uint8_t
	{
		High = 0,
		Mid,
		Low,
		Overhead,
	};

	enum class HitKind : uint8_t
	{
		Strike = 0,
		Throw = 1,
		Projectile = 2,
	};

	enum class AnimId : uint8_t
	{
		Idle = 0,
		Walk,
		WalkBackward,
		JumpNeutral,
		JumpForward,
		JumpBackward,
		Fall,
		Landing,
		Crouch,
		StandingLight,
		StandingHeavy,
		ForwardLight,
		ForwardHeavy,
		CrouchLight,
		CrouchHeavy,
		AirLight,
		AirHeavy,
		BlockStanding,
		BlockCrouching,
		Hit,
		Knockdown,
		WakeUp,
		Knockout,
		Victory,
		UniversalGrab,
		GrabbedFront,
		GrabbedLifted,
		Thrown,
		Frozen,
		Special1,
		Special2,
		Super,
		Count
	};

	struct InputFrame
	{
		bool left = false;
		bool right = false;
		bool up = false;
		bool down = false;
		bool punch = false;
		bool kick = false;
		bool special = false;
		bool block = false;

		bool upPressed = false;
		bool downPressed = false;
		bool leftPressed = false;
		bool rightPressed = false;
		bool punchPressed = false;
		bool kickPressed = false;
		bool specialPressed = false;
		bool blockPressed = false;
		bool grabPressed = false;
		bool superPressed = false;
		bool pausePressed = false;
		bool confirmPressed = false;
		bool debugPressed = false;
	};

	struct Rect
	{
		float x = 0.0f;
		float y = 0.0f;
		float width = 0.0f;
		float height = 0.0f;
	};

	struct HitboxDef
	{
		Rect rect{};
		HitLevel level = HitLevel::Mid;
		HitKind kind = HitKind::Strike;
		int activeFrom = 1;
		int activeTo = 1;
		int damage = 0;
		int chipDamage = 0;
		int hitStun = 10;
		int blockStun = 8;
		int hitStop = 5;
		int priority = 1;
		float knockbackX = 2.0f;
		float knockbackY = 0.0f;
		bool knockdown = false;
		bool crouchAvoidable = false;
		bool airAvoidable = false;
		int freezeFrames = 0;
	};

	struct ProjectileDef
	{
		const char* id = "";
		float offsetX = 36.0f;
		float offsetY = -52.0f;
		float velocityX = 4.8f;
		int lifeFrames = 60;
		int armingFrames = 0;
		bool targetSnapshot = false;
		HitboxDef hitbox{};
	};

	struct MoveDef
	{
		const char* id = "";
		const char* label = "";
		FighterState state = FighterState::LightAttack;
		AnimId anim = AnimId::StandingLight;
		bool air = false;
		bool crouch = false;
		bool isGrab = false;
		bool isSpecial = false;
		bool isSuper = false;
		int meterCost = 0;
		int totalFrames = 18;
		int meterGainOnHit = 8;
		int meterGainOnBlock = 4;
		float moveVelX = 0.0f;
		int moveFrom = 0;
		int moveTo = 0;
		int spawnProjectileFrame = -1;
		int projectileIndex = -1;
		int grantArmorFrame = -1;
		int grantArmorHits = 0;
		int grantParryFrame = -1;
		int grantParryDuration = 0;
		int grantParryDamage = 0;
		int grantDamageReductionFrame = -1;
		int grantDamageReductionDuration = 0;
		float grantDamageReductionMultiplier = 1.0f;
		int holdStartFrame = 14;
		int releaseFrame = 46;
		int whiffRecoveryFrame = 50;
		HitboxDef hitbox{};
	};

	struct FighterStats
	{
		int maxHealth = 1000;
		float walkSpeed = 3.5f;
		float backwardSpeed = 2.8f;
		float jumpSpeed = 14.0f;
		float jumpForwardSpeed = 3.0f;
		float jumpBackwardSpeed = 2.6f;
		float gravity = 0.84f;
		float weight = 1.0f;
		Rect pushbox{-15.0f, -56.0f, 30.0f, 56.0f};
	};

	struct FighterDef
	{
		FighterId id = FighterId::GutoBarba;
		const char* canonicalId = "guto-barba";
		const char* name = "GUTO BARBA";
		const char* archetype = "TANK / GRAPPLER";
		const char* ability1 = "";
		const char* ability2 = "";
		const char* ability3 = "";
		const char* assetFolder = "GutoBarba";
		const char* portraitFile = "guto-barba.png";
		const char* voiceFolder = "guto";
		const char* specialSfxFile = "gelo.ogg";
		const char* superSfxFile = "super-abraco-glacial.ogg";
		int spriteFrameSize = 288;
		int visualSolePad = 29;
		uint32_t bodyColor = 0x61488e;
		uint32_t accentColor = 0xa8e9ff;
		FighterStats stats{};
		std::array<Rect, 3> standingHurtboxes{};
		std::array<Rect, 2> crouchingHurtboxes{};

		MoveDef standingLight{};
		MoveDef standingHeavy{};
		MoveDef forwardLight{};
		MoveDef forwardHeavy{};
		MoveDef crouchLight{};
		MoveDef crouchHeavy{};
		MoveDef airLight{};
		MoveDef airHeavy{};
		MoveDef universalGrab{};
		MoveDef special1{};
		MoveDef special2{};
		MoveDef superMove{};
		std::array<ProjectileDef, 2> projectiles{};
		int projectileCount = 0;
	};

	struct StageDef
	{
		StageId id = StageId::CaisDaCidade;
		const char* canonicalId = "cais-da-cidade";
		const char* name = "CAIS DA CIDADE";
		const char* subtitle = "NOITE | ZONA PORTUARIA";
		const char* folder = "CaisDaCidade";
		const char* musicFile = "cais-da-cidade.ogg";
		const char* finisherBanner = "O CAIS COBRA SUA ALMA";
	};

	inline Rect ToWorldRect(const Rect& Local, float OriginX, float OriginY, int Facing)
	{
		const float WorldX = Facing >= 0
			? OriginX + Local.x
			: OriginX - Local.x - Local.width;
		return Rect{WorldX, OriginY + Local.y, Local.width, Local.height};
	}

	inline bool Intersects(const Rect& A, const Rect& B)
	{
		return A.x < B.x + B.width
			&& A.x + A.width > B.x
			&& A.y < B.y + B.height
			&& A.y + A.height > B.y;
	}

	inline float HorizontalOverlap(const Rect& A, const Rect& B)
	{
		const float Left = A.x > B.x ? A.x : B.x;
		const float RightA = A.x + A.width;
		const float RightB = B.x + B.width;
		const float Right = RightA < RightB ? RightA : RightB;
		return Right > Left ? Right - Left : 0.0f;
	}
}
