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
**	cg_q3f_water.c
**
**  Nice wavy water stuff
*/

#include "cg_local.h"

#ifdef Q3F_WATER

#define MAX_SURFACESIZE 100
//#define MAX_SURFACESTRIPVERTS 100
#define MAX_SURFACESTRIPVERTS 396	// ( 100 - 1 ) * 2 * 2
#define SQRTOFTWOINV 1.0f / 1.414213562f
#define WAVERATE 1000.f / 40.f		// 40 FPS

typedef struct watersurface_s {
	polyVert_t		verts[MAX_SURFACESIZE][MAX_SURFACESIZE];
	float			force[MAX_SURFACESIZE][MAX_SURFACESIZE];
	float			veloc[MAX_SURFACESIZE][MAX_SURFACESIZE];
	int				sub[2];
	float			step[2];
	vec_t			z;
	qhandle_t		shader;
	int				lastwavetime;
} watersurface_t;

static watersurface_t watersurf;

///////////////////////////////////////////////////////////////////////////////////
// Apply physics to the water
void CG_Q3F_WaterWave( watersurface_t *wsurf ) {
	int x, y;
	float d;

	for( x = 0; x < wsurf->sub[0]; x++ ) {
		for( y = 0; y < wsurf->sub[1]; y++ ) {
			wsurf->force[x][y] = 0.0f;
		}
	}

	for( x = 0; x < wsurf->sub[0] - 1; x++ ) {
		for( y = 0; y < wsurf->sub[1] - 1; y++ ) {
			d = wsurf->verts[x][y].xyz[2] - wsurf->verts[x][y-1].xyz[2];
			wsurf->force[x][y] -= d;
			wsurf->force[x][y-1] += d;

			d = wsurf->verts[x][y].xyz[2] - wsurf->verts[x-1][y].xyz[2];
			wsurf->force[x][y] -= d;
			wsurf->force[x-1][y] += d;

			d = wsurf->verts[x][y].xyz[2]- wsurf->verts[x][y+1].xyz[2];
			wsurf->force[x][y] -= d;
			wsurf->force[x][y+1] += d;

			d = wsurf->verts[x][y].xyz[2] - wsurf->verts[x+1][y].xyz[2];
			wsurf->force[x][y] -= d;
			wsurf->force[x+1][y] += d;

			d = (wsurf->verts[x][y].xyz[2] - wsurf->verts[x+1][y+1].xyz[2]) * SQRTOFTWOINV;
			wsurf->force[x][y] -= d;
			wsurf->force[x+1][y+1] += d;

			d = (wsurf->verts[x][y].xyz[2] - wsurf->verts[x-1][y-1].xyz[2]) * SQRTOFTWOINV;
			wsurf->force[x][y] -= d;
			wsurf->force[x-1][y-1] += d;

			d = (wsurf->verts[x][y].xyz[2] - wsurf->verts[x+1][y-1].xyz[2]) * SQRTOFTWOINV;
			wsurf->force[x][y] -= d;
			wsurf->force[x+1][y-1] += d;

			d = (wsurf->verts[x][y].xyz[2] - wsurf->verts[x+1][y-1].xyz[2]) * SQRTOFTWOINV;
			wsurf->force[x][y] -= d;
			wsurf->force[x+1][y-1] += d;
		}
	}

	for( x = 0; x < wsurf->sub[0]; x++ ) {
		for( y = 0; y < wsurf->sub[1]; y++ ) {
			wsurf->veloc[x][y] += wsurf->force[x][y] * 0.02f;
		}
	}
	
	d = (float)(wsurf->sub[0] + wsurf->sub[1]) / 2.f * 2;

	for( x = 0; x < wsurf->sub[0]; x++ ) {
		for( y = 0; y < wsurf->sub[1]; y++ ) {
			wsurf->verts[x][y].xyz[2] += wsurf->veloc[x][y];
			wsurf->verts[x][y].xyz[2] -= wsurf->verts[x][y].xyz[2] / d;
		}
	}
}

void CG_Q3F_WaterPoke( const refEntity_t *re ) {
	vec3_t mins, maxs;
	int x1, x2, y1, y2, y;
	float d;
	vec3_t delta;

	trap_R_ModelBounds( re->hModel, mins, maxs );
	VectorAdd( re->origin, mins, mins );
	VectorAdd( re->origin, maxs, maxs );

	// see if it's intersecting with water (we bulge the water a bit for added realism)

	// z-axis
	if( mins[2] > watersurf.z || maxs[2] + 8 < watersurf.z ) {
		return;
	}

	// x-axis
	if( mins[0] > watersurf.verts[watersurf.sub[0] - 1][0].xyz[0] || maxs[0] < watersurf.verts[0][0].xyz[0] ) {
		return;
	}

	// y-axis
	if( mins[1] > watersurf.verts[0][0].xyz[1] ||  maxs[1] < watersurf.verts[0][watersurf.sub[1] - 1].xyz[1] ) {
		return;
	}

	// we intersect, find out which vertices of the watersurface we touch

	// x-axis
	if( mins[0] <= watersurf.verts[0][0].xyz[0] )
		x1 = 0;
	else {
		d = mins[0] - watersurf.verts[0][0].xyz[0];
		x1 = ceil( d / watersurf.step[0] );
	}

	if( maxs[0] >= watersurf.verts[watersurf.sub[0] - 1][0].xyz[0] )
		x2 = watersurf.sub[0] - 1;
	else {
		d = maxs[0] - watersurf.verts[0][0].xyz[0];
		x2 = floor( d / watersurf.step[0] );
	}

	// y-axis
	if( maxs[1] >= watersurf.verts[0][0].xyz[1] )
		y1 = 0;
	else {
		d = watersurf.verts[0][0].xyz[1] - maxs[1];
		y1 = floor( d / watersurf.step[1] );
	}

	if( mins[1] <= watersurf.verts[0][watersurf.sub[1] - 1].xyz[1] )
		y2 = watersurf.sub[1] - 1;
	else {
		d = watersurf.verts[0][0].xyz[1] - mins[1];
		y2 = ceil( d / watersurf.step[1] );
	}

	// woop got our points, now wave
	VectorSubtract( re->origin, re->oldorigin, delta );
	if( delta[2] > 0 )
		d = 8.f;
	else
		d = -8.f;

	for( ; x1 < x2; x1++ ) {
		for( y = y1; y < y2; y++ ) {
			watersurf.verts[x1][y].xyz[2] = d;
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Render the water

static int lastwaterpoketime = 0;

void CG_Q3F_RenderWater() {
	int x, y;
	//polyVert_t *surfStrip[MAX_SURFACESTRIPVERTS];
	polyVert_t surfStrip[MAX_SURFACESTRIPVERTS];

	// every second, cause a distortion at a random point
	/*if( lastwaterpoketime < cg.time ) {
		x = Q_flrand(0.0f, 1.0f) * watersurf.sub[0];
		y = Q_flrand(0.0f, 1.0f) * watersurf.sub[1];
		watersurf.verts[x][y].xyz[2] = 8;
		watersurf.verts[x][y+1].xyz[2] = 8;
		watersurf.verts[x+1][y].xyz[2] = 8;
		watersurf.verts[x+1][y+1].xyz[2] = 8;

		lastwaterpoketime = cg.time + 1000;
	}*/

	if( watersurf.lastwavetime <= cg.time ) {
		CG_Q3F_WaterWave( &watersurf );
		watersurf.lastwavetime = cg.time + WAVERATE;
	}

	// fill in the water verts, draw them per strip
	for( x = 0; x < watersurf.sub[0] - 1; x++ ) {
		for( y = 0; y < watersurf.sub[1] - 1; y++ ) {
			/*surfStrip[4*y]		= &watersurf.verts[x+1][y];
			surfStrip[(4*y)+1]	= &watersurf.verts[x][y];
			surfStrip[(4*y)+2]	= &watersurf.verts[x][y+1];
			surfStrip[(4*y)+3]	= &watersurf.verts[x+1][y+1];*/
			memcpy( &surfStrip[4*y], &watersurf.verts[x][y], sizeof(polyVert_t) );
			surfStrip[4*y].xyz[2] += watersurf.z;
			memcpy( &surfStrip[(4*y)+1], &watersurf.verts[x+1][y], sizeof(polyVert_t) );
			surfStrip[(4*y)+1].xyz[2] += watersurf.z;
			memcpy( &surfStrip[(4*y)+2], &watersurf.verts[x+1][y+1], sizeof(polyVert_t) );
			surfStrip[(4*y)+2].xyz[2] += watersurf.z;
			memcpy( &surfStrip[(4*y)+3], &watersurf.verts[x][y+1], sizeof(polyVert_t) );
			surfStrip[(4*y)+3].xyz[2] += watersurf.z;
		}
		//trap_R_AddPolysToScene( watersurf.shader, 4, &surfStrip[0], watersurf.sub[1] - 1 );
		{
			int k;
			for (k=0; k<(watersurf.sub[1] - 1); k++) {
				trap_R_AddPolyToScene( watersurf.shader, 4, &surfStrip[k*4] );
			}
		}
	}
}

///////////////////////////////////////////////////////////////////////////////////
// Generate the surface
//
// the entOrigin is the topleft of the water surface

void CG_Q3F_WaterShader( vec3_t entOrigin, int xwidth, int ywidth, int xsub, int ysub, float sscale, float tscale, const char *shader ) {
	int x, y;
	float sstep, tstep;

	memset( &watersurf, 0, sizeof(watersurf) );

	watersurf.sub[0] = xsub + 1;
	if( watersurf.sub[0] >= MAX_SURFACESIZE )
		watersurf.sub[0] = MAX_SURFACESIZE - 1;

	watersurf.sub[1] = ysub + 1;
	if( watersurf.sub[1] >= MAX_SURFACESIZE )
		watersurf.sub[1] = MAX_SURFACESIZE - 1;

	watersurf.step[0] = (float)xwidth / (float)xsub;
	watersurf.step[1] = (float)ywidth / (float)ysub;

	sstep = sscale / (float)xsub;
	tstep = tscale / (float)ysub;

	watersurf.z = entOrigin[2];

	for( x = 0; x < watersurf.sub[0]; x++ ) {
		for( y = 0; y < watersurf.sub[1]; y++ ) {
			watersurf.verts[x][y].xyz[0] = entOrigin[0] + ( x * watersurf.step[0] );
			watersurf.verts[x][y].xyz[1] = entOrigin[1] - ( y * watersurf.step[1] );
			watersurf.verts[x][y].xyz[2] = 0;

			watersurf.verts[x][y].st[0] = x * sstep;
			watersurf.verts[x][y].st[1] = y * tstep;

			watersurf.verts[x][y].modulate[0] = 0xff;
			watersurf.verts[x][y].modulate[1] = 0xff;
			watersurf.verts[x][y].modulate[2] = 0xff;
			watersurf.verts[x][y].modulate[3] = 0xff;

            watersurf.force[x][y] = 0.f;		
			watersurf.force[x][y] = 0.f;
		}
	}

	watersurf.shader = trap_R_RegisterShader( shader );

	watersurf.lastwavetime = 0;
}

#endif // Q3F_WATER