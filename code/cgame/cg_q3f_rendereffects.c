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
**	cg_q3f_rendereffects.c
**
**	Special render effects for Q3F,
*/

#include "cg_local.h"


void CG_Q3F_ShaderBeam( const vec3_t start, const vec3_t end, float width, qhandle_t shader, const vec4_t color ) {

	polyVert_t verts[4];
	vec3_t up;
	byte rgba[4];

	rgba[0] = color[0] * 255;
	rgba[1] = color[1] * 255;
	rgba[2] = color[2] * 255;
	rgba[3] = color[3] * 255;
	width *= 0.5;

	GetPerpendicularViewVector( cg.refdef_current->vieworg, start, end, up );

	VectorMA( end, width, up, verts[0].xyz );
	verts[0].st[0] = 1;
	verts[0].st[1] = 0;
	*(int *)verts[0].modulate = *(int *)rgba;

	VectorMA( end, -width, up, verts[1].xyz );
	verts[1].st[0] = 1;
	verts[1].st[1] = 1;
	*(int *)verts[1].modulate = *(int *)rgba;

	VectorMA( start, -width, up, verts[2].xyz );
	verts[2].st[0] = 0;
	verts[2].st[1] = 1;
	*(int *)verts[2].modulate = *(int *)rgba;
	
	VectorMA( start, width, up, verts[3].xyz );
	verts[3].st[0] = 0;
	verts[3].st[1] = 0;
    *(int *)verts[3].modulate = *(int *)rgba;

	trap_R_AddPolyToScene( shader, 4, verts );
}

void CG_Q3F_AddBeamToScene( vec3_t *points, const int numpoints, qhandle_t shader, qboolean tileshader, vec4_t startColor, float *endColor, float scaleStart, float scaleEnd ) {
	vec3_t		v1, v2, prevpt1, prevpt2, up;
	int			i,ii,jj;
	polyVert_t	verts[4];
	//float		alphafactor;
	int			picW;
	float		scaleStep;
	float		scale;
	vec4_t		colorStep, color, prevColor;

	if( numpoints < 2 )
		return;
	
	picW = 64;

	scale = scaleStart;

	if( scaleEnd > 0 )
		scaleStep = ( scaleEnd - scaleStart ) / ( numpoints - 1 );
	else
		scaleStep = 0;

	//CG_Printf( "scaleStep: %f\n", scaleStep );

	if( endColor ) {
		colorStep[0] = ( endColor[0] - startColor[0] ) / ( numpoints - 1 );
		colorStep[1] = ( endColor[1] - startColor[1] ) / ( numpoints - 1 );
		colorStep[2] = ( endColor[2] - startColor[2] ) / ( numpoints - 1 );
		colorStep[3] = ( endColor[3] - startColor[3] ) / ( numpoints - 1 );
	} else {
		colorStep[0] = colorStep[1] = colorStep[2] = colorStep[3] = 0.f;
	}


	// go through and calculate each point of the beam and offset it by the anglevar
	for ( i = 0; i < numpoints - 1; i++ ) {

		// Create the up vec for the beam which is parallel to the viewplane
		VectorSubtract( points[i], cg.refdef.vieworg, v1 );
		VectorSubtract( points[i+1], cg.refdef.vieworg, v2 );
		CrossProduct( v1, v2, up );
		VectorNormalizeFast( up );

		if( i == 0 ) {
			// Calculate the first points
			VectorMA( points[0], scale, up, prevpt1 );
			VectorMA( points[0], -scale, up, prevpt2 );

			VectorCopy4( startColor, prevColor );
			VectorCopy4( startColor, color );
		}

		// scale up
		scale += scaleStep;

		// update color
		color[0] += colorStep[0];
		color[1] += colorStep[1];
		color[2] += colorStep[2];
		color[3] += colorStep[3];

		// Build the quad
		VectorMA( points[i+1], scale, up, verts[0].xyz );
		VectorCopy( prevpt1, verts[1].xyz );
		VectorCopy( prevpt2, verts[2].xyz );
		VectorMA( points[i+1], -scale, up, verts[3].xyz );

		 // Tile the shader across the beam
		if ( tileshader ) {
			vec3_t delta;
			float length;
			float startS, endS;

			VectorSubtract( points[i+1], points[i], delta );
			length = VectorLength( delta );
			startS = ( length * ( i-1 ) ) / (float)picW;
			endS   = ( length * ( i ) )   / (float)picW;

			verts[0].st[0] = startS; verts[0].st[1] = 1;
			verts[1].st[0] = endS;   verts[1].st[1] = 1;
			verts[2].st[0] = endS;   verts[2].st[1] = 0;
			verts[3].st[0] = startS; verts[3].st[1] = 0;
		} else {
			verts[0].st[0] = 1;   verts[0].st[1] = 1;
			verts[1].st[0] = 0;   verts[1].st[1] = 1;
			verts[2].st[0] = 0;   verts[2].st[1] = 0;
			verts[3].st[0] = 1;   verts[3].st[1] = 0;
		}

		// Set the color of the verts
		for ( ii=0; ii<4; ii++ ) {
			for ( jj=0; jj<4; jj++ ) {
				if( ii == 0 || ii == 3 )
					verts[ii].modulate[jj] = color[jj];
				else
					verts[ii].modulate[jj] = prevColor[jj];
			}
		}

		for ( ii=2; ii<4; ii++ ) {
			for ( jj=0; jj<4; jj++ ) {
				verts[ii].modulate[jj] = color[jj];
			}
		}

		trap_R_AddPolyToScene( shader, 4, verts );

		// Subtract off the overlap
		/*if ( overlap ) {
			p2 = p2 + ( dir * -overlap );
		}*/

		// Save off the last point to use as the first point on the next quad
		//VectorMA( points[i+1], scale, up, prevpt1 );
		//VectorMA( points[i+1], -scale, up, prevpt2 );
		VectorCopy( verts[0].xyz, prevpt1 );
		VectorCopy( verts[3].xyz, prevpt2 );

		VectorCopy4( color, prevColor );
	}

	//CG_Printf( "Done..\n" );
}
