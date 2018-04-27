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
**	cg_q3f_classutil.c
**
**	Playerclass util functions.
**
*/

#include "cg_local.h"
#include "../game/bg_q3f_playerclass.h"

void CG_Q3F_PlayClassnameSound( int clsnum, qboolean xian )
{
	// Play a classname sample if available

	if( clsnum < 0 || clsnum >= Q3F_CLASS_MAX )
		clsnum = 0;
	// Golliwog: Remove temporarily
	if( xian )
	{
		if( cgs.media.classnamesounds[clsnum][1] )
			trap_S_StartLocalSound( cgs.media.classnamesounds[clsnum][1], CHAN_LOCAL_SOUND );
		else if( cgs.media.classnamesounds[clsnum][0] )
			trap_S_StartLocalSound( cgs.media.classnamesounds[clsnum][0], CHAN_LOCAL_SOUND );
	}
	else
	{
		if( cgs.media.classnamesounds[clsnum][0] )
			trap_S_StartLocalSound( cgs.media.classnamesounds[clsnum][0], CHAN_LOCAL_SOUND );
	}
}


