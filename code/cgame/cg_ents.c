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

// cg_ents.c -- present snapshot entities, happens every single frame

#include "cg_local.h"
#include "cg_q3f_grenades.h"
#include "cg_q3f_scanner.h"
#include "../game/bg_q3f_weapon.h"

/*
======================
CG_PositionEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, const char *tagName, int startIndex, vec3_t *offset ) {
	int				i;
	orientation_t	lerped;
	
	// lerp the tag
	trap_R_LerpTag( &lerped, parent, tagName, startIndex );

	// allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );

	if( offset ) {
		VectorAdd( lerped.origin, *offset, lerped.origin );
	}

	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// had to cast away the const to avoid compiler problems...
	MatrixMultiply( lerped.axis, ((refEntity_t *)parent)->axis, entity->axis );
	entity->backlerp = parent->backlerp;
}


/*
======================
CG_PositionRotatedEntityOnTag

Modifies the entities position and axis by the given
tag location
======================
*/
void CG_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, const char *tagName ) {
	int				i;
	orientation_t	lerped;
	matrix3_t		tempAxis;

//AxisClear( entity->axis );
	// lerp the tag
	trap_R_LerpTag( &lerped, parent, tagName, 0 );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	MatrixMultiply( entity->axis, lerped.axis, tempAxis );
	// had to cast away the const to avoid compiler problems...
	MatrixMultiply( tempAxis, ((refEntity_t *)parent)->axis, entity->axis );
}

void CG_GetTagFromModel( orientation_t *tag, qhandle_t hModel, const char *tagName ) {
	refEntity_t re;

	memset( &re, 0, sizeof(refEntity_t) );

	re.hModel = hModel;

    trap_R_LerpTag( tag, &re, tagName, 0 );
}

/*
==========================================================================

FUNCTIONS CALLED EACH FRAME

==========================================================================
*/

/*
======================
CG_SetEntitySoundPosition

Also called by event processing code
======================
*/
void CG_SetEntitySoundPosition( centity_t *cent ) {
	if ( cent->currentState.solid == SOLID_BMODEL ) {
		vec3_t	origin;
		float	*v;

		v = cgs.inlineModelMidpoints[ cent->currentState.modelindex ];
		VectorAdd( cent->lerpOrigin, v, origin );
		trap_S_UpdateEntityPosition( cent->currentState.number, origin );
	} else {
		trap_S_UpdateEntityPosition( cent->currentState.number, cent->lerpOrigin );
	}
}

/*
==================
CG_EntityEffects

Add continuous entity effects, like local entity emission and lighting
==================
*/
static void CG_EntityEffects( centity_t *cent ) {

	// update sound origins
	CG_SetEntitySoundPosition( cent );

	// add loop sound
	if ( cent->currentState.loopSound ) {
		if (cent->currentState.eType == ET_SPEAKER) {
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, 
				cgs.gameSounds[ cent->currentState.loopSound ], 255, 0 );
		} else {
			trap_S_AddRealLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, 
				cgs.gameSounds[ cent->currentState.loopSound ], 1250, 255, 0 );
		}
	}

	// constant light glow
	if ( cent->currentState.constantLight && cent->currentState.eType != ET_INVISIBLE ) {
		int		cl;
		float	i, r, g, b;

		cl = cent->currentState.constantLight;
		r = ( cl & 0xFF ) / 255.f;
		g = ( ( cl >> 8 ) & 0xFF ) / 255.f;
		b = ( ( cl >> 16 ) & 0xFF ) /  255.f;
		i = ( ( cl >> 24 ) & 0xFF ) * 4;

		// Carried goals are special, use player origin intead to avoid lagged-behind light origin
		if( cent->currentState.eType == ET_Q3F_GOAL && cent->currentState.otherEntityNum < MAX_CLIENTS) {
			const centity_t *playerCent = &cg_entities[cent->currentState.otherEntityNum];
			int clNum;

			if ( !playerCent || !playerCent->currentValid )
				return;

			clNum = playerCent->currentState.clientNum;

			if ( cgs.clientinfo[clNum].team == Q3F_TEAM_SPECTATOR || cgs.clientinfo[clNum].cls == Q3F_CLASS_NULL || (playerCent->currentState.eFlags & EF_Q3F_NOSPAWN))
				return;

			trap_R_AddLightToScene( playerCent->lerpOrigin, i, 1.f, r, g, b, 0, 0 );
		} else {
			trap_R_AddLightToScene( cent->lerpOrigin, i, 1.f, r, g, b, 0, 0 );
		}
	}

}

/*
==================
CG_Q3F_Goal

Handle sound, special effects etc.
==================
*/

void CG_Q3F_Goal( centity_t *cent )
{
	// Do the effects for the ent if desired

	refEntity_t			ent;
	entityState_t		*s1;
	vec3_t				dir;
	centity_t			*effectent;
	float				scale;
	int					time;

	s1 = &cent->currentState;

	if( s1->otherEntityNum >= MAX_CLIENTS )
	{
		// Goal is 'loose', render it normally

		if( cg_simpleItems.integer && s1->torsoAnim )
		{
			// Draw a simpleitem version of the ent.

			// create the render entity
			
			memset( &ent, 0, sizeof( ent ) );
			ent.reType = RT_SPRITE;
			VectorCopy( cent->lerpOrigin, ent.origin );
			scale = 0.005 + cent->currentState.number * 0.00001;
			ent.origin[2] += 8 + cos( ( cg.time + 1000 ) *  scale ) * 4;

			ent.radius = 14;
 			ent.customShader = cgs.gameShaders[s1->torsoAnim];

			time = cent->currentState.time;
			scale = 1.0f;
			if( time && time > cg.time && (time - Q3F_GOAL_SHRINKTIME) <= cg.time )
			{
					// Goalitem is about to go back to base
				scale = ((float)(time - cg.time)) / Q3F_GOAL_SHRINKTIME;
				ent.radius *= scale;
			}
			else {
				time = cent->currentState.time2;
				if( time && time <= cg.time && (time + Q3F_GOAL_SHRINKTIME) > cg.time )
				{
						// Goalitem has returned to base
					scale = ((float)(cg.time - time)) / Q3F_GOAL_SHRINKTIME;
					ent.radius *= scale;
				}
			}

			ent.shaderRGBA[0] = 255;
			ent.shaderRGBA[1] = 255;
			ent.shaderRGBA[2] = 255;
			ent.shaderRGBA[3] = (unsigned char) (255.0f * scale);
			trap_R_AddRefEntityToScene(&ent, cent);
		}
		else if( s1->modelindex )	// It has a model, right?
		{
			// calculate the axis
			VectorCopy( s1->angles, cent->lerpAngles);

			// create the render entity
			memset (&ent, 0, sizeof(ent));
			VectorCopy( cent->lerpOrigin, ent.origin);
			VectorCopy( cent->lerpOrigin, ent.oldorigin);
			ent.frame		= s1->frame;
			ent.oldframe	= ent.frame;
			ent.backlerp	= 0;

			VectorCopy( cent->lerpAngles, dir );
			if( cent->currentState.eFlags & EF_Q3F_ROTATING )
			{
				
				// Make the object rotate/bob as it moves
				scale = 0.005 + cent->currentState.number * 0.00001;
				ent.origin[2] += 4 + cos( ( cg.time + 1000 ) *  scale ) * 4;
				ent.oldorigin[2] = ent.origin[2];
				AxisCopy( cg.autoAxis, ent.axis );
			} else {
				// convert angles to axis
				AnglesToAxis( cent->lerpAngles, ent.axis );
			}

			if( 1 )
			{
				time = cent->currentState.time;
				if( time && time > cg.time && (time - Q3F_GOAL_SHRINKTIME) <= cg.time )
				{
					// Goalitem is about to go back to base
					scale = ((float)(time - cg.time)) / Q3F_GOAL_SHRINKTIME;
					VectorScale( ent.axis[0], scale, ent.axis[0] );
					VectorScale( ent.axis[1], scale, ent.axis[1] );
					VectorScale( ent.axis[2], scale, ent.axis[2] );
					ent.nonNormalizedAxes = qtrue;
				}
				else {
					time = cent->currentState.time2;
					if( time && time <= cg.time && (time + Q3F_GOAL_SHRINKTIME) > cg.time )
					{
						// Goalitem has returned to base
						scale = ((float)(cg.time - time)) / Q3F_GOAL_SHRINKTIME;
						VectorScale( ent.axis[0], scale, ent.axis[0] );
						VectorScale( ent.axis[1], scale, ent.axis[1] );
						VectorScale( ent.axis[2], scale, ent.axis[2] );
						ent.nonNormalizedAxes = qtrue;
					}
				}
			}
			ent.hModel = cgs.gameModels[s1->modelindex];
			trap_R_AddRefEntityToScene( &ent, cent );
		}
	}
	else {
		int i, clientNum;
		// Goal is carried - don't necessarily render it, just do the effects

		effectent = NULL;

		for( i = 0; i < MAX_CLIENTS; i++ ) {
			if((cg_entities[i].currentState.number == s1->otherEntityNum) && cg_entities[i].currentValid) {
				effectent = &cg_entities[i];
			}
		}

		if(!effectent) {
			return;
		}

		// Only render if parent is in pvs
		if ( !effectent->currentValid )
			return;

		clientNum = effectent->currentState.clientNum;

		// Don't bother doing any effects if we are a spectator of any kind as you would want to drop it upon team change anyway, unless you are doing something like BFF
		// Where you do not want to render the goal itself anyway but don't wish to see the sparkle at 0,0,0 or intermission origin.
		if ( cgs.clientinfo[clientNum].team == Q3F_TEAM_SPECTATOR || cgs.clientinfo[clientNum].cls == Q3F_CLASS_NULL || (effectent->currentState.eFlags & EF_Q3F_NOSPAWN))
			return;

		// update sound origins (lerpOrigin may be out of date)
		trap_S_UpdateEntityPosition( s1->number, effectent->lerpOrigin );

		// add loop sound
		if ( cent->currentState.loopSound ) {
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, 
				cgs.gameSounds[ cent->currentState.loopSound ], 255, 0 );
		}

		/* Skip rendering the sparkle if agent invisible */
		if ( !(effectent->currentState.eFlags & EF_Q3F_INVISIBLE) )
		{
			// Sparkly thing.
			if (s1->origin2[0] || s1->origin2[1] || s1->origin2[2] )
			{
				Spirit_SetCustomColor( s1->origin2 );
				Spirit_SetCustomShader( cgs.gameShaders[s1->modelindex2] );
				Spirit_RunScript( cgs.gameSpiritScript[s1->legsAnim], 
					cent->lerpOrigin, cent->lerpOrigin, axisDefault,
					(int)cent );
			}
		}

		// Trail the item behind its player
		if( ( s1->eFlags & EF_Q3F_SHOWCARRY ) && s1->modelindex &&
			( s1->otherEntityNum != cg.snap->ps.clientNum || cg.rendering2ndRefDef || cg.renderingThirdPerson ) )
			CG_TrailItem( effectent, cgs.gameModels[s1->modelindex] );
	}
}

/*
==================
CG_Q3F_Hud

Handle HUD icons
==================
*/

#define	Q3F_NUM_HUDPLAYERS	32
#define	HUDCOPY(w,x,y,z) if(z<Q3F_NUM_HUDPLAYERS){*w++ = *(y+(char *)x);z++;}
static void ExpandHUDData( centity_t *cent, unsigned char *ptr )
{
	int size, temp;
	entityState_t *es;

	es = &cent->currentState;

	size = 0;
	HUDCOPY( ptr, &es->time2,			0, size )
	HUDCOPY( ptr, &es->time2,			1, size )
	HUDCOPY( ptr, &es->time2,			2, size )
	HUDCOPY( ptr, &es->time2,			3, size )

	temp = 0;
	temp = es->origin[0];
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	temp = es->origin[1];
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	temp = es->origin[2];
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );

	temp = es->origin2[0];
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	temp = es->origin2[1];
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	temp = es->origin2[2];
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );

	temp = es->angles[0];
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	temp = es->angles[1];
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	temp = es->angles[2];
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );

	temp = es->angles2[0];
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	temp = es->angles2[0];
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	temp = es->angles2[0];
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );

	HUDCOPY( ptr, &es->powerups,				0, size )
	HUDCOPY( ptr, &es->powerups,				1, size )
	HUDCOPY( ptr, &es->otherEntityNum,			0, size )
	HUDCOPY( ptr, &es->otherEntityNum2,			0, size )
//	HUDCOPY( ptr, &es->modelindex2,				0, size )
	HUDCOPY( ptr, &es->legsAnim,				0, size )
	HUDCOPY( ptr, &es->torsoAnim,				0, size )
}

void CG_Q3F_Hud( centity_t *cent )
{
	// See if this ent is renderable, and store in slot for
	// rendering (after all packet entities are rendered)

	unsigned char *ptr;
	unsigned char buff[Q3F_NUM_HUDPLAYERS];
	int count;

	if( !cent->currentState.modelindex && !(cent->currentState.eFlags & EF_VOTED))
		return;		// No model
	if( !(cent->currentState.modelindex2 & (1 << cg.snap->ps.persistant[PERS_TEAM])) )
		return;		// Wrong team

	ExpandHUDData( cent, buff );
	ptr = buff;
	if( *ptr == 0xFF )		// No 'holding' data
	{
		cg.hudslots[cent->currentState.weapon] = cent;
		return;
	}

	for( count = sizeof(cent->currentState.angles) + sizeof(cent->currentState.angles); count; count--, ptr++)
	{
		if( *ptr == 0xFE )
			return;
		if( *ptr == cg.snap->ps.clientNum )
		{
			// We've a match
			cg.hudslots[cent->currentState.weapon] = cent;
			return;
		}
	}
}

/*
==================
CG_General
==================
*/
static void CG_General( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	
	// if set to invisible, skip
	if (!s1->modelindex) {
	return;
	}

	memset (&ent, 0, sizeof(ent));

	// set frame

	ent.frame = s1->frame;
	ent.oldframe = ent.frame;
	ent.backlerp = 0;

	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

//	ent.hModel = cgs.gameModels[s1->modelindex];
	if ( s1->solid == SOLID_BMODEL ) {
		ent.hModel = cgs.inlineDrawModel[s1->modelindex];
	} else {
		ent.hModel = cgs.gameModels[s1->modelindex];
	}

	// player model
	if (s1->number == cg.snap->ps.clientNum) {
		ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
	}

	// convert angles to axis
	AnglesToAxis( cent->lerpAngles, ent.axis );

	// add to refresh list
	trap_R_AddRefEntityToScene (&ent, cent);
}


/*
==================
CG_SniperDot
==================
*/
/*static void CG_ETF_DrawDot( qhandle_t markShader, const vec3_t origin, const vec3_t dir, 
				   float orientation, float red, float green, float blue, float alpha,
				   float radius, float nomark )
{
	vec3_t			axis[3];
	float			texCoordScale;
	vec3_t			originalPoints[4];
	vec5_t			markPoints[384];
	byte			colors[4];
	int				i, j;
	int				numFragments;
	markFragment_t	markFragments[384], *mf;
	vec3_t			projection;
	polyVert_t	*v;
	polyVert_t	verts[10];

	if ( radius <= 0 ) {
		CG_Error( "CG_ETF_DrawDot called with <= 0 radius" );
	}

	// create the texture axis
	VectorNormalize2( dir, axis[0] );
	PerpendicularVector( axis[1], axis[0] );
	RotatePointAroundVector( axis[2], axis[0], axis[1], orientation );
	CrossProduct( axis[0], axis[2], axis[1] );

	texCoordScale = 0.5 * 1.0 / radius;

	// create the full polygon
	for ( i = 0 ; i < 3 ; i++ ) {
		originalPoints[0][i] = origin[i] - radius * axis[1][i] - radius * axis[2][i];
		originalPoints[1][i] = origin[i] + radius * axis[1][i] - radius * axis[2][i];
		originalPoints[2][i] = origin[i] + radius * axis[1][i] + radius * axis[2][i];
		originalPoints[3][i] = origin[i] - radius * axis[1][i] + radius * axis[2][i];
	}

	// get the fragments
	VectorScale( dir, -1, projection );
	numFragments = trap_CM_MarkFragments( 4, (void *)originalPoints,
					projection, 384, markPoints[0],
					-128, markFragments );
	if( nomark || !numFragments )
	{
		numFragments = 1;
		memcpy( markPoints, originalPoints, sizeof(originalPoints) );
		markFragments[0].firstPoint = 0;
		markFragments[0].numPoints = 4;
	}

	colors[0] = red * 255;
	colors[1] = green * 255;
	colors[2] = blue * 255;
	colors[3] = alpha * 255;

	for ( i = 0, mf = markFragments; i < numFragments; i++, mf++ )
	{
		// we have an upper limit on the complexity of polygons
		// that we store persistantly
		if ( mf->numPoints > 10 ) {
			mf->numPoints = 10;
		}
		for ( j = 0, v = verts ; j < mf->numPoints ; j++, v++ )
		{
			vec3_t		delta;

			VectorCopy( markPoints[mf->firstPoint + j], v->xyz );

			VectorSubtract( v->xyz, origin, delta );
			v->st[0] = 0.5 + DotProduct( delta, axis[1] ) * texCoordScale;
			v->st[1] = 0.5 + DotProduct( delta, axis[2] ) * texCoordScale;
			*(int *)v->modulate = *(int *)colors;
		}

		trap_R_AddPolyToScene( markShader, mf->numPoints, verts );
	}
}*/

static vec3_t dotcolours[4] = {
	{ 1, 0, 0 },
	{ 0.3f, 0.3f, 1 },
	{ 1, 1, 0 },
	{ 0, 1, 0 },
};

static void CG_SniperDot( centity_t *cent ) {
	int team;
	vec3_t origin, dir, viewdir, endpos;
	vec4_t dotcolor;
	float size, fovfactor, f;
	trajectory_t t;
	trace_t tr;

	if( cent->nextState.eFlags & EF_NODRAW )
		return;		// Skip it, it's probably in the sky right now.

	if( cg_sniperDotColors.integer )
	{
		team = cgs.clientinfo[cent->nextState.otherEntityNum].team;
		if( team < Q3F_TEAM_RED || team > Q3F_TEAM_GREEN )
			team = Q3F_TEAM_RED;
		team -= Q3F_TEAM_RED;
	}
	else team = 0;

	AngleVectors( cg.predictedPlayerState.viewangles, viewdir, NULL, NULL );
	if( cent->nextState.otherEntityNum == cg.snap->ps.clientNum && !cg_sniperHistoricalSight.integer )
	{
		// It's _our_ dot, let's place it dead on

		VectorCopy( cg.predictedPlayerState.origin, origin );
		origin[2] += cg.predictedPlayerState.viewheight;
//		AngleVectors( cg.predictedPlayerState.viewangles, dir, NULL, NULL );
		VectorMA( origin, 14, viewdir, origin );
		VectorMA( origin, 10000, viewdir, endpos );
		CG_Trace( &tr, origin, NULL, NULL, endpos, cg.snap->ps.clientNum, MASK_SHOT );
//Use normal of surfaces we hit
		VectorCopy( tr.plane.normal, dir );
//Use vector from gun to the hitpoint
//		VectorSubtract(tr.endpos,origin,dir);
		VectorCopy( tr.endpos, origin );

		cg.sniperDotEnt = cent;		// Golliwog: Keep track for later.
	}
	else {
		VectorCopy( cent->nextState.pos.trBase, t.trBase );
		VectorCopy( cent->nextState.pos.trDelta, t.trDelta );
	//	VectorAdd( cent->currentState.angles2, cent->currentState.pos.trDelta, t.trDelta );
		t.trType = TR_LINEAR_STOP;
		t.trTime = cent->nextState.pos.trTime;
		t.trDuration = 500;		// Never move more than half a second ahead of the real pos.
		BG_EvaluateTrajectory( &t, cg.time, origin );

		AngleVectors( cent->nextState.angles, dir, NULL, NULL );
	}

		// Dot is (default) 4 units high at 100 units distant (seems to work, why argue? :)
	size = cg_sniperDotScale.value ? cg_sniperDotScale.value : 4;
	if( size > 0 )
	{
		// Scale the dot to fit
		if( size > 0.7f )
			size = 0.7f;

		if ( cg.zoomed )
			f = ( cg.time - cg.zoomTime ) / (float)ZOOM_TIME;
		else if( cg.autoZoomed )
			f = ( cg.time - cg.autoZoomTime ) / (float)AUTOZOOM_TIME;
		else f = 1.0f - ( cg.time - cg.zoomTime ) / (float)ZOOM_TIME;
		if( f > 1 )
			f = 1;
		else if( f < 0 )
			f = 0;
		fovfactor = cg_fov.integer - f * (float) (cg_fov.integer - cg_zoomFov.integer);

		VectorCopy( cg.snap->ps.origin, endpos );
		endpos[2] += cg.snap->ps.viewheight;
		f = Distance( endpos, origin );
		size = (fovfactor / /*cg_fov.integer*/90) * size * f * 0.05;	// 0.05 is scale to alter old 0.4 default to 0.02
	}
	else {
		size = -size;	// Just use an absolute dot size
		if( size > 15 )
			size = 15;
		VectorCopy( cg.snap->ps.origin, endpos );
		endpos[2] += cg.snap->ps.viewheight;
		f = Distance( endpos, origin );
	}

	if(	f < 20 &&
		DotProduct( dir, viewdir ) > 0.1 &&
		cent->nextState.otherEntityNum != cg.snap->ps.clientNum )
	{
		// The dot is 'on our face'

		cg.sniperWashColour[0] = dotcolours[team][0];
		cg.sniperWashColour[1] = dotcolours[team][1];
		cg.sniperWashColour[2] = dotcolours[team][2];
		cg.sniperWashColour[3] = 0.3f;
	}
	else {
		vec4_t projection;
		VectorNormalize(dir);
		VectorSubtract(vec3_origin, dir, projection );
		projection[ 3 ] = size ;
		VectorMA( origin, 1.0f , dir, origin );

		dotcolor[0]=dotcolours[team][0];
		dotcolor[1]=dotcolours[team][1];
		dotcolor[2]=dotcolours[team][2];
		dotcolor[3]=0.6f;

  		CG_DecalMark(cgs.media.sniperDot, origin, projection, 0, size, dotcolor, 1, 0);  //keeg for sniper
/*		CG_ETF_DrawDot(	cgs.media.sniperDot, origin, dir, 0, 
						dotcolours[team][0], dotcolours[team][1], dotcolours[team][2],
						0.5f, size, cent->nextState.torsoAnim );
*/

	}
}



/*
==================
CG_Flame
==================
*/
static void CG_Flame( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;
	memset (&ent, 0, sizeof(ent));
	ent.frame = s1->frame;
	ent.oldframe = ent.frame;
	ent.backlerp = 0;
	ent.reType = RT_SPRITE;
	ent.customShader = cgs.media.flameShader;
	ent.hModel = cgs.media.flameShader;
	ent.radius = 32;
	ent.skinNum = cg.clientFrame & 1;
	ent.renderfx = RF_NOSHADOW;
	AxisClear( ent.axis );
//	if (s1->number == cg.snap->ps.clientNum) {
//		ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
//	}
	// convert angles to axis
//	AnglesToAxis( cent->currentState.angles, ent.axis );

	VectorCopy( cent->lerpOrigin, ent.origin );
	VectorCopy( cent->lerpOrigin, ent.oldorigin );
		// add to refresh list
	trap_R_AddRefEntityToScene (&ent, cent);
}



/*
==================
CG_Speaker

Speaker entities can automatically play sounds
==================
*/
static void CG_Speaker( centity_t *cent ) {
	if ( ! cent->currentState.clientNum ) {	// FIXME: use something other than clientNum...
		return;		// not auto triggering
	}

	if ( cg.time < cent->miscTime ) {
		return;
	}

	trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.gameSounds[cent->currentState.eventParm] );

	//	ent->s.frame = ent->wait * 10;
	//	ent->s.clientNum = ent->random * 10;
	cent->miscTime = cg.time + cent->currentState.frame * 100 + cent->currentState.clientNum * 100 * Q_flrand(-1.0f, 1.0f);
}

/*
==================
CG_Item
==================
*/
static void CG_Item( centity_t *cent ) {
	refEntity_t		ent;
	entityState_t	*es;
	gitem_t			*item;
	int				msec;
	float			frac;
//	float			scale;
	weaponInfo_t	*wi;

	es = &cent->currentState;
	if ( es->modelindex >= bg_numItems ) {
		CG_Error( "Bad item index %i on entity", es->modelindex );
	}

	// if set to invisible, skip
	if ( !es->modelindex || ( es->eFlags & EF_NODRAW ) ) {
		return;
	}

	item = &bg_itemlist[ es->modelindex ];
	if ( cg_simpleItems.integer /*&& item->giType != IT_TEAM*/ ) {
		memset( &ent, 0, sizeof( ent ) );
		ent.reType = RT_SPRITE;
		VectorCopy( cent->lerpOrigin, ent.origin );
		ent.radius = 14;
		ent.customShader = cg_items[es->modelindex].icon_df;
		ent.shaderRGBA[0] = 255;
		ent.shaderRGBA[1] = 255;
		ent.shaderRGBA[2] = 255;
		ent.shaderRGBA[3] = 255;
		trap_R_AddRefEntityToScene(&ent, cent);
		return;
	}

/*	if( item->giType != IT_AMMO && item->giType != IT_Q3F_AMMOBOX && item->giType != IT_HEALTH ) {
		// items bob up and down continuously
		scale = 0.005 + cent->currentState.number * 0.00001;
		cent->lerpOrigin[2] += 4 + cos( ( cg.time + 1000 ) *  scale ) * 4;
	}*/

	memset (&ent, 0, sizeof(ent));

	// autorotate at one of two speeds
/*	if ( item->giType == IT_HEALTH ) {
		VectorCopy( cg.autoAnglesFast, cent->lerpAngles );
		AxisCopy( cg.autoAxisFast, ent.axis );
	} else {*/
	if( item->giType != IT_AMMO && item->giType != IT_Q3F_AMMOBOX && item->giType != IT_HEALTH ) {
		VectorCopy( cg.autoAngles, cent->lerpAngles );
		AxisCopy( cg.autoAxis, ent.axis );
	} else {
		AxisClear( ent.axis );
	}
//	}

	wi = NULL;
	// the weapons have their origin where they attatch to player
	// models, so we need to offset them or they will rotate
	// eccentricly
	if ( item->giType == IT_WEAPON ) {
		wi = &cg_weapons[item->giTag];
		cent->lerpOrigin[0] -= 
			wi->weaponMidpoint[0] * ent.axis[0][0] +
			wi->weaponMidpoint[1] * ent.axis[1][0] +
			wi->weaponMidpoint[2] * ent.axis[2][0];
		cent->lerpOrigin[1] -= 
			wi->weaponMidpoint[0] * ent.axis[0][1] +
			wi->weaponMidpoint[1] * ent.axis[1][1] +
			wi->weaponMidpoint[2] * ent.axis[2][1];
		cent->lerpOrigin[2] -= 
			wi->weaponMidpoint[0] * ent.axis[0][2] +
			wi->weaponMidpoint[1] * ent.axis[1][2] +
			wi->weaponMidpoint[2] * ent.axis[2][2];

		cent->lerpOrigin[2] += 8;	// an extra height boost
	}

	ent.hModel = cg_items[es->modelindex].models[0];

	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	ent.nonNormalizedAxes = qfalse;

	// if just respawned, slowly scale up
	msec = cg.time - cent->miscTime;
	if ( msec >= 0 && msec < ITEM_SCALEUP_TIME ) {
		frac = (float)msec / ITEM_SCALEUP_TIME;
		VectorScale( ent.axis[0], frac, ent.axis[0] );
		VectorScale( ent.axis[1], frac, ent.axis[1] );
		VectorScale( ent.axis[2], frac, ent.axis[2] );
		ent.nonNormalizedAxes = qtrue;
	} else {
		if( item->giType == IT_HEALTH ) {
			switch( item->quantity ) {
				case 5:
					frac = 0.5f;
					break;
				default:
				case 25:
					frac = 0.75f;
					break;
				case 50:
					frac = 1.f;
					break;
			}

			VectorScale( ent.axis[0], frac, ent.axis[0] );
			VectorScale( ent.axis[1], frac, ent.axis[1] );
			VectorScale( ent.axis[2], frac, ent.axis[2] );
			ent.nonNormalizedAxes = qtrue;
		} else {
			frac = 1.0;
		}
	}

	// items without glow textures need to keep a minimum light value
	// so they are always visible
	if ( ( item->giType == IT_WEAPON ) ||
		 ( item->giType == IT_ARMOR ) ) {
		ent.renderfx |= RF_MINLIGHT;
	}

	// increase the size of the weapons when they are presented as items
	if ( item->giType == IT_WEAPON ) {
		VectorScale( ent.axis[0], 1.5, ent.axis[0] );
		VectorScale( ent.axis[1], 1.5, ent.axis[1] );
		VectorScale( ent.axis[2], 1.5, ent.axis[2] );
		ent.nonNormalizedAxes = qtrue;
	}

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent, cent);

	// accompanying rings / spheres for powerups
	if ( !cg_simpleItems.integer ) 
	{
		vec3_t spinAngles;

		VectorClear( spinAngles );

		if ( /*item->giType == IT_HEALTH ||*/ item->giType == IT_POWERUP )
		{
			if ( ( ent.hModel = cg_items[es->modelindex].models[1] ) != 0 )
			{
				if ( item->giType == IT_POWERUP )
				{
					ent.origin[2] += 12;
					spinAngles[1] = ( cg.time & 1023 ) * 360 / -1024.0f;
				}
				AnglesToAxis( spinAngles, ent.axis );
				
				// scale up if respawning
				if ( frac != 1.0 ) {
					VectorScale( ent.axis[0], frac, ent.axis[0] );
					VectorScale( ent.axis[1], frac, ent.axis[1] );
					VectorScale( ent.axis[2], frac, ent.axis[2] );
					ent.nonNormalizedAxes = qtrue;
				}
				trap_R_AddRefEntityToScene( &ent, cent );
			}
		}
	}
}

static void CG_Missile( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;
	const weaponInfo_t	*weapon;

	s1 = &cent->currentState;
	if ( s1->weapon > WP_NUM_WEAPONS ) {
		s1->weapon = 0;
	}
	weapon = &cg_weapons[s1->weapon];

	// calculate the axis
	VectorCopy( s1->angles, cent->lerpAngles);

	// add trails
	if( weapon->missileTrailFunc ) {
		weapon->missileTrailFunc( cent, weapon );
	}

	// add dynamic light
	if ( weapon->missileDlight ) {
		trap_R_AddLightToScene(cent->lerpOrigin, weapon->missileDlight, 2.f,
			weapon->missileDlightColor[0], weapon->missileDlightColor[1], weapon->missileDlightColor[2], cgs.media.whiteAdditiveShader, 0 );
	}
	
	// add missile sound
	if ( weapon->missileSound ) {
		vec3_t	velocity;

		BG_EvaluateTrajectoryDelta( &cent->currentState.pos, cg.time, velocity );

		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, velocity, weapon->missileSound, 255, 0 );
	}
	

	// create the render entity
	memset (&ent, 0, sizeof(ent));

	// RR2DO2
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

	// flicker between two skins
	ent.skinNum = cg.clientFrame & 1;
	ent.hModel = weapon->missileModel;
	ent.renderfx = weapon->missileRenderfx | RF_NOSHADOW;

	// convert direction of travel into axis
	if(cent->currentState.weapon == WP_PIPELAUNCHER && s1->pos.trType == TR_STATIONARY )
	{
		if ( VectorNormalize2( s1->angles, ent.axis[0] ) == 0 )
			ent.axis[0][2] = 1;		
	}
	else
	{
		if ( VectorNormalize2( s1->pos.trDelta, ent.axis[0] ) == 0 )
			ent.axis[0][2] = 1;
		
	}

	// spin as it moves
	if ( s1->pos.trType != TR_STATIONARY ) {
		RotateAroundDirection( ent.axis, cg.time / 4 );
	} else {
		RotateAroundDirection( ent.axis, s1->time );
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
			team = 0;
		CG_AddWeaponWithPowerups( &ent, s1, team );
	}
}

/*
===============
CG_Mover
===============
*/
static void CG_Mover( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);
	AnglesToAxis( cent->lerpAngles, ent.axis );

	ent.renderfx = RF_NOSHADOW;

	// flicker between two skins (FIXME?)
	ent.skinNum = ( cg.time >> 6 ) & 1;

	// get the model, either as a bmodel or a modelindex
	if ( s1->solid == SOLID_BMODEL ) {
		ent.hModel = cgs.inlineDrawModel[s1->modelindex];
	} else {
		ent.hModel = cgs.gameModels[s1->modelindex];
	}

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent, cent);

	// add the secondary model
	if ( s1->modelindex2 ) {
		ent.skinNum = 0;
		ent.hModel = cgs.gameModels[s1->modelindex2];
		trap_R_AddRefEntityToScene(&ent, cent);
	}

}

/*
===============
CG_Beam

Also called as an event
===============
*/
void CG_Beam( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	VectorCopy( s1->pos.trBase, ent.origin );
	VectorCopy( s1->origin2, ent.oldorigin );
	AxisClear( ent.axis );
	ent.reType = RT_BEAM;

	ent.renderfx = RF_NOSHADOW;

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent, cent);
}

/* RR2DO2
===============
CG_Q3F_Beam

Also called as an event
===============
*/

void CG_Q3F_Beam( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;
	vec3_t				origin, origin2;

	s1 = &cent->currentState;

	BG_EvaluateTrajectory( &s1->pos, cg.time, origin );
	BG_EvaluateTrajectory( &s1->apos, cg.time, origin2 );

	// Straight beam
	if ( s1->otherEntityNum2 & Q3F_BEAM_STRAIGHT || s1->otherEntityNum < 1 ) { 
		// create the render entity
		memset (&ent, 0, sizeof(ent));

		VectorCopy( origin, ent.origin );
		VectorCopy( origin2, ent.oldorigin );
		AxisClear( ent.axis );
		ent.reType = RT_LIGHTNING;
		ent.customShader = cgs.gameShaders[s1->modelindex2];

		// add to refresh list
		trap_R_AddRefEntityToScene(&ent, cent);

		return;
	} else {
		vec3_t		p1, p2, v1, v2, dir, prevpt1, prevpt2, /*nextpt, mid,*/ delta, up;
		int			i,ii,jj;
		polyVert_t	points[4];
		float		length;
		//int			segnum = 0;
		//int			beamnum = 0;
		float		alphafactor;
		int			picW;
		int			seed;

		// Let's use some decent names
		float		angleVar = s1->legsAnim;
		int			numSubdivisions = s1->otherEntityNum;
		vec4_t		colour;
		float		scale = s1->torsoAnim;
		int			flags = s1->otherEntityNum2;
		float		speedscale = s1->angles[0];

		VectorCopy( s1->angles2, colour );
		colour[3] = 1.f;
	
		picW = 64;
		seed = 100;

		// calcluate length of beam segment
		VectorSubtract( origin2, origin, delta );
		length = VectorLength( delta );
		length /= numSubdivisions;

		// get the dir of beam
		VectorCopy( delta, dir );
		VectorNormalizeFast( dir );

		// Calculate the first up vector
		VectorSubtract( origin, cg.refdef.vieworg, v1 );
		VectorSubtract( origin2, cg.refdef.vieworg, v2 );
		CrossProduct( v1, v2, up );
		VectorNormalizeFast( up );
		
		// Calculate the first points
		VectorMA( origin, scale, up, prevpt1 );
		VectorMA( origin, -scale, up, prevpt2 );
		VectorCopy( origin, p1 );

		// go through and calculate each point of the beam and offset it by the anglevar
		for ( i = 1; i <= numSubdivisions; i++ ) {
			// Calculate the next point along the beam
			VectorMA( origin, i * length, dir, p2 );

			// Random variance on the next point ( except if it's the last )
			if ( i != numSubdivisions ) {
				if ( flags & Q3F_BEAM_WAVE_EFFECT ) {
					float phase = p2[0] + p2[1];

					p2[2] += sin( (phase + (float)cg.time) * speedscale ) * angleVar;
				} else if ( flags & Q3F_BEAM_WAVE_EFFECT_3D ) {
					float phase1 = p2[0] + p2[1];
					float phase2 = p2[0] + p2[2];
					float phase3 = p2[1] + p2[2];

					p2[0] += sin( (phase3 + (float)cg.time) * speedscale ) * angleVar;
					p2[1] += sin( (phase2 + (float)cg.time) * speedscale ) * angleVar;
					p2[2] += sin( (phase1 + (float)cg.time) * speedscale ) * angleVar;
				/*} else if ( flags & BEAM_USE_NOISE ) {
					p2.x += cgi.R_Noise( p2.x,p2.y,p2.z,cg.time ) * angleVar;
					p2.y += cgi.R_Noise( p2.x,p2.y,p2.z,cg.time ) * angleVar;
					p2.z += cgi.R_Noise( p2.x,p2.y,p2.z,cg.time ) * angleVar;*/
				} else {
					p2[0] += Q_crandom( &seed ) * angleVar;
					p2[1] += Q_crandom( &seed ) * angleVar;
					p2[2] += Q_crandom( &seed ) * angleVar;
				}
			}

			// Create the up vec for the beam which is parallel to the viewplane
			VectorSubtract( p1, cg.refdef.vieworg, v1 );
			VectorSubtract( p2, cg.refdef.vieworg, v2 );
			CrossProduct( v1, v2, up );
			VectorNormalizeFast( up );
      
			// Build the quad
			VectorMA( p2, scale, up, points[0].xyz );
			VectorCopy( prevpt1, points[1].xyz );
			VectorCopy( prevpt2, points[2].xyz );
			VectorMA( p2, -scale, up, points[3].xyz );

			 // Tile the shader across the beam
			if ( flags & Q3F_BEAM_TILESHADER ) {
				float startS = ( length * ( i-1 ) ) / (float)picW;
				float endS   = ( length * ( i ) )   / (float)picW;

				points[0].st[0] = startS; points[0].st[1] = 1;
				points[1].st[0] = endS;   points[1].st[1] = 1;
				points[2].st[0] = endS;   points[2].st[1] = 0;
				points[3].st[0] = startS; points[3].st[1] = 0;
			} else {
				points[0].st[0] = 1;   points[0].st[1] = 1;
				points[1].st[0] = 0;   points[1].st[1] = 1;
				points[2].st[0] = 0;   points[2].st[1] = 0;
				points[3].st[0] = 1;   points[3].st[1] = 0;
			}
				
			//if ( !alphastep )
				alphafactor = 1.0f;
			//else
			//	alphafactor = startalpha + (alphastep * i);

			// Set the color of the verts
			for ( ii=0; ii<4; ii++ ) {
				for ( jj=0; jj<4; jj++ ) {
					points[ii].modulate[jj] = colour[jj] * alphafactor;
				}
			}

			trap_R_AddPolyToScene( cgs.gameShaders[s1->modelindex2], 4, points );

			// Subtract off the overlap
			/*if ( overlap ) {
				p2 = p2 + ( dir * -overlap );
			}*/

			// Save off the last point to use as the first point on the next quad
			VectorMA( p2, scale, up, prevpt1 );
			VectorMA( p2, -scale, up, prevpt2 );
			VectorCopy( p2, p1 );
		}
	}
}
// RR2DO2


void CG_Q3F_Flamer( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;
	vec3_t				origin, origin2;

	s1 = &cent->currentState;

	BG_EvaluateTrajectory( &s1->pos, cg.time, origin );
	BG_EvaluateTrajectory( &s1->apos, cg.time, origin2 );

	// Straight beam
	if ( s1->otherEntityNum2 & Q3F_BEAM_STRAIGHT || s1->otherEntityNum < 1 ) { 
		// create the render entity
		memset (&ent, 0, sizeof(ent));

		VectorCopy( origin, ent.origin );
		VectorCopy( origin2, ent.oldorigin );
		AxisClear( ent.axis );
		ent.reType = RT_LIGHTNING;
		ent.customShader = cgs.gameShaders[s1->modelindex2];

		// add to refresh list
		trap_R_AddRefEntityToScene(&ent, cent);

		return;
	} else {
		vec3_t		p1, p2, v1, v2, dir, prevpt1, prevpt2, /*nextpt, mid,*/ delta, up;
		int			i,ii,jj;
		polyVert_t	points[4];
		float		length;
		//int			segnum = 0;
		//int			beamnum = 0;
		float		alphafactor;
		int			picW;
		int			seed;

		// Let's use some decent names
		float		angleVar = s1->legsAnim;
		int			numSubdivisions = s1->otherEntityNum;
		vec4_t		colour;
		float		scale = s1->torsoAnim;
		int			flags = s1->otherEntityNum2;
		float		speedscale = s1->angles[0];

		VectorCopy( s1->angles2, colour );
		colour[3] = 1.f;
	
		picW = 64;
		seed = 100;

		// calcluate length of beam segment
		VectorSubtract( origin2, origin, delta );
		length = VectorLength( delta );
		length /= numSubdivisions;

		// get the dir of beam
		VectorCopy( delta, dir );
		VectorNormalizeFast( dir );

		// Calculate the first up vector
		VectorSubtract( origin, cg.refdef.vieworg, v1 );
		VectorSubtract( origin2, cg.refdef.vieworg, v2 );
		CrossProduct( v1, v2, up );
		VectorNormalizeFast( up );
		
		// Calculate the first points
		VectorMA( origin, scale, up, prevpt1 );
		VectorMA( origin, -scale, up, prevpt2 );
		VectorCopy( origin, p1 );

		// go through and calculate each point of the beam and offset it by the anglevar
		for ( i = 1; i <= numSubdivisions; i++ ) {
			// Calculate the next point along the beam
			VectorMA( origin, i * length, dir, p2 );

			// Random variance on the next point ( except if it's the last )
			if ( i != numSubdivisions ) {
				if ( flags & Q3F_BEAM_WAVE_EFFECT ) {
					float phase = p2[0] + p2[1];

					p2[2] += sin( (phase + (float)cg.time) * speedscale ) * angleVar;
				} else if ( flags & Q3F_BEAM_WAVE_EFFECT_3D ) {
					float phase1 = p2[0] + p2[1];
					float phase2 = p2[0] + p2[2];
					float phase3 = p2[1] + p2[2];

					p2[0] += sin( (phase3 + (float)cg.time) * speedscale ) * angleVar;
					p2[1] += sin( (phase2 + (float)cg.time) * speedscale ) * angleVar;
					p2[2] += sin( (phase1 + (float)cg.time) * speedscale ) * angleVar;
				/*} else if ( flags & BEAM_USE_NOISE ) {
					p2.x += cgi.R_Noise( p2.x,p2.y,p2.z,cg.time ) * angleVar;
					p2.y += cgi.R_Noise( p2.x,p2.y,p2.z,cg.time ) * angleVar;
					p2.z += cgi.R_Noise( p2.x,p2.y,p2.z,cg.time ) * angleVar;*/
				} else {
					p2[0] += Q_crandom( &seed ) * angleVar;
					p2[1] += Q_crandom( &seed ) * angleVar;
					p2[2] += Q_crandom( &seed ) * angleVar;
				}
			}

			// Create the up vec for the beam which is parallel to the viewplane
			VectorSubtract( p1, cg.refdef.vieworg, v1 );
			VectorSubtract( p2, cg.refdef.vieworg, v2 );
			CrossProduct( v1, v2, up );
			VectorNormalizeFast( up );
      
			// Build the quad
			VectorMA( p2, scale, up, points[0].xyz );
			VectorCopy( prevpt1, points[1].xyz );
			VectorCopy( prevpt2, points[2].xyz );
			VectorMA( p2, -scale, up, points[3].xyz );

			 // Tile the shader across the beam
			if ( flags & Q3F_BEAM_TILESHADER ) {
				float startS = ( length * ( i-1 ) ) / (float)picW;
				float endS   = ( length * ( i ) )   / (float)picW;

				points[0].st[0] = startS; points[0].st[1] = 1;
				points[1].st[0] = endS;   points[1].st[1] = 1;
				points[2].st[0] = endS;   points[2].st[1] = 0;
				points[3].st[0] = startS; points[3].st[1] = 0;
			} else {
				points[0].st[0] = 1;   points[0].st[1] = 1;
				points[1].st[0] = 0;   points[1].st[1] = 1;
				points[2].st[0] = 0;   points[2].st[1] = 0;
				points[3].st[0] = 1;   points[3].st[1] = 0;
			}
				
			//if ( !alphastep )
				alphafactor = 1.0f;
			//else
			//	alphafactor = startalpha + (alphastep * i);

			// Set the color of the verts
			for ( ii=0; ii<4; ii++ ) {
				for ( jj=0; jj<4; jj++ ) {
					points[ii].modulate[jj] = colour[jj] * alphafactor;
				}
			}

			trap_R_AddPolyToScene( cgs.gameShaders[s1->modelindex2], 4, points );

			// Subtract off the overlap
			/*if ( overlap ) {
				p2 = p2 + ( dir * -overlap );
			}*/

			// Save off the last point to use as the first point on the next quad
			VectorMA( p2, scale, up, prevpt1 );
			VectorMA( p2, -scale, up, prevpt2 );
			VectorCopy( p2, p1 );
		}
	}
}


/*
===============
CG_Portal
===============
*/

static void CG_Portal( centity_t *cent ) {
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// create the render entity
	memset (&ent, 0, sizeof(ent));
	if ( s1->legsAnim >= 0 && s1->legsAnim < Q3F_MAX_PATHS ) {
		vec3_t nullvec;
		VectorSet(nullvec, 0, 0, 0);

		if (cgs.campaths[s1->legsAnim].camtraj.trTime + cgs.campaths[s1->legsAnim].camtraj.trDuration < cg.time) {
			vec3_t vec_angle;
			cgs.campaths[s1->legsAnim].currtrajindex++;
			if ( cgs.campaths[s1->legsAnim].currtrajindex >= cgs.campaths[s1->legsAnim].numsplines )
				cgs.campaths[s1->legsAnim].currtrajindex = 0;
			VectorCopy( cgs.campaths[s1->legsAnim].splines[cgs.campaths[s1->legsAnim].currtrajindex].SegmentVtx[0], cgs.campaths[s1->legsAnim].camtraj.trBase );
			cgs.campaths[s1->legsAnim].camtraj.trDuration = BG_Q3F_CubicSpline_Length(&cgs.campaths[s1->legsAnim].splines[cgs.campaths[s1->legsAnim].currtrajindex])/cgs.campaths[s1->legsAnim].camsplines[cgs.campaths[s1->legsAnim].currtrajindex].speed * 1000;
			cgs.campaths[s1->legsAnim].camtraj.trTime = cg.time;
			cgs.campaths[s1->legsAnim].camtraj.trType = TR_CUBIC_SPLINE_PATH;
			VectorCopy( cgs.campaths[s1->legsAnim].camtraj.trBase, ent.origin );
			VectorCopy( s1->origin2, ent.oldorigin );
			VectorSubtract ( cgs.campaths[s1->legsAnim].splines[cgs.campaths[s1->legsAnim].currtrajindex].ControlPoint[1], cgs.campaths[s1->legsAnim].splines[cgs.campaths[s1->legsAnim].currtrajindex].ControlPoint[0], vec_angle );
			vectoangles( vec_angle, vec_angle );
			vec_angle[ROLL] = cgs.campaths[s1->legsAnim].camsplines[cgs.campaths[s1->legsAnim].currtrajindex].roll;
			AnglesToAxis( vec_angle, ent.axis );
			ent.frame = 0;
			ent.skinNum = 0;
		} else {
			vec3_t pos, vec_angle;

			BG_Q3F_EvaluateSplineTrajectory( &cgs.campaths[s1->legsAnim].camtraj, NULL, &cgs.campaths[s1->legsAnim].splines[cgs.campaths[s1->legsAnim].currtrajindex], cg.time, pos );
			VectorCopy( pos, ent.origin );
			VectorCopy( s1->origin2, ent.oldorigin );
			BG_Q3F_EvaluateSplineTrajectoryAngle( &cgs.campaths[s1->legsAnim].camtraj, NULL, &cgs.campaths[s1->legsAnim].splines[cgs.campaths[s1->legsAnim].currtrajindex], cg.time, vec_angle );
			vectoangles( vec_angle, vec_angle );
			
			vec_angle[ROLL] = (float)cgs.campaths[s1->legsAnim].camsplines[cgs.campaths[s1->legsAnim].currtrajindex].roll +
								( ((float)cg.time - (float)cgs.campaths[s1->legsAnim].camtraj.trTime) / (float)cgs.campaths[s1->legsAnim].camtraj.trDuration *
								AngleNormalize180((float)(cgs.campaths[s1->legsAnim].camsplines[cgs.campaths[s1->legsAnim].currtrajindex].next->roll - cgs.campaths[s1->legsAnim].camsplines[cgs.campaths[s1->legsAnim].currtrajindex].roll)) );

			AnglesToAxis( vec_angle, ent.axis );
			ent.frame = 0;
			ent.skinNum = 0;
		}
//		CG_SmokePuff( ent.origin, nullvec, 8.f, 1.f, 0.f, 0.f, 1.f, 100, 0, RF_MINLIGHT|RF_NOSHADOW, cgs.media.hastePuffShader );
//		CG_SmokePuff( ent.oldorigin, nullvec, 8.f, 0.f, 1.f, 0.f, 1.f, 100, 0, RF_MINLIGHT|RF_NOSHADOW, cgs.media.hastePuffShader );
	} else {
		VectorCopy( cent->lerpOrigin, ent.origin );				// Portal position
		VectorCopy( s1->origin2, ent.oldorigin );				// Target position
		ByteToDir( s1->eventParm, ent.axis[0] );				// Target view direction
		PerpendicularVector( ent.axis[1], ent.axis[0] );

		// negating this tends to get the directions like they want
		// we really should have a camera roll value
		VectorSubtract( vec3_origin, ent.axis[1], ent.axis[1] );

		CrossProduct( ent.axis[0], ent.axis[1], ent.axis[2] );
		ent.frame = s1->frame;		// rotation speed
		ent.skinNum = s1->clientNum/256.0 * 360;	// roll offset
	}
	ent.reType = RT_PORTALSURFACE;
	ent.oldframe = s1->powerups;

	// add to refresh list
	trap_R_AddRefEntityToScene(&ent, cent);
}

/*
=========================
CG_AdjustPositionForMover

Also called by client movement prediction code
=========================
*/
void CG_AdjustPositionForMover( const vec3_t in, int moverNum, int fromTime, int toTime, vec3_t out ) {
	centity_t	*cent;
	vec3_t	oldOrigin, origin, deltaOrigin;
	vec3_t	oldAngles, angles, deltaAngles;

	if ( moverNum <= 0 || moverNum >= ENTITYNUM_MAX_NORMAL ) {
		VectorCopy( in, out );
		return;
	}

	cent = &cg_entities[ moverNum ];
	if ( cent->currentState.eType != ET_MOVER ) {
	//if ( cent->currentState.eType != ET_MOVER && cent->currentState.eType != ET_Q3F_CHILD_MOVER) { // RR2DO2
		VectorCopy( in, out );
		return;
	}

	BG_EvaluateTrajectory( &cent->currentState.pos, fromTime, oldOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, fromTime, oldAngles );

	BG_EvaluateTrajectory( &cent->currentState.pos, toTime, origin );
	BG_EvaluateTrajectory( &cent->currentState.apos, toTime, angles );

	VectorSubtract( origin, oldOrigin, deltaOrigin );
	VectorSubtract( angles, oldAngles, deltaAngles );

	VectorAdd( in, deltaOrigin, out );
}


/*
=============================
CG_InterpolateEntityPosition
=============================
*/
static void CG_InterpolateEntityPosition( centity_t *cent ) {
	vec3_t		current, next;
	float		f;

	f = cg.frameInterpolation;

	// this will linearize a sine or parabolic curve, but it is important
	// to not extrapolate player positions if more recent data is available
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, current );
	BG_EvaluateTrajectory( &cent->nextState.pos, cg.nextSnap->serverTime, next );

	cent->lerpOrigin[0] = current[0] + f * ( next[0] - current[0] );
	cent->lerpOrigin[1] = current[1] + f * ( next[1] - current[1] );
	cent->lerpOrigin[2] = current[2] + f * ( next[2] - current[2] );

	BG_EvaluateTrajectory( &cent->currentState.apos, cg.snap->serverTime, current );
	BG_EvaluateTrajectory( &cent->nextState.apos, cg.nextSnap->serverTime, next );

	cent->lerpAngles[0] = LerpAngle( current[0], next[0], f );
	cent->lerpAngles[1] = LerpAngle( current[1], next[1], f );
	cent->lerpAngles[2] = LerpAngle( current[2], next[2], f );
}

/*
=============================
CG_InterpolateEntityPosition
=============================
*/
static void CG_InterpolateEntityAngle( centity_t *cent ) {
	vec3_t		current, next;
	float		f;

	f = cg.frameInterpolation;

	BG_EvaluateTrajectory( &cent->currentState.apos, cg.snap->serverTime, current );
	BG_EvaluateTrajectory( &cent->nextState.apos, cg.nextSnap->serverTime, next );

	cent->lerpAngles[0] = LerpAngle( current[0], next[0], f );
	cent->lerpAngles[1] = LerpAngle( current[1], next[1], f );
	cent->lerpAngles[2] = LerpAngle( current[2], next[2], f );
}


/*
===============
CG_CalcEntityLerpPositions

===============
*/
static void CG_CalcEntityLerpPositions( centity_t *cent ) {

	int timeshift = 0;

	if ( cent->interpolate && cent->currentState.pos.trType == TR_INTERPOLATE ) {
		CG_InterpolateEntityPosition( cent );
		return;
	}

	// first see if we can interpolate between two snaps for
	// linear extrapolated clients
	if ( cent->interpolate && cent->currentState.pos.trType == TR_LINEAR_STOP && cent->currentState.number < MAX_CLIENTS) {
		CG_InterpolateEntityPosition( cent );
		return;
	}

	// interpolating failed (probably no nextSnap), so extrapolate
	// this can also happen if the teleport bit is flipped, but that
	// won't be noticeable
	if ( cent->currentState.number < MAX_CLIENTS &&
			cent->currentState.clientNum != cg.predictedPlayerState.clientNum ) {
		cent->currentState.pos.trType = TR_LINEAR_STOP;
		cent->currentState.pos.trTime = cg.snap->serverTime;
		cent->currentState.pos.trDuration = 1000 / cgs.sv_fps;
	}

	// just use the current frame and evaluate as best we can
	BG_EvaluateTrajectory( &cent->currentState.pos, cg.time + timeshift, cent->lerpOrigin );
	if (cent->interpolate && cent->currentState.apos.trType == TR_INTERPOLATE )
		CG_InterpolateEntityAngle( cent );
	else
		BG_EvaluateTrajectory( &cent->currentState.apos, cg.time + timeshift, cent->lerpAngles );

	// adjust for riding a mover if it wasn't rolled into the predicted
	// player state
	if ( cent != &cg.predictedPlayerEntity ) {
		CG_AdjustPositionForMover( cent->lerpOrigin, cent->currentState.groundEntityNum, 
		cg.snap->serverTime, cg.time, cent->lerpOrigin );
	}
}

/*
===============
CG_Q3F_ForceField
===============
*/
void CG_Q3F_ForceField( centity_t *cent )
{
	refEntity_t			ent;
	entityState_t		*s1;

	s1 = &cent->currentState;

	// if set to invisible, skip
	if (!s1->modelindex) {
		return;
	}

	memset (&ent, 0, sizeof(ent));

	// set frame

	ent.frame = s1->frame;
	ent.oldframe = ent.frame;
	ent.backlerp = 0;

	VectorCopy( cent->lerpOrigin, ent.origin);
	VectorCopy( cent->lerpOrigin, ent.oldorigin);

//	ent.hModel = cgs.gameModels[s1->modelindex];
	if ( s1->solid == SOLID_BMODEL ) {
		ent.hModel = cgs.inlineDrawModel[s1->modelindex];
	} else {
		ent.hModel = cgs.gameModels[s1->modelindex];
	}

	// player model
	if (s1->number == cg.snap->ps.clientNum) {
		ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
	}

	// convert angles to axis
	AnglesToAxis( cent->lerpAngles, ent.axis );

	// add to refresh list
	trap_R_AddRefEntityToScene (&ent, cent);
}

/*
===============
CG_Q3F_Visibility
===============
*/
void CG_Q3F_Visibility(centity_t * cent)
{
	refEntity_t     ent;
	entityState_t  *s1;
	vec3_t          eyepos;

	s1 = &cent->currentState;

	// if set to invisible, skip
	if(!s1->modelindex)
	{
		return;
	}

	// If the panel is visible at all, regardless of orientation.
	// If eyepos->panelorigin . panelnormal > 0 then the player
	// is 'behind' the panel and it must therefore have it's normal reversed.

	// Don't bother rendering if it's facing the wrong way.
	VectorCopy(cg.currentrefdef->vieworg, eyepos);

	if(!trap_R_inPVS(eyepos, s1->origin))
	{
		if(Distance(s1->origin, eyepos) <= s1->origin2[0]) // maxdist
			cent->trailTime = cg.time;
		else
			cent->dustTrailTime = cg.time;
		return;
	}

	if(!(s1->frame & (1 << cg.snap->ps.persistant[PERS_CURRCLASS])) ||
		!(s1->otherEntityNum2 & (1 << cg.snap->ps.persistant[PERS_TEAM])))
	{
		cent->dustTrailTime = cg.time;
		return;
	}

	memset(&ent, 0, sizeof(ent));

	// set frame

	ent.frame = s1->frame;
	ent.oldframe = ent.frame;
	ent.backlerp = 0;

	ent.shaderRGBA[3] = 0xff;

	if(Distance(s1->origin, eyepos) > s1->origin2[0])
	{
		// We're out of range - render shutdown effect if required.
		if(cent->trailTime && (cg.time - cent->trailTime) < 1000)
		{
			// Time for the shutdown effect.

			if(s1->groundEntityNum != 0)
			{
				ent.shaderRGBA[3] = 0xff * (1.0f - (cg.time - cent->trailTime) / 1000.0f);
				VectorCopy(cent->lerpOrigin, ent.origin);
				VectorCopy(cent->lerpOrigin, ent.oldorigin);

			//  ent.hModel = cgs.gameModels[s1->modelindex];
				if(s1->solid == SOLID_BMODEL)
				{
					ent.hModel = cgs.inlineDrawModel[s1->modelindex];
				}
				else
				{
					ent.hModel = cgs.gameModels[s1->modelindex];
				}

				// player model
				if(s1->number == cg.snap->ps.clientNum)
				{
					ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
				}

				// convert angles to axis
				AnglesToAxis(cent->lerpAngles, ent.axis);

				// add to refresh list
				trap_R_AddRefEntityToScene(&ent, cent);
				ent.shaderRGBA[3] = 0xff;
			}
		}
		cent->dustTrailTime = cg.time;
		return;
	}
	else
	{
		// We're in range - render startup effect if required.

		if(cent->dustTrailTime && (cg.time - cent->dustTrailTime) < 1000)
		{
			// Time for the startup effect.

			if(s1->groundEntityNum != 0)
			{
				ent.shaderRGBA[3] = 0xff * ((cg.time - cent->dustTrailTime) / 1000.0);
				VectorCopy(cent->lerpOrigin, ent.origin);
				VectorCopy(cent->lerpOrigin, ent.oldorigin);

			//  ent.hModel = cgs.gameModels[s1->modelindex];
				if(s1->solid == SOLID_BMODEL)
				{
					ent.hModel = cgs.inlineDrawModel[s1->modelindex];
				}
				else
				{
					ent.hModel = cgs.gameModels[s1->modelindex];
				}

				// player model
				if(s1->number == cg.snap->ps.clientNum)
				{
					ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
				}

				// convert angles to axis
				AnglesToAxis(cent->lerpAngles, ent.axis);

				// add to refresh list
				trap_R_AddRefEntityToScene(&ent, cent);
				ent.shaderRGBA[3] = 0xff;
			}
		}
		else
		{
			// Just render normally.
			ent.shaderRGBA[3] = 0xff;
			VectorCopy(cent->lerpOrigin, ent.origin);
			VectorCopy(cent->lerpOrigin, ent.oldorigin);

		//  ent.hModel = cgs.gameModels[s1->modelindex];
			if(s1->solid == SOLID_BMODEL)
			{
				ent.hModel = cgs.inlineDrawModel[s1->modelindex];
			}
			else
			{
				ent.hModel = cgs.gameModels[s1->modelindex];
			}

			// player model
			if(s1->number == cg.snap->ps.clientNum)
			{
				ent.renderfx |= RF_THIRD_PERSON;	// only draw from mirrors
			}

			// convert angles to axis
			AnglesToAxis(cent->lerpAngles, ent.axis);

			// add to refresh list
			trap_R_AddRefEntityToScene(&ent, cent);
		}
		cent->trailTime = cg.time;
		return;
	}
}

/*
===============
CG_Q3F_TeleportTransition
===============
*/
void CG_Q3F_TeleportTransition( centity_t *cent )
{
	// Draw a 'transition' shader over the viewscreen to allow fade-through teleports.

	float alpha, width, height, depth;
	polyVert_t verts[4];
	unsigned char crgba[4];
	qhandle_t	shader;
	vec3_t forward, right, up;
	refEntity_t ent;

		// Check we should display the effect, and can find the shader.
	if( (alpha = 1 - 0.1 * 0.001 * (cg.time - cent->currentState.time)) <= 0 )
		return;
	if( !(shader = trap_R_RegisterShader( "teletrans" )) )
		return;

		// Calculate the values required to fill the screen
	depth	= 20;	// r_znear is usually 4, it will break our effect if the player sets it higher.
	width	= tan( cg.refdef.fov_x * (M_PI / 360) ) * depth * 0.5;
	height	= tan( cg.refdef.fov_y * (M_PI / 360) ) * depth * 0.5;

		// Calculate the vertices.
	AngleVectors( cg.refdefViewAngles, forward, right, up );
	VectorMA( cg.refdef.vieworg,	depth,			forward,	verts[0].xyz );
	VectorMA( verts[0].xyz,			-width,			right,		verts[0].xyz );
	VectorMA( verts[0].xyz,			height,			up,			verts[0].xyz );
	VectorMA( verts[0].xyz,			2 * width,		right,		verts[1].xyz );
	VectorMA( verts[1].xyz,			-2 * height,	up,			verts[2].xyz );
	VectorMA( verts[2].xyz,			-2 * width,		right,		verts[3].xyz );
	verts[0].st[0] = verts[0].st[1] = verts[1].st[1] = verts[3].st[0] = 0;
	verts[1].st[0] = verts[2].st[0] = verts[2].st[1] = verts[3].st[1] = 1;

		// Set the rgba values on each vertex.
	crgba[0] = crgba[1] = crgba[2] = 0xFF;
	crgba[3] = (unsigned char) (255.0f * alpha);
	*((int *) verts[0].modulate) = *(int *) crgba;
	*((int *) verts[1].modulate) = *(int *) crgba;
	*((int *) verts[2].modulate) = *(int *) crgba;
	*((int *) verts[3].modulate) = *(int *) crgba;

		// And draw.
	trap_R_AddPolyToScene( shader, 4, verts );

		// Then add the portal entity.
	memset( &ent, 0, sizeof(ent) );
	VectorMA( cg.refdef.vieworg, depth - 1, forward, ent.origin );	// Portal position
//	VectorCopy( cent->currentState.origin2, ent.oldorigin );		// Target position
	VectorCopy( ent.origin, ent.oldorigin );						// Generate a mirror
	VectorCopy( cent->currentState.angles, ent.axis[0] );			// Target view direction
	PerpendicularVector( ent.axis[1], ent.axis[0] );
	VectorSubtract( vec3_origin, ent.axis[1], ent.axis[1] );		// negating this tends to get the directions like they want
	CrossProduct( ent.axis[0], ent.axis[1], ent.axis[2] );
	ent.reType = RT_PORTALSURFACE;
//	ent.shaderRGBA[0] = ent.shaderRGBA[1] = ent.shaderRGBA[2] = ent.shaderRGBA[3] = 0xFF;
	trap_R_AddRefEntityToScene( &ent, cent );
}

/*
===============
CG_Q3F_SentryCam
===============
*/
void CG_Q3F_SentryCam( centity_t *cent ) {
	entityState_t		*s1;

	s1 = &cent->currentState;

	VectorCopy( cent->lerpOrigin, cg.sentrycam_origin );
	VectorCopy( cent->lerpAngles, cg.sentrycam_angles );
	cg.sentrycam_entityState = s1;
}

/*
===============
CG_AddCEntity

===============
*/
static void CG_AddCEntity( centity_t *cent ) {
	// event-only entities will have been dealt with already
	if( cent->currentState.eType >= ET_EVENTS ) {
		return;
	}

	// calculate the current origin
	CG_CalcEntityLerpPositions( cent );

	// Golliwog: Don't render anything that's not visible on this side / position
	if( cg.currentrefdef != &cg.refdef && cent->currentState.eType != ET_MOVER )
		cg.drawFilter = !trap_R_inPVS( cg.currentrefdef->vieworg, cent->lerpOrigin );
	else
		cg.drawFilter = qfalse;

	// add automatic effects
	// RR2DO2: HACK HACK TO FREE SOME INTEGERS!
	if ( cent->currentState.eType != ET_Q3F_GRENADE && cent->currentState.eType != ET_Q3F_PANEL )
		CG_EntityEffects( cent );

	switch ( cent->currentState.eType ) {
	default:
		CG_Error( "Bad entity type: %i", cent->currentState.eType );
		break;
	case ET_INVISIBLE:
	case ET_PUSH_TRIGGER:
	case ET_TELEPORT_TRIGGER:
	case ET_Q3F_AGENTDATA:
		break;
	case ET_Q3F_SCANNERDATA:
		CG_Q3F_CheckForScannerData(cent);
		break;
	case ET_GENERAL:
		CG_General( cent );
		break;
	case ET_SNIPER_DOT:
		CG_SniperDot( cent );
		break;
	case ET_FLAME:
		if( !cg.renderingSkyPortal )
			CG_Flame(cent);
		break;
	case ET_PLAYER:
		if( !cg.renderingSkyPortal ) {
			CG_Player( cent );
		}
		break;
	case ET_ITEM:
		CG_Item( cent );
		break;
	case ET_MISSILE:
		// RR2DO2
		/*if ( cent->oldOriginTime < cg.oldTime ) {
		//if ( cent->oldOriginTime < cg.oldTime || cent->oldOriginTime > cg.time ) {
			cent->oldOriginTime = cent->currentState.pos.trTime;
			VectorCopy( cent->currentState.pos.trBase, cent->oldOrigin );
		}*/
		// RR2DO2
		if( !cg.renderingSkyPortal )
			CG_Missile( cent );
		break;
	// Golliwog: Custom rendering on these entities
	case ET_Q3F_GRENADE:
		if( !cg.renderingSkyPortal )
			CG_Q3F_Grenade( cent );
		break;
	case ET_Q3F_GOAL:
		CG_Q3F_Goal( cent );
		break;
	case ET_Q3F_HUD:
		CG_Q3F_Hud( cent );
		break;
	case ET_Q3F_SENTRY:
		if( !cg.renderingSkyPortal )
			CG_Q3F_Sentry( cent );
		break;
	case ET_Q3F_SUPPLYSTATION:
		if( !cg.renderingSkyPortal ) 
			CG_Q3F_Supplystation( cent );
		break;

	// Golliwog.
	case ET_MOVER:
		CG_Mover( cent );
		break;
	// RR2DO2
	/*case ET_Q3F_CHILD_MOVER:
		CG_Child_Mover( cent );
		break;*/
	// RR2DO2
	case ET_BEAM:
		CG_Beam( cent );
		break;
	case ET_PORTAL:
		CG_Portal( cent );
		break;
	case ET_SPEAKER:
		CG_Speaker( cent );
		break;
/*	case ET_GRAPPLE:
		CG_Grapple( cent );
		break;*/
/*	case ET_TEAM:
		CG_TeamBase( cent );
		break;*/
	// RR2DO2
	case ET_Q3F_BEAM:
		CG_Q3F_Beam( cent );
		break;
	case ET_Q3F_MAPSENTRY:
		CG_Q3F_MapSentry( cent );
		break;
	case ET_Q3F_SENTRYCAM:
		CG_Q3F_SentryCam( cent );
		break;
	case ET_Q3F_SKYPORTAL:	// This ent doesn't do anything really
		break;
	// RR2DO2
	// Golliwog
	case ET_Q3F_PANEL:
		CG_Q3F_Panel( cent );
		break;
	case ET_Q3F_FORCEFIELD:
		CG_Q3F_ForceField( cent );
		break;
	case ET_Q3F_TELEPORTTRANSITION:
		CG_Q3F_TeleportTransition( cent );
		break;
	case ET_Q3F_VISIBILITY:
		CG_Q3F_Visibility(cent);
		break;
	}
	// Golliwog.

	cg.drawFilter = qfalse;
}

/*
===============
CG_AddPacketEntities

===============
*/
void CG_AddPacketEntities( void ) {
	int					num;
	centity_t			*cent;
	playerState_t		*ps;

	// set cg.frameInterpolation
	if ( cg.nextSnap ) {
		int		delta;

		delta = (cg.nextSnap->serverTime - cg.snap->serverTime);
		if ( delta == 0 ) {
			cg.frameInterpolation = 0;
		} else {
			cg.frameInterpolation = (float)( cg.time - cg.snap->serverTime ) / delta;
		}
	} else {
		cg.frameInterpolation = 0;	// actually, it should never be used, because 
									// no entities should be marked as interpolating
	}

	// the auto-rotating items will all have the same axis
	cg.autoAngles[0] = 0;
	cg.autoAngles[1] = ( cg.time & 2047 ) * 360 / 2048.0;
	cg.autoAngles[2] = 0;

	cg.autoAnglesFast[0] = 0;
	cg.autoAnglesFast[1] = ( cg.time & 1023 ) * 360 / 1024.0f;
	cg.autoAnglesFast[2] = 0;

	AnglesToAxis( cg.autoAngles, cg.autoAxis );
	AnglesToAxis( cg.autoAnglesFast, cg.autoAxisFast );

	// generate and add the entity from the playerstate
	ps = &cg.predictedPlayerState;
	BG_PlayerStateToEntityState( ps, &cg.predictedPlayerEntity.currentState, qfalse );
	CG_AddCEntity( &cg.predictedPlayerEntity );

	// lerp the non-predicted value for lightning gun origins
	CG_CalcEntityLerpPositions( &cg_entities[ cg.snap->ps.clientNum ] );

	// add each entity sent over by the server
	for ( num = 0 ; num < cg.snap->numEntities ; num++ ) {
		cent = &cg_entities[ cg.snap->entities[ num ].number ];
		CG_AddCEntity( cent );
	}

	//djbob: reset sentry cam fx check
	if(cg.sentrycam_entityState == NULL)
	{
		if(cg.sentryCam_on)
		{
			cg.sentryCam_on = qfalse;
			cg.sentryCamTime_end = cg.time;
		}
	}
	//djbob

	// Golliwog: Stop any artificial render blocks.
	cg.drawFilter = qfalse;

   // Keeg: flamethrower sounds from ET  FT_NEW
	CG_UpdateFlamethrowerSounds();
}

