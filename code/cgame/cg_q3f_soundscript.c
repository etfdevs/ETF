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
#include "cg_q3f_scriptlib.h"
#include "../ui_new/ui_shared.h"

// Defines

typedef struct cg_q3f_sscr_speaker_s {
	char		noise[256];
	int			wait;
	int			random;
	vec3_t		origin;
	int			gameindex;
} cg_q3f_speaker_t;

#define MAX_SCRIPTSPEAKERS 256

static cg_q3f_speaker_t scriptSpeakers[MAX_SCRIPTSPEAKERS];
static int numScriptSpeakers;

void CG_ClearScriptSpeakerPool( void ) {
	numScriptSpeakers = 0;
}

int CG_NumScriptSpeakers( void ) {
	return numScriptSpeakers;
}

int CG_GetIndexForSpeaker( cg_q3f_speaker_t *speaker ) {
	return speaker - scriptSpeakers;
}

cg_q3f_speaker_t *CG_GetScriptSpeaker( int index ) {
	if ( index < 0 || index >= numScriptSpeakers ) {
		return NULL;
	}

	return &scriptSpeakers[ index ];
}

qboolean CG_SS_DeleteSpeaker( int index ) {
	if ( index < 0 || index >= numScriptSpeakers ) {
		return qfalse;
	}

	memcpy( &scriptSpeakers[ index ], &scriptSpeakers[ index + 1 ], sizeof( cg_q3f_speaker_t ) * ( numScriptSpeakers - index - 1 ) );

	numScriptSpeakers--;

	return qtrue;
}

qboolean CG_SS_StoreSpeaker( cg_q3f_speaker_t *speaker ) {
	if ( numScriptSpeakers >= MAX_SCRIPTSPEAKERS ) {
		return qfalse;
	}

	memcpy( &scriptSpeakers[ numScriptSpeakers++ ], speaker, sizeof( cg_q3f_speaker_t ) );

	return qtrue;
}

// Support functions

static void CG_SS_ParseError( int handle, char *format, ... ) {
    // Stop with an error, showing file and line
    
    va_list     argptr;
    char	    filename[128];
    int		    line;
	static char string[4096];
    
	va_start( argptr, format );
	Q_vsnprintf( string, sizeof( string ), format, argptr );
	va_end( argptr );

	filename[0] = '\0';
	line = 0;
	trap_PC_SourceFileAndLine( handle, filename, &line );
    
    Com_Printf( S_COLOR_RED "ERROR: %s, line %d: %s\n", filename, line, string );

	trap_PC_FreeSource( handle );
}

static qboolean CG_SS_ParseSpeaker( int handle ) {
	pc_token_t token;
	cg_q3f_speaker_t speaker;

	memset( &speaker, 0, sizeof( speaker ) );

	if ( !trap_PC_ReadToken( handle, &token ) || Q_stricmp( token.string, "{" ) ) {
		return CG_SS_ParseError( handle, "expected '{'" );
	}

	while ( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if ( !Q_stricmp( token.string, "noise" ) ) {
			if ( !PC_String_ParseNoAlloc( handle, speaker.filename, sizeof( speaker.filename ) ) ) {
				return CG_SS_ParseError( handle, "expected sound filename" );
			}
		} else if ( !Q_stricmp( token.string, "origin" ) ) {
			if ( !PC_Vec_Parse( handle, &speaker.origin ) ) {
				return CG_SS_ParseError( handle, "expected origin vector" );
			}
		} else if ( !Q_stricmp( token.string, "wait" ) ) {
			if ( !PC_Int_Parse( handle, &speaker.wait ) ) {
				return CG_SS_ParseError( handle, "expected wait value" );
			} else if ( speaker.wait < 0 ) {
				return CG_SS_ParseError( handle, "wait value %i is invalid", speaker.wait );
			}
		} else if ( !Q_stricmp( token.string, "random" ) ) {
			if ( !PC_Int_Parse( handle, &speaker.random ) ) {
				return CG_SS_ParseError( handle, "expected random value" );
			} else if ( speaker.random < 0 ) {
				return CG_SS_ParseError( handle, "random value %i is invalid", speaker.random );
			}
		} else if( !Q_stricmp( token.string, "gameindex" ) ) {
			if ( !PC_Int_Parse( handle, &speaker.gameindex ) ) {
				return CG_SS_ParseError( handle, "expected gameindex value" );
			} else if ( speaker.gameindex < 0 ) {
				return CG_SS_ParseError( handle, "gameindex value %i is invalid", speaker.gameindex );
			}
		} else {
			return CG_SS_ParseError( handle, "unknown token '%s'", token.string );
		}
	}

	if ( !CG_SS_StoreSpeaker( &speaker ) ) {
		return CG_SS_ParseError( handle, "Failed to store speaker", token.string );
	}

	return qtrue;
}

static qboolean CG_SS_ParseOldSpeaker( int handle ) {
	pc_token_t token;
	cg_q3f_speaker_t speaker;

	memset( &speaker, 0, sizeof( speaker ) );

	if ( !trap_PC_ReadToken( handle, &token ) || Q_stricmp( token.string, "speaker" ) ) {
		return CG_SS_ParseError( handle, "expected 'speaker'" );
	}

	if ( !trap_PC_ReadToken( handle, &token ) || Q_stricmp( token.string, "{" ) ) {
		return CG_SS_ParseError( handle, "expected '{'" );
	}

	while ( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if ( !Q_stricmp( token.string, "noise" ) ) {
			if ( !PC_String_ParseNoAlloc( handle, speaker.filename, sizeof( speaker.filename ) ) ) {
				return CG_SS_ParseError( handle, "expected sound filename" );
			}
		} else if ( !Q_stricmp( token.string, "origin" ) ) {
			if ( !PC_Vec_Parse( handle, &speaker.origin ) ) {
				return CG_SS_ParseError( handle, "expected origin vector" );
			}
		} else if ( !Q_stricmp( token.string, "wait" ) ) {
			if ( !PC_Int_Parse( handle, &speaker.wait ) ) {
				return CG_SS_ParseError( handle, "expected wait value" );
			} else if ( speaker.wait < 0 ) {
				return CG_SS_ParseError( handle, "wait value %i is invalid", speaker.wait );
			}
		} else if ( !Q_stricmp( token.string, "random" ) ) {
			if ( !PC_Int_Parse( handle, &speaker.random ) ) {
				return CG_SS_ParseError( handle, "expected random value" );
			} else if ( speaker.random < 0 ) {
				return CG_SS_ParseError( handle, "random value %i is invalid", speaker.random );
			}
		} else if( !Q_stricmp( token.string, "gameindex" ) ) {
			if ( !PC_Int_Parse( handle, &speaker.gameindex ) ) {
				return CG_SS_ParseError( handle, "expected gameindex value" );
			} else if ( speaker.gameindex < 0 ) {
				return CG_SS_ParseError( handle, "gameindex value %i is invalid", speaker.gameindex );
			}
		} else {
			return CG_SS_ParseError( handle, "unknown token '%s'", token.string );
		}
	}

	if ( !CG_SS_StoreSpeaker( &speaker ) ) {
		return CG_SS_ParseError( handle, "Failed to store speaker", token.string );
	}

	return qtrue;
}

qboolean CG_LoadSpeakerScript( const char *filename ) {
	pc_token_t token;
	int handle;

	handle = trap_PC_LoadSource( filename );

	if ( !handle ) {
		return qfalse;
	}

	if ( !trap_PC_ReadToken( handle, &token ) /*|| Q_stricmp( token.string, "speakerScript" )*/) {
		if ( !Q_stricmp( token.string, "speaker" ) ) {
			trap_PC_UnReadToken( handle );
			return
			// Run old script parser
		} else if ( !Q_stricmp( token.string, "speakerScript" ) ) {
			// Run new script parser
		} else {
			return CG_SS_ParseError( handle, "expected 'soundScript'" );
		}
	}

	if ( !trap_PC_ReadToken( handle, &token ) || Q_stricmp( token.string, "{" ) ) {
		return CG_SS_ParseError( handle, "expected '{'" );
	}

	while ( 1 ) {
		if ( !trap_PC_ReadToken( handle, &token ) ) {
			break;
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if ( !Q_stricmp( token.string, "speakerDef" ) ) {
			if ( !CG_SS_ParseSpeaker( handle ) ) {
				return qfalse;
			}
		} else {
			return CG_SS_ParseError( handle, "unknown token '%s'", token.string );
		}
	}

	trap_PC_FreeSource( handle );

	return qtrue;
}

#if 0

// Parsing functions
static qboolean CG_Q3F_SSCR_ParseSpeaker( int scriptHandle ) {
    pc_token_t			token;
    cg_q3f_sscr_speaker_t	speaker;
    gentity_t			*ent;
    
    if( !CG_Q3F_SSCR_GetToken( scriptHandle, &token, "{" ) )
		return( qfalse );
	
    memset( &speaker, 0, sizeof(speaker) );
    speaker.looped = qtrue;
	
    while( CG_Q3F_SSCR_GetToken( scriptHandle, &token, NULL ) ) {
		if( !Q_stricmp( token.string, "noise" ) ) {
			if( !CG_Q3F_SSCR_GetToken( scriptHandle, &token, NULL ) )
				return( qfalse );

			Q_strncpyz(	speaker.noise, token.string, sizeof(speaker.noise) );
//		} else if( !Q_stricmp( token.string, "broadcast" ) ) {
//			speaker.broadcast = qtrue;
		} else if( !Q_stricmp( token.string, "wait" ) ) {
			if( !CG_Q3F_SSCR_GetIntegerToken( scriptHandle, &speaker.wait ) )
				return( qfalse );
			
			speaker.looped = qfalse;
		} else if( !Q_stricmp( token.string, "random" ) ) {
			if( !CG_Q3F_SSCR_GetIntegerToken( scriptHandle, &speaker.random ) )
				return( qfalse );
			
			speaker.looped = qfalse;
		} else if( !Q_stricmp( token.string, "origin" ) ) {
			if( !CG_Q3F_SSCR_GetVec( scriptHandle, &speaker.origin ) )
				return( qfalse );
		} else if( !Q_stricmp( token.string, "}" ) ) {
			break;
		} else {
			CG_Q3F_SSCR_Error( scriptHandle, "Unexpected token '%s'", token.string );
		}
	}

	if ( !CG_SS_StoreSpeaker( &speaker ) ) {
		return CG_SS_ParseError( handle, "Failed to store speaker", token.string );
	}

    return( qtrue );
}

// This attempts to load a <mapname>.sscr file. If it succeeds
// it spawns a number of soundentities from the descriptors in the
// file. These act like primitive target_speaker entities, in the 
// way that they can't be toggled, but do have all the looping/random
// capabilities.

qboolean CG_Q3F_SSCR_ParseSoundScript( char *mapname ) {
    char	*scriptName, rawmapname[1024];
    int		scriptHandle;
	int		numSpeakers = 0;

	COM_StripExtension( COM_SkipPath( mapname ), rawmapname );
	scriptName = va( "%s.sscr", rawmapname );

	if ( !trap_PC_ReadToken( handle, &token ) Q_stricmp( token.string, "speakerScript" ) ) {
		if ( !Q_stricmp( token.string, "speaker" ) ) {
			return CG_SS_LoadOldSpeakerScript( handle );
		}
		return CG_SS_ParseError( handle, "expected 'soundScript'" );
	}

	if ( !trap_PC_ReadToken( handle, &token ) || Q_stricmp( token.string, "{" ) ) {
		return CG_SS_ParseError( handle, "expected '{'" );
	}
    
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

#endif
