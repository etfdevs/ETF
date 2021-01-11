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
**	cg_q3f_mapselect.h
**
**	Headers for the client side functions for selecting a map after the end of the game proper
*/

#ifndef	__CG_Q3F_MAPSELECT_H
#define	__CG_Q3F_MAPSELECT_H

#include "cg_local.h"

#define	ETF_MAPSELECT_SELECTCOUNT	10		// Number of maps to offer (no more than 10, or we'll run out of numbers)
#define	Q3F_MAPSELECT_SLIDEINTIME	3000	// Time for slide-in effect
#define	Q3F_MAPSELECT_FADEINTIME	500		// Time for fade-in effect
#define	Q3F_MAPSELECT_FADEOUTTIME	500		// Time for fade-out effect

enum cg_q3f_mapselectmode_t {
	Q3F_MAPSELECT_NONE,
	Q3F_MAPSELECT_SLIDEIN,
	Q3F_MAPSELECT_FADEIN,
	Q3F_MAPSELECT_FADEOUT,
	Q3F_MAPSELECT_READY,
};

void CG_Q3F_MapSelectTally( const char *cs );
void CG_Q3F_MapSelectInit( const char *cs );
qboolean CG_Q3F_MapSelectChoice( int choice );
qboolean CG_Q3F_MapSelectVote();
void CG_Q3F_MapSelectRespond();
qboolean CG_Q3F_MapSelectDraw();

#endif //__CG_Q3F_MAPSELECT_H
