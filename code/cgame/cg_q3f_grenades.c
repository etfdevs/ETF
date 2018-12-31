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
**	cg_q3f_grenades.c
**
**	Server-side functions for handling grenades
*/

#include "cg_local.h"
#include "cg_q3f_grenades.h"
#include "../game/bg_q3f_playerclass.h"

/*
**	Rendering functions for grenades
*/

#include "cg_local.h"
#include "cg_q3f_grenades.h"

#define PULSEDURATION 800

extern cg_q3f_grenade_t *cg_q3f_grenades[Q3F_NUM_GRENADES];

static qboolean FlareRender( centity_t *cent, refEntity_t *ent )
{
	// Render the grenade - basically, just the usual grenade plus a light entity.

	int offset;
	float r, g, b, i;
	cg_q3f_grenade_t *gren;

	offset = cg.time - cent->currentState.time;
	if( offset < 5000 )
	{
		// Startup time

		r = 1;
		g = 0.1f;
		b = 0.1f;
		i = 200;
	}
	else if( offset < 10000 )
	{
		// Warmup

		r = 1;
		g = 0.1f + 0.9f * ((float)(offset - 5000)) / 5000;
		b = (rand() & 1) ? 0.5f : 0.2f;
		i = g * 3000;
	}
	else if( offset < 25000 )
	{
		// Fullbright time

		r = 1;
		g = 0.6f + 0.4f * (((float)rand()) / 32768.0f);
		b = 0.5f;
		i = 3000;
	}
	else {
		// Dropoff time

		r = 1;
		g = 1 - ((float)(offset - 25000)) / 5000.0f;
		if( g < 0 )
			g = 0;
		b = 0;
		i = 200 + g * 2800;
	}

	r /= 3.f;
	g /= 3.f;
	b /= 3.f;
	i /= 4.f;

	gren = CG_Q3F_GetGrenade( cent->currentState.weapon );

#ifdef API_Q3
	// Arnout : Q3F Port - FIXME, find a replacement for this in ET
	trap_R_AddAdditiveLightToScene( cent->lerpOrigin, i, r, g, b );
#endif
	ent->hModel = gren->hModel;
	ent->customSkin = gren->hSkin;
	if ( cg_lowEffects.value )
		trap_R_AddRefEntityToScene( ent, cent );
	else
		CG_AddRefEntityWithPowerups( ent, &cent->currentState, Q3F_TEAM_FREE );

	return( qfalse );		// Prevent normal rendering
}

static qboolean NapalmRender( centity_t *cent, refEntity_t *ent )
{
	// Cheap 'standin' effect until I get something decent.

	vec3_t dir, colour;
	float radius;
	int count;
	localEntity_t *le;
	refEntity_t *re;

	count = cg.time - cent->currentState.time;

	colour[0] = ((float)(count % 255 )) / 255;
	colour[0] = 0.9 + 0.1*sin( 2*M_PI*colour[0]);
	colour[1] = ((float)(count % 341 )) / 341;
	colour[1] = 0.7 + 0.3*sin( 2*M_PI*colour[1]);
	colour[2] = 0;

	radius = ((float)(count % 1000 )) / 1000;
	radius = 0.8 + 0.2*sin( 2*M_PI*radius);

	trap_R_AddLightToScene( cent->lerpOrigin, 300 - radius*30, 1, colour[0], colour[1], colour[2], 0,0);

	/* Spawn a new sprite */
	if (cg.time >= cent->miscTime ) {
		cent->miscTime = cg.time + 20 + 20 * cg_lowEffects.integer;

		count = rand() % NUMVERTEXNORMALS;
		VectorCopy(bytedirs[count],dir);
		dir[2]/=2;
		VectorScale(dir, 100, dir);
        
		le = CG_AllocLocalEntity( 1000 );
		le->fadeRate = 1.0 / (1000 - 800);
		le->radius = 10;
		le->radiusrate = (40.0 - 10.0) / 1000.0;

		re = &le->refEntity;
		re->rotation = Q_flrand(0.0f, 1.0f) * 360;
		le->leType = LE_NAPALM_FLAME;

		le->pos.trType = TR_LINEAR;
		le->pos.trTime = cg.oldTime;			//Give it a slightly older time will project it further
		VectorCopy( dir, le->pos.trDelta );
		VectorCopy( cent->lerpOrigin, le->pos.trBase );
		le->pos.trBase[2]+=4;

		re->shaderRGBA[0] = 0xff;
		re->shaderRGBA[1] = 0xff;
		re->shaderRGBA[2] = 0xff;
		re->shaderRGBA[3] = 0xff;

		re->reType = RT_SPRITE;
	}

	if( !cent->muzzleFlashTime )
	{
		cent->muzzleFlashTime = cg.time;

		if ( CG_PointContents( cent->lerpOrigin, -1 ) & CONTENTS_WATER ) {
			trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_AUTO, cgs.media.sfx_napalmWater );
		} else {
			trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_AUTO, cgs.media.sfx_napalmExplode );
		}
	}
	else if( cent->muzzleFlashTime + 1720 < cg.time )		// Let the first sound play out first
		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.sfx_napalmBurn, 255, 0 ); 

	return( qtrue );	// Continue normal rendering
}


#define GASINTERVAL 90				//Interval between outputting a sprite

static qboolean HallucinogenicRender( centity_t *cent, refEntity_t *ent )
{
	int timespent, needed;
	float fraction, fraction_add;
	int volume;


	volume = cg.time - cent->currentState.time;
	if (volume > 18000) 
		volume = 90 - 60 * (volume - 18000) / 2000;
	else volume = 90;
	trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin,
				vec3_origin, cgs.media.gasSmokeSound, volume, 0 );

	if (cg.time < cent->miscTime + GASINTERVAL ) return qfalse;

//	if( cg_lowEffects.integer && (cg.clientFrame & 3) )
//		return( qtrue );		// Low effects, reduce rendering.

	/* Determine if we need to catch up */
	needed = ( cg.time - cent->currentState.time ) / GASINTERVAL;
	if (needed > 30) needed = 30;
	fraction = 0;
	if (needed > cent->muzzleFlashTime ) {
		fraction_add= 1 / (float)( needed - cent->muzzleFlashTime );
	} else fraction_add=0;
	do {
		needed--;	
		if (!CG_SpawnGasSprite(cent, fraction ))
			CG_SpawnGasSprite(cent, fraction );
		fraction+=fraction_add;
	} while ( needed > cent->muzzleFlashTime );
	/* Determine the next time to add a sprite */
	timespent=cg.time - cent->miscTime;
	if (timespent > GASINTERVAL*2) {
		/* Give an immediate new sprite next frame */
		cent->miscTime = cg.time - GASINTERVAL;
	} else {
		cent->miscTime = cg.time - timespent + GASINTERVAL;
	}
	return qfalse;		//show up the original grenade
}

extern cg_q3f_grenade_t cg_q3f_grenade_emp;

void CG_PulseExplosion(vec3_t base_origin) {
	vec3_t origin,dir;
//	cg_q3f_grenade_t *gren = &cg_q3f_grenade_emp;
	localEntity_t	*le;
	int i;

	VectorCopy ( base_origin, origin);
	if ( cg_no3DExplosions.value < 1 ) {
		// Lift it up a bit to stop zfighting and other evil things
		origin[2] += 8;
		le = CG_MakeExplosion( origin, PULSEDURATION*0.8, 2, 4,
								cgs.media.sphereFlashModel,
							    cgs.media.pulse3DExplosionShader
							   );

//		VectorCopy( gren->g->lightColor, le->lightColor );
//		le->light = gren->g->light;

		// Now add the ring
		VectorSet(dir, 0, 0, 1);
		le = CG_Q3F_MakeRing( origin, dir, PULSEDURATION,
							  40, 128,
							  0, cgs.media.pulseRingShader );
		// And now, add a bunch of electric bolts
		for ( i = 0; i < 18; i++ ) {
			vec3_t dest;
			trace_t trace;
			int r;
			r = rand() % NUMVERTEXNORMALS;
			VectorMA( origin, 256, bytedirs[r], dest );
			CG_Trace( &trace, origin, NULL, NULL, dest, -1, CONTENTS_SOLID );
			if ( trace.fraction == 1.0 )
				continue;
			le = CG_Q3F_MakeBeam( origin, 
					 		      trace.endpos,
								  15,
								  trace.fraction * 12 + 2,
								  2,
								  Q3F_BEAM_WAVE_EFFECT | Q3F_BEAM_FADE,
								  colorWhite,
								  PULSEDURATION*0.8,
								  0.6,
								  cgs.media.pulseBeamShader );
			CG_OldMark(cgs.media.energyMarkShader,trace.endpos, trace.plane.normal,Q_flrand(0.0f, 1.0f)*360,7,colorWhite,cg_markTime.integer >> 1,LEMFT_ALPHA);
		}
	} else {
		le = CG_MakeExplosion(	origin, 800, 2, 4,
								cgs.media.sphereFlashModel,
								cgs.media.pulseExplosionShader );
	}
}

static qboolean PulseRender( centity_t *cent, refEntity_t *ent ) {
	vec3_t	dir;
	cg_q3f_grenade_t *gren;
	gren = CG_Q3F_GetGrenade( cent->currentState.weapon );

	if( cent->miscTime != cent->currentState.time )	{	
		cent->miscTime = cent->currentState.time;

		trap_S_StartSound( ent->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.sfx_pulseexp );
		CG_PulseExplosion( cent->lerpOrigin);
		CG_OldMark(cgs.media.energyMarkShader,cent->lerpOrigin,dir,Q_flrand(0.0f, 1.0f)*360,gren->g->damage / 2,colorWhite,cg_markTime.integer >> 1,LEMFT_ALPHA);
		CG_Q3F_Vibrate( gren->g->damage, ent->origin );
	}
	return( qtrue );
}

/*
**	Nail rendering functions (a big enough topic to merit it's own comment :)
*/

struct gnail {
	float xvel, yvel, xpos, ypos, angle;
	int starttime, endtime;
};
static int nailarraylifetime[Q3F_NUM_NAILGRENADES];
static centity_t *nailarrayowner[Q3F_NUM_NAILGRENADES];
static struct gnail nailarray[Q3F_NUM_NAILGRENADES][Q3F_NUM_NAILGRENNAILS];

static int NailEndTime( struct gnail *nail, float zpos, int parentnum )
{
	// Work out how long the nail will last before impacting on a wall.

	trace_t tr;
	vec3_t start, end;

	start[0] = nail->xpos;
	start[1] = nail->ypos;
	start[2] = zpos;
	end[0] = nail->xvel;
	end[1] = nail->yvel;
	end[2] = 0;
	VectorScale( end, 20000 * Q3F_NAILSPEED / 1000 , end );		// 20 seconds travel
	VectorAdd( start, end, end );

	CG_Trace( &tr, start, NULL, NULL, end, parentnum, MASK_SHOT );
	return( nail->starttime + 20000 * tr.fraction );
}

static int InitNailArray( centity_t *cent )
{
	// Find an unused array, or return Q3F_NUM_NAILGRENADES

	int index, nailindex, starttime;
	vec3_t angles, direction, origin;
	struct gnail *ptr;

	starttime = cent->currentState.time + 500;		// There's a half-second delay before starting
	BG_EvaluateTrajectory( &cent->currentState.pos, starttime, origin );

	for( index = 0; index < Q3F_NUM_NAILGRENADES; index++ )
	{
		if( nailarraylifetime[index] < cg.time )
		{
			nailarraylifetime[index] = cg.time + Q3F_NAILGRENADETIME + 1000;
			angles[YAW] = 0;
			angles[PITCH] = 0;
			angles[ROLL] = 0;
			for( nailindex = 0; nailindex < Q3F_NUM_NAILGRENNAILS; )
			{
				ptr = &nailarray[index][nailindex];
				AngleVectors( angles, direction, 0, 0 );

				ptr->xpos		= origin[0];
				ptr->ypos		= origin[1];
				ptr->xvel		= direction[0];
				ptr->yvel		= direction[1];
				ptr->angle		= angles[YAW];
				ptr->starttime	= starttime;
				ptr->endtime	= NailEndTime( ptr, origin[2], cent->currentState.otherEntityNum );
				nailindex++;
				ptr++;

				ptr->xpos		= origin[0];
				ptr->ypos		= origin[1];
				ptr->xvel		= -direction[0];
				ptr->yvel		= -direction[1];
				ptr->angle		= 180 + angles[YAW];
				ptr->starttime	= starttime;
				ptr->endtime	= NailEndTime( ptr, origin[2], cent->currentState.otherEntityNum );
				nailindex++;
				ptr++;
				angles[YAW] = AngleMod( angles[YAW] + Q3F_NAILGRENANGLE );
				starttime += Q3F_NAILGRENINTERVAL;
			}
			nailarrayowner[index] = cent;
			return( index );
		}
	}
	return( Q3F_NUM_NAILGRENADES );
}

static unsigned char nailbits[Q3F_NUM_NAILGRENNAILS>>3];
#define	NAILBIT(x) (nailbits[x>>3]&(1<<(x&7)))
#define	NBITCOPY(w,x,y,z) if(z<Q3F_NUM_NAILGRENNAILS){*w++ = *(y+(char *)x);z+=8;}
static void ExpandNailBits( centity_t *cent )
{
	// Extract the nail bits into a single block. This function MUST
	// only extract as many bits as required, or it'll overflow the buffer.
	// Every two bytes below are 16 bits, so 16*18 = 320 nails (!)

	int size, temp;
	unsigned char *bitptr;

	size = 0;
	bitptr = nailbits;
	NBITCOPY( bitptr, &cent->currentState.time2,			0, size )
	NBITCOPY( bitptr, &cent->currentState.time2,			1, size )
	NBITCOPY( bitptr, &cent->currentState.time2,			2, size )
	NBITCOPY( bitptr, &cent->currentState.time2,			3, size )
	temp = cent->currentState.origin2[0];
	NBITCOPY( bitptr, &temp,		0, size )
	NBITCOPY( bitptr, &temp,		1, size )
	temp = cent->currentState.origin2[1];
	NBITCOPY( bitptr, &temp,		0, size )
	NBITCOPY( bitptr, &temp,		1, size )
	temp = cent->currentState.origin2[2];
	NBITCOPY( bitptr, &temp,		0, size )
	NBITCOPY( bitptr, &temp,		1, size )
	temp = cent->currentState.angles2[0];
	NBITCOPY( bitptr, &temp,		0, size )
	NBITCOPY( bitptr, &temp,		1, size )
	temp = cent->currentState.angles2[1];
	NBITCOPY( bitptr, &temp,		0, size )
	NBITCOPY( bitptr, &temp,		1, size )
	temp = cent->currentState.angles2[2];
	NBITCOPY( bitptr, &temp,		0, size )
	NBITCOPY( bitptr, &temp,		1, size )

	NBITCOPY( bitptr, &cent->currentState.constantLight,	0, size )
	NBITCOPY( bitptr, &cent->currentState.constantLight,	1, size )
//	NBITCOPY( bitptr, &cent->currentState.otherEntityNum,	0, size )	// 10-bit
	NBITCOPY( bitptr, &cent->currentState.otherEntityNum2,	0, size )	// 10-bit
	NBITCOPY( bitptr, &cent->currentState.modelindex2,		0, size )	// 8-bit
	NBITCOPY( bitptr, &cent->currentState.legsAnim,			0, size )	// 8-bit
	NBITCOPY( bitptr, &cent->currentState.torsoAnim,		0, size )	// 8 bit

	NBITCOPY( bitptr, &cent->currentState.generic1,			0, size )
	NBITCOPY( bitptr, &cent->currentState.generic1,			1, size )
//	NBITCOPY( bitptr, &cent->currentState.generic1,			2, size )
//	NBITCOPY( bitptr, &cent->currentState.generic1,			3, size )
}

static qhandle_t nailmodel;
static qboolean NailRender( centity_t *cent, refEntity_t *ent )
{
	// Render the nail grenade - on startup, a complete set of
	// struct gnails are spawned and stored in one of the available
	// arrays (assuming there is one available, of course).
	// Each frame it culls the nails marked as having 'hit' something.
	// Nails that hit scenery on the server side aren't marked, since
	// the client can work that out on it's own (so only things like player
	// impacts are marked).

	struct gnail *array, *ptr;
	int index, r;
	vec3_t dir;
	cg_q3f_grenade_t *gren;
	refEntity_t nailent;
	vec3_t start, end;
	trace_t trace;
	sfxHandle_t sfx;

 	if( !nailmodel )
 		nailmodel = trap_R_RegisterModel( "models/ammo/nail/nail.md3" );

	ExpandNailBits( cent );

		// create the render entity
	memset (&nailent, 0, sizeof(nailent));
	nailent.hModel = nailmodel;
	nailent.reType = RT_MODEL;
	
	AxisClear( nailent.axis );

	if( cg.time >= (cent->currentState.time + 500) )
	{
			// Check we have an array
		if(	!cent->muzzleFlashTime ||
			nailarrayowner[cent->muzzleFlashTime - 1] != cent ||
			nailarraylifetime[cent->muzzleFlashTime - 1] < cg.time )
		{
			// Attempt to locate it (in case we have one running)
			for( index = 0; index < Q3F_NUM_NAILGRENADES; index++ )
			{
				if(	nailarrayowner[index] != cent ||
					nailarraylifetime[index] < cg.time )
					continue;
				cent->muzzleFlashTime = index + 1;
				break;
			}
			if( index == Q3F_NUM_NAILGRENADES )
			{
				// Couldn't find it, make a new one.

				index = InitNailArray( cent );
				if( index < 0 )
					return( qtrue );	// Failed to allocate.
				cent->muzzleFlashTime = index + 1;
			}
		}
		array = nailarray[cent->muzzleFlashTime - 1];

		for( index = 0; index < Q3F_NUM_NAILGRENNAILS; index++ )
		{
			ptr = array + index;
			if( ptr->starttime && ptr->starttime <= cg.time )
			{
				if( !NAILBIT(index) )
					ptr->starttime = 0;		// We've hit something, remove this nail.
				else {
					start[0]	= ptr->xpos;
					start[1]	= ptr->ypos;
					start[2]	= cent->lerpOrigin[2];
					end[0]		= cent->lerpOrigin[0] + (cg.time - ptr->starttime) * ptr->xvel * Q3F_NAILSPEED / 1000;
					end[1]		= cent->lerpOrigin[1] + (cg.time - ptr->starttime) * ptr->yvel * Q3F_NAILSPEED / 1000;
					end[2]		= cent->lerpOrigin[2];
					if( cg.time < ptr->endtime )
					{
						if( !cg_lowEffects.integer )
							CG_BubbleTrail(start, end, 8);
						ptr->xpos = end[0];
						ptr->ypos = end[1];

						VectorCopy( end, nailent.origin);
						VectorCopy( end, nailent.oldorigin);
						dir[PITCH] = dir[ROLL] = 0;
						dir[YAW] = ptr->angle;
						AnglesToAxis( dir, nailent.axis );
						// add to refresh list, possibly with quad glow
						if( cg_lowEffects.value )
							trap_R_AddRefEntityToScene( &nailent, cent );
						else
						{
							int             team;
							int             parent = cent->currentState.otherEntityNum;

							if( parent >= 0 && parent < MAX_CLIENTS )
								team = cgs.clientinfo[parent].team;
							else
								team = Q3F_TEAM_FREE;
							CG_AddWeaponWithPowerups( &nailent, &cent->currentState, team );
							//CG_AddRefEntityWithPowerups( &nailent, &cent->currentState, team );
						}
					}
					else {
						if( ptr->endtime > (cg.time - 500) )
						{
							// It expired less than half a second ago.
							// We check this to avoid marks for nails that expired far enough
							// back that we'd have missed the effect.


								// Do an impact effect.
							r = rand() & 3;
							if ( r < 2 ) {
								sfx = cgs.media.sfx_ric1;
							} else if ( r == 2 ) {
								sfx = cgs.media.sfx_ric2;
							} else {
								sfx = cgs.media.sfx_ric3;
							}

							CG_Trace( &trace, start, NULL, NULL, end, cent->currentState.otherEntityNum, MASK_SHOT );

							trap_S_StartSound(	trace.endpos, ENTITYNUM_WORLD,
								((rand() & 1) ? CHAN_WEAPON : CHAN_ITEM), sfx );

							dir[0] = ptr->xvel;
							dir[1] = ptr->yvel;
							dir[2] = 0;

							if( !cg_lowEffects.integer && trace.fraction < 1.0f )
							{
								if( trace.entityNum != ENTITYNUM_NONE && trace.contents & CONTENTS_FORCEFIELD )
								{
									vec3_t axis[3];
									VectorNormalize2( trace.plane.normal, axis[2] );
									MakeNormalVectors( axis[2], axis[1], axis[0] );
									/* Ensiform - We do a check here because some surfaces are forcefield'd but not necessarily func_forcefield */
									if( !VectorCompare( cg_entities[trace.entityNum].currentState.angles2, vec3_origin ) )
										Spirit_SetCustomColor( cg_entities[trace.entityNum].currentState.angles2 );
									else
										Spirit_SetCustomColor( colorWhite );
									Spirit_RunScript( cgs.spirit.forcefieldspark, trace.endpos, trace.endpos, axis, 0 );
								}
								else if( !( trace.surfaceFlags & SURF_NOIMPACT ) )
								{
									// Adding this extra check prevents nailbomb nails from creating explosion marks on sky
									// and other noimpact surfaces
									trace.endpos[2] += Q_flrand(-1.0f, 1.0f) * 6;
									CG_BulletExplosion( trace.endpos, trace.plane.normal );
									CG_BulletMark(cgs.media.bulletMarkShader,trace.endpos,trace.plane.normal,8,colorWhite);
								}
							}

							//CG_Q3F_Vibrate( 2, trace.endpos );
						}
						ptr->starttime = 0;		// Mark this nail as expired.
					}
				}
			}
		}
	}

	gren = CG_Q3F_GetGrenade( cent->currentState.weapon );
	if( cent->miscTime != cent->currentState.time )
	{
		if( cg.time >= (cent->currentState.time + Q3F_NAILGRENINTERVAL * Q3F_NUM_NAILGRENNAILS / 2 + 500) )
		{
			cent->miscTime = cent->currentState.time;
			/* Ensiform - Don't need this here as server sends us the EV_ETF_GRENADE_EXPLOSION anyway */
			/*if ( cg_no3DExplosions.value  ) {
				Spirit_RunScript( cgs.spirit.explosion_simple, ent->origin, ent->origin, axisDefault, 0 ); 
			} else {
				Spirit_RunScript( cgs.spirit.explosion_normal, ent->origin, ent->origin, axisDefault, 0 ); 
			}
			
			trap_S_StartSound( ent->origin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.sfx_rockexp );
			cent->miscTime = cent->currentState.time;

			CG_Q3F_Vibrate( 100, ent->origin );
			return qtrue;		//Don't render the original grenade*/
		}
		else {
			// We need to draw the nail grenade, spinning away
			ent->hModel = gren->hModel;
			ent->customSkin = gren->hSkin;
			start[YAW] = cg.time;
			start[PITCH] = start[ROLL] = 0;
			AnglesToAxis( start, ent->axis );
			if ( cg_lowEffects.value )
				trap_R_AddRefEntityToScene( ent, cent );
			else
			{
				int             team;
				int             parent = cent->currentState.otherEntityNum;

				if ( parent >= 0 && parent < MAX_CLIENTS )
					team = cgs.clientinfo[parent].team;
				else
					team = Q3F_TEAM_FREE;
				CG_AddWeaponWithPowerups( ent, &cent->currentState, team );
				//CG_AddRefEntityWithPowerups( ent, &cent->currentState, team );
			}
			//if ( cg_lowEffects.value )
			//	trap_R_AddRefEntityToScene( ent, cent );
			//else 
			//	CG_AddRefEntityWithPowerups( ent, &cent->currentState, cent->currentState.clientNum );
		}
	}
	return( qtrue );
}


/*
**	Client-side grenade definitions
*/

extern bg_q3f_grenade_t bg_q3f_grenade_none;
extern bg_q3f_grenade_t bg_q3f_grenade_normal;
extern bg_q3f_grenade_t bg_q3f_grenade_concuss;
extern bg_q3f_grenade_t bg_q3f_grenade_flash;
extern bg_q3f_grenade_t bg_q3f_grenade_flare;
extern bg_q3f_grenade_t bg_q3f_grenade_nail;
extern bg_q3f_grenade_t bg_q3f_grenade_cluster;
extern bg_q3f_grenade_t bg_q3f_grenade_clustersection;
extern bg_q3f_grenade_t bg_q3f_grenade_napalm;
extern bg_q3f_grenade_t bg_q3f_grenade_gas;
extern bg_q3f_grenade_t bg_q3f_grenade_emp;
extern bg_q3f_grenade_t bg_q3f_grenade_charge;

cg_q3f_grenade_t cg_q3f_grenade_none = {
	&bg_q3f_grenade_none,
	0,
	NULL_HANDLE, NULL_HANDLE
};

cg_q3f_grenade_t cg_q3f_grenade_normal = {
	&bg_q3f_grenade_normal,
	0,
	NULL_HANDLE, NULL_HANDLE
};

cg_q3f_grenade_t cg_q3f_grenade_concuss = {
	&bg_q3f_grenade_concuss,
	0,
	NULL_HANDLE, NULL_HANDLE
};

cg_q3f_grenade_t cg_q3f_grenade_flash = {
	&bg_q3f_grenade_flash,
	0,
	NULL_HANDLE, NULL_HANDLE
};

cg_q3f_grenade_t cg_q3f_grenade_flare = {
	&bg_q3f_grenade_flare,
	&FlareRender,
	NULL_HANDLE, NULL_HANDLE
};

cg_q3f_grenade_t cg_q3f_grenade_nail = {
	&bg_q3f_grenade_nail,
	&NailRender,
	NULL_HANDLE, NULL_HANDLE
};

cg_q3f_grenade_t cg_q3f_grenade_cluster = {
	&bg_q3f_grenade_cluster,
	0,
	NULL_HANDLE, NULL_HANDLE
};

cg_q3f_grenade_t cg_q3f_grenade_clustersection = {
	&bg_q3f_grenade_clustersection,
	0,
	NULL_HANDLE, NULL_HANDLE
};

cg_q3f_grenade_t cg_q3f_grenade_napalm = {
	&bg_q3f_grenade_napalm,
	&NapalmRender,
	NULL_HANDLE, NULL_HANDLE
};

cg_q3f_grenade_t cg_q3f_grenade_gas = {
	&bg_q3f_grenade_gas,
	&HallucinogenicRender,
	NULL_HANDLE, NULL_HANDLE
};

cg_q3f_grenade_t cg_q3f_grenade_emp = {
	&bg_q3f_grenade_emp,
	&PulseRender,
	NULL_HANDLE, NULL_HANDLE
};

cg_q3f_grenade_t cg_q3f_grenade_charge = {
	&bg_q3f_grenade_charge,
	NULL,
	NULL_HANDLE, NULL_HANDLE
};

cg_q3f_grenade_t *cg_q3f_grenades[Q3F_NUM_GRENADES] = {
	&cg_q3f_grenade_none,
	&cg_q3f_grenade_normal,
	&cg_q3f_grenade_concuss,
	&cg_q3f_grenade_flash,
	&cg_q3f_grenade_flare,
	&cg_q3f_grenade_nail,
	&cg_q3f_grenade_cluster,
	&cg_q3f_grenade_clustersection,
	&cg_q3f_grenade_napalm,
	&cg_q3f_grenade_gas,
	&cg_q3f_grenade_emp,
	&cg_q3f_grenade_charge
};

cg_q3f_grenade_t *CG_Q3F_GetGrenade( int index )
{
	cg_q3f_grenade_t *gren;

	if(	index < 1 ||
		index >= Q3F_NUM_GRENADES )
		return( &cg_q3f_grenade_none );
	gren = cg_q3f_grenades[index];
	return( gren ? gren : &cg_q3f_grenade_none );
}


// TODO add more things here that get used by grenades
void CG_Q3F_RegisterGrenade( int grenadeNum ) {
	cg_q3f_grenade_t	*gren;

	gren = CG_Q3F_GetGrenade(grenadeNum);

	if ( grenadeNum == 0 || gren == &cg_q3f_grenade_none ) {
		return;
	}

	if ( !gren->hModel )
		gren->hModel = trap_R_RegisterModel( gren->g->model );
	if ( !gren->hSkin && gren->g->skin && *gren->g->skin )
		gren->hSkin = trap_R_RegisterSkin( gren->g->skin );

	// Make sure nails are registered
	if ( grenadeNum == Q3F_GREN_NAIL ) {
		CG_RegisterWeapon( WP_NAILGUN );
		nailmodel = trap_R_RegisterModel( "models/ammo/nail/nail.md3" );
	}

	if ( grenadeNum == Q3F_GREN_NAPALM ) {
		cgs.media.sfx_napalmExplode = trap_S_RegisterSound( "sound/weapons/explosive/gren_napalm_start.wav", qfalse );
		cgs.media.sfx_napalmBurn = trap_S_RegisterSound( "sound/weapons/explosive/gren_napalm_loop.wav", qfalse );
		cgs.media.sfx_napalmWater = trap_S_RegisterSound( "sound/weapons/explosive/gren_napalm_water.wav", qfalse );
	}

	// Also used by gas grenades if flames of any kind trigger a burnout
	if ( grenadeNum == Q3F_GREN_GAS && cgs.media.sfx_napalmExplode == NULL_HANDLE ) {
		cgs.media.sfx_napalmExplode = trap_S_RegisterSound( "sound/weapons/explosive/gren_napalm_start.wav", qfalse );
	}

	// Make sure the cluster child grenades get reg'd too
	if ( grenadeNum == Q3F_GREN_CLUSTER )
		CG_Q3F_RegisterGrenade( Q3F_GREN_CLUSTERSECTION );
}


/*
** Grenade 'run' function - Works largely like an item, except with the
** option of 'sticky grenades'.
*/

void CG_Q3F_Grenade( centity_t *cent )
{
	// This is the normal render, also w/ callbacks

	refEntity_t			ent;
	entityState_t		*s1;
	cg_q3f_grenade_t	*gren;
	vec3_t				dir;

	s1 = &cent->currentState;

	// calculate the axis
	VectorCopy( s1->angles, cent->lerpAngles);

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	gren = CG_Q3F_GetGrenade( s1->weapon );
	if( !gren->hModel )
		gren->hModel = trap_R_RegisterModel( gren->g->model );
	if( !gren->hSkin && gren->g->skin && *gren->g->skin )
		gren->hSkin = trap_R_RegisterSkin( gren->g->skin );

	ent.frame		= s1->frame;
	ent.oldframe	= ent.frame;
	ent.backlerp	= 0;
	ent.hModel		= gren->hModel;
	ent.customSkin	= gren->hSkin;

	// convert angles to axis
	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, dir );
	AnglesToAxis( dir, ent.axis );

	if( s1->time <= cg.time )
	{
		// moved it to server broadcast temp entity
		// We've reached the 'bang' time.
		if( cent->dustTrailTime != s1->time ) {
			/* Check if we need to reset the variables that grenades use */
			cent->muzzleFlashTime = 0;
			cent->miscTime = 0;
			cent->dustTrailTime = s1->time;
		}
		if (!gren->RenderGren) return;
		else if ( gren->RenderGren( cent, &ent )) return;
	} else {
		cent->dustTrailTime = 0;
	}

	// add to refresh list, possibly with quad glow
	if ( cg_lowEffects.value )
		trap_R_AddRefEntityToScene( &ent, cent );
	else {
		int team;
		int parent = s1->otherEntityNum;
		if (parent>=0 && parent <MAX_CLIENTS)
			team = cgs.clientinfo[ parent ].team;
		else 
			team = Q3F_TEAM_FREE;

		CG_AddWeaponWithPowerups( &ent, &cent->currentState, team );
		//CG_AddRefEntityWithPowerups( &ent, s1, team );
	}
}

/*
**	Grenade priming/throwing functions
*/

void CG_Q3F_GrenOnePrime()
{
	// Prime grenade type one.

	bg_q3f_grenade_t *gren;
	bg_q3f_playerclass_t *cls;
	playerState_t *ps;

	if(!cg.snap)
		return;						// JT - Safety - stops crashed by pushing prime buttons during map loads.

		// Check we can throw
	ps = &cg.snap->ps;
	if(	cg.grenadeprimeTime ||
		cg.grenadenextTime > cg.time ||
		(ps->stats[STAT_HEALTH] <= 0) ||
		(ps->persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR) ||
		(ps->eFlags & EF_Q3F_NOSPAWN) ||
		cg.snap->ps.pm_flags & PMF_FOLLOW ||
		cg.snap->ps.pm_flags & PMF_CHASE ||
		(ps->pm_type == PM_INTERMISSION) ||
		cg.ceaseFire || ps->powerups[PW_Q3F_CEASEFIRE] )
		return;

	cls = BG_Q3F_GetClass( ps );
	gren = BG_Q3F_GetGrenade( cls->gren1type );

	if( gren->flags & Q3F_GFLAG_NOTHROW )
		return;		// Can't throw this type of grenade

	if( !(ps->ammo[AMMO_GRENADES] & 0xFF) )
	{
		CG_Printf( BOX_PRINT_MODE_CHAT, "You have no %ss left.\n", gren->name );
		return;
	}

		// Prepare to throw.
	cg.grenadetype = cls->gren1type;
	cg.grenadeprimeTime = cg.time;
	cg.grenadeHudTime = cg.time;

	if( gren->flags & Q3F_GFLAG_QUICKTHROW )
		CG_Q3F_GrenThrow();
	else {
		if( !cgs.media.grenadePrimeSound &&
			cg_grenadePrimeSound.string && 
			*cg_grenadePrimeSound.string )
			cgs.media.grenadePrimeSound = trap_S_RegisterSound( cg_grenadePrimeSound.string, qfalse );
		if( cgs.media.grenadePrimeSound )
			trap_S_StartLocalSound( cgs.media.grenadePrimeSound, CHAN_AUTO); //keeger: Fix dual prime sound issue with channel overrides

		CG_Printf( BOX_PRINT_MODE_CHAT, "%s primed, four seconds.\n", gren->name );			// JT: Used to say 'three seconds'
	}
}
void CG_Q3F_GrenTwoPrime()
{
	// Prime grenade type two.

	bg_q3f_grenade_t *gren;
	bg_q3f_playerclass_t *cls;
	playerState_t *ps;

	if(!cg.snap)
		return;						// JT - Safety - stops crashed by pushing prime buttons during map loads.

		// Check we can throw
	ps = &cg.snap->ps;
	if(	cg.grenadeprimeTime ||
		cg.grenadenextTime > cg.time ||
		(ps->stats[STAT_HEALTH] <= 0) || (ps->persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR) || cg.snap->ps.pm_flags & PMF_FOLLOW || cg.snap->ps.pm_flags & PMF_CHASE ||
		(ps->pm_type == PM_INTERMISSION) ||
		(ps->eFlags & EF_Q3F_NOSPAWN) ||
		cg.ceaseFire || ps->powerups[PW_Q3F_CEASEFIRE] )
		return;

	cls = BG_Q3F_GetClass( ps );
	gren = BG_Q3F_GetGrenade( cls->gren2type );

	if( gren->flags & Q3F_GFLAG_NOTHROW )
		return;		// Can't throw this type of grenade

	if( !(ps->ammo[AMMO_GRENADES] & 0xFF00) )
	{
		CG_Printf( BOX_PRINT_MODE_CHAT, "You have no %ss left.\n", gren->name );
		return;
	}

		// Prepare to throw.
	cg.grenadetype = cls->gren2type;
	cg.grenadeprimeTime = cg.time;
	cg.grenadeHudTime = cg.time;

	if( gren->flags & Q3F_GFLAG_QUICKTHROW )
		CG_Q3F_GrenThrow();
	else {
		if( !cgs.media.grenadePrimeSound &&
			cg_grenadePrimeSound.string && 
			*cg_grenadePrimeSound.string )
			cgs.media.grenadePrimeSound = trap_S_RegisterSound( cg_grenadePrimeSound.string, qfalse );
		if( cgs.media.grenadePrimeSound )
			trap_S_StartLocalSound( cgs.media.grenadePrimeSound, CHAN_AUTO ); //Keeger: fix dual primes running

		CG_Printf( BOX_PRINT_MODE_CHAT, "%s primed, four seconds.\n", gren->name );	// JT - Three seconds?
	}
}

void CG_Q3F_GrenThrow()
{
	// Send a message to the server that we've thrown, reset the throw timer
	char commandbuff[64];

	if( !cg.grenadeprimeTime )
		return;

	if( (cg.grenadeprimeTime + 750) > cg.time )	// RR2DO2: WAS 500
	{
		// Must prime for at least half a second
		cg.grenadethrowTime = cg.grenadeprimeTime + 750;	// RR2DO2: WAS 500
		return;
	}

	Com_sprintf(	commandbuff, 64, "tgren %d %d %d",
					cg.grenadetype, (int)(cg.grenadeprimeTime + cl_timeNudge.value), (int)(cg.time + cl_timeNudge.value) );

	trap_SendClientCommand( commandbuff );
	cg.grenadeprimeTime = 0;
	cg.grenadethrowTime = 0;
	cg.grenadenextTime = cg.time + 100;		// This long before priming another grenade ( RR2DO2: WAS 300 )
}

void CG_Q3F_GrenOnePlusPrime( void )
{
	if( !cg.grenade1latch )
	{
		CG_Q3F_GrenOnePrime();
		cg.grenade1latch = qtrue;
	}
}
void CG_Q3F_GrenOneThrow( void )
{
	// If gren1 is primed, throw it (result of a -gren1 command)

	bg_q3f_playerclass_t *cls;

	cls = BG_Q3F_GetClass( &cg.snap->ps );
	if( cg.grenadetype == cls->gren1type )
		CG_Q3F_GrenThrow();
	cg.grenade1latch = 0;
}

void CG_Q3F_GrenTwoPlusPrime( void )
{
	if( !cg.grenade2latch )
	{
		CG_Q3F_GrenTwoPrime();
		cg.grenade2latch = qtrue;
	}
}
void CG_Q3F_GrenTwoThrow( void )
{
	// If gren2 is primed, throw it

	bg_q3f_playerclass_t *cls;

	cls = BG_Q3F_GetClass( &cg.snap->ps );
	if( cg.grenadetype == cls->gren2type )
		CG_Q3F_GrenThrow();
	cg.grenade2latch = 0;
}

void CG_Q3F_GrenOneToggle()
{
	// Prime OR throw a type 1 grenade

	if( cg.grenadeprimeTime )
		CG_Q3F_GrenOneThrow();
	else CG_Q3F_GrenOnePrime();
}

void CG_Q3F_GrenTwoToggle()
{
	// Prime OR throw a type 2 grenade

	if( cg.grenadeprimeTime )
		CG_Q3F_GrenTwoThrow();
	else CG_Q3F_GrenTwoPrime();
}
