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

#ifndef	__G_Q3F_ADMIN_H
#define	__G_Q3F_ADMIN_H

#include "g_local.h"

typedef struct g_q3f_extIpFilter_s {
	unsigned int mask, compare;
	int endtime;
	char *reason;
} g_q3f_extIpFilter_t;

extern q3f_array_t *ipFilters;
extern int g_q3f_banCheckTime;

extern q3f_array_t *ipMute;
extern int g_q3f_muteCheckTime;

qboolean StringToFilter( gentity_t *admin, char *s, g_q3f_extIpFilter_t *f );
void UpdateIPBans (void);
qboolean G_FilterPacket( char *from, char **reason );
void AddIP( gentity_t *admin, char *str, int time, char *reason );
void G_ProcessIPBans(void);
void G_Q3F_AdminCommand( gentity_t *admin );
void G_Q3F_AdminCommand_ClientOnly( gentity_t* admin );
void G_Q3F_AdminPasswordCommand( gentity_t *admin );
void G_Q3F_RCONPasswordCommand( gentity_t *ent );
void G_Q3F_AdminTempBan( gentity_t *player, char *reason, int time );
void G_Q3F_AdminCheckBannedPlayers();
int G_Q3F_AdminNextExpireBans();

#if 0
void UpdateIPMutes (void);
qboolean G_PlayerIPIsMuted( char *from );
qboolean G_PlayerIsMuted( gentity_t *ent );
void AddIPMute( gentity_t *admin, char *str, int time );
void G_ProcessIPMutes(void);
void G_Q3F_AdminCheckMutedPlayers();
int G_Q3F_AdminNextExpireMutes();

#define	Q3F_ADMIN_MUTEFILE		"mutelist.txt"		// Where to store mutes
#endif

#define	Q3F_ADMIN_BANFILE		"banlist.txt"		// Where to store bans
//#define	Q3F_ADMIN_FILE			"adminlist.txt"		// Where to store admins
#define	Q3F_ADMIN_MAXAUTH		5					// Number of auth attempts allowed before banning ; Ensiform: Lowered from 20 to 5
#define	Q3F_ADMIN_TEMPBAN_TIME	180					// Default ban time (3 minutes)
#define Q3F_ADMIN_DISMANTLECHEAT_TIME 180000		// Change-team/dismantle/change-team in this time and get kicked
#define	Q3F_ADMIN_TEAMKILL_BIAS	15					// Bias added for each ally kill > enemy kill
#define	Q3F_ADMIN_TEAMKILL_HEAT	15					// 'Heat' of a teamkill.
#define	Q3F_ADMIN_TEAMKILL_WARN	60					// 'Heat' at which player is warned.
#define	Q3F_ADMIN_TEAMKILL_KICK_1	75					// Lower 'Heat' at which player is kicked.
#define	Q3F_ADMIN_TEAMKILL_KICK_2	105					// Higher 'Heat' at which player is kicked.
#define Q3F_ADMIN_TEAMKILL_GRACE 3000				// Time when no more heat is added (to stop a spam kill causing an instant kick)

#endif
