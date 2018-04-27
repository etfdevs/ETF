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

// cg_weapons.c -- events and effects dealing with weapons
#include "cg_local.h"
#include "../game/bg_q3f_playerclass.h"
#include "../game/bg_q3f_weapon.h"
#include "../game/bg_q3f_util.h"
#include "cg_q3f_menu.h"

weaponInfo_t *CG_Q3F_GetWeaponStruct(int clsnum, int weapon);

// RR2DO2: we use this in bg
void CG_Q3F_EndReload() {
	cg.reloadendtime = cg.time;
}

/*
==========================
CG_MiniGunEjectBrass
==========================
*/

static void CG_MiniGunEjectBrass( centity_t *cent, refEntity_t *parent ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			velocity, xvelocity;
	float			waterScale = 1.0f;
	vec3_t			v[3];
	int				i;

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	for( i = 0; i < 2; i++ ) {
		refEntity_t smoke;

		memset( &smoke, 0, sizeof(smoke) );

		smoke.reType = RT_MODEL;
		smoke.hModel = cgs.media.minigunSmokeTag2;
		smoke.renderfx = 0;

		VectorCopy( parent->lightingOrigin, smoke.lightingOrigin );
		smoke.shadowPlane = parent->shadowPlane;
		smoke.renderfx = parent->renderfx;
		smoke.frame = smoke.oldframe = 0;

		if( i == 0 )
			CG_PositionEntityOnTag( &smoke, parent, "tag_smoke0", 0, NULL );
		else
			CG_PositionEntityOnTag( &smoke, parent, "tag_smoke1", 0, NULL );

		VectorCopy( smoke.origin, smoke.oldorigin );

		trap_R_AddRefEntityToScene( &smoke, cent );
	}

	for ( i = 0; i < 2; i++ ) {
		le = CG_AllocLocalEntity(cg_brassTime.integer + ( cg_brassTime.integer / 4 ) * Q_flrand(0.0f, 1.0f));
		re = &le->refEntity;
		re->frame = re->oldframe = 0;

		velocity[0] = -20 + 25 * Q_flrand(-1.0f, 1.0f);
		velocity[1] = -80 + 30 * Q_flrand(-1.0f, 1.0f);
		velocity[2] = 50 + 15 * Q_flrand(-1.0f, 1.0f);

		le->leType = LE_FRAGMENT;

		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time - (rand()&15);

		AnglesToAxis( cent->lerpAngles, v );

		if( i < 2 )
			CG_PositionEntityOnTag( re, parent, "tag_shells0", 0, NULL );
		else
			CG_PositionEntityOnTag( re, parent, "tag_shells1", 0, NULL );

		VectorCopy( re->origin, le->pos.trBase );

		if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
			waterScale = 0.10f;
		}

		xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
		xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
		xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
		VectorMA( re->axis[0], waterScale, xvelocity, le->pos.trDelta );

		re->hModel = cgs.media.machinegunBrassModel;

		le->bounceFactor = 0.55 * waterScale;

		le->angles.trType = TR_LINEAR;
		le->angles.trTime = cg.time;
		le->angles.trBase[0] = cent->lerpAngles[0] + 90 + ( rand() & 10 );
		le->angles.trBase[1] = cent->lerpAngles[1] + ( rand() & 31 );
		le->angles.trBase[2] = cent->lerpAngles[2] + ( rand() & 31 );
		le->angles.trDelta[0] = 20;
		le->angles.trDelta[1] = 1;
		le->angles.trDelta[2] = 0;

		le->leFlags = LEF_TUMBLE | LEF_SOUND_BRASS;
	}
}


/*
==========================
CG_SingleShotgunEjectBrass
==========================
*/
static void CG_SingleShotgunEjectBrass( centity_t *cent, refEntity_t *parent ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			velocity, xvelocity;
	vec3_t			offset, xoffset;
	vec3_t			v[3];
	int				i;

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	for ( i = 0; i < 1; i++ ) {
		float	waterScale = 1.0f;

		le = CG_AllocLocalEntity( cg_brassTime.integer*3 + cg_brassTime.integer * Q_flrand(0.0f, 1.0f) );
		re = &le->refEntity;

		velocity[0] = 60 + 60 * Q_flrand(-1.0f, 1.0f);
		if ( i == 0 ) {
			velocity[1] = 40 + 10 * Q_flrand(-1.0f, 1.0f);
		} else {
			velocity[1] = -40 + 10 * Q_flrand(-1.0f, 1.0f);
		}
		velocity[2] = 100 + 50 * Q_flrand(-1.0f, 1.0f);

		le->leType = LE_FRAGMENT;

		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;

		AnglesToAxis( cent->lerpAngles, v );

		offset[0] = 8;
		offset[1] = 0;
		offset[2] = 24;

		xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
		xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
		xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
		VectorAdd( cent->lerpOrigin, xoffset, re->origin );
		VectorCopy( re->origin, le->pos.trBase );
		if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
			waterScale = 0.10f;
		}

		xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
		xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
		xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
		VectorScale( xvelocity, waterScale, le->pos.trDelta );

		AxisCopy( axisDefault, re->axis );
		re->hModel = cgs.media.shotgunBrassModel;
		le->bounceFactor = 0.3f;

		le->angles.trType = TR_LINEAR;
		le->angles.trTime = cg.time;
		le->angles.trBase[0] = rand()&31;
		le->angles.trBase[1] = rand()&31;
		le->angles.trBase[2] = rand()&31;
		le->angles.trDelta[0] = 1;
		le->angles.trDelta[1] = 0.5;
		le->angles.trDelta[2] = 0;

		le->leFlags = LEF_TUMBLE | LEF_SOUND_BRASS;
	}
}


/*
==========================
CG_ShotgunEjectBrass
==========================
*/
static void CG_ShotgunEjectBrass( centity_t *cent, refEntity_t *parent ) {
	localEntity_t	*le;
	refEntity_t		*re;
	vec3_t			velocity, xvelocity;
	vec3_t			offset, xoffset;
	vec3_t			v[3];
	int				i;

	if ( cg_brassTime.integer <= 0 ) {
		return;
	}

	for ( i = 0; i < 2; i++ ) {
		float	waterScale = 1.0f;

		le = CG_AllocLocalEntity( cg_brassTime.integer*3 + cg_brassTime.integer * Q_flrand(0.0f, 1.0f) );
		re = &le->refEntity;

		velocity[0] = 60 + 60 * Q_flrand(-1.0f, 1.0f);
		if ( i == 0 ) {
			velocity[1] = 40 + 10 * Q_flrand(-1.0f, 1.0f);
		} else {
			velocity[1] = -40 + 10 * Q_flrand(-1.0f, 1.0f);
		}
		velocity[2] = 100 + 50 * Q_flrand(-1.0f, 1.0f);

		le->leType = LE_FRAGMENT;

		le->pos.trType = TR_GRAVITY;
		le->pos.trTime = cg.time;

		AnglesToAxis( cent->lerpAngles, v );

		offset[0] = 8;
		offset[1] = 0;
		offset[2] = 24;

		xoffset[0] = offset[0] * v[0][0] + offset[1] * v[1][0] + offset[2] * v[2][0];
		xoffset[1] = offset[0] * v[0][1] + offset[1] * v[1][1] + offset[2] * v[2][1];
		xoffset[2] = offset[0] * v[0][2] + offset[1] * v[1][2] + offset[2] * v[2][2];
		VectorAdd( cent->lerpOrigin, xoffset, re->origin );
		VectorCopy( re->origin, le->pos.trBase );
		if ( CG_PointContents( re->origin, -1 ) & CONTENTS_WATER ) {
			waterScale = 0.10f;
		}

		xvelocity[0] = velocity[0] * v[0][0] + velocity[1] * v[1][0] + velocity[2] * v[2][0];
		xvelocity[1] = velocity[0] * v[0][1] + velocity[1] * v[1][1] + velocity[2] * v[2][1];
		xvelocity[2] = velocity[0] * v[0][2] + velocity[1] * v[1][2] + velocity[2] * v[2][2];
		VectorScale( xvelocity, waterScale, le->pos.trDelta );

		AxisCopy( axisDefault, re->axis );
		re->hModel = cgs.media.shotgunBrassModel;
		le->bounceFactor = 0.3f;

		le->angles.trType = TR_LINEAR;
		le->angles.trTime = cg.time;
		le->angles.trBase[0] = rand()&31;
		le->angles.trBase[1] = rand()&31;
		le->angles.trBase[2] = rand()&31;
		le->angles.trDelta[0] = 1;
		le->angles.trDelta[1] = 0.5f;
		le->angles.trDelta[2] = 0;

		le->leFlags = LEF_TUMBLE | LEF_SOUND_BRASS;
	}
}

float           cg_q3f_trailteamcolours[5][3] = {
	{1.f, 0.f, 0.f},			// RED
	{0.f, 0.f, 1.f},			// BLUE
	{1.f, 1.f, 0.f},			// YELLOW
	{0.f, 1.f, 0.f},			// GREEN
	{1.f, 1.f, 1.f},			// FREE
};

static float   *CG_MissileTrail_TeamColoured(int team, const vec4_t defaultclr, const char *src)
{
	static vec4_t   colour;

	if(!Q_stricmp(src, "team"))
	{
		VectorCopy4(cg_q3f_trailteamcolours[team - Q3F_TEAM_RED], colour);
	}
	else if(GetColourFromHex(src, colour))
	{
		//
	}
	else if(GetColourFromString(src, colour))
	{
		//
	}
	else
	{
		VectorCopy4(defaultclr, colour);
		//VectorSet4(colour, 1.f, 1.f, 1.f, 0.33f);
	}

	colour[3] = defaultclr[3];	//0.33f;

	return colour;
}

static void CG_WeaponSmokeTrail( centity_t *ent, const weaponInfo_t *wi ) {
	vec3_t	origin, lastPos;
	entityState_t *es = &ent->currentState;
	vec4_t          color;
	int             team;
	int             parent;
	const char     *trailVar = 0;
	qboolean        colorSupported = qfalse;

	if( cg_lowEffects.integer )
		return;

	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );

	CG_BubbleTrail( lastPos, origin, 8 );

	switch (wi->item->giTag)
	{
		default:
			trailVar = 0;
			colorSupported = qfalse;
			break;
		case WP_ROCKET_LAUNCHER:
			trailVar = cg_rocketTrail.string;
			colorSupported = qtrue;
			break;
		case WP_GRENADE_LAUNCHER:
			trailVar = cg_grenadeTrail.string;
			colorSupported = qtrue;
			break;
		case WP_PIPELAUNCHER:
			trailVar = cg_pipeTrail.string;
			colorSupported = qtrue;
			break;
		case WP_DARTGUN:
			trailVar = cg_dartTrail.string;
			colorSupported = qfalse;
			break;
		case WP_NAPALMCANNON:
			trailVar = cg_napalmTrail.string;
			colorSupported = qfalse;
			break;
	}

	if(((!trailVar || !trailVar[0]) && colorSupported) || !Q_stricmp(trailVar, "0"))
		return;

	parent = es->otherEntityNum;
	if(parent >= 0 && parent < MAX_CLIENTS) {
		if(cg.gasEndTime && cg.gasPlayerTeam[parent] != 0xFF)
			team = cg.gasPlayerTeam[parent];
		else
			team = cgs.clientinfo[parent].team;
	}
	else
		team = Q3F_TEAM_FREE;

	if(!colorSupported)
		VectorCopy4(wi->trailColor, color);
	else
		VectorCopy4(CG_MissileTrail_TeamColoured(team, wi->trailColor, trailVar), color);

	for ( ; ent->trailTime <= cg.time ; ent->trailTime += wi->trailStep ) {
		BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
		if (CG_PointContents( lastPos, 0 ) & CONTENTS_WATER ) continue;
		CG_SpawnSmokeSprite( lastPos, wi->trailDuration , color/*wi->trailColor*/, 8, wi->trailRadius);
	}
}

static void CG_NailTrail( centity_t *ent, const weaponInfo_t *wi ) {
	vec3_t	origin, lastPos;
	entityState_t	*es = &ent->currentState;
	if( cg_lowEffects.integer )
		return;
	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	ent->trailTime = cg.time;
	CG_BubbleTrail( lastPos, origin, 16 );
}


// JT - New Railgun Trail - Adapted from RR2's NailTrail (above)
/*
==========================
CG_NewRailTrail
==========================
*/
static void CG_NewRailTrail( centity_t *ent, const weaponInfo_t *wi ) {
	vec3_t	origin, lastPos;
	entityState_t	*es;
	localEntity_t	*le;
	refEntity_t		*re;

	es = &ent->currentState;

	// if object (e.g. grenade) is stationary, don't toss up smoke
	if ( es->pos.trType == TR_STATIONARY ) {
		ent->trailTime = cg.time;
		return;
	}

	BG_EvaluateTrajectory( &es->pos, cg.time, origin );
	BG_EvaluateTrajectory( &es->pos, ent->trailTime, lastPos );
	ent->trailTime = cg.time;

	CG_BubbleTrail( lastPos, origin, 8 );
	// rings
	le = CG_AllocLocalEntity( wi->trailDuration );
	re = &le->refEntity;

	le->leType = LE_FADE_RGB;
	re->reType = RT_RAIL_RINGS;
	re->customShader = cgs.media.railRingsShader;
	VectorCopy( origin, re->origin);
	VectorCopy( lastPos, re->oldorigin );

	le->color[0] = 0.75;
	le->color[1] = 0.75;
	le->color[2] = 0.75;
	le->color[3] = 0.7;

	AxisClear( re->axis );
}


/*
================
CG_Q3F_RegisterHandsModel
================
*/
static void CG_Q3F_RegisterHandsModel( weaponInfo_t *weaponInfo, const char *filename )
{
	weaponInfo->handsModel = trap_R_RegisterModel( filename );
	weaponInfo->F2RScript = F2R_GetForModel( weaponInfo->handsModel );
	if( !weaponInfo->F2RScript ) {
		char f2rname[MAX_QPATH];
		Q_strncpyz( f2rname, filename, sizeof(f2rname) );
		COM_StripExtension( f2rname, f2rname, sizeof(f2rname) );
		Q_strcat( f2rname, sizeof(f2rname), ".f2r" );
		Com_Printf( "^3Hands model F2R load failure: %s\n", f2rname );
	}
}

/*
=================
CG_RegisterExtendedWeapon
JT: Used for different Axes
=================
*/
void CG_RegisterExtendedWeapon( int weaponNum ) {
	weaponInfo_t	*weaponInfo;
	vec3_t			mins, maxs;
	int				i;

	if ( weaponNum <= 0 || weaponNum >= MAX_EXTENDED_WEAPONS ) {
		return;
	}

	weaponInfo = &cg_extendedweapons[weaponNum];

	if ( weaponInfo->registered ) {
		return;
	}

	memset( weaponInfo, 0, sizeof( *weaponInfo ) );
	weaponInfo->registered = qtrue;

	switch ( weaponNum ) {
	case Q3F_WP_KNIFE:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1 );
		weaponInfo->flashSound[0]	= trap_S_RegisterSound( "sound/weapons/knife/knife_swing1.wav", qfalse );
		weaponInfo->flashSound[1]	= trap_S_RegisterSound( "sound/weapons/knife/knife_swing2.wav", qfalse );
		weaponInfo->flashSound[2]	= trap_S_RegisterSound( "sound/weapons/knife/knife_swing3.wav", qfalse );
		weaponInfo->wallSound[0][0]	= trap_S_RegisterSound( "sound/weapons/knife/knife_stone1.wav", qfalse );
		weaponInfo->wallSound[0][1]	= trap_S_RegisterSound( "sound/weapons/knife/knife_stone2.wav", qfalse );
		weaponInfo->wallSound[0][2]	= trap_S_RegisterSound( "sound/weapons/knife/knife_stone3.wav", qfalse );
		weaponInfo->wallSound[1][0]	= trap_S_RegisterSound( "sound/weapons/knife/knife_metal1.wav", qfalse );
		weaponInfo->wallSound[1][1]	= trap_S_RegisterSound( "sound/weapons/knife/knife_metal2.wav", qfalse );
		weaponInfo->wallSound[1][2]	= trap_S_RegisterSound( "sound/weapons/knife/knife_metal3.wav", qfalse );
		weaponInfo->fleshSound[0]	= trap_S_RegisterSound( "sound/weapons/knife/knife_flesh1.wav", qfalse );
		weaponInfo->fleshSound[1]	= trap_S_RegisterSound( "sound/weapons/knife/knife_flesh2.wav", qfalse );
		weaponInfo->fleshSound[2]	= trap_S_RegisterSound( "sound/weapons/knife/knife_flesh3.wav", qfalse );
		weaponInfo->weaponModel		= trap_R_RegisterModel( "models/weapons2/knife/knife.md3" );
		CG_Q3F_RegisterHandsModel( weaponInfo, "models/weapons2/knife/knife_hand.md3" );
		weaponInfo->item = &(bg_extendeditemlist[Q3F_WP_KNIFE]);
		weaponInfo->weaponIcon		= trap_R_RegisterShader("icons/iconw_knife");
		break;
	case Q3F_WP_BIOAXE:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1 );
		weaponInfo->flashSound[0]	= trap_S_RegisterSound( "sound/weapons/syringe/syringe_swing1.wav", qfalse );
		weaponInfo->flashSound[1]	= trap_S_RegisterSound( "sound/weapons/syringe/syringe_swing2.wav", qfalse );
		weaponInfo->flashSound[2]	= trap_S_RegisterSound( "sound/weapons/syringe/syringe_swing3.wav", qfalse );
		weaponInfo->wallSound[0][0]	= trap_S_RegisterSound( "sound/weapons/syringe/syringe_stone1.wav", qfalse );
		weaponInfo->wallSound[0][1]	= trap_S_RegisterSound( "sound/weapons/syringe/syringe_stone2.wav", qfalse );
		weaponInfo->wallSound[0][2]	= trap_S_RegisterSound( "sound/weapons/syringe/syringe_stone3.wav", qfalse );
		weaponInfo->wallSound[1][0]	= trap_S_RegisterSound( "sound/weapons/syringe/syringe_metal1.wav", qfalse );
		weaponInfo->wallSound[1][1]	= trap_S_RegisterSound( "sound/weapons/syringe/syringe_metal2.wav", qfalse );
		weaponInfo->wallSound[1][2]	= trap_S_RegisterSound( "sound/weapons/syringe/syringe_metal3.wav", qfalse );
		weaponInfo->fleshSound[0]	= trap_S_RegisterSound( "sound/weapons/syringe/syringe_flesh1.wav", qfalse );
		weaponInfo->fleshSound[1]	= trap_S_RegisterSound( "sound/weapons/syringe/syringe_flesh2.wav", qfalse );
		weaponInfo->fleshSound[2]	= trap_S_RegisterSound( "sound/weapons/syringe/syringe_flesh3.wav", qfalse );
		weaponInfo->weaponModel		= trap_R_RegisterModel( "models/weapons2/bioweapon/bioweapon.md3" );
		CG_Q3F_RegisterHandsModel( weaponInfo, "models/weapons2/bioweapon/bioweapon_hand.md3" );
		weaponInfo->item = &(bg_extendeditemlist[Q3F_WP_BIOAXE]);
		weaponInfo->weaponIcon = trap_R_RegisterShader("icons/iconw_bioweapon");
		break;
	case Q3F_WP_WRENCH:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1 );
		weaponInfo->flashSound[0]	= trap_S_RegisterSound( "sound/weapons/wrench/wrench_swing1.wav", qfalse );
		weaponInfo->flashSound[1]	= trap_S_RegisterSound( "sound/weapons/wrench/wrench_swing2.wav", qfalse );
		weaponInfo->flashSound[2]	= trap_S_RegisterSound( "sound/weapons/wrench/wrench_swing3.wav", qfalse );
		weaponInfo->wallSound[0][0]	= trap_S_RegisterSound( "sound/weapons/wrench/wrench_stone1.wav", qfalse );
		weaponInfo->wallSound[0][1]	= trap_S_RegisterSound( "sound/weapons/wrench/wrench_stone2.wav", qfalse );
		weaponInfo->wallSound[0][2]	= trap_S_RegisterSound( "sound/weapons/wrench/wrench_stone3.wav", qfalse );
		weaponInfo->wallSound[1][0]	= trap_S_RegisterSound( "sound/weapons/wrench/wrench_metal1.wav", qfalse );
		weaponInfo->wallSound[1][1]	= trap_S_RegisterSound( "sound/weapons/wrench/wrench_metal2.wav", qfalse );
		weaponInfo->wallSound[1][2]	= trap_S_RegisterSound( "sound/weapons/wrench/wrench_metal3.wav", qfalse );
		weaponInfo->fleshSound[0]	= trap_S_RegisterSound( "sound/weapons/wrench/wrench_flesh1.wav", qfalse );
		weaponInfo->fleshSound[1]	= trap_S_RegisterSound( "sound/weapons/wrench/wrench_flesh2.wav", qfalse );
		weaponInfo->fleshSound[2]	= trap_S_RegisterSound( "sound/weapons/wrench/wrench_flesh3.wav", qfalse );
		weaponInfo->weaponModel		= trap_R_RegisterModel( "models/weapons2/wrench/wrench.md3" );
		CG_Q3F_RegisterHandsModel( weaponInfo, "models/weapons2/wrench/wrench_hand.md3" );
		weaponInfo->item = &(bg_extendeditemlist[Q3F_WP_WRENCH]);
		weaponInfo->weaponIcon = trap_R_RegisterShader("icons/iconw_wrench");
		break;
	 default:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1 );
		weaponInfo->firingSound		= trap_S_RegisterSound( "sound/weapons/melee/fstrun.wav", qfalse );
		weaponInfo->flashSound[0]	= trap_S_RegisterSound( "sound/weapons/melee/fstatck.wav", qfalse );
		weaponInfo->barrelModel		= trap_R_RegisterModel( "models/weapons2/gauntlet/gauntlet_barrel.md3" );
		break;
	}

	// calc midpoint for rotation
	trap_R_ModelBounds( weaponInfo->weaponModel, mins, maxs );
	for ( i = 0 ; i < 3 ; i++ ) {
		weaponInfo->weaponMidpoint[i] = mins[i] + 0.5f * ( maxs[i] - mins[i] );
	}

	if ( !weaponInfo->handsModel ) {
		weaponInfo->handsModel = trap_R_RegisterModel( "models/weapons2/gauntlet/gauntlet_hand.md3" );
	}
}


/*
=================
CG_RegisterWeapon

The server says this item is used on this level
=================
*/
void CG_RegisterWeapon( int weaponNum ) {
	weaponInfo_t	*weaponInfo;
	gitem_t			*item, *ammo;
	char			path[MAX_QPATH];
	vec3_t			mins, maxs;
	int				i;

	if ( weaponNum <= 0 || weaponNum >= WP_NUM_WEAPONS ) {
		return;
	}

	weaponInfo = &cg_weapons[weaponNum];

	if ( weaponInfo->registered ) {
		return;
	}

	memset( weaponInfo, 0, sizeof( *weaponInfo ) );
	weaponInfo->registered = qtrue;

	for ( item = bg_itemlist + 1 ; item->classname ; item++ ) {
		if ( item->giType == IT_WEAPON && item->giTag == weaponNum ) {
			weaponInfo->item = item;
			break;
		}
	}
	if ( !item->classname ) {
		CG_Error( "Couldn't find weapon %i", weaponNum );
	}
	CG_RegisterItemVisuals( item - bg_itemlist );

	// load cmodel before model so filecache works
	weaponInfo->weaponModel = trap_R_RegisterModel( item->world_model[0] );

	// calc midpoint for rotation
	trap_R_ModelBounds( weaponInfo->weaponModel, mins, maxs );
	for ( i = 0 ; i < 3 ; i++ ) {
		weaponInfo->weaponMidpoint[i] = mins[i] + 0.5 * ( maxs[i] - mins[i] );
	}

	weaponInfo->weaponIcon = trap_R_RegisterShader( item->icon );

	for ( ammo = bg_itemlist + 1 ; ammo->classname ; ammo++ ) {
		if ( ammo->giType == IT_AMMO && ammo->giTag == Q3F_GetAmmoTypeForWeapon(weaponNum) ) {
			break;
		}
	}

	strcpy( path, item->world_model[0] );
	COM_StripExtension( path, path, sizeof(path) );
	strcat( path, "_flash.md3" );
	weaponInfo->flashModel = trap_R_RegisterModel( path );

	//if ( /*weaponNum == WP_NAILGUN || JT */ weaponNum == WP_MINIGUN || weaponNum == WP_SUPERNAILGUN/*|| weaponNum == WP_AXE JT || weaponNum == WP_BFG */) {
	if ( weaponNum != WP_AXE ) {
		strcpy( path, item->world_model[0] );
		COM_StripExtension( path, path, sizeof(path) );
		strcat( path, "_barrel.md3" );
		weaponInfo->barrelModel = trap_R_RegisterModel( path );
	}

	strcpy( path, item->world_model[0] );
	COM_StripExtension( path, path, sizeof(path) );
	strcat( path, "_hand.md3" );
	//weaponInfo->handsModel = trap_R_RegisterModel( path );
	CG_Q3F_RegisterHandsModel( weaponInfo, path );

	if ( !weaponInfo->handsModel ) {
		weaponInfo->handsModel = trap_R_RegisterModel( "models/weapons2/shotgun/shotgun_hand.md3" );
	}

	weaponInfo->loopFireSound = qfalse;

	switch ( weaponNum ) {
	case WP_AXE:
		MAKERGB( weaponInfo->flashDlightColor, 0.6f, 0.6f, 1.0f );
		weaponInfo->flashSound[0]	= trap_S_RegisterSound( "sound/weapons/axe/axe_swing1.wav", qfalse );
		weaponInfo->flashSound[1]	= trap_S_RegisterSound( "sound/weapons/axe/axe_swing2.wav", qfalse );
		weaponInfo->flashSound[2]	= trap_S_RegisterSound( "sound/weapons/axe/axe_swing3.wav", qfalse );
		weaponInfo->wallSound[0][0]	= trap_S_RegisterSound( "sound/weapons/axe/axe_stone1.wav", qfalse );
		weaponInfo->wallSound[0][1]	= trap_S_RegisterSound( "sound/weapons/axe/axe_stone2.wav", qfalse );
		weaponInfo->wallSound[0][2]	= trap_S_RegisterSound( "sound/weapons/axe/axe_stone3.wav", qfalse );
		weaponInfo->wallSound[1][0]	= trap_S_RegisterSound( "sound/weapons/axe/axe_metal1.wav", qfalse );
		weaponInfo->wallSound[1][1]	= trap_S_RegisterSound( "sound/weapons/axe/axe_metal2.wav", qfalse );
		weaponInfo->wallSound[1][2]	= trap_S_RegisterSound( "sound/weapons/axe/axe_metal3.wav", qfalse );
		weaponInfo->fleshSound[0]	= trap_S_RegisterSound( "sound/weapons/axe/axe_flesh1.wav", qfalse );
		weaponInfo->fleshSound[1]	= trap_S_RegisterSound( "sound/weapons/axe/axe_flesh2.wav", qfalse );
		weaponInfo->fleshSound[2]	= trap_S_RegisterSound( "sound/weapons/axe/axe_flesh3.wav", qfalse );
		weaponInfo->handsModel		= trap_R_RegisterModel( "models/weapons2/axe/axe_hand.md3" );
		weaponInfo->flashModel		= 0;	
		break;
	case WP_NAILGUN:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/nail/nail.md3" );
		weaponInfo->missileTrailFunc = CG_NailTrail;
		weaponInfo->trailDuration = 200;
		weaponInfo->trailRadius = 8;
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/nailgun/nailgun.wav", qfalse );
		weaponInfo->missileRenderfx = RF_MINLIGHT;
		break;

	case WP_MINIGUN:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/minigun/minigun_fire.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_MiniGunEjectBrass;
		break;

	case WP_DARTGUN:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/nail/nail.md3" );
		weaponInfo->missileTrailFunc = CG_WeaponSmokeTrail;
		weaponInfo->trailDuration = 200;
		weaponInfo->trailRadius = 8;
		weaponInfo->trailStep = 30;
		VectorSet4(weaponInfo->trailColor, 0.75, 1, 0.75f, 0.33f);

		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/dartgun/dart.wav", qfalse );
		break;

	case WP_RAILGUN:
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/railgun/railgun_hum.wav", qfalse );
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/nail/nail.md3" );
		weaponInfo->missileTrailFunc = CG_NewRailTrail;
		weaponInfo->trailDuration = 400;
		weaponInfo->trailRadius = 16;
//		weaponInfo->missileTrailFunc = CG_WeaponSmokeTrail;
//		weaponInfo->trailDuration = 200;
//		weaponInfo->trailRadius = 3.5;
//		weaponInfo->trailStep = 20;
//		VectorSet4(weaponInfo->trailColor, 1, 1, 1, 0.75f);
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/railgun/railgun.wav", qfalse );
		cgs.media.railExplosionShader = trap_R_RegisterShader( "railExplosion" );
		cgs.media.railRingsShader = trap_R_RegisterShader( "railDisc" );
		break;

	case WP_SUPERSHOTGUN:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/shotgun/shotgun_db.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_ShotgunEjectBrass;
		break;

	case WP_SHOTGUN:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/shotgun/shotgun_sb.wav", qfalse );
		weaponInfo->ejectBrassFunc = CG_SingleShotgunEjectBrass;
		break;

	case WP_ROCKET_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/rocket/etfrocket.md3" );
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rocket_fly.wav", qfalse );
		weaponInfo->missileTrailFunc = CG_WeaponSmokeTrail;
		weaponInfo->missileDlight = 200;
		weaponInfo->trailDuration = 1500;
		weaponInfo->trailRadius = 16; // 64 -> 16
		weaponInfo->trailStep = 50;
		VectorSet4(weaponInfo->trailColor, 1, 1, 1, 0.33f );
		MAKERGB( weaponInfo->missileDlightColor, 1, 0.75, 0 );
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.75, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocket_fire.wav", qfalse );
		cgs.media.rocketExplosionShader = trap_R_RegisterShader( "rocketExplosion" );
		cgs.media.rocket3DExplosionShader = trap_R_RegisterShader( "rocketExplosion3D" );
		break;

	case WP_NAPALMCANNON:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/rocket/etfrocket.md3" );
		weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/rocket/rocket_fly.wav", qfalse );
		weaponInfo->missileTrailFunc = CG_WeaponSmokeTrail;
		weaponInfo->trailStep = 50;
		weaponInfo->trailDuration = 1000;
		weaponInfo->trailRadius = 20; // 80 -> 20
		VectorSet4(weaponInfo->trailColor, 1.0, 0.5, 0.1, 0.40);
		weaponInfo->missileDlight = 200;
		MAKERGB( weaponInfo->missileDlightColor, 1, 0, 0 );	// JT - Redder?
		MAKERGB( weaponInfo->flashDlightColor, 1, 0, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocket_fire.wav", qfalse );
		cgs.media.rocketExplosionShader = trap_R_RegisterShader( "rocketExplosion" );
		cgs.media.rocket3DExplosionShader = trap_R_RegisterShader( "rocketExplosion3D" );
		break;

	case WP_GRENADE_LAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/pipebomb/pipegren.md3" );
		weaponInfo->missileTrailFunc = CG_WeaponSmokeTrail;
		weaponInfo->trailStep = 40;
		weaponInfo->trailDuration = 700;
		weaponInfo->trailRadius = 12; // 32 -> 12
		VectorSet4(weaponInfo->trailColor, 0.75, 0.75, 0.75f, 0.50f);
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.7f, 0.5f );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/grenlauncher/grenlauncher.wav", qfalse );
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
		cgs.media.grenade3DExplosionShader = trap_R_RegisterShader( "grenadeExplosion3D" );
		break;

	case WP_PIPELAUNCHER:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/pipebomb/pipebomb.md3" );
		weaponInfo->missileTrailFunc = CG_WeaponSmokeTrail;
		weaponInfo->trailStep = 40;
		weaponInfo->trailDuration = 700;
		weaponInfo->trailRadius = 12; // 32 -> 12
		VectorSet4(weaponInfo->trailColor, 0.75, 0.75, 0.50f, 0.50f);
		MAKERGB( weaponInfo->flashDlightColor, 1, 0.7f, 0.5f );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/grenlauncher/grenlauncher.wav", qfalse );
		cgs.media.grenadeExplosionShader = trap_R_RegisterShader( "grenadeExplosion" );
		cgs.media.grenade3DExplosionShader = trap_R_RegisterShader( "grenadeExplosion3D" );
		break;

	case WP_FLAMETHROWER:
		weaponInfo->readySound = trap_S_RegisterSound( "sound/weapons/flamer/flamer_idle.wav", qfalse );
		break;

	case WP_SNIPER_RIFLE:
		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/sniper/sniper_fire.wav", qfalse);
		break;

	case WP_ASSAULTRIFLE:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound("sound/weapons/sniper/sniper_burst.wav", qfalse);
		break;

	case WP_SUPERNAILGUN:
		weaponInfo->missileModel = trap_R_RegisterModel( "models/ammo/nail/nail.md3" );
		//weaponInfo->missileSound = trap_S_RegisterSound( "sound/weapons/nailgun/nail_fly.wav", qfalse );
		weaponInfo->missileTrailFunc = CG_NailTrail;
		weaponInfo->trailDuration = 200;
		weaponInfo->trailRadius = 8;
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 0 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/nailgun/supernailgun.wav", qfalse );
		weaponInfo->missileRenderfx = RF_MINLIGHT;
		break;
	
	default:
		MAKERGB( weaponInfo->flashDlightColor, 1, 1, 1 );
		weaponInfo->flashSound[0] = trap_S_RegisterSound( "sound/weapons/rocket/rocklf1a.wav", qfalse );
		break;
	}
}

qboolean CG_FileExists( const char *filename );

/*
=================
CG_RegisterItemVisuals

The server says this item is used on this level
=================
*/
void CG_RegisterItemVisuals( int itemNum ) {
	itemInfo_t		*itemInfo;
	gitem_t			*item;
	char			df_icon[MAX_QPATH] = { 0 };

	if ( itemNum < 0 || itemNum >= bg_numItems ) {
		CG_Error( "CG_RegisterItemVisuals: itemNum %d out of range [0-%d]", itemNum, bg_numItems-1 );
	}

	itemInfo = &cg_items[ itemNum ];
	if ( itemInfo->registered ) {
		return;
	}

	item = &bg_itemlist[ itemNum ];

	memset( itemInfo, 0, sizeof( *itemInfo ) );
	itemInfo->registered = qtrue;

	itemInfo->models[0] = trap_R_RegisterModel( item->world_model[0] );

	if( item->icon )
		itemInfo->icon = trap_R_RegisterShader( item->icon );

	Com_sprintf( df_icon, sizeof( df_icon ), "%s_df", item->icon );
	// try to register depth-fragment shaders
	if ( CG_FileExists( df_icon ) ) {
		itemInfo->icon_df = trap_R_RegisterShader( df_icon );
	}

	if ( !itemInfo->icon_df )
		itemInfo->icon_df = itemInfo->icon;

	if ( item->giType == IT_WEAPON ) {
		CG_RegisterWeapon( item->giTag );
	}

	//
	// powerups have an accompanying ring or sphere
	//
	if ( item->giType == IT_POWERUP || item->giType == IT_HEALTH || 
		item->giType == IT_ARMOR || item->giType == IT_HOLDABLE ) {
		if ( item->world_model[1] ) {
			itemInfo->models[1] = trap_R_RegisterModel( item->world_model[1] );
		}
	}
}


/*
========================================================================================

VIEW WEAPON

========================================================================================
*/

/*
==============
CG_CalculateWeaponPosition
==============
*/
static void CG_CalculateWeaponPosition( vec3_t origin, vec3_t angles ) {
	float	scale;
	int		delta;
	float	fracsin;

	VectorCopy( cg.refdef.vieworg, origin );
	VectorCopy( cg.refdefViewAngles, angles );

	// on odd legs, invert some angles
	if ( cg.bobcycle & 1 ) {
		scale = -cg.xyspeed;
	} else {
		scale = cg.xyspeed;
	}

	// RR2DO2: TF maniacs :P
	if ( cg_weaponBob.value ) {
		// gun angles from bobbing
		angles[ROLL] += scale * cg.bobfracsin * 0.005;
		angles[YAW] += scale * cg.bobfracsin * 0.01;
		angles[PITCH] += cg.xyspeed * cg.bobfracsin * 0.005;
	}

	// RR2DO2: TF maniacs :P
	if ( cg_fallingBob.value ) {
		// drop the weapon when landing
		delta = cg.time - cg.landTime;
		if ( delta < LAND_DEFLECT_TIME ) {
			origin[2] += cg.landChange*0.25 * delta / LAND_DEFLECT_TIME;
		} else if ( delta < LAND_DEFLECT_TIME + LAND_RETURN_TIME ) {
			origin[2] += cg.landChange*0.25 * 
				(LAND_DEFLECT_TIME + LAND_RETURN_TIME - delta) / LAND_RETURN_TIME;
		}
	}
	// RR2DO2

#if 0
	// drop the weapon when stair climbing
	delta = cg.time - cg.stepTime;
	if ( delta < STEP_TIME/2 ) {
		origin[2] -= cg.stepChange*0.25 * delta / (STEP_TIME/2);
	} else if ( delta < STEP_TIME ) {
		origin[2] -= cg.stepChange*0.25 * (STEP_TIME - delta) / (STEP_TIME/2);
	}
#endif

	// RR2DO2: TF maniacs :P
	if ( cg_weaponBob.value ) {
		// idle drift
		scale = cg.xyspeed + 40;
		fracsin = sin( cg.time * 0.001 );
		angles[ROLL] += scale * fracsin * 0.01;
		angles[YAW] += scale * fracsin * 0.01;
		angles[PITCH] += scale * fracsin * 0.01;
	}
}


/*
======================
CG_MachinegunSpinAngle
======================
*/
#define		SPIN_SPEED	0.9
#define		COAST_TIME	1000
static float CG_MachinegunSpinAngle( centity_t *cent ) {
	int		delta;
	float	angle;
	float	speed;

	delta = cg.time - cent->pe.barrelTime;
	if ( cent->pe.barrelSpinning ) {
		angle = cent->pe.barrelAngle + delta * SPIN_SPEED;
	} else {
		if ( delta > COAST_TIME ) {
			delta = COAST_TIME;
		}

		speed = 0.5 * ( SPIN_SPEED + (float)( COAST_TIME - delta ) / COAST_TIME );
		angle = cent->pe.barrelAngle + delta * speed;
	}

	if ( cent->pe.barrelSpinning == !(cent->currentState.eFlags & EF_FIRING) ) {
		cent->pe.barrelTime = cg.time;
		cent->pe.barrelAngle = AngleMod( angle );
		cent->pe.barrelSpinning = !!(cent->currentState.eFlags & EF_FIRING);
	}

	return angle;
}

/*
======================
CG_MinigunSpinAngle
======================
*/
static float CG_MinigunSpinAngle( centity_t *cent ) {
	int		delta;
	float	angle;
	float	speed;

	delta = cg.time - cent->pe.barrelTime;
	if ( cent->pe.barrelSpinning ) {
		angle = cent->pe.barrelAngle + delta * SPIN_SPEED;
	} else {
		if ( delta > COAST_TIME ) {
			delta = COAST_TIME;
		}

		speed = 0.5 * ( SPIN_SPEED + (float)( COAST_TIME - delta ) / COAST_TIME );
		angle = cent->pe.barrelAngle + delta * speed;
	}

	if ( cent->pe.barrelSpinning == !(cent->currentState.eFlags & EF_FIRING) ) {
		cent->pe.barrelTime = cg.time;
		cent->pe.barrelAngle = AngleMod( angle );
		cent->pe.barrelSpinning = !!(cent->currentState.eFlags & EF_FIRING);

		if( cent->pe.barrelSpinning ) {
			trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, cgs.media.sfx_minigun_windup );
			cent->pe.minigunTime = cg.time;
		} else {
			trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, cgs.media.sfx_minigun_winddown );
			cent->pe.minigunTime = 0;
		}
	}

	return angle;
}

/*
======================
CG_GrenadeLauncherSpinAngle
======================
*/

static float CG_GrenadeLauncherSpinAngle( centity_t *cent ) {
	int		deltaTime;

	deltaTime = cg.time - cent->muzzleFlashTime;

	if( deltaTime > 50 && deltaTime < 175 ) {
		// move to the right by 60 degrees
		// dt = 50 -> 0
		// dt = 175 -> 60
		// that means every msec = 60 / ( 175 - 50 ) degrees
		return( ( 60.f / ( 175.f - 50.f ) ) * ( (float)deltaTime - 50.f ) );
	} else if( deltaTime >= 175 && deltaTime <= 200 ) {
		// wait a bit
		return( 60.f );
	} else if( deltaTime > 200 && deltaTime < 450 ) {
		// then move 65 degrees back
		// dt = 200 -> 60
		// dt = 450 -> -10
		return( 60.f - ( 70.f / ( 450.f - 200.f ) ) * ( ( float)deltaTime - 200.f ) );
	} else if( deltaTime >= 450 && deltaTime <= 465 ) {
		// wait a bit
		return( -10.f );
	} else if( deltaTime > 465 && deltaTime < 600 ) {
		// and clunk in place
		// dt = 475 -> -10
		// dt = 600 -> 0
		return( -10.f + ( 10.f / ( 600 - 475 ) ) * ( ( float)deltaTime - 475 ) );
	} else {
		return( 0.f );
	}
}

/*
===============
CG_FlamethrowerFlame
===============
*/
static void CG_FlamethrowerFlame( centity_t *cent, vec3_t origin ) {

	if (cent->currentState.weapon != WP_FLAMETHROWER) {
		return;
	}

	CG_FireFlameChunks( cent, origin, cent->lerpAngles, qtrue );
	return;
}

/*
========================
CG_AddWeaponWithPowerups
========================
*/
void CG_AddWeaponWithPowerups( refEntity_t *gun, entityState_t *state, int team ) {
	// add powerup effects
	if ( state->powerups & ( 1 << PW_INVIS ) ) {
		gun->customShader = cgs.media.invisShader;
		trap_R_AddRefEntityToScene( gun, NULL );
	} else {
		trap_R_AddRefEntityToScene( gun, NULL );
		if ( state->powerups & ( 1 << PW_QUAD ) )
		{
			if (team == Q3F_TEAM_RED)
				gun->customShader = cgs.media.redQuadWeaponShader;
			else if (team == Q3F_TEAM_YELLOW)
				gun->customShader = cgs.media.yellowQuadWeaponShader;
			else if (team == Q3F_TEAM_GREEN)
				gun->customShader = cgs.media.greenQuadWeaponShader;
			else
				gun->customShader = cgs.media.quadWeaponShader;
			trap_R_AddRefEntityToScene( gun, NULL );
		}
		/*if ( state->powerups & ( 1 << PW_REGEN ) ) {
			if ( ( ( cg.time / 100 ) % 10 ) == 1 ) {
				gun->customShader = cgs.media.regenShader;
				trap_R_AddRefEntityToScene( gun, NULL );
			}
		}*/

		if ( state->powerups & ( 1 << PW_BATTLESUIT ) ) {
			gun->customShader = cgs.media.battleWeaponShader;
			trap_R_AddRefEntityToScene( gun, NULL );
		}
		if ( state->powerups & ( 1 << PW_PENTAGRAM ) ) {
			gun->customShader = cgs.media.battleWeaponShader;
			trap_R_AddRefEntityToScene( gun, NULL );
		}
		if ( state->extFlags & EXTF_BURNING ) {
			gun->customShader = cgs.media.onFireShader0;
			trap_R_AddRefEntityToScene( gun, NULL );

			gun->customShader = cgs.media.onFireShader1;
			trap_R_AddRefEntityToScene( gun, NULL );
		}
	}
}



/*
=============
CG_AddPlayerWeapon

Used for both the view weapon (ps is valid) and the world modelother character models (ps is NULL)
The main player will have this called for BOTH cases, so effects like light and
sound should only be done on the world model case.
=============
*/
void CG_AddPlayerWeapon( refEntity_t *parent, playerState_t *ps, centity_t *cent, int team, centity_t *agentdata )
{
	// Golliwog: Modified so agent disguise affects weapon as well.

	refEntity_t	gun;
	refEntity_t	barrel;
	refEntity_t	flash;
	vec3_t		angles;
	weapon_t	weaponNum;
	weaponInfo_t	*weapon;
	centity_t	*nonPredictedCent;
	clientInfo_t	*ci;
	qboolean	scaleup, drawmodel, newmodel;
	float		shaderalpha;
	int			agentclass;
	int			muzzlecontents = 0;
	
	ci = &cgs.clientinfo[ps ? ps->clientNum : cent->currentState.number];

	if( ps && !agentdata && cg.agentDataEntity && cg.agentDataEntity->currentValid )
		agentdata = cg.agentDataEntity;
	if( agentdata )
	{
		CG_Q3F_CalcAgentVisibility(	&drawmodel, &shaderalpha, &newmodel,
									1.0f/6.0f, 5.0f/6.0f, &agentdata->currentState );
		if( newmodel )
			cg.agentLastClass = agentclass = agentdata->currentState.torsoAnim;
		else
			agentclass = cg.agentLastClass;
	}
	else {
		drawmodel = qtrue;
		shaderalpha = 0;
		newmodel = qfalse;
		//ci = NULL;		// RR2DO2: what's the need for this?
		cg.agentLastClass = agentclass = 0;
	}

	weaponNum = cent->currentState.weapon;
	
	// Golliwog: Agent weapon 'masquerades'
	if(	agentclass )
	{
		// Change the weapon we want to render
		weaponNum = BG_Q3F_GetRemappedWeaponFromWeaponNum( ci->cls, agentclass, weaponNum );
	}
	// Golliwog.

	// Golliwog: Minigun special effects
	scaleup = ps && !cg.renderingThirdPerson && !cg.renderingFlyBy && !cg.rendering2ndRefDef && weaponNum == WP_MINIGUN;

	//CG_RegisterWeapon( weaponNum );
	weapon = &cg_weapons[weaponNum];
	
	// add the weapon
	memset( &gun, 0, sizeof( gun ) );
	VectorCopy( parent->lightingOrigin, gun.lightingOrigin );
	gun.shadowPlane = parent->shadowPlane;
	gun.renderfx = parent->renderfx;

	gun.hModel = weapon->weaponModel;				
	// JT - Custom override for other classes 'axes'.
	if( weaponNum == WP_AXE )
		switch( agentclass ? agentclass : cent->currentState.otherEntityNum2 )
		{
			case Q3F_CLASS_AGENT:
				gun.hModel = cg_extendedweapons[Q3F_WP_KNIFE].weaponModel;
				break;
			case Q3F_CLASS_PARAMEDIC:
				gun.hModel = cg_extendedweapons[Q3F_WP_BIOAXE].weaponModel;
				break;
			case Q3F_CLASS_ENGINEER:
				gun.hModel = cg_extendedweapons[Q3F_WP_WRENCH].weaponModel;
				break;			
		}

	if (!gun.hModel) {
		return;
	}
	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on teh single player podiums), so
	// go ahead and use the cent
	if( ( nonPredictedCent - cg_entities ) != cent->currentState.clientNum ) {
		nonPredictedCent = cent;
	}

	if ( !ps ) {
		// add weapon ready sound
		if ( ( cent->currentState.eFlags & EF_FIRING ) && weapon->firingSound ) {
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->firingSound, 255, 0 );
		} else if( ( cent->currentState.eFlags & EF_FIRING ) && weaponNum == WP_FLAMETHROWER ) {
			if ( weaponNum == WP_FLAMETHROWER && CG_PointContents( cent->lerpOrigin, -1 ) & CONTENTS_WATER ) {
				trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.sfx_flamethrower_firewater, 255, 0 );
			} else {
				trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.sfx_flamethrower_fire, 255, 0 );
			}
		} else if ( weapon->readySound ) {
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, weapon->readySound, 255, 0 );
		}

		if( weaponNum == WP_MINIGUN && cent->pe.minigunTime && (cent->pe.minigunTime + 700) < cg.time ) {
			trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.sfx_minigun_loop, 255, 0 );
			CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_MINIGUN);
		}
	}

	// RR2DO2: flamethrower windup/winddown sounds
	if( weaponNum == WP_FLAMETHROWER ) {
		if ( cent->pe.barrelSpinning == !(cent->currentState.eFlags & EF_FIRING) ) {
			cent->pe.barrelSpinning = !!(cent->currentState.eFlags & EF_FIRING);

			if( cent->pe.barrelSpinning ) {
				trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, cgs.media.sfx_flamethrower_windup );
			} else {
				trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, cgs.media.sfx_flamethrower_winddown );
			}
		}
	}	

	if( weaponNum == WP_AXE )
		CG_PositionEntityOnTag( &gun, parent, "tag_melee", 0, NULL );
	else
		CG_PositionEntityOnTag( &gun, parent, "tag_weapon", 0, NULL );

	if( scaleup )
	{
		VectorScale( gun.axis[0], 1.1, gun.axis[0] );
		VectorScale( gun.axis[1], 1.1, gun.axis[1] );
		VectorScale( gun.axis[2], 1.1, gun.axis[2] );
		gun.nonNormalizedAxes = qtrue;
	}

	if( drawmodel )
		CG_AddWeaponWithPowerups( &gun, &cent->currentState, team );

	//Special effect for when the agent is diguising
	if( shaderalpha )
	{
		memcpy( gun.shaderRGBA, cg_q3f_agentcolours[ci->team - Q3F_TEAM_RED], 3 );
		gun.shaderRGBA[3] = 255.0 * shaderalpha;
		gun.customShader = cgs.media.agentShader;
		trap_R_AddRefEntityToScene( &gun, cent );
	}

	// add the spinning barrel
	if ( weapon->barrelModel ) {
		memset( &barrel, 0, sizeof( barrel ) );
		VectorCopy( parent->lightingOrigin, barrel.lightingOrigin );
		barrel.shadowPlane = parent->shadowPlane;
		barrel.renderfx = parent->renderfx;

		barrel.hModel = weapon->barrelModel;
		angles[YAW] = 0;
		angles[PITCH] = 0;

		// grenade and pipe launcher have their own rendering
		if( weaponNum == WP_GRENADE_LAUNCHER || weaponNum == WP_PIPELAUNCHER )
			angles[ROLL] = CG_GrenadeLauncherSpinAngle( cent );
		else if( weaponNum == WP_MINIGUN )
			angles[ROLL] = CG_MinigunSpinAngle( cent );
		else
			angles[ROLL] = CG_MachinegunSpinAngle( cent );
		AnglesToAxis( angles, barrel.axis );

		CG_PositionRotatedEntityOnTag( &barrel, &gun, "tag_barrel" );

		if( scaleup )
		{
			VectorScale( barrel.axis[0], 1.1, barrel.axis[0] );
			VectorScale( barrel.axis[1], 1.1, barrel.axis[1] );
			VectorScale( barrel.axis[2], 1.1, barrel.axis[2] );
			gun.nonNormalizedAxes = qtrue;
		}

		if( drawmodel )
			CG_AddWeaponWithPowerups( &barrel, &cent->currentState, team );

		if( shaderalpha )
		{
			memcpy( barrel.shaderRGBA, cg_q3f_agentcolours[ci->team - Q3F_TEAM_RED], 3 );
			barrel.shaderRGBA[3] = 255.0 * shaderalpha;
			barrel.customShader = cgs.media.agentShader;
			trap_R_AddRefEntityToScene( &barrel, cent );
		}
	}

	// make sure we aren't looking at cg.predictedPlayerEntity for LG
	nonPredictedCent = &cg_entities[cent->currentState.clientNum];

	// if the index of the nonPredictedCent is not the same as the clientNum
	// then this is a fake player (like on teh single player podiums), so
	// go ahead and use the cent
	if( ( nonPredictedCent - cg_entities ) != cent->currentState.clientNum ) {
		nonPredictedCent = cent;
	}

	if (cent->currentState.eFlags & EF_Q3F_AIMING ) {
		if (!cent->dustTrailTime)
			cent->dustTrailTime = cg.time;
	} else {
		cent->dustTrailTime = 0;
	}

	//canabis, laser beam
	if ( weaponNum == WP_SNIPER_RIFLE && cent->currentState.eFlags & EF_Q3F_AIMING && 
		( ps || cg.renderingThirdPerson || cg.renderingFlyBy || cg.rendering2ndRefDef ||
			cent->currentState.number != cg.predictedPlayerState.clientNum
			))
	{
		refEntity_t laserbeam;
		float held; float * teamcolor;
		vec4_t color;

		held = cg.time - cent->dustTrailTime;
		if (held < 0) 
			held = 0;
		else if (held > 3000 )
			held = 1;
		else
			held /= 3000;

		held = 0.3 + 0.7*held;

        teamcolor = CG_TeamColor( ci->team );
		VectorScale( teamcolor, held , color);
		color[3] = 1;
		memset( &laserbeam, 0, sizeof(laserbeam) );
		CG_PositionEntityOnTag( &laserbeam, &gun, "tag_laser", 0, NULL );
		VectorMA( laserbeam.origin, 12, laserbeam.axis[0], laserbeam.oldorigin );
		CG_Q3F_ShaderBeam( laserbeam.origin, laserbeam.oldorigin, 0.5, cgs.media.sniperLaser, color );
	}

	// RR2DO2: moved to here to allow for positioning on tags
	// do brass ejection
	if( !ps && !cg.rendering2ndRefDef ) {
		if ( weapon->ejectBrassFunc && cg_brassTime.integer > 0 && cent->ejectBrassTime == cg.time ) {
			weapon->ejectBrassFunc( cent, &gun );

			// stop firing it twice
			//cent->ejectBrassTime -= 1;
		}
		// RR2DO2: minigun startup smoke
		if( weaponNum == WP_MINIGUN && cent->pe.barrelSpinning ) {
			int i;

			for( i = 0; i < 2; i++ ) {
				refEntity_t smoke;

				memset( &smoke, 0, sizeof(smoke) );

				smoke.reType = RT_MODEL;
				smoke.hModel = cgs.media.minigunSmokeTag1;
				smoke.renderfx = 0;
				smoke.frame = smoke.oldframe = 0;

				VectorCopy( gun.lightingOrigin, smoke.lightingOrigin );
				smoke.shadowPlane = gun.shadowPlane;
				smoke.renderfx = gun.renderfx;

				if ( i == 0 ) {
					CG_PositionEntityOnTag( &smoke, &gun, "tag_smoke0", 0, NULL );
				}
				else {
					CG_PositionEntityOnTag( &smoke, &gun, "tag_smoke1", 0, NULL );
				}

				VectorCopy( smoke.origin, smoke.oldorigin );

				trap_R_AddRefEntityToScene( &smoke, cent );
			}
		}
	}

	// add the flash
	if ( ( weaponNum == WP_AXE && ( nonPredictedCent->currentState.eFlags & EF_FIRING )) || weaponNum == WP_FLAMETHROWER)
	{
		// continuous flash
	} else {
		// impulse flash
		if ( cg.time - cent->muzzleFlashTime > MUZZLE_FLASH_TIME) {
			return;
		}
	}

	memset( &flash, 0, sizeof( flash ) );
	VectorCopy( parent->lightingOrigin, flash.lightingOrigin );
	flash.shadowPlane = parent->shadowPlane;
	flash.renderfx = parent->renderfx;

	flash.hModel = weapon->flashModel;
	if (!flash.hModel) {
		return;
	}
	angles[YAW] = 0;
	angles[PITCH] = 0;
	angles[ROLL] = Q_flrand(-1.0f, 1.0f) * 10;
	AnglesToAxis( angles, flash.axis );

	ci = &cgs.clientinfo[ cent->currentState.clientNum ];

	// colorize the railgun blast
	if ( weaponNum == WP_RAILGUN ) {
		flash.shaderRGBA[0] = 255;
		flash.shaderRGBA[1] = 255;
		flash.shaderRGBA[2] = 255;
	}
	if( weaponNum == WP_NAILGUN )
	{
		if( ci->flashtagnumber == 0 )
			CG_PositionRotatedEntityOnTag( &flash, &gun, "tag_flash0" );
		else
			CG_PositionRotatedEntityOnTag( &flash, &gun, "tag_flash1" );
	}
	else
		CG_PositionRotatedEntityOnTag( &flash, &gun, "tag_flash" );

	muzzlecontents = CG_PointContents( flash.origin, -1 );

	if( weaponNum == WP_FLAMETHROWER ) {
		if( !( muzzlecontents & MASK_WATER ) ) {
			VectorCopy( flash.origin, flash.oldorigin );
			trap_R_AddRefEntityToScene( &flash, cent );
		}
	} else if( drawmodel ) {
		VectorCopy( flash.origin, flash.oldorigin );
		flash.renderfx |= RF_NOCELSHADING;
		trap_R_AddRefEntityToScene( &flash, cent );
	}

	// RR2DO2: minigun muzzleflash
	if( weaponNum == WP_MINIGUN && ci->cls != Q3F_CLASS_AGENT ) {
		flash.hModel = cgs.media.minigunFlashTag;
		flash.renderfx |= RF_NOCELSHADING;
		trap_R_AddRefEntityToScene( &flash, cent );
	}

	if ( ps || cg.renderingThirdPerson || cg.renderingFlyBy || cg.rendering2ndRefDef ||
			cent->currentState.number != cg.predictedPlayerState.clientNum ) 
	{
		int radius;
//Keeg copied in from ET's code for flamethrower
		if( (cent->currentState.eFlags & EF_FIRING) && !(cent->currentState.eFlags & EF_DEAD) ) {
			// Ridah, Flamethrower effect
			// Ensiform: Added agent check
			if ( weaponNum == WP_FLAMETHROWER && cent->currentState.otherEntityNum2 == Q3F_CLASS_FLAMETROOPER ) 
				CG_FireFlameChunks( cent, flash.origin, cent->lerpAngles, qtrue );
		}
		else {
			if ( weaponNum == WP_FLAMETHROWER && !(muzzlecontents & MASK_WATER) ) {
				//vec3_t angles;
				AxisToAngles( flash.axis, angles );
				angles[0]=angles[0]+180;		//canabis, rotate this, model tag is wrong direction
// JPW NERVE
				if (ps) {
					if (ps->ammo[AMMO_CELLS])
						CG_FireFlameChunks( cent, flash.origin, angles, qfalse );
				}
				else CG_FireFlameChunks( cent, flash.origin, angles, qfalse);
// jpw
			}
		}

		// use our own muzzle point as dlight origin 
		// and put it a bit closer to vieworigin to avoid bad normals near walls
		if ( ps && cent->currentState.number == cg.predictedPlayerState.clientNum && !cg.renderingFlyBy && !cg.rendering2ndRefDef ) {
			vec3_t	start, end, muzzle, forward, up;
			trace_t	tr;
			AngleVectors( cg.refdefViewAngles, forward, NULL, up );
			VectorMA( cg.refdef.vieworg, 14, forward, muzzle );
			//if ( weaponNum == WP_LIGHTNING )
			//	VectorMA( muzzle, -8, up, muzzle );
			//else
				VectorMA( muzzle, -6, up, muzzle );
			VectorMA( cg.refdef.vieworg, 14, forward, start );
			VectorMA( cg.refdef.vieworg, 28, forward, end );
			CG_Trace( &tr, start, NULL, NULL, end, cent->currentState.number, MASK_SHOT | CONTENTS_TRANSLUCENT );
			if ( tr.fraction != 1.0 ) {
				VectorMA( muzzle, -13.0 * ( 1.0 - tr.fraction ), forward, flash.origin );
			} else {
				VectorCopy( muzzle, flash.origin );
			}
		}

		if ( weaponNum == WP_NAILGUN || weaponNum == WP_SUPERNAILGUN ) // make it a bit less annoying
			radius = NG_FLASH_RADIUS + (rand() & WEAPON_FLASH_RADIUS_MOD);
		else if ( weaponNum == WP_MINIGUN )
			radius = MINI_FLASH_RADIUS + (rand() & WEAPON_FLASH_RADIUS_MOD);
		else if ( weaponNum == WP_ASSAULTRIFLE )
			radius = AR_FLASH_RADIUS + (rand() & WEAPON_FLASH_RADIUS_MOD);
		else
			radius = WEAPON_FLASH_RADIUS + (rand() & WEAPON_FLASH_RADIUS_MOD);

		if (weapon->flashDlightColor[0] || weapon->flashDlightColor[1] || weapon->flashDlightColor[2]) {
			trap_R_AddLightToScene(flash.origin, radius, WEAPON_FLASH_INTENSITY,
									weapon->flashDlightColor[0], weapon->flashDlightColor[1],
									weapon->flashDlightColor[2], 0, 0);
        }
    }
}

/*
==============
CG_AddViewWeapon

Add the weapon, and flash for the player's view
==============
*/
void CG_AddViewWeapon( playerState_t *ps ) {
	refEntity_t		hand;
	centity_t		*cent;
	clientInfo_t	*ci;
	float			fovOffset;
	vec3_t			angles;
	weaponInfo_t	*weapon;
	weapon_t		weaponNum;
	//bg_q3f_playerclass_t *cls;
	//int starttime, endtime;
	int agentclass;
	qboolean		drawmodel, newmodel;
	float			shaderalpha;
	F2RDef_t		*F2RScript;
	int				animNumber;
	float			weapFov = cg_fovViewmodel.integer ? cg_fovViewmodel.value : cg_fov.value;

	// RR2DO2: added players without a class
	if ( ps->persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR 
		 || ps->persistant[PERS_CURRCLASS]==Q3F_CLASS_NULL) {
		return;
	}

	if ( ps->pm_type == PM_INTERMISSION ) {
		return;
	}

	// no gun if in third person view or a camera is active
	if ( cg.renderingThirdPerson || cg.renderingFlyBy || cg.rendering2ndRefDef ) {
		return;
	}

	// allow the gun to be completely removed
   if ( !cg_drawGun.integer ) { // && ps->weapon != WP_FLAMETHROWER ) {
		vec3_t		origin;

		if ( cg.predictedPlayerState.eFlags & EF_FIRING ) {
			// special hack for lightning gun...
			VectorCopy( cg.refdef.vieworg, origin );
			//VectorMA( origin, -8, cg.refdef.viewaxis[2], origin );

			VectorMA( origin, 18, cg.refdef_current->viewaxis[0], origin );
			VectorMA( origin, -7, cg.refdef_current->viewaxis[1], origin );
			VectorMA( origin, -4, cg.refdef_current->viewaxis[2], origin );

	        // Keeger, brought in from ET...Flamethrower effect
			CG_FlamethrowerFlame( &cg.predictedPlayerEntity, origin  );
		}
		return;
	}

	// don't draw if testing a gun model
	if ( cg.testGun ) {
		return;
	}

	// drop gun lower at higher fov
	if ( cg_fovViewmodelAdjust.integer && weapFov > 90 ) {
		fovOffset = -0.2f * ( weapFov - 90 );
	} else {
		fovOffset = 0;
	}

	cent = &cg.predictedPlayerEntity;	// &cg_entities[cg.snap->ps.clientNum];
	//CG_RegisterWeapon( ps->weapon );
	weaponNum = cent->currentState.weapon;
	weapon = &cg_weapons[ ps->weapon ];
	memset (&hand, 0, sizeof(hand));

	// set up gun position
	CG_CalculateWeaponPosition( hand.origin, angles );

	VectorMA( hand.origin, cg_gun_x.value, cg.refdef.viewaxis[0], hand.origin );
	VectorMA( hand.origin, cg_gun_y.value, cg.refdef.viewaxis[1], hand.origin );
	VectorMA( hand.origin, (cg_gun_z.value+fovOffset), cg.refdef.viewaxis[2], hand.origin );

	AnglesToAxis( angles, hand.axis );

	if ( cg_fovViewmodel.integer )
	{
		float fracDistFOV = tanf( cg.refdef.fov_x * ( M_PI/180 ) * 0.5f );
		float fracWeapFOV = ( 1.0f / fracDistFOV ) * tanf( weapFov * ( M_PI/180 ) * 0.5f );
		VectorScale( hand.axis[0], fracWeapFOV, hand.axis[0] );
	}

	// map torso animations to weapon animations
	/*if ( cg_gun_frame.integer ) {
		// development tool
		hand.frame = hand.oldframe = cg_gun_frame.integer;
		hand.backlerp = 0;
	} else {
		// get clientinfo for animation map
		ci = &cgs.clientinfo[ cent->currentState.clientNum ];
		hand.frame = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.frame, weaponNum );	// RR2DO2
		hand.oldframe = CG_MapTorsoToWeaponFrame( ci, cent->pe.torso.oldFrame, weaponNum );	// RR2DO2
		hand.backlerp = cent->pe.torso.backlerp;
	}*/

	// disguised agent hand models fix
	agentclass = 0;
	ci = &cgs.clientinfo[ cent->currentState.clientNum ];

	if ( cent->currentState.otherEntityNum2 == Q3F_CLASS_AGENT && cg.agentDataEntity && cg.agentDataEntity->currentValid ) {
		CG_Q3F_CalcAgentVisibility(	&drawmodel, &shaderalpha, &newmodel, 1.0f/6.0f, 5.0f/6.0f, &cg.agentDataEntity->currentState );
		if( newmodel )
			agentclass = cg.agentDataEntity->currentState.torsoAnim;
		else
			agentclass = cg.agentLastClass;

		if ( agentclass ) {
			// Change the weapon we want to render
			weaponNum = BG_Q3F_GetRemappedWeaponFromWeaponNum( ci->cls, agentclass, weaponNum );
			weapon = &cg_weapons[ weaponNum ];

			// Get weapon animation number
			animNumber = cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT;

			// Override animations when we are reloading
			// move weapon in view
			if( cg.reloadendtime + 80 > cg.time )
				animNumber = ANI_INTERACT_WPCHANGE_END;
			// move weapon out of view
			else if( cg.predictedPlayerState.weaponstate == WEAPON_RDROPPING ||
					 cg.predictedPlayerState.weaponstate == WEAPON_RELOADING )
				animNumber = ANI_INTERACT_WPCHANGE_START;
			// Map any special animations to idle animations
			else if( animNumber == ANI_DEATH_1 ||
					 animNumber == ANI_DEAD_1 ||
					 animNumber == ANI_DEATH_2 ||
					 animNumber == ANI_DEAD_2 ||
					 animNumber == ANI_SIGNAL_POINT ||
					 animNumber == ANI_SIGNAL_STOP ||
					 animNumber == ANI_SIGNAL_BECON ||
					 animNumber == ANI_SIGNAL_LOOK ||
					 animNumber == ANI_SIGNAL_WAVEYES ||
					 animNumber == ANI_SIGNAL_WAVENO ||
					 animNumber == ANI_SIGNAL_TAUNT ||
					 animNumber == ANI_INTERACT_OPERATING ||
					 animNumber == ANI_INTERACT_THROW ||
					 animNumber == ANI_SPECIAL )
					 animNumber = PM_GetIdleAnim( cent->currentState.weapon, ci->cls );

			F2RScript = CG_Q3F_TorsoF2RScript( agentclass );
			animNumber = BG_Q3F_GetRemappedAnimFromWeaponNumAndAnim( cent->currentState.weapon, ci->cls, weaponNum, agentclass, animNumber );
		} else {
			// Default weapon animation number
			animNumber = cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT;

			// Override animations when we are reloading
			// move weapon in view
			if( cg.reloadendtime + 80 > cg.time )
				animNumber = ANI_INTERACT_WPCHANGE_END;
			// move weapon out of view
			else if( cg.predictedPlayerState.weaponstate == WEAPON_RDROPPING ||
					cg.predictedPlayerState.weaponstate == WEAPON_RELOADING )
				animNumber = ANI_INTERACT_WPCHANGE_START;
			// Map any special animations to idle animations
			else if( animNumber == ANI_DEATH_1 ||
					animNumber == ANI_DEAD_1 ||
					animNumber == ANI_DEATH_2 ||
					animNumber == ANI_DEAD_2 ||
					animNumber == ANI_SIGNAL_POINT ||
					animNumber == ANI_SIGNAL_STOP ||
					animNumber == ANI_SIGNAL_BECON ||
					animNumber == ANI_SIGNAL_LOOK ||
					animNumber == ANI_SIGNAL_WAVEYES ||
					animNumber == ANI_SIGNAL_WAVENO ||
					animNumber == ANI_SIGNAL_TAUNT ||
					animNumber == ANI_INTERACT_OPERATING ||
					animNumber == ANI_INTERACT_THROW ||
					animNumber == ANI_SPECIAL )
					animNumber = PM_GetIdleAnim( cent->currentState.weapon, ci->cls );
			else
			animNumber = cent->currentState.torsoAnim;	// maintain the togglebit
		}
	} else {
		// Default weapon animation number
		animNumber = cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT;

		// Override animations when we are reloading
		// move weapon in view
		if( cg.reloadendtime + 80 > cg.time )
			animNumber = ANI_INTERACT_WPCHANGE_END;
		// move weapon out of view
		else if( cg.predictedPlayerState.weaponstate == WEAPON_RDROPPING ||
				 cg.predictedPlayerState.weaponstate == WEAPON_RELOADING )
			animNumber = ANI_INTERACT_WPCHANGE_START;
		// Map any special animations to idle animations
		else if( animNumber == ANI_DEATH_1 ||
				 animNumber == ANI_DEAD_1 ||
				 animNumber == ANI_DEATH_2 ||
				 animNumber == ANI_DEAD_2 ||
				 animNumber == ANI_SIGNAL_POINT ||
				 animNumber == ANI_SIGNAL_STOP ||
				 animNumber == ANI_SIGNAL_BECON ||
				 animNumber == ANI_SIGNAL_LOOK ||
				 animNumber == ANI_SIGNAL_WAVEYES ||
				 animNumber == ANI_SIGNAL_WAVENO ||
				 animNumber == ANI_SIGNAL_TAUNT ||
				 animNumber == ANI_INTERACT_OPERATING ||
				 animNumber == ANI_INTERACT_THROW ||
				 animNumber == ANI_SPECIAL )
				 animNumber = PM_GetIdleAnim( cent->currentState.weapon, ci->cls );
		else
			animNumber = cent->currentState.torsoAnim;	// maintain the togglebit
	}

	if(weaponNum == WP_AXE)
		switch( agentclass ? agentclass : cent->currentState.otherEntityNum2)
		{
			case Q3F_CLASS_AGENT:
				hand.hModel = cg_extendedweapons[Q3F_WP_KNIFE].handsModel;
				F2RScript = cg_extendedweapons[Q3F_WP_KNIFE].F2RScript;
				break;
			case Q3F_CLASS_PARAMEDIC:
				hand.hModel = cg_extendedweapons[Q3F_WP_BIOAXE].handsModel;
				F2RScript = cg_extendedweapons[Q3F_WP_BIOAXE].F2RScript;
				break;
			case Q3F_CLASS_ENGINEER:
				hand.hModel = cg_extendedweapons[Q3F_WP_WRENCH].handsModel;
				F2RScript = cg_extendedweapons[Q3F_WP_WRENCH].F2RScript;
				break;
			default:
				hand.hModel = weapon->handsModel;
				F2RScript = weapon->F2RScript;
		}
	else {
		hand.hModel = weapon->handsModel;
		F2RScript = weapon->F2RScript;
	}

	hand.renderfx = RF_DEPTHHACK | RF_FIRST_PERSON | RF_MINLIGHT;

	// animate
	//slothy -- note to self.. this is weapon movement animation, eg needle stabbing, axe chopping etc
	CG_RunLerpFrame( F2RScript, &cent->pe.hands, animNumber, 1.f );
	hand.oldframe = cent->pe.hands.oldFrame;
	hand.frame = cent->pe.hands.frame;
	hand.backlerp = cent->pe.hands.backlerp;

	// add everything onto the hand
	CG_AddPlayerWeapon( &hand, ps, &cg.predictedPlayerEntity, ps->persistant[PERS_TEAM], NULL );
}

/*
==============================================================================

WEAPON SELECTION

==============================================================================
*/


weaponInfo_t *CG_Q3F_GetWeaponStruct(int clsnum, int weapon)
{
	if(weapon != WP_AXE)
		return(&(cg_weapons[weapon]));
	
	switch(clsnum)
	{
		case Q3F_CLASS_ENGINEER:
			return(&(cg_extendedweapons[Q3F_WP_WRENCH]));

		case Q3F_CLASS_PARAMEDIC:
			return(&(cg_extendedweapons[Q3F_WP_BIOAXE]));

		case Q3F_CLASS_AGENT:
			return(&(cg_extendedweapons[Q3F_WP_KNIFE]));
		
		default:
			return(&(cg_weapons[weapon]));
	}
}


/*
===============
CG_WeaponSelectable
===============
*/
static qboolean CG_WeaponSelectable( int i ) {

	int realweapon;
	bg_q3f_weapon_t *wp;

	realweapon = i;

	if (realweapon == 0)
		return qfalse;

	if ( ! (cg.snap->ps.stats[ STAT_WEAPONS ] & ( 1 << realweapon ) ) ) {
		return qfalse;
	}

	wp = BG_Q3F_GetWeapon( i );
	if ( cg.snap->ps.ammo[wp->ammotype] < wp->numammo && wp->numammo) {		// JT: Does it have less ammo than a single shot needs?
		return qfalse;
	}

	return qtrue;
}

/*
===============
CG_Q3F_WeaponSelectable
===============
*/
static qboolean CG_Q3F_WeaponSelectable( int i ) {

	int realweapon;
	bg_q3f_weapon_t *wp;
	realweapon = BG_Q3F_GetClass(&(cg.snap->ps))->weaponslot[i];

	if (realweapon == 0)
		return qfalse;

	if( !BG_Q3F_GetWeaponSlotFromWeaponNum( &cg.snap->ps, realweapon ) ) {
	//if ( ! (cg.snap->ps.stats[ STAT_WEAPONS ] & ( 1 << realweapon ) ) ) {
		return qfalse;
	}

	wp = BG_Q3F_GetWeapon( realweapon );

	if ( cg.snap->ps.ammo[wp->ammotype] < wp->numammo && wp->numammo ) {
		return qfalse;
	}

	return qtrue;
}


/*
===============
CG_NextWeapon_f
===============
*/
void CG_NextWeapon_f( void ) {
	int		i;
	int		original;
	int		slot;


	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW || cg.snap->ps.pm_flags & PMF_CHASE ) {

		// JT - Make weapon cycling cycle through people
		if(cg.snap->ps.pm_flags & PMF_FOLLOW)
			trap_SendConsoleCommand( "follownext\n" );
		else
			trap_SendConsoleCommand( "chasenext\n" );

		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;


	slot = BG_Q3F_GetWeaponSlotFromWeaponNum(&(cg.snap->ps), cg.weaponSelect);

	for ( i = 0 ; i < Q3F_NUM_WEAPONSLOTS ; i++ ) {
		slot++;
		if ( slot == Q3F_NUM_WEAPONSLOTS ) {
			slot = 0;
		}
/* JT - Actually... _DO_ cycle to gauntlet
		if ( cg.weaponSelect == WP_AXE ) {
			continue;		// never cycle to gauntlet
		}

*/
		if ( CG_Q3F_WeaponSelectable( slot ) ) {
			break;
		}
	}
	if ( i == Q3F_NUM_WEAPONSLOTS ) {
		cg.weaponSelect = original;
	}
	else
	{
		cg.weaponSelect = BG_Q3F_GetClass(&(cg.snap->ps))->weaponslot[slot];
	}
}

/* 
===============
CG_PrevWeapon_f
===============
*/
void CG_PrevWeapon_f( void ) {
	int		i;
	int		original;
	int		slot;


	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW || cg.snap->ps.pm_flags & PMF_CHASE ) {

		// JT - Make weapon cycling cycle through people
		if(cg.snap->ps.pm_flags & PMF_FOLLOW)
			trap_SendConsoleCommand( "followprev\n" );
		else
			trap_SendConsoleCommand( "chaseprev\n" );

		return;
	}

	cg.weaponSelectTime = cg.time;
	original = cg.weaponSelect;

	slot = BG_Q3F_GetWeaponSlotFromWeaponNum(&(cg.snap->ps), cg.weaponSelect);

	for ( i = 0 ; i < Q3F_NUM_WEAPONSLOTS ; i++ ) {
		slot--;
		if ( slot == -1 ) {
			slot = Q3F_NUM_WEAPONSLOTS-1;
		}
/* JT - Actually _DO_ cycle to the Axe
		if ( cg.weaponSelect == WP_AXE ) {
			continue;		// never cycle to gauntlet
		}
	JT */
		if ( CG_Q3F_WeaponSelectable( slot ) ) {
			break;
		}
	}
	if ( i == Q3F_NUM_WEAPONSLOTS ) {
		cg.weaponSelect = original;
	}
	else
	{
		cg.weaponSelect = BG_Q3F_GetClass(&(cg.snap->ps))->weaponslot[slot];
	}
}

/* 
===============
CG_LastWeapon_f
===============
*/
void CG_LastWeapon_f( void ) {
	// Golliwog: Switch to last used weapon

	int		slot;

	if( !cg.snap ||
		(cg.snap->ps.pm_flags & PMF_FOLLOW) || cg.snap->ps.pm_flags & PMF_CHASE ||
		!cg.lastWeapon )
		return;

	slot = BG_Q3F_GetWeaponSlotFromWeaponNum(&(cg.snap->ps), cg.lastWeapon );
	if( CG_Q3F_WeaponSelectable( slot ) )
	{
		cg.weaponSelect = cg.lastWeapon;
		cg.weaponSelectTime = cg.time;
	}
}

/*
===============
CG_Weapon_f
===============
*/
void CG_Weapon_f( void ) {
	int		num;

	num = atoi( CG_Argv( 1 ) );

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW || cg.snap->ps.pm_flags & PMF_CHASE ) {
		return;
	}

	if ( num < 1 || num > 10 ) {
		return;
	}

	if( !CG_Q3F_WeaponSelectable( num ) )
		return;

	cg.weaponSelectTime = cg.time;

	num = BG_Q3F_GetClass(&(cg.snap->ps))->weaponslot[num];

	if ( ! num || ! ( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << num ) ) ) {
		return;		// don't have the weapon
	}

	cg.weaponSelect = num;
}

static weapon_t WeaponSlotTable[Q3F_CLASS_MAX][4] = {
	{ WP_NONE,	WP_NONE,			WP_NONE,			WP_NONE		},			/* NULL	*/
	{ WP_AXE,	WP_SHOTGUN,			WP_NAILGUN,			WP_NAILGUN	},			/* RECON */
	{ WP_AXE,	WP_SNIPER_RIFLE,	WP_ASSAULTRIFLE,	WP_NAILGUN	},			/* SNIPER */
	{ WP_AXE,	WP_SHOTGUN,			WP_SUPERSHOTGUN,	WP_ROCKET_LAUNCHER	},	/* SOLDIER */
	{ WP_AXE,	WP_SHOTGUN,			WP_GRENADE_LAUNCHER,WP_PIPELAUNCHER	},		/* GRENADIER */
	{ WP_AXE,	WP_SHOTGUN,			WP_SUPERSHOTGUN,	WP_SUPERNAILGUN	},		/* PARAMEDIC */
	{ WP_AXE,	WP_SHOTGUN,			WP_SUPERSHOTGUN,	WP_MINIGUN	},			/* MINIGUNNER */
	{ WP_AXE,	WP_SHOTGUN,			WP_FLAMETHROWER,	WP_NAPALMCANNON	},		/* FLAMETROOPER */
	{ WP_AXE,	WP_DARTGUN,			WP_SUPERSHOTGUN,	WP_NAILGUN},			/* AGENT */
	{ WP_AXE,	WP_RAILGUN,			WP_SUPERSHOTGUN,	WP_SUPERSHOTGUN},		/* ENGINEER */
	{ WP_AXE,	WP_AXE,				WP_AXE,				WP_AXE},				/* CIVILIAN */
};

void CG_WeaponSlot_f( void ) {
	int		num, index;

	num = atoi( CG_Argv( 1 ) );

	if ( !cg.snap ) {
		return;
	}
	if ( cg.snap->ps.pm_flags & PMF_FOLLOW || cg.snap->ps.pm_flags & PMF_CHASE ) {
		return;
	}

	if ( num < 1 || num > 4 ) {
		return;
	}
	index = cg.snap->ps.persistant[PERS_CURRCLASS];
	if (index >= Q3F_CLASS_MAX)
		return;
	num = WeaponSlotTable[index][num-1];
	if ( ! num || ! ( cg.snap->ps.stats[STAT_WEAPONS] & ( 1 << num ) ) ) {
		return;		// don't have the weapon
	}
	cg.weaponSelectTime = cg.time;
	cg.weaponSelect = num;
}

/*
===================
CG_OutOfAmmoChange

The current weapon has just run out of ammo
===================
*/
void CG_OutOfAmmoChange( void ) {
	int		i;

	cg.weaponSelectTime = cg.time;

	for ( i = 15 ; i > 0 ; i-- ) {
		if ( CG_WeaponSelectable( i ) ) {
			cg.weaponSelect = i;
			break;
		}
	}
}



/*
===================================================================================================

WEAPON EVENTS

===================================================================================================
*/

/*
================
CG_FireWeapon

Caused by an EV_FIRE_WEAPON event
================
*/
void CG_FireWeapon( centity_t *cent ) {
	entityState_t *ent;
	int				c;
	weaponInfo_t	*weap;

	ent = &cent->currentState;
	if ( ent->weapon == WP_NONE ) {
		return;
	}
	if ( ent->weapon >= WP_NUM_WEAPONS ) {
		CG_Error( "CG_FireWeapon: ent->weapon >= WP_NUM_WEAPONS" );
		return;
	}
	weap = CG_Q3F_GetWeaponStruct( ent->otherEntityNum2, ent->weapon );// &cg_weapons[ ent->weapon ];

	// RR2DO2: sniperrifle doesn't shoot or flash or anything when in air
	if( ent->weapon == WP_SNIPER_RIFLE && cg.snap->ps.groundEntityNum == ENTITYNUM_NONE )
		return;

	// mark the entity as muzzle flashing, so when it is added it will
	// append the flash to the weapon model
	cent->muzzleFlashTime = cg.time;

	// play quad sound if needed
	if ( cent->currentState.powerups & ( 1 << PW_QUAD ) ) {
		trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.media.quadSound );
	}

	// play a sound
	for ( c = 0 ; c < 4 ; c++ ) {
		if ( !weap->flashSound[c] ) {
			break;
		}
	}
	if ( c > 0 ) {
		c = rand() % c;
		if ( weap->flashSound[c] )
		{
			trap_S_StartSound( NULL, ent->number, CHAN_WEAPON, weap->flashSound[c] );
		}
	}

	// do brass ejection
	if ( weap->ejectBrassFunc && cg_brassTime.integer > 0 ) {
		cent->ejectBrassTime = cg.time;
		//weap->ejectBrassFunc( cent );
	}
	if(ent->weapon == WP_NAILGUN)
	{
		clientInfo_t	*ci;

		ci = &cgs.clientinfo[ cent->currentState.clientNum ];
		if(ci->flashtagnumber == 1)
			ci->flashtagnumber = 0;
		else
			ci->flashtagnumber = 1;
	}

	CG_PredictWeaponEffects( cent );
}


/*
==============
CG_WaterRipple
==============
*/
void CG_WaterRipple( qhandle_t shader, vec3_t loc, vec3_t dir, int size, int lifetime ) {
	localEntity_t   *le;
	refEntity_t     *re;

	le = CG_AllocLocalEntity(0);
	le->leType          = LE_SCALE_FADE;
	//le->leFlags         = LEF_PUFF_DONT_SCALE;

	le->startTime       = cg.time;
	le->endTime         = cg.time + lifetime;
	le->lifeRate        = 1.0 / ( le->endTime - le->startTime );

	re = &le->refEntity;
	VectorCopy( loc, re->origin );
	re->shaderTime      = cg.time / 1000.0f;
	re->reType          = RT_SPLASH;
	re->radius          = size;
	re->customShader    = shader;
	re->shaderRGBA[0]   = 0xff;
	re->shaderRGBA[1]   = 0xff;
	re->shaderRGBA[2]   = 0xff;
	re->shaderRGBA[3]   = 0xff;
	le->color[3]        = 1.0;
}


/*
=================
CG_MissileHitWall

Caused by an EV_MISSILE_MISS event, or directly by local bullet tracing
=================
*/
void CG_MissileHitWall( int weapon, int clientNum, vec3_t origin, vec3_t dir, impactSound_t soundType ) {
	qhandle_t		mark = 0;
	sfxHandle_t		sfx = 0;

	float			radius = 0; /* Ensiform - Default to 0 so as to stfu compiler */
	int				r;
	weaponInfo_t	*weap;
	qboolean		forcefield;
	trace_t			tr;
	vec3_t			end;



	// Golliwog: See if we're really hitting off a forcefield.
	VectorMA( origin, -10, dir, end );
	CG_Trace( &tr, origin, NULL, NULL, end, ENTITYNUM_NONE, MASK_SHOT );
	forcefield = (tr.entityNum != ENTITYNUM_NONE && (tr.contents & CONTENTS_FORCEFIELD)) ? qtrue : qfalse;
	// Golliwog.

	switch ( weapon & 15 ) {
	default:
		Com_Printf("Unhandled weapon impact %d\n", weapon );
		break;
	case WP_GRENADE_LAUNCHER:
	case WP_PIPELAUNCHER:
	case WP_ROCKET_LAUNCHER:
		mark = cgs.media.burnMarkShader;
		if (weapon > 15 ) CG_QuadExplosion( origin, cgs.clientinfo[clientNum].team );
		else CG_NormalExplosion( origin );
		radius = 40;
		break;
	case WP_NAPALMCANNON:
		mark = cgs.media.burnMarkShader;
		radius = 50;
		if (weapon > 15 ) CG_QuadExplosion( origin, cgs.clientinfo[clientNum].team );
		else Spirit_RunScript( cgs.spirit.explosion_napalm, origin, origin, axisDefault, 0);
		break;
	
	case WP_FLAMETHROWER:
		forcefield = qfalse;		//Always disable sparks
		//Pretty much does nothing :)
		break;

	case WP_SHOTGUN:
		CG_BulletExplosion( origin, dir );
		mark = cgs.media.bulletMarkShader;
		radius = 4;
		break;

	case WP_NAILGUN:
	case WP_DARTGUN:
		CG_BulletExplosion( origin, dir );
		mark = cgs.media.bulletMarkShader;

		r = rand() & 3;
		if ( r < 2 ) {
			sfx = cgs.media.sfx_ric1;
		} else if ( r == 2 ) {
			sfx = cgs.media.sfx_ric2;
		} else {
			sfx = cgs.media.sfx_ric3;
		}

			radius = 8;
		break;

	case WP_RAILGUN:
//		impactmod = cgs.media.ringFlashModel;
//		shader = cgs.media.railExplosionShader;
		mark = cgs.media.energyMarkShader;
		sfx = cgs.media.sfx_railhit;
		radius = 16;
		break;

	case WP_AXE:
		weap = CG_Q3F_GetWeaponStruct( cgs.clientinfo[clientNum].cls, WP_AXE );
		{
			int		c;

			for ( c = 0 ; c < 3 ; c++ ) {
				if ( !weap->wallSound[(int)soundType][c] ) {
					break;
				}
			}

			if ( c > 0 ) {
				c = rand() % c;
				if ( weap->wallSound[(int)soundType][c] )
				{
					sfx = weap->wallSound[(int)soundType][c];
				}
			}
		}
		//sfx = weap->wallSound;
		break;

	case WP_SNIPER_RIFLE:
		CG_BulletExplosion( origin, dir );
		mark = cgs.media.bulletMarkShader;
		
		r = rand() & 3;
		if ( r < 2 ) {
			sfx = cgs.media.sfx_ric1;
		} else if ( r == 2 ) {
			sfx = cgs.media.sfx_ric2;
		} else {
			sfx = cgs.media.sfx_ric3;
		}

		radius = 12;
		break;
	}

	if ( sfx ) {
		trap_S_StartSound( origin, ENTITYNUM_WORLD, CHAN_AUTO, sfx );
	}

	//Force field still enabled, run some sparks
	if( forcefield ) {
		const centity_t *field = &cg_entities[tr.entityNum];
		vec3_t axis[3];

		VectorNormalize2( tr.plane.normal, axis[2] );
		MakeNormalVectors( axis[2], axis[1], axis[0] );
		/* Ensiform - We do a check here because some surfaces are forcefield'd but not necessarily func_forcefield */
		if( field->currentValid && field->currentState.eType == ET_Q3F_FORCEFIELD && !VectorCompare( field->currentState.angles2, vec3_origin ) )
			Spirit_SetCustomColor( field->currentState.angles2 );
		else
			Spirit_SetCustomColor( colorWhite );
		Spirit_RunScript( cgs.spirit.forcefieldspark, origin, origin, axis, 0 );
	}

	if (mark == cgs.media.burnMarkShader )	{	
		CG_ExplosionMark( origin, radius, colorWhite );
	} else if (mark == cgs.media.energyMarkShader )	{	
		CG_OldMark( mark, origin, dir, 0, radius, colorWhite, cg_markTime.integer >> 1, LEMFT_ALPHA);
	} else if (mark) {
		//CG_OldMark(mark,origin,dir,0,radius,colorWhite,cg_markTime.integer,LEMFT_NORMAL);
		CG_BulletMark(mark,origin,dir,radius,colorWhite);
	}
}

static void CG_Q3F_AxeHitFlesh( vec3_t origin, vec3_t dir, int playerNum, int entityNum )
{
	// Hit someone with the axe, play the appropriate sound.

	weaponInfo_t	*weap;
	int				c;

	weap = CG_Q3F_GetWeaponStruct( cgs.clientinfo[playerNum].cls, WP_AXE );
	// play a sound
	for ( c = 0 ; c < 3 ; c++ ) {
		if ( !weap->fleshSound[c] ) {
			break;
		}
	}
	if ( c > 0 ) {
		c = rand() % c;
		if ( weap->fleshSound[c] )
		{
			trap_S_StartSound( origin, entityNum, CHAN_WEAPON, weap->fleshSound[c] );
		}
	}
}

/*
=================
CG_MissileHitPlayer
=================
*/
void CG_MissileHitPlayer( int weapon, vec3_t origin, vec3_t dir, int clientNum, int entityNum ) {
	CG_Bleed( origin, entityNum );

	// some weapons will make an explosion with the blood, while
	// others will just make the blood
	switch ( weapon & 15 ) {
	case WP_GRENADE_LAUNCHER:
	case WP_ROCKET_LAUNCHER:
	case WP_PIPELAUNCHER:
	case WP_NAPALMCANNON:
		CG_MissileHitWall( weapon, clientNum, origin, dir, IMPACTSOUND_FLESH );
		break;
	case WP_AXE:
		CG_Q3F_AxeHitFlesh( origin, dir, clientNum, entityNum );
	default:
		break;
	}
}



/*
============================================================================

SHOTGUN/MINIGUN TRACING

============================================================================
*/


/*
================
CG_MinigunPellet
================
*/
static void CG_MinigunPellet( vec3_t start, vec3_t end, int skipNum ) {
	trace_t		tr;
	vec4_t		rgba;
	vec3_t		bulletmins, bulletmaxs;

	VectorSet( bulletmins, -3, -3, -3 );
	VectorSet( bulletmaxs, 3, 3, 3 );

	CG_Trace( &tr, start, NULL, NULL, end, skipNum, MASK_SHOT );

	CG_BubbleTrail( start, tr.endpos, 32 );

	if (  tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	if ( cg_entities[tr.entityNum].currentState.eType == ET_PLAYER ) {
		CG_MissileHitPlayer( WP_SHOTGUN, tr.endpos, tr.plane.normal, skipNum, tr.entityNum );
	} else {
		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			// SURF_NOIMPACT will not make a flame puff or a mark
			return;
		}
		CG_MissileHitWall( WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_DEFAULT );
		//CG_Tracer(start, tr.endpos, qfalse);
		rgba[0]=rgba[1]=rgba[2]=rgba[3]=1.f;

		if ( Q_flrand(0.0f, 1.0f) > 0.8f )
			CG_Tracer(start, tr.endpos, qfalse, 1, cgs.media.tracerShader, rgba);
	}
	CG_Q3F_Vibrate( 1, tr.endpos );	// Golliwog: Each pellet does a single point of vibration
}



/*
================
CG_ShotgunPellet
================
*/
static void CG_ShotgunPellet( vec3_t start, vec3_t end, int skipNum ) {
	trace_t		tr;

	CG_Trace( &tr, start, NULL, NULL, end, skipNum, MASK_SHOT );
	CG_BubbleTrail( start, tr.endpos, 32 );

	if (  tr.surfaceFlags & SURF_NOIMPACT ) {
		return;
	}

	if ( cg_entities[tr.entityNum].currentState.eType == ET_PLAYER ) {
		CG_MissileHitPlayer( WP_SHOTGUN, tr.endpos, tr.plane.normal, skipNum, tr.entityNum );
	} else {
		if ( tr.surfaceFlags & SURF_NOIMPACT ) {
			// SURF_NOIMPACT will not make a flame puff or a mark
			return;
		}
		CG_MissileHitWall( WP_SHOTGUN, 0, tr.endpos, tr.plane.normal, IMPACTSOUND_DEFAULT );
	}
	CG_Q3F_Vibrate( 1, tr.endpos );	// Golliwog: Each pellet does a single point of vibration
}

/*
================
CG_ShotgunPattern

Perform the same traces the server did to locate the
hit splashes
================
*/
void CG_ShotgunPattern( vec3_t origin, vec3_t origin2, int otherEntNum, int seed ) {
	int			i;
	float		r, u;
	vec3_t		end;
	vec3_t		forward, right, up, v;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );
	
	if (!cg_lowEffects.integer && cg_shotgunPuff.integer && !(CG_PointContents( origin, 0) & CONTENTS_WATER )) {
		static const vec4_t color =	{1, 1, 1, 0.55};
		static const vec3_t vel =	{0, 0, 8};
		VectorMA( origin, 32, forward, v );
		CG_SmokePuff( v, vel, 500, 100, 20, 25, color, cgs.media.shotgunSmokePuffShader );
	}
	// generate the "random" spread pattern
	for ( i = 0 ; i < DEFAULT_SHOTGUN_COUNT ; i++ ) {
		r = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 16;
		u = Q_crandom( &seed ) * DEFAULT_SHOTGUN_SPREAD * 16;
		VectorMA( origin, 8192 * 16, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		CG_ShotgunPellet( origin, end, otherEntNum );
	}
}

void CG_MinigunPattern( vec3_t origin, vec3_t origin2, int otherEntNum, int seed, int spread ) {
	int			i;
	float		r, u, factor;
	vec3_t		end;
	vec3_t		forward, right, up;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	factor = DEFAULT_MINIGUN_PELLET_SPREAD + (EXTRA_MINIGUN_PELLET_SPREAD * spread) / 15;

	// generate the "random" spread pattern
	for ( i = 0 ; i < DEFAULT_MINIGUN_PELLET_COUNT ; i++ ) {
		r = Q_crandom( &seed ) * factor * 16;
		u = Q_crandom( &seed ) * factor * 16;
		VectorMA( origin, 8192 * 16, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		CG_MinigunPellet( origin, end, otherEntNum );
	}
}


/*
================
CG_SingleShotgunPattern

Perform the same traces the server did to locate the
hit splashes (FIXME: ranom seed isn't synce anymore)
================
*/
void CG_SingleShotgunPattern( vec3_t origin, vec3_t origin2, int otherEntNum, int seed ) {
	int			i;
	float		r, u;
	vec3_t		end;
	vec3_t		forward, right, up, v;

	// derive the right and up vectors from the forward vector, because
	// the client won't have any other information
	VectorNormalize2( origin2, forward );
	PerpendicularVector( right, forward );
	CrossProduct( forward, right, up );

	if (!cg_lowEffects.integer && cg_shotgunPuff.integer && !(CG_PointContents( origin, 0) & CONTENTS_WATER )) {
		static const vec4_t color =	{1, 1, 1, 0.50};
		static const vec3_t vel =	{0, 0, 8};
		VectorMA( origin, 24, forward, v );
		CG_SmokePuff( v, vel, 500, 100, 15, 20, color, cgs.media.shotgunSmokePuffShader );
	}

	// generate the "random" spread pattern
	for ( i = 0 ; i < DEFAULT_SINGLE_SHOTGUN_COUNT ; i++ ) {
		r = Q_crandom( &seed ) * DEFAULT_SINGLE_SHOTGUN_SPREAD * 16;
		u = Q_crandom( &seed ) * DEFAULT_SINGLE_SHOTGUN_SPREAD * 16;
		VectorMA( origin, 8192 * 16, forward, end);
		VectorMA (end, r, right, end);
		VectorMA (end, u, up, end);

		CG_ShotgunPellet( origin, end, otherEntNum );
	}
}



/*
===============
CG_Tracer
===============
*/
void CG_Tracer( vec3_t source, vec3_t dest, qboolean sound, float width, qhandle_t shader, vec4_t rgba ) {
	vec3_t		forward, right;
	polyVert_t	verts[4];
	vec3_t		line;
	float		len, begin, end;
	vec3_t		start, finish;
	vec3_t		midpoint;

	// tracer
	VectorSubtract( dest, source, forward );
	len = VectorNormalize( forward );

	// start at least a little ways from the muzzle
	begin = width * -0.5;
	end = len + width * 0.5;
	VectorMA( source, begin, forward, start );
	VectorMA( source, end, forward, finish );
	line[0] = DotProduct( forward, cg.refdef.viewaxis[1] );
	line[1] = DotProduct( forward, cg.refdef.viewaxis[2] );

	VectorScale( cg.refdef.viewaxis[1], line[1], right );
	VectorMA( right, -line[0], cg.refdef.viewaxis[2], right );
	VectorNormalize( right );

	VectorMA( finish, width, right, verts[0].xyz );
	verts[0].st[0] = 0;
	verts[0].st[1] = 1;
	verts[0].modulate[0] = 255 * rgba[0];
	verts[0].modulate[1] = 255 * rgba[1];
	verts[0].modulate[2] = 255 * rgba[2];
	verts[0].modulate[3] = 255 * rgba[3];
	
	VectorMA( finish, -width, right, verts[1].xyz );
	verts[1].st[0] = 1;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = 255 * rgba[0];
	verts[1].modulate[1] = 255 * rgba[1];
	verts[1].modulate[2] = 255 * rgba[2];
	verts[1].modulate[3] = 255 * rgba[3];

	VectorMA( start, -width, right, verts[2].xyz );
	verts[2].st[0] = 1;
	verts[2].st[1] = 0;
	verts[2].modulate[0] = 255 * rgba[0];
	verts[2].modulate[1] = 255 * rgba[1];
	verts[2].modulate[2] = 255 * rgba[2];
	verts[2].modulate[3] = 255 * rgba[3];

	VectorMA( start, width, right, verts[3].xyz );
	verts[3].st[0] = 0;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255 * rgba[0];
	verts[3].modulate[1] = 255 * rgba[1];
	verts[3].modulate[2] = 255 * rgba[2];
	verts[3].modulate[3] = 255 * rgba[3];

	trap_R_AddPolyToScene( shader, 4, verts );

	midpoint[0] = ( start[0] + finish[0] ) * 0.5;
	midpoint[1] = ( start[1] + finish[1] ) * 0.5;
	midpoint[2] = ( start[2] + finish[2] ) * 0.5;

	// add the tracer sound
	if(sound)
		trap_S_StartSound( midpoint, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.tracerSound );

}


/*
======================
CG_CalcMuzzlePoint
======================
*/
qboolean	CG_CalcMuzzlePoint( int entityNum, vec3_t muzzle, vec3_t forward ) {
	centity_t	*cent;
	int			anim;
	vec3_t		realforward;

	if ( entityNum == cg.snap->ps.clientNum ) {
		VectorCopy( cg.snap->ps.origin, muzzle );
		muzzle[2] += cg.snap->ps.viewheight;
		AngleVectors( cg.snap->ps.viewangles, realforward, NULL, NULL );
		VectorMA( muzzle, 14, realforward, muzzle );
		if( forward )
			memcpy( forward, realforward, sizeof(vec3_t) );
		return qtrue;
	}

	cent = &cg_entities[entityNum];
	if ( !cent->currentValid ) {
		return qfalse;
	}

	VectorCopy( cent->currentState.pos.trBase, muzzle );

	AngleVectors( cent->currentState.apos.trBase, realforward, NULL, NULL );
	anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
	if ( anim == ANI_MOVE_WALKCROUCH || anim == ANI_MOVE_IDLECROUCH ) {
		muzzle[2] += CROUCH_VIEWHEIGHT;
	} else {
		muzzle[2] += DEFAULT_VIEWHEIGHT;
	}

	VectorMA( muzzle, 14, realforward, muzzle );
	if( forward )
		memcpy( forward, realforward, sizeof(vec3_t) );

	return qtrue;
}

/*
======================
CG_Bullet

Renders bullet effects.
======================
*/
void CG_Bullet( vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum ) {
	vec3_t		start;
	vec4_t		rgba;

	// if the shooter is currently valid, calc a source point and possibly
	// do trail effects
	
	if ( sourceEntityNum >= 0 && cg_tracerChance.value > 0 ) {
		if ( CG_CalcMuzzlePoint( sourceEntityNum, start, NULL ) ) {
			CG_BubbleTrail( start, end, 32 );	
			// draw a tracer
			if ( Q_flrand(0.0f, 1.0f) < cg_tracerChance.value ) {
				//CG_Tracer( start, end, qtrue );
				rgba[0]=rgba[1]=rgba[2]=rgba[3]=1.f;
				CG_Tracer(start, end, qtrue, 1, cgs.media.tracerShader, rgba);
			}
		}
	}

	// impact splash and mark
	if ( flesh ) {
		CG_Bleed( end, fleshEntityNum );
	} else {
		CG_MissileHitWall( WP_NAILGUN, 0, end, normal, IMPACTSOUND_DEFAULT );
	}

}

/*
======================
CG_Sniper_Bullet

Renders bullet effects.
======================
*/
void CG_SniperBullet( vec3_t end, int sourceEntityNum, vec3_t normal, qboolean flesh, int fleshEntityNum ) {
	vec3_t		start;
	vec4_t		rgba;

	if ( sourceEntityNum >= 0 && cg_tracerChance.value > 0 ) {
		if ( CG_CalcMuzzlePoint( sourceEntityNum, start, NULL ) ) {
			CG_BubbleTrail( start, end, 32 );	
			// draw a tracer
			if ( Q_flrand(0.0f, 1.0f) < cg_tracerChance.value ) {
				//CG_Tracer( start, end, qtrue );
				rgba[0]=rgba[1]=rgba[2]=rgba[3]=1.f;
				CG_Tracer(start, end, qtrue, 1, cgs.media.tracerShader, rgba);
			}
		}
	}

	// impact splash and mark
	if ( flesh ) {
		CG_Bleed( end, fleshEntityNum );
	} else {
		CG_MissileHitWall( WP_NAILGUN, 0, end, normal, IMPACTSOUND_DEFAULT );
	}
}

