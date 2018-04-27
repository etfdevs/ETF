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
**	cg_q3f_flyby.c
**
**	Flyby camera system
*/

#include "cg_local.h"

// Debug:
void CG_Q3F_DrawCamPaths( void ) {
	int segmentindex, splineindex, pathindex;
	vec3_t nullvec;
	vec3_t vec_lookat;

	VectorSet(nullvec, 0, 0, 0);
	
	for ( pathindex = 0; pathindex < cgs.camNumPaths; pathindex++ ) {
		for ( splineindex = 0; splineindex < cgs.campaths[pathindex].numsplines; splineindex++ ) {
			for ( segmentindex=0; segmentindex<cgs.campaths[pathindex].splines[splineindex].nSegments; segmentindex++ )
				CG_SpawnSmokeSprite(cgs.campaths[pathindex].splines[splineindex].SegmentVtx[segmentindex], 1, colorRed, 8, 8);
			VectorSet( vec_lookat, 200000, 200000, 200000 );
			if( !VectorCompare( cgs.campaths[pathindex].camsplines[splineindex].lookat, vec_lookat ) ) {
				CG_SpawnSmokeSprite(cgs.campaths[pathindex].camsplines[splineindex].lookat, 1, colorWhite, 8, 8);
			}
		}
	}
}
