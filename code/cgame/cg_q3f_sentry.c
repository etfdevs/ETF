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
**	cg_q3f_sentry.c
**
**	Handles all sentry and supplystation rendering and animation
*/

#include "cg_local.h"
#include "cg_q3f_panel.h"

void CG_Q3F_RegisterSentry( void )
{
	//Always load the rocket launcher for sentries too
	CG_RegisterWeapon( WP_ROCKET_LAUNCHER );

	cgs.media.sentryBase = trap_R_RegisterModel( "models/objects/sentry/sentry_base.md3" );
	cgs.media.sentryTurret1 = trap_R_RegisterModel( "models/objects/sentry/sentry_turret.md3" );
	cgs.media.sentryTurret2 = trap_R_RegisterModel( "models/objects/sentry/sentry_turret_l2.md3" );
	cgs.media.sentryTurret3 = trap_R_RegisterModel( "models/objects/sentry/sentry_turret_l3.md3" );
	cgs.media.sentryCannon1 = trap_R_RegisterModel( "models/objects/sentry/sentry_turret_inner.md3" );
	cgs.media.sentryCannon2 = trap_R_RegisterModel( "models/objects/sentry/sentry_turret_inner_l2.md3" );
	cgs.media.sentryCannon3 = trap_R_RegisterModel( "models/objects/sentry/sentry_turret_inner_l3.md3" );
	cgs.media.sentryBarrel = trap_R_RegisterModel( "models/objects/sentry/sentry_minigun.md3" );
	cgs.media.sentryRocketLauncher = trap_R_RegisterModel( "models/objects/sentry/sentry_rocketl.md3" );
	cgs.media.sentryFlash = trap_R_RegisterModel( "models/objects/sentry/sentry_flash.md3" );
	cgs.media.sentryBits[0] = trap_R_RegisterModel( "models/objects/sentry/sentry_bit1.md3" );
	cgs.media.sentryBits[1] = trap_R_RegisterModel( "models/objects/sentry/sentry_bit2.md3" );
	cgs.media.sentryBits[2] = trap_R_RegisterModel( "models/objects/sentry/sentry_bit3.md3" );
	cgs.media.sentryBits[3] = trap_R_RegisterModel( "models/objects/sentry/sentry_bit4.md3" );
	cgs.media.sentrySpinupSound = trap_S_RegisterSound( "sound/movers/motors/motor_start_01.wav", qfalse );
	cgs.media.sentryFireSound = trap_S_RegisterSound( "sound/weapons/deploy/sentry_fire.wav", qfalse );
	cgs.media.sentryStartSound = trap_S_RegisterSound( "sound/weapons/deploy/sentry_seek.wav", qfalse );
	cgs.media.sentryStopSound = trap_S_RegisterSound( "sound/weapons/deploy/sentry_reset.wav", qfalse );
	cgs.media.sentryBuildSound = trap_S_RegisterSound( "sound/weapons/deploy/sentry_build.wav", qtrue );
	cgs.media.sentryUpgradeSound = trap_S_RegisterSound( "sound/weapons/deploy/sentry_upgrade.wav", qtrue );
	cgs.media.sentryExplodeSound = trap_S_RegisterSound( "sound/weapons/deploy/sentry_explode.wav", qtrue );
	// start hack of the year!
	if ( r_vertexLight.integer )
		trap_Cvar_Set( "r_vertexlight", "0" );
	cgs.media.sentryConstruct_Base = trap_R_RegisterShader( "models/objects/sentry/texture_sentry_base_construct" );
	cgs.media.sentryConstructShader_1 = trap_R_RegisterShader( "models/objects/sentry/texture_sentry_level1_construct" );
	cgs.media.sentryConstructShader_2 = trap_R_RegisterShader( "models/objects/sentry/texture_sentry_level2_construct" );
	cgs.media.sentryTvFx = trap_R_RegisterShader( "gfx/sfx/sentryCamTvBlur" );
	// exit hack of the year!
	if ( r_vertexLight.integer )
		trap_Cvar_Set( "r_vertexlight", "1" );
}

void CG_Q3F_RegisterSupplyStation( void )
{
	cgs.media.supplystationBase = trap_R_RegisterModel( "models/objects/supplystation/supply.md3" );
	cgs.media.supplystationHUD = trap_R_RegisterModel( "models/objects/supplystation/supply_hud.md3" );
	cgs.media.supplystationBits[0] = trap_R_RegisterModel( "models/objects/supplystation/supplystation_bit1.md3" );
	cgs.media.supplystationBits[1] = trap_R_RegisterModel( "models/objects/supplystation/supplystation_bit2.md3" );
	cgs.media.supplystationBits[2] = trap_R_RegisterModel( "models/objects/supplystation/supplystation_bit3.md3" );

	cgs.media.supplyBuildSound = trap_S_RegisterSound( "sound/weapons/deploy/supply_build.wav", qtrue );
	cgs.media.supplyPopup = trap_S_RegisterSound( "sound/weapons/deploy/supply_out.wav", qfalse );
	cgs.media.supplyRetract = trap_S_RegisterSound( "sound/weapons/deploy/supply_in.wav", qfalse );
	cgs.media.supplyExplodeSound = trap_S_RegisterSound( "sound/weapons/deploy/supply_explode.wav", qtrue );

	// start hack of the year!
	if ( r_vertexLight.integer )
		trap_Cvar_Set( "r_vertexlight", "0" );
	cgs.media.supplystationConstruct_Base = trap_R_RegisterShader( "models/objects/supplystation/base_construct" );
	cgs.media.supplystationConstruct_Screen = trap_R_RegisterShader( "models/objects/supplystation/screen_construct" );
	// exit hack of the year!
	if ( r_vertexLight.integer )
		trap_Cvar_Set( "r_vertexlight", "1" );
}

/*
===============
CG_Q3F_BuildableFloatSprite

Float a sprite over the sentry/supplystation
===============
*/
static void CG_Q3F_BuildableFloatSprite(centity_t * cent, qhandle_t shader, int height, qboolean depthhack)
{
	int             rf;
	refEntity_t     ent;

	/*if(cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson && !cg.renderingFlyBy &&
		!cg.rendering2ndRefDef)
	{
		rf = RF_THIRD_PERSON;	// only show in mirrors
	}
	else
	{
		rf = 0;
	}*/

	if(depthhack)
		rf = RF_DEPTHHACK;
	else
		rf = 0;

	memset(&ent, 0, sizeof(ent));
	VectorCopy(cent->lerpOrigin, ent.origin);
	ent.origin[2] += height;
	ent.reType = RT_SPRITE;
	ent.customShader = shader;
	ent.radius = 10;
	ent.renderfx = rf;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 255;
	trap_R_AddRefEntityToScene(&ent, cent);
}

static QINLINE int CG_Q3F_SentryMaxHealth(int level)
{
	switch (level)
	{
	case 1:
		return (150);
	case 2:
		return (180);
	case 3:
		return (220);
	default:
		return (0);
	}
}

static QINLINE int CG_Q3F_SentryNeedsShells(int level)
{
	switch (level)
	{
	case 1:
		return (70); //100 = max
	case 2:
		return (90); // 120 = max
	case 3:
		return (100); // 150 = max
	default:
		return (0);
	}
}

static QINLINE int CG_Q3F_SupplyStationMaxHealth( int supplevel )
{
	switch( supplevel )
	{
		case 1:		return( 150 );
		case 2:		return( 180 );
		case 3:		return( 220 );
		case 4:		return( 250 );
		default:	return( 0 );
	}
}

static QINLINE qboolean CG_Q3F_CanUpgrade(void)
{
	return (cg.snap->ps.ammo[AMMO_CELLS] >= 130) ? qtrue : qfalse;
}

static QINLINE qboolean CG_Q3F_CanUpgradeSupply(void)
{
	return (cg.snap->ps.ammo[AMMO_CELLS] >= 100) ? qtrue : qfalse;
}

static QINLINE qboolean CG_Q3F_HasCells(void)
{
	return (cg.snap->ps.ammo[AMMO_CELLS] > 0) ? qtrue : qfalse;
}

static QINLINE qboolean CG_Q3F_HasShells(void)
{
	return (cg.snap->ps.ammo[AMMO_SHELLS] > 0) ? qtrue : qfalse;
}

static QINLINE qboolean CG_Q3F_HasRockets(void)
{
	return (cg.snap->ps.ammo[AMMO_ROCKETS] > 0) ? qtrue : qfalse;
}

/*
===============
CG_Q3F_BuildableSprites

Float sprites over the sentry/supplystation
===============
*/
static void CG_Q3F_BuildableSprites(centity_t * cent)
{
	int             team;

	// If you're gassed, sprites's are out.
	if(cg.gasEndTime)
		return;

	team = cgs.clientinfo[cent->currentState.clientNum].team;

	if(!CG_AlliedTeam(cg.snap->ps.persistant[PERS_TEAM], team))
		return;

	if(cg.snap->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_ENGINEER || cg.snap->ps.stats[STAT_HEALTH] <= 0)
		return;

	// dead
	if(cent->currentState.torsoAnim <= 0)
		return;

	if((cent->currentState.eType == ET_Q3F_SENTRY && cent->currentState.legsAnim != 99))
	{
		int level = cent->currentState.legsAnim;
		if(CG_Q3F_HasCells() && cent->currentState.torsoAnim < CG_Q3F_SentryMaxHealth(level))
		{
			switch (level)
			{
			case 1:
				CG_Q3F_BuildableFloatSprite(cent, cgs.media.repairmeShader, 80, qtrue);
				break;
			case 2:
			case 3:
				CG_Q3F_BuildableFloatSprite(cent, cgs.media.repairmeShader, 84, qtrue);
				break;
			default:
				break;
			}
			return;
		}
		if((level == 1 || level == 2) && CG_Q3F_CanUpgrade())
		{
			switch (level)
			{
			case 1:
				CG_Q3F_BuildableFloatSprite(cent, cgs.media.upgrademeShader, 80, qtrue);
				break;
			case 2:
			case 3:
				CG_Q3F_BuildableFloatSprite(cent, cgs.media.upgrademeShader, 84, qtrue);
				break;
			default:
				break;
			}
			return;
		}
		if((CG_Q3F_HasShells() && cent->currentState.otherEntityNum <= CG_Q3F_SentryNeedsShells(level)) || (level == 3 && CG_Q3F_HasRockets() && cent->currentState.otherEntityNum2 < 10))
		{
			switch (level)
			{
			case 1:
				CG_Q3F_BuildableFloatSprite(cent, cgs.media.refillmeShader, 80, qtrue);
				break;
			case 2:
			case 3:
				CG_Q3F_BuildableFloatSprite(cent, cgs.media.refillmeShader, 84, qtrue);
				break;
			default:
				break;
			}
			return;
		}
	}

	if((cent->currentState.eType == ET_Q3F_SUPPLYSTATION && cent->currentState.legsAnim != 99))
	{
		int level = cent->currentState.legsAnim;
		if(CG_Q3F_HasCells() && cent->currentState.torsoAnim < CG_Q3F_SupplyStationMaxHealth(level))
		{
			CG_Q3F_BuildableFloatSprite(cent, cgs.media.repairmeShader, cent->currentState.frame || cg.rendering2ndRefDef ? 80 : 64, qtrue);
			return;
		}
		if((level == 1 || level == 2) && CG_Q3F_CanUpgradeSupply())
		{
			CG_Q3F_BuildableFloatSprite(cent, cgs.media.upgrademeShader, cent->currentState.frame || cg.rendering2ndRefDef ? 80 : 64, qtrue);
			return;
		}
		return;
	}
}

#ifdef RIMLIGHTING_OUTLINE
byte cg_q3f_sentrycolours[5][3] = {
	{ 0xFF, 0x20, 0x20 },		// RED
	{ 0x20, 0x20, 0xFF },		// BLUE
	{ 0xFF, 0xFF, 0x20 },		// YELLOW
	{ 0x20, 0xFF, 0x20 },		// GREEN
	{ 0xFF, 0xFF, 0xFF },		// FREE
};

#define Byte4Copy(a,b)		((b)[0]=(a)[0],(b)[1]=(a)[1],(b)[2]=(a)[2],(b)[3]=(a)[3])

void CG_BuildableWithOutline( refEntity_t *ent, entityState_t *state ) {
	int			team;
	qhandle_t	realShader;
	byte		color[4];
	vec3_t		axis[3];
	qboolean	_axis;

	trap_R_AddRefEntityToScene( ent, NULL );

	team = cgs.clientinfo[state->clientNum].team;

	if(!CG_AlliedTeam(cg.snap->ps.persistant[PERS_TEAM], team))
		return;

	if(cg.snap->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_ENGINEER || cg.snap->ps.stats[STAT_HEALTH] <= 0)
		return;

	// dead
	if(state->torsoAnim <= 0)
		return;

	realShader = ent->customShader;
	Byte4Copy(ent->shaderRGBA, color);
	VectorCopy(ent->axis[0], axis[0]);
	VectorCopy(ent->axis[1], axis[1]);
	VectorCopy(ent->axis[2], axis[2]);
	_axis = ent->nonNormalizedAxes;
	ent->customShader = cgs.media.outlineShader;
	ent->shaderRGBA[0] = cg_q3f_sentrycolours[team - Q3F_TEAM_RED][0];
	ent->shaderRGBA[1] = cg_q3f_sentrycolours[team - Q3F_TEAM_RED][1];
	ent->shaderRGBA[2] = cg_q3f_sentrycolours[team - Q3F_TEAM_RED][2];
	ent->shaderRGBA[3] = 25;
	VectorScale( ent->axis[0], 1.2f, ent->axis[0] );
	VectorScale( ent->axis[1], 1.2f, ent->axis[1] );
	VectorScale( ent->axis[2], 1.2f, ent->axis[2] );
	ent->nonNormalizedAxes = qtrue;
	trap_R_AddRefEntityToScene(ent, NULL);
	ent->nonNormalizedAxes = _axis;
	VectorCopy(axis[2], ent->axis[2]);
	VectorCopy(axis[1], ent->axis[1]);
	VectorCopy(axis[0], ent->axis[0]);
	Byte4Copy(color, ent->shaderRGBA);
	ent->customShader = realShader;
}
#endif

void CG_Q3F_Sentry( centity_t *cent )
{
	// Construct the sentry from the base up.

	refEntity_t base, turret, cannon, barrel, flash;
	vec3_t angles;
	float barrelspin, shadowplane;
	float shaderTime;

	if( cent->currentState.legsAnim == 99 ) {
		return;
	}

	CG_Q3F_BuildableSprites(cent);

//CaNaBiS
//CANATODO, This sentries shadow seems to screw up the decal queue?
//	CG_ShadowMark(cent->lerpOrigin,20,128,&shadowplane);
	shadowplane = 0;
	if( cg.time < cent->currentState.time2)	{
		// Spin up
		barrelspin = cent->currentState.time2 - cg.time;
		if( barrelspin > Q3F_SENTRY_SPINUP_TIME  )
			barrelspin = 1;
		else 
			barrelspin = (barrelspin / Q3F_SENTRY_SPINUP_TIME);
	} else if( cg.time > cent->currentState.time2 + Q3F_SENTRY_SPINKEEP_TIME) {
		// spin down or not spinning
		barrelspin = cg.time - cent->currentState.time - Q3F_SENTRY_SPINKEEP_TIME;
		if( barrelspin < Q3F_SENTRY_SPINDOWN_TIME )
			barrelspin =  ((Q3F_SENTRY_SPINDOWN_TIME - barrelspin) / Q3F_SENTRY_SPINDOWN_TIME);
		else 
			barrelspin = 0;
	} else {
		// Full spin
		barrelspin = 1;
	}

	cent->pe.barrelAngle += barrelspin * cg.frametime * 0.8;
	cent->pe.barrelAngle = AngleNormalize360( cent->pe.barrelAngle );

	memset( &base, 0, sizeof(base) );
	VectorCopy( cent->lerpOrigin, base.origin );
//	VectorCopy( cent->lerpOrigin, base.oldorigin );
	base.hModel = cgs.media.sentryBase;
	base.shadowPlane = shadowplane;

	VectorSet( angles, 0, 0, 0 );
	AnglesToAxis( angles, base.axis );			// Base never turns

	if( cent->currentState.legsAnim == 0 ) {
		base.customShader = cgs.media.sentryConstruct_Base;
		shaderTime = cent->currentState.time * 0.001f;
		base.shaderTime = shaderTime;
	} else {
		shaderTime = 0;
	}

#ifdef RIMLIGHTING_OUTLINE
	CG_BuildableWithOutline( &base, &cent->currentState );		// All version have the legs
#else
	trap_R_AddRefEntityToScene( &base, cent );		// All version have the legs
#endif

	memset( &turret, 0, sizeof(turret) );
	memset( &cannon, 0, sizeof(cannon) );
	memset( &barrel, 0, sizeof(barrel) );
	memset( &flash, 0, sizeof(flash) );

	// play upgrade sound - +1 offset to take care of vid_restarts resetting it to 0
	if( cent->currentState.legsAnim >= 1 && cent->currentState.legsAnim <= 3 ) {
		if( (cent->beamEnd[0] - 1) < cent->currentState.legsAnim ) {
			cent->beamEnd[0] = cent->currentState.legsAnim + 1;
			trap_S_StartSound( NULL, cent->currentState.number, CHAN_AUTO, cgs.media.sentryUpgradeSound );
		}
	} else {
		cent->beamEnd[0] = cent->currentState.legsAnim + 1;
	}

	if( cent->currentState.legsAnim == 1 || cent->currentState.legsAnim == 0 )
	{
		// Level one sentry

		turret.hModel = cgs.media.sentryTurret1;
		turret.shadowPlane = shadowplane;
		angles[YAW] = cent->lerpAngles[YAW];
		AnglesToAxis( angles, turret.axis );
		CG_PositionRotatedEntityOnTag( &turret, &base, "tag_turret" );
		if( cent->currentState.legsAnim == 0 ) {
			turret.customShader = cgs.media.sentryConstructShader_1;
			turret.shaderTime = shaderTime;
		}
		trap_R_AddRefEntityToScene( &turret, cent );

		cannon.shadowPlane = shadowplane;
		cannon.hModel = cgs.media.sentryCannon1;
		angles[YAW] = 0;	// It's attached to the already yaw'ed outer turret
		angles[PITCH] = cent->lerpAngles[PITCH];
		AnglesToAxis( angles, cannon.axis );
		CG_PositionRotatedEntityOnTag( &cannon, &turret, "tag_cannon" );
		if( cent->currentState.legsAnim == 0 ) {
			cannon.customShader = cgs.media.sentryConstructShader_1;
			cannon.shaderTime = shaderTime;
		}
		trap_R_AddRefEntityToScene( &cannon, cent );

		barrel.hModel = cgs.media.sentryBarrel;
		barrel.shadowPlane = shadowplane;
		angles[PITCH] = 0;
		if( cent->currentState.legsAnim == 0 ) {
			barrel.customShader = cgs.media.sentryConstructShader_2;
			barrel.shaderTime = shaderTime;
			angles[ROLL] = 0;
		} else
			angles[ROLL] = cent->pe.barrelAngle;
		AnglesToAxis( angles, barrel.axis );
		CG_PositionRotatedEntityOnTag( &barrel, &cannon, "tag_barrel" );
		trap_R_AddRefEntityToScene( &barrel, cent );

		if( !cent->currentState.legsAnim )
			return;		// Level 0 - building

		if( cent->currentState.weapon && (cent->currentState.frame & 1) && !(cent->miscTime & 1) && cent->currentState.otherEntityNum )
		{
			angles[ROLL] = Q_flrand(-1.0f, 1.0f) * 10;
			AnglesToAxis( angles, flash.axis );
			flash.hModel = cgs.media.sentryFlash;
			flash.renderfx = RF_NOCELSHADING;
			CG_PositionRotatedEntityOnTag( &flash, &barrel, "tag_flash");
			trap_R_AddRefEntityToScene( &flash, cent );
			trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, cgs.media.sentryFireSound );
			trap_R_AddLightToScene( flash.origin, 100, 1.f, 1.f, 1.f, 0.3f, 0, 0 );
		}
	}
	else if( cent->currentState.legsAnim == 2 )
	{
		// Level two sentry

		turret.hModel = cgs.media.sentryTurret2;
		turret.shadowPlane = shadowplane;
		angles[YAW] = cent->lerpAngles[YAW];
		AnglesToAxis( angles, turret.axis );
		CG_PositionRotatedEntityOnTag( &turret, &base, "tag_turret" );
		trap_R_AddRefEntityToScene( &turret, cent );

		cannon.hModel = cgs.media.sentryCannon2;
		cannon.shadowPlane = shadowplane;
		angles[YAW] = 0;	// It's attached to the already yaw'ed outer turret
		angles[PITCH] = cent->lerpAngles[PITCH];
		AnglesToAxis( angles, cannon.axis );
		CG_PositionRotatedEntityOnTag( &cannon, &turret, "tag_cannon" );
		trap_R_AddRefEntityToScene( &cannon, cent );

		barrel.hModel = cgs.media.sentryBarrel;
		barrel.shadowPlane = shadowplane;
		angles[PITCH] = 0;
		angles[ROLL] = cent->pe.barrelAngle;
		AnglesToAxis( angles, barrel.axis );
		CG_PositionRotatedEntityOnTag( &barrel, &cannon, "tag_barrel0" );
		trap_R_AddRefEntityToScene( &barrel, cent );

		if( cent->currentState.weapon && (cent->currentState.frame & 1) && !(cent->miscTime & 1) && cent->currentState.otherEntityNum )
		{
			angles[ROLL] = Q_flrand(-1.0f, 1.0f) * 10;
			AnglesToAxis( angles, flash.axis );
			flash.hModel = cgs.media.sentryFlash;
			flash.renderfx = RF_NOCELSHADING;
			CG_PositionRotatedEntityOnTag( &flash, &barrel, "tag_flash");
			trap_R_AddRefEntityToScene( &flash, cent );
			//trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, cgs.media.sentryFireSound[rand() & 3] );
			trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, cgs.media.sentryFireSound );
			trap_R_AddLightToScene( flash.origin, 100, 1.f, 1.f, 1.f, 0.3f, 0, 0 );
		}

		angles[ROLL] = 180 - cent->pe.barrelAngle;
		AnglesToAxis( angles, barrel.axis );
		CG_PositionRotatedEntityOnTag( &barrel, &cannon, "tag_barrel1" );
		trap_R_AddRefEntityToScene( &barrel, cent );

		if( cent->currentState.weapon && !(cent->currentState.frame & 1) && (cent->miscTime & 1) && cent->currentState.otherEntityNum  )
		{
			angles[ROLL] = Q_flrand(-1.0f, 1.0f) * 10;
			AnglesToAxis( angles, flash.axis );
			flash.hModel = cgs.media.sentryFlash;
			flash.renderfx = RF_NOCELSHADING;
			CG_PositionRotatedEntityOnTag( &flash, &barrel, "tag_flash");
			trap_R_AddRefEntityToScene( &flash, cent );
			trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, cgs.media.sentryFireSound );
			trap_R_AddLightToScene( flash.origin, 100, 1.f, 1.f, 1.f, 0.3f, 0, 0 );
		}
	}
	else if( cent->currentState.legsAnim >= 3 )
	{
		// Level three sentry

		turret.hModel = cgs.media.sentryTurret3;
		angles[YAW] = cent->lerpAngles[YAW];
		AnglesToAxis( angles, turret.axis );
		CG_PositionRotatedEntityOnTag( &turret, &base, "tag_turret" );
		trap_R_AddRefEntityToScene( &turret, cent );

		cannon.hModel = cgs.media.sentryCannon3;
		angles[YAW] = 0;	// It's attached to the already yaw'ed outer turret
		angles[PITCH] = cent->lerpAngles[PITCH];
		AnglesToAxis( angles, cannon.axis );
		CG_PositionRotatedEntityOnTag( &cannon, &turret, "tag_cannon" );
		trap_R_AddRefEntityToScene( &cannon, cent );

		barrel.hModel = cgs.media.sentryBarrel;
		angles[PITCH] = 0;
		angles[ROLL] = cent->pe.barrelAngle;
		AnglesToAxis( angles, barrel.axis );
		CG_PositionRotatedEntityOnTag( &barrel, &cannon, "tag_barrel0" );
		trap_R_AddRefEntityToScene( &barrel, cent );

		if( cent->currentState.weapon && (cent->currentState.frame & 1) && !(cent->miscTime & 1) && cent->currentState.otherEntityNum )
		{
			angles[ROLL] = Q_flrand(-1.0f, 1.0f) * 10;
			AnglesToAxis( angles, flash.axis );
			flash.hModel = cgs.media.sentryFlash;
			flash.renderfx = RF_NOCELSHADING;
			CG_PositionRotatedEntityOnTag( &flash, &barrel, "tag_flash");
			trap_R_AddRefEntityToScene( &flash, cent );
			//trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, cgs.media.sentryFireSound[rand() & 3] );
			trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, cgs.media.sentryFireSound );
			trap_R_AddLightToScene( flash.origin, 100, 1.f, 1.f, 1.f, 0.3f, 0, 0 );
		}

		angles[ROLL] = 180 - cent->pe.barrelAngle;
		AnglesToAxis( angles, barrel.axis );
		CG_PositionRotatedEntityOnTag( &barrel, &cannon, "tag_barrel1" );
		trap_R_AddRefEntityToScene( &barrel, cent );

		if( cent->currentState.weapon && !(cent->currentState.frame & 1) && (cent->miscTime & 1) && cent->currentState.otherEntityNum )
		{
			angles[ROLL] = Q_flrand(-1.0f, 1.0f) * 10;
			AnglesToAxis( angles, flash.axis );
			flash.hModel = cgs.media.sentryFlash;
			flash.renderfx = RF_NOCELSHADING;
			CG_PositionRotatedEntityOnTag( &flash, &barrel, "tag_flash");
			trap_R_AddRefEntityToScene( &flash, cent );
			//trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, cgs.media.sentryFireSound[rand() & 3] );
			trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, cgs.media.sentryFireSound );
			trap_R_AddLightToScene( flash.origin, 100, 1.f, 1.f, 1.f, 0.3f, 0, 0 );
		}

		barrel.hModel = cgs.media.sentryRocketLauncher;
		memcpy( barrel.axis, base.axis, sizeof(base.axis) );
		CG_PositionRotatedEntityOnTag( &barrel, &cannon, "tag_rl0" );
		trap_R_AddRefEntityToScene( &barrel, cent );

		memcpy( barrel.axis, base.axis, sizeof(base.axis) );
		CG_PositionRotatedEntityOnTag( &barrel, &cannon, "tag_rl1" );
		trap_R_AddRefEntityToScene( &barrel, cent );
	}

	cent->miscTime = cent->currentState.frame;

	if ( cg_drawBBox.integer ) 
		CG_AddBoundingBox( cent , colorMagenta );

}
static vec4_t smokecolor = { 1, 1, 1, 0.4 };


static void LaunchBit( const vec3_t origin, qhandle_t model ) {
	vec3_t velocity;
	int temp;
	localEntity_t *le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity( 8000 );
	re = &le->refEntity;
	le->leType = LE_FRAGMENT;

	VectorCopy( origin, re->origin );

	re->hModel = model;

	le->bounceFactor = 0.4f;

	le->pos.trType = TR_GRAVITY;
	le->pos.trTime = cg.time;

	VectorCopy( origin, le->pos.trBase );

	velocity[0] = Q_flrand(-1.0f, 1.0f);
	velocity[1] = Q_flrand(-1.0f, 1.0f);
	velocity[2] = 0.8+Q_flrand(0.0f, 1.0f);

	VectorNormalizeFast( velocity );
	
	temp = 500 + (rand()&127);
	VectorScale( velocity, temp, le->pos.trDelta );

	//Angles
	le->angles.trType = TR_LINEAR;
	le->angles.trTime = cg.time;
	for (temp=0;temp<3;temp++) {
		le->angles.trBase[temp] = rand() % 360;
		le->angles.trDelta[temp] = ((rand() & 2) - 1) * (360 + rand() % 360);
	}	
	AxisCopy( axisDefault, re->axis );
	le->leFlags = LEF_TUMBLE | LEF_SOUND_BRASS;
}

void CG_Q3F_Sentry_Explode( centity_t *cent ) {
	int i, temp;
	localEntity_t *le;
	vec3_t velocity, origin;

	VectorCopy( cent->lerpOrigin, origin );
	origin[2] += 24;
	Spirit_RunScript( cgs.spirit.explosion_normal, origin, origin, axisDefault, 0 );

	for (i=0;i<5;i++) {
		temp = rand() % NUMVERTEXNORMALS;
		VectorCopy( bytedirs[temp], velocity );
		velocity[2]+=0.5;
		temp = 20 + (rand() & 15);
		VectorScale( velocity, temp, velocity );
		le = CG_SmokePuff( origin, velocity, 800, 200, 16, 24, smokecolor, cgs.media.smokePuffShader );
		temp = rand() & 511;
		le->startTime += temp;
		le->endTime += temp;
	}
	//Main parts
	LaunchBit( origin, cgs.media.sentryBits[0]);
	LaunchBit( origin, cgs.media.sentryBits[1]);
	LaunchBit( origin, cgs.media.sentryBits[2]);
	//3 legs
	LaunchBit( origin, cgs.media.sentryBits[3]);
	LaunchBit( origin, cgs.media.sentryBits[3]);
	LaunchBit( origin, cgs.media.sentryBits[3]);

	LaunchBit( origin, cgs.media.sentryBarrel);
}

void CG_Q3F_Supplystation_Explode( centity_t *cent ) {
	vec3_t origin;

	VectorCopy( cent->lerpOrigin, origin );
	origin[2] += 24;
	Spirit_RunScript( cgs.spirit.explosion_normal, origin, origin, axisDefault, 0 );

	LaunchBit( origin, cgs.media.supplystationBits[0]);
	LaunchBit( origin, cgs.media.supplystationBits[0]);
	LaunchBit( origin, cgs.media.supplystationBits[0]);
	LaunchBit( origin, cgs.media.supplystationBits[1]);
	LaunchBit( origin, cgs.media.supplystationBits[2]);
}


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Supplystation

#define Q3F_SUPPLYSTATION_SHELLS		400	
#define Q3F_SUPPLYSTATION_NAILS			600
#define Q3F_SUPPLYSTATION_ROCKETS		300
#define Q3F_SUPPLYSTATION_CELLS			400
#define Q3F_SUPPLYSTATION_ARMOUR		500

#define Q3F_SUPPLYSTATION_MAXHEALTH CG_Q3F_SupplyStationMaxHealth(cent->currentState.legsAnim)

static int CG_Q3F_SupplystationPanel()
{
	centity_t *cent = (centity_t *) panel.data;
	int health, shells, nails, rockets, cells, armour, grenades/*, numLines*/;
	//float charSize;
	int level;

	CG_Q3F_PanelPrepareCoords( PANEL_LAYER_MAIN );

//	CG_Q3F_PanelDrawString(	cgs.clientinfo[cg.clientNum].name, 0, y, 80, 0, 0,
//							PANEL_STR_COLOUR|PANEL_STR_RIGHT|PANEL_STR_WRAP|PANEL_STR_SPACEBREAK, panel.transrgba );
//	CG_Q3F_PanelDrawString(	"^1Peter ^2Piper ^3picked ^4a ^5peck ^6of ^7pickled ^1peppers, ^2so ^3where's ^4the ^5peck ^6of ^7pickled ^1peppers ^2Peter ^3Piper ^4picked?",
//							0, 0, 20, 0, 0, PANEL_STR_COLOUR|PANEL_STR_CENTER|PANEL_STR_WRAP|PANEL_STR_SPACEBREAK, panel.transrgba );

	switch( cg.usingSupplystation )
	{
	// Ready for use
	case 0: //CG_Q3F_PanelFitString( &numLines, &charSize, "Supply Station.", 0, 640, 480 );
			//CG_Q3F_PanelDrawString( "Supply Station.", 0,4*charSize, charSize, 0, 0, PANEL_STR_CENTER, panel.transrgba );
			//CG_Q3F_PanelDrawString( "Ready For Use", 0,5*charSize, charSize, 0, 0, PANEL_STR_CENTER, panel.transrgba );
			CG_Q3F_PanelDrawString( "Supply Station", 0,5*24, 24, 0, 0, PANEL_STR_CENTER, panel.transrgba );
			CG_Q3F_PanelDrawString( "Ready For Use", 0,6*24, 24, 0, 0, PANEL_STR_CENTER, panel.transrgba );
			break;
	// Using
	case 1:	/*CG_Q3F_PanelFitString( &numLines, &charSize, "Use Supply Station", 0, 640, 480 );
			CG_Q3F_PanelDrawString( "Use Supply Station", 0, 0, charSize, 0, 0, PANEL_STR_LEFT, panel.transrgba );

			shells = cent->currentState.origin2[0];
			nails = cent->currentState.origin2[1];
			rockets = cent->currentState.origin2[2];
			cells = cent->currentState.angles2[0];
			armour = cent->currentState.angles2[1];

			CG_Q3F_PanelDrawString( va( "Shells:  %d", shells ), 0, 2*charSize, charSize, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Nails:   %d", nails ), 0, 3*charSize, charSize, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Rockets: %d", rockets ), 0, 4*charSize, charSize, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Cells:   %d", cells ), 0, 5*charSize, charSize, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Armour:  %d", armour ), 0, 6*charSize, charSize, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( "^31.^7 Supply Ammo", 0, 8*charSize, charSize, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( "^32.^7 Supply Armor", 0, 9*charSize, charSize, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );*/

			CG_Q3F_PanelDrawString( "Use Supply Station", 0, 0, 24, 0, 0, PANEL_STR_LEFT, panel.transrgba );

			level = cent->currentState.legsAnim;

			shells = cent->currentState.origin2[0];
			nails = cent->currentState.origin2[1];
			rockets = cent->currentState.origin2[2];
			cells = cent->currentState.angles2[0];
			armour = cent->currentState.angles2[1];
			grenades = cent->currentState.angles2[2];

			CG_Q3F_PanelDrawString( va( "Shells:   %03d/%d", shells, Q3F_SUPPLYSTATION_SHELLS ), 0, 2*24, 24, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Nails:    %03d/%d", nails, Q3F_SUPPLYSTATION_NAILS ), 0, 3*24, 24, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Rockets:  %03d/%d", rockets, Q3F_SUPPLYSTATION_ROCKETS ), 0, 4*24, 24, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Cells:    %03d/%d", cells, Q3F_SUPPLYSTATION_CELLS ), 0, 5*24, 24, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Armour:   %03d/%d", armour, Q3F_SUPPLYSTATION_ARMOUR ), 0, 6*24, 24, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			if(level == 3)
			{
				CG_Q3F_PanelDrawString( va( "^7Grenades: %03d/%03d^3  NEW!", grenades, 2 ), 0, 7*24, 24, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );
				CG_Q3F_PanelDrawString( "^31^7 Supply Ammo", 0, 9*24, 24, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );
				CG_Q3F_PanelDrawString( "^32^7 Supply Armor", 0, 10*24, 24, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );
				CG_Q3F_PanelDrawString( "^33^7 Supply Grenade", 0, 11*24, 24, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );
			}
			else
			{
				CG_Q3F_PanelDrawString( "^31^7 Supply Ammo", 0, 8*24, 24, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );
				CG_Q3F_PanelDrawString( "^32^7 Supply Armor", 0, 9*24, 24, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );
			}
			break;
	// End use
	case 2: //CG_Q3F_PanelFitString( &numLines, &charSize, "this Supply Station.", 0, 640, 480 );
			//CG_Q3F_PanelDrawString( "Thank you for using this Supply Station.", 0,4*charSize, charSize, 0, 0, PANEL_STR_CENTER|PANEL_STR_WRAP|PANEL_STR_SPACEBREAK, panel.transrgba );
			//CG_Q3F_PanelDrawString( "Have a nice day!", 0,7*charSize, charSize, 0, 0, PANEL_STR_CENTER, panel.transrgba );
			CG_Q3F_PanelDrawString( "Thank you for using", 0,4*24, 24, 0, 0, PANEL_STR_CENTER, panel.transrgba );
			CG_Q3F_PanelDrawString( "this Supply Station.", 0,5*24, 24, 0, 0, PANEL_STR_CENTER, panel.transrgba );
			CG_Q3F_PanelDrawString( "Have a nice day!", 0,7*24, 24, 0, 0, PANEL_STR_CENTER, panel.transrgba );
			
			break;
	// Engineer 'upgrading'
	case 3:	/*CG_Q3F_PanelFitString( &numLines, &charSize, "^33.^7 Dismantle Supply Station", 0, 640, 480 );
			CG_Q3F_PanelDrawString( "Upgrade Supply Station", 0, 0, 24, 0, 0, PANEL_STR_LEFT, panel.transrgba );

			health = cent->currentState.otherEntityNum;
			shells = cent->currentState.origin2[0];
			nails = cent->currentState.origin2[1];
			rockets = cent->currentState.origin2[2];
			cells = cent->currentState.angles2[0];
			armour = cent->currentState.angles2[1];

			CG_Q3F_PanelDrawString( va( "Health:  %d", health ), 0, 2*charSize, charSize, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Shells:  %d", shells ), 0, 4*charSize, charSize, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Nails:   %d", nails ), 0, 5*charSize, charSize, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Rockets: %d", rockets ), 0, 6*charSize, charSize, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Cells:   %d", cells ), 0, 7*charSize, charSize, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Armour:  %d", armour ), 0, 8*charSize, charSize, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( "^31.^7 Repair Supply Station", 0, 10*charSize, charSize, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( "^32.^7 Refill Supply Station", 0, 11*charSize, charSize, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( "^33.^7 Dismantle Supply Station", 0, 12*charSize, charSize, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );*/
//			CG_Q3F_PanelDrawString( "Upgrade Supply Station", 0, 0, 24, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( "Maintain Supply Station", 0, 0, 24, 0, 0, PANEL_STR_LEFT, panel.transrgba );

			health = cent->currentState.otherEntityNum;
			level = cent->currentState.legsAnim;
			shells = cent->currentState.origin2[0];
			nails = cent->currentState.origin2[1];
			rockets = cent->currentState.origin2[2];
			cells = cent->currentState.angles2[0];
			armour = cent->currentState.angles2[1];
			grenades = cent->currentState.angles2[2];

			CG_Q3F_PanelDrawString( va( "Health:   %03d/%d", health, Q3F_SUPPLYSTATION_MAXHEALTH ), 0, 2*24, 24, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Shells:   %03d/%d", shells, Q3F_SUPPLYSTATION_SHELLS ), 0, 4*24, 24, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Nails:    %03d/%d", nails, Q3F_SUPPLYSTATION_NAILS ), 0, 5*24, 24, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Rockets:  %03d/%d", rockets, Q3F_SUPPLYSTATION_ROCKETS ), 0, 6*24, 24, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Cells:    %03d/%d", cells, Q3F_SUPPLYSTATION_CELLS ), 0, 7*24, 24, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			CG_Q3F_PanelDrawString( va( "Armour:   %03d/%d", armour, Q3F_SUPPLYSTATION_ARMOUR ), 0, 8*24, 24, 0, 0, PANEL_STR_LEFT, panel.transrgba );
			if(level == 3)
			{
				CG_Q3F_PanelDrawString( va( "^7Grenades: %03d/%03d^3  NEW!", grenades, 2 ), 0, 9*24, 24, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );
				CG_Q3F_PanelDrawString( "^31^7 Dismantle Supply Station", 0, 13*24, 24, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );
			}
			else
			{
//				CG_Q3F_PanelDrawString( "^31.^7 Repair Supply Station", 0, 10*24, 24, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );
//				CG_Q3F_PanelDrawString( "^32.^7 Refill Supply Station", 0, 11*24, 24, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );
				CG_Q3F_PanelDrawString( "^31^7 Dismantle Supply Station", 0, 12*24, 24, 0, 0, PANEL_STR_COLOUR|PANEL_STR_LEFT, panel.transrgba );
			}
			break;
	}

	return( 0 );

}

void CG_Q3F_Supplystation( centity_t *cent ) {
	refEntity_t base, hud;
	float shadowplane;
	int drawshadow;
	orientation_t tag;
	int i;
	vec3_t panelorigin;
	float shaderTime;

	if( cent->currentState.legsAnim == 99 ) {
		return;
	}

	CG_Q3F_BuildableSprites(cent);

	drawshadow = CG_ShadowMark( cent->lerpOrigin, 9, 128, &shadowplane);

	memset( &base, 0, sizeof(base) );
	VectorCopy( cent->lerpOrigin, base.origin);
	base.origin[2] -= 2;
	VectorCopy( cent->lerpOrigin, base.oldorigin);
	base.hModel = cgs.media.supplystationBase;
	base.shadowPlane = shadowplane;

	if( cent->currentState.legsAnim == 0 ) {
		// we're building
		base.customShader = cgs.media.supplystationConstruct_Base;
		shaderTime = cent->currentState.time * 0.001f;
		base.shaderTime = shaderTime;
	} else {
		shaderTime = 0;
	}

	// play upgrade sound - +1 offset to take care of vid_restarts resetting it to 0
	if( cent->currentState.legsAnim > 1 && cent->currentState.legsAnim <= 3 ) {
		if( (cent->beamEnd[0] - 1) < cent->currentState.legsAnim ) {
			cent->beamEnd[0] = cent->currentState.legsAnim + 1;
			trap_S_StartSound( NULL, cent->currentState.number, CHAN_AUTO, cgs.media.supplyBuildSound );
		}
	} else {
		cent->beamEnd[0] = cent->currentState.legsAnim + 1;
	}

	// convert angles to axis
	AnglesToAxis( cent->lerpAngles, base.axis );
	trap_R_AddRefEntityToScene( &base, cent );

	memset( &hud, 0, sizeof(hud) );
	VectorCopy( cent->lerpOrigin, hud.origin);
	VectorCopy( cent->lerpOrigin, hud.oldorigin);
	hud.hModel = cgs.media.supplystationHUD;
	hud.shadowPlane = shadowplane;

	CG_PositionEntityOnTag( &hud, &base, "tag_hud", 0, NULL );

	if( !cent->currentState.legsAnim ) {
		// we're building
		hud.customShader = cgs.media.supplystationConstruct_Screen;
		hud.shaderTime = shaderTime;

		hud.origin[2] -= 18;
		hud.oldorigin[2] -= 18;

		trap_R_AddRefEntityToScene( &hud, cent );

		cent->dustTrailTime = 0;

		return;
	}

	// retract/popup bits
	//
	// miscTime holds the time the animation starts. If the state opening/closing
	// changes while animating place miscTime backwards so it looks like animation started earlier
	//
	// dustTrailTime holds the status of the last frame

	// start popup or retract
	if( ( cent->currentState.frame && !cent->dustTrailTime ) || ( !cent->currentState.frame && cent->dustTrailTime ) ) {
		if( cg.time - cent->miscTime < 250.f ) {
			cent->miscTime = cg.time - ( cg.time - cent->miscTime );
		} else
			cent->miscTime = cg.time;

		if( cent->dustTrailTime )
			trap_S_StartSound( NULL, cent->currentState.number, CHAN_AUTO, cgs.media.supplyPopup );
		else
			trap_S_StartSound( NULL, cent->currentState.number, CHAN_AUTO, cgs.media.supplyRetract );
	}

	if( cent->currentState.frame ) {
		if( !cg.rendering2ndRefDef && !cg.renderingSkyPortal ) {
			if( cg.time - cent->miscTime < 250.f )
				hud.origin[2] = ( hud.origin[2] - 18 ) + ( ( cg.time - cent->miscTime ) / 250.f ) * 18;
			else
				hud.origin[2] = hud.origin[2];

			// draw the panel
			//trap_R_LerpTag( &tag, cgs.media.supplystationHUD, 0, 0, 1, "tag_display" );
			CG_GetTagFromModel( &tag, cgs.media.supplystationHUD, "tag_display" );

			VectorCopy( hud.origin, panelorigin );
			for ( i = 0 ; i < 3 ; i++ ) {
				VectorMA( panelorigin, tag.origin[i], hud.axis[i], panelorigin );
			}

			if( CG_Q3F_RenderPanel( 0,
									0,
									1,
									panelorigin,
									cent->lerpAngles,
									14,
									14,
									80,
									&CG_Q3F_SupplystationPanel,
									cent,
									cent->trailTime,
									cent->teleportFlag,
									qtrue,
									qtrue ) )
				cent->trailTime = cg.time;
			else
				cent->teleportFlag = cg.time;
		}
	} else {
		if( !cg.rendering2ndRefDef && !cg.renderingSkyPortal ) {
			if( cg.time - cent->miscTime < 250.f )
				hud.origin[2] = hud.origin[2] - ( ( cg.time - cent->miscTime ) / 250.f ) * 18;
			else
				hud.origin[2] = hud.origin[2] - 18;

			// draw the panel
			//trap_R_LerpTag( &tag, cgs.media.supplystationHUD, 0, 0, 1, "tag_display" );
			CG_GetTagFromModel( &tag, cgs.media.supplystationHUD, "tag_display" );

			VectorCopy( hud.origin, panelorigin );
			for ( i = 0 ; i < 3 ; i++ ) {
				VectorMA( panelorigin, tag.origin[i], hud.axis[i], panelorigin );
			}

			CG_Q3F_RenderPanel( 0,
								0,
								1,
								panelorigin,
								cent->lerpAngles,
								14,
								14,
								80,
								&CG_Q3F_SupplystationPanel,
								cent,
								cent->trailTime,
								cent->teleportFlag,
								qtrue,
								qtrue );
			cent->teleportFlag = cg.time;
		}			
	}

	cent->dustTrailTime = cent->currentState.frame;

	VectorCopy( hud.origin, hud.oldorigin );

	trap_R_AddRefEntityToScene( &hud, cent );

	// draw the panel
	/*trap_R_LerpTag( &tag, cgs.media.supplystationHUD, 0, 0, 1, "tag_display" );

	VectorCopy( hud.origin, panelorigin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( panelorigin, tag.origin[i], hud.axis[i], panelorigin );
	}*/

	/*if( CG_Q3F_RenderPanel( 0,
							0,
							1,
							panelorigin,
							cent->lerpAngles,
							14,
							14,
							80,
							&CG_Q3F_SupplystationPanel,
							cent,
							cent->trailTime,
							cent->teleportFlag,
							qtrue ) )
		cent->trailTime = cg.time;
	else
		cent->teleportFlag = cg.time;*/

}
