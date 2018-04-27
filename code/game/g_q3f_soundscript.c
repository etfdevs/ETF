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

#include "g_local.h"

// Defines

typedef struct g_q3f_sscr_speaker_s {
	char	noise[256];
//	qboolean	broadcast;
	qboolean	looped;
	int			wait;
	int			random;
	vec3_t		origin;
} g_q3f_sscr_speaker_t;

// Support functions

static void G_Q3F_SSCR_Error( int scriptHandle, char *format, ... ) {
    // Stop with an error, showing file and line
    
    va_list	argptr;
    char	buff[2048], filename[1024];
    int		line;
    
    va_start( argptr, format );
	Q_vsnprintf( buff, sizeof(buff), format, argptr );
    va_end( argptr );
    
    trap_PC_SourceFileAndLine( scriptHandle, filename, &line );
    G_Printf( "^1Error in soundscript: %s at '%s' line %d.^0\n", buff, filename, line );    
}

static qboolean G_Q3F_SSCR_GetToken( int scriptHandle, pc_token_t *token, char *requiredToken ) {
    // Get the next token
    
    if( !trap_PC_ReadToken( scriptHandle, token ) ) {
		if( !requiredToken )
			return( qfalse );
	    
		G_Q3F_SSCR_Error( scriptHandle, "Unexpected end of file" );
			return( qfalse );
    }
    
    if( requiredToken && *requiredToken && Q_stricmp( token->string, requiredToken ) ) {
		G_Q3F_SSCR_Error( scriptHandle, "Expected '%s', found '%s'", requiredToken, token->string );
		return( qfalse );
    }
    
    return( qtrue );
}

static qboolean G_Q3F_SSCR_GetIntegerToken( int scriptHandle, int *i ) {
	pc_token_t	token;
	qboolean	negative = qfalse;

	if( !G_Q3F_SSCR_GetToken( scriptHandle, &token, NULL ) )
		return( qfalse );
	
	if( token.string[0] == '-' ) {
		if( !G_Q3F_SSCR_GetToken( scriptHandle, &token, NULL ) )
			return( qfalse );
			
		negative = qtrue;
	}
	
	if( token.type != TT_NUMBER ) {
		G_Q3F_SSCR_Error( scriptHandle, "Expected integer but found '%s'", token.string );
		return( qfalse );
	}
	    
	*i = token.intvalue;

	if( negative )
		*i = - *i;

	return( qtrue );    
}

static qboolean G_Q3F_SSCR_GetFloatToken( int scriptHandle, float *f ) {
	pc_token_t	token;
	qboolean	negative = qfalse;

	if( !G_Q3F_SSCR_GetToken( scriptHandle, &token, NULL ) )
		return( qfalse );

	if( token.string[0] == '-' ) {
		if( !G_Q3F_SSCR_GetToken( scriptHandle, &token, NULL ) )
			return( qfalse );
		
		negative = qtrue;
	}
	if( token.type != TT_NUMBER ) {
		G_Q3F_SSCR_Error( scriptHandle, "Expected float but found '%s'", token.string );
		return( qfalse );
	}
	if( negative )
		*f = -token.floatvalue;
	else
		*f = token.floatvalue;

	return( qtrue );
}

static qboolean G_Q3F_SSCR_GetVec( int scriptHandle, vec3_t *c ) {
	int i;
	float f;

	for( i = 0; i < 3; i++ ) {
		if( !G_Q3F_SSCR_GetFloatToken( scriptHandle, &f ) ) {
			return( qfalse );
		}
		(*c)[i] = f;
	}
	return( qtrue );
}

// Parsing functions
static qboolean G_Q3F_SSCR_ParseSpeaker( int scriptHandle ) {
    pc_token_t			token;
    g_q3f_sscr_speaker_t	speaker;
    gentity_t			*ent;
    
    if( !G_Q3F_SSCR_GetToken( scriptHandle, &token, "{" ) )
		return( qfalse );
	
    memset( &speaker, 0, sizeof(speaker) );
    speaker.looped = qtrue;
	
    while( G_Q3F_SSCR_GetToken( scriptHandle, &token, NULL ) ) {
		if( !Q_stricmp( token.string, "noise" ) ) {
			if( !G_Q3F_SSCR_GetToken( scriptHandle, &token, NULL ) )
				return( qfalse );

			Q_strncpyz(	speaker.noise, token.string, sizeof(speaker.noise) );
//		} else if( !Q_stricmp( token.string, "broadcast" ) ) {
//			speaker.broadcast = qtrue;
		} else if( !Q_stricmp( token.string, "wait" ) ) {
			if( !G_Q3F_SSCR_GetIntegerToken( scriptHandle, &speaker.wait ) )
				return( qfalse );
			
			speaker.looped = qfalse;
		} else if( !Q_stricmp( token.string, "random" ) ) {
			if( !G_Q3F_SSCR_GetIntegerToken( scriptHandle, &speaker.random ) )
				return( qfalse );
			
			speaker.looped = qfalse;
		} else if( !Q_stricmp( token.string, "origin" ) ) {
			if( !G_Q3F_SSCR_GetVec( scriptHandle, &speaker.origin ) )
				return( qfalse );
		} else if( !Q_stricmp( token.string, "}" ) ) {
			break;
		} else {
			G_Q3F_SSCR_Error( scriptHandle, "Unexpected token '%s'", token.string );
		}
    }
    
    // Check if we had a noise key (nonfatal error)
    if( !speaker.noise[0] ) {
		G_Printf( "^3Warning: speaker entry in soundscript without noise key^0\n" );
		return( qtrue );
    }
    
    // We got everything needed, spawn our speaker

    ent = G_Spawn();
	ent->classname = "soundscript_speaker";
    ent->noise_index = G_SoundIndex( speaker.noise );
    ent->s.eType = ET_SPEAKER;
    ent->s.eventParm = ent->noise_index;
    ent->wait = speaker.wait;
    ent->random = speaker.random;
    ent->s.frame = ent->wait * 10;
    ent->s.clientNum = ent->random * 10;
    
    if( speaker.looped )
		ent->s.loopSound = ent->noise_index;
    
//    if( speaker.broadcast )
//		ent->r.svFlags |= SVF_BROADCAST;
	
    VectorCopy( speaker.origin, ent->s.origin );
    VectorCopy( speaker.origin, ent->s.pos.trBase );
	VectorCopy( speaker.origin, ent->r.currentOrigin );

	trap_LinkEntity( ent );
    
    return( qtrue );
}

// This attempts to load a <mapname>.sscr file. If it succeeds
// it spawns a number of soundentities from the descriptors in the
// file. These act like primitive target_speaker entities, in the 
// way that they can't be toggled, but do have all the looping/random
// capabilities.

qboolean G_Q3F_SSCR_ParseSoundScript( char *mapname ) {
    char	*scriptName, rawmapname[1024];
    int		scriptHandle;
	int		numSpeakers = 0;

	COM_StripExtension( COM_SkipPath( mapname ), rawmapname, sizeof(rawmapname) );
	scriptName = va( "%s.sscr", rawmapname );
    
    // See if we have a soundscript for this map, if so, parse it
    if( ( scriptHandle = trap_PC_LoadSource( scriptName ) ) ) {
		pc_token_t	token;
		
		while( G_Q3F_SSCR_GetToken( scriptHandle, &token, NULL ) ) {
			if( !Q_stricmp( token.string, "speaker" ) ) {
					if( !G_Q3F_SSCR_ParseSpeaker( scriptHandle ) )
						break;
					numSpeakers++;
			} else {
				G_Q3F_SSCR_Error( scriptHandle, "Unexpected token '%s'", token.string );
				break;
			}
		}

		G_Printf( "Spawned %i speakers from %s\n", numSpeakers, scriptName );
    }
    
    return( qtrue );
}
