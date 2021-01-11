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

/*
**	g_q3f_mapselect.h
**
**	Headers for the Q3F map query/vote/select functions
*/

#ifndef	__G_Q3F_MAPSELECT_H
#define	__G_Q3F_MAPSELECT_H

#include "g_local.h"

#define	Q3F_MAPSELECT_MAXQUERY		10		// Number of maps to query at a time
#define	Q3F_MAPSELECT_QUERYDELAY	300		// Delay between queries
#define	Q3F_MAPSELECT_REPONSETIME	3000	// Time to wait for laggard responses
#define	Q3F_MAPSELECT_THRESHHOLD	0.75	// What proportion of players require a map to select it
#define	ETF_MAPSELECT_SELECTCOUNT	10		// Number of maps to offer.

#define	Q3F_MAPSELECT_VOTEALLTIME	15000	// 15 seconds if everyone has voted
#define	Q3F_MAPSELECT_VOTETIME		45000	// 45 seconds to vote for a map

enum g_q3f_mapselectmode_t {
	Q3F_MAPSELECT_NONE,
	Q3F_MAPSELECT_QUERY,
	Q3F_MAPSELECT_RESPONSE,
	Q3F_MAPSELECT_READY,
	Q3F_MAPSELECT_CHANGEMAP,
};

void G_Q3F_MapSelectQuery();
void G_Q3F_MapSelectInit();

void G_Q3F_MapSelectResponse( gentity_t *ent );
void G_Q3F_MapSelectVote( gentity_t *ent );

qboolean G_Q3F_MapSelectGetArenaField( char *buf, char *queryfield, char *outbuff, int outsize );

#endif //__G_Q3F_MAPSELECT_H
