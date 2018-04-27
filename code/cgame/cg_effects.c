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

// cg_effects.c -- these functions generate localentities, usually as a result
// of event processing

#include "cg_local.h"

/*
==================
CG_BubbleTrail

Bullets shot underwater
==================
*/
void CG_BubbleTrail( vec3_t start, vec3_t end, float spacing ) {
	vec3_t		move,vec;
	float		len;
	int			i;
	trace_t		trace;

	int sourceContentType = CG_PointContents( start, 0 ) & CONTENTS_WATER ;
	int destContentType = CG_PointContents( end, 0 ) & CONTENTS_WATER;

	// do a complete bubble trail if necessary
	if ( sourceContentType == destContentType ) {
		//Air to Air no trails
		if ( !sourceContentType ) return;
		VectorCopy( start ,move );
		VectorSubtract( end, start, vec );
	}
	// bubble trail from water into air
	else if ( sourceContentType ) {
		trap_CM_BoxTrace( &trace, end, start, NULL, NULL, 0, CONTENTS_WATER );
		VectorCopy( start ,move );
		VectorSubtract( trace.endpos, start, vec );
	}
	// bubble trail from air into water
	else /* if ( destContentType ) */ {
		trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, CONTENTS_WATER );
		VectorCopy( trace.endpos ,move );
		VectorSubtract( end, trace.endpos, vec );
	}

	len = VectorNormalize (vec);

	// advance a random amount first
	i = Q_flrand(0.0f, 1.0f) * spacing;
	VectorMA( move, i, vec, move );

	VectorScale (vec, spacing, vec);

	for ( ; i < len; i += spacing ) {
		localEntity_t	*le;
		refEntity_t		*re;
		int duration;

		duration = 1000 + (rand() & 255);

		le = CG_AllocLocalEntity( duration );
		le->leType = LE_MOVE_SCALE_FADE;
		le->radius = 6;
		le->radiusrate = -3.0f / duration;
		le->fadeRate = 1.0 / (duration / 4 );
		le->color[3] = 0.8;

		re = &le->refEntity;

		re->reType = RT_SPRITE;
		re->rotation = 0;
		re->customShader = cgs.media.waterBubbleShader;
		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;

		le->pos.trType = TR_LINEAR;
		le->pos.trTime = cg.time;
		VectorCopy( move, le->pos.trBase );
		le->pos.trDelta[0] = Q_flrand(-1.0f, 1.0f)*5;
		le->pos.trDelta[1] = Q_flrand(-1.0f, 1.0f)*5;
		le->pos.trDelta[2] = Q_flrand(-1.0f, 1.0f)*5 + 6;

		VectorAdd (move, vec, move);
	}
}

void CG_BulletExplosion( const vec3_t origin, const vec3_t dir ) {
	localEntity_t *le = CG_AllocLocalEntity( 500 + (rand() & 63) );
	refEntity_t *re = &le->refEntity;

	le->leType = LE_BULLET_EXPLOSION;
	re->reType = RT_MODEL;
	re->hModel = cgs.media.bulletFlashModel;
	VectorNormalize2( dir, re->axis[0]);
	RotateAroundDirection( re->axis, rand() % 360 );
	VectorCopy( origin, re->origin );
	VectorCopy( origin, re->oldorigin );
}


/*
=====================
CG_SmokePuff

Adds a smoke puff or blood trail localEntity.
=====================
*/
localEntity_t *CG_SmokePuff( const vec3_t p, const vec3_t vel, 
					int duration, int fadedelay,
				    float startradius, float endradius,
				    const vec4_t color, qhandle_t hShader )
{
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity( duration );
	le->fadeRate = (fadedelay < 0) ? 2000 : 1.0 / (duration - fadedelay);
	le->radius = startradius;
	le->radiusrate = (endradius - startradius) / duration;

	re = &le->refEntity;
	re->rotation = Q_flrand(0.0f, 1.0f) * 360;
	re->radius = startradius;
	le->leType = LE_MOVE_SCALE_FADE;

	VectorCopy4( color, le->color);

	le->pos.trType = TR_LINEAR;
	le->pos.trTime = cg.time;
	VectorCopy( vel, le->pos.trDelta );
	VectorCopy( p, le->pos.trBase );

	VectorCopy( p, re->origin );
	re->customShader = hShader;

	re->shaderRGBA[0] = color[0] * 0xff;
	re->shaderRGBA[1] = color[1] * 0xff;
	re->shaderRGBA[2] = color[2] * 0xff;
	re->shaderRGBA[3] = 0xff;

	re->reType = RT_SPRITE;
	re->radius = le->radius;

	return le;
}

void CG_NormalExplosion( const vec3_t origin ) {
	if (cg_no3DExplosions.value) 
		Spirit_RunScript( cgs.spirit.explosion_simple, origin, origin, axisDefault, 0 );
	else 
		Spirit_RunScript( cgs.spirit.explosion_normal, origin, origin, axisDefault, 0 );
}

void CG_QuadExplosion( const vec3_t origin, int team ) {
	if (cg_no3DExplosions.value) {
		Spirit_RunScript( cgs.spirit.explosion_simple, origin, origin, axisDefault, 0 );
		return;
	}
	Spirit_SetCustomShader( CG_Q3F_ShaderForQuad (team ) );
	Spirit_SetCustomColor( CG_Q3F_LightForQuad( team ) );
	Spirit_RunScript( cgs.spirit.explosion_quad, origin, origin, axisDefault,0 );
}

/*
====================
CG_MakeExplosion
====================
*/
localEntity_t *CG_MakeExplosion( vec3_t origin,
								int duration, float startradius, float endradius,
								qhandle_t hModel, qhandle_t shader
								)
{
	localEntity_t	*le;
	refEntity_t		*re;
	//int				offset;

	// skew the time a bit so they aren't all in sync
	//offset = rand() & 63;
	le = CG_AllocLocalEntity( duration );
	re = &le->refEntity;

	le->leType = LE_EXPLOSION;
	le->radius = startradius;
	le->radiusrate = ( endradius - startradius ) / duration;

	if ( !hModel ) {
		// randomly rotate sprite orientation
		le->refEntity.rotation = rand() % 360;
	} else {
		re->reType = RT_MODEL;
		AxisClear(re->axis);
	}

	re->hModel = hModel;
	re->customShader = shader;

	// set origin
	VectorCopy( origin, re->origin );
	VectorCopy( origin, re->oldorigin );

	le->color[0] = le->color[1] = le->color[2] = 1.0;
	re->shaderRGBA[0] = re->shaderRGBA[1] = re->shaderRGBA[2] = 255;
	return le;
}

/*
==================
CG_SpawnEffect

Player teleporting in or out
==================
*/
void CG_SpawnEffect( vec3_t org ) {
	Spirit_RunScript( cgs.spirit.spawn, org, org, axisDefault, 0 );
}


/*
==================
CG_ScorePlum
==================
*/
void CG_ScorePlum( int client, vec3_t org, int score ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			angles;
	static vec3_t lastPos;

	// only visualize for the client that scored
	if (client != cg.predictedPlayerState.clientNum || cg_scorePlum.integer == 0) {
		return;
	}

	le = CG_AllocLocalEntity( 4000 );
	le->leFlags = 0;
	le->leType = LE_SCOREPLUM;

	
	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1.0;
	le->leFlags = score;
	
	VectorCopy( org, le->pos.trBase );
	if (org[2] >= lastPos[2] - 20 && org[2] <= lastPos[2] + 20) {
		le->pos.trBase[2] -= 20;
	}

	VectorCopy(org, lastPos);

	re = &le->refEntity;

	re->reType = RT_SPRITE;
	re->radius = 16;

	VectorClear(angles);
	AnglesToAxis( angles, re->axis );
}

/*
====================
CG_Q3F_MakeRing
====================
*/
localEntity_t *CG_Q3F_MakeRing( const vec3_t origin, 
							    const vec3_t angles,
							    int duration,
								float startradius,
								float endradius,
								int fadedelay,
								qhandle_t hShader ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity( duration );
	le->radius = startradius;
	le->radiusrate = ( endradius - startradius ) / duration;
	le->fadeRate = (fadedelay < 0) ? 2000 : 1.0 / (duration - fadedelay);

	re = &le->refEntity;
	re->rotation = Q_flrand(0.0f, 1.0f) * 360;
	le->leType = LE_EXPAND_FADE_RING;
	VectorCopy( angles, le->angles.trBase );

	VectorCopy( origin, re->origin );
	re->customShader = hShader;

	return le;
}

/*
====================
CG_Q3F_MakeBeam
====================
*/
localEntity_t *CG_Q3F_MakeBeam( const vec3_t origin, 
							    const vec3_t dest,
							    float max_offset,
								int numSubdivisions,
								float scale,
								int leFlags,
								vec3_t colour,
								int duration,
								float speedscale,
								qhandle_t hShader ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity( duration );
	re = &le->refEntity;

	le->leType = LE_BEAM;
	le->leFlags = leFlags;
	le->radius = scale;
	le->bounceFactor = max_offset;
	le->lifeRate = numSubdivisions;
	le->light = speedscale;
	VectorCopy4( colour, le->color );

	VectorCopy( origin, le->pos.trBase );
	VectorCopy( dest, le->angles.trBase );

	re->customShader = hShader;

	return le;
}


/*
=================
CG_Bleed

This is the spurt of blood when a character gets hit
=================
*/
void CG_Bleed( vec3_t origin, int entityNum ) {
	localEntity_t	*le;

	if ( !cg_blood.integer ) {
		return;
	}

	le = CG_AllocLocalEntity( 500 );
	le->leType = LE_FADE_RGB;

	VectorCopy ( origin, le->refEntity.origin);
	le->color[0] = le->color[1] = le->color[2] = le->color[3] = 1;
	le->refEntity.reType = RT_SPRITE;
	le->refEntity.rotation = rand() % 360;
	le->refEntity.radius = 24;
	le->refEntity.customShader = cgs.media.bloodExplosionShader;

	// don't show player's own blood in view
	if ( entityNum == cg.snap->ps.clientNum ) {
		le->refEntity.renderfx |= RF_THIRD_PERSON;
	}
}



/*
==================
CG_LaunchGib
==================
*/
void CG_LaunchGib( vec3_t origin, vec3_t velocity, qhandle_t hModel ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity( 3000 + (rand() & 4095) );
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;

	VectorCopy( origin, re->origin );
	AxisCopy( axisDefault, re->axis );
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	VectorCopy( origin, le->pos.trBase );
	VectorCopy( velocity, le->pos.trDelta );
	le->pos.trTime = cg.time;

	le->bounceFactor = 0.6f;
	le->leFlags = LEF_MARK_BLOOD | LEF_SOUND_BLOOD;
}

/*
===================
CG_GibPlayer

Generated a bunch of gibs launching out from the bodies location
===================
*/
#define	GIB_VELOCITY	250
#define	GIB_JUMP		250
void CG_GibPlayer( vec3_t playerOrigin ) {
	vec3_t	origin, velocity;

	if ( !cg_blood.integer ) {
		return;
	}

	VectorCopy( playerOrigin, origin );
	velocity[0] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[1] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	if ( rand() & 1 ) {
		CG_LaunchGib( origin, velocity, cgs.media.gibSkull );
	} else {
		CG_LaunchGib( origin, velocity, cgs.media.gibBrain );
	}

	// allow gibs to be turned off for speed
	if ( !cg_gibs.integer ) {
		return;
	}

	VectorCopy( playerOrigin, origin );
	velocity[0] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[1] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibAbdomen );

	VectorCopy( playerOrigin, origin );
	velocity[0] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[1] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibArm );

	VectorCopy( playerOrigin, origin );
	velocity[0] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[1] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibChest );

	VectorCopy( playerOrigin, origin );
	velocity[0] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[1] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibFist );

	VectorCopy( playerOrigin, origin );
	velocity[0] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[1] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibFoot );

	VectorCopy( playerOrigin, origin );
	velocity[0] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[1] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibForearm );

	VectorCopy( playerOrigin, origin );
	velocity[0] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[1] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibIntestine );

	VectorCopy( playerOrigin, origin );
	velocity[0] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[1] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibLeg );

	VectorCopy( playerOrigin, origin );
	velocity[0] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[1] = Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	velocity[2] = GIB_JUMP + Q_flrand(-1.0f, 1.0f)*GIB_VELOCITY;
	CG_LaunchGib( origin, velocity, cgs.media.gibLeg );
}

/*
==================
CG_LaunchGib
==================
*/
void CG_LaunchExplode( vec3_t origin, vec3_t velocity, qhandle_t hModel ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity( 10000 + Q_flrand(0.0f, 1.0f) * 6000 );
	re = &le->refEntity;

	le->leType = LE_FRAGMENT;

	VectorCopy( origin, re->origin );
	AxisCopy( axisDefault, re->axis );
	re->hModel = hModel;

	le->pos.trType = TR_GRAVITY;
	VectorCopy( origin, le->pos.trBase );
	VectorCopy( velocity, le->pos.trDelta );
	le->pos.trTime = cg.time;

	le->bounceFactor = 0.1f;
	le->leFlags = LEF_SOUND_BRASS;
}

#define	EXP_VELOCITY	100
#define	EXP_JUMP		150
/*
===================
CG_GibPlayer

Generated a bunch of gibs launching out from the bodies location
===================
*/
void CG_BigExplode( vec3_t playerOrigin ) {
	vec3_t	origin, velocity;

	if ( !cg_blood.integer ) {
		return;
	}

	VectorCopy( playerOrigin, origin );
	velocity[0] = Q_flrand(-1.0f, 1.0f)*EXP_VELOCITY;
	velocity[1] = Q_flrand(-1.0f, 1.0f)*EXP_VELOCITY;
	velocity[2] = EXP_JUMP + Q_flrand(-1.0f, 1.0f)*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = Q_flrand(-1.0f, 1.0f)*EXP_VELOCITY;
	velocity[1] = Q_flrand(-1.0f, 1.0f)*EXP_VELOCITY;
	velocity[2] = EXP_JUMP + Q_flrand(-1.0f, 1.0f)*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = Q_flrand(-1.0f, 1.0f)*EXP_VELOCITY*1.5;
	velocity[1] = Q_flrand(-1.0f, 1.0f)*EXP_VELOCITY*1.5;
	velocity[2] = EXP_JUMP + Q_flrand(-1.0f, 1.0f)*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = Q_flrand(-1.0f, 1.0f)*EXP_VELOCITY*2.0;
	velocity[1] = Q_flrand(-1.0f, 1.0f)*EXP_VELOCITY*2.0;
	velocity[2] = EXP_JUMP + Q_flrand(-1.0f, 1.0f)*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

	VectorCopy( playerOrigin, origin );
	velocity[0] = Q_flrand(-1.0f, 1.0f)*EXP_VELOCITY*2.5;
	velocity[1] = Q_flrand(-1.0f, 1.0f)*EXP_VELOCITY*2.5;
	velocity[2] = EXP_JUMP + Q_flrand(-1.0f, 1.0f)*EXP_VELOCITY;
	CG_LaunchExplode( origin, velocity, cgs.media.smoke2 );

}

localEntity_t *CG_MakeLight( const vec3_t origin, int duration, 
				  int fadedelay, const vec4_t color,
				  float startradius, float endradius ) 
{
	localEntity_t *le = CG_AllocLocalEntity( duration );
	VectorCopy( origin, le->pos.trBase);
	le->leType = LE_LIGHT;
	le->fadeRate = (fadedelay < 0) ? 2000 : 1.0 / (duration - fadedelay);
	le->radius = startradius;
	VectorCopy4( color, le->color );
	le->radiusrate = ( endradius - startradius ) / duration;
	return le;
}

/* 
	Gas and Smoke Sprites

*/

#define MAX_SMOKESPRITES 768
#define MAX_GASSPRITES 256
#define MAX_SMOKEPOLYS (MAX_SMOKESPRITES+MAX_GASSPRITES)*4
#define GASLIFE	3400
#define GASFADE	2900
#define GASVELOCITY ((120.f)/GASLIFE)	// units per msec
#define GASRADIUSRATE (((float)(75-4))/GASLIFE)
//#define GASSCALE 1.0003
#define GASBURNLIFE	1000
#define GASBURNFADE	700

typedef struct smokesprite_s {
	struct smokesprite_s *next;
	vec3_t	origin;
	float	radius,radiusrate;
	float	rotation;
	int		starttime,life;
	byte	rgba[4];
} smokesprite_t;

typedef struct gassprite_s {
	struct gassprite_s *next;
	vec3_t	pos;
	vec3_t	dir;
	float	radius;
	int		life;
	centity_t *cent;
} gassprite_t;

static gassprite_t GasSprites[MAX_GASSPRITES];
static gassprite_t * freegassprites;			// List of free gassprites
static gassprite_t * activegassprites;			// List of active smokesprites

static smokesprite_t SmokeSprites[MAX_SMOKESPRITES];
static smokesprite_t * freesmokesprites;			// List of free smokesprites
static smokesprite_t * activesmokesprites;			// List of active smokesprites

static polyVert_t SmokePolyPool[MAX_SMOKEPOLYS];
static int smokepolys;

void CG_InitSmokeSprites( void ) {
	int i;

	memset( &GasSprites, 0, sizeof(GasSprites) );
	for( i = 0; i < MAX_GASSPRITES - 1; i++ ) {
		GasSprites[i].next = &GasSprites[i+1];
	}

	freegassprites = &GasSprites[0];
	activegassprites = NULL;

	memset( &SmokeSprites, 0, sizeof(SmokeSprites) );
	for( i = 0; i < MAX_SMOKESPRITES - 1; i++ ) {
		SmokeSprites[i].next = &SmokeSprites[i+1];
	}

	freesmokesprites = &SmokeSprites[0];
	activesmokesprites = NULL;
}

void CG_SpawnSmokeSprite(const vec3_t origin, int life, 
						const vec4_t color, float startradius, float endradius) {

	smokesprite_t *smokesprite = freesmokesprites;
	if (!smokesprite) return;

	freesmokesprites = smokesprite->next;
	smokesprite->next = activesmokesprites;
	activesmokesprites = smokesprite;

	VectorCopy( origin, smokesprite->origin );
	smokesprite->starttime = cg.time;
	smokesprite->life = life;
	smokesprite->rgba[0] = color[0]*255;
	smokesprite->rgba[1] = color[1]*255;
	smokesprite->rgba[2] = color[2]*255;
	smokesprite->rgba[3] = color[3]*255;

	smokesprite->radius = startradius;
	smokesprite->radiusrate = (endradius - startradius) / life;

	smokesprite->rotation = Q_flrand(0.0f, 1.0f) * 360;
};

static qboolean CG_GasSpritePhysics( gassprite_t *gassprite, const int time ) {
	trace_t tr;
	vec3_t oldpos;

	VectorCopy( gassprite->pos, oldpos );

	if (gassprite->cent) {
		VectorMA( oldpos, GASVELOCITY*time, gassprite->dir, gassprite->pos );
		gassprite->life += time;
		gassprite->radius += GASRADIUSRATE * time;
//		gassprite->size *= pow(GASSCALE,time);
		if (gassprite->life > GASLIFE) return qfalse;
	} else {
		VectorMA( oldpos, 1.3*GASVELOCITY*time, gassprite->dir, gassprite->pos );
		gassprite->life += time;
		gassprite->radius += 1.3f * GASRADIUSRATE * time;
		if (gassprite->life > GASBURNLIFE) return qfalse;
	}

	CG_Trace( &tr, oldpos, NULL, NULL, gassprite->pos, -1, CONTENTS_SOLID );

	/* Check if we hit something */
	if( tr.fraction != 1.f ) {
		/* If it's still a very young gas just remove it */
		if (gassprite->life < 100) return qfalse;
		VectorCopy( tr.endpos, gassprite->pos );
	}
	return qtrue;
}

qboolean CG_SpawnGasSprite( centity_t * cent,float fraction) {
	/* Alloc a new sprite and return if no free ones */
	float angle;
	gassprite_t *gassprite = freegassprites;
	if (!gassprite) return qfalse;

	freegassprites = gassprite->next;
	gassprite->next=activegassprites;
	activegassprites=gassprite;

	/* Init the sprite values */
	VectorCopy( cent->lerpOrigin, gassprite->pos );
	gassprite->pos[2] += 4;
	/* Determine the angle of the vectors */
	angle=(cg.time/90)*((70.42*2*M_PI)/360.0f);
	angle+=fraction*2*M_PI;
	gassprite->dir[0]=cos(angle);
	gassprite->dir[1]=sin(angle);
	angle=(cg.time/90)*((31.12f*2*M_PI)/360.0f);
	angle+=fraction*2*M_PI;
	//Minumum of -0.15 downwards
	gassprite->dir[2]=0.25f+(sin(angle)/2.5);
	VectorNormalize(gassprite->dir);
	gassprite->radius = 4.f;
	gassprite->life = 0;
	gassprite->cent = cent;
	if( !CG_GasSpritePhysics( gassprite, fraction*GASLIFE ) ) {
		/* Remove sprite from active list and back into free list */
		activegassprites=gassprite->next;
		gassprite->next=freegassprites;
		freegassprites=gassprite;
		return( qfalse );
	} 
	gassprite->cent->muzzleFlashTime++;
	return( qtrue );
}

void CG_BurnGasSprites( centity_t * cent) {
	gassprite_t *gassprite;int count;
	float fraction,fraction_add;
	static vec4_t GasBurnLight ={1.0, 0.2, 0.2, 1};

	if( !cgs.media.sfx_napalmExplode )
		cgs.media.sfx_napalmExplode = trap_S_RegisterSound( "sound/weapons/q3f_napalm_start.wav", qfalse );
	trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_AUTO, cgs.media.sfx_napalmExplode );

	if (cent->muzzleFlashTime<35) {
		count=35 - cent->muzzleFlashTime;
		fraction_add=1.0/count;
		fraction=0;
		while (count--) {
			CG_SpawnGasSprite(cent,fraction);
			fraction+=fraction_add;
		}
	}
	/* Go through list of gassprite and check for this entities ones and burn those */
	for (gassprite=activegassprites;gassprite;gassprite=gassprite->next) {
		if (gassprite->cent!=cent) continue;
		gassprite->cent = 0;		//Signal this sprite as having flames
		gassprite->life = rand()%300;
	}
	/* Add a light */
	CG_MakeLight(cent->lerpOrigin, GASBURNLIFE, GASBURNLIFE/2, GasBurnLight, 300, 200);
}

static void RenderSmokeSprite(const vec3_t origin, const float radius, const float rotation, int rgba, qhandle_t shader) {
	polyVert_t * verts;
	vec3_t top, bottom;
	vec3_t right, up, tmp;

	if (smokepolys >= MAX_SMOKEPOLYS) return;
	verts = &SmokePolyPool[smokepolys];
	smokepolys+=4;

	VectorCopy( cg.refdef_current->viewaxis[1], tmp );
	RotatePointAroundVector( right, cg.refdef_current->viewaxis[0], tmp, rotation );
	CrossProduct( cg.refdef_current->viewaxis[0], right, up );

	VectorMA( origin, radius, up, top );
	VectorMA( origin, -radius, up, bottom );

	VectorMA( top, radius, right, verts[0].xyz );
	verts[0].st[0] = 1;
	verts[0].st[1] = 0;
	*(int *)&verts[0].modulate = rgba;

	VectorMA( top, -radius, right, verts[1].xyz );
	verts[1].st[0] = 0;
	verts[1].st[1] = 0;
	*(int *)&verts[1].modulate = rgba;
	
	VectorMA( bottom, -radius, right, verts[2].xyz );
	verts[2].st[0] = 0;
	verts[2].st[1] = 1;
	*(int *)&verts[2].modulate = rgba;

	VectorMA( bottom, radius, right, verts[3].xyz );
	verts[3].st[0] = 1;
	verts[3].st[1] = 1;
	*(int *)&verts[3].modulate = rgba;

	if (shader != cgs.media.smokePuffShader) {
		trap_R_AddPolyToScene( shader, 4, verts );
		smokepolys -= 4;
	}
}

void CG_AddSmokeSprites( void ) {
	gassprite_t *gassprite;
	gassprite_t * * wheregassprite;
	smokesprite_t *smokesprite;
	smokesprite_t * * wheresmokesprite;
	
	float radius;byte color[4];
	int index;

	/* Render/Move the gas sprites */
	gassprite = activegassprites;
	wheregassprite=&activegassprites;
	while( gassprite ) {
		// Do physics
		if( !CG_GasSpritePhysics( gassprite, cg.frametime ) ) {
			/* Clear up this gas sprite */
			if (gassprite->cent) 
				gassprite->cent->muzzleFlashTime--;
			*wheregassprite=gassprite->next;
			gassprite->next=freegassprites;
			freegassprites=gassprite;
			gassprite=*wheregassprite;
			continue;
		}
		if (gassprite->cent) {
skipflames:
			color[0] = 0.1 * 0xff;
			color[1] = 0.6 * 0xff;
			color[2] = 0.2 * 0xff;
			// fadeout
			if( gassprite->life > (GASFADE) ) {
				color[3] = (0.83 -  0.83 * ((float)(gassprite->life - GASFADE)/(float)(GASLIFE-GASFADE))) * 0xff;
			} else {
				color[3] = 0.83 * 0xff;
			}
			RenderSmokeSprite(gassprite->pos, gassprite->radius, 0, *(int*)&color, cgs.media.smokePuffShader );
		} else {
			if (gassprite->life < 300) goto skipflames;
			color[0] = 0xff;
			color[1] = 0xff;
			color[2] = 0xff;
			// fadeout
			if( gassprite->life > (GASBURNFADE) ) {
				color[3] = (0.75 -  0.75 * ((float)(gassprite->life - GASBURNFADE)/(float)(GASBURNLIFE-GASBURNFADE))) * 0xff;
			} else {
				color[3] = 0.75 * 0xff;
			}
			index = NUM_FLAME_SPRITES*((float)(gassprite->life-300)/(float)(GASBURNLIFE-300));
			if (index>=NUM_FLAME_SPRITES) index=NUM_FLAME_SPRITES-1;
			RenderSmokeSprite(gassprite->pos, gassprite->radius, 0, *(int*)&color, cgs.media.flameShaders[index] );
		}
		wheregassprite=&gassprite->next;
		gassprite = gassprite->next;
	}
	/* Render the smoke sprites */
	smokesprite = activesmokesprites;
	wheresmokesprite=&activesmokesprites;
	while( smokesprite ) {
		int lifedone = cg.time - smokesprite->starttime;
		if( lifedone >= smokesprite->life ) {
freesmokesprite:
			*wheresmokesprite=smokesprite->next;
			smokesprite->next=freesmokesprites;
			freesmokesprites=smokesprite;
			smokesprite=*wheresmokesprite;
			continue;
		}
		radius = smokesprite->radius + smokesprite->radiusrate * lifedone;
		if ( 2*radius*radius > DistanceSquared(smokesprite->origin, cg.refdef.vieworg ))
			goto freesmokesprite;

		color[0] = smokesprite->rgba[0];
		color[1] = smokesprite->rgba[1];
		color[2] = smokesprite->rgba[2];
		color[3] = smokesprite->rgba[3] - (smokesprite->rgba[3] * lifedone) / smokesprite->life;
		RenderSmokeSprite(smokesprite->origin, radius, smokesprite->rotation, *(int*)&color, cgs.media.smokePuffShader );	

		wheresmokesprite=&smokesprite->next;
		smokesprite = smokesprite->next;
	}

	trap_R_AddPolysToScene( cgs.media.smokePuffShader, 4, SmokePolyPool, smokepolys >> 2 );
	smokepolys = 0;
}
