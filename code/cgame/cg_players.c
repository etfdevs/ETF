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

// cg_players.c -- handle the media and animation for player entities
#include "cg_local.h"
#include "../game/bg_q3f_playerclass.h"
#include "../game/bg_q3f_weapon.h"

// From cg_weapons.c

//void CG_FlameRender(centity_t *cent);

char	*cg_customSoundNames[MAX_CUSTOM_SOUNDS] = {
	"*death1.wav",
	"*death2.wav",
	"*death3.wav",
	"*jump1.wav",
	"*pain25_1.wav",
	"*pain50_1.wav",
	"*pain75_1.wav",
	"*pain100_1.wav",
	"*falling1.wav",
	"*gasp.wav",
	"*drown.wav",
	"*fall1.wav",
	"*taunt.wav",
	"*drown1.wav",
	"*drown2.wav",
	"*drown3.wav",
	"*drown4.wav",
	"*burn1.wav",
	"*burn2.wav",
	"*burn3.wav",
	"*burn4.wav",
	"*burn.wav",
};

//static int lastflags[MAX_CLIENTS];
static centity_t *agentdata;

/*
================
CG_CustomSound
================
*/
sfxHandle_t	CG_CustomSound( int clientNum, const char *soundName ) {
	clientInfo_t *ci;
	int			i;

	if( !soundName || !*soundName )
		return( 0 );

	if ( soundName[0] != '*' ) {
		return trap_S_RegisterSound( soundName ,qfalse );
	}

	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	ci = &cgs.clientinfo[ clientNum ];

	for ( i = 0 ; i < MAX_CUSTOM_SOUNDS && cg_customSoundNames[i] ; i++ ) {
		if ( !strcmp( soundName, cg_customSoundNames[i] ) ) {
			// RR2DO2: disguised sounds
			int j;

			// Search for agentdata
			for( j = 0; j < MAX_GENTITIES; j++ )
			{
				if(	cg_entities[ j ].currentState.eType == ET_Q3F_AGENTDATA &&			// Type of ent
					cg_entities[ j ].currentState.otherEntityNum == clientNum )			// Ent owner
					break;		// Found a match
			}

			if( j < MAX_GENTITIES && cg_entities[ j ].currentState.torsoAnim ) { // They're disguised, see what as.
				return cgs.media.soundcache[cg_entities[ j ].currentState.torsoAnim][i];
			} else
				return cgs.media.soundcache[ci->cls][i];
			// RR2DO2
		}
	}

	CG_Error( "Unknown custom sound: %s", soundName );
	return 0;
}



/*
=============================================================================

CLIENT INFO

=============================================================================
*/

qhandle_t *CG_Q3F_LegsModel( int classNum ) {
	return( &cgs.media.modelcache[classNum][0] );
}

qhandle_t *CG_Q3F_TorsoModel( int classNum ) {
	return( &cgs.media.modelcache[classNum][1] );
}

qhandle_t *CG_Q3F_HeadModel( int classNum ) {
	return( &cgs.media.modelcache[classNum][2] );
}

qhandle_t *CG_Q3F_LegsSkin( int classNum ) {
	return( &cgs.media.skincache[classNum][0] );
}

qhandle_t *CG_Q3F_TorsoSkin( int classNum ) {
	return( &cgs.media.skincache[classNum][1] );
}

qhandle_t *CG_Q3F_HeadSkin( int classNum ) {
	return( &cgs.media.skincache[classNum][2] );
}

qhandle_t *CG_Q3F_ModelIcon( int classNum ) {
	return( &cgs.media.modeliconcache[classNum] );
}

F2RDef_t *CG_Q3F_LegsF2RScript( int classNum ) {
	return( cgs.media.f2rcache[classNum][0] );
}

F2RDef_t *CG_Q3F_TorsoF2RScript( int classNum ) {
	return( cgs.media.f2rcache[classNum][1] );
}

F2RDef_t *CG_Q3F_HeadF2RScript( int classNum ) {
	return( cgs.media.f2rcache[classNum][2] );
}

byte *CG_Q3F_LegsColour( int classNum, q3f_team_t teamNum ) {
	return( &cgs.media.skincolours[classNum][teamNum-Q3F_TEAM_RED][0][0] );
}

byte *CG_Q3F_TorsoColour( int classNum, q3f_team_t teamNum ) {
	return( &cgs.media.skincolours[classNum][teamNum-Q3F_TEAM_RED][1][0] );
}

byte *CG_Q3F_HeadColour( int classNum, q3f_team_t teamNum ) {
	return( &cgs.media.skincolours[classNum][teamNum-Q3F_TEAM_RED][2][0] );
}

/*
==========================
CG_Q3F_RegisterPlayerClass
==========================
*/
qboolean CG_Q3F_RegisterClassModels( int classNum ) {
	qboolean				noErrors = qtrue;
	bg_q3f_playerclass_t	*cls;
	char					filename[MAX_QPATH];
	int						skinColourHandle;

	cls = bg_q3f_classlist[classNum];

	//
	// Load models - F2R automatically loads the animation data
	//

	// Load legs
	Com_sprintf( filename, sizeof( filename ), "models/classes/%s/lower.md3", cls->commandstring );
	cgs.media.modelcache[classNum][0] = trap_R_RegisterModel( filename );
	if( !cgs.media.modelcache[classNum][0] ) {
		Com_Printf( "^3Leg model load failure: %s\n", filename );
		noErrors = qfalse;
	} else {
		// Find F2RDef_t belonging to this model
		cgs.media.f2rcache[classNum][0] = F2R_GetForModel( cgs.media.modelcache[classNum][0] );
		if( !cgs.media.f2rcache[classNum][0] ) {
			COM_StripExtension( filename, filename, sizeof(filename) );
			Q_strcat( filename, sizeof(filename), ".f2r" );
			Com_Printf( "^3Leg model F2R load failure: %s\n", filename );
			noErrors = qfalse;
		}
	}

	// Load torso
	Com_sprintf( filename, sizeof( filename ), "models/classes/%s/upper.md3", cls->commandstring );
	cgs.media.modelcache[classNum][1] = trap_R_RegisterModel( filename );
	if( !cgs.media.modelcache[classNum][1] ) {
		Com_Printf( "^3Torso model load failure: %s\n", filename );
		noErrors = qfalse;
	} else {
		// Find F2RDef_t belonging to this model
		cgs.media.f2rcache[classNum][1] = F2R_GetForModel( cgs.media.modelcache[classNum][1] );
		if( !cgs.media.f2rcache[classNum][1] ) {
			COM_StripExtension( filename, filename, sizeof(filename) );
			Q_strcat( filename, sizeof(filename), ".f2r" );
			Com_Printf( "^3Torso model F2R load failure: %s\n", filename );
			noErrors = qfalse;
		}
	}

	// Load head
	Com_sprintf( filename, sizeof( filename ), "models/classes/%s/head.md3", cls->commandstring );
	cgs.media.modelcache[classNum][2] = trap_R_RegisterModel( filename );
	if( !cgs.media.modelcache[classNum][2] ) {
		Com_Printf( "^3Head model load failure: %s\n", filename );
		noErrors = qfalse;
	}

	//
	// Load skins
	//

	// start hack of the year!
	if( r_vertexLight.integer )
		trap_Cvar_Set( "r_vertexlight", "0" );

	// Load legs skin
	//Com_sprintf( filename, sizeof( filename ), "models/classes/%s/lower%s.skin", cls->commandstring, ( r_vertexLight.integer ? "_vertex" : "" ) );
	Com_sprintf( filename, sizeof( filename ), r_loresskins.integer ? "models/classes/%s/lower_lores.skin" : "models/classes/%s/lower.skin", cls->commandstring );
	cgs.media.skincache[classNum][0] = trap_R_RegisterSkin( filename );
	if( !cgs.media.skincache[classNum][0] ) {
		Com_Printf( "^3Leg skin load failure: %s\n", filename );
		noErrors = qfalse;
	}

	// Load torso skin
	//Com_sprintf( filename, sizeof( filename ), "models/classes/%s/upper%s.skin", cls->commandstring, ( r_vertexLight.integer ? "_vertex" : "" ) );
	Com_sprintf( filename, sizeof( filename ), r_loresskins.integer ? "models/classes/%s/upper_lores.skin" : "models/classes/%s/upper.skin", cls->commandstring );
	cgs.media.skincache[classNum][1] = trap_R_RegisterSkin( filename );
	if( !cgs.media.skincache[classNum][1] ) {
		Com_Printf( "^3Torso skin load failure: %s\n", filename );
		noErrors = qfalse;
	}

	// Load head skin
	/*Com_sprintf( filename, sizeof( filename ), "models/classes/%s/head%s.skin", cls->commandstring, ( r_vertexLight.integer ? "_vertex" : "" ) );
	cgs.media.skincache[classNum][2] = trap_R_RegisterSkin( filename );
	if( !cgs.media.skincache[classNum][2] ) {
		Com_Printf( "^3Head skin load failure: %s\n", filename );
		noErrors = qfalse;
	}*/

	// exit hack of the year!
	if( r_vertexLight.integer )
		trap_Cvar_Set( "r_vertexlight", "1" );

	//
	// Load icon
	//

	Com_sprintf( filename, sizeof( filename ), "models/classes/%s/icon.tga", cls->commandstring );
	cgs.media.modeliconcache[classNum] = trap_R_RegisterShaderNoMip( filename );
	if( !cgs.media.modeliconcache[classNum] ) {
		Com_Printf( "^3Model icon load failure: %s\n", filename );
		noErrors = qfalse;
	}

	//
	// Load team colours
	//
	Com_sprintf( filename, sizeof( filename ), "models/classes/%s/skin.colours", cls->commandstring );
	if( ( skinColourHandle = trap_PC_LoadSource( filename ) ) != NULL_FILE ) {
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
						cgs.media.skincolours[classNum][teamNum][i][j] = (byte)token.intvalue;
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

qboolean CG_Q3F_RegisterClassSounds( int classNum ) {
	qboolean				noErrors = qtrue;
	bg_q3f_playerclass_t	*cls;
	char					filename[MAX_QPATH];
	int						i;

	cls = bg_q3f_classlist[classNum];

	//
	// Load sounds
	//

	// Load player sounds
	for ( i = 0 ; i < MAX_CUSTOM_SOUNDS ; i++ ) {
		const char *s;

		s = cg_customSoundNames[i];
		if ( !s ) {
			break;
		}
		
		Com_sprintf( filename, sizeof( filename ), "sound/classes/%s/%s", cls->commandstring, s + 1 );
		cgs.media.soundcache[classNum][i] = trap_S_RegisterSound( filename, qfalse );
		if( !cgs.media.soundcache[classNum][i] ) {
			//Com_Printf( "^3Sound load failure: %s\n", filename );
			noErrors = qfalse;
		}
	}

	// Load the player's name sound
	if( cls->sound[0] && *cls->sound[0] ) {
		cgs.media.classnamesounds[classNum][0] = trap_S_RegisterSound( cls->sound[0], qtrue );
		if( !cgs.media.classnamesounds[classNum][0] ) {
			noErrors = qfalse;
		}
	}
	if( cls->sound[1] && *cls->sound[1] ) {
		cgs.media.classnamesounds[classNum][1] = trap_S_RegisterSound( cls->sound[1], qtrue );
		if( !cgs.media.classnamesounds[classNum][1] ) {
			noErrors = qfalse;
		}
	}

	return( noErrors );
}

/*
====================
CG_ColorFromString
====================
*/
/*static void CG_ColorFromString( const char *v, vec3_t color ) {
	int val;

	VectorClear( color );

	val = atoi( v );

	if ( val < 1 || val > 7 ) {
		VectorSet( color, 1, 1, 1 );
		return;
	}

	if ( val & 1 ) {
		color[2] = 1.0f;
	}
	if ( val & 2 ) {
		color[1] = 1.0f;
	}
	if ( val & 4 ) {
		color[0] = 1.0f;
	}
}*/

/*
======================
CG_NewClientInfo
======================
*/
void CG_NewClientInfo( int clientNum ) {
	clientInfo_t *ci;
	clientInfo_t newInfo;
	const char	*configstring;
	const char	*v;

	ci = &cgs.clientinfo[clientNum];

	configstring = CG_ConfigString( clientNum + CS_PLAYERS );
	if ( !configstring[0] ) {
		memset( ci, 0, sizeof( *ci ) );
		return;		// player just left
	}


	// build into a temp buffer so the defer checks can use
	// the old value
	memset( &newInfo, 0, sizeof( newInfo ) );

	// isolate the player's name
	v = Info_ValueForKey(configstring, "n");
	Q_strncpyz( newInfo.name, v, sizeof( newInfo.name ) );

	// team
	v = Info_ValueForKey( configstring, "t" );
	newInfo.team = atoi( v );

	// Golliwog: class
	v = Info_ValueForKey( configstring, "cls" );
	newInfo.cls = atoi( v );

	// djbob: gender
	v = Info_ValueForKey( configstring, "g" );
	newInfo.gender = atoi( v );

	// Ensiform: shoutcaster
	v = Info_ValueForKey( configstring, "sc" );
	newInfo.shoutcaster = atoi( v ) != 0;

	// replace whatever was there with the new one
	newInfo.infoValid = qtrue;
	*ci = newInfo;
	
	// RR2DO2
	//InitFlames( ci );
}

/*
=============================================================================

PLAYER ANIMATION

=============================================================================
*/

qboolean CG_Q3F_UseFakeAgentModel( int modelpart )
{
	entityState_t *agentstate;
	//int modelswitchtime;
	float starttime, endtime, currfrac;

	if( !agentdata )
		return( qfalse );

	agentstate = &agentdata->currentState;

	if( !agentstate )
		return( qfalse );

	//modelswitchtime = agentstate->time + (agentstate->time2 - agentstate->time) * 0.625 * (4.0f + (float)modelpart) / 6.0f;
	starttime	= agentstate->time + (agentstate->time2 - agentstate->time) * ( (float)modelpart / 6.0f );
	endtime		= agentstate->time + (agentstate->time2 - agentstate->time) * ( (4.0f + (float)modelpart) / 6.0f );
	currfrac	= ((float) (cg.time - starttime)) / (float) (endtime - starttime);

	return( (agentstate->modelindex2 & 1) &&
			!(agentstate->modelindex2 & 2) &&
			//( cg.time >= modelswitchtime ||
			( 0.625 < currfrac ||
			  (agentstate->torsoAnim && agentstate->otherEntityNum2 == DISGUISING_TEAM) ) );
}


/*
===============
CG_SetLerpFrameAnimation

may include ANIM_TOGGLEBIT
===============
*/
static void CG_SetLerpFrameAnimation( F2RDef_t *F2RScript, lerpFrame_t *lf, int newAnimation ) {
	animation_t	*anim;

	lf->animationNumber = newAnimation;
	newAnimation &= ~ANIM_TOGGLEBIT;

	if ( newAnimation < 1 || newAnimation > ANI_NUM ) {
		CG_Error( "Bad animation number: %i", newAnimation );
	}

	anim = F2RScript->animations[newAnimation - 1];

	if ( !anim ) {
		CG_Error( "Missing animation number %i in '%s'", newAnimation, F2RScript->F2RFile );
	}

	lf->animation = anim;
	lf->animationTime = lf->frameTime + anim->initialLerp;

	if ( cg_debugAnim.integer ) {
		CG_Printf( BOX_PRINT_MODE_CHAT, "Anim: %i\n", newAnimation );
	}
}

/*
===============
CG_RunLerpFrame

RR2DO2: the following lines make no sense at all
Sets cg.snap, cg.oldFrame, and cg.backlerp
cg.time should be between oldFrameTime and frameTime after exit
===============
*/
void CG_RunLerpFrame( F2RDef_t *F2RScript, lerpFrame_t *lf, int newAnimation, float speedScale ) {
	int			f, numFrames;
	animation_t	*anim;

	// debugging tool to get no animations
	if ( cg_animSpeed.integer == 0 || !F2RScript ) {
		lf->oldFrame = lf->frame = lf->backlerp = 0;
		return;
	}

	// see if the animation sequence is switching
	if(	newAnimation != lf->animationNumber || !lf->animation || F2RScript->animations[0] != lf->animblock )
	{
		// If we've switched 
		CG_SetLerpFrameAnimation( F2RScript, lf, newAnimation );

		if( F2RScript->animations[0] != lf->animblock )
		{
			// We've done a agent switch, so do a 'fast switch' instead of lerping
			lf->animblock = F2RScript->animations[0];
			lf->frameTime = cg.time;
			lf->oldFrameTime = cg.time;
			lf->animationTime = cg.time;
			lf->oldFrameTime = cg.time;
			lf->backlerp = 0;
			lf->frame = lf->animation->firstFrame;
			return;
		}
	}

	// if we have passed the current frame, move it to
	// oldFrame and calculate a new frame
	if ( cg.time >= lf->frameTime ) {
		lf->oldFrame = lf->frame;
		lf->oldFrameTime = lf->frameTime;

		// get the next frame based on the animation
		anim = lf->animation;
		if ( !anim->frameLerp ) {
			return;		// shouldn't happen
		}
		if ( cg.time < lf->animationTime ) {
			lf->frameTime = lf->animationTime;		// initial lerp
		} else {
			lf->frameTime = lf->oldFrameTime + anim->frameLerp;
		}
		f = ( lf->frameTime - lf->animationTime ) / anim->frameLerp;
		f *= speedScale;		// adjust for haste, etc

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
				lf->frameTime = cg.time;
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
		if ( cg.time > lf->frameTime ) {
			lf->frameTime = cg.time;
			if ( cg_debugAnim.integer ) {
				CG_Printf( BOX_PRINT_MODE_CHAT, "Clamp lf->frameTime\n");
			}
		}
	}

	if ( lf->frameTime > cg.time + 200 ) {
		lf->frameTime = cg.time;
	}

	if ( lf->oldFrameTime > cg.time ) {
		lf->oldFrameTime = cg.time;
	}
	// calculate current lerp value
	if ( lf->frameTime == lf->oldFrameTime ) {
		lf->backlerp = 0;
	} else {
		lf->backlerp = 1.0 - (float)( cg.time - lf->oldFrameTime ) / ( lf->frameTime - lf->oldFrameTime );
	}
}


/*
===============
CG_ClearLerpFrame
===============
*/
static void CG_ClearLerpFrame( F2RDef_t *F2RScript, lerpFrame_t *lf, int animationNumber ) {
	lf->frameTime = lf->oldFrameTime = cg.time;

	if ( !F2RScript ) {
		lf->oldFrame = lf->frame = lf->backlerp = 0;
		return;
	}

	CG_SetLerpFrameAnimation( F2RScript, lf, animationNumber );
	lf->oldFrame = lf->frame = lf->animation->firstFrame;
}


/*
===============
CG_PlayerAnimation
===============
*/
static void CG_PlayerAnimation( centity_t *cent, refEntity_t *legs, refEntity_t *torso ) {
	clientInfo_t	*ci;
	int				clientNum;
	float			speedScale;
	F2RDef_t		*F2RScript;
	int				fakeplayerclass;

	clientNum = cent->currentState.clientNum;

	if ( cg_noPlayerAnims.integer ) {
		legs->oldframe = legs->frame = torso->oldframe = torso->frame = legs->animNumber = torso->animNumber = 0;
		return;
	}

	if ( cent->currentState.powerups & ( 1 << PW_HASTE ) ) {
		speedScale = 1.5;
	} else {
		speedScale = 1;
	}

	ci = &cgs.clientinfo[ clientNum ];

	// get our disguised information
	// if we are doing disguise->disguise, grab the old disguise class/team
	fakeplayerclass = 0;
	if( agentdata ) {
		if( ( agentdata->currentState.modelindex2 & Q3F_AGENT_DISGUISE ) && ( agentdata->currentState.legsAnim ) ) {
			fakeplayerclass = agentdata->currentState.legsAnim;
		}
		if( CG_Q3F_UseFakeAgentModel( 0 ) ) {
			fakeplayerclass = agentdata->currentState.torsoAnim;
		}
	}

	F2RScript = fakeplayerclass
				? CG_Q3F_LegsF2RScript(fakeplayerclass)
				: CG_Q3F_LegsF2RScript(ci->cls);

	// do the shuffle turn frames locally
	if ( cent->pe.legs.yawing && ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) == ANI_MOVE_IDLESTAND ) {
		CG_RunLerpFrame( F2RScript, &cent->pe.legs, ANI_MOVE_TURN, speedScale );
		legs->animNumber = ANI_MOVE_TURN;
	} else {
		// remap leg anims if needed
		if( fakeplayerclass ) {
			int animNumber = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;

			// if ANI_SPECIAL, default to the idle animation of the current selected weapon
			if( animNumber == ANI_SPECIAL )
				animNumber = ANI_MOVE_IDLESTAND;

			CG_RunLerpFrame( F2RScript, &cent->pe.legs, animNumber, speedScale );
			legs->animNumber = animNumber;
		} else {
			CG_RunLerpFrame( F2RScript, &cent->pe.legs, cent->currentState.legsAnim, speedScale );
			legs->animNumber = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
		}
	}

	legs->oldframe = cent->pe.legs.oldFrame;
	legs->frame = cent->pe.legs.frame;
	legs->backlerp = cent->pe.legs.backlerp;

	// if we are doing disguise->disguise, grab the old disguise class/team
	fakeplayerclass = 0;
	if( agentdata ) {
		if( ( agentdata->currentState.modelindex2 & Q3F_AGENT_DISGUISE ) && ( agentdata->currentState.legsAnim ) ) {
			fakeplayerclass = agentdata->currentState.legsAnim;
		}
		if( CG_Q3F_UseFakeAgentModel( 1 ) ) {
			fakeplayerclass = agentdata->currentState.torsoAnim;
		}
	}

	// remap torso anims if needed
	if( fakeplayerclass ) {
//		bg_q3f_playerclass_t *cls = bg_q3f_classlist[fakeplayerclass];
		int animNumber = cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT;
		int weapNumber = BG_Q3F_GetRemappedWeaponFromWeaponNum( ci->cls, fakeplayerclass, cent->currentState.weapon );

		// if ANI_SPECIAL, default to the idle animation of the current selected weapon
		if( animNumber == ANI_SPECIAL )
			animNumber = PM_GetIdleAnim( weapNumber, fakeplayerclass );

		F2RScript = CG_Q3F_TorsoF2RScript( fakeplayerclass );

		animNumber = BG_Q3F_GetRemappedAnimFromWeaponNumAndAnim( cent->currentState.weapon, ci->cls, weapNumber, fakeplayerclass, animNumber );

		CG_RunLerpFrame( F2RScript, &cent->pe.torso, animNumber, speedScale );
		torso->animNumber = animNumber;
	} else {
		F2RScript = CG_Q3F_TorsoF2RScript( ci->cls );
		CG_RunLerpFrame( F2RScript, &cent->pe.torso, cent->currentState.torsoAnim, speedScale );
		torso->animNumber = cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT;
	}

	torso->oldframe = cent->pe.torso.oldFrame;
	torso->frame = cent->pe.torso.frame;
	torso->backlerp = cent->pe.torso.backlerp;
}

/*
=============================================================================

PLAYER ANGLES

=============================================================================
*/

/*
==================
CG_SwingAngles
==================
*/
static void CG_SwingAngles( float destination, float swingTolerance, float clampTolerance,
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
		move = cg.frametime * scale * speed;
		if ( move >= swing ) {
			move = swing;
			*swinging = qfalse;
		}
		*angle = AngleMod( *angle + move );
	} else if ( swing < 0 ) {
		move = cg.frametime * scale * -speed;
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
=================
CG_AddPainTwitch
=================
*/
static void CG_AddPainTwitch( centity_t *cent, vec3_t torsoAngles ) {
	int		t;
	float	f;

	t = cg.time - cent->pe.painTime;
	if ( t >= PAIN_TWITCH_TIME ) {
		return;
	}

	f = 1.0 - (float)t / PAIN_TWITCH_TIME;

	if ( cent->pe.painDirection ) {
		torsoAngles[ROLL] += 20 * f;
	} else {
		torsoAngles[ROLL] -= 20 * f;
	}
}


/*
===============
CG_PlayerAngles

Handles seperate torso motion

  legs pivot based on direction of movement

  head always looks exactly at cent->lerpAngles

  if motion < 20 degrees, show in head only
  if < 45 degrees, also show in torso
===============
*/
static void CG_PlayerAngles( centity_t *cent, vec3_t legs[3], vec3_t torso[3], vec3_t head[3] ) {
	vec3_t		legsAngles, torsoAngles, headAngles;
	float		dest;
	static	int	movementOffsets[8] = { 0, 22, 45, -22, 0, 22, -45, -22 };
	vec3_t		velocity;
	float		speed;
	int			dir;


	VectorCopy( cent->lerpAngles, headAngles );
	headAngles[YAW] = AngleMod( headAngles[YAW] );
	VectorClear( legsAngles );
	VectorClear( torsoAngles );

	// --------- yaw -------------

	// allow yaw to drift a bit
	if ( ( cent->currentState.legsAnim & ~ANIM_TOGGLEBIT ) != ANI_MOVE_IDLESTAND 
		|| ( cent->currentState.torsoAnim & ~ANIM_TOGGLEBIT ) != PM_GetIdleAnim( cent->currentState.weapon, cgs.clientinfo[ cent->currentState.number ].cls ) ) {
		// if not standing still, always point all in the same direction
		cent->pe.torso.yawing = qtrue;	// always center
		cent->pe.torso.pitching = qtrue;	// always center
		cent->pe.legs.yawing = qtrue;	// always center
	}

	// adjust legs for movement dir
	if ( cent->currentState.eFlags & EF_DEAD ) {
		// don't let dead bodies twitch
		dir = 0;
	} else {
		dir = cent->currentState.angles2[YAW];
		if ( dir < 0 || dir > 7 ) {
			CG_Error( "Bad player movement angle" );
		}
	}
	legsAngles[YAW] = headAngles[YAW] + movementOffsets[ dir ];
	torsoAngles[YAW] = headAngles[YAW] + 0.25 * movementOffsets[ dir ];

	// torso
	CG_SwingAngles( torsoAngles[YAW], 25, 90, cg_swingSpeed.value, &cent->pe.torso.yawAngle, &cent->pe.torso.yawing );
	CG_SwingAngles( legsAngles[YAW], 40, 90, cg_swingSpeed.value, &cent->pe.legs.yawAngle, &cent->pe.legs.yawing );

	torsoAngles[YAW] = cent->pe.torso.yawAngle;
	legsAngles[YAW] = cent->pe.legs.yawAngle;

	// --------- pitch -------------

	// only show a fraction of the pitch angle in the torso
	if ( headAngles[PITCH] > 180 ) {
		dest = (-360 + headAngles[PITCH]) * 0.75f;
	} else {
		dest = headAngles[PITCH] * 0.75f;
	}
	CG_SwingAngles( dest, 15, 30, 0.1f, &cent->pe.torso.pitchAngle, &cent->pe.torso.pitching );
	torsoAngles[PITCH] = cent->pe.torso.pitchAngle;

	// --------- roll -------------


	// lean towards the direction of travel
	VectorCopy( cent->currentState.pos.trDelta, velocity );
	speed = VectorNormalize( velocity );
	if ( speed ) {
		vec3_t	axis[3];
		float	side;

		speed *= 0.05f;

		AnglesToAxis( legsAngles, axis );
		side = speed * DotProduct( velocity, axis[1] );
		legsAngles[ROLL] -= side;

		side = speed * DotProduct( velocity, axis[0] );
		legsAngles[PITCH] += side;
	}

	// pain twitch
	CG_AddPainTwitch( cent, torsoAngles );

	// pull the angles back out of the hierarchial chain
	AnglesSubtract( headAngles, torsoAngles, headAngles );
	AnglesSubtract( torsoAngles, legsAngles, torsoAngles );
	AnglesToAxis( legsAngles, legs );
	AnglesToAxis( torsoAngles, torso );
	AnglesToAxis( headAngles, head );
}


//==========================================================================

/*
===============
CG_HasteTrail
===============
*/
static void CG_HasteTrail( centity_t *cent ) {
	vec3_t			origin;
	int				anim;

	if ( cent->trailTime > cg.time ) {
		return;
	}
	anim = cent->pe.legs.animationNumber & ~ANIM_TOGGLEBIT;
	if ( anim != ANI_MOVE_RUN && anim != ANI_MOVE_WALKBACK ) {
		return;
	}

	cent->trailTime += 100;
	if ( cent->trailTime < cg.time ) {
		cent->trailTime = cg.time;
	}

	VectorCopy( cent->lerpOrigin, origin );
	origin[2] -= 16;
	CG_SpawnSmokeSprite( origin, 500, colorWhite, 8, 6);
}


/*
===============
CG_TrailItem
===============
*/
void CG_TrailItem( centity_t *cent, qhandle_t hModel ) {
	refEntity_t		ent;
//	clientInfo_t	*ci;

//	ci = &cgs.clientinfo[ cent->currentState.number ];
	if( cent->currentState.number == cg.predictedPlayerEntity.currentState.clientNum )
		cent = &cg.predictedPlayerEntity;

	/*if( VectorCompare( vec3_origin, ci->tag_pack.origin ) ) */{
		vec3_t			angles;
		vec3_t			axis[3];

		VectorCopy( cent->lerpAngles, angles );
		angles[PITCH] = 0;
		angles[ROLL] = 0;
		AnglesToAxis( angles, axis );

		memset( &ent, 0, sizeof( ent ) );
		VectorMA( cent->lerpOrigin, -16, axis[0], ent.origin );
		ent.origin[2] += 16;
		angles[YAW] += 90;
		AnglesToAxis( angles, ent.axis );

		// RR2DO2: for lerping functions drawing these models
		VectorCopy( ent.origin, ent.oldorigin );

		ent.hModel = hModel;
		trap_R_AddRefEntityToScene( &ent, cent );
	}
}

/*
===============
CG_PlayerPowerups
===============
*/
static void CG_PlayerPowerups( centity_t *cent, refEntity_t *torso ) {
	int		powerups;

	powerups = cent->currentState.powerups;
	if ( !powerups ) {
		return;
	}

	// quad gives a dlight
	if ( powerups & ( 1 << PW_QUAD ) ) {
		//trap_R_AddLightToScene( cent->lerpOrigin, 200, 1.25f + (rand()&31), 0.2f, 0.2f, 1.f, 0, 0 );  // team colour quad
		if ( cgs.clientinfo[ cent->currentState.clientNum ].team == Q3F_TEAM_RED ) {
			trap_R_AddLightToScene( cent->lerpOrigin, 200, 1.25f + (rand()&31), 1, 0.2f, 0.2f, 0, 0 );	// RED RGB
		} else if ( cgs.clientinfo[ cent->currentState.clientNum ].team == Q3F_TEAM_GREEN ) {
			trap_R_AddLightToScene( cent->lerpOrigin, 200, 1.25f + (rand()&31), 0.2f, 1, 0.2f, 0, 0 );	// GREEN RGB
		} else if ( cgs.clientinfo[ cent->currentState.clientNum ].team == Q3F_TEAM_YELLOW ) {
			trap_R_AddLightToScene( cent->lerpOrigin, 200, 1.25f + (rand()&31), 1, 1, 0.2f, 0, 0 );		// YELLOW RGB
		} else { // Blue
			trap_R_AddLightToScene( cent->lerpOrigin, 200, 1.25f + (rand()&31), 0.2f, 0.2f, 1, 0, 0 );	// BLUE RGB
		}
	}

	if ( powerups & ( 1 << PW_BATTLESUIT ) || powerups & ( 1 << PW_PENTAGRAM ) ) {
		trap_R_AddLightToScene( cent->lerpOrigin, 150, 1.25f + ( rand() & 31 ), 1, 0.84f, 0.1f, 0, 0 );		// GOLD RGB
	}

	// flight plays a looped sound
	if ( powerups & ( 1 << PW_FLIGHT ) ) {
		trap_S_AddLoopingSound( cent->currentState.number, cent->lerpOrigin, vec3_origin, cgs.media.flightSound, 255, 0 );
	}

	// haste leaves smoke trails
	if ( powerups & ( 1 << PW_HASTE ) ) {
		CG_HasteTrail( cent );
	}
}


/*
===============
CG_PlayerFloatSprite

Float a sprite over the player's head
===============
*/
static void CG_PlayerFloatSprite( centity_t *cent, qhandle_t shader ) {
	int				rf;
	refEntity_t		ent;

	if ( cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson && !cg.renderingFlyBy && !cg.rendering2ndRefDef ) {
		rf = RF_THIRD_PERSON;		// only show in mirrors
	} else {
		rf = 0;
	}

	memset( &ent, 0, sizeof( ent ) );
	VectorCopy( cent->lerpOrigin, ent.origin );
	ent.origin[2] += 48;
	ent.reType = RT_SPRITE;
	ent.customShader = shader;
	ent.radius = 10;
	ent.renderfx = rf;
	ent.shaderRGBA[0] = 255;
	ent.shaderRGBA[1] = 255;
	ent.shaderRGBA[2] = 255;
	ent.shaderRGBA[3] = 255;
	trap_R_AddRefEntityToScene( &ent, cent );
}

byte cg_q3f_iconteamcolours[5][3] = {
	{ 0xFF, 0x20, 0x20 },		// RED
	{ 0x20, 0x20, 0xFF },		// BLUE
	{ 0xFF, 0xFF, 0x20 },		// YELLOW
	{ 0x20, 0xFF, 0x20 },		// GREEN
	{ 0xFF, 0xFF, 0xFF },		// FREE
};

static void CG_PlayerFloatSprite_TeamColoured( centity_t *cent, qhandle_t shader, int team ) {
	int				rf;
	refEntity_t		ent;
	vec4_t			colour;
	
	if ( cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson && !cg.renderingFlyBy && !cg.rendering2ndRefDef ) {
		rf = RF_THIRD_PERSON;		// only show in mirrors
	} else {
		rf = 0;
	}

	memset( &ent, 0, sizeof( ent ) );
	VectorCopy( cent->lerpOrigin, ent.origin );
	ent.radius = cg_drawFriendSize.integer;
	ent.origin[2] += 56 + (ent.radius - 6);
	ent.reType = RT_SPRITE;
	ent.customShader = shader;
	ent.renderfx = rf;

	ent.shaderRGBA[3] = 200;

	if( !Q_stricmp( cg_drawFriend.string, "team" ) ) {
		ent.shaderRGBA[0] = cg_q3f_iconteamcolours[team - Q3F_TEAM_RED][0];
		ent.shaderRGBA[1] = cg_q3f_iconteamcolours[team - Q3F_TEAM_RED][1];
		ent.shaderRGBA[2] = cg_q3f_iconteamcolours[team - Q3F_TEAM_RED][2];
	} else if( GetColourFromHex( cg_drawFriend.string, colour ) ) {
		ent.shaderRGBA[0] = colour[0];
		ent.shaderRGBA[1] = colour[1];
		ent.shaderRGBA[2] = colour[2];
	} else if( GetColourFromString( cg_drawFriend.string, colour ) ) {
		ent.shaderRGBA[0] = colour[0] * 0xFF;
		ent.shaderRGBA[1] = colour[1] * 0xFF;
		ent.shaderRGBA[2] = colour[2] * 0xFF;
	} else {
		ent.shaderRGBA[0] = 0x20;
		ent.shaderRGBA[1] = 0xFF;
		ent.shaderRGBA[2] = 0x20;
	}

	trap_R_AddRefEntityToScene( &ent, cent );
}


/*
===============
CG_PlayerSprites

Float sprites over the player's head
===============
*/
static void CG_PlayerSprites( centity_t *cent, centity_t *_agentdata ) {
	int		team;

	// Lagged icon should always show up :/
	if ( cent->currentState.eFlags & EF_CONNECTION ) {
		CG_PlayerFloatSprite( cent, cgs.media.connectionShader);
		return;
	}

	// If you're gassed, sprites's are out.
	if(cg.gasEndTime)
		return;

	team = cgs.clientinfo[ cent->currentState.number ].team;
 
	if (!CG_AlliedTeam( cg.snap->ps.persistant[PERS_TEAM], team )) {
		if (_agentdata && _agentdata->currentState.weapon) {
			if (!CG_AlliedTeam( cg.snap->ps.persistant[PERS_TEAM], _agentdata->currentState.weapon ))
				return;
			team = _agentdata->currentState.weapon;
		} else
			return;
	}

	if( (cent->currentState.eFlags & EF_Q3F_INVISIBLE) && !CG_AlliedTeam( cg.snap->ps.persistant[PERS_TEAM], cgs.clientinfo[ cent->currentState.number ].team ) ) {
	//if( (cent->currentState.eFlags & EF_Q3F_INVISIBLE) && (cgs.clientinfo[ cent->currentState.number ].team != cgs.clientinfo[cg.clientNum].team) ) {
		return;
	}

/*
	if ( cent->currentState.eFlags & EF_CONNECTION ) {
		CG_PlayerFloatSprite( cent, cgs.media.connectionShader );
		return;
	}
*/

	if ( (cent->currentState.eFlags & EF_TALK) ) {
		// Golliwog: Only show to teammates
		if( cgs.clientinfo[cent->currentState.number].infoValid )
			CG_PlayerFloatSprite( cent, cgs.media.balloonShader );
		return;
	}

	if ( cent->currentState.eFlags & EF_AWARD_EXCELLENT ) {
		CG_PlayerFloatSprite( cent, cgs.media.medalExcellent );
		return;
	}

	/* All icons below don't show for dead players */
	if (cent->currentState.eFlags & EF_DEAD)
		return;

	// Medic red cross sprite
	if( cent->currentState.eFlags & EF_Q3F_SAVEME && cg.snap->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_PARAMEDIC ) {
		CG_PlayerFloatSprite( cent, cgs.media.savemeShader );
		return;
	} 

	// Engineer armour sprite
	if( cent->currentState.eFlags & EF_Q3F_ARMORME && cg.snap->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_ENGINEER ) {
		CG_PlayerFloatSprite( cent, cgs.media.armormeShader );
		return;
	} 

	/* Always do the drawfriend state flag last if all others don't show */
	if( Q_stricmp(cg_drawFriend.string , "0" ) ) {
		CG_PlayerFloatSprite_TeamColoured( cent, cgs.media.friendShader, team );
	}
}

/*
===============
CG_PlayerShadow

Returns the Z component of the surface being shadowed

  should it return a full plane instead of a Z?
===============
*/
/* 
Canabis
Replaced with a global shadow handler 
static qboolean CG_PlayerShadow( centity_t *cent, float *shadowPlane ) {
	vec3_t		end, mins = {-15, -15, 0}, maxs = {15, 15, 2};
	trace_t		trace;
	float		alpha;

	*shadowPlane = 0;

	if ( cg_shadows.integer == 0 ) {
		return qfalse;
	}

	// no shadows when invisible (Golliwog: That includes agent invisible)
	if ( (cent->currentState.powerups & ( 1 << PW_INVIS )) || (cent->currentState.eFlags & EF_Q3F_INVISIBLE) ) {
		return qfalse;
	}

	// send a trace down from the player to the ground
	VectorCopy( cent->lerpOrigin, end );
	end[2] -= SHADOW_DISTANCE;

	trap_CM_BoxTrace( &trace, cent->lerpOrigin, end, mins, maxs, 0, MASK_PLAYERSOLID );

	// no shadow if too high
	if ( trace.fraction == 1.0 || trace.startsolid || trace.allsolid ) {
		return qfalse;
	}

	*shadowPlane = trace.endpos[2] + 1;

	if ( cg_shadows.integer != 1 ) {	// no mark for stencil or projection shadows
		return qtrue;
	}

	// fade the shadow out with height
	alpha = 1.0 - trace.fraction;

	// add the mark as a temporary, so it goes directly to the renderer
	// without taking a spot in the cg_marks array
	CG_ImpactMark( cgs.media.shadowMarkShader, trace.endpos, trace.plane.normal, 
		cent->pe.legs.yawAngle, alpha,alpha,alpha,1, qfalse, 24, qtrue );

	return qtrue;
}
*/

/*
===============
CG_PlayerSplash

Draw a mark at the water surface
===============
*/
static void CG_PlayerSplash( centity_t *cent ) {
	vec3_t		start, end;
	trace_t		trace;
	int			contents;
	polyVert_t	verts[4];

	if ( !cg_shadows.integer ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, end );
	end[2] -= 24;

	// if the feet aren't in liquid, don't make a mark
	// this won't handle moving water brushes, but they wouldn't draw right anyway...
	contents = CG_PointContents( end, 0 );
	if ( !( contents & ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) ) {
		return;
	}

	VectorCopy( cent->lerpOrigin, start );
	start[2] += 32;

	// if the head isn't out of liquid, don't make a mark
	contents = CG_PointContents( start, 0 );
	if ( contents & ( CONTENTS_SOLID | CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) ) {
		return;
	}

	// trace down to find the surface
	trap_CM_BoxTrace( &trace, start, end, NULL, NULL, 0, ( CONTENTS_WATER | CONTENTS_SLIME | CONTENTS_LAVA ) );

	if ( trace.fraction == 1.0 ) {
		return;
	}

	// create a mark polygon
	VectorCopy( trace.endpos, verts[0].xyz );
	verts[0].xyz[0] -= 32;
	verts[0].xyz[1] -= 32;
	verts[0].st[0] = 0;
	verts[0].st[1] = 0;
	verts[0].modulate[0] = 255;
	verts[0].modulate[1] = 255;
	verts[0].modulate[2] = 255;
	verts[0].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[1].xyz );
	verts[1].xyz[0] -= 32;
	verts[1].xyz[1] += 32;
	verts[1].st[0] = 0;
	verts[1].st[1] = 1;
	verts[1].modulate[0] = 255;
	verts[1].modulate[1] = 255;
	verts[1].modulate[2] = 255;
	verts[1].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[2].xyz );
	verts[2].xyz[0] += 32;
	verts[2].xyz[1] += 32;
	verts[2].st[0] = 1;
	verts[2].st[1] = 1;
	verts[2].modulate[0] = 255;
	verts[2].modulate[1] = 255;
	verts[2].modulate[2] = 255;
	verts[2].modulate[3] = 255;

	VectorCopy( trace.endpos, verts[3].xyz );
	verts[3].xyz[0] += 32;
	verts[3].xyz[1] -= 32;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0;
	verts[3].modulate[0] = 255;
	verts[3].modulate[1] = 255;
	verts[3].modulate[2] = 255;
	verts[3].modulate[3] = 255;

	trap_R_AddPolyToScene( cgs.media.wakeMarkShader, 4, verts );
}

qhandle_t CG_Q3F_ShaderForQuad(int team)
{
	if(team == Q3F_TEAM_RED)
		return (cgs.media.redQuadShader);
	else if(team == Q3F_TEAM_YELLOW)
		return (cgs.media.yellowQuadShader);
	else if(team == Q3F_TEAM_GREEN)
		return (cgs.media.greenQuadShader);
	else
		return (cgs.media.quadShader);
}

float          *CG_Q3F_LightForQuad(int team)
{
	static vec3_t   red = { 0.7f, 0.2f, 0.2f };
	static vec3_t   blue = { 0.2f, 0.2f, 0.7f };
	static vec3_t   yellow = { 0.7f, 0.7f, 0.2f };
	static vec3_t   green = { 0.2f, 0.7f, 0.2f };
/*	static vec3_t   red = { 1.0f, 0.2f, 0.2f };
	static vec3_t   blue = { 0.2f, 0.2f, 1.0f };
	static vec3_t   yellow = { 1.0, 1.0, 0.2f };
	static vec3_t   green = { 0.2f, 1.0, 0.2f };*/

	switch (team)
	{
		case Q3F_TEAM_RED:
			return red;
		case Q3F_TEAM_YELLOW:
			return yellow;
		case Q3F_TEAM_GREEN:
			return green;
		case Q3F_TEAM_BLUE:
		default:
			return blue;
	}
}

/*
===============
CG_AddRefEntityWithPowerups

Adds a piece with modifications or duplications for powerups
Also called by CG_Missile for quad rockets, but nobody can tell...
===============
*/
void CG_AddRefEntityWithPowerups( refEntity_t *ent, entityState_t *state, int team ) {

	if ( state->powerups & ( 1 << PW_INVIS ) ) {
		ent->customShader = cgs.media.invisShader;
		trap_R_AddRefEntityToScene( ent, NULL );
	} else {
		trap_R_AddRefEntityToScene( ent, NULL );
		if ( state->powerups & ( 1 << PW_QUAD ) ) {
			ent->customShader = CG_Q3F_ShaderForQuad( team );
			trap_R_AddRefEntityToScene( ent, NULL );
		}
		if ( state->powerups & ( 1 << PW_REGEN ) ) {
			ent->customShader = cgs.media.regenShader;
			trap_R_AddRefEntityToScene( ent, NULL );
		}
		if ( state->powerups & ( 1 << PW_BATTLESUIT ) ) {
			ent->customShader = cgs.media.battleSuitShader;
			trap_R_AddRefEntityToScene( ent, NULL );
		}
		if ( state->powerups & ( 1 << PW_PENTAGRAM ) ) {
			ent->customShader = cgs.media.battleSuitShader;
			trap_R_AddRefEntityToScene( ent, NULL );
		}
		if ( state->extFlags & EXTF_BURNING ) {
			ent->customShader = cgs.media.onFireShader0;
			trap_R_AddRefEntityToScene( ent, NULL );

			ent->customShader = cgs.media.onFireShader1;
			trap_R_AddRefEntityToScene( ent, NULL );
		}
	}
}

/*
=================
CG_LightVerts
=================
*/
void CG_LightVerts( vec3_t normal, int numVerts, polyVert_t *verts, qboolean keepSrcColor )
{
#ifdef API_Q3
	int				i, j;
	float			incoming;
	vec3_t			ambientLight;
	vec3_t			lightDir;
	vec3_t			directedLight;
	vec3_t			lightForVert;

	trap_R_LightForPoint( verts[0].xyz, ambientLight, directedLight, lightDir );

	for (i = 0; i < numVerts; i++) {
		incoming = DotProduct (normal, lightDir);
		if ( incoming <= 0 ) {
			if( keepSrcColor ) {
				// FIXME this isn't a correct algorithm, just one that looked right in one testmap
				verts[i].modulate[0] = verts[i].modulate[0] * ( ambientLight[0] / 255.f );
				verts[i].modulate[1] = verts[i].modulate[1] * ( ambientLight[1] / 255.f );
				verts[i].modulate[2] = verts[i].modulate[2] * ( ambientLight[2] / 255.f );
			} else {
				verts[i].modulate[0] = ambientLight[0];
				verts[i].modulate[1] = ambientLight[1];
				verts[i].modulate[2] = ambientLight[2];
				verts[i].modulate[3] = 255;
			}
		
			continue;
		} 
		j = ( ambientLight[0] + incoming * directedLight[0] );
		if ( j > 255 ) {
			j = 255;
		}
		//verts[i].modulate[0] = j;
		lightForVert[0] = j;

		j = ( ambientLight[1] + incoming * directedLight[1] );
		if ( j > 255 ) {
			j = 255;
		}
		//verts[i].modulate[1] = j;
		lightForVert[1] = j;

		j = ( ambientLight[2] + incoming * directedLight[2] );
		if ( j > 255 ) {
			j = 255;
		}
		//verts[i].modulate[2] = j;
		lightForVert[2] = j;

		if( keepSrcColor ) {
			//verts[i].modulate[0] = verts[i].modulate[0] * ( lightForVert[0] / 255.f );
			//verts[i].modulate[1] = verts[i].modulate[1] * ( lightForVert[1] / 255.f );
			//verts[i].modulate[2] = verts[i].modulate[2] * ( lightForVert[2] / 255.f );
			// FIXME this isn't a correct algorithm, just one that looked right in one testmap
			verts[i].modulate[0] = verts[i].modulate[0] * ( lightForVert[0] / 2.f / 255.f );
			verts[i].modulate[1] = verts[i].modulate[1] * ( lightForVert[1] / 2.f / 255.f );
			verts[i].modulate[2] = verts[i].modulate[2] * ( lightForVert[2] / 2.f / 255.f );
		} else {
			verts[i].modulate[0] = lightForVert[0];
			verts[i].modulate[1] = lightForVert[1];
			verts[i].modulate[2] = lightForVert[2];
			verts[i].modulate[3] = 255;
		}

		//verts[i].modulate[3] = 255;
	}
#endif
}

byte cg_q3f_agentcolours[5][3] = {
	{ 0xFF, 0x80, 0x80 },		// RED
	{ 0x80, 0x80, 0xFF },		// BLUE
	{ 0xFF, 0xFF, 0x80 },		// YELLOW
	{ 0x80, 0xFF, 0x80 },		// GREEN
	{ 0xFF, 0xFF, 0xFF },		// FREE
};

/*byte cg_q3f_teamcolours[5][3] = {
	{ 0x96, 0x14, 0x14 },		// RED
	{ 0x14, 0x14, 0x96 },		// BLUE
	{ 0xFF, 0x96, 0x14 },		// YELLOW
	{ 0x14, 0x96, 0x14 },		// GREEN
	{ 0xFF, 0xFF, 0xFF },		// FREE
};*/

void CG_Q3F_CalcAgentVisibility( qboolean *drawmodel, float *shaderalpha, qboolean *newmodel, float fracstart, float fracend, entityState_t *agentstate )
{
	// Golliwog: Calculate the visibility of the entity. The morph covers:
	// normal -> fadeout -> fadein -> new, where fracstart and fracend are the fade
	// start and end points.

	float morphalpha, invisalpha, currfrac;
	qboolean drawmorph, drawinvis, hasmorph, hasinvis;
	int starttime = 0, endtime = 0;

	drawinvis = qtrue;
	hasinvis = qfalse;
	invisalpha = 0;

	if( agentstate->modelindex2 & Q3F_AGENT_INVIS )
	{
		// We want the agent invisible effect

		hasinvis = qtrue;
		starttime = agentstate->origin2[0];
		if( agentstate->modelindex2 & Q3F_AGENT_INVISEND )
		{
			// Going to visible.

			if( cg.time > (starttime + (Q3F_AGENT_VISIBLE_TIME/2)) )
				invisalpha = ((float)((starttime + Q3F_AGENT_VISIBLE_TIME) - cg.time)) / (Q3F_AGENT_VISIBLE_TIME/2);
			else {
				invisalpha	= 1.0 - ((float)((starttime + (Q3F_AGENT_VISIBLE_TIME/2)) - cg.time)) / (Q3F_AGENT_VISIBLE_TIME/2);
				drawinvis	= qfalse;
			}
		}
		else {
			// Going invisible

			if( cg.time > (starttime + (Q3F_AGENT_INVISIBLE_TIME/2)) )
			{
				invisalpha	= ((float)((starttime + Q3F_AGENT_INVISIBLE_TIME) - cg.time)) / (Q3F_AGENT_INVISIBLE_TIME/2);
				drawinvis	= qfalse;
			}
			else invisalpha = 1.0 - ((float)((starttime + (Q3F_AGENT_INVISIBLE_TIME/2)) - cg.time)) / (Q3F_AGENT_INVISIBLE_TIME/2);
		}
		if( invisalpha <= 0 ) {
			/* Fully invisble stop it */
			*shaderalpha = 0;
			*drawmodel = qfalse;
			return;
		}
		if( invisalpha > 1 )
			invisalpha = 1;
	}

	drawmorph = qtrue;
	hasmorph = qfalse;
	morphalpha = 0;
	*newmodel = qfalse;
	if( agentstate->modelindex2	& Q3F_AGENT_DISGUISE )
	{
		hasmorph = qtrue;
		if( agentstate->modelindex2 & Q3F_AGENT_DISGUISEEND )
		{
			// Going backwards, normal fade, or else not disguising
			starttime = agentstate->time;
			endtime = agentstate->time2;
		}
		else {
			// Going forwards, staggered fade
			starttime	= agentstate->time + (agentstate->time2 - agentstate->time) * fracstart;
			endtime		= agentstate->time + (agentstate->time2 - agentstate->time) * fracend;
		}

		if( cg.time >= starttime )
		{
			if( cg.time >= endtime )
				*newmodel = qtrue;
			else {
				// Mid-morph, calculate the alpha

				currfrac = ((float) (cg.time - starttime)) / (float) (endtime - starttime);

				if( agentstate->modelindex2 & Q3F_AGENT_DISGUISEEND )
				{
					// Fast fadeout.

					morphalpha = 1 - currfrac;
					*newmodel = qtrue;
				}
				else if( currfrac < 0.5 )		// Fade in chrome
				{
					morphalpha = 2 * currfrac;
				}
				else if( currfrac < 0.625 )
				{
					morphalpha = 1 - 8 * (currfrac - 0.5);
					drawmorph = qfalse;
				}
				else if( currfrac < 0.75 )
				{
					morphalpha = 8 * (currfrac - 0.625);
					drawmorph = qfalse;
					*newmodel = qtrue;
				}
				else {
					morphalpha = 1 - 4 * (currfrac - 0.75);
					*newmodel = qtrue;
				}
			}
		}
		if( morphalpha < 0 )
			morphalpha = 0;
		if( morphalpha > 1 )
			morphalpha = 1;
	}

	if( agentstate->torsoAnim && agentstate->otherEntityNum2 == DISGUISING_TEAM ) {
		if(starttime && endtime)
			currfrac = ((float) (cg.time - starttime)) / (float) (endtime - starttime);
		else
			currfrac = 1.0;
		if( currfrac > 0.5 )
			*newmodel = qtrue;
	}

	if( drawmorph && drawinvis )
	{
		// Draw model, use highest shader alpha effect.

		*drawmodel = qtrue;
		*shaderalpha = (morphalpha > invisalpha) ? morphalpha : invisalpha;
	}
	else {
		// Don't draw model, use combined shader alpha effect.

		*drawmodel = qfalse;
		*shaderalpha = morphalpha ? (invisalpha ? (invisalpha * morphalpha) : morphalpha) : invisalpha;
		//*shaderalpha = hasmorph ? (hasinvis ? (invisalpha * morphalpha) : morphalpha) : invisalpha;
	}
}

qboolean CG_Q3F_AddRefEntityWithAgentEffect( refEntity_t *ent, centity_t *cent, entityState_t *state, q3f_team_t team, int section )
{
	// Render with agent effect
	// Effect is divided into 6 units, with each of the 3 parts covering 4 units.
	// Frame bit 0: disguise
	// Frame bit 1: 0 for forward, 1 for reverse.
	// Frame bit 2: invis
	// Frame bit 3: 0 for forward, 1 for reverse
	// New class is held in torsoAnim, new team in weapon.
	// Disguise times are held

	float shaderalpha;
	qboolean drawmodel, newmodel, oldnewmodel;
	entityState_t *agentstate;
	int playerclass, playerteam;

	if( state->powerups & (1 << PW_INVIS) )
	{
		// Do nothing if 'invisible'
		CG_AddRefEntityWithPowerups( ent, state, team );
		return( qfalse );
	}

	agentstate = &agentdata->currentState;

	if( !cgs.media.agentShader )
			cgs.media.agentShader = trap_R_RegisterShader( "gfx/agenteffect" );

	CG_Q3F_CalcAgentVisibility(	&drawmodel, &shaderalpha, &newmodel,
								section / 6.0, (section + 4.0) / 6.0,
								agentstate );

	// if we are doing disguise->disguise, grab the old disguise class/team
	oldnewmodel = qfalse;
	if( ( agentstate->modelindex2	& Q3F_AGENT_DISGUISE ) && ( agentstate->legsAnim || agentstate->frame ) ) {
		oldnewmodel = qtrue;
		playerclass = agentstate->legsAnim;
		playerteam = agentstate->frame;
	}

	 if( newmodel ) {
		playerclass = agentstate->torsoAnim;
		playerteam = agentstate->weapon;
		/*if( agentstate->torsoAnim ) {
			ent->hModel	= cgs.media.modelcache[agentstate->torsoAnim][section];
			ent->customSkin = cgs.media.skincache[agentstate->torsoAnim ? agentstate->torsoAnim : cent->currentState.otherEntityNum2][section];
		}
		if( agentstate->torsoAnim || agentstate->weapon ) {
			switch( section ) {
				case 0: memcpy( ent->shaderRGBA, CG_Q3F_LegsColour( ( agentstate->torsoAnim ? agentstate->torsoAnim : cent->currentState.otherEntityNum2 ),
																	( agentstate->weapon ? agentstate->weapon : cgs.clientinfo[cent->currentState.number].team ) ), 3 );
						break;
				case 1:	memcpy( ent->shaderRGBA, CG_Q3F_TorsoColour( ( agentstate->torsoAnim ? agentstate->torsoAnim : cent->currentState.otherEntityNum2 ),
																	( agentstate->weapon ? agentstate->weapon : cgs.clientinfo[cent->currentState.number].team ) ), 3 );
						break;
				case 2:	memcpy( ent->shaderRGBA, CG_Q3F_HeadColour( ( agentstate->torsoAnim ? agentstate->torsoAnim : cent->currentState.otherEntityNum2 ),
																	( agentstate->weapon ? agentstate->weapon : cgs.clientinfo[cent->currentState.number].team ) ), 3 );
						break;
			}
			ent->shaderRGBA[3] = 255.0;
		}*/
	}

	if( newmodel || oldnewmodel ) {
		if( playerclass ) {
			ent->hModel	= cgs.media.modelcache[playerclass][section];
			ent->customSkin = cgs.media.skincache[playerclass ? playerclass : cent->currentState.otherEntityNum2][section];
		}
		if( playerclass || playerteam ) {
			switch( section ) {
				case 0: memcpy( ent->shaderRGBA, CG_Q3F_LegsColour( ( playerclass ? playerclass : cent->currentState.otherEntityNum2 ),
																	( playerteam ? playerteam : cgs.clientinfo[cent->currentState.number].team ) ), 3 );
						break;
				case 1:	memcpy( ent->shaderRGBA, CG_Q3F_TorsoColour( ( playerclass ? playerclass : cent->currentState.otherEntityNum2 ),
																	( playerteam ? playerteam : cgs.clientinfo[cent->currentState.number].team ) ), 3 );
						break;
				case 2:	memcpy( ent->shaderRGBA, CG_Q3F_HeadColour( ( playerclass ? playerclass : cent->currentState.otherEntityNum2 ),
																	( playerteam ? playerteam : cgs.clientinfo[cent->currentState.number].team ) ), 3 );
						break;
			}
			ent->shaderRGBA[3] = 255.0;
		}
	}


	if( drawmodel )
		CG_AddRefEntityWithPowerups( ent, state, team );
	
	if( shaderalpha > 0 ) {
		memcpy( ent->shaderRGBA, cg_q3f_agentcolours[team - Q3F_TEAM_RED], 3 );
		ent->shaderRGBA[3] = 255.0 * shaderalpha;
		ent->customShader = cgs.media.agentShader;
		trap_R_AddRefEntityToScene( ent, cent );
	}

	return( newmodel );
}

void CG_Q3F_AddBackPack( centity_t *cent, clientInfo_t *ci, int renderfx ) {
	int				i;
	float			shaderalpha;
	qboolean		drawmodel, newmodel;
	refEntity_t		backpack;

	drawmodel = qtrue;
	shaderalpha = 0.f;

	if (ci->cls == Q3F_CLASS_CIVILIAN)
		return;

	if( agentdata )
	{
		int agentclass = 0;
		if( ( agentdata->currentState.modelindex2 & Q3F_AGENT_DISGUISE ) && ( agentdata->currentState.legsAnim ) ) {
			agentclass = agentdata->currentState.legsAnim;
		}
		if( CG_Q3F_UseFakeAgentModel( 1 ) ) {
			agentclass = agentdata->currentState.torsoAnim;
		}

		if( agentclass == Q3F_CLASS_CIVILIAN )
			return;

		CG_Q3F_CalcAgentVisibility(	&drawmodel, &shaderalpha, &newmodel,
									1.0f/6.0f, 5.0f/6.0f, &agentdata->currentState );
	}

	memset( &backpack, 0, sizeof( backpack ) );

	// lerp the tag
	VectorCopy( ci->torso_refent.origin, backpack.origin );
	for ( i = 0 ; i < 3 ; i++ ) {
		VectorMA( backpack.origin, ci->tag_pack.origin[i], ci->torso_refent.axis[i], backpack.origin );
	}

	MatrixMultiply( ci->tag_pack.axis, ci->torso_refent.axis, backpack.axis );
	backpack.backlerp = ci->torso_backlerp;
	
	// RR2DO2: for lerping functions drawing these models
	VectorCopy( backpack.origin, backpack.oldorigin );

	VectorCopy( cent->lerpOrigin, backpack.lightingOrigin );

	backpack.reType = RT_MODEL;
	backpack.renderfx = renderfx;
	backpack.hModel = cgs.media.backpack;

	if( drawmodel )
		//trap_R_AddRefEntityToScene( &backpack, cent );
		CG_AddRefEntityWithPowerups( &backpack, &cent->currentState, ci->team );

	if( shaderalpha )
	{
		memcpy( backpack.shaderRGBA, cg_q3f_agentcolours[ci->team - Q3F_TEAM_RED], 3 );
		backpack.shaderRGBA[3] = 255.0 * shaderalpha;
		backpack.customShader = cgs.media.agentShader;
		trap_R_AddRefEntityToScene( &backpack, cent );
	}
}

void CG_AddPlayerEffects( centity_t *cent, const refEntity_t *torso, const refEntity_t *head )
{
	int clientNum = cent->currentState.clientNum;
	vec3_t	origin;

	if (cgs.clientinfo[clientNum].team == Q3F_TEAM_SPECTATOR ||
		cgs.clientinfo[clientNum].cls == Q3F_CLASS_NULL )
		return;

	//Don't show when invisible
	if ( (cent->currentState.powerups & ( 1 << PW_INVIS )) ||
		(cent->currentState.eFlags & EF_Q3F_INVISIBLE) ) 
		return;

	//Diseased player effect
	if( cent->currentState.eFlags & EF_Q3F_DISEASED ) 
	{
		Spirit_RunScript(cgs.spirit.diseased, cent->lerpOrigin, cent->lerpOrigin, axisDefault, (int)cent );
		cent->pe.EffectFlags |= PE_EF_DISEASED;
	} else if ( cent->pe.EffectFlags & PE_EF_DISEASED ) {
		if( !Spirit_UpdateScript( cgs.spirit.diseased, cent->lerpOrigin, axisDefault, (int)cent ))
			cent->pe.EffectFlags &= ~PE_EF_DISEASED;
	}

	/* Don't show the rest of the effects when your in 1st person */
	if ( cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson 
		&& !cg.renderingFlyBy && !cg.rendering2ndRefDef )
		return;

	//Stunned player effect
	if( cent->currentState.powerups & ( 1 << PW_Q3F_CONCUSS ))
	{
		Spirit_RunModel( cgs.spirit.stunned, torso , "tag_head", (int)cent );
		cent->pe.EffectFlags |= PE_EF_STUNNED;
	} else if (cent->pe.EffectFlags & PE_EF_STUNNED ) {
		if (!Spirit_UpdateModel( cgs.spirit.stunned, torso , "tag_head", (int)cent ))
			cent->pe.EffectFlags &= ~PE_EF_STUNNED;
	}

	//Gassed player effect
	if( cent->currentState.powerups & ( 1 << PW_Q3F_GAS ))
	{
		Spirit_RunModel( cgs.spirit.gassed, torso , "tag_head", (int)cent );
		cent->pe.EffectFlags |= PE_EF_GASSED;
	} else if (cent->pe.EffectFlags & PE_EF_GASSED ) {
		if (!Spirit_UpdateModel( cgs.spirit.gassed, torso , "tag_head", (int)cent )) 
			cent->pe.EffectFlags &= ~PE_EF_GASSED;
	}

	//Flashed player effect
	if( cent->currentState.powerups & ( 1 << PW_Q3F_FLASH ))
	{
		Spirit_RunModel( cgs.spirit.flashed, torso , "tag_head", (int)cent );
		cent->pe.EffectFlags |= PE_EF_FLASHED;
	} else if (cent->pe.EffectFlags & PE_EF_FLASHED ) {
		if (!Spirit_UpdateModel( cgs.spirit.flashed, torso , "tag_head", (int)cent ))
			cent->pe.EffectFlags &= ~PE_EF_FLASHED;
	}

	//Tranqed player effect
	if( cent->currentState.extFlags & EXTF_TRANQED)
	{
		Spirit_RunModel( cgs.spirit.tranqed, torso , "tag_head", (int)cent );
		cent->pe.EffectFlags |= PE_EF_TRANQED;
	} else if (cent->pe.EffectFlags & PE_EF_TRANQED ) {
		if (!Spirit_UpdateModel( cgs.spirit.tranqed, torso , "tag_head", (int)cent ))
			cent->pe.EffectFlags &= ~PE_EF_TRANQED;
	}

	VectorCopy(cent->lerpOrigin, origin);
	origin[2] -= 16;

	//Leg wounded player effect
	if( cent->currentState.extFlags & EXTF_LEGWOUNDS)
	{
		Spirit_RunScript(cgs.spirit.legshot, origin, origin, axisDefault, (int)cent );
		cent->pe.EffectFlags |= PE_EF_DISEASED;
	} else if ( cent->pe.EffectFlags & PE_EF_LEGWOUNDS ) {
		if( !Spirit_UpdateScript( cgs.spirit.legshot, origin, axisDefault, (int)cent ))
			cent->pe.EffectFlags &= ~PE_EF_LEGWOUNDS;
	}
}

/*
===============
CG_Player
===============
*/
void CG_Player( centity_t *cent ) {
	// Golliwog: Modified for agent effects.
	clientInfo_t	*ci;
	refEntity_t		legs;
	refEntity_t		torso;
	refEntity_t		head;
	int				clientNum;
	int				renderfx;
	qboolean		shadow, newModel;
	float			shadowPlane;
	centity_t		hallucination;

	// the client number is stored in clientNum.  It can't be derived
	// from the entity number, because a single client may have
	// multiple corpses on the level using the same clientinfo
	clientNum = cent->currentState.clientNum;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		CG_Error( "Bad clientNum on player entity");
	}
	ci = &cgs.clientinfo[ clientNum ];

	// it is possible to see corpses from disconnected players that may
	// not have valid clientinfo (Golliwog: Or unteamed players)
	if ( !ci->infoValid || !(cgs.teams & (1 << ci->team)) || (cent->currentState.eFlags & EF_Q3F_NOSPAWN) )
	{
		if( cg_debugAnim.integer && !ci->infoValid )
			CG_Printf( BOX_PRINT_MODE_CHAT, "Invalid clientinfo for %d\n.", clientNum );
		return;
	}
	// Golliwog: Check the class is valid too
	if( !(cgs.classes & (1 << cent->currentState.otherEntityNum2)) )
	{
		if( cg_debugAnim.integer )
			CG_Printf( BOX_PRINT_MODE_CHAT, "Invalid class (%d) for %d\n.", cent->currentState.otherEntityNum2, clientNum );
		return;
	}

/*	if(cent->currentState.weapon == WP_MINIGUN)
	{

		int difflags = lastflags[clientNum] ^ cent->currentState.eFlags;
		if(difflags & EF_Q3F_AIMING)
		{
			if(cent->currentState.eFlags & EF_Q3F_AIMING)
			{
				if(clientNum == cg.snap->ps.clientNum)
					cg.shudderStart = cg.time;
			}
			else if( !(cent->currentState.eFlags & EF_Q3F_AIMING))
				if(clientNum == cg.snap->ps.clientNum)
					cg.shudderStart = 0;
		}
		lastflags[clientNum] = cent->currentState.eFlags;
	}
*/
	if( cent->currentState.eFlags & (EF_Q3F_DISGUISE|EF_Q3F_INVISIBLE) ) {
		// We don't draw, but we might want a agent effect instead.

		for( renderfx = 0; renderfx < MAX_ENTITIES; renderfx++ ) {
			agentdata = &cg_entities[renderfx];

			if( (agentdata->currentState.eType == ET_Q3F_AGENTDATA) &&
				agentdata->currentValid &&
				(agentdata->currentState.otherEntityNum == cent->currentState.number) )
				break;		// We've found one.
		}

		if( renderfx == MAX_ENTITIES )
			agentdata = NULL;	// We might not have the control ent yet, or it's finished
	} else {
		agentdata = NULL;
	}

	if ( cent->currentState.number == cg.snap->ps.clientNum )
		cg.agentDataEntity = agentdata;

	if( cg.gasEndTime &&
		cent->currentState.number < MAX_CLIENTS &&
		cg.gasPlayerClass[cent->currentState.number] != 0xFF &&
		cent->currentState.number != cg.snap->ps.clientNum ) {
		// We're hallucinating, create an 'agentdata' for the occasion.

		if(	agentdata &&
			//(agentdata->currentState.modelindex2 & 12) == 4 &&
			//agentdata->currentState.origin2[0] < (cg.time + 4000) )
			(agentdata->currentState.modelindex2 & 12) == Q3F_AGENT_INVIS &&
			agentdata->currentState.origin2[0] < (cg.time + Q3F_AGENT_INVISIBLE_TIME) )
			return;		// They're invisible
		memset( &hallucination, 0, sizeof(hallucination) );
		hallucination.currentState.eType = ET_Q3F_AGENTDATA;
		hallucination.currentState.otherEntityNum = cent->currentState.number;
		hallucination.currentState.modelindex2 = 1;
		hallucination.currentState.time = cg.time - 1001;
		hallucination.currentState.time2 = cg.time - 1;
		hallucination.currentState.torsoAnim = cg.gasPlayerClass[cent->currentState.number];
		hallucination.currentState.weapon = cg.gasPlayerTeam[cent->currentState.number];
		// RR2DO2: FIXME: set .frame to a random valid team
		hallucination.currentValid = qtrue;
		agentdata = &hallucination;
	} else {
		hallucination.currentValid = qfalse;
	}

	if( agentdata &&
		(agentdata->currentState.eType != ET_Q3F_AGENTDATA ||
		!agentdata->currentValid ||
		!(agentdata->currentState.otherEntityNum == cent->currentState.number)) ) {
//		CG_Printf( "Invalid Agentdata.\n" );
//		return;
		agentdata = NULL;
	}

	// get the player model information
	renderfx = 0;
	if ( cent->currentState.number == cg.snap->ps.clientNum) {
		if ( cent->currentState.number == cg.snap->ps.clientNum && !cg.renderingThirdPerson && !cg.renderingFlyBy && !cg.rendering2ndRefDef ) {
			renderfx = RF_THIRD_PERSON;			// only draw in mirrors
		} else {
			if (cg_cameraMode.integer) {
				return;
			}
		}
	}

	memset( &legs, 0, sizeof(legs) );
	memset( &torso, 0, sizeof(torso) );
	memset( &head, 0, sizeof(head) );

	// get the rotation information
	CG_PlayerAngles( cent, legs.axis, torso.axis, head.axis );

	// get the animation state (after rotation, to allow feet shuffle)
	CG_PlayerAnimation( cent, &legs, &torso );

	CG_PlayerSprites( cent, agentdata );

	// no shadows when invisible (Golliwog: That includes agent invisible)
	if ( (cent->currentState.powerups & ( 1 << PW_INVIS )) || (cent->currentState.eFlags & EF_Q3F_INVISIBLE) ) {
		shadow=qfalse;
		shadowPlane=0;
	} else shadow = CG_ShadowMark(cent->lerpOrigin,15,128,&shadowPlane);

	// add a water splash if partially in and out of water
	CG_PlayerSplash( cent );

	if ( cg_shadows.integer == 3 && shadow ) {
		renderfx |= RF_SHADOW_PLANE;
	}
	renderfx |= RF_LIGHTING_ORIGIN;			// use the same origin for all
	renderfx |= RF_NOSKYPORTAL;				// RR2DO2

	//
	// add the legs
	//
	legs.hModel = *CG_Q3F_LegsModel( ci->cls );
	legs.customSkin = *CG_Q3F_LegsSkin( ci->cls );

	VectorCopy( cent->lerpOrigin, legs.origin );
	VectorCopy( legs.origin, legs.oldorigin );

	VectorCopy( cent->lerpOrigin, legs.lightingOrigin );
	legs.shadowPlane = shadowPlane;
	legs.renderfx = renderfx;
	VectorCopy (legs.origin, legs.oldorigin);	// don't positionally lerp at all

	memcpy( legs.shaderRGBA, CG_Q3F_LegsColour( ci->cls, ci->team ), 3 );
	legs.shaderRGBA[3] = 255.0;

	if( agentdata ) {
		CG_Q3F_AddRefEntityWithAgentEffect( &legs, cent, &cent->currentState, ci->team, 0 );
	} else {
		CG_AddRefEntityWithPowerups( &legs, &cent->currentState, ci->team );
	}


	// if the model failed, allow the default nullmodel to be displayed
	if (!legs.hModel) {
		return;
	}

	//
	// add the torso
	//
	torso.hModel = *CG_Q3F_TorsoModel( ci->cls );
	if (!torso.hModel) {
		return;
	}

	torso.customSkin = *CG_Q3F_TorsoSkin( ci->cls );

	VectorCopy( cent->lerpOrigin, torso.lightingOrigin );

	CG_PositionRotatedEntityOnTag( &torso, &legs, "tag_torso" );
	VectorCopy( torso.origin, torso.oldorigin );

	torso.shadowPlane = shadowPlane;
	torso.renderfx = renderfx;

	memcpy( torso.shaderRGBA, CG_Q3F_TorsoColour( ci->cls, ci->team ), 3 );
	torso.shaderRGBA[3] = 255.0;

	if( agentdata ) {
		newModel = CG_Q3F_AddRefEntityWithAgentEffect( &torso, cent, &cent->currentState, ci->team, 1 );
	} else {
		CG_AddRefEntityWithPowerups( &torso, &cent->currentState, ci->team );
	}

	// RR2DO2: get tag_pack for usage with attached goal objects
	if(!(cent->currentState.eFlags & EF_DEAD)) {
		trap_R_LerpTag( &ci->tag_pack, &torso, "tag_pack", 0 );
		VectorCopy( torso.origin, ci->torso_refent.origin );
		VectorCopy( torso.axis[0], ci->torso_refent.axis[0] );
		VectorCopy( torso.axis[1], ci->torso_refent.axis[1] );
		VectorCopy( torso.axis[2], ci->torso_refent.axis[2] );
		ci->torso_backlerp = torso.backlerp;
	}

	//
	// add the head
	//
	head.hModel = *CG_Q3F_HeadModel( ci->cls );
	if (!head.hModel) {
		return;
	}
	//head.customSkin = *CG_Q3F_HeadSkin( ci->team );// FIXME paramter asks for skin but we're passing team wtf? , changing for 1.7 to see if it breaks anything
	//head.customSkin = *CG_Q3F_HeadSkin( ci->cls );

	VectorCopy( cent->lerpOrigin, head.lightingOrigin );

	CG_PositionRotatedEntityOnTag( &head, &torso,"tag_head" );
	VectorCopy( head.origin, head.oldorigin );

	head.shadowPlane = shadowPlane;
	head.renderfx = renderfx;

	memcpy( head.shaderRGBA, CG_Q3F_HeadColour( ci->cls, ci->team ), 3 );
	head.shaderRGBA[3] = 255.0;

	if( agentdata )
		CG_Q3F_AddRefEntityWithAgentEffect( &head, cent, &cent->currentState, ci->team, 2 );
	else
		CG_AddRefEntityWithPowerups( &head, &cent->currentState, ci->team );

	// EF_NODRAW
	// add the gun / barrel / flash
	//
	// add weapon if the player is alive
	if ( !(cent->currentState.eFlags & EF_DEAD) ) {
		CG_AddPlayerWeapon( &torso, NULL, cent, ci->team, agentdata );
	}

	// add powerups floating behind the player
	CG_PlayerPowerups( cent, &torso );
	CG_AddPlayerEffects( cent, &torso, &head);

	// Golliwog: Store velocity and apparent team for later reference by sniper
	cent->pe.visibleTeam	=	(agentdata != NULL && agentdata->currentState.weapon)
								? agentdata->currentState.weapon : ci->team;
	cent->pe.visibleClass	=	(agentdata != NULL && newModel)
								? agentdata->currentState.torsoAnim : ci->cls;
	// Golliwog.

	// add backpack if the player is alive
	if ( !(cent->currentState.eFlags & EF_DEAD) ) {
		CG_Q3F_AddBackPack( cent, ci, renderfx );

	}
	// add the bounding box (if cg_drawBBox is 1)
	if ( cg_drawBBox.integer ) 
		CG_AddBoundingBox( cent , CG_TeamColor( ci->team ) );
}


//=====================================================================

/*
===============
CG_ResetPlayerEntity

A player just came into view or teleported, so reset all animation info
===============
*/
void CG_ResetPlayerEntity( centity_t *cent ) {
	cent->errorTime = -99999;		// guarantee no error decay added
	cent->extrapolated = qfalse;	

	CG_ClearLerpFrame( cgs.media.f2rcache[cgs.clientinfo[ cent->currentState.clientNum ].cls][0], &cent->pe.legs, cent->currentState.legsAnim );
	CG_ClearLerpFrame( cgs.media.f2rcache[cgs.clientinfo[ cent->currentState.clientNum ].cls][1], &cent->pe.torso, cent->currentState.torsoAnim );

	BG_EvaluateTrajectory( &cent->currentState.pos, cg.time, cent->lerpOrigin );
	BG_EvaluateTrajectory( &cent->currentState.apos, cg.time, cent->lerpAngles );

	VectorCopy( cent->lerpOrigin, cent->rawOrigin );
	VectorCopy( cent->lerpAngles, cent->rawAngles );

	memset( &cent->pe.legs, 0, sizeof( cent->pe.legs ) );
	cent->pe.legs.yawAngle = cent->rawAngles[YAW];
	cent->pe.legs.yawing = qfalse;
	cent->pe.legs.pitchAngle = 0;
	cent->pe.legs.pitching = qfalse;

	memset( &cent->pe.torso, 0, sizeof( cent->pe.torso ) );
	cent->pe.torso.yawAngle = cent->rawAngles[YAW];
	cent->pe.torso.yawing = qfalse;
	cent->pe.torso.pitchAngle = cent->rawAngles[PITCH];
	cent->pe.torso.pitching = qfalse;

	cent->pe.EffectFlags = 0;

	if ( cg_debugPosition.integer ) {
		CG_Printf(BOX_PRINT_MODE_CHAT, "%i ResetPlayerEntity yaw=%i\n", cent->currentState.number, cent->pe.torso.yawAngle );
	}
}
