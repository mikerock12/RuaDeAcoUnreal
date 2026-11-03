#include "RDAFighter.h"
#include <cstring>

namespace RDA
{
	namespace
	{
		#include "../../../Data/generated/RDACanonicalData.inc"
		constexpr std::array<Rect, 3> kStandardStandingHurtboxes = {{
			{-11.0f, -68.0f, 22.0f, 22.0f},
			{-15.0f, -47.0f, 30.0f, 30.0f},
			{-13.0f, -18.0f, 26.0f, 18.0f},
		}};

		constexpr std::array<Rect, 2> kStandardCrouchingHurtboxes = {{
			{-12.0f, -43.0f, 24.0f, 19.0f},
			{-18.0f, -28.0f, 36.0f, 24.0f},
		}};

		MoveDef MakeUniversalGrab()
		{
			return MoveDef{
				"universalGrab",
				"Agarrar e esmagar",
				FighterState::HeavyAttack,
				AnimId::UniversalGrab,
				false,
				false,
				true,
				false,
				false,
				0,
				66,
				10,
				0,
				0.0f,
				0,
				0,
				-1,
				-1,
				-1,
				0,
				-1,
				0,
				0,
				-1,
				0,
				1.0f,
				14,
				46,
				50,
				HitboxDef{
					Rect{0.0f, -120.0f, 58.0f, 115.0f},
					HitLevel::Mid,
					HitKind::Throw,
					6,
					9,
					125,
					0,
					8,
					0,
					3,
					3,
					2.0f,
					10.0f,
					true,
					false,
					true,
					0
				}
			};
		}

		FighterDef BuildGutoBarba()
		{
			FighterDef Def{};
			Def.id = FighterId::GutoBarba;
			Def.canonicalId = "guto-barba";
			Def.name = "GUTO BARBA";
			Def.archetype = "TANK / GRAPPLER";
			Def.ability1 = "Muralha Norte";
			Def.ability2 = "Gancho do Urso";
			Def.ability3 = "Abraco Glacial";
			Def.assetFolder = "GutoBarba";
			Def.portraitFile = "guto-barba.png";
			Def.voiceFolder = "guto";
			Def.specialSfxFile = "gelo.ogg";
			Def.superSfxFile = "super-abraco-glacial.ogg";
			Def.spriteFrameSize = 288;
			Def.visualSolePad = 29;
			Def.bodyColor = 0x61488e;
			Def.accentColor = 0xa8e9ff;
			Def.stats = FighterStats{
				1250, 2.47f, 2.01f, 12.8f, 2.19f, 1.96f, 0.81f, 1.28f,
				Rect{-19.0f, -61.0f, 38.0f, 61.0f}
			};
			Def.standingHurtboxes = {{
				{-15.0f, -75.0f, 30.0f, 23.0f},
				{-21.0f, -53.0f, 42.0f, 35.0f},
				{-18.0f, -20.0f, 36.0f, 20.0f},
			}};
			Def.crouchingHurtboxes = kStandardCrouchingHurtboxes;
			Def.universalGrab = MakeUniversalGrab();

			Def.standingLight = MoveDef{
				"elbow", "Cotovelada", FighterState::LightAttack, AnimId::StandingLight,
				false, false, false, false, false, 0, 20, 9, 4, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -58.0f, 34.0f, 22.0f}, HitLevel::Mid, HitKind::Strike, 6, 9, 66, 2, 15, 10, 7, 2, 2.2f, 0.0f, false}
			};
			Def.standingHeavy = MoveDef{
				"heavyPunch", "Soco pesado", FighterState::HeavyAttack, AnimId::StandingHeavy,
				false, false, false, false, false, 0, 36, 15, 7, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{6.0f, -65.0f, 51.0f, 28.0f}, HitLevel::Mid, HitKind::Strike, 14, 18, 122, 8, 23, 16, 11, 4, 5.0f, -2.4f, true}
			};
			Def.forwardLight = MoveDef{
				"ombrada", "Ombrada", FighterState::LightAttack, AnimId::ForwardLight,
				false, false, false, false, false, 0, 23, 10, 4, 2.0f, 4, 10, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -60.0f, 36.0f, 24.0f}, HitLevel::Mid, HitKind::Strike, 9, 12, 72, 3, 15, 11, 7, 2, 2.6f, 0.0f, false}
			};
			Def.forwardHeavy = MoveDef{
				"martelo", "Martelo do Norte", FighterState::HeavyAttack, AnimId::ForwardHeavy,
				false, false, false, false, false, 0, 34, 16, 7, 1.4f, 6, 15, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -64.0f, 46.0f, 32.0f}, HitLevel::Overhead, HitKind::Strike, 13, 17, 128, 9, 25, 18, 12, 4, 4.2f, -1.8f, true}
			};
			Def.crouchLight = MoveDef{
				"frontKick", "Chute frontal", FighterState::KickAttack, AnimId::CrouchLight,
				false, true, false, false, false, 0, 29, 11, 5, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -118.0f, 90.0f, 46.0f}, HitLevel::Low, HitKind::Strike, 10, 14, 82, 4, 17, 12, 8, 3, 3.3f, 0.0f, false}
			};
			Def.crouchHeavy = MoveDef{
				"rasteiraUrso", "Rasteira do Urso", FighterState::KickAttack, AnimId::CrouchHeavy,
				false, true, false, false, false, 0, 36, 14, 6, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -82.0f, 104.0f, 36.0f}, HitLevel::Low, HitKind::Strike, 14, 18, 118, 7, 24, 15, 10, 3, 3.4f, -3.4f, true}
			};
			Def.airLight = MoveDef{
				"jumpLightNeutral", "Joelhada aerea", FighterState::LightAttack, AnimId::AirLight,
				true, false, false, false, false, 0, 24, 9, 4, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{0.0f, -44.0f, 34.0f, 24.0f}, HitLevel::Overhead, HitKind::Strike, 6, 14, 70, 3, 16, 11, 7, 2, 2.2f, 0.0f, false}
			};
			Def.airHeavy = MoveDef{
				"jumpHeavyNeutral", "Prancha aerea", FighterState::HeavyAttack, AnimId::AirHeavy,
				true, false, false, false, false, 0, 30, 14, 6, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{-4.0f, -52.0f, 46.0f, 30.0f}, HitLevel::Overhead, HitKind::Strike, 9, 18, 110, 6, 20, 14, 10, 3, 3.6f, 0.0f, false}
			};
			Def.special1 = MoveDef{
				"muralhaNorte", "Muralha Norte", FighterState::SpecialAttack, AnimId::Special1,
				false, false, false, true, false, 0, 32, 16, 8, 2.8f, 6, 16, -1, -1, 3, 2, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -68.0f, 52.0f, 36.0f}, HitLevel::Mid, HitKind::Strike, 10, 17, 134, 12, 24, 16, 12, 5, 4.8f, -2.5f, true, false, false, 18}
			};
			Def.special2 = MoveDef{
				"ganchoUrso", "Gancho do Urso", FighterState::SpecialAttack, AnimId::Special2,
				false, false, false, true, false, 0, 34, 16, 8, 2.2f, 5, 14, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -78.0f, 44.0f, 46.0f}, HitLevel::Mid, HitKind::Strike, 8, 15, 142, 10, 26, 15, 12, 5, 3.8f, -4.2f, true}
			};
			Def.superMove = MoveDef{
				"abracoGlacial", "Abraco Glacial", FighterState::SpecialAttack, AnimId::Super,
				false, false, false, true, true, 50, 44, 0, 0, 3.4f, 6, 20, -1, -1, 1, 2, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{6.0f, -76.0f, 64.0f, 48.0f}, HitLevel::Mid, HitKind::Strike, 10, 22, 265, 24, 34, 20, 16, 8, 6.2f, -4.5f, true, false, false, 24}
			};
			return Def;
		}

		FighterDef BuildRafaMare()
		{
			FighterDef Def{};
			Def.id = FighterId::RafaMare;
			Def.canonicalId = "rafa-mare";
			Def.name = "RAFA MARE";
			Def.archetype = "AGILE / RUSHDOWN";
			Def.ability1 = "Mao da Mare";
			Def.ability2 = "Chute da Ressaca";
			Def.ability3 = "Eco Tatuado";
			Def.assetFolder = "RafaMare";
			Def.portraitFile = "rafa-mare.png";
			Def.voiceFolder = "rafa";
			Def.specialSfxFile = "agua.ogg";
			Def.superSfxFile = "super-chute-da-ressaca.ogg";
			Def.spriteFrameSize = 256;
			Def.visualSolePad = 26;
			Def.bodyColor = 0x127da8;
			Def.accentColor = 0x29d9ff;
			Def.stats = FighterStats{
				1000, 4.26f, 3.51f, 15.6f, 3.8f, 3.34f, 0.86f, 0.9f,
				Rect{-14.0f, -54.0f, 28.0f, 54.0f}
			};
			Def.standingHurtboxes = kStandardStandingHurtboxes;
			Def.crouchingHurtboxes = kStandardCrouchingHurtboxes;
			Def.universalGrab = MakeUniversalGrab();

			Def.projectiles[0] = ProjectileDef{
				"onda-curta", 28.0f, -52.0f, 4.1f, 52, 0, false,
				HitboxDef{Rect{-15.0f, -10.0f, 30.0f, 20.0f}, HitLevel::Mid, HitKind::Projectile, 1, 52, 112, 8, 20, 15, 8, 4, 4.0f, -2.0f, false}
			};
			Def.projectileCount = 1;

			Def.standingLight = MoveDef{
				"lightPunch", "Soco rapido", FighterState::LightAttack, AnimId::StandingLight,
				false, false, false, false, false, 0, 15, 8, 3, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -54.0f, 29.0f, 15.0f}, HitLevel::High, HitKind::Strike, 4, 6, 45, 0, 11, 8, 5, 1, 1.6f, 0.0f, false}
			};
			Def.standingHeavy = MoveDef{
				"heavyPunch", "Soco forte", FighterState::HeavyAttack, AnimId::StandingHeavy,
				false, false, false, false, false, 0, 29, 12, 5, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{7.0f, -58.0f, 40.0f, 22.0f}, HitLevel::Mid, HitKind::Strike, 10, 13, 88, 5, 18, 13, 8, 2, 3.3f, -1.0f, false}
			};
			Def.forwardLight = MoveDef{
				"avancoMare", "Palma avancando", FighterState::LightAttack, AnimId::ForwardLight,
				false, false, false, false, false, 0, 19, 9, 4, 3.0f, 3, 8, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -52.0f, 32.0f, 18.0f}, HitLevel::Mid, HitKind::Strike, 6, 9, 56, 2, 13, 9, 6, 2, 2.2f, 0.0f, false}
			};
			Def.forwardHeavy = MoveDef{
				"chuteCorrente", "Chute corrente", FighterState::HeavyAttack, AnimId::ForwardHeavy,
				false, false, false, false, false, 0, 29, 13, 6, 2.2f, 4, 12, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -52.0f, 42.0f, 22.0f}, HitLevel::Mid, HitKind::Strike, 10, 14, 96, 6, 19, 14, 9, 3, 3.8f, -1.5f, false}
			};
			Def.crouchLight = MoveDef{
				"lowKick", "Chute baixo", FighterState::KickAttack, AnimId::CrouchLight,
				false, true, false, false, false, 0, 23, 9, 4, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -21.0f, 42.0f, 15.0f}, HitLevel::Low, HitKind::Strike, 7, 10, 58, 2, 14, 10, 6, 1, 2.1f, 0.0f, false}
			};
			Def.crouchHeavy = MoveDef{
				"rasteiraMare", "Rasteira da mare", FighterState::KickAttack, AnimId::CrouchHeavy,
				false, true, false, false, false, 0, 29, 12, 5, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -18.0f, 48.0f, 16.0f}, HitLevel::Low, HitKind::Strike, 9, 13, 84, 5, 20, 14, 8, 2, 3.0f, -3.2f, true}
			};
			Def.airLight = MoveDef{
				"jumpLightNeutral", "Chute voador leve", FighterState::KickAttack, AnimId::AirLight,
				true, false, false, false, false, 0, 22, 8, 3, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{4.0f, -40.0f, 32.0f, 20.0f}, HitLevel::Overhead, HitKind::Strike, 5, 12, 54, 2, 14, 10, 6, 2, 1.8f, 0.0f, false}
			};
			Def.airHeavy = MoveDef{
				"jumpHeavyNeutral", "Voadora da mare", FighterState::HeavyAttack, AnimId::AirHeavy,
				true, false, false, false, false, 0, 26, 12, 5, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{4.0f, -44.0f, 40.0f, 24.0f}, HitLevel::Overhead, HitKind::Strike, 7, 15, 86, 4, 18, 13, 8, 3, 3.0f, 0.0f, false}
			};
			Def.special1 = MoveDef{
				"maoDaMare", "Mao da Mare", FighterState::SpecialAttack, AnimId::Special1,
				false, false, false, true, false, 0, 28, 14, 6, 0.0f, 0, 0, 10, 0, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{12.0f, -58.0f, 38.0f, 24.0f}, HitLevel::Mid, HitKind::Strike, 8, 12, 92, 8, 18, 13, 8, 4, 3.2f, -1.2f, false}
			};
			Def.special2 = MoveDef{
				"ecoTatuado", "Eco Tatuado", FighterState::SpecialAttack, AnimId::Special2,
				false, false, false, true, false, 0, 26, 14, 6, 4.2f, 4, 14, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -56.0f, 40.0f, 24.0f}, HitLevel::Mid, HitKind::Strike, 7, 14, 108, 9, 20, 14, 9, 4, 3.8f, -2.0f, true}
			};
			Def.superMove = MoveDef{
				"chuteDaRessaca", "Chute da Ressaca", FighterState::SpecialAttack, AnimId::Super,
				false, false, false, true, true, 50, 40, 0, 0, 4.6f, 5, 22, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -68.0f, 56.0f, 38.0f}, HitLevel::Mid, HitKind::Strike, 8, 22, 235, 20, 30, 18, 14, 7, 5.4f, -4.0f, true}
			};
			return Def;
		}

		FighterDef BuildNoirReflexo()
		{
			FighterDef Def{};
			Def.id = FighterId::NoirReflexo;
			Def.canonicalId = "noir-reflexo";
			Def.name = "NOIR REFLEXO";
			Def.archetype = "COUNTER / ZONER";
			Def.ability1 = "Reflexo Negro";
			Def.ability2 = "Quebra-Luz";
			Def.ability3 = "Impacto Solar";
			Def.assetFolder = "NoirReflexo";
			Def.portraitFile = "noir-reflexo.png";
			Def.voiceFolder = "noir";
			Def.specialSfxFile = "espelho.ogg";
			Def.superSfxFile = "super-impacto-solar.ogg";
			Def.spriteFrameSize = 256;
			Def.visualSolePad = 26;
			Def.bodyColor = 0x11151b;
			Def.accentColor = 0x2d9cff;
			Def.stats = FighterStats{
				980, 3.74f, 3.57f, 14.4f, 3.28f, 3.11f, 0.86f, 0.98f,
				Rect{-14.0f, -54.0f, 28.0f, 54.0f}
			};
			Def.standingHurtboxes = kStandardStandingHurtboxes;
			Def.crouchingHurtboxes = kStandardCrouchingHurtboxes;
			Def.universalGrab = MakeUniversalGrab();

			Def.projectiles[0] = ProjectileDef{
				"quebraLuz", 36.0f, -50.0f, 5.8f, 96, 0, false,
				HitboxDef{Rect{-16.0f, -10.0f, 32.0f, 20.0f}, HitLevel::Mid, HitKind::Projectile, 1, 96, 78, 6, 18, 12, 7, 4, 3.0f, 0.0f, false}
			};
			Def.projectileCount = 1;

			Def.standingLight = MoveDef{
				"lightPunch", "Corte de sombra", FighterState::LightAttack, AnimId::StandingLight,
				false, false, false, false, false, 0, 15, 8, 3, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -54.0f, 30.0f, 16.0f}, HitLevel::High, HitKind::Strike, 4, 6, 46, 0, 11, 8, 5, 2, 1.7f, 0.0f, false}
			};
			Def.standingHeavy = MoveDef{
				"heavyPunch", "Estocada prismatica", FighterState::HeavyAttack, AnimId::StandingHeavy,
				false, false, false, false, false, 0, 27, 11, 4, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -58.0f, 42.0f, 22.0f}, HitLevel::Mid, HitKind::Strike, 9, 13, 86, 4, 17, 11, 7, 3, 3.1f, -1.2f, false}
			};
			Def.forwardLight = MoveDef{
				"passoEspelho", "Passo espelho", FighterState::LightAttack, AnimId::ForwardLight,
				false, false, false, false, false, 0, 18, 8, 3, 2.4f, 3, 8, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -52.0f, 32.0f, 18.0f}, HitLevel::Mid, HitKind::Strike, 6, 9, 54, 1, 11, 8, 5, 2, 1.7f, 0.0f, false}
			};
			Def.forwardHeavy = MoveDef{
				"arcoRefletido", "Arco refletido", FighterState::HeavyAttack, AnimId::ForwardHeavy,
				false, false, false, false, false, 0, 28, 11, 4, 1.8f, 4, 11, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -56.0f, 44.0f, 24.0f}, HitLevel::Overhead, HitKind::Strike, 10, 14, 92, 4, 17, 11, 7, 3, 3.1f, -1.2f, false}
			};
			Def.crouchLight = MoveDef{
				"lowKick", "Chute baixo", FighterState::KickAttack, AnimId::CrouchLight,
				false, true, false, false, false, 0, 20, 8, 3, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -20.0f, 40.0f, 14.0f}, HitLevel::Low, HitKind::Strike, 6, 9, 50, 1, 11, 8, 5, 2, 1.7f, 0.0f, false}
			};
			Def.crouchHeavy = MoveDef{
				"rasteiraSombra", "Rasteira de sombra", FighterState::KickAttack, AnimId::CrouchHeavy,
				false, true, false, false, false, 0, 28, 11, 4, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -18.0f, 48.0f, 16.0f}, HitLevel::Low, HitKind::Strike, 9, 13, 82, 4, 17, 11, 7, 3, 3.1f, -3.2f, true}
			};
			Def.airLight = MoveDef{
				"jumpLightNeutral", "Corte aereo", FighterState::LightAttack, AnimId::AirLight,
				true, false, false, false, false, 0, 20, 8, 3, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{6.0f, -42.0f, 32.0f, 18.0f}, HitLevel::Overhead, HitKind::Strike, 5, 11, 52, 1, 11, 8, 5, 2, 1.7f, 0.0f, false}
			};
			Def.airHeavy = MoveDef{
				"jumpHeavyNeutral", "Queda lunar", FighterState::HeavyAttack, AnimId::AirHeavy,
				true, false, false, false, false, 0, 25, 11, 4, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{4.0f, -46.0f, 40.0f, 24.0f}, HitLevel::Overhead, HitKind::Strike, 7, 14, 84, 4, 17, 11, 7, 3, 3.1f, -1.2f, false}
			};
			Def.special1 = MoveDef{
				"quebraLuz", "Quebra-Luz", FighterState::SpecialAttack, AnimId::Special1,
				false, false, false, true, false, 0, 28, 12, 5, 0.0f, 0, 0, 9, 0, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{12.0f, -54.0f, 34.0f, 22.0f}, HitLevel::Mid, HitKind::Strike, 8, 11, 74, 5, 16, 11, 7, 4, 2.8f, 0.0f, false}
			};
			Def.special2 = MoveDef{
				"reflexoNegro", "Reflexo Negro", FighterState::SpecialAttack, AnimId::Special2,
				false, false, false, true, false, 0, 28, 14, 6, 0.0f, 0, 0, -1, -1, -1, 0, 3, 14, 115, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -60.0f, 44.0f, 28.0f}, HitLevel::Mid, HitKind::Strike, 10, 16, 110, 8, 20, 13, 9, 5, 3.8f, -2.0f, true}
			};
			Def.superMove = MoveDef{
				"impactoSolar", "Impacto Solar", FighterState::SpecialAttack, AnimId::Super,
				false, false, false, true, true, 50, 42, 0, 0, 3.8f, 5, 20, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -72.0f, 58.0f, 44.0f}, HitLevel::Mid, HitKind::Strike, 9, 22, 240, 22, 32, 18, 15, 7, 5.5f, -4.0f, true}
			};
			return Def;
		}

		FighterDef BuildAstroRiso()
		{
			FighterDef Def{};
			Def.id = FighterId::AstroRiso;
			Def.canonicalId = "astro-riso";
			Def.name = "ASTRO RISO";
			Def.archetype = "SPEED / MIX-UP";
			Def.ability1 = "Sorriso Relampago";
			Def.ability2 = "Rajada Neon";
			Def.ability3 = "Astro Giro";
			Def.assetFolder = "AstroRiso";
			Def.portraitFile = "astro-riso.png";
			Def.voiceFolder = "astro";
			Def.specialSfxFile = "neon.ogg";
			Def.superSfxFile = "super-astro-giro.ogg";
			Def.spriteFrameSize = 256;
			Def.visualSolePad = 26;
			Def.bodyColor = 0x356fd1;
			Def.accentColor = 0x59ecff;
			Def.stats = FighterStats{
				930, 4.72f, 3.85f, 15.4f, 4.2f, 3.62f, 0.88f, 0.84f,
				Rect{-13.0f, -52.0f, 26.0f, 52.0f}
			};
			Def.standingHurtboxes = kStandardStandingHurtboxes;
			Def.crouchingHurtboxes = kStandardCrouchingHurtboxes;
			Def.universalGrab = MakeUniversalGrab();

			Def.standingLight = MoveDef{
				"lightPunch", "Soco relampago", FighterState::LightAttack, AnimId::StandingLight,
				false, false, false, false, false, 0, 12, 7, 3, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -53.0f, 30.0f, 15.0f}, HitLevel::High, HitKind::Strike, 3, 5, 38, 0, 10, 7, 4, 1, 1.4f, 0.0f, false}
			};
			Def.standingHeavy = MoveDef{
				"heavyPunch", "Soco estrela", FighterState::HeavyAttack, AnimId::StandingHeavy,
				false, false, false, false, false, 0, 24, 10, 4, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -58.0f, 39.0f, 21.0f}, HitLevel::Mid, HitKind::Strike, 8, 11, 74, 4, 16, 11, 7, 2, 2.8f, -1.0f, false}
			};
			Def.forwardLight = MoveDef{
				"passoEstelar", "Passo estelar", FighterState::LightAttack, AnimId::ForwardLight,
				false, false, false, false, false, 0, 16, 8, 3, 3.1f, 2, 6, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{11.0f, -52.0f, 32.0f, 17.0f}, HitLevel::High, HitKind::Strike, 6, 8, 48, 2, 11, 8, 4, 1, 1.9f, 0.0f, false}
			};
			Def.forwardHeavy = MoveDef{
				"highKick", "Chute crescente", FighterState::KickAttack, AnimId::ForwardHeavy,
				false, false, false, false, false, 0, 25, 11, 4, 2.6f, 3, 10, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{9.0f, -62.0f, 40.0f, 26.0f}, HitLevel::Overhead, HitKind::Strike, 8, 12, 82, 4, 17, 12, 7, 3, 3.0f, -1.6f, false}
			};
			Def.crouchLight = MoveDef{
				"lowKick", "Chute baixo", FighterState::KickAttack, AnimId::CrouchLight,
				false, true, false, false, false, 0, 18, 8, 3, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -20.0f, 41.0f, 14.0f}, HitLevel::Low, HitKind::Strike, 5, 8, 45, 2, 12, 9, 5, 1, 1.8f, 0.0f, false}
			};
			Def.crouchHeavy = MoveDef{
				"rasteira", "Rasteira orbital", FighterState::KickAttack, AnimId::CrouchHeavy,
				false, true, false, false, false, 0, 26, 11, 4, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{7.0f, -16.0f, 47.0f, 16.0f}, HitLevel::Low, HitKind::Strike, 9, 13, 78, 4, 20, 13, 7, 2, 2.8f, -3.0f, true}
			};
			Def.airLight = MoveDef{
				"jumpLightNeutral", "Estrela aerea", FighterState::LightAttack, AnimId::AirLight,
				true, false, false, false, false, 0, 18, 7, 3, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{6.0f, -42.0f, 30.0f, 18.0f}, HitLevel::Overhead, HitKind::Strike, 4, 10, 46, 1, 12, 9, 5, 2, 1.6f, 0.0f, false}
			};
			Def.airHeavy = MoveDef{
				"jumpHeavyNeutral", "Cometa aereo", FighterState::HeavyAttack, AnimId::AirHeavy,
				true, false, false, false, false, 0, 24, 10, 4, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{4.0f, -46.0f, 38.0f, 22.0f}, HitLevel::Overhead, HitKind::Strike, 6, 13, 76, 4, 16, 12, 7, 3, 2.8f, 0.0f, false}
			};
			Def.special1 = MoveDef{
				"sorrisoRelampago", "Sorriso Relampago", FighterState::SpecialAttack, AnimId::Special1,
				false, false, false, true, false, 0, 24, 12, 5, 5.2f, 3, 13, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -56.0f, 42.0f, 24.0f}, HitLevel::Mid, HitKind::Strike, 6, 13, 94, 7, 18, 12, 7, 4, 3.4f, -1.6f, false}
			};
			Def.special2 = MoveDef{
				"rajadaNeon", "Rajada Neon", FighterState::SpecialAttack, AnimId::Special2,
				false, false, false, true, false, 0, 26, 13, 5, 3.8f, 4, 14, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -60.0f, 44.0f, 28.0f}, HitLevel::Mid, HitKind::Strike, 7, 15, 104, 8, 20, 13, 8, 4, 3.6f, -2.2f, true}
			};
			Def.superMove = MoveDef{
				"astroGiro", "Astro Giro", FighterState::SpecialAttack, AnimId::Super,
				false, false, false, true, true, 50, 38, 0, 0, 5.4f, 4, 22, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{6.0f, -70.0f, 56.0f, 42.0f}, HitLevel::Mid, HitKind::Strike, 7, 22, 225, 18, 30, 18, 13, 7, 5.2f, -3.8f, true}
			};
			return Def;
		}

		FighterDef BuildDanteSinal()
		{
			FighterDef Def{};
			Def.id = FighterId::DanteSinal;
			Def.canonicalId = "dante-sinal";
			Def.name = "DANTE SINAL";
			Def.archetype = "TECHNICAL / ZONER";
			Def.ability1 = "Ponto Final";
			Def.ability2 = "Bomba de Fumaca";
			Def.ability3 = "Chave Binaria";
			Def.assetFolder = "DanteSinal";
			Def.portraitFile = "dante-sinal.png";
			Def.voiceFolder = "dante";
			Def.specialSfxFile = "fumaca.ogg";
			Def.superSfxFile = "super-ponto-final.ogg";
			Def.spriteFrameSize = 256;
			Def.visualSolePad = 26;
			Def.bodyColor = 0x405e78;
			Def.accentColor = 0x70ffbf;
			Def.stats = FighterStats{
				960, 4.03f, 3.74f, 14.8f, 3.51f, 3.28f, 0.88f, 0.95f,
				Rect{-14.0f, -54.0f, 28.0f, 54.0f}
			};
			Def.standingHurtboxes = kStandardStandingHurtboxes;
			Def.crouchingHurtboxes = kStandardCrouchingHurtboxes;
			Def.universalGrab = MakeUniversalGrab();

			Def.projectiles[0] = ProjectileDef{
				"chave-binaria-hazard", 45.0f, -54.0f, 5.0f, 120, 0, false,
				HitboxDef{Rect{-14.0f, -8.0f, 28.0f, 16.0f}, HitLevel::Mid, HitKind::Projectile, 1, 120, 65, 4, 18, 12, 7, 10, 2.5f, 0.0f, false, true, true, 0}
			};
			Def.projectiles[1] = ProjectileDef{
				"ponto-final-hazard", 0.0f, 0.0f, 0.0f, 47, 42, true,
				HitboxDef{Rect{-24.0f, -80.0f, 48.0f, 80.0f}, HitLevel::Mid, HitKind::Projectile, 42, 47, 240, 25, 35, 22, 20, 20, 6.0f, -8.0f, true}
			};
			Def.projectileCount = 2;

			Def.standingLight = MoveDef{
				"lightPunch", "Soco rapido", FighterState::LightAttack, AnimId::StandingLight,
				false, false, false, false, false, 0, 16, 8, 3, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -54.0f, 29.0f, 15.0f}, HitLevel::High, HitKind::Strike, 5, 7, 45, 0, 11, 8, 5, 1, 1.6f, 0.0f, false}
			};
			Def.standingHeavy = MoveDef{
				"heavyPunch", "Soco forte", FighterState::HeavyAttack, AnimId::StandingHeavy,
				false, false, false, false, false, 0, 30, 12, 5, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -58.0f, 42.0f, 22.0f}, HitLevel::Mid, HitKind::Strike, 10, 14, 88, 5, 18, 12, 8, 2, 3.2f, -1.2f, false}
			};
			Def.forwardLight = MoveDef{
				"passoTatico", "Passo tatico", FighterState::LightAttack, AnimId::ForwardLight,
				false, false, false, false, false, 0, 19, 9, 4, 2.5f, 3, 8, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -52.0f, 32.0f, 18.0f}, HitLevel::Mid, HitKind::Strike, 6, 9, 54, 2, 13, 9, 6, 2, 2.0f, 0.0f, false}
			};
			Def.forwardHeavy = MoveDef{
				"corteSinal", "Corte de sinal", FighterState::HeavyAttack, AnimId::ForwardHeavy,
				false, false, false, false, false, 0, 29, 13, 5, 2.0f, 4, 11, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -58.0f, 44.0f, 24.0f}, HitLevel::Overhead, HitKind::Strike, 10, 14, 94, 5, 19, 13, 8, 3, 3.4f, -1.5f, false}
			};
			Def.crouchLight = MoveDef{
				"lowKick", "Chute baixo", FighterState::KickAttack, AnimId::CrouchLight,
				false, true, false, false, false, 0, 21, 8, 3, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -20.0f, 41.0f, 14.0f}, HitLevel::Low, HitKind::Strike, 6, 9, 52, 2, 13, 9, 5, 1, 1.9f, 0.0f, false}
			};
			Def.crouchHeavy = MoveDef{
				"rasteiraTatica", "Rasteira tatica", FighterState::KickAttack, AnimId::CrouchHeavy,
				false, true, false, false, false, 0, 29, 12, 5, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -18.0f, 48.0f, 16.0f}, HitLevel::Low, HitKind::Strike, 9, 13, 84, 5, 20, 13, 8, 2, 3.0f, -3.2f, true}
			};
			Def.airLight = MoveDef{
				"jumpLightNeutral", "Pulso aereo", FighterState::LightAttack, AnimId::AirLight,
				true, false, false, false, false, 0, 20, 8, 3, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{6.0f, -42.0f, 32.0f, 18.0f}, HitLevel::Overhead, HitKind::Strike, 5, 11, 52, 2, 13, 9, 5, 2, 1.8f, 0.0f, false}
			};
			Def.airHeavy = MoveDef{
				"jumpHeavyNeutral", "Vetor descendente", FighterState::HeavyAttack, AnimId::AirHeavy,
				true, false, false, false, false, 0, 26, 11, 4, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{4.0f, -46.0f, 40.0f, 24.0f}, HitLevel::Overhead, HitKind::Strike, 7, 14, 84, 4, 17, 12, 7, 3, 3.0f, 0.0f, false}
			};
			Def.special1 = MoveDef{
				"chaveBinaria", "Chave Binaria", FighterState::SpecialAttack, AnimId::Special1,
				false, false, false, true, false, 0, 28, 12, 5, 0.0f, 0, 0, 10, 0, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{12.0f, -54.0f, 34.0f, 20.0f}, HitLevel::Mid, HitKind::Strike, 8, 11, 68, 4, 16, 11, 7, 3, 2.5f, 0.0f, false}
			};
			Def.special2 = MoveDef{
				"bombaFumaca", "Bomba de Fumaca", FighterState::SpecialAttack, AnimId::Special2,
				false, false, false, true, false, 0, 26, 12, 5, -2.4f, 3, 10, -1, -1, -1, 0, -1, 0, 0, 6, 120, 0.70f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -58.0f, 42.0f, 28.0f}, HitLevel::Mid, HitKind::Strike, 6, 12, 76, 5, 18, 12, 7, 4, 3.2f, -1.5f, false}
			};
			Def.superMove = MoveDef{
				"pontoFinal", "Ponto Final", FighterState::SpecialAttack, AnimId::Super,
				false, false, false, true, true, 50, 42, 0, 0, 0.0f, 0, 0, 12, 1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -72.0f, 54.0f, 42.0f}, HitLevel::Mid, HitKind::Strike, 10, 20, 240, 25, 35, 22, 16, 8, 6.0f, -4.0f, true}
			};
			return Def;
		}

		FighterDef BuildLeoVioleta()
		{
			FighterDef Def{};
			Def.id = FighterId::LeoVioleta;
			Def.canonicalId = "leo-violeta";
			Def.name = "LEO VIOLETA";
			Def.archetype = "PRESSURE / BRAWLER";
			Def.ability1 = "Olhar Frio";
			Def.ability2 = "Impacto Sombrio";
			Def.ability3 = "Pressao Violeta";
			Def.assetFolder = "LeoVioleta";
			Def.portraitFile = "leo-violeta.png";
			Def.voiceFolder = "leo";
			Def.specialSfxFile = "sombra.ogg";
			Def.superSfxFile = "super-pressao-violeta.ogg";
			Def.spriteFrameSize = 256;
			Def.visualSolePad = 26;
			Def.bodyColor = 0x17151c;
			Def.accentColor = 0x8c4fd6;
			Def.stats = FighterStats{
				1050, 4.08f, 3.22f, 14.4f, 3.39f, 2.93f, 0.88f, 1.05f,
				Rect{-15.0f, -56.0f, 30.0f, 56.0f}
			};
			Def.standingHurtboxes = kStandardStandingHurtboxes;
			Def.crouchingHurtboxes = kStandardCrouchingHurtboxes;
			Def.universalGrab = MakeUniversalGrab();

			Def.projectiles[0] = ProjectileDef{
				"olharFrio", 48.0f, -78.0f, 5.2f, 42, 0, false,
				HitboxDef{Rect{-16.0f, -12.0f, 32.0f, 24.0f}, HitLevel::Mid, HitKind::Projectile, 1, 42, 72, 6, 18, 12, 7, 4, 2.8f, 0.0f, false}
			};
			Def.projectileCount = 1;

			Def.standingLight = MoveDef{
				"lightPunch", "Jab direto", FighterState::LightAttack, AnimId::StandingLight,
				false, false, false, false, false, 0, 15, 8, 3, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -54.0f, 30.0f, 16.0f}, HitLevel::High, HitKind::Strike, 4, 7, 48, 0, 12, 8, 5, 2, 1.8f, 0.0f, false}
			};
			Def.standingHeavy = MoveDef{
				"heavyPunch", "Cruzado violeta", FighterState::HeavyAttack, AnimId::StandingHeavy,
				false, false, false, false, false, 0, 28, 11, 4, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -58.0f, 42.0f, 22.0f}, HitLevel::Mid, HitKind::Strike, 9, 13, 92, 4, 18, 12, 8, 3, 3.4f, -1.4f, false}
			};
			Def.forwardLight = MoveDef{
				"avancoCurto", "Avanco curto", FighterState::LightAttack, AnimId::ForwardLight,
				false, false, false, false, false, 0, 18, 8, 3, 2.6f, 3, 8, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -52.0f, 34.0f, 18.0f}, HitLevel::Mid, HitKind::Strike, 6, 9, 58, 0, 12, 8, 5, 2, 1.8f, 0.0f, false}
			};
			Def.forwardHeavy = MoveDef{
				"ombroPesado", "Ombro pesado", FighterState::HeavyAttack, AnimId::ForwardHeavy,
				false, false, false, false, false, 0, 29, 11, 4, 2.8f, 4, 12, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -56.0f, 44.0f, 24.0f}, HitLevel::Mid, HitKind::Strike, 10, 14, 98, 4, 18, 12, 8, 3, 3.4f, -1.4f, false}
			};
			Def.crouchLight = MoveDef{
				"lowKick", "Chute baixo", FighterState::KickAttack, AnimId::CrouchLight,
				false, true, false, false, false, 0, 20, 8, 3, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -20.0f, 40.0f, 14.0f}, HitLevel::Low, HitKind::Strike, 6, 9, 54, 1, 12, 8, 5, 2, 1.8f, 0.0f, false}
			};
			Def.crouchHeavy = MoveDef{
				"rasteiraSombria", "Rasteira sombria", FighterState::KickAttack, AnimId::CrouchHeavy,
				false, true, false, false, false, 0, 28, 11, 4, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -18.0f, 48.0f, 16.0f}, HitLevel::Low, HitKind::Strike, 9, 13, 86, 4, 18, 12, 8, 3, 3.4f, -3.2f, true}
			};
			Def.airLight = MoveDef{
				"jumpLightNeutral", "Soco aereo", FighterState::LightAttack, AnimId::AirLight,
				true, false, false, false, false, 0, 20, 8, 3, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{6.0f, -42.0f, 32.0f, 18.0f}, HitLevel::Overhead, HitKind::Strike, 5, 11, 54, 0, 12, 8, 5, 2, 1.8f, 0.0f, false}
			};
			Def.airHeavy = MoveDef{
				"jumpHeavyNeutral", "Marreta aerea", FighterState::HeavyAttack, AnimId::AirHeavy,
				true, false, false, false, false, 0, 25, 11, 4, 0.0f, 0, 0, -1, -1, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{4.0f, -46.0f, 40.0f, 24.0f}, HitLevel::Overhead, HitKind::Strike, 7, 14, 88, 4, 18, 12, 8, 3, 3.4f, -1.4f, false}
			};
			Def.special1 = MoveDef{
				"olharFrio", "Olhar Frio", FighterState::SpecialAttack, AnimId::Special1,
				false, false, false, true, false, 0, 28, 12, 5, 0.0f, 0, 0, 9, 0, -1, 0, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{12.0f, -56.0f, 36.0f, 22.0f}, HitLevel::Mid, HitKind::Strike, 8, 11, 76, 5, 17, 11, 7, 4, 2.8f, 0.0f, false}
			};
			Def.special2 = MoveDef{
				"impactoSombrio", "Impacto Sombrio", FighterState::SpecialAttack, AnimId::Special2,
				false, false, false, true, false, 0, 30, 14, 6, 4.0f, 4, 15, -1, -1, 3, 1, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{10.0f, -60.0f, 46.0f, 28.0f}, HitLevel::Mid, HitKind::Strike, 8, 15, 116, 9, 21, 14, 10, 5, 4.0f, -2.2f, true}
			};
			Def.superMove = MoveDef{
				"pressaoVioleta", "Pressao Violeta", FighterState::SpecialAttack, AnimId::Super,
				false, false, false, true, true, 50, 42, 0, 0, 4.5f, 5, 22, -1, -1, 1, 1, -1, 0, 0, -1, 0, 1.0f, 14, 46, 50,
				HitboxDef{Rect{8.0f, -72.0f, 58.0f, 44.0f}, HitLevel::Mid, HitKind::Strike, 8, 22, 245, 22, 32, 19, 15, 8, 5.6f, -4.2f, true}
			};
			return Def;
		}

		constexpr std::array<StageDef, kStageCount> kStages = {{
			StageDef{
				StageId::CaisDaCidade,
				"cais-da-cidade",
				"CAIS DA CIDADE",
				"NOITE | ZONA PORTUARIA",
				"CaisDaCidade",
				"cais-da-cidade.ogg",
				"O CAIS COBRA SUA ALMA"
			},
			StageDef{
				StageId::CozinhaMacabra,
				"cozinha-macabra",
				"COZINHA MACABRA",
				"MEIA-NOITE | O JANTAR ESTA VIVO",
				"CozinhaMacabra",
				"cozinha-macabra.ogg",
				"O JANTAR ESTA SERVIDO"
			},
			StageDef{
				StageId::Sitio,
				"sitio",
				"SITIO",
				"DIA | VIDA NO TERREIRO",
				"Sitio",
				"sitio.ogg",
				"A COLHEITA ESTA FEITA"
			},
		}};
	}

	const FighterDef& GetGutoBarba()
	{
		static const FighterDef Def = [] { auto d=BuildGutoBarba(); EnrichCanonicalDefinition(d); return d; }();
		return Def;
	}

	const FighterDef& GetRafaMare()
	{
		static const FighterDef Def = [] { auto d=BuildRafaMare(); EnrichCanonicalDefinition(d); return d; }();
		return Def;
	}

	const FighterDef& GetNoirReflexo()
	{
		static const FighterDef Def = [] { auto d=BuildNoirReflexo(); EnrichCanonicalDefinition(d); return d; }();
		return Def;
	}

	const FighterDef& GetAstroRiso()
	{
		static const FighterDef Def = [] { auto d=BuildAstroRiso(); EnrichCanonicalDefinition(d); return d; }();
		return Def;
	}

	const FighterDef& GetDanteSinal()
	{
		static const FighterDef Def = [] { auto d=BuildDanteSinal(); EnrichCanonicalDefinition(d); return d; }();
		return Def;
	}

	const FighterDef& GetLeoVioleta()
	{
		static const FighterDef Def = [] { auto d=BuildLeoVioleta(); EnrichCanonicalDefinition(d); return d; }();
		return Def;
	}

	const FighterDef& GetFighterDef(FighterId Id)
	{
		switch (Id)
		{
		case FighterId::GutoBarba: return GetGutoBarba();
		case FighterId::RafaMare: return GetRafaMare();
		case FighterId::NoirReflexo: return GetNoirReflexo();
		case FighterId::AstroRiso: return GetAstroRiso();
		case FighterId::DanteSinal: return GetDanteSinal();
		case FighterId::LeoVioleta: return GetLeoVioleta();
		default: return GetGutoBarba();
		}
	}

	const FighterDef* FindFighterByCanonicalId(const char* CanonicalId)
	{
		if (!CanonicalId) return nullptr;
		for (int i = 0; i < kFighterCount; ++i)
		{
			const FighterDef& Def = GetFighterDef(static_cast<FighterId>(i));
			if (std::strcmp(Def.canonicalId, CanonicalId) == 0)
			{
				return &Def;
			}
		}
		return nullptr;
	}

	const StageDef& GetStageDef(StageId Id)
	{
		const int Index = static_cast<int>(Id);
		if (Index >= 0 && Index < kStageCount)
		{
			return kStages[static_cast<size_t>(Index)];
		}
		return kStages[0];
	}

	const StageDef* FindStageByCanonicalId(const char* CanonicalId)
	{
		if (!CanonicalId) return nullptr;
		for (const StageDef& Stage : kStages)
		{
			if (std::strcmp(Stage.canonicalId, CanonicalId) == 0)
			{
				return &Stage;
			}
		}
		return nullptr;
	}
}
