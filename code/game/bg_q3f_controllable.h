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
**	bg_q3f_controllable.h
**
**	Shared headers for handling 'controllable' objects, such as vehicles
**	or gun turrets.
*/

#ifndef	_BG_Q3F_CONTROLLABLE_H
#define	_BG_Q3F_CONTROLLABLE_H

typedef enum {
	CONTROLLABLE_CAR,

	NUM_CONTROLLABLES
} controllableid_t;

typedef enum {
	CONTROLLABLETYPE_WHEELED,
	CONTROLLABLETYPE_SUBMERSIBLE,
	CONTROLLABLETYPE_FLOATING,
	CONTROLLABLETYPE_FLYING,

	NUM_CONTROLLABLTYPES
} controllabletype_t;

#define MAX_CONTROL_POINTS	6

typedef struct controllabledata_s {
	int id;
	char *name;
	int type, numControlPoints;
	vec4_t controlPoints[MAX_CONTROL_POINTS];		// Each wheel/turbine/rotor, whatever
	float controlPointGive[MAX_CONTROL_POINTS];		// The amount of 'give' against solid objects.

} controllabledata_t;


#endif//_BG_Q3F_CONTROLLABLE_H
