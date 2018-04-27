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

#include "cg_local.h"
#include "../game/bg_q3f_grenades.h"

/*
=======================
CG_PredictWeaponEffects

Draws predicted effects for the railgun, shotgun, and machinegun.  The
lightning gun is done in CG_LightningBolt, since it was just a matter
of setting the right origin and angles.
=======================
*/
void CG_PredictWeaponEffects( centity_t *cent ) {
	vec3_t	muzzlePoint, forward, endPoint;
	vec3_t	viewAngles;
	trace_t tr;
	qboolean flesh;
	int fleshNum;
	entityState_t *ent = &cent->currentState;

	// if the client isn't us, forget it
	if ( cent->currentState.number != cg.predictedPlayerState.clientNum ) {
		return;
	}

	// if it's not switched on server-side, forget it
	if ( !cg_predictWeapons.integer ) {
		return;
	}

	// get the muzzle point
	VectorCopy( cg.predictedPlayerState.origin, muzzlePoint );
	muzzlePoint[2] += cg.predictedPlayerState.viewheight;

	// get forward, right, and up
	if( cg.snap->ps.powerups[PW_Q3F_CONCUSS] >= cg.time ) {
		BG_Q3F_ConcussionEffect( cg.snap->ps.generic1, cg.snap->ps.powerups[PW_Q3F_CONCUSS] - cg.time, viewAngles );
		VectorAdd( cg.predictedPlayerState.viewangles, viewAngles, viewAngles );
		AngleVectors( viewAngles, forward, NULL, NULL );
	} else {
		AngleVectors( cg.predictedPlayerState.viewangles, forward, NULL, NULL );
	}
	VectorMA( muzzlePoint, 14, forward, muzzlePoint );

	if ( ent->weapon == WP_SHOTGUN ) {
		VectorScale( forward, 4096, endPoint );
		SnapVector( endPoint );
		SnapVector( muzzlePoint );
		CG_SingleShotgunPattern( muzzlePoint, endPoint , ent->clientNum, cg.oldTime % 256 );
	} else if ( ent->weapon == WP_SUPERSHOTGUN ) {
		VectorScale( forward, 4096, endPoint );
		SnapVector( endPoint );
		SnapVector( muzzlePoint );
		CG_ShotgunPattern( muzzlePoint, endPoint , ent->clientNum, cg.oldTime % 256 );
	} else if ( ent->weapon == WP_MINIGUN )  {
		int heat = cg.minigunHeat - ((cg.time - cg.minigunLast)*15) / 5000;
		if (heat < 0 )
			heat = 0;
		VectorScale( forward, 4096, endPoint );
		SnapVector( endPoint );
		SnapVector( muzzlePoint );
		CG_MinigunPattern(muzzlePoint, endPoint , ent->clientNum, cg.oldTime % 256, heat );
	} else if ( ent->weapon == WP_ASSAULTRIFLE ) {
		VectorMA (muzzlePoint, 4096, forward, endPoint );
		CG_Trace( &tr, muzzlePoint, NULL, NULL, endPoint, ent->clientNum, MASK_SHOT );
		if ( tr.surfaceFlags & SURF_NOIMPACT )
			return;
		SnapVectorTowards( tr.endpos, muzzlePoint );
		// do bullet impact
		if ( tr.entityNum < MAX_CLIENTS ) {
			flesh = qtrue;
			fleshNum = tr.entityNum;
		} else {
			fleshNum = ENTITYNUM_NONE;
			flesh = qfalse;
		}
		CG_Bullet( tr.endpos, cg.predictedPlayerState.clientNum, tr.plane.normal, flesh, fleshNum );
	} else if ( ent->weapon == WP_SNIPER_RIFLE ) {
		/* Don't predict weapon when in the air */
		if ( cg.predictedPlayerState.groundEntityNum == ENTITYNUM_NONE )
			return;
		VectorMA (muzzlePoint, 4096, forward, endPoint );
		CG_Trace( &tr, muzzlePoint, NULL, NULL, endPoint, ent->clientNum, MASK_SHOT );
		if ( tr.surfaceFlags & SURF_NOIMPACT ) 
			return;
		SnapVectorTowards( tr.endpos, muzzlePoint );
		// do bullet impact
		if ( tr.entityNum < MAX_CLIENTS ) {
			flesh = qtrue;
			fleshNum = tr.entityNum;
		} else {
			fleshNum = ENTITYNUM_NONE;
			flesh = qfalse;
		}
		CG_Bullet( tr.endpos, cg.predictedPlayerState.clientNum, tr.plane.normal, flesh, fleshNum );
	} else 
		return;

	if ((cg_predictWeapons.integer & 2) && cgs.sv_cheats == qtrue ) {
		int x,zd,zu;
		localEntity_t *le;
		centity_t *tracecent;

		VectorMA (muzzlePoint, 4096, forward, endPoint );
		CG_Trace( &tr, muzzlePoint, NULL, NULL, endPoint, ent->clientNum, MASK_SHOT );

		if ( tr.entityNum >= MAX_CLIENTS ) 
			return;

		tracecent = &cg_entities[tr.entityNum];

		le = CG_AllocLocalEntity( 5500 );
		le->leType = LE_DEBUG_BOX;
            
		VectorCopy( tracecent->lerpOrigin ,le->pos.trBase);

		x = (tracecent->currentState.solid & 255);
		zd = ((tracecent->currentState.solid>>8) & 255);
		zu = ((tracecent->currentState.solid>>16) & 255) - 32;

		le->angles.trBase[0] = le->angles.trBase[1] = -x;
		le->angles.trDelta[0] = le->angles.trDelta[1] = x;
		le->angles.trBase[2] = -zd;
		le->angles.trDelta[2] = zu;

		le->color[0] = colorYellow[0];
		le->color[1] = colorYellow[1];
		le->color[2] = colorYellow[2];
		le->color[3] = colorYellow[3];
	}
}

/*
=================
CG_AddBoundingBox

Draws a bounding box around a player.  Called from CG_Player.
=================
*/
static qhandle_t bboxShader = 0;
static qhandle_t bboxShader_nocull = 0;

void CG_DrawBoundingBox( vec3_t origin, vec3_t mins,vec3_t maxs, vec3_t color ) {
	int i;
	polyVert_t verts[4];
	vec3_t corners[8];

	// get the extents (size)
	float extx = maxs[0] - mins[0];
	float exty = maxs[1] - mins[1];
	float extz = maxs[2] - mins[2];

	// get the shader handles
	if (!bboxShader || !bboxShader_nocull) {
		bboxShader = trap_R_RegisterShader( "bbox" );
		bboxShader_nocull = trap_R_RegisterShader( "bbox_nocull" );
		// if they don't exist, forget it
		if ( !bboxShader || !bboxShader_nocull ) {
			return;
		}
	}

	// set the polygon's texture coordinates
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;

	// set the polygon's vertex colors
	for ( i = 0; i < 4; i++ ) {
		verts[i].modulate[0] = color[0] * 255;
		verts[i].modulate[1] = color[1] * 255;
		verts[i].modulate[2] = color[2] * 255;
		verts[i].modulate[3] = 255;
	}

	VectorAdd( origin, maxs, corners[3] );

	VectorCopy( corners[3], corners[2] );
	corners[2][0] -= extx;

	VectorCopy( corners[2], corners[1] );
	corners[1][1] -= exty;

	VectorCopy( corners[1], corners[0] );
	corners[0][0] += extx;

	for ( i = 0; i < 4; i++ ) {
		VectorCopy( corners[i], corners[i + 4] );
		corners[i + 4][2] -= extz;
	}

	// top
	VectorCopy( corners[0], verts[0].xyz );
	VectorCopy( corners[1], verts[1].xyz );
	VectorCopy( corners[2], verts[2].xyz );
	VectorCopy( corners[3], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader, 4, verts );

	// bottom
	VectorCopy( corners[7], verts[0].xyz );
	VectorCopy( corners[6], verts[1].xyz );
	VectorCopy( corners[5], verts[2].xyz );
	VectorCopy( corners[4], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader, 4, verts );

	// top side
	VectorCopy( corners[3], verts[0].xyz );
	VectorCopy( corners[2], verts[1].xyz );
	VectorCopy( corners[6], verts[2].xyz );
	VectorCopy( corners[7], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );

	// left side
	VectorCopy( corners[2], verts[0].xyz );
	VectorCopy( corners[1], verts[1].xyz );
	VectorCopy( corners[5], verts[2].xyz );
	VectorCopy( corners[6], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );

	// right side
	VectorCopy( corners[0], verts[0].xyz );
	VectorCopy( corners[3], verts[1].xyz );
	VectorCopy( corners[7], verts[2].xyz );
	VectorCopy( corners[4], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );

	// bottom side
	VectorCopy( corners[1], verts[0].xyz );
	VectorCopy( corners[0], verts[1].xyz );
	VectorCopy( corners[4], verts[2].xyz );
	VectorCopy( corners[5], verts[3].xyz );
	trap_R_AddPolyToScene( bboxShader_nocull, 4, verts );


}


void CG_AddBoundingBox( centity_t *cent, vec3_t color ) {
	vec3_t mins = {-15, -15, -24};
	vec3_t maxs = {15, 15, 32};

	// don't draw it if it's us in first-person
	if ( cent->currentState.number == cg.predictedPlayerState.clientNum &&
			!cg.renderingThirdPerson ) {
		return;
	}

	// don't draw it for dead players
	if ( cent->currentState.eFlags & EF_DEAD ) {
		return;
	}

	// if it's us
	if ( cent->currentState.number == cg.predictedPlayerState.clientNum ) {
		// use the view height
		maxs[2] = cg.predictedPlayerState.viewheight + 6;
	}
	else {
		int x, zd, zu;

		// otherwise grab the encoded bounding box
		x = (cent->currentState.solid & 255);
		zd = ((cent->currentState.solid>>8) & 255);
		zu = ((cent->currentState.solid>>16) & 255) - 32;

		mins[0] = mins[1] = -x;
		maxs[0] = maxs[1] = x;
		mins[2] = -zd;
		maxs[2] = zu;
	}
	CG_DrawBoundingBox( cent->lerpOrigin, mins, maxs, color );
}


