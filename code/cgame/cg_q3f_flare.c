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
**	cg_q3f_flare.c
**
**	Light flare code, based on code (c) 2000 ydnar
*/

#include "cg_local.h"

#define DEFAULT_FLARE_RADIUS	1.0
#define DEFAULT_FLARE_SHADER	"etf_flareshader"	// default flare shader
//#define DEFAULT_FLARE_SHADER	"blobFlare"		// default flare shader
#define FLARE_FADE_SPEED		0.002			// amount to fade in 1 ms

static int			*gFlareFadeTime = NULL;		// timer and value are used for smoothly fading out an obscured flare
static float		*gFlareFadeValue = NULL;
static refdef_t		*gFlareRefDef = NULL;
static float		*gFlareBlind = NULL;
static qhandle_t	gFlareDefaultShader;
static float		fovproduct;					// used in culling

typedef struct lensflare_s {
	float distance;
	float scale;
	float atten;
	qhandle_t *shader;
} lensflare_t;

/*lensflare_t lensflares[] = {
	{ ..4f, 6f, .15f, &cgs.media.flare3 },
	{ .6f, .4f, .3f, &cgs.media.flare4 },
	{ .75f, 1.7f, .15f, &cgs.media.flare2 },
	{ 1.2f, .75f, .2f, &cgs.media.flare2 },
	{ 1.5f, 4.f, .15f, &cgs.media.flare2 },
	{ 2.f, 1.5f, .175f, &cgs.media.flare2 },
	{ 2.8f, 11.f, .15f, &cgs.media.flare2 }
};*/

/*lensflare_t lensflares[] = {
	{ .5f, 1.1f, .15f, &cgs.media.flare3 },
	{ 1.25f, 1.8f, .175f, &cgs.media.flare3 },
	{ 1.4f, .9f, .15f, &cgs.media.flare2 },
	{ 1.6f, 1.6f, .125f, &cgs.media.flare2 }
};*/

lensflare_t lensflares[] = {
	{ .5f, 1.1f, .1f, &cgs.media.flare3 },
	{ 1.25f, 1.8f, .125f, &cgs.media.flare3 },
	{ 1.4f, .9f, .1f, &cgs.media.flare2 },
	{ 1.6f, 1.6f, .075f, &cgs.media.flare2 }
};

int NUM_LENSFLARES = sizeof(lensflares) / sizeof(lensflares[0]);

lensflare_t sunflares[] = {
	{ -.3f, .12f, .5f, &cgs.media.whitering },
	{ .7f, .0564f, .5f, &cgs.media.bluedisc },
	{ .826f, .0936f, .5f, &cgs.media.bluedisc },
	{ .77f, .18f, .5f, &cgs.media.bluediscweak },
	{ 1.12f, .06f, .5f, &cgs.media.browndisc },
	{ 1.26f, .0192f, .5f, &cgs.media.whitegradient },
	{ 1.806f, .0324f, .5f, &cgs.media.whitegradient },
	{ 2.1f, .06f, .5f, &cgs.media.browndisc },
	{ 2.002f, .132f, .5f, &cgs.media.browndisc },
	{ 2.086f, .228f, .5f, &cgs.media.brownring },
	{ 2.352f, .096f, .5f, &cgs.media.bluedisc },
	{ 2.38f, .0516f, .5f, &cgs.media.bluegradient },
	{ 1.456f, .324f, .5f, &cgs.media.greenring },
	{ 1.89f, .636f, .5f, &cgs.media.rainbowring }
};

int NUM_SUNFLARES = sizeof(sunflares) / sizeof(sunflares[0]);

/*
==============
CG_RawFlare()

  adds the raw flare ref entity. doesn't clip it out, fade it, or anything.
  this *will* draw a flare where you specify. r, g, and b must be 0-255.
==============
*/
static void CG_RawFlare( const vec3_t org, float radius, float rotation, qhandle_t shader, vec3_t color, float atten, qboolean isLensFlare, qboolean doScreenBlind, qboolean isSunFlare ) {
	polyVert_t verts[4];
	int i;
	float halfRadius;
	vec3_t right, up, origin;
	
	// don't bother if brightness is zero
	if( !( atten > 0 ) )
		return;
	
	// null shader?
	if( !shader ) {
		// get default shader
		if( !gFlareDefaultShader ) {
			shader = gFlareDefaultShader = trap_R_RegisterShader( DEFAULT_FLARE_SHADER );
		} else {
			shader = gFlareDefaultShader;
		}
	}

	// calc radius based on fov (90 deg == 1:1)
	radius *= (gFlareRefDef->fov_x / 90);
	halfRadius = 0.5f * radius;

	if( isSunFlare ) {
		VectorMA( gFlareRefDef->vieworg, 8, org, origin );
	} else {
		// 'push' location towards camera to create fake origin
		VectorSubtract( org, gFlareRefDef->vieworg, origin );
		VectorNormalize( origin );
		VectorMA( gFlareRefDef->vieworg, 8, origin, origin );
	}

	if( rotation ) {
		vec3_t tmp;

		VectorCopy( gFlareRefDef->viewaxis[1], tmp );
		RotatePointAroundVector( right, gFlareRefDef->viewaxis[0], tmp, rotation );
		CrossProduct( gFlareRefDef->viewaxis[0], right, up );
	} else {
		VectorCopy( gFlareRefDef->viewaxis[1], right );
		VectorCopy( gFlareRefDef->viewaxis[2], up );
	}

	// Fov scaling
	VectorScale( right, 90.f / gFlareRefDef->fov_x, right );
	VectorScale( up, (90.f / cgs.glconfig.windowAspect) / gFlareRefDef->fov_y, up );

	// create the full polygon
	for( i = 0 ; i < 3 ; i++ ) {
		verts[0].xyz[i] = origin[i] + halfRadius * right[i] + halfRadius * up[i];
		verts[1].xyz[i] = origin[i] - halfRadius * right[i] + halfRadius * up[i];
		verts[2].xyz[i] = origin[i] - halfRadius * right[i] - halfRadius * up[i];
		verts[3].xyz[i] = origin[i] + halfRadius * right[i] - halfRadius * up[i];
	}

	for( i = 0 ; i < 4 ; i++ ) {
		verts[i].modulate[0] = 0xff * color[0] * atten;
		verts[i].modulate[1] = 0xff * color[1] * atten;
		verts[i].modulate[2] = 0xff * color[2] * atten;
		verts[i].modulate[3] = 0xff;
	}

	verts[0].st[0] = 1;
	verts[0].st[1] = 0;
	verts[1].st[0] = 0;
	verts[1].st[1] = 0;
	verts[2].st[0] = 0;
	verts[2].st[1] = 1;
	verts[3].st[0] = 1;
	verts[3].st[1] = 1;

	trap_R_AddPolyToScene( shader, 4, verts );

	// RR2DO2: lensflares
	if( isLensFlare ) {
		vec3_t flaredelta, viewcenter;
		float distanceFromCenter;
		float scopeFade = 1.f;

		if( !isSunFlare ) {
			if( cg.scopeEnabled ) {
				if( cg.scopeTime > cg.time )
					scopeFade = 1.f - ( ( cg.scopeTime - cg.time ) / Q3F_SCOPE_FADEINTIME );
				else
					scopeFade = 1.f;
			} else if ( cg.scopeTime > cg.time )
				scopeFade = ( cg.scopeTime - cg.time ) / Q3F_SCOPE_FADEOUTTIME;
			else {
				// RR2DO2: UGLY UGLY
				VectorMA( gFlareRefDef->vieworg, 7, gFlareRefDef->viewaxis[0], viewcenter );
				VectorSubtract( viewcenter, origin, flaredelta );

				distanceFromCenter = VectorLength( flaredelta );

				goto blindflash;
			}
		}

		VectorCopy( gFlareRefDef->viewaxis[1], right );
		VectorCopy( gFlareRefDef->viewaxis[2], up );

		VectorMA( gFlareRefDef->vieworg, 7, gFlareRefDef->viewaxis[0], viewcenter );
		VectorSubtract( viewcenter, origin, flaredelta );

		distanceFromCenter = VectorLength( flaredelta );

		if( distanceFromCenter < ( isSunFlare ? 6.5f : 4.5f ) ) {
			float scaledHalfRadius, attenScale;
			int nLensFlare;
			vec3_t renderorigin;
			
			attenScale = 0.35f * ( ( isSunFlare ? 6.5f : 4.5f ) - distanceFromCenter );

			attenScale = (attenScale > 1.f ? 1.f : attenScale);

			if( isSunFlare ) {
				for( nLensFlare = 0; nLensFlare < NUM_SUNFLARES; nLensFlare++ ) {
					for( i = 0 ; i < 4 ; i++ ) {
						float colorScale = atten * sunflares[nLensFlare].atten * attenScale;

						verts[i].modulate[0] = 0xff * color[0] * colorScale;
						verts[i].modulate[1] = 0xff * color[1] * colorScale;
						verts[i].modulate[2] = 0xff * color[2] * colorScale;
					}

					scaledHalfRadius = halfRadius * sunflares[nLensFlare].scale /** atten*/;	// uncomment atten to let the flare scale down on fade out due to blocking of line of sight to flare origin
					VectorMA( origin, sunflares[nLensFlare].distance, flaredelta, renderorigin );
					for( i = 0 ; i < 3 ; i++ ) {
						verts[0].xyz[i] = renderorigin[i] + scaledHalfRadius * right[i] + scaledHalfRadius * up[i] / cgs.glconfig.windowAspect;
						verts[1].xyz[i] = renderorigin[i] - scaledHalfRadius * right[i] + scaledHalfRadius * up[i] / cgs.glconfig.windowAspect;
						verts[2].xyz[i] = renderorigin[i] - scaledHalfRadius * right[i] - scaledHalfRadius * up[i] / cgs.glconfig.windowAspect;
						verts[3].xyz[i] = renderorigin[i] + scaledHalfRadius * right[i] - scaledHalfRadius * up[i] / cgs.glconfig.windowAspect;
					}
					trap_R_AddPolyToScene( *sunflares[nLensFlare].shader, 4, verts );
				}
			} else {
				for( nLensFlare = 0; nLensFlare < NUM_LENSFLARES; nLensFlare++ ) {
					for( i = 0 ; i < 4 ; i++ ) {
						float colorScale = atten * lensflares[nLensFlare].atten * attenScale * scopeFade;

						verts[i].modulate[0] = 0xff * color[0] * colorScale;
						verts[i].modulate[1] = 0xff * color[1] * colorScale;
						verts[i].modulate[2] = 0xff * color[2] * colorScale;
					}

					scaledHalfRadius = halfRadius * lensflares[nLensFlare].scale /** atten*/;	// uncomment atten to let the flare scale down on fade out due to blocking of line of sight to flare origin
					VectorMA( origin, lensflares[nLensFlare].distance, flaredelta, renderorigin );
					for( i = 0 ; i < 3 ; i++ ) {
						verts[0].xyz[i] = renderorigin[i] + scaledHalfRadius * right[i] + scaledHalfRadius * up[i] / cgs.glconfig.windowAspect;
						verts[1].xyz[i] = renderorigin[i] - scaledHalfRadius * right[i] + scaledHalfRadius * up[i] / cgs.glconfig.windowAspect;
						verts[2].xyz[i] = renderorigin[i] - scaledHalfRadius * right[i] - scaledHalfRadius * up[i] / cgs.glconfig.windowAspect;
						verts[3].xyz[i] = renderorigin[i] + scaledHalfRadius * right[i] - scaledHalfRadius * up[i] / cgs.glconfig.windowAspect;
					}
					trap_R_AddPolyToScene( *lensflares[nLensFlare].shader, 4, verts );
				}
			}
	

		}

blindflash:

		// Lensflare blinding
		if( distanceFromCenter < 4.5f && doScreenBlind && gFlareBlind && distanceFromCenter < 2.f) {
			gFlareBlind[0] = ( gFlareBlind[0] + color[0] * atten ) / 2.f;
			gFlareBlind[1] = ( gFlareBlind[1] + color[1] * atten ) / 2.f;
			gFlareBlind[2] = ( gFlareBlind[2] + color[2] * atten ) / 2.f;
			gFlareBlind[3] += ( 2.f - distanceFromCenter ) * atten;
		}
	}
	// RR2DO2
}

#define Q3F_FLARE_TRACE_INTERVAL 15

/*
==============
CG_AddFullFlareToScene()

  the fullblown flare call.
==============
*/
void CG_AddFullFlareToScene( const vec3_t org, float radius, float rotation, qhandle_t shader, float intensity, float r, float g, float b, qboolean isLensFlare, qboolean doScreenBlind ) {
	trace_t	trace;
	vec3_t	color, tracedelta;
	float	dist2, atten, fadeValue;
	int		obscured = 0;
	
	if( !cg_flares.integer )
		return;

	// dummy check
	if( intensity <= 0 || !gFlareRefDef )
		return;

	//if( !CG_CullPointAndRadius( org, radius ) ) {
	//	obscured = 1;
	if( CG_CullPointAndRadius( org, radius ) ) {
		obscured = 1;
	} else {
		// don't draw if flare is behind camera
		VectorSubtract( gFlareRefDef->vieworg, org, tracedelta );
		if( DotProduct( gFlareRefDef->viewaxis[0], tracedelta ) > fovproduct )
			obscured = 1;
		else {
			// determine if the flare is visible 
			CG_Trace( &trace, gFlareRefDef->vieworg, NULL, NULL, org, cg.snap->ps.clientNum, CONTENTS_SOLID|CONTENTS_BODY );
			if( trace.entityNum != ENTITYNUM_NONE ) {
				VectorSubtract( org, trace.endpos, trace.endpos );
				if( trace.endpos[0] > 4 || trace.endpos[0] < -4 ||
					trace.endpos[1] > 4 || trace.endpos[1] < -4 ||
					trace.endpos[2] > 4 || trace.endpos[2] < -4 ) {

					obscured = 1;
				}
			}
		}
	}

	// fade in?
	if( !gFlareFadeTime && obscured)
		return;
	if( gFlareFadeTime ) {
		if( !obscured ) {
			// reset the flare timer to current time if flare was obscured last frame
			if( *gFlareFadeTime <= 0 )
				*gFlareFadeTime = cg.time;
				
			// fade it in a bit
			if( *gFlareFadeValue < 1 ) {
				fadeValue = cg.time - *gFlareFadeTime;
				fadeValue *= (float)FLARE_FADE_SPEED;
				*gFlareFadeValue += fadeValue;
				if( *gFlareFadeValue > 1 )
					*gFlareFadeValue = 1;
			}
		} else {
			// reset the flare timer to current time if flare was visible last frame
			if( *gFlareFadeTime > 0 )
				*gFlareFadeTime = -1 * cg.time;
			
			// fade it out a bit
			if( *gFlareFadeValue > 0 ) {
				fadeValue = cg.time + *gFlareFadeTime;	// add because time is negative
				fadeValue *= (float)FLARE_FADE_SPEED;
				*gFlareFadeValue -= fadeValue; 
				if( *gFlareFadeValue < 0 )
					*gFlareFadeValue = 0;
			}
		}
	}
	
	// short circuit?
	fadeValue = 1.0;	
	if( gFlareFadeValue )
		fadeValue = *gFlareFadeValue;
	if( fadeValue <= 0 )
		return;
	
	// attenuate
	dist2 = DistanceSquared( org, gFlareRefDef->vieworg ) + .001;	// don't divide by zero
	atten = (intensity * intensity * 4) / dist2;
	atten = (atten > 1 ? 1 : atten);

	// fade off to zero as flare nears eye
	if( dist2 < 10000 )	// 200 game units
		atten *= (dist2 / 10000);
	
	// attenuate by timed fade
	atten *= fadeValue;

	// calc color
	color[0] = r;
	color[1] = g;
	color[2] = b;
	NormalizeColor( color, color );
	
	// add the raw flare
	//CG_RawFlare( org, radius, rotation, shader, (0xff * color[ 0 ] * atten), (0xff * color[ 1 ] * atten), (0xff * color[ 2 ] * atten), isLensFlare );
	CG_RawFlare( org, radius, rotation, shader, color, atten, isLensFlare, doScreenBlind, qfalse );
}

/*
==============
CG_AddFlareToScene()

  complements trap_R_AddLightToScene(). call this function
  wherever your cgame calls trap_R_AddLightToScene() for best effect.
==============
*/
void CG_AddFlareToScene( const vec3_t org, float intensity, float r, float g, float b, qboolean isLensFlare, qboolean doScreenBlind ) {
	if( !cg_flares.integer )
		return;

	CG_AddFullFlareToScene( org, DEFAULT_FLARE_RADIUS, 0, 0, intensity, r, g, b, isLensFlare, doScreenBlind );
}

/*
==============
CG_PositionSunflares()

  positions the sunflares
==============
*/
void CG_PositionSunflares( void ) {
	int i;

	// dummy check
	if( !gFlareRefDef )
		return;

	for( i = 0; i < cgs.numSunFlares; i++ ) {
		
		// dummy check
		if( cgs.sunFlares[i].intensity <= 0 )
			continue;

		// 'push' location towards camera to create fake origin
		VectorSubtract( cgs.sunFlares[i].pos, gFlareRefDef->vieworg, cgs.sunFlareTraceDelta[i] );
		VectorNormalize( cgs.sunFlareTraceDelta[i] );
	}
}

/* Ensiform - Sunflares from a sunportal origin*/
void CG_PositionSunflares2( void ) {
	int i;

	// dummy check
	if( !gFlareRefDef )
		return;

	if( !cgs.sunportal.exists )
		return;

	for( i = 0; i < cgs.numSunFlares; i++ ) {
		
		// dummy check
		if( cgs.sunFlares[i].intensity <= 0 )
			continue;

		// 'push' location towards camera to create fake origin
		VectorSubtract( cgs.sunFlares[i].pos, cgs.sunportal.origin/*gFlareRefDef->vieworg*/, cgs.sunFlareTraceDelta[i] );
		VectorNormalize( cgs.sunFlareTraceDelta[i] );
	}
}

/*
==============
CG_RenderSunflares()

  renders the sunflares
==============
*/
void CG_RenderSunflares( void ) {
	int i;

	// dummy check
	if( !gFlareRefDef )
		return;

	if( r_fastSky.integer )
		return;

	for( i = 0; i < cgs.numSunFlares; i++ ) {
		vec3_t	color, target, tracedelta;
		trace_t	trace;
		float	atten, fadeValue;
		int		obscured = 0;
		
		// dummy check
		if( cgs.sunFlares[i].intensity <= 0 )
			continue;

		if( !cg.rendering2ndRefDef )
			CG_SetFlareFader( &cgs.sunFlares[i].flareFadeTime, &cgs.sunFlares[i].flareFadeValue );

		// don't draw if flare is behind camera
		VectorNegate( cgs.sunFlareTraceDelta[i], tracedelta ); 
		if( DotProduct( gFlareRefDef->viewaxis[0], tracedelta ) > fovproduct )
			obscured = 1;
		else {
			// calculate atten for sunflare
			VectorMA( gFlareRefDef->vieworg, 65536,	cgs.sunFlareTraceDelta[i], target );	// FIXME: this isn't complete map, but should be enough

			// determine if the sunflare is visible 
			CG_Trace( &trace, gFlareRefDef->vieworg, NULL, NULL, target, cg.snap->ps.clientNum, CONTENTS_SOLID|CONTENTS_BODY );
			if( trace.entityNum != ENTITYNUM_NONE ) {
				VectorSubtract( target, trace.endpos, trace.endpos );
				/*if( trace.endpos[0] > 4 || trace.endpos[0] < -4 ||
					trace.endpos[1] > 4 || trace.endpos[1] < -4 ||
					trace.endpos[2] > 4 || trace.endpos[2] < -4 ) {

					if( !(trace.surfaceFlags & SURF_SKY) );
						obscured = 1;
				}*/
				//BirdDawg:: added the SURF_NODRAW check to avoid thinking it's obscured by weapclip
				if( trace.fraction < 1.0 && !(trace.surfaceFlags & SURF_SKY) && !(trace.surfaceFlags & SURF_NODRAW))
					obscured = 1;
			}
		}

		// fade in?
		if( !gFlareFadeTime && obscured )
			return;
		if( gFlareFadeTime ) {
			if( !obscured ) {
				// reset the flare timer to current time if flare was obscured last frame
				if( *gFlareFadeTime <= 0 )
					*gFlareFadeTime = cg.time;
					
				// fade it in a bit
				if( *gFlareFadeValue < 1 ) {
					fadeValue = cg.time - *gFlareFadeTime;
					fadeValue *= (float)FLARE_FADE_SPEED;
					*gFlareFadeValue += fadeValue;
					if( *gFlareFadeValue > 1 )
						*gFlareFadeValue = 1;
				}
			} else {
				// reset the flare timer to current time if flare was visible last frame
				if( *gFlareFadeTime > 0 )
					*gFlareFadeTime = -1 * cg.time;
				
				// fade it out a bit
				if( *gFlareFadeValue > 0 ) {
					fadeValue = cg.time + *gFlareFadeTime;	// add because time is negative
					fadeValue *= (float)FLARE_FADE_SPEED;
					*gFlareFadeValue -= fadeValue; 
					if( *gFlareFadeValue < 0 )
						*gFlareFadeValue = 0;
				}
			}
		}

		// short circuit?
		fadeValue = 1.0;	
		if( gFlareFadeValue )
			fadeValue = *gFlareFadeValue;
		if( fadeValue <= 0 )
			return;
		
		// attenuate
		atten = (cgs.sunFlares[i].intensity * cgs.sunFlares[i].intensity * 4);
		atten = (atten > 1 ? 1 : atten);
			
		// attenuate by timed fade
		atten *= fadeValue;
	
		NormalizeColor( cgs.sunFlares[i].color, color );

		CG_RawFlare( cgs.sunFlareTraceDelta[i],
					 cgs.sunFlares[i].radius,
					 (float)cgs.sunFlares[i].rotation,
					 cgs.sunFlares[i].shader,
					 color,
					 atten,
					 ( ( cgs.sunFlares[i].type == FL_LENSFLARE || cgs.sunFlares[i].type == FL_LENSFLAREBLIND ) ? qtrue : qfalse ),
					 ( ( cgs.sunFlares[i].type == FL_LENSBLIND || cgs.sunFlares[i].type == FL_LENSFLAREBLIND ) ? qtrue : qfalse ),
					 qtrue );
	}
}

/*
==============
CG_SetFlareFader()

  sets flare fade time counter to int/float pair pointed to. call with NULL, NULL to disable flare fading.
==============
*/
void CG_SetFlareFader( int *timer, float *value ) {
	if( !timer || !value )
	{
		timer = NULL;
		value = NULL;
	}
	gFlareFadeTime = timer;
	gFlareFadeValue = value;
}

/*
==============
CG_SetFlareRenderer()

  sets refdef where to render the flare to
==============
*/
void CG_SetFlareRenderer( refdef_t *refdef, vec4_t *flareblind ) {
	gFlareRefDef = refdef;
	gFlareBlind = (float *)flareblind;

	if( gFlareRefDef)
		fovproduct = sin( (M_PI / 360) * (gFlareRefDef->fov_x > gFlareRefDef->fov_y ? gFlareRefDef->fov_x : gFlareRefDef->fov_y) );
	else
		fovproduct = 0.f;
}

qboolean CG_FLR_StoreFlare( cg_q3f_flare_t *flare ) {
	if( cgs.numFlares >= (sizeof(cgs.flares) / sizeof(cg_q3f_flare_t)) ) {
		return qfalse;
	}

	memcpy( &cgs.flares[ cgs.numFlares++ ], flare, sizeof( cg_q3f_flare_t ) );

	return qtrue;
}

qboolean CG_FLR_UpdateFlare( cg_q3f_flare_t *flare ) {
	int		i;
	for( i = 0; i < cgs.numFlares; i++ ) {
		if(VectorCompare(cgs.flares[ i ].pos, flare->pos))
		{
			memcpy( &cgs.flares[ i ], flare, sizeof( cg_q3f_flare_t ) );
			return qtrue;
		}
	}

	return qfalse;
}

qboolean CG_FLR_StoreSunFlare( cg_q3f_flare_t *sunflare ) {
	if( cgs.numSunFlares >= (sizeof(cgs.sunFlares) / sizeof(cg_q3f_flare_t)) ) {
		return qfalse;
	}

	memcpy( &cgs.sunFlares[ cgs.numSunFlares++ ], sunflare, sizeof( cg_q3f_flare_t ) );

	return qtrue;
}

qboolean CG_FLR_UpdateSunFlare( cg_q3f_flare_t *sunflare ) {
	int		i;
	for( i = 0; i < cgs.numSunFlares; i++ ) {
		if(VectorCompare(cgs.sunFlares[ i ].pos, sunflare->pos))
		{
			memcpy( &cgs.sunFlares[ i ], sunflare, sizeof( cg_q3f_flare_t ) );
			return qtrue;
		}
	}

	return qfalse;
}

static qboolean CG_FLR_ParseError( int handle, char *format, ... ) {
	int line;
	char filename[128];
	va_list argptr;
	static char string[4096];

	va_start( argptr, format );
	Q_vsnprintf( string, sizeof( string ), format, argptr );
	va_end( argptr );

	filename[0] = '\0';
	line = 0;
	trap_PC_SourceFileAndLine( handle, filename, &line );

	Com_Printf( S_COLOR_RED "ERROR: %s, line %d: %s\n", filename, line, string );

	trap_PC_FreeSource( handle );

	return qfalse;
}

static qboolean CG_FLR_ParseFlare( int handle ) {
	pc_token_t token;
	cg_q3f_flare_t flare;

	memset( &flare, 0, sizeof( flare ) );

	flare.shader = trap_R_RegisterShader( DEFAULT_FLARE_SHADER );
	flare.radius = 1.0f;
	flare.type = FL_FLARE;
	flare.intensity = 300;
	flare.rotation = 0;
	VectorSet(flare.color, 1.f, 1.f, 1.f);

	if ( !trap_PC_ReadToken( handle, &token ) || Q_stricmp( token.string, "{" ) ) {
		return CG_FLR_ParseError( handle, "expected '{'" );
	}

	while ( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if ( !Q_stricmp( token.string, "origin" ) ) {
			if ( !PC_Vec_Parse( handle, &flare.pos ) ) {
				return CG_FLR_ParseError( handle, "expected origin vector" );
			}
		} else if ( !Q_stricmp( token.string, "shader" ) ) {
			if ( !trap_PC_ReadToken( handle, &token ) ) {
				return CG_FLR_ParseError( handle, "expected shader string" );
			} else {
				if(!Q_stricmp( token.string, "flareshader"))
					flare.shader = trap_R_RegisterShader( DEFAULT_FLARE_SHADER );
				else
					flare.shader = trap_R_RegisterShader( token.string );
			}
		} else if ( !Q_stricmp( token.string, "radius" ) ) {
			if ( !PC_Float_Parse( handle, &flare.radius ) ) {
				return CG_FLR_ParseError( handle, "expected wait value" );
			} else if ( flare.radius < 0 ) {
				return CG_FLR_ParseError( handle, "radius value %f is invalid", flare.radius );
			}
		} else if ( !Q_stricmp( token.string, "light" ) ) {
			if ( !PC_Float_Parse( handle, &flare.intensity ) ) {
				return CG_FLR_ParseError( handle, "expected light intensity value" );
			} else if ( flare.intensity < 0 ) {
				return CG_FLR_ParseError( handle, "light value %f is invalid", flare.intensity );
			}
		} else if ( !Q_stricmp( token.string, "rotation" ) ) {
			if ( !PC_Int_Parse( handle, &flare.rotation ) ) {
				return CG_FLR_ParseError( handle, "expected rotation value" );
			}
			flare.rotation %= 360;
		} else if ( !Q_stricmp( token.string, "color" ) ) {
			if ( !PC_Vec_Parse( handle, &flare.color ) ) {
				return CG_FLR_ParseError( handle, "expected color vector" );
			}
		} else if ( !Q_stricmp( token.string, "type" ) ) {
			if ( !trap_PC_ReadToken( handle, &token ) ) {
				return CG_FLR_ParseError( handle, "expected type value" );
			} else {
				if ( !Q_stricmp( token.string, "lensflareblind" ) ) {
					flare.type = FL_LENSFLAREBLIND;
				} else if ( !Q_stricmp( token.string, "lensblind" ) ) {
					flare.type = FL_LENSBLIND;
				} else if ( !Q_stricmp( token.string, "lensflare" ) ) {
					flare.type = FL_LENSFLARE;
				} else if ( !Q_stricmp( token.string, "flare" ) ) {
					flare.type = FL_FLARE;
				} else {
					return CG_FLR_ParseError( handle, "unknown type value '%s'", token.string );
				}
			}
		} else {
			return CG_FLR_ParseError( handle, "unknown token '%s'", token.string );
		}
	}

	if ( !CG_FLR_UpdateFlare( &flare ) ) {
		if ( !CG_FLR_StoreFlare( &flare ) ) {
			return CG_FLR_ParseError( handle, "Failed to store flare", token.string );
		}
	}

	return qtrue;
}

static qboolean CG_FLR_ParseSunFlare( int handle ) {
	pc_token_t token;
	cg_q3f_flare_t sunflare;

	memset( &sunflare, 0, sizeof( sunflare ) );

	sunflare.shader = trap_R_RegisterShader( DEFAULT_FLARE_SHADER );
	sunflare.radius = 1.0f;
	sunflare.type = FL_FLARE;
	sunflare.intensity = 300;
	sunflare.rotation = 0;
	VectorSet(sunflare.color, 1.f, 1.f, 1.f);

	if ( !trap_PC_ReadToken( handle, &token ) || Q_stricmp( token.string, "{" ) ) {
		return CG_FLR_ParseError( handle, "expected '{'" );
	}

	while ( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if ( !Q_stricmp( token.string, "origin" ) ) {
			if ( !PC_Vec_Parse( handle, &sunflare.pos ) ) {
				return CG_FLR_ParseError( handle, "expected origin vector" );
			}
		} else if ( !Q_stricmp( token.string, "shader" ) ) {
			if ( !trap_PC_ReadToken( handle, &token ) ) {
				return CG_FLR_ParseError( handle, "expected shader string" );
			} else {
				if(!Q_stricmp( token.string, "flareshader"))
					sunflare.shader = trap_R_RegisterShader( DEFAULT_FLARE_SHADER );
				else
					sunflare.shader = trap_R_RegisterShader( token.string );
			}
		} else if ( !Q_stricmp( token.string, "radius" ) ) {
			if ( !PC_Float_Parse( handle, &sunflare.radius ) ) {
				return CG_FLR_ParseError( handle, "expected wait value" );
			} else if ( sunflare.radius < 0 ) {
				return CG_FLR_ParseError( handle, "radius value %f is invalid", sunflare.radius );
			}
		} else if ( !Q_stricmp( token.string, "light" ) ) {
			if ( !PC_Float_Parse( handle, &sunflare.intensity ) ) {
				return CG_FLR_ParseError( handle, "expected light intensity value" );
			} else if ( sunflare.intensity < 0 ) {
				return CG_FLR_ParseError( handle, "light value %f is invalid", sunflare.intensity );
			}
		} else if ( !Q_stricmp( token.string, "rotation" ) ) {
			if ( !PC_Int_Parse( handle, &sunflare.rotation ) ) {
				return CG_FLR_ParseError( handle, "expected rotation value" );
			}
			sunflare.rotation %= 360;
		} else if ( !Q_stricmp( token.string, "color" ) ) {
			if ( !PC_Vec_Parse( handle, &sunflare.color ) ) {
				return CG_FLR_ParseError( handle, "expected color vector" );
			}
		} else if ( !Q_stricmp( token.string, "type" ) ) {
			if ( !trap_PC_ReadToken( handle, &token ) ) {
				return CG_FLR_ParseError( handle, "expected type value" );
			} else {
				if ( !Q_stricmp( token.string, "lensflareblind" ) ) {
					sunflare.type = FL_LENSFLAREBLIND;
				} else if ( !Q_stricmp( token.string, "lensblind" ) ) {
					sunflare.type = FL_LENSBLIND;
				} else if ( !Q_stricmp( token.string, "lensflare" ) ) {
					sunflare.type = FL_LENSFLARE;
				} else if ( !Q_stricmp( token.string, "flare" ) ) {
					sunflare.type = FL_FLARE;
				} else {
					return CG_FLR_ParseError( handle, "unknown type value '%s'", token.string );
				}
			}
		} else {
			return CG_FLR_ParseError( handle, "unknown token '%s'", token.string );
		}
	}

	if ( !CG_FLR_UpdateSunFlare( &sunflare ) ) {
		if ( !CG_FLR_StoreSunFlare( &sunflare ) ) {
			return CG_FLR_ParseError( handle, "Failed to store sunflare", token.string );
		}
	}

	return qtrue;
}

static qboolean CG_FLR_ParsePortal( int handle ) {
	pc_token_t token;

	if ( !trap_PC_ReadToken( handle, &token ) || Q_stricmp( token.string, "{" ) ) {
		return CG_FLR_ParseError( handle, "expected '{'" );
	}

	while ( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if ( !Q_stricmp( token.string, "origin" ) ) {
			if ( !PC_Vec_Parse( handle, &cgs.sunportal.origin ) ) {
				return CG_FLR_ParseError( handle, "expected origin vector" );
			}
		} else {
			return CG_FLR_ParseError( handle, "unknown token '%s'", token.string );
		}
	}

	if ( VectorCompare(cgs.sunportal.origin, vec3_origin ) ) {
		return CG_FLR_ParseError( handle, "Missing sunportal origin", token.string );
	}

	cgs.sunportal.exists = qtrue;

	return qtrue;
}

qboolean CG_LoadFlareScript( const char *filename ) {
	pc_token_t token;
	int handle;

	handle = trap_PC_LoadSource( filename );

	if ( !handle ) {
		return qfalse;
	}

	if ( !trap_PC_ReadToken( handle, &token ) || Q_stricmp( token.string, "flareScript" ) ) {
		return CG_FLR_ParseError( handle, "expected 'flareScript'" );
	}

	if ( !trap_PC_ReadToken( handle, &token ) || Q_stricmp( token.string, "{" ) ) {
		return CG_FLR_ParseError( handle, "expected '{'" );
	}

	while ( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if ( !Q_stricmp( token.string, "flareDef" ) ) {
			if ( !CG_FLR_ParseFlare( handle ) ) {
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "sunflareDef" ) ) {
			if ( !CG_FLR_ParseSunFlare( handle ) ) {
				return qfalse;
			}
		} else if ( !Q_stricmp( token.string, "portalDef" ) ) {
			if ( !CG_FLR_ParsePortal( handle ) ) {
				return qfalse;
			}
		} else {
			return CG_FLR_ParseError( handle, "unknown token '%s'", token.string );
		}
	}

	trap_PC_FreeSource( handle );

	return qtrue;
}
