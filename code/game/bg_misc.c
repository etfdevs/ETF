/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

Enemy Territory Fortress
Copyright (C) 2000-2006 Quake III Fortress (Q3F) Development Team / Splash Damage Ltd.
Copyright (C) 2005-2018 Enemy Territory Fortress Development Team

This file is part of Enemy Territory Fortress (ETF).

ETF is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ETF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ETF. If not, see <http://www.gnu.org/licenses/>.

In addition, the Wolfenstein: Enemy Territory GPL Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the ETF Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

// bg_misc.c -- both games misc functions, all completely stateless

#include "q_shared.h"
//#include "bg_local.h"
#include "bg_public.h"
#include "bg_q3f_playerclass.h"

/*QUAKED item_***** ( 0 0 0 ) (-16 -16 -16) (16 16 16) suspended
DO NOT USE THIS CLASS, IT JUST HOLDS GENERAL INFORMATION.
The suspended flag will allow items to hang in the air, otherwise they are dropped to the next surface.

If an item is the target of another entity, it will not spawn in until fired.

An item fires all of its targets when it is picked up.  If the toucher can't carry it, the targets won't be fired.

"notfree" if set to 1, don't spawn in free for all games
"notteam" if set to 1, don't spawn in team games
"notsingle" if set to 1, don't spawn in single player games
"wait"	override the default wait before respawning.  -1 = never respawn automatically, which can be used with targeted spawning.
"random" random number of plus or minus seconds varied from the respawn time
"count" override quantity or duration on most items.
*/

gitem_t bg_extendeditemlist[] = 
{
	{
		NULL,
		NULL,
		{ NULL,
		NULL,
		0, 0} ,
/* icon */		NULL,
/* pickup */	NULL,
		0,
		0,
		0,
/* precache */ "",
/* sounds */ ""
	},	// leave index 0 alone


/*QUAKED weapon_gauntlet (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_bioaxe", 
		"sound/misc/w_pkup.wav",
        { "models/weapons2/bioweapon/bioweapon.md3",
		0, 0, 0},
/* icon */		"",
/* pickup */	"Bioweapon",
		0,
		IT_WEAPON,
		WP_AXE,
/* precache */ "",
/* sounds */ "sound/items/use_medikit.wav"
	},

	{
		"weapon_knife", 
		"sound/misc/w_pkup.wav",
        { "models/weapons2/knife/knife.md3",
		0, 0, 0},
/* icon */		"",
/* pickup */	"Knife",
		0,
		IT_WEAPON,
		WP_AXE,
/* precache */ "",
/* sounds */ ""
	},

	{
		"weapon_wrench", 
		"sound/misc/w_pkup.wav",
        { "models/weapons2/wrench/wrench.md3",
		0, 0, 0},
/* icon */		"",
/* pickup */	"Wrench",
		0,
		IT_WEAPON,
		WP_AXE,
/* precache */ "",
/* sounds */ ""
	}

};

gitem_t	bg_itemlist[] = 
{
	{
		NULL,
		NULL,
		{ NULL,
		NULL,
		0, 0} ,
/* icon */		NULL,
/* pickup */	NULL,
		0,
		0,
		0,
/* precache */ "",
/* sounds */ ""
	},	// leave index 0 alone

	//
	// ARMOR
	//

/*QUAKED item_armor_shard (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_armor_shard", 
		"sound/misc/am_pkup.wav",
		{ "models/powerups/armor/shard.md3", 
		"models/powerups/armor/shard_sphere.md3",
		0, 0} ,
/* icon */		"icons/iconr_shard",
/* pickup */	"Armor Shard",
		5,
		IT_ARMOR,
		0,
/* precache */ "",
/* sounds */ ""
	},

// FALCON : START : Q3F Armour types
/*QUAKED item_armor_yellow (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_armor_green", 
		"sound/misc/ar2_pkup.wav",
        { "models/powerups/armor/armor_gre.md3",
		0, 0, 0},
/* icon */		"icons/iconr_green",
/* pickup */	"Green Armor",
		50,
		IT_GREEN_ARMOUR,
		0,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED item_armor_yellow (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_armor_yellow", 
		"sound/misc/ar2_pkup.wav",
        { "models/powerups/armor/armor_yel.md3",
		0, 0, 0},
/* icon */		"icons/iconr_yellow",
/* pickup */	"Yellow Armor",
		100,
		IT_YELLOW_ARMOUR,
		0,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED item_armor_red (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_armor_red", 
		"sound/misc/ar2_pkup.wav",
        { "models/powerups/armor/armor_red.md3",
		0, 0, 0},
/* icon */		"icons/iconr_red",
/* pickup */	"Red Armor",
		150,
		IT_RED_ARMOUR,
		0,
/* precache */ "",
/* sounds */ ""
	},
// FALCON : END

/*QUAKED item_armor_combat (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_armor_combat", 
		"sound/misc/ar2_pkup.wav",
        { "models/powerups/armor/armor_yel.md3",
		0, 0, 0},
/* icon */		"icons/iconr_yellow",
/* pickup */	"Armor",
		50,
		IT_ARMOR,
		0,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED item_armor_body (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_armor_body", 
		"sound/misc/ar2_pkup.wav",
        { "models/powerups/armor/armor_red.md3",
		0, 0, 0},
/* icon */		"icons/iconr_red",
/* pickup */	"Heavy Armor",
		100,
		IT_ARMOR,
		0,
/* precache */ "",
/* sounds */ ""
	},

	//
	// health
	//
/*QUAKED item_health_small (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_health_small",
		"sound/items/s_health.wav",
        { "models/powerups/health/medium_sphere.md3", 0, 0, 0 },
/* icon */		"icons/iconh_green",
/* pickup */	"5 Health",
		5,
		IT_HEALTH,
		0,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED item_health (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_health",
		"sound/items/n_health.wav",
        { "models/powerups/health/medium_sphere.md3", 0, 0, 0 },
/* icon */		"icons/iconh_yellow",
/* pickup */	"25 Health",
		25,
		IT_HEALTH,
		0,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED item_health_large (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_health_large",
		"sound/items/l_health.wav",
        { "models/powerups/health/medium_sphere.md3", 0, 0, 0 },
/* icon */		"icons/iconh_red",
/* pickup */	"50 Health",
		50,
		IT_HEALTH,
		0,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED item_health_mega (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_health_mega",
		"sound/items/m_health.wav",
        { "models/powerups/health/mega_sphere.md3", 0, 0 },
/* icon */		"icons/iconh_mega",
/* pickup */	"Mega Health",
		100,
		IT_HEALTH,
		0,
/* precache */ "",
/* sounds */ ""
	},


	//
	// WEAPONS 
	//

/*QUAKED weapon_gauntlet (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_axe", 
		"sound/misc/w_pkup.wav",
        { "models/weapons2/axe/axe.md3",
		0, 0, 0},
/* icon */		"icons/iconw_axe",
/* pickup */	"Axe",
		0,
		IT_WEAPON,
		WP_AXE,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_shotgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_shotgun", 
		"sound/misc/w_pkup.wav",
        { "models/weapons2/single_shotgun/single_shotgun.md3", 
		0, 0, 0},
/* icon */		"icons/iconw_singleshotgun",	// JT
/* pickup */	"Single Barelled Shotgun",
		10,
		IT_WEAPON,
		WP_SHOTGUN,
/* precache */ "",
/* sounds */ ""
	},


/*QUAKED weapon_supershotgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_supershotgun", 
		"sound/misc/w_pkup.wav",
        { "models/weapons2/shotgun/shotgun.md3", 
		0, 0, 0},
/* icon */		"icons/iconw_shotgun",
/* pickup */	"Super Shotgun",
		10,
		IT_WEAPON,
		WP_SUPERSHOTGUN,
/* precache */ "",
/* sounds */ ""
	},


/*QUAKED weapon_nailgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_nailgun", 
		"sound/misc/w_pkup.wav",
		{ "models/weapons2/nailgun/nailgun.md3",
		0, 0, 0},
/* icon */		"icons/iconw_nailgun",
/* pickup */	"Nailgun",
		40,
		IT_WEAPON,
		WP_NAILGUN,
/* precache */ "",
/* sounds */ ""
	},


/*QUAKED weapon_supernailgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_supernailgun", 
		"sound/misc/w_pkup.wav",
		{ "models/weapons2/super_nailgun/sng.md3",
		0, 0, 0},
/* icon */		"icons/iconw_machinegun",
/* pickup */	"Supernailgun",
		40,
		IT_WEAPON,
		WP_SUPERNAILGUN,
/* precache */ "",
/* sounds */ ""
	},


/*QUAKED weapon_grenadelauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_grenadelauncher",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/grenlauncher/grenadel.md3", 
		0, 0, 0},
/* icon */		"icons/iconw_grenade",
/* pickup */	"Grenade Launcher",
		10,
		IT_WEAPON,
		WP_GRENADE_LAUNCHER,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_rocketlauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_rocketlauncher",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/rocketl/rocketl.md3", 
        //{ "models/ammo/nail/nail.md3",	// JT - HACK HACK HACK HACK
		0, 0, 0},
/* icon */		"icons/iconw_rocket",
/* pickup */	"Rocket Launcher",
		10,
		IT_WEAPON,
		WP_ROCKET_LAUNCHER,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_sniperrifle (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_sniperrifle", 
		"sound/misc/w_pkup.wav",
        { "models/weapons2/sniper_rifle/sniper_rifle.md3", 
		0, 0, 0},
/* icon */		"icons/iconw_sniper_rifle",
/* pickup */	"Sniper Rifle",
		100,
		IT_WEAPON,
		WP_SNIPER_RIFLE,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_railgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_railgun", 
		"sound/misc/w_pkup.wav",
        { "models/weapons2/eng_railgun/eng_railgun.md3", 
		0, 0, 0},
/* icon */		"icons/iconw_eng_railgun",
/* pickup */	"Railgun",
		10,
		IT_WEAPON,
		WP_RAILGUN,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_flamethrower (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_flamethrower", 
		"sound/misc/w_pkup.wav",
        { "models/weapons2/flamer/flamer.md3", 
		0, 0, 0},
/* icon */		"icons/iconw_flamer",
/* pickup */	"Flamethrower",
		50,
		IT_WEAPON,
		WP_FLAMETHROWER,
/* precache */ "",
/* sounds */ ""
	},

// weapon_minigun
	{
		"weapon_minigun",
		"sound/misc/w_pkup.wav",
		{ "models/weapons2/minigun/minigun.md3",
		0, 0, 0},
				"icons/iconw_minigun",
				"Minigun",
		50,
		IT_WEAPON,
		WP_MINIGUN,
			"",			// Precache
			"",			// Sounds
	},

/*QUAKED weapon_assaultrifle (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_assaultrifle", 
		"sound/misc/w_pkup.wav",
        { "models/weapons2/autocan/autocan.md3", 
		0, 0, 0},
/* icon */		"icons/iconw_rifle_assault",
/* pickup */	"Assault Cannon",
		100,
		IT_WEAPON,
		WP_ASSAULTRIFLE,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_dartgun (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_dartgun", 
		"sound/misc/w_pkup.wav",
		{ "models/weapons2/dartgun/dartgun.md3",		// JT: Currently use single shotty instead
		0, 0, 0},

		"icons/iconw_dartgun",	
/* pickup */	"Dart Gun",
		10,
		IT_WEAPON,
		WP_DARTGUN,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED weapon_pipelauncher (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_pipelauncher",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/pipelauncher/grenadep.md3", 
		0, 0, 0},
/* icon */		"icons/iconw_pipe",
/* pickup */	"Pipe Launcher",
		10,
		IT_WEAPON,
		WP_PIPELAUNCHER,
/* precache */ "",
/* sounds */ ""
	},




/*QUAKED weapon_napalm (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"weapon_napalm",
		"sound/misc/w_pkup.wav",
        { "models/weapons2/nplauncher/nplauncher.md3", 
		0, 0, 0},
/* icon */		"icons/iconw_napalm",
/* pickup */	"Napalm Cannon",
		20,
		IT_WEAPON,
		WP_NAPALMCANNON,
/* precache */ "",
/* sounds */ ""
	},
	//
	// AMMO ITEMS
	//

/*QUAKED ammo_shells (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_shells",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/shotgunam.md3", 
		0, 0, 0},
/* icon */		"icons/icona_shotgun",
/* pickup */	"Shells",
		10,
		IT_AMMO,
		AMMO_SHELLS,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED ammo_nails (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_nails",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/machinegunam.md3", 
		0, 0, 0},
/* icon */		"icons/icona_machinegun",
/* pickup */	"Nails",
		50,
		IT_AMMO,
		AMMO_NAILS,
/* precache */ "",
/* sounds */ ""
	},


/*QUAKED ammo_grenades (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_grenades",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/grenadeam.md3", 
		0, 0, 0},
/* icon */		"icons/icona_grenade",
/* pickup */	"Grenades",
		5,
		IT_AMMO,
		0,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED ammo_cells (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_cells",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/plasmaam.md3", 
		0, 0, 0},
/* icon */		"icons/icona_plasma",
/* pickup */	"Cells",
		30,
		IT_AMMO,
		AMMO_CELLS,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED ammo_lightning (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_lightning",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/lightningam.md3", 
		0, 0, 0},
/* icon */		"icons/icona_lightning",
/* pickup */	"Lightning",
		60,
		IT_AMMO,
		0,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED ammo_rockets (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_rockets",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/rocketam.md3", 
		0, 0, 0},
/* icon */		"icons/icona_rocket",
/* pickup */	"Rockets",
		5,
		IT_AMMO,
		AMMO_ROCKETS,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED ammo_slugs (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_slugs",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/railgunam.md3", 
		0, 0, 0},
/* icon */		"icons/icona_railgun",
/* pickup */	"Slugs",
		10,
		IT_AMMO,
		AMMO_CHARGE,															
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED ammo_bfg (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"ammo_bfg",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/bfgam.md3", 
		0, 0, 0},
/* icon */		"icons/icona_bfg",
/* pickup */	"Bfg Ammo",
		15,
		IT_AMMO,
		AMMO_MEDIKIT,															
/* precache */ "",
/* sounds */ ""
	},

	//
	// HOLDABLE ITEMS
	//
/*QUAKED holdable_teleporter (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"holdable_teleporter", 
		"sound/misc/ar2_pkup.wav",
        { "models/powerups/holdable/teleporter.md3", 
		0, 0, 0},
/* icon */		"icons/teleporter",
/* pickup */	"Personal Teleporter",
		60,
		IT_HOLDABLE,
		HI_TELEPORTER,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED holdable_medkit (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"holdable_medkit", 
		"sound/misc/health_pickup.wav",
        { 
		"models/powerups/holdable/medkit.md3", 
		"models/powerups/holdable/medkit_sphere.md3",
		0, 0},
/* icon */		"icons/medkit",
/* pickup */	"Medkit",
		60,
		IT_HOLDABLE,
		HI_MEDKIT,
/* precache */ "",
/* sounds */ "sound/items/use_medkit.wav"
	},

	//
	// POWERUP ITEMS
	//
/*QUAKED item_quad (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_quad", 
		"sound/items/quaddamage.wav",
        { "models/powerups/instant/quad.md3", 
        "models/powerups/instant/quad_ring.md3",
		0, 0 },
/* icon */		"icons/quad",
/* pickup */	"Quad Damage",
		30,
		IT_POWERUP,
		PW_QUAD,
/* precache */ "",
/* sounds */ "sound/items/damage3.wav"
	},

/*QUAKED item_enviro (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_enviro",
		"sound/items/protect3.wav",
        { "models/powerups/instant/enviro.md3", 
		"models/powerups/instant/enviro_ring.md3", 
		0, 0 },
/* icon */		"icons/envirosuit",
/* pickup */	"Battle Suit",
		30,
		IT_POWERUP,
		PW_BATTLESUIT,
/* precache */ "",
/* sounds */ "sound/items/protect3.wav"
	},

/*QUAKED item_haste (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_haste",
		"sound/misc/windfly.wav",
        { "models/powerups/instant/speed.md3",  //keeg renamed
		"models/powerups/instant/haste_ring.md3", 
		0, 0 },
/* icon */		"icons/haste",
/* pickup */	"Speed",
		30,
		IT_POWERUP,
		PW_HASTE,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED item_invis (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_invis",
		"sound/items/invisibility.wav",
        { "models/powerups/instant/invis.md3", 
		"models/powerups/instant/invis_ring.md3", 
		0, 0 },
/* icon */		"icons/invis",
/* pickup */	"Invisibility",
		30,
		IT_POWERUP,
		PW_INVIS,
/* precache */ "",
/* sounds */ ""
	},

/*QUAKED item_regen (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_regen",
		"sound/items/regeneration.wav",
        { "models/powerups/instant/regen.md3", 
		"models/powerups/instant/regen_ring.md3", 
		0, 0 },
/* icon */		"icons/regen",
/* pickup */	"Regeneration",
		30,
		IT_POWERUP,
		PW_REGEN,
/* precache */ "",
/* sounds */ "sound/items/regen.wav"
	},

/*QUAKED item_flight (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_flight",
		"sound/items/flight.wav",
        { "models/powerups/instant/flight.md3", 
		"models/powerups/instant/flight_ring.md3", 
		0, 0 },
/* icon */		"icons/flight",
/* pickup */	"Flight",
		60,
		IT_POWERUP,
		PW_FLIGHT,
/* precache */ "",
/* sounds */ "sound/items/flight.wav"
	},

/*QUAKED item_pentagram (.3 .3 1) (-16 -16 -16) (16 16 16) suspended
*/
	{
		"item_pentagram",
		"sound/items/protect3.wav",
        { "models/powerups/instant/pentagram.md3", 
		"models/powerups/instant/pentagram_ring.md3", 
		0, 0 },
/* icon */		"icons/pentagram",
/* pickup */	"Pentagram of Protection",
		30,
		IT_POWERUP,
		PW_PENTAGRAM,
/* precache */ "",
/* sounds */ "sound/items/protect3.wav"
	},

/*QUAKED team_CTF_redflag (1 0 0) (-16 -16 -16) (16 16 16)
Only in CTF games
*/
//	{
//		"team_CTF_redflag",
//		NULL,
//        { "models/flags/r_flag.md3",
//		0, 0, 0 },
/* icon */	//	"icons/iconf_red1",
/* pickup *///	"Red Flag",
//		0,
//		IT_TEAM,
//		PW_REDFLAG,
/* precache */ //"",
/* sounds */// ""
//	},

/*QUAKED team_CTF_blueflag (0 0 1) (-16 -16 -16) (16 16 16)
Only in CTF games
*/
//	{
//		"team_CTF_blueflag",
//		NULL,
//        { "models/flags/b_flag.md3",
//		0, 0, 0 },
/* icon */	//	"icons/iconf_blu1",
/* pickup *///	"Blue Flag",
//		0,
//		IT_TEAM,
//		PW_BLUEFLAG,
/* precache */// "",
/* sounds */// ""
//	},

	// Backpack

	{
		"backpack",
		"sound/misc/am_pkup.wav",
        { "models/objects/backpack/backpack.md3", 
		0, 0, 0},
/* icon */		"icons/backpack",
/* pickup */	"Backpack",
		15,
		IT_Q3F_BACKPACK,
		0,															
/* precache */ "",
/* sounds */ ""
	},

	{
		"ammobox_shells",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/shotgunam.md3", 
		0, 0, 0},
/* icon */		"icons/icona_shotgun",
/* pickup */	"Ammo (Shells)",
		10,
		IT_Q3F_AMMOBOX,
		AMMO_SHELLS,
/* precache */ "",
/* sounds */ ""
	},

	{
		"ammobox_nails",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/machinegunam.md3", 
		0, 0, 0},
/* icon */		"icons/icona_machinegun",
/* pickup */	"Ammo (Nails)",
		50,
		IT_Q3F_AMMOBOX,
		AMMO_NAILS,
/* precache */ "",
/* sounds */ ""
	},

	{
		"ammobox_cells",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/plasmaam.md3", 
		0, 0, 0},
/* icon */		"icons/icona_plasma",
/* pickup */	"Ammo (Cells)",
		30,
		IT_Q3F_AMMOBOX,
		AMMO_CELLS,
/* precache */ "",
/* sounds */ ""
	},

	{
		"ammobox_rockets",
		"sound/misc/am_pkup.wav",
        { "models/powerups/ammo/rocketam.md3", 
		0, 0, 0},
/* icon */		"icons/icona_rocket",
/* pickup */	"Ammo (Rockets)",
		5,
		IT_Q3F_AMMOBOX,
		AMMO_ROCKETS,
/* precache */ "",
/* sounds */ ""
	},

	// end of list marker
	{NULL}
};

int		bg_numItems = sizeof(bg_itemlist) / sizeof(bg_itemlist[0]) - 1;


/*
==============
BG_FindItemForPowerup
==============
*/
gitem_t	*BG_FindItemForPowerup( powerup_t pw ) {
	int		i;

	for ( i = 0 ; i < bg_numItems ; i++ ) {
		if ( (bg_itemlist[i].giType == IT_POWERUP || 
//					bg_itemlist[i].giType == IT_TEAM ||
					bg_itemlist[i].giType == IT_PERSISTANT_POWERUP) && 
			bg_itemlist[i].giTag == (int)pw ) {
			return &bg_itemlist[i];
		}
	}

	return NULL;
}


/*
==============
BG_FindItemForHoldable
==============
*/
gitem_t	*BG_FindItemForHoldable( holdable_t pw ) {
	int		i;

	for ( i = 0 ; i < bg_numItems ; i++ ) {
		if ( bg_itemlist[i].giType == IT_HOLDABLE && bg_itemlist[i].giTag == (int)pw ) {
			return &bg_itemlist[i];
		}
	}

	Com_Error( ERR_DROP, "HoldableItem not found" );

	return NULL;
}


/*
===============
BG_FindItemForWeapon

===============
*/
gitem_t	*BG_FindItemForWeapon( weapon_t weapon ) {
	gitem_t	*it;
	
	for ( it = bg_itemlist + 1 ; it->classname ; it++) {
		if ( it->giType == IT_WEAPON && it->giTag == (int)weapon ) {
			return it;
		}
	}

	Com_Error( ERR_DROP, "Couldn't find item for weapon %i", weapon);
	return NULL;
}

/*
===============
BG_FindItem

===============
*/
gitem_t	*BG_FindItem( const char *pickupName ) {
	gitem_t	*it;
	
	for ( it = bg_itemlist + 1 ; it->classname ; it++ ) {
		if ( !Q_stricmp( it->pickup_name, pickupName ) )
			return it;
	}

	return NULL;
}

/*
============
BG_PlayerTouchesItem

Items can be picked up without actually touching their physical bounds to make
grabbing them easier
============
*/
qboolean	BG_PlayerTouchesItem( playerState_t *ps, entityState_t *item, int atTime ) {
	vec3_t		origin;

	BG_EvaluateTrajectory( &item->pos, atTime, origin );

	// we are ignoring ducked differences here
	if ( ps->origin[0] - origin[0] > 44
		|| ps->origin[0] - origin[0] < -50
		|| ps->origin[1] - origin[1] > 36
		|| ps->origin[1] - origin[1] < -36
		|| ps->origin[2] - origin[2] > 36
		|| ps->origin[2] - origin[2] < -36 ) {
		return qfalse;
	}

	return qtrue;
}



/*
================
BG_CanItemBeGrabbed

Returns false if the item should not be picked up.
This needs to be the same for client side prediction and server use.
================
*/
qboolean	BG_CanItemBeGrabbed( int gametype, const entityState_t *ent, const playerState_t *ps, int time ) {

	gitem_t	*item;
	bg_q3f_playerclass_t *cls;

	if ( ent->modelindex < 1 || ent->modelindex >= bg_numItems ) {
		Com_Error( ERR_DROP, "BG_CanItemBeGrabbed: index out of range" );
	}

	cls = BG_Q3F_GetClass( ps );
	item = &bg_itemlist[ent->modelindex];

	switch( item->giType ) {
	case IT_WEAPON:
		return qfalse;	// weapons are always picked up	- JT - Not in Q3F, they're not.

	case IT_AMMO:
		if ( ps->ammo[ item->giTag ] >= 200 ) {
			return qfalse;		// can't hold any more
		}
		return qtrue;

	case IT_ARMOR:
	case IT_GREEN_ARMOUR:
	case IT_YELLOW_ARMOUR:
	case IT_RED_ARMOUR:
		// we also clamp armor to the maxhealth for handicapping
		// Golliwog: No, we don't :)
		if (ps->stats[STAT_ARMOR] >= cls->maxarmour) {
			return qfalse;
		}
		return qtrue;
	
	case IT_HEALTH:
		// small and mega healths will go over the max, otherwise
		// don't pick up if already at max
		// Golliwog: again, check against class health
		if ( item->quantity == 5 || item->quantity == 100 ) {
			if ( ps->stats[STAT_HEALTH] >= cls->maxhealth * 2 ) {
				return qfalse;
			}
			return qtrue;
		}
		// Golliwog.

		// Golliwog: It's a megahealth, check against _double_ class health.
		if ( ps->stats[STAT_HEALTH] >= cls->maxhealth ) {
			return qfalse;
		}
		return qtrue;
		// Golliwog.

	case IT_POWERUP:
		return qtrue;	// powerups are always picked up

	case IT_HOLDABLE:
		// can only hold one item at a time
		if ( ps->stats[STAT_HOLDABLE_ITEM] ) {
			return qfalse;
		}
		return qtrue;

	// TA powerups not valid in Q3F/ETF
	case IT_PERSISTANT_POWERUP:
		return qfalse;

	case IT_Q3F_BACKPACK:
	case IT_Q3F_AMMOBOX:
/*		// Don't pick up if you've already got whatever it offers.
		if( ent->time2 && ps->ammo[AMMO_SHELLS] < cls->maxammo_shells )
			return( qtrue );
		if( ent->legsAnim && ps->ammo[AMMO_NAILS] < cls->maxammo_nails )
			return( qtrue );
		if( ent->torsoAnim && ps->ammo[AMMO_ROCKETS] < cls->maxammo_rockets )
			return( qtrue );
		if( ent->weapon && ps->ammo[AMMO_CELLS] < cls->maxammo_cells )
			return( qtrue );
		return( qfalse );*/
		if( ent->otherEntityNum == ps->clientNum && time < (ent->time + 2000) )
			return( qfalse );	// Can't pick up our own backpack within two seconds
		return( qtrue );

        case IT_BAD:
            Com_Error( ERR_DROP, "BG_CanItemBeGrabbed: IT_BAD" );
	}

	return qfalse;
}

//======================================================================

/*
================
BG_EvaluateTrajectory

================
*/
float bg_evaluategravity;
void BG_EvaluateTrajectory( const trajectory_t *tr, int atTime, vec3_t result ) {
	float		deltaTime;
	float		phase;

	switch( tr->trType ) {
	case TR_STATIONARY:
	case TR_INTERPOLATE:
		VectorCopy( tr->trBase, result );
		break;
	case TR_LINEAR:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_SINE:
		deltaTime = ( atTime - tr->trTime ) / (float) tr->trDuration;
		phase = sin( deltaTime * M_PI * 2 );
		VectorMA( tr->trBase, phase, tr->trDelta, result );
		break;
	case TR_LINEAR_STOP:
		if ( atTime > tr->trTime + tr->trDuration ) {
			atTime = tr->trTime + tr->trDuration;
		}
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		if ( deltaTime < 0 ) {
			deltaTime = 0;
		}
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	case TR_GRAVITY:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		result[2] -= 0.5 * bg_evaluategravity * deltaTime * deltaTime;		// FIXME: local gravity...
		break;
	case TR_ROTATING:
		if ( tr->trTime > 0 )
			deltaTime = tr->trTime * 0.001;	// milliseconds to seconds
		else if ( tr->trTime < 0 )
			deltaTime = ( atTime + tr->trTime ) * 0.001;
		else
			deltaTime = ( atTime - tr->trTime ) * 0.001;
		VectorMA( tr->trBase, deltaTime, tr->trDelta, result );
		break;
	// RR2DO2
	//case TR_CIRCULAR:
		// step 1: calculate origin of circle
		//circ_origin[0] = tr->trBase[0] * tr->trDuration; // hack to get a third vec3_t
		//circ_origin[1] = tr->trBase[1] * tr->trDuration; // hack to get a third vec3_t
		//circ_origin[2] = tr->trBase[2] * tr->trDuration; // hack to get a third vec3_t
		// step 2: calculate lenght of circle
		//phase = 2 * M_PI * tr->
		// step 3: calculate speed (length of the delta)
		// step 4: calculate angle (360 / speed ?)
		// step 5: calculate new position in 3d (matrix multiplying?)
	// RR2DO2
	default:
		Com_Error( ERR_DROP, "BG_EvaluateTrajectory: unknown trType: %i", tr->trTime );
		break;
	}
}

/*
================
BG_EvaluateTrajectoryDelta

For determining velocity at a given time
================
*/
void BG_EvaluateTrajectoryDelta( const trajectory_t *tr, int atTime, vec3_t result ) {
	float	deltaTime;
	float	phase;

	switch( tr->trType ) {
	case TR_STATIONARY:
	case TR_INTERPOLATE:
		VectorClear( result );
		break;
	case TR_ROTATING:
	case TR_LINEAR:
		VectorCopy( tr->trDelta, result );
		break;
	case TR_SINE:
		deltaTime = ( atTime - tr->trTime ) / (float) tr->trDuration;
		phase = cos( deltaTime * M_PI * 2 );	// derivative of sin = cos
		phase *= 0.5;
		VectorScale( tr->trDelta, phase, result );
		break;
	case TR_LINEAR_STOP:
		if ( atTime > tr->trTime + tr->trDuration ) {
			VectorClear( result );
			return;
		}
		VectorCopy( tr->trDelta, result );
		break;
	case TR_GRAVITY:
		deltaTime = ( atTime - tr->trTime ) * 0.001;	// milliseconds to seconds
		VectorCopy( tr->trDelta, result );
		result[2] -= bg_evaluategravity * deltaTime;		// FIXME: local gravity...
		break;
	default:
		Com_Error( ERR_DROP, "BG_EvaluateTrajectoryDelta: unknown trType: %i", tr->trTime );
		break;
	}
}

// these are just for logging, the client prints its own messages
char *modNames[MOD_LASTONE] = {
	"MOD_UNKNOWN",
	"MOD_SHOTGUN",
	"MOD_AXE",
	"MOD_NAILGUN",
	"MOD_GRENADE",
	"MOD_GRENADE_SPLASH",
	"MOD_PIPE",
	"MOD_ROCKET",
	"MOD_ROCKET_SPLASH",
	"MOD_FLAME",
	"MOD_FLAME_SPLASH",
	"MOD_RAILGUN",
	"MOD_WATER",
	"MOD_SLIME",
	"MOD_LAVA",
	"MOD_CRUSH",
	"MOD_TELEFRAG",
	"MOD_FALLING",
	"MOD_SUICIDE",
	"MOD_TARGET_LASER",
	"MOD_TRIGGER_HURT",
	"MOD_SNIPER_RIFLE",
	"MOD_SNIPER_RIFLE_HEAD",
	"MOD_SNIPER_RIFLE_FOOT",
	"MOD_RIFLE_ASSAULT",
	"MOD_DARTGUN",
	"MOD_KNIFE",
	"MOD_DISEASE",
	"MOD_FAILED_OPERATION",
	"MOD_WRENCH",
	"MOD_HANDGREN",
	"MOD_FLASHGREN",
	"MOD_NAILGREN",
	"MOD_CLUSTERGREN",
	"MOD_NAPALMGREN",
	"MOD_GASGREN",
	"MOD_PULSEGREN",
	"MOD_CHARGE",
	"MOD_AUTOSENTRY_BULLET",
	"MOD_AUTOSENTRY_ROCKET",
	"MOD_AUTOSENTRY_EXPLODE",
	"MOD_SUPPLYSTATION_EXPLODE",
	"MOD_SINGLESHOTGUN",
	"MOD_MINIGUN",
	"MOD_CUSTOM",
	"MOD_MIRROR",
	"MOD_BEAM",
	"MOD_MAPSENTRY",
	"MOD_GASEXPLOSION",
	"MOD_CRUSHEDBYSENTRY",
	"MOD_MAPSENTRY_BULLET",
	"MOD_MAPSENTRY_ROCKET",
	"MOD_NODROP",
	"MOD_SUPERNAILGUN",
	"MOD_CRUSHEDBYSUPPLYSTATION",
	"MOD_NEEDLE_PRICK",
	"MOD_SWITCHTEAM",
	"MOD_DISCONNECT",
};

char *eventnames[] = {
	"EV_NONE",

	"EV_FOOTSTEP",
	"EV_FOOTSTEP_METAL",
	"EV_FOOTSPLASH",
	"EV_FOOTWADE",
	"EV_SWIM",

	"EV_STEP_4",
	"EV_STEP_8",
	"EV_STEP_12",
	"EV_STEP_16",

	"EV_FALL_SHORT",
	"EV_FALL_D11",
	"EV_FALL_D13",
	"EV_FALL_D15",
	"EV_FALL_D17",
	"EV_FALL_D19",
	"EV_FALL_D21",
	"EV_FALL_D23",
	"EV_FALL_D25",
	"EV_FALL_D27",
	"EV_FALL_D29",
	"EV_FALL_D31",

	"EV_JUMP_PAD",			// boing sound at origin, jump sound on player

	"EV_JUMP",
	"EV_WATER_TOUCH",	// foot touches
	"EV_WATER_LEAVE",	// foot leaves
	"EV_WATER_UNDER",	// head touches
	"EV_WATER_CLEAR",	// head leaves

	"EV_ITEM_PICKUP",			// normal item pickups are predictable
	"EV_GLOBAL_ITEM_PICKUP",	// powerup / team sounds are broadcast to everyone

	"EV_NOAMMO",
	"EV_CHANGE_WEAPON",
	"EV_COCK_WEAPON",
	"EV_FIRE_WEAPON",

	"EV_USE_ITEM0",
	"EV_USE_ITEM1",
	"EV_USE_ITEM2",
	"EV_USE_ITEM3",
	"EV_USE_ITEM4",
	"EV_USE_ITEM5",
	"EV_USE_ITEM6",
	"EV_USE_ITEM7",
	"EV_USE_ITEM8",
	"EV_USE_ITEM9",
	"EV_USE_ITEM10",
	"EV_USE_ITEM11",
	"EV_USE_ITEM12",
	"EV_USE_ITEM13",
	"EV_USE_ITEM14",
	"EV_USE_ITEM15",

	"EV_ITEM_RESPAWN",
	"EV_ITEM_POP",
	"EV_PLAYER_TELEPORT_IN",
	"EV_PLAYER_TELEPORT_OUT",

	"EV_GRENADE_BOUNCE",		// eventParm will be the soundindex

	"EV_GENERAL_SOUND",
	"EV_GLOBAL_SOUND",		// no attenuation
	"EV_GLOBAL_TEAM_SOUND",

	"EV_BULLET_HIT_FLESH",
	"EV_BULLET_HIT_WALL",

	"EV_SNIPER_HIT_FLESH",
	"EV_SNIPER_HIT_WALL",

	"EV_MISSILE_HIT",
	"EV_MISSILE_MISS",
	"EV_MISSILE_MISS_METAL",
	"EV_RAILTRAIL",
	"EV_SHOTGUN",
	"EV_SINGLESHOTGUN",
	"EV_MINIGUN",
	"EV_NAIL",				// otherEntity is the shooter

	"EV_PAIN",
	"EV_DEATH1",
	"EV_DEATH2",
	"EV_DEATH3",
	"EV_OBITUARY",

	"EV_POWERUP_QUAD",
	"EV_POWERUP_BATTLESUIT",
	"EV_POWERUP_REGEN",

	"EV_GIB_PLAYER",			// gib a previously living player
	"EV_SCOREPLUM",			// score plum

//#ifdef MISSIONPACK
	/*"EV_PROXIMITY_MINE_STICK",
	"EV_PROXIMITY_MINE_TRIGGER",
	"EV_KAMIKAZE",			// kamikaze explodes
	"EV_OBELISKEXPLODE",		// obelisk explodes
	"EV_OBELISKPAIN",			// obelisk is in pain
	"EV_INVUL_IMPACT",		// invulnerability sphere impact
	"EV_JUICED",				// invulnerability juiced effect
	"EV_LIGHTNINGBOLT",		// lightning bolt bounced of invulnerability sphere*/
//#endif

	"EV_DEBUG_LINE",

	"EV_STOPLOOPINGSOUND",

	"EV_TAUNT",
/*	"EV_TAUNT_YES",
	"EV_TAUNT_NO",
	"EV_TAUNT_FOLLOWME",
	"EV_TAUNT_GETFLAG",
	"EV_TAUNT_GUARDBASE",
	"EV_TAUNT_PATROL",*/

	"EV_WEAPON_AIM",
	"EV_WEAPON_START",
	"EV_WEAPON_END",
	"EV_DISEASE",

	"EV_SENTRY_IDLESTART",	// Golliwog: Idle start and stop sounds
	"EV_SENTRY_IDLESTOP",

	"EV_MUZZLEFLASH",				// Golliwog: The old classic

	"EV_ALLYOBITUARY",				// Just killed a teammate.

	"EV_ETF_EXPLOSION",			// Ent-driven explosion effect

	"EV_ETF_DISCARD_AMMO",
	"EV_ETF_USE_ITEM_FAILED",

	"EV_ETF_GASEXPLOSION",

	"EV_ETF_WATERSPLASH",

	"EV_VISUAL_TRIGGER",
	"EV_SENTRY_BUILD",

	"EV_ETF_MINIGUN_START",

	"EV_VISUAL_NAILFIRE",

	"EV_DEBUG_DATA",

	"EV_SENTRY_EXPLOSION",
	"EV_ETF_SUPPLYSTATION_EXPLOSION",

	"EV_GURP",
	"EV_DROWN",
	"EV_BURN",
	"EV_BURNTODEATH",

	"EV_HE_BEEP",
	"EV_HE_BEEP2",
	"EV_HE_EXPLODE",

	"EV_RELOAD_WEAPON",

	"EV_DOOR",				// slothy
	"EV_LIFT",				// slothy
	"EV_BOT_DEBUG_LINE",
	"EV_BOT_DEBUG_RADIUS",
	"EV_HEAL_PERSON",
	"EV_SENTRY_SPINUP",
	"EV_DISCONNECT",
	"EV_ETF_FLAMETHROWER_EFFECT",
	"EV_SUPPLY_BUILD",
	"EV_PLACE_BUILDING",
	"EV_POWERUP_PENTAGRAM"
};

/*
===============
BG_AddPredictableEventToPlayerstate

Handles the sequence numbers
===============
*/

void	trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

void BG_AddPredictableEventToPlayerstate( int newEvent, int eventParm, playerState_t *ps ) {
#ifdef _DEBUG
	{
		char buf[256];
		trap_Cvar_VariableStringBuffer("showevents", buf, sizeof(buf));
		if ( atof(buf) != 0 ) {
#ifdef QAGAME
			Com_Printf(" game event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount/*ps->commandTime*/, ps->eventSequence, eventnames[newEvent], eventParm);
#else
			Com_Printf("Cgame event svt %5d -> %5d: num = %20s parm %d\n", ps->pmove_framecount/*ps->commandTime*/, ps->eventSequence, eventnames[newEvent], eventParm);
#endif
		}
	}
#endif
	ps->events[ps->eventSequence & (MAX_EVENTS-1)] = newEvent;
	ps->eventParms[ps->eventSequence & (MAX_EVENTS-1)] = eventParm;
	ps->eventSequence++;
}

/*
========================
BG_TouchJumpPad
========================
*/
void BG_TouchJumpPad( playerState_t *ps, entityState_t *jumppad ) {
	vec3_t	angles;
	float p;
	int effectNum;

	// spectators don't use jump pads
	if ( ps->pm_type != PM_NORMAL ) {
		return;
	}

	// flying characters don't hit bounce pads
	if ( ps->powerups[PW_FLIGHT] ) {
		return;
	}

	// if we didn't hit this same jumppad the previous frame
	// then don't play the event sound again if we are in a fat trigger
	if ( ps->jumppad_ent != jumppad->number ) {

		vectoangles( jumppad->origin2, angles);
		p = fabs( AngleNormalize180( angles[PITCH] ) );
		if( p < 45 ) {
			effectNum = 0;
		} else {
			effectNum = 1;
		}
		BG_AddPredictableEventToPlayerstate( EV_JUMP_PAD, effectNum, ps );
	}
	// remember hitting this jumppad this frame
	ps->jumppad_ent = jumppad->number;
	ps->jumppad_frame = ps->pmove_framecount;
	// give the player the velocity from the jumppad
	VectorCopy( jumppad->origin2, ps->velocity );
}

/*
========================
BG_PlayerStateToEntityState

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityState( playerState_t *ps, entityState_t *s, qboolean snap ) {
	int		i;

	if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR || ps->pm_type == PM_ADMINSPECTATOR ) { // RR2DO2
		s->eType = ET_INVISIBLE;
	} else if ( ps->stats[STAT_HEALTH] <= GIB_HEALTH ) {
		s->eType = ET_INVISIBLE;
	} else {
		s->eType = ET_PLAYER;
		// JT
		s->otherEntityNum2 = ps->persistant[PERS_CURRCLASS];

	}

	s->number = ps->clientNum;

	s->pos.trType = TR_INTERPOLATE;
	VectorCopy( ps->origin, s->pos.trBase );
	if ( snap ) {
		SnapVector( s->pos.trBase );
	}

	// set the trDelta for flag direction
	VectorCopy( ps->velocity, s->pos.trDelta );

	s->apos.trType = TR_INTERPOLATE;
	VectorCopy( ps->viewangles, s->apos.trBase );
	if ( snap ) {
		SnapVector( s->apos.trBase );
	}

	s->angles2[YAW] = ps->movementDir;
	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;
	s->clientNum = ps->clientNum;		// ET_PLAYER looks here instead of at number
										// so corpses can also reference the proper config
	s->eFlags = ps->eFlags;
	if ( ps->stats[STAT_HEALTH] <= 0 ) {
		s->eFlags |= EF_DEAD;
	} else {
		s->eFlags &= ~EF_DEAD;
	}
	// Ridah, now using a circular list of events for all entities
	// add any new events that have been added to the playerState_t
	// (possibly overwriting entityState_t events)
	for (i = ps->oldEventSequence; i != ps->eventSequence; i++) {
		s->events[s->eventSequence & (MAX_EVENTS-1)] = ps->events[i & (MAX_EVENTS-1)];
		s->eventParms[s->eventSequence & (MAX_EVENTS-1)] = ps->eventParms[i & (MAX_EVENTS-1)];
		s->eventSequence++;
	}
	ps->oldEventSequence = ps->eventSequence;

	s->weapon = ps->weapon;
	s->groundEntityNum = ps->groundEntityNum;

	s->powerups = 0;
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( ps->powerups[ i ] ) {
			s->powerups |= 1 << i;
		}
	}

	s->loopSound = ps->loopSound;
	s->extFlags = ps->extFlags;
	s->generic1 = ps->generic1;
}

/*
========================
BG_PlayerStateToEntityStateExtraPolate

This is done after each set of usercmd_t on the server,
and after local prediction on the client
========================
*/
void BG_PlayerStateToEntityStateExtraPolate( playerState_t *ps, entityState_t *s, int time, qboolean snap ) {
	int		i;

	if ( ps->pm_type == PM_INTERMISSION || ps->pm_type == PM_SPECTATOR ) {
		s->eType = ET_INVISIBLE;
	} else if ( ps->stats[STAT_HEALTH] <= GIB_HEALTH ) {
		s->eType = ET_INVISIBLE;
	} else {
		s->eType = ET_PLAYER;
	}

	s->number = ps->clientNum;

	s->pos.trType = TR_LINEAR_STOP;
	VectorCopy( ps->origin, s->pos.trBase );
	if ( snap ) {
		SnapVector( s->pos.trBase );
	}
	// set the trDelta for flag direction and linear prediction
	VectorCopy( ps->velocity, s->pos.trDelta );
	// set the time for linear prediction
	s->pos.trTime = time;
	// set maximum extra polation time
	s->pos.trDuration = 50; // 1000 / sv_fps (default = 20)

	s->apos.trType = TR_INTERPOLATE;
	VectorCopy( ps->viewangles, s->apos.trBase );
	if ( snap ) {
		SnapVector( s->apos.trBase );
	}

	s->angles2[YAW] = ps->movementDir;
	s->legsAnim = ps->legsAnim;
	s->torsoAnim = ps->torsoAnim;
	s->clientNum = ps->clientNum;		// ET_PLAYER looks here instead of at number
										// so corpses can also reference the proper config
	s->eFlags = ps->eFlags;
	if ( ps->stats[STAT_HEALTH] <= 0 ) {
		s->eFlags |= EF_DEAD;
	} else {
		s->eFlags &= ~EF_DEAD;
	}

	for (i = ps->oldEventSequence; i != ps->eventSequence; i++) {
		s->events[s->eventSequence & (MAX_EVENTS-1)] = ps->events[i & (MAX_EVENTS-1)];
		s->eventParms[s->eventSequence & (MAX_EVENTS-1)] = ps->eventParms[i & (MAX_EVENTS-1)];
		s->eventSequence++;
	}
	ps->oldEventSequence = ps->eventSequence;

	s->weapon = ps->weapon;
	s->groundEntityNum = ps->groundEntityNum;

	s->powerups = 0;
	for ( i = 0 ; i < MAX_POWERUPS ; i++ ) {
		if ( ps->powerups[ i ] ) {
			s->powerups |= 1 << i;
		}
	}

	s->loopSound = ps->loopSound;
	s->extFlags = ps->extFlags;
}

float BG_JulianDay( int year, int month, int day ) {
	float extra = 100.f * year + month - 190002.5;
	float julian = 367.f * year;

	julian -= floor( 7.f * ( year + floor( ( month + 9.f ) / 12.f ) ) / 4.f );
	julian += floor( 275.f * month / 9.f );
	julian += day;
	julian -= 678985.5;
	julian -= .5f * extra / Q_fabs(extra);

	return julian;

}

// Note: this isn't too precise, but will do for an approximiation
int BG_ApproxDaysSinceCompile( qtime_t time ) {
	char *compiledate = __DATE__;
	char buf[64];
	int compileday, compilemonth, compileyear;
	float startjulian, endjulian;

	const char *mon_name[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
	};

	// get the month
	Q_strncpyz( buf, compiledate, sizeof(buf) );
	buf[3] = '\0';

	for( compilemonth = 0; compilemonth < 12; compilemonth++ ) {
		if( !Q_stricmp( buf, mon_name[compilemonth] ) ) {
			compilemonth += 1;
			break;
		}
	}

	// get the day
	Q_strncpyz( buf, compiledate + 4, sizeof(buf) );
	buf[3] = '\0';

	compileday = atoi(buf);

	// get the year
	Q_strncpyz( buf, compiledate + 7, sizeof(buf) );

	compileyear = atoi(buf);

	// get days passed
	startjulian = BG_JulianDay( compileyear, compilemonth, compileday );
	endjulian = BG_JulianDay( time.tm_year + 1900, time.tm_mon + 1, time.tm_mday );

	if( startjulian > endjulian )
		return 0;

	return( (int) (endjulian - startjulian) );
}


// Keeg might come in handy...
// strip colors and control codes, copying up to dwMaxLength-1 "good" chars and nul-terminating
// returns the length of the cleaned string
int BG_cleanName( const char *pszIn, char *pszOut, unsigned int dwMaxLength, qboolean fCRLF )
{
	const char *pInCopy = pszIn;
	const char *pszOutStart = pszOut;

	while( *pInCopy && ( pszOut - pszOutStart < (ptrdiff_t)dwMaxLength - 1 ) ) {
		if( *pInCopy == '^' )
			pInCopy += ((pInCopy[1] == 0) ? 1 : 2);
		else if( (*pInCopy < 32 && (!fCRLF || *pInCopy != '\n')) || (*pInCopy > 126))
			pInCopy++;
		else
			*pszOut++ = *pInCopy++;
	}

	*pszOut = 0;
	return( pszOut - pszOutStart );
}


//Keeg for crosshair code from ET
// Only used locally

typedef struct {
	char *colorname;
	vec4_t *color;
} colorTable_t;

// Colors for crosshairs
colorTable_t OSP_Colortable[] =
						{
							{ "white",		&colorWhite },
							{ "red",		&colorRed },
							{ "green",		&colorGreen },
							{ "blue",		&colorBlue },
							{ "yellow",		&colorYellow },
							{ "magenta",	&colorMagenta },
							{ "cyan",		&colorCyan },
							{ "orange",		&colorOrange },
							{ "mdred",		&colorMdRed },
							{ "mdgreen",	&colorMdGreen },
							{ "dkgreen",	&colorDkGreen },
							{ "mdcyan",		&colorMdCyan },
							{ "mdyellow",	&colorMdYellow },
							{ "mdorange",	&colorMdOrange },
							{ "mdblue",		&colorMdBlue },
							{ "ltgrey",		&colorLtGrey },
							{ "mdgrey",		&colorMdGrey },
							{ "dkgrey",		&colorDkGrey },
							{ "black",		&colorBlack },
							{ NULL,			NULL }
						};

extern void trap_Cvar_Set( const char *var_name, const char *value );
void BG_setCrosshair(char *colString, float *col, float alpha, char *cvarName)
{
	char *s = colString;

	col[0] = 1.0f;
	col[1] = 1.0f;
	col[2] = 1.0f;
	col[3] = (alpha > 1.0f) ? 1.0f : (alpha < 0.0f) ? 0.0f : alpha;

	if(*s == '0' && (*(s+1) == 'x' || *(s+1) == 'X')) {
		s +=2;
		//parse rrggbb
		if(Q_IsHexColorString(s)) {
			col[0] = ((float)(gethex(*(s)) * 16 + gethex(*(s+1)))) / 255.00;
			col[1] = ((float)(gethex(*(s+2)) * 16 + gethex(*(s+3)))) / 255.00;
			col[2] = ((float)(gethex(*(s+4)) * 16 + gethex(*(s+5)))) / 255.00;
			return;
		}
	} else {
		int i = 0;
		while(OSP_Colortable[i].colorname != NULL) {
			if(Q_stricmp(s, OSP_Colortable[i].colorname) == 0) {
				col[0] = (*OSP_Colortable[i].color)[0];
				col[1] = (*OSP_Colortable[i].color)[1];
				col[2] = (*OSP_Colortable[i].color)[2];
				return;
			}
			i++;
		}
	}

	trap_Cvar_Set(cvarName, "White");
}

char *_MS_FixColour( char *ptr, int colour ) {
	// Reset any trailing colours to the specified colour

	qboolean needfix;

	needfix = qfalse;
	while( *ptr )
	{
		if( *ptr == '^' )
		{
			if( !*++ptr )
				continue;
			if( *ptr == '*' )
			{
				*ptr = '0' + colour;
				needfix = qfalse;
			}
			else if( *ptr >= '0' && *ptr <= 'O' )
				needfix = qtrue;
		}
		ptr++;
	}
	if( needfix )
	{
		*(ptr++) = '^';
		*(ptr++) = '0' + colour;
		*ptr = 0;
	}
	return( ptr );
}
