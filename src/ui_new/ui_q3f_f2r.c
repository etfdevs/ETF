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

#include "ui_local.h"

F2RDef_t F2RScripts[MAX_F2R];			// defined F2R scripts
int F2RCount = 0;						// how many
int F2RParseStartCount = 0;				// the F2RCount value at the start of a parse
int F2RParseDepth = 0;					// it is possible that a model gets loaded inside an F2R script

void PC_SourceError(int handle, char *format, ...);

/*
===============
Keyword Hash
===============
*/

#define F2RKEYWORDHASH_SIZE		512

typedef struct F2RKeywordHash_s {
	char *keyword;
	qboolean (*func)( F2RDef_t *F2RScript, int handle, int animNumber );
	struct F2RKeywordHash_s *next;
} F2RKeywordHash_t;

int F2R_KeywordHash_Key( char *keyword ) {
	int register hash, i;

	hash = 0;
	for (i = 0; keyword[i] != '\0'; i++) {
		if (keyword[i] >= 'A' && keyword[i] <= 'Z')
			hash += (keyword[i] + ('a' - 'A')) * (119 + i);
		else
			hash += keyword[i] * (119 + i);
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20)) & (F2RKEYWORDHASH_SIZE-1);
	return hash;
}

void F2R_KeywordHash_Add( F2RKeywordHash_t *table[], F2RKeywordHash_t *key ) {
	int hash;

	hash = F2R_KeywordHash_Key( key->keyword );
/*
	if (table[hash]) {
		int collision = qtrue;
	}
*/
	key->next = table[hash];
	table[hash] = key;
}

F2RKeywordHash_t *F2R_KeywordHash_Find( F2RKeywordHash_t *table[], char *keyword )
{
	F2RKeywordHash_t *key;
	int hash;

	hash = F2R_KeywordHash_Key( keyword );
	for ( key = table[hash]; key; key = key->next ) {
		if ( !Q_stricmp( key->keyword, keyword ) )
			return key;
	}
	return NULL;
}

/*
===============
F2R Keyword Parse functions
===============
*/

// tagemitter <string:tagname> <string:emittername> ( <parameter list> )
qboolean F2RParse_tagemitter( F2RDef_t *F2RScript, int handle, int animNumber ) {
	const char *tagname;
	const char *emittername;
	pc_token_t token;
//	SpiritDef_t *SpiritSystem;

	if ( !PC_String_Parse( handle, &tagname ) ) {
		return( qfalse );
	}

	if ( !PC_String_Parse( handle, &emittername ) ) {
		return( qfalse );
	}

//	SpiritSystem = Spirit_New( handle );

//	SetCurrentMemory( MEM_F2R );			// set it back to MEM_F2R, as Spirit_New set it to MEM_SPIRIT

	//if( Spirit_Count() < MAX_SPIRIT ) {	// if there is room to allocate a new spirit system
		//SpiritDef_t *SpiritSystem;

		//Spirit_New( handle );	
		//SpiritSystem = Spirit_Get( Spirit_Count() - 1 );
	
/*	if ( SpiritSystem ) {
		SpiritSystem->type = SPIRIT_EMITTER;
		SpiritSystem->tagname = tagname;
		SpiritSystem->emittername = emittername;
		SpiritSystem->animNumber = animNumber;
		SpiritSystem->F2RScript = F2RScript;
	} else {								// else skip this section (we do NOT want the parsing to fail in this case (with exeption of a bad script). Just drop the effect is the best tradeoff)*/
		while( 1 ) {
			if ( !trap_PC_ReadToken( handle, &token ) ) {
				PC_SourceError(handle, "end of file inside F2R-script");
				return( qfalse );
			}

			if (*token.string == ')') {
				return( qtrue );
			}
		}
//	}

	return( qtrue );
}

// tagspawn <int:framenum> <string:tagname> <string:emittername> ( <parameter list> )
qboolean F2RParse_tagspawn( F2RDef_t *F2RScript, int handle, int animNumber ) {
	int spawnframe;
	const char *tagname;
	const char *emittername;
	pc_token_t token;
//	SpiritDef_t *SpiritSystem;

	if ( !PC_Int_Parse( handle, &spawnframe ) ) {
		return( qfalse );
	}

	if ( !PC_String_Parse( handle, &tagname ) ) {
		return( qfalse );
	}

	if ( !PC_String_Parse( handle, &emittername ) ) {
		return( qfalse );
	}

//	SpiritSystem = Spirit_New( handle );

//	SetCurrentMemory( MEM_F2R );			// set it back to MEM_F2R, as Spirit_New set it to MEM_SPIRIT
	
/*	if ( SpiritSystem ) {
		SpiritSystem->type = SPIRIT_SPAWN;
		SpiritSystem->spawnframe = spawnframe;
		SpiritSystem->tagname = tagname;
		SpiritSystem->emittername = emittername;
		SpiritSystem->animNumber = animNumber;
		SpiritSystem->F2RScript = F2RScript;
	} else {								// else skip this section (we do NOT want the parsing to fail in this case (with exeption of a bad script). Just drop the effect is the best tradeoff)*/
		while( 1 ) {
			if ( !trap_PC_ReadToken( handle, &token ) ) {
				PC_SourceError(handle, "end of file inside F2R-script");
				return( qfalse );
			}

			if (*token.string == ')') {
				return( qtrue );
			}
		}
//	}

	return( qtrue );
}

F2RKeywordHash_t F2RParseKeywords[] = {
	{"tagemitter", F2RParse_tagemitter, NULL},
	{"tagspawn", F2RParse_tagspawn, NULL},
	{NULL, NULL, NULL}
};

F2RKeywordHash_t *F2RParseKeywordHash[F2RKEYWORDHASH_SIZE];

/*
===============
F2R_SetupKeywordHash
===============
*/
void F2R_SetupKeywordHash( void ) {
	int i;

	memset( F2RParseKeywordHash, 0, sizeof(F2RParseKeywordHash) );
	for ( i = 0; F2RParseKeywords[i].keyword; i++ ) {
		F2R_KeywordHash_Add( F2RParseKeywordHash, &F2RParseKeywords[i] );
	}
}

/*
===============
F2R_Parse
===============
*/
qboolean F2R_Parse( int handle, F2RDef_t *F2RScript, int animNumber ) {
	pc_token_t token;
	F2RKeywordHash_t *key;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;

	if (*token.string != '{') {
		return qfalse;
	}
    
	while ( 1 ) {

		memset(&token, 0, sizeof(pc_token_t));
		if (!trap_PC_ReadToken(handle, &token)) {
			PC_SourceError(handle, "end of file inside F2R-script");
			return qfalse;
		}

		if (*token.string == '}') {
			return qtrue;
		}

		key = F2R_KeywordHash_Find(F2RParseKeywordHash, token.string);
		if (!key) {
			PC_SourceError(handle, "unknown F2R keyword %s", token.string);
			continue;
		}
		if ( !key->func(F2RScript, handle, animNumber) ) {
			PC_SourceError(handle, "couldn't parse F2R keyword %s", token.string);
			return qfalse;
		}
	}

	// should never get here
	return qfalse;
}

/*
===============
F2R_Init
===============
*/
void F2R_Init( F2RDef_t *F2RScript ) {
	memset(F2RScript, 0, sizeof(F2RDef_t));
}

/*
===============
F2R_PostParse
===============
*/
void F2R_PostParse( F2RDef_t *F2RScript ) {
	if( F2RScript == NULL ) {
		return;
	}
}

/*
===============
F2R_New
===============
*/
F2RDef_t *F2R_New( int handle ) {
	F2RDef_t *F2RScript = &F2RScripts[F2RParseDepth + F2RCount];

	if( F2RParseDepth == 0 )
		F2RParseStartCount = F2RCount;

	F2RParseDepth++;

	if( F2RCount < MAX_F2R ) {
//		SetCurrentMemory( MEM_F2R );
		F2R_Init( F2RScript );
		if( F2R_Parse( handle, F2RScript, 0 ) ) {
			F2R_PostParse( F2RScript );
			F2RCount++;
			F2RParseDepth--;
			return( F2RScript );		
		}
	}
	
	// If we got here something went wrong
	F2RParseDepth--;
	return( NULL );	
}

/*
===============
F2R_Count
===============
*/
int F2R_Count( ) {
	return F2RCount;
}

F2RDef_t *F2R_Get( int f2rnum ) {
	if( F2RParseDepth + F2RCount <= f2rnum || f2rnum < 0 )
		return NULL;
	else
		return &F2RScripts[f2rnum];
}

F2RDef_t *F2R_GetForModel( qhandle_t model ) {
	if( model ) {
		int i;

		for( i = 0; i < F2RCount; i++ ) {
			if( model == F2RScripts[i].model ) {
				return( &F2RScripts[i] );
			}
		}
	}
	
	return NULL;
}

int F2R_NumGet( F2RDef_t *F2RScript ) {
	return( F2RScript - F2RScripts );
}

/*
===============
F2R_Reset
===============
*/
void F2R_Reset( ) {
	F2RCount = 0;
}

/*
===============
ParseF2RFile_Init

  Parses "init" section of F2R file
===============
*/
qboolean ParseF2RFile_Init( const int handle, const char *F2RFile, F2RDef_t **F2RScript ) {
	pc_token_t token;

	if (!trap_PC_ReadToken( handle, &token )) {
		Com_Printf( "Corrupt init section in F2R file '%s'\n", F2RFile );
		return( qfalse );
	}

	if ( token.string[0] != '{' ) {
		Com_Printf( "Missing { in init section of F2R file '%s'\n", F2RFile );
		return( qfalse );
	}

	while( 1 ) {
		if (!trap_PC_ReadToken( handle, &token )) {
			Com_Printf( "Corrupt init section in F2R file '%s'\n", F2RFile );
			return( qfalse );
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if( !Q_stricmp( token.string, "client" ) ) {
			if( !*F2RScript ) {
				// we don't have parsed any F2RScript sections for this F2RFile yet
				*F2RScript = F2R_New(handle);

				if( *F2RScript ) {
					Q_strncpyz( (*F2RScript)->F2RFile, F2RFile, sizeof( (*F2RScript)->F2RFile ) );
				}
			} else {
				// we already have a F2RScript for this F2RFile
				F2RParseDepth++;
				
//				SetCurrentMemory( MEM_F2R );

				if( F2R_Parse( handle, *F2RScript, 0 ) ) {
					F2R_PostParse( *F2RScript );
				}

				F2RParseDepth--;
			}
		}
	}

	return( qtrue );
}

/*
===============
ParseF2RFile_Animation

  Parses a single animation from an F2R file
===============
*/
qboolean ParseF2RFile_Animation( const int handle, const char *F2RFile, F2RDef_t **F2RScript, const int animNumber ) {
	int			firstFrame, numFrames, loopFrames, fps;
	qboolean	reversed, flipflop;
	animation_t	*animation;

	if ( !PC_Int_Parse( handle, &firstFrame ) ) {
		return( qfalse );
	}

	if ( !PC_Int_Parse( handle, &numFrames ) ) {
		return( qfalse );
	}

	if ( !PC_Int_Parse( handle, &loopFrames ) ) {
		return( qfalse );
	}

	if ( !PC_Int_Parse( handle, &fps ) ) {
		return( qfalse );
	}

	if ( !PC_Int_Parse( handle, (int*)&reversed ) ) {
		return( qfalse );
	}

	if ( !PC_Int_Parse( handle, (int*)&flipflop ) ) {
		return( qfalse );
	}

	// we got a complete animation
	// force the allocation of an F2RScript here if we don`'t have it yet
	// we don't want to parse an F2R section yet though
	if( *F2RScript ) {
		if( (*F2RScript)->numAnims + 1 > ANI_NUM ) {
			Com_Printf( "More than %i animations in '%s'\n", ANI_NUM, F2RFile );
			return( qfalse );
		}
	} else {
		*F2RScript = &F2RScripts[F2RParseDepth + F2RCount];
		
		if( F2RParseDepth == 0 )
			F2RParseStartCount = F2RCount;

		if( F2RCount < MAX_F2R ) {

			F2R_Init( *F2RScript );

			if( F2RScript ) {
				Q_strncpyz( (*F2RScript)->F2RFile, F2RFile, sizeof( (*F2RScript)->F2RFile ) );
				F2RCount++;
			} else {
				// FATAL
				trap_Error("ParseF2RFile_Animation: Failure. No F2RScript available for animations" );
				return( qfalse );
			}
		} else {
			// FATAL
			trap_Error("ParseF2RFile_Animation: Failure. No F2RScript available for animations" );
			return( qfalse );			
		}
	}

	// allocate animation_t and add to F2RScript
	animation = (animation_t *)UI_Alloc( sizeof(animation_t) );

	// init animation
	animation->animNumber = animNumber;
	animation->firstFrame = firstFrame;
	animation->numFrames = numFrames;
	animation->loopFrames = loopFrames;
	animation->frameLerp = animation->initialLerp = 1000 / (float)fps;
	animation->reversed = reversed;
	animation->flipflop = flipflop;

	(*F2RScript)->animations[animNumber-1] = animation;
	
	(*F2RScript)->numAnims++;

	// parse our F2R section
	F2RParseDepth++;
				
//	SetCurrentMemory( MEM_F2R );

	if( F2R_Parse( handle, *F2RScript, animNumber ) ) {
		F2R_PostParse( *F2RScript );
	}

	F2RParseDepth--;
	
	return( qtrue );
}

/*
===============
ParseF2RFile_Animations

  Parses "animations" section of F2R file
===============
*/
qboolean ParseF2RFile_Animations( const int handle, const char *F2RFile, F2RDef_t **F2RScript  ) {
	pc_token_t token;

	if (!trap_PC_ReadToken( handle, &token )) {
		Com_Printf( "Corrupt animations section in F2R file '%s'\n", F2RFile );
		return( qfalse );
	}

	if ( token.string[0] != '{' ) {
		Com_Printf( "Missing { in animations section of F2R file '%s'\n", F2RFile );
		return( qfalse );
	}

	while( 1 ) {
		if (!trap_PC_ReadToken( handle, &token )) {
			Com_Printf( "Corrupt animations section in F2R file '%s'\n", F2RFile );
			return( qfalse );
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if ( token.type != TT_NUMBER ) {
			PC_SourceError(handle, "expected integer but found %s", token.string);
			return( qfalse );
		}

		if( !ParseF2RFile_Animation( handle, F2RFile, F2RScript, token.intvalue ) ) {
			return( qfalse );
		}
	}

	return( qtrue );
}

/*
===============
ParseF2RFile
	FIXME: this should always return qfalse if it doesn't parse properly, it isn't doing now
		example: includefile missing/notfound bombs out and results in crash
	TESTME: should be fixed now
===============
*/
F2RDef_t *Parse_F2RFile( const char *F2RFile ) {
	pc_token_t token;
	int handle;
	F2RDef_t *F2RScript = NULL; 

	handle = trap_PC_LoadSource( F2RFile );

	if (!handle)
		return( NULL );

	while ( 1 ) {
		if (!trap_PC_ReadToken( handle, &token )) {
			if( !F2RScript ) {
				Com_Printf( "No valid script found in F2R file '%s'\n", F2RFile );
				return( NULL );							
			}
			break;
		}

		if ( token.string[0] == '}' ) {
			break;
		}

		if ( !Q_stricmp( token.string, "init" ) ) {
			if( !ParseF2RFile_Init( handle, F2RFile, &F2RScript ) )
				break;

		} else if ( !Q_stricmp(token.string, "animations") ) {
			if( !ParseF2RFile_Animations( handle, F2RFile, &F2RScript ) )
				break;
		}
			
	} 
	trap_PC_FreeSource( handle );

	return( F2RScript );
}

/*
===============
Load_F2RFile
===============
*/
F2RDef_t *Load_F2RFile( const char *modelFile ) {
	char F2RFile[256];
	int i;

	COM_StripExtension( modelFile, F2RFile, sizeof(F2RFile) );

	Q_strcat( F2RFile, sizeof(F2RFile), ".f2r" );

	// See if we already parsed this model once
	// in case a model gets registered multiple times, this saves us some parsing
	for( i = 0; i < F2RCount + F2RParseDepth; i++ ) {
		if( !Q_stricmp( F2R_Get( i )->F2RFile, F2RFile ) ) {
			return( NULL );
		}
	}

	return( Parse_F2RFile( F2RFile ) );
}

/*
===============
trap_R_RegisterModel
===============
*/
qhandle_t trap_R_RegisterModel( const char *name ) {
	F2RDef_t *F2RScript;

	if( ( F2RScript = Load_F2RFile( name ) ) != NULL ) {
		F2RScript->model = trap_R_RealRegisterModel( name );
		return( F2RScript->model );
	} else {
		return( trap_R_RealRegisterModel( name ) );
	}
}

/*
===============
Load_StandAloneF2RFile

  This loads an F2R file and returns a pointer to it
===============
*/
F2RDef_t *Load_StandAloneF2RFile( const char *F2RFile ) {
	return( Parse_F2RFile( F2RFile ) );
}
