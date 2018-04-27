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

// ui_players.c

#include "ui_local.h"

#define UI_TIMER_GESTURE		2300
#define UI_TIMER_JUMP			1000
#define UI_TIMER_LAND			130
#define UI_TIMER_WEAPON_SWITCH	300
#define UI_TIMER_ATTACK			500
#define	UI_TIMER_MUZZLE_FLASH	20
#define	UI_TIMER_WEAPON_DELAY	250

#define JUMP_HEIGHT				56

#define SWINGSPEED				0.3f

#define SPIN_SPEED				0.9f
#define COAST_TIME				1000


static int			dp_realtime;
static float		jumpHeight;
sfxHandle_t weaponChangeSound;


F2RDef_t *F2R_GetForModel( qhandle_t model );

/*
===============
UI_PlayerInfo_SetWeapon
===============
*/
static void UI_PlayerInfo_SetWeapon( playerInfo_t *pi, weapon_t weaponNum ) {
	gitem_t *	item;
	char		path[MAX_QPATH];

	pi->currentWeapon = weaponNum;
tryagain:
	pi->realWeapon = weaponNum;
	pi->weaponModel = 0;
	pi->barrelModel = 0;
	pi->flashModel = 0;

	if ( weaponNum == WP_NONE ) {
		return;
	}

	for ( item = bg_itemlist + 1; item->classname ; item++ ) {
		if ( item->giType != IT_WEAPON ) {
			continue;
		}
		if ( item->giTag == (int)weaponNum ) {
			break;
		}
	}

	if ( item->classname ) {
		pi->weaponModel = trap_R_RegisterModel( item->world_model[0] );
	}

	if( pi->weaponModel == 0 ) {
		if( weaponNum == WP_NAILGUN ) {
			weaponNum = WP_NONE;
			goto tryagain;
		}
		weaponNum = WP_NAILGUN;
		goto tryagain;
	}

	if ( weaponNum == WP_NAILGUN || weaponNum == WP_AXE /* JT || weaponNum == WP_BFG  */) {
		strcpy( path, item->world_model[0] );
		COM_StripExtension( path, path, sizeof(path) );
		strcat( path, "_barrel.md3" );
		pi->barrelModel = trap_R_RegisterModel( path );
	}

	strcpy( path, item->world_model[0] );
	COM_StripExtension( path, path, sizeof(path) );
	strcat( path, "_flash.md3" );
	pi->flashModel = trap_R_RegisterModel( path );

	switch( weaponNum ) {
	case WP_AXE:
		MAKERGB( pi->flashDlightColor, 0.6f, 0.6f, 1 );
		break;

	case WP_NAILGUN:
		MAKERGB( pi->flashDlightColor, 1, 1, 0 );
		break;

	case WP_SHOTGUN:
		MAKERGB( pi->flashDlightColor, 1, 1, 0 );
		break;

	case WP_GRENADE_LAUNCHER:
		MAKERGB( pi->flashDlightColor, 1, 0.7f, 0.5f );
		break;

	case WP_ROCKET_LAUNCHER:
		MAKERGB( pi->flashDlightColor, 1, 0.75f, 0 );
		break;
/* JT
	case WP_LIGHTNING:
		MAKERGB( pi->flashDlightColor, 0.6, 0.6, 1 );
		break;
	JT */
	case WP_RAILGUN:
		MAKERGB( pi->flashDlightColor, 1, 0.5f, 0 );
		break;
/* JT
	case WP_PLASMAGUN:
		MAKERGB( pi->flashDlightColor, 0.6, 0.6, 1 );
		break;
	
	case WP_BFG:
		MAKERGB( pi->flashDlightColor, 1, 0.7, 1 );
		break;
	
	case WP_GRAPPLING_HOOK:
		MAKERGB( pi->flashDlightColor, 0.6, 0.6, 1 );
		break;
	JT */
	default:
		MAKERGB( pi->flashDlightColor, 1, 1, 1 );
		break;
	}
}

/*
======================
UI_PositionEntityOnTag
======================
*/
static void UI_PositionEntityOnTag( refEntity_t *entity, const refEntity_t *parent, char *tagName ) {
	int				i;
	orientation_t	lerped;
	
	// lerp the tag
	trap_CM_LerpTag( &lerped, (const refEntity_t *)parent, (const char *)tagName, 0 );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// cast away const because of compiler problems
	MatrixMultiply( lerped.axis, ((refEntity_t*)parent)->axis, entity->axis );
	entity->backlerp = parent->backlerp;
}


/*
======================
UI_PositionRotatedEntityOnTag
======================
*/
static void UI_PositionRotatedEntityOnTag( refEntity_t *entity, const refEntity_t *parent, char *tagName ) {
	int				i;
	orientation_t	lerped;
	matrix3_t		tempAxis;

	// lerp the tag
	trap_CM_LerpTag( &lerped, (const refEntity_t *)parent, (const char *)tagName, 0 );

	// FIXME: allow origin offsets along tag?
	VectorCopy( parent->origin, entity->origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( entity->origin, lerped.origin[i], parent->axis[i], entity->origin );
	}

	// cast away const because of compiler problems
	MatrixMultiply( entity->axis, ((refEntity_t *)parent)->axis, tempAxis );
	MatrixMultiply( lerped.axis, tempAxis, entity->axis );
}

/*
==================
UI_SwingAngles
==================
*/
static void UI_SwingAngles( float destination, float swingTolerance, float clampTolerance,
					float speed, float *angle, qboolean *swinging ) {
	float	swing;
	float	move;
	float	scale;

	if ( !*swinging ) {
		// see if a swing should be started
		swing = AngleSubtract( *angle, destination );
		if ( swing > swingTolerance || swing < -swingTolerance ) {
			*swinging = qtrue;
		}
	}

	if ( !*swinging ) {
		return;
	}
	
	// modify the speed depending on the delta
	// so it doesn't seem so linear
	swing = AngleSubtract( destination, *angle );
	scale = fabs( swing );
	if ( scale < swingTolerance * 0.5 ) {
		scale = 0.5;
	} else if ( scale < swingTolerance ) {
		scale = 1.0;
	} else {
		scale = 2.0;
	}

	// swing towards the destination angle
	if ( swing >= 0 ) {
		move = uiInfo.uiDC.frameTime * scale * speed;
		if ( move >= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod( *angle + move );
	} else if ( swing < 0 ) {
		move = uiInfo.uiDC.frameTime * scale * -speed;
		if ( move <= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod( *angle + move );
	}

	// clamp to no more than tolerance
	swing = AngleSubtract( destination, *angle );
	if ( swing > clampTolerance ) {
		*angle = AngleMod( destination - (clampTolerance - 1) );
	} else if ( swing < -clampTolerance ) {
		*angle = AngleMod( destination + (clampTolerance - 1) );
	}
}

/*
=============================================================================

CLIENT INFO

=============================================================================
*/

qhandle_t *UI_Q3F_LegsModel( int classNum ) {
	return( &uiInfo.modelcache[classNum][0] );
}

qhandle_t *UI_Q3F_TorsoModel( int classNum ) {
	return( &uiInfo.modelcache[classNum][1] );
}

qhandle_t *UI_Q3F_HeadModel( int classNum ) {
	return( &uiInfo.modelcache[classNum][2] );
}

qhandle_t *UI_Q3F_LegsSkin( int classNum ) {
	return( &uiInfo.skincache[classNum][0] );
}

qhandle_t *UI_Q3F_TorsoSkin( int classNum ) {
	return( &uiInfo.skincache[classNum][1] );
}

qhandle_t *UI_Q3F_HeadSkin( int classNum ) {
	return( &uiInfo.skincache[classNum][2] );
}

qhandle_t *UI_Q3F_ModelIcon( int classNum ) {
	return( &uiInfo.modeliconcache[classNum] );
}

F2RDef_t *UI_Q3F_LegsF2RScript( int classNum ) {
	return( uiInfo.f2rcache[classNum][0] );
}

F2RDef_t *UI_Q3F_TorsoF2RScript( int classNum ) {
	return( uiInfo.f2rcache[classNum][1] );
}

F2RDef_t *UI_Q3F_HeadF2RScript( int classNum ) {
	return( uiInfo.f2rcache[classNum][2] );
}

byte *UI_Q3F_LegsColour( int classNum, q3f_team_t teamNum ) {
	return( &uiInfo.skincolours[classNum][teamNum-Q3F_TEAM_RED][0][0] );
}

byte *UI_Q3F_TorsoColour( int classNum, q3f_team_t teamNum ) {
	return( &uiInfo.skincolours[classNum][teamNum-Q3F_TEAM_RED][1][0] );
}

byte *UI_Q3F_HeadColour( int classNum, q3f_team_t teamNum ) {
	return( &uiInfo.skincolours[classNum][teamNum-Q3F_TEAM_RED][2][0] );
}

/*
==========================
UI_Q3F_RegisterPlayerClass
==========================
*/
qboolean UI_Q3F_RegisterClassModels( int classNum ) {
	qboolean				noErrors = qtrue;
	bg_q3f_playerclass_t	*cls;
	char					filename[MAX_QPATH];
	fileHandle_t			skinColourHandle;
	int						lores;

	cls = bg_q3f_classlist[classNum];

	//
	// Load models - F2R automatically loads the animation data
	//

	// Load legs
	Com_sprintf( filename, sizeof( filename ), "models/classes/%s/lower.md3", cls->commandstring );
	uiInfo.modelcache[classNum][0] = trap_R_RegisterModel( filename );
	if( !uiInfo.modelcache[classNum][0] ) {
		Com_Printf( "^3Leg model load failure: %s\n", filename );
		noErrors = qfalse;
	} else {
		// Find F2RDef_t belonging to this model
		uiInfo.f2rcache[classNum][0] = F2R_GetForModel( uiInfo.modelcache[classNum][0] );
		if( !uiInfo.f2rcache[classNum][0] ) {
			COM_StripExtension( filename, filename, sizeof(filename) );
			Q_strcat( filename, sizeof(filename), ".f2r" );
			Com_Printf( "^3Leg model F2R load failure: %s\n", filename );
			noErrors = qfalse;
		}
	}

	// Load torso
	Com_sprintf( filename, sizeof( filename ), "models/classes/%s/upper.md3", cls->commandstring );
	uiInfo.modelcache[classNum][1] = trap_R_RegisterModel( filename );
	if( !uiInfo.modelcache[classNum][1] ) {
		Com_Printf( "^3Torso model load failure: %s\n", filename );
		noErrors = qfalse;
	} else {
		// Find F2RDef_t belonging to this model
		uiInfo.f2rcache[classNum][1] = F2R_GetForModel( uiInfo.modelcache[classNum][1] );
		if( !uiInfo.f2rcache[classNum][1] ) {
			COM_StripExtension( filename, filename, sizeof(filename) );
			Q_strcat( filename, sizeof(filename), ".f2r" );
			Com_Printf( "^3Torso model F2R load failure: %s\n", filename );
			noErrors = qfalse;
		}
	}

	// Load head
	Com_sprintf( filename, sizeof( filename ), "models/classes/%s/head.md3", cls->commandstring );
	uiInfo.modelcache[classNum][2] = trap_R_RegisterModel( filename );
	if( !uiInfo.modelcache[classNum][2] ) {
		Com_Printf( "^3Head model load failure: %s\n", filename );
		noErrors = qfalse;
	}

	//
	// Load skins
	//

	// start hack of the year!
	if( r_vertexLight.integer )
		trap_Cvar_Set( "r_vertexlight", "0" );

	lores = trap_Cvar_VariableValue("r_loresskins");

	// Load legs skin
	//Com_sprintf( filename, sizeof( filename ), "models/classes/%s/lower%s.skin", cls->commandstring, ( r_vertexLight.integer ? "_vertex" : "" ) );
	Com_sprintf( filename, sizeof( filename ), lores ? "models/classes/%s/lower_lores.skin" : "models/classes/%s/lower.skin", cls->commandstring );
	uiInfo.skincache[classNum][0] = trap_R_RegisterSkin( filename );
	if( !uiInfo.skincache[classNum][0] ) {
		Com_Printf( "^3Leg skin load failure: %s\n", filename );
		noErrors = qfalse;
	}

	// Load torso skin
	//Com_sprintf( filename, sizeof( filename ), "models/classes/%s/upper%s.skin", cls->commandstring, ( r_vertexLight.integer ? "_vertex" : "" ) );
	Com_sprintf( filename, sizeof( filename ), lores ? "models/classes/%s/upper_lores.skin" : "models/classes/%s/upper.skin", cls->commandstring );
	uiInfo.skincache[classNum][1] = trap_R_RegisterSkin( filename );
	if( !uiInfo.skincache[classNum][1] ) {
		Com_Printf( "^3Torso skin load failure: %s\n", filename );
		noErrors = qfalse;
	}

	// Load head skin
	/*Com_sprintf( filename, sizeof( filename ), "models/classes/%s/head%s.skin", cls->commandstring, ( r_vertexLight.integer ? "_vertex" : "" ) );
	uiInfo.skincache[classNum][2] = trap_R_RegisterSkin( filename );
	if( !uiInfo.skincache[classNum][2] ) {
		Com_Printf( "^3Head skin load failure: %s\n", filename );
		noErrors = qfalse;
	}*/

	// exit hack of the year!
	if( r_vertexLight.integer )
		trap_Cvar_Set( "r_vertexlight", "1" );

	//
	// Load icon
	//

	Com_sprintf( filename, sizeof( filename ), "models/classes/%s/icon", cls->commandstring );
	uiInfo.modeliconcache[classNum] = trap_R_RegisterShaderNoMip( filename );
	if( !uiInfo.modeliconcache[classNum] ) {
		Com_Printf( "^3Model icon load failure: %s\n", filename );
		noErrors = qfalse;
	}

	//
	// Load team colours
	//
	Com_sprintf( filename, sizeof( filename ), "models/classes/%s/skin.colours", cls->commandstring );
	if( (skinColourHandle = trap_PC_LoadSource( filename )) != NULL_FILE ) {
		pc_token_t token;
		int i, j, teamNum;

		for( teamNum = 0; teamNum < 4 && noErrors; teamNum++ ) {
			for( i = 0; i < 3 && noErrors; i++ ) {
				for( j = 0; j < 3 && noErrors; j++ ) {
					if( !trap_PC_ReadToken( skinColourHandle, &token ) ) {
						Com_Printf( "^3Skin colour load failure: %s\n", filename );
						noErrors = qfalse;
					} else {
						if( token.intvalue > 255 )
							token.intvalue = 255;
						else if( token.intvalue < 0 )
							token.intvalue = 0;
						uiInfo.skincolours[classNum][teamNum][i][j] = (byte)token.intvalue;
					}
				}
			}
		}

		trap_PC_FreeSource( skinColourHandle );
	} else {
		Com_Printf( "^3Skin colour load failure: %s\n", filename );
		noErrors = qfalse;
	}

	return( noErrors );
}


/*
======================
UI_MovedirAdjustment
======================
*/
static float UI_MovedirAdjustment( playerInfo_t *pi ) {
	vec3_t		relativeAngles;
	vec3_t		moveVector;

	VectorSubtract( pi->viewAngles, pi->moveAngles, relativeAngles );
	AngleVectors( relativeAngles, moveVector, NULL, NULL );
	if ( Q_fabs( moveVector[0] ) < 0.01 ) {
		moveVector[0] = 0.0;
	}
	if ( Q_fabs( moveVector[1] ) < 0.01 ) {
		moveVector[1] = 0.0;
	}

	if ( moveVector[1] == 0 && moveVector[0] > 0 ) {
		return 0;
	}
	if ( moveVector[1] < 0 && moveVector[0] > 0 ) {
		return 22;
	}
	if ( moveVector[1] < 0 && moveVector[0] == 0 ) {
		return 45;
	}
	if ( moveVector[1] < 0 && moveVector[0] < 0 ) {
		return -22;
	}
	if ( moveVector[1] == 0 && moveVector[0] < 0 ) {
		return 0;
	}
	if ( moveVector[1] > 0 && moveVector[0] < 0 ) {
		return 22;
	}
	if ( moveVector[1] > 0 && moveVector[0] == 0 ) {
		return  -45;
	}

	return -22;
}

/*
===============
UI_PlayerAngles
===============
*/
static void UI_PlayerAngles( playerInfo_t *pi, vec3_t legs[3], vec3_t torso[3], vec3_t head[3] ) {
	vec3_t		legsAngles, torsoAngles, headAngles;
	float		dest;
	float		adjust;

	VectorCopy( pi->viewAngles, headAngles );
	headAngles[YAW] = AngleMod( headAngles[YAW] );
	VectorClear( legsAngles );
	VectorClear( torsoAngles );

	// --------- yaw -------------

	// allow yaw to drift a bit
	if ( ( pi->legsAnim & ~ANIM_TOGGLEBIT ) != ANI_MOVE_IDLESTAND 
		|| ( pi->torsoAnim & ~ANIM_TOGGLEBIT ) != ANI_MOVE_IDLESTAND  ) {
		// if not standing still, always point all in the same direction
		pi->torso.yawing = qtrue;	// always center
		pi->torso.pitching = qtrue;	// always center
		pi->legs.yawing = qtrue;	// always center
	}

	// adjust legs for movement dir
	adjust = UI_MovedirAdjustment( pi );
	legsAngles[YAW] = headAngles[YAW] + adjust;
	torsoAngles[YAW] = headAngles[YAW] + 0.25 * adjust;


	// torso
	UI_SwingAngles( torsoAngles[YAW], 25.0f, 90.0f, SWINGSPEED, &pi->torso.yawAngle, &pi->torso.yawing );
	UI_SwingAngles( legsAngles[YAW], 40.0f, 90.0f, SWINGSPEED, &pi->legs.yawAngle, &pi->legs.yawing );

	torsoAngles[YAW] = pi->torso.yawAngle;
	legsAngles[YAW] = pi->legs.yawAngle;

	// --------- pitch -------------

	// only show a fraction of the pitch angle in the torso
	if ( headAngles[PITCH] > 180 ) {
		dest = (-360 + headAngles[PITCH]) * 0.75;
	} else {
		dest = headAngles[PITCH] * 0.75;
	}
	UI_SwingAngles( dest, 15, 30, 0.1f, &pi->torso.pitchAngle, &pi->torso.pitching );
	torsoAngles[PITCH] = pi->torso.pitchAngle;

	// pull the angles back out of the hierarchial chain
	AnglesSubtract( headAngles, torsoAngles, headAngles );
	AnglesSubtract( torsoAngles, legsAngles, torsoAngles );
	AnglesToAxis( legsAngles, legs );
	AnglesToAxis( torsoAngles, torso );
	AnglesToAxis( headAngles, head );
}

/*
===============
UI_ForceLegsAnim
===============
*/
static void UI_ForceLegsAnim( playerInfo_t *pi, int anim ) {
	pi->legsAnim = anim;//( ( pi->legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

	if ( anim == ANI_MOVE_JUMP ) {
		pi->legsAnimationTimer = UI_TIMER_JUMP;
	}
}

/*
===============
UI_SetLegsAnim
===============
*/
static void UI_SetLegsAnim( playerInfo_t *pi, int anim ) {
	if ( pi->pendingLegsAnim ) {
		anim = pi->pendingLegsAnim;
		pi->pendingLegsAnim = 0;
	}
	UI_ForceLegsAnim( pi, anim );
}


/*
===============
UI_ForceTorsoAnim
===============
*/
static void UI_ForceTorsoAnim( playerInfo_t *pi, int anim ) {
	pi->torsoAnim = anim;//( ( pi->torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

	if ( anim == ANI_SIGNAL_TAUNT ) {
		pi->torsoAnimationTimer = UI_TIMER_GESTURE;
	}

/*	if ( anim == TORSO_ATTACK || anim == TORSO_ATTACK2 ) {
		pi->torsoAnimationTimer = UI_TIMER_ATTACK;
	}*/
}


/*
===============
UI_SetTorsoAnim
===============
*/
static void UI_SetTorsoAnim( playerInfo_t *pi, int anim ) {
	if ( pi->pendingTorsoAnim ) {
		anim = pi->pendingTorsoAnim;
		pi->pendingTorsoAnim = 0;
	}

	UI_ForceTorsoAnim( pi, anim );
}


/*
===============
UI_TorsoSequencing
===============
*/
static void UI_TorsoSequencing( playerInfo_t *pi ) {
	int		currentAnim;

	currentAnim = pi->torsoAnim & ~ANIM_TOGGLEBIT;

	if ( pi->weapon != pi->currentWeapon ) {
		if ( currentAnim != ANI_INTERACT_WPCHANGE_START ) {
			pi->torsoAnimationTimer = 90;
			UI_ForceTorsoAnim( pi, ANI_INTERACT_WPCHANGE_START );
		}
	}

	if ( pi->torsoAnimationTimer > 0 ) {
		return;
	}

	if( currentAnim == ANI_SIGNAL_TAUNT ) {
		UI_SetTorsoAnim( pi, ANI_MOVE_IDLESTAND );
		return;
	}

/*	if( currentAnim == TORSO_ATTACK || currentAnim == TORSO_ATTACK2 ) {
		UI_SetTorsoAnim( pi, ANI_MOVE_IDLESTAND );
		return;
	}*/

	if ( currentAnim == ANI_INTERACT_WPCHANGE_START ) {
		UI_PlayerInfo_SetWeapon( pi, pi->weapon );
		pi->torsoAnimationTimer = 120;
		UI_ForceTorsoAnim( pi, ANI_INTERACT_WPCHANGE_END );
		return;
	}

/*	if ( currentAnim == TORSO_RAISE ) {
		UI_SetTorsoAnim( pi, ANI_MOVE_IDLESTAND );
		return;
	}*/
}


/*
===============
UI_LegsSequencing
===============
*/
static void UI_LegsSequencing( playerInfo_t *pi ) {
	int		currentAnim;

	currentAnim = pi->legsAnim & ~ANIM_TOGGLEBIT;

	if ( pi->legsAnimationTimer > 0 ) {
		if ( currentAnim == ANI_MOVE_JUMP ) {
			jumpHeight = JUMP_HEIGHT * sin( M_PI * ( UI_TIMER_JUMP - pi->legsAnimationTimer ) / UI_TIMER_JUMP );
		}
		return;
	}

	if ( currentAnim == ANI_MOVE_JUMP ) {
		UI_ForceLegsAnim( pi, ANI_MOVE_LAND );
		pi->legsAnimationTimer = UI_TIMER_LAND;
		jumpHeight = 0;
		return;
	}

	if ( currentAnim == ANI_MOVE_LAND ) {
		UI_SetLegsAnim( pi, ANI_MOVE_IDLESTAND );
		return;
	}
}

/*
===============
UI_SetLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
static void UI_SetLerpFrameAnimation( F2RDef_t *F2RScript, lerpFrame_t *lf, int newAnimation ) {
	animation_t	*anim;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if ( newAnimation < 1 || newAnimation > ANI_NUM ) {
		trap_Error( va("Bad animation number: %i", newAnimation) );
	}

	anim = F2RScript->animations[newAnimation - 1];

	if ( !anim ) {
		trap_Error( va("Missing animation number: %i", newAnimation) );
	}

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;
}

/*
===============
UI_RunLerpFrame
===============
*/
static void UI_RunLerpFrame( playerInfo_t *pi, F2RDef_t	*F2RScript, lerpFrame_t *lf, int newAnimation ) {
	int			f, numFrames;
	animation_t	*anim;

	// debugging tool to get no animations
	if ( !F2RScript ) {
		lf->oldFrame = lf->frame = lf->backlerp = 0;
		return;
	}

	// see if the animation sequence is switching
	if(	newAnimation != lf->animationNumber || !lf->animation || F2RScript->animations[0] != lf->animblock )
	{
		// If we've switched 
		UI_SetLerpFrameAnimation( F2RScript, lf, newAnimation );

		if( F2RScript->animations[0] != lf->animblock )
		{
			// We've done a agent switch, so do a 'fast switch' instead of lerping
			lf->animblock = F2RScript->animations[0];
			lf->frameTime = dp_realtime;
			lf->oldFrameTime = dp_realtime;
			lf->animationTime = dp_realtime;
			lf->oldFrameTime = dp_realtime;
			lf->backlerp = 0;
			lf->frame = lf->animation->firstFrame;
			return;
		}
	}

	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if ( dp_realtime >= lf->frameTime ) {
		lf->oldFrame = lf->frame;
		lf->oldFrameTime = lf->frameTime;

		// get the next frame based on the animation
		anim = lf->animation;
		if ( !anim->frameLerp ) {
			return;		// shouldn't happen
		}
		if ( dp_realtime < lf->animationTime ) {
			lf->frameTime = lf->animationTime;		// initial lerp
		} else {
			lf->frameTime = lf->oldFrameTime + anim->frameLerp;
		}
		f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;

		numFrames = anim->numFrames;
		if (anim->flipflop) {
			numFrames *= 2;
		}
		if ( f >= numFrames ) {
			f -= numFrames;
			if ( anim->loopFrames ) {
				f %= anim->loopFrames;
				f += anim->numFrames - anim->loopFrames;
			} else {
				f = numFrames - 1;
				// the animation is stuck at the end, so it
				// can immediately transition to another sequence
				lf->frameTime = dp_realtime;
			}
		}
		if ( anim->reversed ) {
			lf->frame = anim->firstFrame + anim->numFrames - 1 - f;
		}
		else if (anim->flipflop && f>=anim->numFrames) {
			lf->frame = anim->firstFrame + anim->numFrames - 1 - (f%anim->numFrames);
		}
		else {
			lf->frame = anim->firstFrame + f;
		}
		if ( dp_realtime > lf->frameTime ) {
			lf->frameTime = dp_realtime;
		}
	}

	if ( lf->frameTime > dp_realtime + 200 ) {
		lf->frameTime = dp_realtime;
	}

	if ( lf->oldFrameTime > dp_realtime ) {
		lf->oldFrameTime = dp_realtime;
	}
	// calculate current lerp value
	if ( lf->frameTime == lf->oldFrameTime ) {
		lf->backlerp = 0;
	} else {
		lf->backlerp = 1.0 - (float)( dp_realtime - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
	}
}

/*
===============
UI_PlayerAnimation
===============
*/
static void UI_PlayerAnimation( playerInfo_t *pi, int *legsOld, int *legs, float *legsBackLerp,
						int *torsoOld, int *torso, float *torsoBackLerp ) {
	F2RDef_t		*F2RScript;

	// legs animation
	pi->legsAnimationTimer -= uiInfo.uiDC.frameTime;
	if ( pi->legsAnimationTimer < 0 ) {
		pi->legsAnimationTimer = 0;
	}

	UI_LegsSequencing( pi );

	// do the shuffle turn frames locally
	if ( pi->legs.yawing && ( pi->legsAnim & ~ANIM_TOGGLEBIT ) == ANI_MOVE_IDLESTAND ) {
		F2RScript = UI_Q3F_LegsF2RScript(pi->classnum);
		UI_RunLerpFrame( pi, F2RScript, &pi->legs, ANI_MOVE_TURN);
		pi->legs.animationNumber = ANI_MOVE_TURN;
	} else {
		F2RScript = UI_Q3F_LegsF2RScript(pi->classnum);
		UI_RunLerpFrame( pi, F2RScript, &pi->legs, pi->legsAnim);
		pi->legs.animationNumber = pi->legsAnim & ~ANIM_TOGGLEBIT;
	}

	*legsOld = pi->legs.oldFrame;
	*legs = pi->legs.frame;
	*legsBackLerp = pi->legs.backlerp;

	// torso animation
	pi->torsoAnimationTimer -= uiInfo.uiDC.frameTime;
	if ( pi->torsoAnimationTimer < 0 ) {
		pi->torsoAnimationTimer = 0;
	}

	UI_TorsoSequencing( pi );

	F2RScript = UI_Q3F_TorsoF2RScript(pi->classnum);
	UI_RunLerpFrame( pi, F2RScript, &pi->torso, pi->torsoAnim );
	pi->torso.animationNumber = pi->torsoAnim & ~ANIM_TOGGLEBIT;

	*torsoOld = pi->torso.oldFrame;
	*torso = pi->torso.frame;
	*torsoBackLerp = pi->torso.backlerp;
}

/*
===============
UI_DrawPlayer
===============
*/
void UI_DrawPlayer( float x, float y, float w, float h, playerInfo_t *pi, int time ) {
	refdef_t		refdef;
	refEntity_t		legs;
	refEntity_t		torso;
	refEntity_t		head;
	refEntity_t		gun;
	refEntity_t		barrel;
	vec3_t			origin;
	int				renderfx;
	vec3_t			mins = {-16, -16, -24};
	vec3_t			maxs = {16, 16, 32};
	float			len;
	char			info[MAX_INFO_STRING];
	uiClientState_t cs;
	int				team;


	team = trap_Cvar_VariableValue( "hud_chosenClass" ) + 1;

	// slothy - temp hack for not being able to show models in menu
	if(uiInfo.ModelImages[team]) {
		UI_AdjustFrom640( &x, &y, &w, &h );
		trap_R_DrawStretchPic(x, y, w, h, 0, 0, 1, 1, uiInfo.ModelImages[team]);
	}

	return;
	// slothy

	trap_GetClientState( &cs );
	trap_GetConfigString( CS_PLAYERS + cs.clientNum, info, MAX_INFO_STRING );

	team = atoi(Info_ValueForKey(info, "t"));
	if(team < Q3F_TEAM_RED || team > Q3F_TEAM_GREEN) {
		team = Q3F_TEAM_RED; // djbob: team bogged up :?
	}

	dp_realtime = time;

	if ( pi->pendingWeapon != WP_INVALID && dp_realtime > pi->weaponTimer ) {
		pi->weapon = pi->pendingWeapon;
		pi->lastWeapon = pi->pendingWeapon;
		pi->pendingWeapon = WP_INVALID;
		pi->weaponTimer = 0;
		if( pi->currentWeapon != pi->weapon ) {
			trap_S_StartLocalSound( weaponChangeSound, CHAN_LOCAL );
		}
	}

	UI_AdjustFrom640( &x, &y, &w, &h );

	y -= jumpHeight;

	memset( &refdef, 0, sizeof( refdef ) );
	memset( &legs, 0, sizeof(legs) );
	memset( &torso, 0, sizeof(torso) );
	memset( &head, 0, sizeof(head) );

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

/*	refdef.fov_x = (int)((float)refdef.width / 640.0f * 90.0f);
	xx = refdef.width / tan( refdef.fov_x / 360 * M_PI );
	refdef.fov_y = atan2( refdef.height, xx );
	refdef.fov_y *= ( 360 / M_PI );*/

	refdef.fov_x = 25;
	refdef.fov_y = 35;

	// calculate distance so the player nearly fills the box
	len = 0.7 * ( maxs[2] - mins[2] );		
	origin[0] = len / tan( DEG2RAD(refdef.fov_x) * 0.5 );
	origin[1] = 0.5 * ( mins[1] + maxs[1] );
	origin[2] = -0.5 * ( mins[2] + maxs[2] );

	refdef.time = dp_realtime;

	trap_R_ClearScene();

	// get the rotation information
	UI_PlayerAngles( pi, legs.axis, torso.axis, head.axis );
	
	// get the animation state (after rotation, to allow feet shuffle)
	UI_PlayerAnimation( pi, &legs.oldframe, &legs.frame, &legs.backlerp,
		 &torso.oldframe, &torso.frame, &torso.backlerp );

	renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	
	//
	// add the legs
	//
	legs.hModel = *UI_Q3F_LegsModel( pi->classnum );
	legs.customSkin = *UI_Q3F_LegsSkin( pi->classnum );
	memcpy( legs.shaderRGBA, UI_Q3F_TorsoColour( pi->classnum, team ), 3 );
	legs.shaderRGBA[3] = 255.0f;

	VectorCopy( origin, legs.origin );

	VectorCopy( origin, legs.lightingOrigin );
	legs.renderfx = renderfx;

	VectorCopy (legs.origin, legs.oldorigin);

	trap_R_AddRefEntityToScene( &legs );//, &cent->currentState, ci->team );

	// if the model failed, allow the default nullmodel to be displayed
	if (!legs.hModel) {
		return;
	}

	
	//
	// add the torso
	//

	torso.hModel = *UI_Q3F_TorsoModel( pi->classnum );
	torso.customSkin = *UI_Q3F_TorsoSkin( pi->classnum );
	memcpy( torso.shaderRGBA, UI_Q3F_TorsoColour( pi->classnum, team ), 3 );
	torso.shaderRGBA[3] = 255.0f;

	VectorCopy( origin, torso.lightingOrigin );
	torso.renderfx = renderfx;
	
	UI_PositionRotatedEntityOnTag( &torso, &legs, "tag_torso");
	VectorCopy( torso.origin, torso.oldorigin );

	trap_R_AddRefEntityToScene( &torso );

	if (!torso.hModel) {
		return;
	}

	//
	// add the head
	//
	head.hModel = *UI_Q3F_HeadModel( pi->classnum );
//	head.customSkin = *UI_Q3F_HeadSkin( pi->classnum );
	memcpy( head.shaderRGBA, UI_Q3F_TorsoColour( pi->classnum, team ), 3 );
	head.shaderRGBA[3] = 255.0f;

	VectorCopy( origin, head.lightingOrigin );
	head.renderfx = renderfx;

	UI_PositionRotatedEntityOnTag( &head, &torso, "tag_head");

	trap_R_AddRefEntityToScene( &head );

	if (!head.hModel) {
		return;
	}

	//
	// add the gun
	//
	if ( pi->currentWeapon != WP_NONE ) {

		memset( &gun, 0, sizeof(gun) );
	
		// add the weapon
		gun.hModel = uiInfo.weaponModels[pi->currentWeapon];

		VectorCopy( origin, gun.origin );

		VectorCopy( origin, gun.lightingOrigin );
		gun.renderfx = RF_NOSHADOW;//renderfx;

		if( pi->currentWeapon == WP_AXE )
			UI_PositionEntityOnTag( &gun, &torso, "tag_melee");
		else
			UI_PositionEntityOnTag( &gun, &torso, "tag_weapon");
		
		VectorCopy (gun.origin, gun.oldorigin);
	
		trap_R_AddRefEntityToScene( &gun );

		// if the model failed, allow the default nullmodel to be displayed
		if (!gun.hModel) {
			return;
		}


		// add the spinning barrel
		if ( uiInfo.weaponBarrelModels[pi->currentWeapon] ) {
			vec3_t angles;

			memset( &barrel, 0, sizeof( barrel ) );
			
			VectorCopy( origin, barrel.lightingOrigin );
			barrel.renderfx = renderfx;

			barrel.hModel = uiInfo.weaponBarrelModels[pi->currentWeapon];
		
			angles[YAW] = 0;
			angles[PITCH] = 0;
			angles[ROLL] = 0;

			// grenade and pipe launcher have their own rendering
/*			if( weaponNum == WP_GRENADE_LAUNCHER || weaponNum == WP_PIPELAUNCHER )
				angles[ROLL] = CG_GrenadeLauncherSpinAngle( cent );
			else if( weaponNum == WP_MINIGUN )
				angles[ROLL] = CG_MinigunSpinAngle( cent );
			else
				angles[ROLL] = CG_MachinegunSpinAngle( cent );*/
		
			AnglesToAxis( angles, barrel.axis );

			UI_PositionEntityOnTag( &barrel, &gun, "tag_barrel" );

			trap_R_AddRefEntityToScene( &barrel );
		}
	}

	//
	// add an accent light
	//
	origin[0] -= 100;	// + = behind, - = in front
	origin[1] += 100;	// + = left, - = right
	origin[2] += 100;	// + = above, - = below
	trap_R_AddLightToScene( origin, 500, 1.0, 1.0, 1.0, 1.0, 0, 0 );

	origin[0] -= 100;
	origin[1] -= 100;
	origin[2] -= 100;
	trap_R_AddLightToScene( origin, 500, 1.0, 1.0, 0.0, 0.0, 0, 0 );

	trap_R_RenderScene( &refdef );
}

/*
===============
UI_DrawPlayer
===============
*/
void UI_DrawPlayerHead( float x, float y, float w, float h, playerInfo_t *pi, int time ) {
	refdef_t refdef;
	refEntity_t		head;
	vec3_t			mins, maxs, origin;
	vec3_t			angles;

	memset( &head, 0, sizeof(head) );


	if (!(head.hModel = *UI_Q3F_HeadModel( pi->classnum ))) {
		return;
	}

	// setup the refdef
	memset( &refdef, 0, sizeof( refdef ) );
	refdef.rdflags = RDF_NOWORLDMODEL;
	AxisClear( refdef.viewaxis );

	UI_AdjustFrom640( &x, &y, &w, &h );

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	uiInfo.uiDC.modelBounds( head.hModel, mins, maxs );

	origin[2] = -0.5 * ( mins[2] + maxs[2] );
	origin[1] = 0.5 * ( mins[1] + maxs[1] );
	origin[0] = (0.5 * ( maxs[2] - mins[2] )) / 0.12;

/*	refdef.fov_x = (int)((float)refdef.width / 640.0f * 90.0f);
	refdef.fov_y = atan2( refdef.height, refdef.width / tan( refdef.fov_x / 360 * M_PI ) );
	refdef.fov_y *= ( 360 / M_PI );*/

	refdef.fov_x = 25;
	refdef.fov_y = 35;

	trap_R_ClearScene();

	refdef.time = time;

	// add the model

	//adjust = 5.0 * sin( (float)uis.realtime / 500 );
	//adjust = 360 % (int)((float)uis.realtime / 1000);
	//VectorSet( angles, 0, 0, 1 );

	VectorSet( angles, 0, pi->viewAngles[1], 0 );
	AnglesToAxis( angles, head.axis );

	VectorCopy( origin, head.origin );
	VectorCopy( origin, head.lightingOrigin );
	head.renderfx = RF_LIGHTING_ORIGIN | RF_NOSHADOW;
	VectorCopy( head.origin, head.oldorigin );

	trap_R_AddRefEntityToScene( &head );
	trap_R_RenderScene( &refdef );
}

/*
=================
UI_RegisterWeapon
=================
*/
void UI_RegisterWeapon( int weaponNum ) {
	
	gitem_t			*item;
	char			path[MAX_QPATH];

	if ( weaponNum == 0 ) {
		return;
	}

	if ( uiInfo.weaponModels[weaponNum] ) {
		return;
	}

	for ( item = bg_itemlist + 1 ; item->classname ; item++ ) {
		if ( item->giType == IT_WEAPON && item->giTag == weaponNum ) {
			break;
		}
	}
	if ( !item->classname ) {
		trap_Error( va("Couldn't find weapon %i", weaponNum) );
	}

	// load cmodel before model so filecache works
	uiInfo.weaponModels[weaponNum] = trap_R_RegisterModel( item->world_model[0] );

	if ( weaponNum != WP_AXE ) {
		strcpy( path, item->world_model[0] );
		COM_StripExtension( path, path, sizeof(path) );
		strcat( path, "_barrel.md3" );
		uiInfo.weaponBarrelModels[weaponNum] = trap_R_RegisterModel( path );
	}
}

/*
===============
UI_PlayerInfo_SetInfo
===============
*/
void UI_PlayerInfo_SetInfo( playerInfo_t *pi, int legsAnim, int torsoAnim, vec3_t viewAngles, vec3_t moveAngles, weapon_t weaponNumber, int cls ) {
//	int			currentAnim;
//	weapon_t	weaponNum;

	pi->classnum = cls;

	// view angles
	VectorCopy( viewAngles, pi->viewAngles );

	// move angles
	VectorCopy( moveAngles, pi->moveAngles );

	jumpHeight = 0;
	pi->pendingLegsAnim = 0;
	UI_ForceLegsAnim( pi, legsAnim );
	pi->legs.yawAngle = viewAngles[YAW];
	pi->legs.yawing = qfalse;

	pi->pendingTorsoAnim = 0;
	UI_ForceTorsoAnim( pi, torsoAnim );
	pi->torso.yawAngle = viewAngles[YAW];
	pi->torso.yawing = qfalse;

	if ( weaponNumber != WP_INVALID ) {
		pi->weapon = weaponNumber;
		pi->currentWeapon = weaponNumber;
		pi->lastWeapon = weaponNumber;
		pi->pendingWeapon = WP_INVALID;
		pi->weaponTimer = 0;
		UI_PlayerInfo_SetWeapon( pi, pi->weapon );
		UI_RegisterWeapon( weaponNumber );
	}

	return;
}
