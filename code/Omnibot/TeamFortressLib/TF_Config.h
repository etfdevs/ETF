////////////////////////////////////////////////////////////////////////////////
// 
// $LastChangedBy$
// $LastChangedDate$
// $LastChangedRevision$
//
// Title: TF Config
//
////////////////////////////////////////////////////////////////////////////////

#ifndef __TF_EVENTS_H__
#define __TF_EVENTS_H__

#include "../Common/Omni-Bot_Types.h"
#include "../Common/Omni-Bot_Events.h"

// enumerations: TF_ButtonFlags
//		TF_BOT_BUTTON_GREN1 - Prime grenade 1.
//		TF_BOT_BUTTON_GREN2 - Prime grenade 2.
//		TF_BOT_BUTTON_DROPITEM - Drop carried item(flag).
//		TF_BOT_BUTTON_DROPAMMO - Drop ammo(discard).
//		TF_BOT_BUTTON_BUILDSENTRY - Build sentry
//		TF_BOT_BUTTON_BUILDDISPENSER - Build dispenser.
//		TF_BOT_BUTTON_BUILDDETPACK_5 - Build detpack, 5 seconds.
//		TF_BOT_BUTTON_BUILDDETPACK_10 - Build detpack, 10 seconds.
//		TF_BOT_BUTTON_BUILDDETPACK_20 - Build detpack, 20 seconds.
//		TF_BOT_BUTTON_BUILDDETPACK_30 - Build detpack, 30 seconds.
//		TF_BOT_BUTTON_AIMSENTRY - Aim sentry along current facing.
//		TF_BOT_BUTTON_DETSENTRY - Detonate sentry gun.
//		TF_BOT_BUTTON_DETDISPENSER - Detonate dispenser.
//		TF_BOT_BUTTON_DETPIPES - Detonate deployed pipes.
//		TF_BOT_BUTTON_CALLFORMEDIC - Calls for medic.
//		TF_BOT_BUTTON_CALLFORENGY - Calls for engineer.
//		TF_BOT_BUTTON_DISCARD - Drops spare ammo.
//		TF_BOT_BUTTON_SABOTAGE_SENTRY - Detonate enemy dispenser.
//		TF_BOT_BUTTON_SABOTAGE_DISPENSER - Detonate enemy dispenser.
typedef enum eTF_ButtonFlags
{	
	TF_BOT_BUTTON_GREN1 = BOT_BUTTON_FIRSTUSER,
	TF_BOT_BUTTON_GREN2,	
	TF_BOT_BUTTON_DROPITEM,
	TF_BOT_BUTTON_DROPAMMO,	
	TF_BOT_BUTTON_BUILDSENTRY,
	TF_BOT_BUTTON_BUILDDISPENSER,
	TF_BOT_BUTTON_BUILD_TELE_ENTRANCE,
	TF_BOT_BUTTON_BUILD_TELE_EXIT,
	TF_BOT_BUTTON_BUILDDETPACK_5,
	TF_BOT_BUTTON_BUILDDETPACK_10,
	TF_BOT_BUTTON_BUILDDETPACK_20,
	TF_BOT_BUTTON_BUILDDETPACK_30,
	TF_BOT_BUTTON_AIMSENTRY,
	TF_BOT_BUTTON_DETSENTRY,
	TF_BOT_BUTTON_DETDISPENSER,
	TF_BOT_BUTTON_DETTELE_ENTRANCE,
	TF_BOT_BUTTON_DETTELE_EXIT,
	TF_BOT_BUTTON_DETPIPES,
	TF_BOT_BUTTON_CALLFORMEDIC,
	TF_BOT_BUTTON_CALLFORENGY,
	TF_BOT_BUTTON_SABOTAGE_SENTRY,
	TF_BOT_BUTTON_SABOTAGE_DISPENSER,
	TF_BOT_BUTTON_CLOAK,
	TF_BOT_BUTTON_SILENT_CLOAK,
	TF_BOT_BUTTON_RADAR,
	TF_BOT_BUTTON_CANCELBUILD,

	// THIS MUST BE LAST
	TF_BOT_BUTTON_FIRSTUSER
} TF_ButtonFlags;

// enumerations: TF_EntityCategory
//		TF_ENT_CAT_BUILDABLE - Buildable entities
typedef enum eTF_EntityCategory
{
	TF_ENT_CAT_BUILDABLE = ENT_CAT_MAX,

	// THIS MUST BE LAST
	TF_ENT_CAT_MAX,
} TF_EntityCategory;

// enumerations: TF_EntityClass
//		TF_CLASS_SCOUT - Scout player class.
//		TF_CLASS_SNIPER - Sniper player class.
//		TF_CLASS_SOLDIER - Soldier player class.
//		TF_CLASS_DEMOMAN - Demo-man player class.
//		TF_CLASS_MEDIC - Medic player class.
//		TF_CLASS_HWGUY - HWGuy player class.
//		TF_CLASS_PYRO - Pyro player class.
//		TF_CLASS_SPY - Spy player class.
//		TF_CLASS_ENGINEER - Engineer player class.
//		TF_CLASS_CIVILIAN - Civilian player class.
//		TF_CLASSEX_SENTRY - Sentry entity.
//		TF_CLASSEX_DISPENSER - Dispenser entity.
//		TF_CLASSEX_DETPACK - Detpack entity.
//		TF_CLASSEX_GRENADE - Grenade entity.
//		TF_CLASSEX_EMP_GRENADE - EMP Grenade entity.
//		TF_CLASSEX_NAIL_GRENADE - Nail Grenade entity.
//		TF_CLASSEX_MIRV_GRENADE - Mirvlet Grenade entity.
//		TF_CLASSEX_MIRVLET_GRENADE - Mirvlet Grenade entity.
//		TF_CLASSEX_NAPALM_GRENADE - Napalm Grenade entity.
//		TF_CLASSEX_GAS_GRENADE - Gas Grenade entity.
//		TF_CLASSEX_CONC_GRENADE - Concussion Grenade entity.
//		TF_CLASSEX_CALTROP - Caltrop Grenade entity.
//		TF_CLASSEX_PIPE - Pipe Grenade entity.
//		TF_CLASSEX_ROCKET - Rocket Grenade entity.
//		TF_CLASSEX_NAPALM - Napalm Rocket entity.
//		TF_CLASSEX_SYRINGE - Syringe Gun entity.
//		TF_CLASSEX_BACKPACK_AMMO - Backpack entity.
//		TF_CLASSEX_BACKPACK_HEALTH - Backpack entity.
//		TF_CLASSEX_BACKPACK_ARMOR - Backpack entity.
//		TF_CLASSEX_BACKPACK_GRENADES - Backpack entity.
typedef enum eTF_EntityClass
{
	TF_CLASS_UNKNOWN = 0,
	TF_CLASS_NONE = 0,
	TF_CLASS_SCOUT,
	TF_CLASS_SNIPER,
	TF_CLASS_SOLDIER,
	TF_CLASS_DEMOMAN,
	TF_CLASS_MEDIC,
	TF_CLASS_HWGUY,
	TF_CLASS_PYRO,
	TF_CLASS_SPY,
	TF_CLASS_ENGINEER,
	TF_CLASS_CIVILIAN,
	TF_CLASS_MAX,
	TF_CLASS_ANY = TF_CLASS_MAX,
	// Other values to identify the "class"
	TF_CLASSEX_SENTRY,
	TF_CLASSEX_DISPENSER,
	TF_CLASSEX_TELEPORTER_ENTRANCE,
	TF_CLASSEX_TELEPORTER_EXIT,
	TF_CLASSEX_RESUPPLY,
	TF_CLASSEX_BACKPACK,
	TF_CLASSEX_BACKPACK_AMMO,
	TF_CLASSEX_BACKPACK_HEALTH,
	TF_CLASSEX_BACKPACK_ARMOR,
	TF_CLASSEX_BACKPACK_GRENADES,
	TF_CLASSEX_DETPACK,
	TF_CLASSEX_GRENADE,
	TF_CLASSEX_EMP_GRENADE,
	TF_CLASSEX_NAIL_GRENADE,
	TF_CLASSEX_MIRV_GRENADE,
	TF_CLASSEX_MIRVLET_GRENADE,
	TF_CLASSEX_NAPALM_GRENADE,
	TF_CLASSEX_GAS_GRENADE,
	TF_CLASSEX_CONC_GRENADE,
	TF_CLASSEX_CALTROP,
	TF_CLASSEX_PIPE,
	TF_CLASSEX_GLGRENADE,
	TF_CLASSEX_ROCKET,
	TF_CLASSEX_NAPALM,
	TF_CLASSEX_SYRINGE,
	TF_CLASSEX_TURRET,
	TF_CLASSEX_HUNTEDESCAPE,
	TF_CLASSEX_VEHICLE,
	TF_CLASSEX_VEHICLE_NODAMAGE,

	// THIS MUST STAY LAST
	TF_NUM_CLASSES
} TF_EntityClass;

// enumerations: TF_EntityFlags
//		TF_ENT_FLAG_SAVEME - This entity is has called for medic.
//		TF_ENT_FLAG_ARMORME - This entity has called for armor.
//		TF_ENT_FLAG_BURNING - This entity is on fire.
//		TF_ENT_FLAG_TRANQED - This entity is tranquilized.
//		TF_ENT_FLAG_INFECTED - This entity is infected.
//		TF_ENT_FLAG_GASSED - This entity is gassed(hallucinating).
//		TF_ENT_SNIPERAIMING - This entity is aiming a scoped weapon.
//		TF_ENT_ASSAULTFIRING - This entity is shooting an assault weapon.
//		TF_ENT_LEGSHOT - This entity is suffering from a leg shot.
//		TF_ENT_CALTROP - This entity is suffering from a caltrop.
//		TF_ENT_RADIOTAGGED - This entity has been radio tagged.
//		TF_ENT_CAN_SABOTAGE - This entity can be sabotaged.
//		TF_ENT_SABOTAGED - This entity has been sabotaged.
//		TF_ENT_SABOTAGING - This entity is sabotaging something.
//		TF_ENT_FLAG_BUILDING_SG - Entity is building a sentry.
//		TF_ENT_FLAG_BUILDING_DISP - Entity is building a dispenser.
//		TF_ENT_FLAG_BUILDING_DETP - Entity is building a detpack.
//		TF_ENT_FLAG_BUILDINPROGRESS - Entity is currently being built.
typedef enum eTF_EntityFlags
{
	TF_ENT_FLAG_SAVEME = ENT_FLAG_FIRST_USER,	
	TF_ENT_FLAG_ARMORME,
	TF_ENT_FLAG_BURNING,
	TF_ENT_FLAG_TRANQED,
	TF_ENT_FLAG_INFECTED,
	TF_ENT_FLAG_GASSED,
	TF_ENT_FLAG_ASSAULTFIRING,
	TF_ENT_FLAG_LEGSHOT,
	TF_ENT_FLAG_CALTROP,
	TF_ENT_FLAG_RADIOTAGGED,
	TF_ENT_FLAG_CAN_SABOTAGE,
	TF_ENT_FLAG_SABOTAGED,
	TF_ENT_FLAG_SABOTAGING,
	TF_ENT_FLAG_BUILDING_SG,
	TF_ENT_FLAG_BUILDING_DISP,
	TF_ENT_FLAG_BUILDING_DETP,
	TF_ENT_FLAG_BUILDING_ENTRANCE,
	TF_ENT_FLAG_BUILDING_EXIT,
	TF_ENT_FLAG_BUILDINPROGRESS,
	TF_ENT_FLAG_LEVEL2,
	TF_ENT_FLAG_LEVEL3,
} TF_EntityFlags;

// enumerations: TF_Powerups
//		TF_PWR_DISGUISE_BLUE - Disguised as blue team.
//		TF_PWR_DISGUISE_RED - Disguised as red team.
//		TF_PWR_DISGUISE_YELLOW - Disguised as yellow team.
//		TF_PWR_DISGUISE_GREEN - Disguised as green team.
//		TF_PWR_DISGUISE_SCOUT - Disguised as scout.
//		TF_PWR_DISGUISE_SNIPER - Disguised as sniper.
//		TF_PWR_DISGUISE_SOLDIER - Disguised as soldier.
//		TF_PWR_DISGUISE_DEMOMAN - Disguised as demo-man.
//		TF_PWR_DISGUISE_MEDIC - Disguised as medic.
//		TF_PWR_DISGUISE_HWGUY - Disguised as hwguy.
//		TF_PWR_DISGUISE_PYRO - Disguised as pyro.
//		TF_PWR_DISGUISE_SPY - Disguised as spy.
//		TF_PWR_DISGUISE_CIVILIAN - Disguised as civilian.
//		TF_PWR_FEIGNED - Entity is feigned.
typedef enum eTF_Powerups
{
	// Team Disguise
	TF_PWR_DISGUISE_BLUE = PWR_FIRST_USER,
	TF_PWR_DISGUISE_RED,
	TF_PWR_DISGUISE_YELLOW,
	TF_PWR_DISGUISE_GREEN,

	// Class Disguise
	TF_PWR_DISGUISE_SCOUT,
	TF_PWR_DISGUISE_SNIPER,
	TF_PWR_DISGUISE_SOLDIER,
	TF_PWR_DISGUISE_DEMOMAN,
	TF_PWR_DISGUISE_MEDIC,
	TF_PWR_DISGUISE_HWGUY,
	TF_PWR_DISGUISE_PYRO,
	TF_PWR_DISGUISE_ENGINEER,
	TF_PWR_DISGUISE_SPY,
	TF_PWR_DISGUISE_CIVILIAN,
	
	// Other powerups
	TF_PWR_CLOAKED,
} TF_Powerups;

// enumerations: TF_Weapon
// 		WP_UMBRELLA - Umbrella.
// 		WP_AXE - Axe.
// 		WP_CROWBAR - Crowbar.
// 		WP_MEDKIT - Med-kit.
// 		WP_KNIFE - Knife.
// 		WP_SPANNER - Spanner/wrench.
// 		WP_SHOTGUN - Shotgun.
// 		WP_SUPERSHOTGUN - Super shotgun.
// 		WP_NAILGUN - Nailgun.
// 		WP_SUPERNAILGUN - Super Nailgun.
// 		WP_GRENADE_LAUNCHER - Grenade Launcher.
// 		WP_ROCKET_LAUNCHER - Rocket Launcher.
// 		WP_SNIPER_RIFLE - Sniper Rifle.
// 		WP_RAILGUN - Railgun.
// 		WP_FLAMETHROWER - Flamethrower.
// 		WP_MINIGUN - Minigun/Assault cannon.
// 		WP_AUTORIFLE - Auto-rifle.
// 		WP_DARTGUN - Dart gun.
// 		WP_PIPELAUNCHER - Pipe Launcher.
// 		WP_NAPALMCANNON - Napalm Cannon.
//		TF_WP_GRENADE - Offhand Grenade.
typedef enum eTF_Weapon
{
	TF_WP_NONE = INVALID_WEAPON,
	TF_WP_UMBRELLA,
	TF_WP_AXE,
	TF_WP_CROWBAR,
	TF_WP_MEDKIT,
	TF_WP_KNIFE,
	TF_WP_SPANNER,
	TF_WP_SHOTGUN,
	TF_WP_SUPERSHOTGUN,
	TF_WP_NAILGUN,
	TF_WP_SUPERNAILGUN,
	TF_WP_GRENADE_LAUNCHER,
	TF_WP_ROCKET_LAUNCHER,
	TF_WP_SNIPER_RIFLE,
	TF_WP_RAILGUN,
	TF_WP_FLAMETHROWER,
	TF_WP_MINIGUN,
	TF_WP_AUTORIFLE,
	TF_WP_DARTGUN,
	TF_WP_PIPELAUNCHER,
	TF_WP_NAPALMCANNON,
	TF_WP_TOMMYGUN,
	TF_WP_DEPLOY_SG,
	TF_WP_DEPLOY_DISP,
	TF_WP_DEPLOY_DETP,
	TF_WP_DEPLOY_JUMPPAD,
	TF_WP_FLAG,

	TF_WP_GRENADE1,
	TF_WP_GRENADE2,

	TF_WP_GRENADE,
	TF_WP_GRENADE_CONC,
	TF_WP_GRENADE_EMP,
	TF_WP_GRENADE_NAIL,
	TF_WP_GRENADE_MIRV,
	TF_WP_GRENADE_GAS,
	TF_WP_GRENADE_CALTROPS,
	TF_WP_GRENADE_NAPALM,
	
	TF_WP_DETPACK,

	// THIS MUST STAY LAST
	TF_WP_MAX
} TF_Weapon;

// constants: TF_ItemTypes
//		TF_GREN_NORMAL - Normal grenade.
//		TF_GREN_CONCUSS - Concussion grenade.
//		TF_GREN_FLASH - Flash grenade.
//		TF_GREN_FLARE - Flare grenade.
//		TF_GREN_NAIL - Nail grenade.
//		TF_GREN_CLUSTER - Cluster Grenade(Mirv).
//		TF_GREN_CLUSTERSECTION - Cluster Section(Mirvlet).
//		TF_GREN_NAPALM - Napalm/Incendiary grenade.
//		TF_GREN_GAS - Gas/Hallucinogen grenade.
//		TF_GREN_EMP - EMP grenade.
typedef enum eTF_ItemTypes
{
	TF_GREN_NORMAL = 1,
	TF_GREN_CONCUSS,
	TF_GREN_FLASH,
	TF_GREN_FLARE,
	TF_GREN_NAIL,
	TF_GREN_CLUSTER,
	TF_GREN_CLUSTERSECTION,
	TF_GREN_NAPALM,
	TF_GREN_GAS,
	TF_GREN_EMP,

	// THIS MUST STAY LAST
	TF_NUM_GRENADES
} TF_ItemTypes;

// enumerations: TF_Team
//		TF_TEAM_BLUE - Blue team.
//		TF_TEAM_RED - Red team.
//		TF_TEAM_YELLOW - Yellow team.
//		TF_TEAM_GREEN - Green team.
typedef enum eTF_Team
{
	TF_TEAM_NONE = OB_TEAM_NONE,
	TF_TEAM_BLUE,
	TF_TEAM_RED,
	TF_TEAM_YELLOW,
	TF_TEAM_GREEN,

	// THIS MUST STAY LAST
	TF_TEAM_MAX
} TF_Team;

// typedef: TF_Events
//		Defines the events specific to the TF game, numbered starting at the end of
//		the global events.
typedef enum eTF_Events
{
	TF_MSG_BEGIN = EVENT_NUM_EVENTS,

	// General Events
	TF_MSG_CLASS_DISABLED, // todo: implement this
	TF_MSG_CLASS_NOTAVAILABLE, // todo: implement this
	TF_MSG_CLASS_CHANGELATER, // todo: implement this
	TF_MSG_BUILD_MUSTBEONGROUND,
	TF_MSG_INFECTED,
	TF_MSG_CURED,
	TF_MSG_BURNLEVEL,

	TF_MSG_GOT_ENGY_ARMOR,
	TF_MSG_GAVE_ENGY_ARMOR,
	TF_MSG_GOT_MEDIC_HEALTH,
	TF_MSG_GAVE_MEDIC_HEALTH,

	TF_MSG_GOT_DISPENSER_AMMO,

	// Scout
	TF_MSG_SCOUT_START,
	// Game Events
	TF_MSG_RADAR_DETECT_ENEMY,
	// Internal Events.
	TF_MSG_SCOUT_END,

	// Sniper
	TF_MSG_SNIPER_START,
	// Game Events
	TF_MSG_RADIOTAG_UPDATE,
	// Internal Events
	TF_MSG_SNIPER_END,

	// Soldier
	TF_MSG_SOLDIER_START,
	// Game Events
	// Internal Events
	TF_MSG_SOLDIER_END,

	// Demo-man
	TF_MSG_DEMOMAN_START,
	// Game Events
	TF_MSG_DETPACK_BUILDING,
	TF_MSG_DETPACK_BUILT,
	TF_MSG_DETPACK_BUILDCANCEL,
	TF_MSG_DETPACK_NOTENOUGHAMMO,
	TF_MSG_DETPACK_CANTBUILD,
	TF_MSG_DETPACK_ALREADYBUILT,
	TF_MSG_DETPACK_DETONATED,
	// Internal Events
	TF_MSG_PIPE_PROXIMITY,
	TF_MSG_DETPIPES,		// The bot has detected the desire to det pipes.
	TF_MSG_DETPIPESNOW,		// Configurable delayed message for the actual detting.
	TF_MSG_DEMOMAN_END,

	// Medic
	TF_MSG_MEDIC_START,
	// Game Events
	TF_MSG_CALLFORMEDIC,
	TF_MSG_UBERCHARGED,
	TF_MSG_UBERCHARGE_DEPLOYED,
	// Internal Events
	TF_MSG_MEDIC_END,

	// HW-Guy
	TF_MSG_HWGUY_START,
	// Game Events	
	// Internal Events
	TF_MSG_HWGUY_END,

	// Pyro
	TF_MSG_PYRO_START,
	// Game Events	
	// Internal Events
	TF_MSG_PYRO_END,

	// Spy
	TF_MSG_SPY_START,	
	// Game Events
	TF_MSG_DISGUISING,
	TF_MSG_DISGUISED,
	TF_MSG_DISGUISE_LOST,
	TF_MSG_CANT_CLOAK,
	TF_MSG_CLOAKED,
	TF_MSG_UNCLOAKED,
	TF_MSG_SABOTAGED_SENTRY,
	TF_MSG_SABOTAGED_DISPENSER,
	TF_MSG_CANTDISGUISE_AS_TEAM,
	TF_MSG_CANTDISGUISE_AS_CLASS,
	// Internal Events
	TF_MSG_SPY_END,

	// Engineer
	TF_MSG_ENGINEER_START,
	TF_MSG_SENTRY_START,
	// Game Events
	TF_MSG_CALLFORENGINEER,
	TF_MSG_SENTRY_NOTENOUGHAMMO,
	TF_MSG_SENTRY_ALREADYBUILT,
	TF_MSG_SENTRY_CANTBUILD,
	TF_MSG_SENTRY_BUILDING,
	TF_MSG_SENTRY_BUILT,
	TF_MSG_SENTRY_BUILDCANCEL,
	TF_MSG_SENTRY_DESTROYED,
	TF_MSG_SENTRY_SPOTENEMY,
	TF_MSG_SENTRY_AIMED,
	TF_MSG_SENTRY_DAMAGED,
	TF_MSG_SENTRY_STATS,
	TF_MSG_SENTRY_UPGRADED,
	TF_MSG_SENTRY_DETONATED,
	TF_MSG_SENTRY_DISMANTLED,
	// Internal Events
	TF_MSG_SENTRY_END,

	TF_MSG_DISPENSER_START,
	// Game Events
	TF_MSG_DISPENSER_NOTENOUGHAMMO,
	TF_MSG_DISPENSER_ALREADYBUILT,
	TF_MSG_DISPENSER_CANTBUILD,			
	TF_MSG_DISPENSER_BUILDING,
	TF_MSG_DISPENSER_BUILT,
	TF_MSG_DISPENSER_BUILDCANCEL,
	TF_MSG_DISPENSER_DESTROYED,
	TF_MSG_DISPENSER_ENEMYUSED,
	TF_MSG_DISPENSER_DAMAGED,
	TF_MSG_DISPENSER_STATS,
	TF_MSG_DISPENSER_DETONATED,
	TF_MSG_DISPENSER_DISMANTLED,

	TF_MSG_TELE_ENTRANCE_BUILDING,
	TF_MSG_TELE_ENTRANCE_BUILT,
	TF_MSG_TELE_ENTRANCE_DESTROYED,
	TF_MSG_TELE_ENTRANCE_CANCEL,
	TF_MSG_TELE_ENTRANCE_CANTBUILD,

	TF_MSG_TELE_EXIT_BUILDING,
	TF_MSG_TELE_EXIT_BUILT,
	TF_MSG_TELE_EXIT_DESTROYED,
	TF_MSG_TELE_EXIT_CANCEL,
	TF_MSG_TELE_EXIT_CANTBUILD,

	TF_MSG_TELE_STATS,

	// Internal Events
	TF_MSG_DISPENSER_BLOWITUP,
	TF_MSG_DISPENSER_END,
	TF_MSG_ENGINEER_END,

	// Civilian
	TF_MSG_CIVILIAN_START,
	// Game Events	
	// Internal Events
	TF_MSG_CIVILIAN_END,

	// THIS MUST STAY LAST
	TF_MSG_END_EVENTS
} TF_Events;

typedef enum eTF_SoundType
{
	TF_SND_RADAR = SND_MAX_SOUNDS,
	
	// THIS MUST BE LAST
	TF_SND_MAX_SOUNDS
} TF_SoundType;

// typedef: TF_GameMessage
//		Events that allow the bot to query for information from the game.
typedef enum eTF_GameMessage
{
	TF_MSG_START = GEN_MSG_END,

	// Info.
	TF_MSG_GETBUILDABLES,
	TF_MSG_GETHEALTARGET,

	// Get Info
	TF_MSG_PLAYERPIPECOUNT,
	TF_MSG_TEAMPIPEINFO,

	// Commands
	TF_MSG_CANDISGUISE,
	TF_MSG_DISGUISE,
	TF_MSG_CLOAK,
	TF_MSG_LOCKPOSITION,
	TF_MSG_HUDHINT,
	TF_MSG_HUDMENU,
	TF_MSG_HUDTEXT,

	// THIS MUST STAY LAST
	TF_MSG_END
} TF_GameMessage;

// typedef: TF_BuildableStatus
//		Enumerations for TF building status.
typedef enum eTF_BuildableStatus
{
	BUILDABLE_INVALID,
	BUILDABLE_BUILDING,
	BUILDABLE_BUILT,
} TF_BuildableStatus;
#endif
