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
**	cg_q3f_spirit.c
**
**	Particle engine. Spawns particles, executes physics on them, the usual blabla
*/

#ifdef CGAME
#include "cg_local.h"
#endif

#ifndef CGAME
#include "../ui_new/ui_local.h"
#endif

#define MAX_SCRIPT		128
#define MAX_SYSTEM		256
#define MAX_STANDALONE	64
#define MAX_PARTICLES	2048
#define MAX_GROUPS		256
#define GROUP_HASHSIZE	256
#define GROUP_HASHMASK	(GROUP_HASHSIZE-1)
#define GROUP_HASH(_KEY,_SYSTEM) ((( _KEY >> 5 ) | ((int)_SYSTEM)) & GROUP_HASHMASK)

#define SPIRIT_ACCEL				0x1
#define	SPIRIT_ALIGN				0x2
#define	SPIRIT_ANGLES				0x4
#define SPIRIT_ANIMATEDSHADER		0x8
#define SPIRIT_AMOVE				0x10
#define	SPIRIT_COLLISION			0x40
#define	SPIRIT_RGBRANGE				0x80
#define	SPIRIT_CUSTOMRGB			0x100
#define	SPIRIT_CUSTOMSHADER			0x200
#define	SPIRIT_DIETOUCH				0x1000
#define	SPIRIT_DIRECTION			0x2000
#define	SPIRIT_FADEOUT				0x10000
#define	SPIRIT_FADEIN				0x20000
#define SPIRIT_FRICTION				0x40000
#define SPIRIT_MOVE					0x80000
#define SPIRIT_DISTANCE				0x100000
#define SPIRIT_ROTATE				0x200000
#define SPIRIT_GROUPED				0x400000
#define SPIRIT_SPAWNRANGE			0x800000
#define SPIRIT_OFFSET				0x1000000
#define SPIRIT_WORLDLIGHTCOLOURED	0x20000000

typedef enum {
	SPIRIT_TLIGHT,
	SPIRIT_TSPRITE,
	SPIRIT_TMODEL,
	SPIRIT_TTRACER,
} SystemType_t;

typedef enum {
	SPIRIT_WAVE_LINEAR = 0,
	SPIRIT_WAVE_SIN,
	SPIRIT_WAVE_TRIANGLE
} WaveType_t;

typedef struct rnd_val_s {
	float base;
	float range;
} rnd_val_t;

typedef enum {
	COL_SOLID,
	COL_WATER
} col_t;

typedef enum {
	GROUP_DIE,
	GROUP_UNLINK,
	GROUP_WAIT,
} group_t;

typedef struct {
	WaveType_t type;
	float speed;
	rnd_val_t index;
} SpiritWave_t;

typedef struct SpiritSystem_s {
	//Runtime related variables
	struct SpiritSystem_s *next;
	SystemType_t type;
	group_t		group;
	int			flags;
	vec3_t		accel;
	float		bouncefactor;
	int			collisionmask;
	vec3_t		rgb[2];
	float		alpha;
	float		fadeout;
	float		fadein;
	float		friction;
	int			life;
	qhandle_t	model;
	qhandle_t	shader;
	float		size;
	qhandle_t	*animshaders;
	int			animnumframes;
	float		animframerate;
	float		rotate[3];
	float		cullrange;

	//Spawn related variables
	int			count;					// number of particles to be spawned.

	SpiritWave_t scalewave;
	SpiritWave_t rgbwave;
	SpiritWave_t alphawave;

	rnd_val_t	radius[2];
	rnd_val_t	angles[3];
	rnd_val_t	avelocity[3];

	rnd_val_t	direction[3];
	rnd_val_t	offset[3];
	rnd_val_t	distance;

	rnd_val_t	velocity;
	rnd_val_t	delay;

	int			spawndelay;
	int			spawnlast;
	int			spawntime;
	float		spawnrange;
	float		spawnlerp,spawnlerp_add;
} SpiritSystem_t;

static SpiritScript_t SpiritScripts[MAX_SCRIPT];
static SpiritSystem_t SpiritSystems[MAX_SYSTEM];
static int SpiritScriptCount = 0;
static int SpiritScriptDepth = 0;
static int SpiritSystemCount = 0;
static int SpiritSystemDepth = 0;
static refdef_t *SpiritRefDef = NULL;
static vec3_t SpiritCustomColor;
static qhandle_t SpiritCustomShader = 0;

typedef struct StandAloneScript_s {
	SpiritScript_t *SpiritScript;
	vec3_t axis[3];
	vec3_t origin;
} StandAloneScript_t;

static StandAloneScript_t StandAloneScripts[MAX_STANDALONE];
static int StandAloneCount = 0;

typedef struct pgroup_s {
	vec3_t			origin;
	vec3_t			axis[3];
	int				key;
	const SpiritSystem_t	*system;
	int				count;
	qboolean		updated;
	struct pgroup_s *next;
	struct pgroup_s *hashnext,**hashstorage;
} pgroup_t;

typedef struct particle_s {
	const SpiritSystem_t	*SpiritSystem;
	struct particle_s		*next;
	struct pgroup_s			*pgroup;
	
	vec3_t		pos;
	vec3_t		delta;
	vec3_t		oldpos;

	vec3_t		angles;
	vec3_t		avelocity;

	int			lastruntime;
	int			starttime;
	int			endtime;
	
	float		radiusbase, radiusdelta;
	float		scalewave_index;

	vec3_t		rgb;
	qhandle_t	shader;
} particle_t;

static particle_t SpiritParticles[MAX_PARTICLES];
static particle_t *freeparticles;				// pointer free particle list
static particle_t *usedparticles;				// pointer to use particle list

static pgroup_t	SpiritGroups[MAX_GROUPS];
static pgroup_t *SpiritGroupHash[GROUP_HASHSIZE];
static pgroup_t *freegroups;
static pgroup_t *usedgroups;

static int SpiritRunTime = 0;

////////////////////////////////////////////////////////
// Misc Functions

static QINLINE float Spirit_Random( const rnd_val_t *rnd ) {
	if (!rnd->range) return rnd->base;
	else if (rnd->range>0) return rnd->base + rnd->range*Q_flrand(0.0f, 1.0f);
	else return ((int)((rand()&2)-1)) * (rnd->base - rnd->range*Q_flrand(0.0f, 1.0f));
}

static QINLINE float Spirit_GetWave(const SpiritWave_t * wave, const float fraction, const float index ) {
	float x;
	switch (wave->type) {
	case SPIRIT_WAVE_LINEAR:
		return fraction;
	case SPIRIT_WAVE_SIN:
//		return sin(fraction * wave->speed);
		return 0.5 + 0.5*sin(index + fraction * wave->speed);
	case SPIRIT_WAVE_TRIANGLE:
		x = fmod(index + fraction * wave->speed, 2 );
		return x < 1 ? x : 2 - x;
	}
	return 0;
}

void Spirit_SetCustomColor( const vec3_t color ) {
	VectorCopy( color, SpiritCustomColor );
}

void Spirit_SetCustomShader( const qhandle_t shader ) {
	SpiritCustomShader = shader;
}


////////////////////////////////////////////////////////
// Spirit Particle System

void InitParticles( void ) {
	int i;

	memset( &SpiritParticles, 0, sizeof(SpiritParticles) );
	for( i = 0; i < MAX_PARTICLES - 1; i++ ) {
		SpiritParticles[i].next = &SpiritParticles[i+1];
	}
	freeparticles = SpiritParticles;
	usedparticles = NULL;

	memset( &SpiritGroups, 0, sizeof(SpiritGroups) );
	memset( &SpiritGroupHash, 0, sizeof(SpiritGroupHash) );
	for( i = 0; i < MAX_GROUPS - 1; i++ ) {
		SpiritGroups[i].next = &SpiritGroups[i+1];
	}
	freegroups = SpiritGroups;
	usedgroups = NULL;
}

static particle_t *AllocParticle( void ) {
	particle_t *alloc;

	if( !freeparticles )
		return( NULL );

	alloc = freeparticles;
	freeparticles = freeparticles->next;

	memset( alloc, 0, sizeof(particle_t) );
	alloc->next = usedparticles;
	usedparticles = alloc; 

	return( alloc );
}

static void SpawnParticle( const SpiritSystem_t *SpiritSystem, pgroup_t *pgroup,const vec3_t origin,const vec3_t axis[3], int attime ) {
	int i;
	float velocity, delay;
	particle_t * particle = AllocParticle();

	if (!particle) return;
	/* Init the particle basics */
	particle->SpiritSystem = SpiritSystem;
	particle->pgroup = pgroup;

	if ( SpiritSystem->flags & SPIRIT_CUSTOMSHADER) {
		particle->shader = SpiritCustomShader;
	} else {
		particle->shader = SpiritSystem->shader;
	}

	if ( SpiritSystem->flags & SPIRIT_CUSTOMRGB) {
		VectorCopy(SpiritCustomColor, particle->rgb);
	} else {
		VectorCopy(SpiritSystem->rgb[0], particle->rgb);
	}

	VectorCopy( origin, particle->pos );

	if( SpiritSystem->flags & SPIRIT_OFFSET ) {
		for ( i = 0; i < 3; i++ ) {
			float len = Spirit_Random( &SpiritSystem->offset[i] );
			VectorMA( particle->pos, len, axis[i], particle->pos );
		}
	}

	VectorCopy( particle->pos, particle->oldpos );

	if (SpiritSystem->flags & SPIRIT_DIRECTION ) {
		vec3_t direction;

		direction[0] = Spirit_Random( &SpiritSystem->direction[0] );
		direction[1] = Spirit_Random( &SpiritSystem->direction[1] );
		direction[2] = Spirit_Random( &SpiritSystem->direction[2] );

		particle->delta[0] = axis[0][0]*direction[0] + axis[1][0]*direction[1] + axis[2][0]*direction[2];
		particle->delta[1] = axis[0][1]*direction[0] + axis[1][1]*direction[1] + axis[2][1]*direction[2];
		particle->delta[2] = axis[0][2]*direction[0] + axis[1][2]*direction[1] + axis[2][2]*direction[2];
		VectorNormalizeFast( particle->delta );
	} else {
		VectorCopy( axis[2], particle->delta );
	}
	if (SpiritSystem->flags & SPIRIT_DISTANCE) {
		float distance = Spirit_Random( &SpiritSystem->distance );
		VectorMA(particle->pos, distance, particle->delta, particle->pos );
	}

	velocity = Spirit_Random( &SpiritSystem->velocity );
	VectorScale( particle->delta, velocity , particle->delta );

	// set angles
	if( SpiritSystem->flags & SPIRIT_ANGLES ) {
		for( i = 0; i < 3; i++ ) {
			particle->angles[i] += Spirit_Random( &SpiritSystem->angles[i] );
		}
	// align particle along delta
	} else if( SpiritSystem->flags & SPIRIT_ALIGN ) {
		vectoangles( particle->delta, particle->angles );
	} else {
		vectoangles( axis[0], particle->angles );
	}

	// set avelocity
	if( SpiritSystem->flags & SPIRIT_AMOVE ) {
		for( i = 0; i < 3; i++ ) {
			particle->avelocity[i] = Spirit_Random( &SpiritSystem->avelocity[i] );
		}
	}

	delay = Spirit_Random( &SpiritSystem->delay );
	attime += (int)(delay * 1000);

	/* Life related init */
	particle->starttime = attime;
	particle->lastruntime = attime;
	particle->endtime = attime + SpiritSystem->life;

	// Radius related init
	particle->radiusbase = Spirit_Random( &SpiritSystem->radius[0] );
	particle->radiusdelta = Spirit_Random( &SpiritSystem->radius[1] );
	
	particle->scalewave_index = Spirit_Random( &SpiritSystem->scalewave.index);

}

int Particle_Count( ) {
	int count = 0;
	particle_t * particle = usedparticles;
	while (particle) {
		particle = particle->next;
		count++;
	}
	return count;
}

static float Spirit_GetState( particle_t *particle, vec4_t rgba ) {
	float fraction = (float)(cg.time - particle->starttime ) / ( particle->endtime - particle->starttime );

	// Calculate color effects
	if( particle->SpiritSystem->flags & SPIRIT_RGBRANGE )	{
		rgba[0] = particle->SpiritSystem->rgb[0][0] + fraction * particle->SpiritSystem->rgb[1][0];
		rgba[1] = particle->SpiritSystem->rgb[0][1] + fraction * particle->SpiritSystem->rgb[1][1];
		rgba[2] = particle->SpiritSystem->rgb[0][2] + fraction * particle->SpiritSystem->rgb[1][2];
	} else {
		VectorCopy( particle->rgb, rgba );
	}

	if( fraction < particle->SpiritSystem->fadein ) {
		rgba[3] = (fraction/particle->SpiritSystem->fadein) * particle->SpiritSystem->alpha;
	} else if ( fraction > particle->SpiritSystem->fadeout ) {
		rgba[3] = (1 - fraction)/(1 - particle->SpiritSystem->fadeout ) * particle->SpiritSystem->alpha;
	} else rgba[3] = particle->SpiritSystem->alpha;

	return particle->radiusbase + particle->radiusdelta * Spirit_GetWave(&particle->SpiritSystem->scalewave, fraction, particle->scalewave_index );
}

// Spirit_RunPhysics : calculates the physics for a particle and returns qfalse if it should die
static qboolean Spirit_RunParticle( particle_t *particle, float runTime) {
	vec3_t delta;
	trace_t trace;

	if (particle->pgroup && !particle->pgroup->updated) {
		switch ( particle->SpiritSystem->group ) {
		case GROUP_DIE:
			return qfalse;
		case GROUP_UNLINK:
			particle->pgroup = NULL;
			break;
		case GROUP_WAIT:
			break;
		}
	}
	// save the pos
	VectorCopy( particle->pos, particle->oldpos );

	// move the particles
	if( particle->SpiritSystem->flags & (SPIRIT_MOVE|SPIRIT_ACCEL) ) {
		VectorScale( particle->delta, runTime, delta);
		VectorMA( delta, runTime*runTime, particle->SpiritSystem->accel, delta );
		VectorAdd( particle->pos, delta, particle->pos );
	}

	// align particle along delta
	if( particle->SpiritSystem->flags & SPIRIT_ALIGN ) {
		vectoangles( particle->delta, particle->angles );
	}

	// update angles based on avelocity
	if( particle->SpiritSystem->flags & SPIRIT_AMOVE ) {
		VectorMA( particle->angles, runTime, particle->avelocity, particle->angles );
	}
	
	if ( particle->SpiritSystem->flags & SPIRIT_ROTATE ) {
		RotatePointAroundVertex(particle->delta,
			particle->SpiritSystem->rotate[0] * runTime,
			particle->SpiritSystem->rotate[1] * runTime,
			particle->SpiritSystem->rotate[2] * runTime,
			vec3_origin
		);
		RotatePointAroundVertex(particle->pos,
			particle->SpiritSystem->rotate[0] * runTime,
			particle->SpiritSystem->rotate[1] * runTime,
			particle->SpiritSystem->rotate[2] * runTime,
			vec3_origin
		);
	}

	if( particle->SpiritSystem->flags & SPIRIT_ACCEL ) {
		VectorMA( particle->delta, runTime, particle->SpiritSystem->accel, particle->delta );
	}

	// modify delta
	if( particle->SpiritSystem->flags & SPIRIT_FRICTION ) {
		VectorMA( particle->delta, runTime * particle->SpiritSystem->friction, particle->delta, particle->delta );
	}

	// Check for collisions
	if( particle->SpiritSystem->flags & SPIRIT_COLLISION ) {
		CG_Trace( &trace, particle->oldpos, NULL, NULL, particle->pos, -1, particle->SpiritSystem->collisionmask );

		if( trace.fraction != 1 ) {
			float dot;
			if( particle->SpiritSystem->flags & SPIRIT_DIETOUCH || trace.surfaceFlags & SURF_NOIMPACT ) {
				return( qfalse );
			}

			VectorCopy( trace.endpos, particle->pos );
			if( particle->SpiritSystem->flags & SPIRIT_ACCEL ) {
				VectorMA( particle->delta, runTime, particle->SpiritSystem->accel, particle->delta );
			}
			dot = DotProduct( particle->delta, trace.plane.normal );
			VectorMA( particle->delta, -2*dot, trace.plane.normal, particle->delta );
			VectorScale( particle->delta, particle->SpiritSystem->bouncefactor, particle->delta );
		}
	}
	
	// Calculate shader if animated

	if( particle->SpiritSystem->flags & SPIRIT_ANIMATEDSHADER ) {
		particle->shader = particle->SpiritSystem->animshaders[((int)((cg.time - particle->starttime) * particle->SpiritSystem->animframerate)) % particle->SpiritSystem->animnumframes];
	}

	return( qtrue );
}


void Spirit_PrepareFrame( void ) {
	float frameTimeMul = 1.0 / (cg.time - cg.oldTime);
	int spiritnum;

	for( spiritnum = 0; spiritnum < SpiritSystemCount; spiritnum++ ) {
		int passedtime;
		SpiritSystem_t *SpiritSystem = &SpiritSystems[spiritnum];

		if (!SpiritSystem->spawndelay) {
			SpiritSystem->spawntime = cg.time;
			continue;
		}

		passedtime = cg.time - SpiritSystem->spawnlast;
		SpiritSystem->count = passedtime/SpiritSystem->spawndelay;
		if (!SpiritSystem->count) 
			continue;

		passedtime = SpiritSystem->spawnlast - cg.oldTime + SpiritSystem->spawndelay;
		SpiritSystem->spawntime = SpiritSystem->spawnlast + SpiritSystem->spawndelay;
		SpiritSystem->spawnlerp = passedtime * frameTimeMul;
		SpiritSystem->spawnlerp_add = SpiritSystem->spawndelay * frameTimeMul;
		SpiritSystem->spawnlast += SpiritSystem->count * SpiritSystem->spawndelay;
	}

	/* Spawn particles from the standalone system */
	for( spiritnum = 0; spiritnum < StandAloneCount; spiritnum++ ) {
		StandAloneScript_t *StandAlone = &StandAloneScripts[spiritnum];
		Spirit_RunScript( StandAlone->SpiritScript, StandAlone->origin, StandAlone->origin, StandAlone->axis, 0);
	}
}

static void Spirit_DrawParticle(particle_t * particle ) {
	polyVert_t verts[4];
	vec3_t top, bottom;
	vec3_t right, up;
	vec3_t forward;
	vec3_t pos,oldpos;
	float size;
	refEntity_t	re;
	vec4_t colors;
	byte rgba[4];

	float radius = Spirit_GetState( particle, colors ); 
	rgba[0] = colors[0]*255;
	rgba[1] = colors[1]*255;
	rgba[2] = colors[2]*255;
	rgba[3] = colors[3]*255;

	if (!particle->pgroup) {
		VectorCopy( particle->pos, pos);
		VectorCopy( particle->oldpos, oldpos);
	} else {
		pgroup_t *pgroup = particle->pgroup;
		pgroup->count++;
		VectorMA( pgroup->origin, particle->pos[0], pgroup->axis[0], pos );
		VectorMA( pos, particle->pos[1], pgroup->axis[1], pos );
		VectorMA( pos, particle->pos[2], pgroup->axis[2], pos );

		VectorMA( pgroup->origin, particle->oldpos[0], pgroup->axis[0], oldpos );
		VectorMA( oldpos, particle->oldpos[1], pgroup->axis[1], oldpos );
		VectorMA( oldpos, particle->oldpos[2], pgroup->axis[2], oldpos );
	}

	if (particle->SpiritSystem->cullrange > 0) {
		if (DistanceSquared(pos, SpiritRefDef->vieworg) <= 
			particle->SpiritSystem->cullrange ) 
			return;
	}

	switch (particle->SpiritSystem->type) {
	case SPIRIT_TLIGHT:
		trap_R_AddLightToScene( pos, 
			radius,
			colors[3] * particle->SpiritSystem->size,
			colors[0],
			colors[1],
			colors[2],
			particle->shader,
			0 );
		break;
	case SPIRIT_TSPRITE:
		if( particle->SpiritSystem->flags & (SPIRIT_ALIGN|SPIRIT_ANGLES|SPIRIT_AMOVE ) ) {
			vec3_t tmp;
			VectorCopy( SpiritRefDef->viewaxis[1], tmp );
			RotatePointAroundVector( right, SpiritRefDef->viewaxis[0], tmp, particle->angles[2] );
			CrossProduct( SpiritRefDef->viewaxis[0], right, up );
		} else {
			VectorCopy( SpiritRefDef->viewaxis[1], right );
			VectorCopy( SpiritRefDef->viewaxis[2], up );
		}

		VectorMA( pos, radius, up, top );
		VectorMA( pos, -radius, up, bottom );
			
		VectorMA( top, radius, right, verts[0].xyz );
		verts[0].st[0] = 0;
		verts[0].st[1] = 0;
		*(int*)&verts[0].modulate = *(int*)&rgba;

		VectorMA( top, -radius, right, verts[1].xyz );
		verts[1].st[0] = 1;
		verts[1].st[1] = 0;
		*(int*)&verts[1].modulate = *(int*)&rgba;

		VectorMA( bottom, -radius, right, verts[2].xyz );
		verts[2].st[0] = 1;
		verts[2].st[1] = 1;
		*(int*)&verts[2].modulate = *(int*)&rgba;

		VectorMA( bottom, radius, right, verts[3].xyz );
		verts[3].st[0] = 0;
		verts[3].st[1] = 1;
		*(int*)&verts[3].modulate = *(int*)&rgba;

		if( particle->SpiritSystem->flags & SPIRIT_WORLDLIGHTCOLOURED ) {
			CG_LightVerts( SpiritRefDef->viewaxis[0], 4, verts, qtrue );
		}

		trap_R_AddPolyToScene( particle->shader, 4, verts );
		break;
	case SPIRIT_TTRACER:
		/* Determine endpoint and direction of the tracer */
		VectorSubtract( pos, oldpos, forward );
		VectorNormalize( forward );
		VectorMA( pos, radius, forward, bottom );
	
		size = particle->SpiritSystem->size;

		GetPerpendicularViewVector( cg.refdef_current->vieworg, pos, bottom, right );

		VectorMA( pos, size, right, verts[0].xyz );
		verts[0].st[0] = 1;
		verts[0].st[1] = 0;
		*(int*)&verts[0].modulate = *(int*)&rgba;

		VectorMA( pos, -size, right, verts[1].xyz );
		verts[1].st[0] = 0;
		verts[1].st[1] = 0;
		*(int*)&verts[1].modulate = *(int*)&rgba;

		VectorMA( bottom, -size, right, verts[2].xyz );
		verts[2].st[0] = 0;
		verts[2].st[1] = 1;
		*(int*)&verts[2].modulate = *(int*)&rgba;

		VectorMA( bottom, size, right, verts[3].xyz );
		verts[3].st[0] = 1;
		verts[3].st[1] = 1;
		*(int*)&verts[3].modulate = *(int*)&rgba;

		if( particle->SpiritSystem->flags & SPIRIT_WORLDLIGHTCOLOURED ) {
			CG_LightVerts( SpiritRefDef->viewaxis[0], 4, verts, qtrue );
		}

		trap_R_AddPolyToScene( particle->shader, 4, verts );
		break;
	case SPIRIT_TMODEL:
		memset( &re, 0, sizeof(re) );

		re.reType = RT_MODEL;
		re.hModel = particle->SpiritSystem->model;

		VectorCopy( pos, re.origin);
		VectorCopy( oldpos, re.oldorigin);


		if( particle->SpiritSystem->flags & (SPIRIT_ALIGN|SPIRIT_ANGLES|SPIRIT_AMOVE ) ) {
			// Update the axis
			AnglesToAxis( particle->angles, re.axis );
		} else {
			AxisCopy( SpiritRefDef->viewaxis, re.axis );
		}
		
		VectorScale( re.axis[0], radius, re.axis[0] );
		VectorScale( re.axis[1], radius, re.axis[1] );
		VectorScale( re.axis[2], radius, re.axis[2] );
		re.nonNormalizedAxes = qtrue;

		*(int*)&re.shaderRGBA = *(int*)&rgba;
		re.customShader = particle->shader;
		re.shaderTime = 0;

		trap_R_AddRefEntityToScene(&re, NULL);
		break;
	}		//End of case

}

void Spirit_AddParticles( void ) {
	pgroup_t *pgroup;
	pgroup_t **pgroupstorage;
	particle_t *particle = usedparticles;
	particle_t **particlestorage = &usedparticles;

	SpiritRefDef = cg.currentrefdef;

	while( particle ) {
		float runTime;
		if ( particle->starttime > cg.time ) {
			particle = particle->next;
			continue;
		}
		if ( cg.time > particle->endtime ) {
			*particlestorage = particle->next;
			particle->next = freeparticles;
			freeparticles = particle;
			particle = *particlestorage;
			continue;
		}

		runTime = 0.001 * (cg.time - particle->lastruntime);
		particle->lastruntime = cg.time;
		if (runTime > 0 && !Spirit_RunParticle( particle, runTime) ) {
			*particlestorage = particle->next;
			particle->next = freeparticles;
			freeparticles = particle;
			particle = *particlestorage;
			continue;
		}
		Spirit_DrawParticle( particle );
		particlestorage = &particle->next;
		particle = particle->next;
	}

	pgroup = usedgroups;
	pgroupstorage = &usedgroups;
	while (pgroup) {
		if (!pgroup->count) {
			*pgroup->hashstorage = pgroup->hashnext;
			if (pgroup->hashnext) pgroup->hashnext->hashstorage = pgroup->hashstorage;
			*pgroupstorage = pgroup->next;
			pgroup->next = freegroups;
			freegroups = pgroup;
			pgroup = *pgroupstorage;
			continue;
		}
		pgroup->count = 0;
		pgroup->updated = qfalse;
		pgroup = pgroup->next;
	}
	SpiritRunTime = cg.time;
}

void Spirit_RunSystem( const SpiritSystem_t *SpiritSystem, int key, const vec3_t origin, const vec3_t oldorigin,  const vec3_t axis[3]) {
	vec3_t delta;
	int i, hash;
	pgroup_t *pgroup;

	if (SpiritRunTime >= cg.time ) return;
	VectorSubtract( origin, oldorigin, delta );
	for ( ; SpiritSystem ; SpiritSystem = SpiritSystem->next ) {
//		if ( SpiritSystem->flags & SPIRIT_SPAWNRANGE) {
//			if (DistanceSquared( origin, 
//
//		}
		if ( SpiritSystem->flags & SPIRIT_GROUPED) {
			if (!key) continue;
			hash = GROUP_HASH( key, SpiritSystem );
			pgroup = SpiritGroupHash[hash];
			/* Already have a group */	
			while (pgroup && (pgroup->key != key || pgroup->system != SpiritSystem )) {
				pgroup = pgroup->hashnext;
			}
			/* No group found, create a new one */
			if (!pgroup) {
				if (!freegroups) 
					continue;

				pgroup = freegroups;
				freegroups = freegroups->next;
				pgroup->next = usedgroups;
				usedgroups = pgroup;
				pgroup->hashnext = SpiritGroupHash[hash];
				if (pgroup->hashnext) 
					pgroup->hashnext->hashstorage = &pgroup->hashnext;
				pgroup->hashstorage = &SpiritGroupHash[hash];
				SpiritGroupHash[hash] = pgroup;
				pgroup->key = key;
				pgroup->system = SpiritSystem;
			}
			AxisCopy( axis, pgroup->axis );
			VectorCopy( origin, pgroup->origin );
			pgroup->updated = qtrue;

			for( i = 0; i < SpiritSystem->count; i++ ) 
				SpawnParticle( SpiritSystem, pgroup, vec3_origin, axisDefault, SpiritSystem->spawntime );
		} else {
			int time = SpiritSystem->spawntime;
			float lerp = SpiritSystem->spawnlerp;
			for( i = 0; i < SpiritSystem->count; i++ ) {
				vec3_t lerporigin;
				VectorMA( origin, lerp, delta, lerporigin);
				SpawnParticle( SpiritSystem, NULL, lerporigin, axis, time );
				time += SpiritSystem->spawndelay;
				lerp += SpiritSystem->spawnlerp_add;
			}
		}
	}
}

qboolean Spirit_UpdateScript( const SpiritScript_t *SpiritScript, const vec3_t origin, const vec3_t axis[3], int key ) {
	int hash;
	pgroup_t *pgroup;
	SpiritSystem_t *SpiritSystem;
	qboolean ret = qfalse;

	if (SpiritRunTime >= cg.time )
		return qtrue;

	for ( SpiritSystem = SpiritScript->SpiritSystem ; SpiritSystem ; SpiritSystem = SpiritSystem->next ) {
		if ( !(SpiritSystem->flags & SPIRIT_GROUPED) )
			continue;
		hash = GROUP_HASH( key, SpiritSystem );
		pgroup = SpiritGroupHash[hash];
		/* Find an active group */	
		while (pgroup && (pgroup->key != key || pgroup->system != SpiritSystem )) {
			pgroup = pgroup->hashnext;
		}
		if (!pgroup) continue;
		pgroup->updated = qtrue;
		AxisCopy( axis, pgroup->axis );
		VectorCopy( origin, pgroup->origin );
		ret = qtrue;
	}
	return ret;
}

void Spirit_RunScript( const SpiritScript_t *SpiritScript, const vec3_t origin, const vec3_t oldorigin, const vec3_t axis[3], int key ) {
	if (! SpiritScript ) return;
	if ( SpiritRunTime >= cg.time ) return;
	if ( SpiritScript->sound ) {
		if (SpiritScript->flags & SPIRIT_SCRIPT_LOOPEDSOUND ) {
			trap_S_AddLoopingSound( ENTITYNUM_WORLD, origin, vec3_origin, SpiritScript->sound, 127, 0 );
		} else {
			trap_S_StartSound( origin , ENTITYNUM_WORLD, CHAN_AUTO, SpiritScript->sound );
		}
	}
	if (SpiritScript->link) {
		Spirit_RunScript( SpiritScript->link, origin, oldorigin, axis, key );
	}

	Spirit_RunSystem( SpiritScript->SpiritSystem, key , origin, oldorigin, axis );
}


void Spirit_RunModel( const SpiritScript_t *SpiritScript, const refEntity_t *re, const char * tagname, int key ) {
	orientation_t	lerped;
	vec3_t		origin, oldorigin;
	matrix3_t	axis;

	if (SpiritRunTime >= cg.time ) return;

	/* Determine postion of the tag from model origin */
	trap_R_LerpTag( &lerped, re, tagname, 0 );
	VectorScale( re->axis[0], lerped.origin[0], origin );
	VectorMA( origin, lerped.origin[1], re->axis[1], origin );
	VectorMA( origin, lerped.origin[2], re->axis[2], origin );
	VectorAdd( origin, re->oldorigin, oldorigin );
	VectorAdd( origin, re->origin, origin );
	// had to cast away the const to avoid compiler problems...
	MatrixMultiply( lerped.axis, ((refEntity_t *)re)->axis, axis );
	Spirit_RunScript(SpiritScript, origin, oldorigin, axis, key );
}

qboolean Spirit_UpdateModel( const SpiritScript_t *SpiritScript, const refEntity_t *re, const char * tagname, int key ) {
	orientation_t	lerped;
	vec3_t		origin;
	matrix3_t	axis;

	if (SpiritRunTime >= cg.time ) return qtrue;
	trap_R_LerpTag( &lerped, re, tagname, 0 );
	VectorMA( re->origin, lerped.origin[1], re->axis[1], origin );
	VectorMA( origin, lerped.origin[1], re->axis[1], origin );
	VectorMA( origin, lerped.origin[2], re->axis[2], origin );
	// had to cast away the const to avoid compiler problems...
	MatrixMultiply( lerped.axis, ((refEntity_t *)re)->axis, axis );
	return Spirit_UpdateScript( SpiritScript, origin , axis, key );
}

////////////////////////////////////////////////////////
// Spirit Scripting

/*
===============
Keyword Hash
===============
*/

#define SPIRITKEYWORDHASH_SIZE	512

typedef struct SpiritKeywordHash_s {
	char *keyword;
	qboolean (*func)( SpiritSystem_t *SpiritSystem, int handle );
	struct SpiritKeywordHash_s *next;
} SpiritKeywordHash_t;

static int Spirit_KeywordHash_Key( char *keyword ) {
	int register hash, i;

	hash = 0;
	for (i = 0; keyword[i] != '\0'; i++) {
		if (keyword[i] >= 'A' && keyword[i] <= 'Z')
			hash += (keyword[i] + ('a' - 'A')) * (119 + i);
		else
			hash += keyword[i] * (119 + i);
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20)) & (SPIRITKEYWORDHASH_SIZE-1);
	return hash;
}

static void Spirit_KeywordHash_Add( SpiritKeywordHash_t *table[], SpiritKeywordHash_t *key ) {
	int hash;

	hash = Spirit_KeywordHash_Key( key->keyword );
/*
	if (table[hash]) {
		int collision = qtrue;
	}
*/
	key->next = table[hash];
	table[hash] = key;
}

static SpiritKeywordHash_t *Spirit_KeywordHash_Find( SpiritKeywordHash_t *table[], char *keyword )
{
	SpiritKeywordHash_t *key;
	int hash;

	hash = Spirit_KeywordHash_Key( keyword );
	for ( key = table[hash]; key; key = key->next ) {
		if ( !Q_stricmp( key->keyword, keyword ) )
			return key;
	}
	return NULL;
}

static qboolean SpiritParseRandom(int handle,rnd_val_t *rnd_val) {
	if ( !PC_Float_Parse( handle, &rnd_val->base ) )
		return( qfalse );
	if ( !PC_Float_Parse( handle, &rnd_val->range ) )
		return( qfalse );
    return qtrue;
}

static qboolean SpiritParseWave(int handle , SpiritWave_t *wave) {
	char str[256];

	if ( !PC_String_ParseNoAlloc( handle, str, sizeof(str) ) ) 
		return qfalse;

	if ( !PC_Float_Parse( handle, &wave->speed ))
		return qfalse;

	if (!SpiritParseRandom(handle, &wave->index))
		return qfalse;

	if ( !Q_stricmp(str, "saw" )) {
		wave->type = SPIRIT_WAVE_LINEAR;
	} else if ( !Q_stricmp(str, "sin" )) {
		wave->type = SPIRIT_WAVE_SIN;
		wave->speed *= 2 * M_PI;
		wave->index.base *= 2 * M_PI;
		wave->index.range *= 2 * M_PI;
	} else if ( !Q_stricmp( str, "triangle" )) {
		wave->type = SPIRIT_WAVE_TRIANGLE;
		wave->speed *= 2;
		wave->index.base *= 2;
		wave->index.range *= 2;
	} else return qfalse;

	return qtrue;
}

static qboolean SpiritParse_accel( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->flags |= SPIRIT_ACCEL;
	if ( !PC_Vec_Parse( handle, &SpiritSystem->accel ) ) 
		return( qfalse );
	return( qtrue );
}

static qboolean SpiritParse_align( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->flags |= SPIRIT_ALIGN;
	return( qtrue );
}

static qboolean SpiritParse_alpha( SpiritSystem_t *SpiritSystem, int handle ) {
	if (!PC_Float_Parse(handle, &SpiritSystem->alpha ))
		return qfalse;
	if (!PC_Float_Parse(handle, &SpiritSystem->fadein ))
		return qfalse;
	if (!PC_Float_Parse(handle, &SpiritSystem->fadeout ))
		return qfalse;
	
	return( qtrue );
}


static qboolean SpiritParse_angles( SpiritSystem_t *SpiritSystem, int handle ) {
	int i;
	SpiritSystem->flags |= SPIRIT_ANGLES;
	for (i=0;i<3;i++) 
		if (!SpiritParseRandom(handle,&SpiritSystem->angles[i]))
			return (qfalse);
	return( qtrue );
}

static qboolean SpiritParse_animshaders( SpiritSystem_t *SpiritSystem, int handle ) {
	char str[256];
	int i;
	if ( !PC_Int_Parse( handle, &SpiritSystem->animnumframes ) ) 
		return( qfalse );
	if ( !PC_Float_Parse( handle, &SpiritSystem->animframerate ) ) 
		return( qfalse );
	if ( !SpiritSystem->animnumframes )
		return( qfalse );
	SpiritSystem->animshaders = UI_Alloc( sizeof(qhandle_t) * SpiritSystem->animnumframes );
	for( i = 0; i < SpiritSystem->animnumframes; i++ ) {
		if ( !PC_String_ParseNoAlloc( handle, str, sizeof(str) ) ) 
			return( qfalse );
		SpiritSystem->animshaders[i] = trap_R_RegisterShader( str );
	}
	SpiritSystem->flags |= SPIRIT_ANIMATEDSHADER;
	return( qtrue );
}

static qboolean SpiritParse_avelocity( SpiritSystem_t *SpiritSystem, int handle ) {
	int i;
	SpiritSystem->flags |= SPIRIT_AMOVE;
	for (i = 0 ; i < 3 ; i++ )  
		if (! SpiritParseRandom( handle, &SpiritSystem->avelocity[i] ))
			return ( qfalse );
	return( qtrue );
}

static qboolean SpiritParse_bouncefactor( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->flags |= SPIRIT_COLLISION;
	SpiritSystem->collisionmask |= MASK_PARTICLESOLID;
	if ( !PC_Float_Parse( handle, &SpiritSystem->bouncefactor ) ) 
		return( qfalse );
	return( qtrue );
}

static qboolean SpiritParse_collision( SpiritSystem_t *SpiritSystem, int handle ) {
	int i;
	SpiritSystem->flags |= SPIRIT_COLLISION;
	SpiritSystem->collisionmask = MASK_PARTICLESOLID;
	if ( !PC_Int_Parse( handle, &i ) ) 
		return( qfalse );
	if( i == COL_WATER ) 
		SpiritSystem->collisionmask = ( MASK_PARTICLESOLID | CONTENTS_WATER );
	return( qtrue );
}

static qboolean SpiritParse_rgb( SpiritSystem_t *SpiritSystem, int handle ) {
	if ( !PC_Vec_Parse( handle, &SpiritSystem->rgb[0] ) )
		return( qfalse );
	return( qtrue );
}

static qboolean SpiritParse_rgbrange( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->flags |= SPIRIT_RGBRANGE;
	if ( !PC_Vec_Parse( handle, &SpiritSystem->rgb[0] ) )
		return( qfalse );
	if ( !PC_Vec_Parse( handle, &SpiritSystem->rgb[1] ) )
		return( qfalse );
	VectorSubtract( SpiritSystem->rgb[1], SpiritSystem->rgb[0], SpiritSystem->rgb[1] );
	return( qtrue );
}

static qboolean SpiritParse_count( SpiritSystem_t *SpiritSystem, int handle ) {
	if ( !PC_Int_Parse( handle, &SpiritSystem->count ) )
		return( qfalse );
	return( qtrue );
}

static qboolean SpiritParse_cullrange( SpiritSystem_t *SpiritSystem, int handle ) {
	if ( !PC_Float_Parse( handle, &SpiritSystem->cullrange ) ) 
		return( qfalse );

	SpiritSystem->cullrange *= SpiritSystem->cullrange;	//So i can use vectorsquared
	return ( qtrue );
}


static qboolean SpiritParse_customrgb( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->flags |= SPIRIT_CUSTOMRGB;
	return( qtrue );
}

static qboolean SpiritParse_customshader( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->flags |= SPIRIT_CUSTOMSHADER;
	return( qtrue );
}

static qboolean SpiritParse_delay( SpiritSystem_t *SpiritSystem, int handle ) {
	if ( !SpiritParseRandom(handle, &SpiritSystem->delay ))
		return( qfalse );
	return( qtrue );
}


static qboolean SpiritParse_dietouch( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->flags |= SPIRIT_DIETOUCH;
	return( qtrue );
}

static qboolean SpiritParse_direction( SpiritSystem_t *SpiritSystem, int handle ) {
	int i;
	SpiritSystem->flags |= SPIRIT_MOVE | SPIRIT_DIRECTION;
	for ( i = 0 ; i < 3 ; i++ ) 
		if ( !SpiritParseRandom( handle, &SpiritSystem->direction[i] ))
			return (qfalse);
	return( qtrue );
}

static qboolean SpiritParse_distance( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->flags |= SPIRIT_DISTANCE;
	if ( !SpiritParseRandom( handle, &SpiritSystem->distance ))
		return (qfalse);
	return( qtrue );
}


static qboolean SpiritParse_dlight( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->type = SPIRIT_TLIGHT;
	if ( !PC_Float_Parse( handle, &SpiritSystem->size ) ) {
		return( qfalse );
	}
	if ( !SpiritParseRandom(handle, &SpiritSystem->radius[0] ))
		return( qfalse );
	if ( !SpiritParseRandom(handle, &SpiritSystem->radius[1] ))
		return( qfalse );

	return( qtrue );
}

/*static qboolean SpiritParse_fadein( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->flags |= SPIRIT_FADEIN;
	if ( !PC_Float_Parse( handle, &SpiritSystem->fadein ) ) {
		return( qfalse );
	}
	return( qtrue );
}

static qboolean SpiritParse_fadeout( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->flags |= SPIRIT_FADEOUT;
	if ( !PC_Float_Parse( handle, &SpiritSystem->fadeout ) ) {
		return( qfalse );
	}
	return( qtrue );
}*/

static qboolean SpiritParse_friction( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->flags |= SPIRIT_FRICTION;
	if ( !PC_Float_Parse( handle, &SpiritSystem->friction ) ) {
		return( qfalse );
	}
	return( qtrue );
}

static qboolean SpiritParse_group( SpiritSystem_t *SpiritSystem, int handle ) {
	char str[256];
	SpiritSystem->flags |= SPIRIT_GROUPED;

	if ( !PC_String_ParseNoAlloc( handle, str, 256 ) ) {
		return( qfalse );
	}
	if ( !Q_stricmp( str, "die" ) ) {
		SpiritSystem->group = GROUP_DIE;
	} else if ( !Q_stricmp( str, "unlink" ) ) {
		SpiritSystem->group = GROUP_UNLINK;
	} else if ( !Q_stricmp( str, "wait" ) ) {
		SpiritSystem->group = GROUP_WAIT;
	} else return qfalse;
	return( qtrue );
}

static qboolean SpiritParse_life( SpiritSystem_t *SpiritSystem, int handle ) {
	float life;
	if ( !PC_Float_Parse( handle, &life ) ) {
		return( qfalse );
	}
	SpiritSystem->life = life * 1000;
	return( qtrue );
}

static qboolean SpiritParse_model( SpiritSystem_t *SpiritSystem, int handle ) {
	char str[256];
	if ( !SpiritParseRandom(handle, &SpiritSystem->radius[0] ))
		return( qfalse );
	if ( !SpiritParseRandom(handle, &SpiritSystem->radius[1] ))
		return( qfalse );

	if ( !PC_String_ParseNoAlloc( handle, str, sizeof(str) ) ) {
		return( qfalse );
	}
	SpiritSystem->type = SPIRIT_TMODEL;
	SpiritSystem->model = trap_R_RegisterModel( str );
	//Ensure memory still being in spirit
	SetCurrentMemory( MEM_SPIRIT );
	return( qtrue );
}

static qboolean SpiritParse_offset( SpiritSystem_t *SpiritSystem, int handle ) {
	int i;
	SpiritSystem->flags |= SPIRIT_OFFSET;
	for (i=0; i<3; i++) 
		if (!SpiritParseRandom(handle, &SpiritSystem->offset[i]) )
			return (qfalse);
	return( qtrue );
}

static qboolean SpiritParse_rotate( SpiritSystem_t *SpiritSystem, int handle ) {
	int i;
	SpiritSystem->flags |= SPIRIT_ROTATE;
	for (i=0;i<3;i++)  {
		if ( !PC_Float_Parse( handle, &SpiritSystem->rotate[i] ) ) 
			return( qfalse );
		SpiritSystem->rotate[i] = DEG2RAD(SpiritSystem->rotate[i]);
	}
	return( qtrue );
}

static qboolean SpiritParse_scalewave( SpiritSystem_t *SpiritSystem, int handle ) {
	return SpiritParseWave( handle, &SpiritSystem->scalewave);
}


static qboolean SpiritParse_shader( SpiritSystem_t *SpiritSystem, int handle ) {
	char str[256];
	if ( !PC_String_ParseNoAlloc( handle, str, sizeof(str) ) ) {
		return( qfalse );
	}
	SpiritSystem->shader = trap_R_RegisterShader( str );
	return( qtrue );
}

static qboolean SpiritParse_spawnrange( SpiritSystem_t *SpiritSystem, int handle ) {
	if ( !PC_Float_Parse( handle, &SpiritSystem->spawnrange ) ) 
		return( qfalse );
	SpiritSystem->flags |= SPIRIT_SPAWNRANGE;
	SpiritSystem->spawnrange *= SpiritSystem->spawnrange;
	return ( qtrue );
}

static qboolean SpiritParse_spawnrate( SpiritSystem_t *SpiritSystem, int handle ) {
	float spawnrate;
	if ( !PC_Float_Parse( handle, &spawnrate ) ) {
		return( qfalse );
	}
	SpiritSystem->spawndelay = 1000 / spawnrate;
	return( qtrue );
}

static qboolean SpiritParse_sprite( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->type = SPIRIT_TSPRITE;
	if ( !SpiritParseRandom(handle, &SpiritSystem->radius[0] ))
		return( qfalse );
	if ( !SpiritParseRandom(handle, &SpiritSystem->radius[1] ))
		return( qfalse );
	return qtrue;
}

static qboolean SpiritParse_tracer( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->type = SPIRIT_TTRACER;
	if ( !PC_Float_Parse( handle, &SpiritSystem->size ))
		return( qfalse );
	if ( !SpiritParseRandom(handle, &SpiritSystem->radius[0] ))
		return( qfalse );
	if ( !SpiritParseRandom(handle, &SpiritSystem->radius[1] ))
		return( qfalse );
	return ( qtrue );
}

static qboolean SpiritParse_velocity( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->flags |= SPIRIT_MOVE;
	if ( !SpiritParseRandom( handle, &SpiritSystem->velocity ) ) {
		return( qfalse );
	}
	return( qtrue );
}

static qboolean SpiritParse_worldlight( SpiritSystem_t *SpiritSystem, int handle ) {
	SpiritSystem->flags |= SPIRIT_WORLDLIGHTCOLOURED;
	return( qtrue );
}

static SpiritKeywordHash_t SpiritParseKeywords[] = {
	{"accel",			SpiritParse_accel, NULL},
	{"alpha",			SpiritParse_alpha, NULL}, 
	{"align",			SpiritParse_align, NULL}, 
	{"angles",			SpiritParse_angles, NULL},
	{"animshaders",		SpiritParse_animshaders, NULL},
	{"avelocity",		SpiritParse_avelocity, NULL},
	{"bouncefactor",	SpiritParse_bouncefactor, NULL},
	{"collision",		SpiritParse_collision, NULL},
	{"cullrange",		SpiritParse_cullrange, NULL},
	{"count",			SpiritParse_count, NULL},
	{"customrgb",		SpiritParse_customrgb, NULL},
	{"customshader",	SpiritParse_customshader, NULL},
	{"delay",			SpiritParse_delay, NULL},
	{"dietouch",		SpiritParse_dietouch, NULL},
	{"direction",		SpiritParse_direction, NULL},
	{"distance",		SpiritParse_distance, NULL},
	{"dlight",			SpiritParse_dlight, NULL},
	{"friction",		SpiritParse_friction, NULL},
	{"group",			SpiritParse_group, NULL},
	{"life",			SpiritParse_life, NULL},
	{"model",			SpiritParse_model, NULL},
	{"offset",			SpiritParse_offset, NULL},
	{"rgb",				SpiritParse_rgb, NULL},
	{"rgbrange",		SpiritParse_rgbrange, NULL},
	{"rotate",			SpiritParse_rotate, NULL},
	{"scalewave",		SpiritParse_scalewave, NULL},
	{"shader",			SpiritParse_shader, NULL},
	{"sprite",			SpiritParse_sprite, NULL},
	{"spawnrange",		SpiritParse_spawnrange, NULL},
	{"spawnrate",		SpiritParse_spawnrate, NULL},
	{"tracer",			SpiritParse_tracer, NULL},
	{"velocity",		SpiritParse_velocity, NULL},
	{"worldlight",		SpiritParse_worldlight, NULL},
	{NULL, NULL, NULL}
};

static SpiritKeywordHash_t *SpiritParseKeywordHash[SPIRITKEYWORDHASH_SIZE];

void Spirit_SetupKeywordHash( void ) {
	int i;

	memset( SpiritParseKeywordHash, 0, sizeof(SpiritParseKeywordHash) );
	for ( i = 0; SpiritParseKeywords[i].keyword; i++ ) {
		Spirit_KeywordHash_Add( SpiritParseKeywordHash, &SpiritParseKeywords[i] );
	}
}

static qboolean Spirit_ParseSystem(int handle, SpiritSystem_t *SpiritSystem) {
	pc_token_t token;
	SpiritKeywordHash_t *key;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (*token.string != '{') {
		return qfalse;
	}
    
	while ( 1 ) {
		memset(&token, 0, sizeof(pc_token_t));

		if (!trap_PC_ReadToken(handle, &token)) {
			PC_SourceError(handle, "end of file inside Spirit-script");
			return qfalse;
		}

		if (*token.string == '}') {
			return qtrue;
		}

		key = Spirit_KeywordHash_Find(SpiritParseKeywordHash, token.string);
		if (!key) {
			PC_SourceError(handle, "unknown Spirit keyword %s", token.string);
			continue;
		}
		if ( !key->func(SpiritSystem, handle) ) {
			PC_SourceError(handle, "couldn't parse Spirit keyword %s", token.string);
			return qfalse;
		}
	}

	// should never get here
	return qfalse;
}

static void Spirit_InitSystem( SpiritSystem_t *SpiritSystem ) {
	memset(SpiritSystem, 0, sizeof(SpiritSystem_t));

	SpiritSystem->type = SPIRIT_TSPRITE;
	SpiritSystem->bouncefactor = 1.0f;
	SpiritSystem->rgb[0][0] = 1.0f;
	SpiritSystem->rgb[0][1] = 1.0f;
	SpiritSystem->rgb[0][2] = 1.0f;
	SpiritSystem->alpha = 1.0f;
}

static void Spirit_PostParseSystem( SpiritSystem_t *SpiritSystem ) {
	if( SpiritSystem == NULL ) {
		return;
	}

	if( SpiritSystem->life < 0 )
		SpiritSystem->life = 0;

	if ( SpiritSystem->flags & SPIRIT_ANIMATEDSHADER ) {
		SpiritSystem->shader = SpiritSystem->animshaders[0];
		SpiritSystem->animframerate = SpiritSystem->animnumframes * SpiritSystem->animframerate / (SpiritSystem->life+1 );
	}
}

static SpiritSystem_t *Spirit_NewSystem( int handle ) {
	int TempCount = SpiritSystemDepth + SpiritSystemCount;
	SpiritSystem_t *SpiritSystem = &SpiritSystems[TempCount];

	if( TempCount >= MAX_SYSTEM ) 
		return NULL;

	SpiritSystemDepth++;
	SetCurrentMemory( MEM_SPIRIT );
	Spirit_InitSystem( SpiritSystem);
	if( Spirit_ParseSystem( handle, SpiritSystem ) ) {
		Spirit_PostParseSystem( SpiritSystem );
		SpiritSystemCount++;
		SpiritSystemDepth--;
		return( SpiritSystem );
	}
	// If we got here something went wrong
	SpiritSystemDepth--;
	return( NULL );
}

/*
===============
Spirit_Count
===============
*/
int Spirit_SystemCount( ) {
	return SpiritSystemCount;
}

int Spirit_ScriptCount( ) {
	return SpiritScriptCount;
}

void Spirit_Reset( ) {
	SpiritSystemCount = 0;
	SpiritSystemDepth = 0;
	SpiritScriptCount = 0;
	SpiritScriptDepth = 0;

	Memory_Init( MEM_SPIRIT );
}

static SpiritScript_t * Spirit_ParseScript(const char *filename); 
/* CaNaBiS, This could probably go horribly wrong, but might still be useful */
void Spirit_Reload( ) {
	int i, oldcount;
	char name[256];
	SpiritScript_t *script;

	oldcount = SpiritScriptCount;
	Spirit_Reset();
	for (i=0 ; i < oldcount ; i++ ) {
		Q_strncpyz( name, SpiritScripts[i].filename, 256 );
		script = Spirit_ParseScript( name );
		if ( script != &SpiritScripts[i] ) {
			CG_Error( "Reloading Sprit Script  %s failed\n", name );
		}
	}
}

static SpiritScript_t * Spirit_ParseScript(const char *filename) {
	pc_token_t token;
	int handle;
	int TempCount = SpiritScriptDepth + SpiritScriptCount;
	SpiritScript_t *SpiritScript = &SpiritScripts[TempCount];

	if( TempCount >= MAX_SCRIPT ) 
		return NULL;

	handle = trap_PC_LoadSource( filename );
	if (!handle)
		return( NULL );

	SpiritScriptDepth++;
	SetCurrentMemory( MEM_SPIRIT );
	memset(SpiritScript, 0, sizeof(SpiritScript_t));
	/* Copy the filename into this script */
	strncpy( SpiritScript->filename, filename, 256 );
	while ( 1 ) {
		if (!trap_PC_ReadToken( handle, &token )) 
		break;
		if ( !Q_stricmp(token.string, "link") ) {
			if (!trap_PC_ReadToken( handle, &token ))
				break;
			if (SpiritScript->link)
				break;
			SpiritScript->link = Spirit_LoadScript( token.string );
		} else if ( !Q_stricmp(token.string, "sound") ) {
			if (!trap_PC_ReadToken( handle, &token ))
				break;
			SpiritScript->sound = trap_S_RegisterSound( token.string, qfalse );
		} else if ( !Q_stricmp(token.string, "loopsound") ) {
			if (!trap_PC_ReadToken( handle, &token ))
				break;
			SpiritScript->sound = trap_S_RegisterSound( token.string, qfalse );
			SpiritScript->flags |=  SPIRIT_SCRIPT_LOOPEDSOUND;
		} else if ( !Q_stricmp(token.string, "system") ) {
			SpiritSystem_t *NewSystem = Spirit_NewSystem(handle);
			if ( !NewSystem ) {
				Com_Printf( "Invalid system section found in Spirit script '%s'\n", filename );
				continue;
			}
            NewSystem->next = SpiritScript->SpiritSystem;
			SpiritScript->SpiritSystem = NewSystem;
		} else {
			Com_Printf( "Invalid token %s found in Spirit script '%s'\n", token.string, filename );
			break;
		}
	}
	trap_PC_FreeSource( handle );
	SpiritScriptCount++;
	SpiritScriptDepth--;
	return( SpiritScript );
}

SpiritScript_t *Spirit_LoadScript(const char *filename) {
	int i;

	// See if we already parsed this file before
	for( i = 0; i < SpiritScriptCount; i++ ) {
		SpiritScript_t *SpiritScript = &SpiritScripts[i];
		if( !Q_stricmp( SpiritScript->filename, filename) ) {
			return SpiritScript;
		}
	}
	return Spirit_ParseScript( filename);
}


void Spirit_AddStandAlone( char * script,vec3_t origin, vec3_t dir, float rotation) {
	SpiritScript_t *SpiritScript;
	StandAloneScript_t * standalone;

	if (StandAloneCount >= MAX_STANDALONE)
		return;
	SpiritScript = Spirit_LoadScript( script );
	if (!SpiritScript)
		return;

	standalone = &StandAloneScripts[StandAloneCount++];

	standalone->SpiritScript = SpiritScript;
	VectorCopy(origin, standalone->origin );
	VectorNormalize2( dir, standalone->axis[2] );
	MakeNormalVectors( standalone->axis[2], standalone->axis[1], standalone->axis[0] );
	RotateAroundDirection( standalone->axis, rotation );
}


