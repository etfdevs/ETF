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

//
// g_arenas.c
//

#include "g_local.h"

static int ModConversion[MOD_LASTONE] = {
/*	MOD_UNKNOWN	*/				STATS_OTHER,
/*	MOD_SHOTGUN	*/				STATS_WP + WP_SUPERSHOTGUN,
/*	MOD_AXE	*/					STATS_WP + WP_AXE,
/*	MOD_NAILGUN	*/				STATS_WP + WP_NAILGUN,
/*	MOD_GRENADE	*/				STATS_WP + WP_GRENADE_LAUNCHER,
/*	MOD_GRENADE_SPLASH	*/		STATS_WP + WP_GRENADE_LAUNCHER,
/*	MOD_PIPE	*/				STATS_WP + WP_PIPELAUNCHER,
/*	MOD_ROCKET	*/				STATS_WP + WP_ROCKET_LAUNCHER,
/*	MOD_ROCKET_SPLASH	*/		STATS_WP + WP_ROCKET_LAUNCHER,
/*	MOD_FLAME	*/				STATS_WP + WP_NAPALMCANNON,
/*	MOD_FLAME_SPLASH	*/		STATS_WP + WP_NAPALMCANNON,
/*	MOD_RAILGUN	*/				STATS_WP + WP_RAILGUN,
/*	MOD_WATER	*/				STATS_OTHER,
/*	MOD_SLIME	*/				STATS_OTHER,
/*	MOD_LAVA	*/				STATS_OTHER,
/*	MOD_CRUSH	*/				STATS_OTHER,
/*	MOD_TELEFRAG	*/			STATS_OTHER,
/*	MOD_FALLING	*/				STATS_OTHER,
/*	MOD_SUICIDE	*/				STATS_OTHER,
/*	MOD_TARGET_LASER	*/		STATS_OTHER,
/*	MOD_TRIGGER_HURT	*/		STATS_OTHER,
/*	MOD_SNIPER_RIFLE	*/		STATS_WP + WP_SNIPER_RIFLE,
/*	MOD_SNIPER_RIFLE_HEAD	*/	STATS_WP + WP_SNIPER_RIFLE,
/*	MOD_SNIPER_RIFLE_FEET	*/	STATS_WP + WP_SNIPER_RIFLE,
/*	MOD_RIFLE_ASSAULT	*/		STATS_WP + WP_ASSAULTRIFLE,
/*	MOD_DARTGUN	*/				STATS_WP + WP_DARTGUN,
/*	MOD_KNIFE	*/				STATS_WP + WP_AXE,
/*	MOD_DISEASE	*/				STATS_WP + WP_AXE,
/*	MOD_FAILED_OPERATION	*/	STATS_WP + WP_AXE,
/*	MOD_WRENCH	*/				STATS_WP + WP_AXE,
/*	MOD_HANDGREN	*/			STATS_GREN + Q3F_GREN_NORMAL,
/*	MOD_FLASHGREN	*/			STATS_GREN + Q3F_GREN_FLASH,
/*	MOD_NAILGREN	*/			STATS_GREN + Q3F_GREN_NAIL,
/*	MOD_CLUSTERGREN	*/			STATS_GREN + Q3F_GREN_CLUSTERSECTION,
/*	MOD_NAPALMGREN	*/			STATS_GREN + Q3F_GREN_NAPALM,
/*	MOD_GASGREN	*/				STATS_GREN + Q3F_GREN_GAS,
/*	MOD_PULSEGREN	*/			STATS_GREN + Q3F_GREN_EMP,
/*	MOD_CHARGE	*/				STATS_GREN + Q3F_GREN_CHARGE,
/*	MOD_AUTOSENTRY_BULLET	*/	STATS_SENTRY,
/*	MOD_AUTOSENTRY_ROCKET	*/	STATS_SENTRY,
/*	MOD_AUTOSENTRY_EXPLODE	*/	STATS_SENTRY,
/*	MOD_SUPPLYSTATION_EXPLODE	*/	STATS_SUPPLY,
/*	MOD_SINGLESHOTGUN	*/		STATS_WP + WP_SHOTGUN,
/*	MOD_MINIGUN	*/				STATS_WP + WP_MINIGUN,
/*	MOD_CUSTOM	*/				STATS_OTHER,
/*	MOD_MIRROR	*/				STATS_OTHER,
/*	MOD_BEAM	*/				STATS_OTHER,
/*	MOD_MAPSENTRY	*/			STATS_OTHER,	
/*	MOD_GASEXPLOSION	*/		STATS_GREN + Q3F_GREN_GAS,
/*	MOD_CRUSHEDBYSENTRY	*/		STATS_SENTRY,
/*	MOD_MAPSENTRY_BULLET	*/	STATS_OTHER,
/*	MOD_MAPSENTRY_ROCKET	*/	STATS_OTHER,
/*	MOD_NODROP	*/				STATS_OTHER,
/*	MOD_SUPERNAILGUN	*/		STATS_WP + WP_SUPERNAILGUN,
/*	MOD_CRUSHEDBYSUPPLYSTATION */	STATS_SUPPLY,
/*	MOD_NEEDLE_PRICK	*/		STATS_WP + WP_AXE,
/*	MOD_SWITCHTEAM		*/		STATS_OTHER,
/*	MOD_DISCONNECT		*/		STATS_OTHER,
};

void G_DamageStats(gclient_t *target, gclient_t *attacker, int damage, int mod ) {
	int index;
	if (mod >= MOD_LASTONE )
		index = STATS_OTHER;
	else index = ModConversion[ mod ];
	target->pers.stats.data[index].taken += damage;
	attacker->pers.stats.data[index].given += damage;
};

void G_DeathStats(gclient_t *target, gclient_t *attacker, int mod ) {
	int index;
	if (mod >= MOD_LASTONE )
		index = STATS_OTHER;
	else index = ModConversion[ mod ];
	target->pers.stats.data[index].deaths++;
	attacker->pers.stats.data[index].kills++;
};

int G_StatsModIndex( int mod ) {
	if (mod >= MOD_LASTONE )
		return STATS_OTHER;
	else return ModConversion[ mod ];
}
