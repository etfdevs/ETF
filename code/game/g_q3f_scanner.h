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
**	g_q3f_scanner.h
**
**	Server side functions for scanner handling
*/

#ifndef	__G_Q3F_SCANNER_H
#define	__G_Q3F_SCANNER_H

#include "g_local.h"

#define Q3F_SCANNER_CELL_TIME				600		// 1.5 cells a second
#define Q3F_SCANNER_UPDATE_INTERVAL			100
#define Q3F_SCANNER_RANGE_DIVISOR			10

void G_Q3F_Check_Scanner(struct gentity_s *ent);
void G_Q3F_Pack_Scanner_Update(struct gentity_s *ent, int clientNum);
int G_Q3F_FindNextMovingClient(struct gentity_s *ent, int lastnum, struct gclient_s *ignore);

#endif //__G_Q3F_SCANNER_H

