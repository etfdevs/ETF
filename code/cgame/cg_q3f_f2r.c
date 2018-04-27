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

#define F2R_HASHSIZE	1024
#define MAX_LINKS		512

static F2RDef_t F2RScripts[MAX_F2R];			// defined F2R scripts
static F2RDef_t *F2RHash[F2R_HASHSIZE];			// For easy access to F2R
static F2RLink_t F2RLinkCache[MAX_LINKS];		// Linking F2R files to spirit systems

static int F2RCount = 0;						// how many 
static int F2RParseDepth = 0;					// it is possible that a model gets loaded inside an F2R script
static int F2RLinks = 0;						// how many

/*
===============
trap_R_AddRefEntityToScene
Check for linked SpiritScript for this model
===============
*/
#ifdef Q3F_WATER
void CG_Q3F_WaterPoke( const refEntity_t *re );
#endif // Q3F_WATER

void trap_R_AddRefEntityToScene( const refEntity_t *re, const centity_t *cent ) {
	if( cg.drawFilter )	{
		return;
	}

	if( re->renderfx & RF_NOSKYPORTAL && cg.renderingSkyPortal ) {
		return;
	}

	if( re->reType == RT_MODEL ) {
		F2RDef_t		*F2RScript;
		F2RLink_t		*F2RLink;

#ifdef Q3F_WATER
		CG_Q3F_WaterPoke( re );
#endif // Q3F_WATER

		F2RScript = F2R_GetForModel( re->hModel );
		if ( F2RScript ) {
			for (F2RLink = F2RScript->links; F2RLink ; F2RLink = F2RLink->next )
				Spirit_RunModel( F2RLink->SpiritScript, re, 
				F2RLink->tag, (int)cent );
		}
	}
	trap_R_RealAddRefEntityToScene( re );

	if( cgs.media.celshader && re->reType == RT_MODEL ) {
		qhandle_t realShader = re->customShader;

		((refEntity_t *)re)->customShader = cgs.media.celshader;
		trap_R_RealAddRefEntityToScene( re );
		((refEntity_t *)re)->customShader = realShader;
	}
}



void F2R_Init( F2RDef_t *F2RScript ) {
	memset(F2RScript, 0, sizeof(F2RDef_t));
}

void F2R_Reset( ) {
	F2RCount = 0;
	F2RLinks = 0;
	Memory_Init( MEM_F2R );
	SetCurrentMemory( MEM_F2R );			// slothy not sure this needs to be added?
	memset(F2RHash, 0, sizeof(F2RHash));
	memset(F2RLinkCache, 0 ,sizeof(F2RLinkCache));
}

/* CaNaBiS, This could probably go horribly wrong, but might still be useful */
void F2R_Reload( ) {
	int i, oldcount;
	qhandle_t model;
	char name[256];
	F2RDef_t *F2RScript;

	oldcount = F2R_Count();
	F2R_Reset();
	for (i=0 ; i < oldcount ; i++ ) {
		Q_strncpyz( name, F2RScripts[i].F2RFile, 256 );
		model = F2RScripts[i].model;
		F2RScript = Parse_F2RFile( name );
		if ( F2RScript != &F2RScripts[i] ) {
			CG_Error( "Reloading F2R File %s failed", name );
		}
		F2RScript->model = model;
		/* Add this script in the hash */
		F2RScript->next = F2RHash[F2RScript->model & (F2R_HASHSIZE-1)];
		F2RHash[F2RScript->model & (F2R_HASHSIZE-1)] = F2RScript;
	}
}

int F2R_Count( ) {
	return F2RCount + F2RParseDepth;
}

F2RDef_t *F2R_Get( int f2rnum ) {
	if( F2RParseDepth + F2RCount <= f2rnum || f2rnum < 0 )
		return NULL;
	else
		return &F2RScripts[f2rnum];
}

F2RDef_t *F2R_GetForModel( qhandle_t model ) {
	if( model ) {
		F2RDef_t *F2RScript = F2RHash[model & (F2R_HASHSIZE-1)];
		while (F2RScript) {
			if (F2RScript->model == model)
				return F2RScript;
			F2RScript = F2RScript->next;
		}
	}
	return NULL;
}

int F2R_NumGet( F2RDef_t *F2RScript ) {
	return( F2RScript - F2RScripts );
}

/*
===============
ParseF2RFile_Spirit

  Parses "spirit" section of F2R file
===============
*/
static qboolean ParseF2RFile_Spirit( const int handle, const char *F2RFile, F2RLink_t **StoreLink ) {
	pc_token_t token;

	if (!trap_PC_ReadToken( handle, &token )) {
		Com_Printf( "Corrupt spirit section in F2R file '%s'\n", F2RFile );
		return( qfalse );
	}

	if ( token.string[0] != '{' ) {
		Com_Printf( "Missing { in spirit section of F2R file '%s'\n", F2RFile );
		return( qfalse );
	}

	while( 1 ) {
		char			tagname[TAGLENGTH];
		SpiritScript_t	*SpiritScript;
		F2RLink_t		*Link;

		if (!trap_PC_ReadToken( handle, &token )) {
			Com_Printf( "Corrupt spirit section in F2R file '%s'\n", F2RFile );
			return( qfalse );
		}

		if ( token.string[0] == '}' )
			break;
		
		if (!trap_PC_ReadToken( handle, &token )) {
			Com_Printf( "Corrupt spirit section in F2R file '%s'\n", F2RFile );
			return( qfalse );
		}

		strncpy( tagname, token.string, TAGLENGTH );

		Com_Printf( "'%s' contains tagname: '%s'\n", F2RFile, tagname );

		if (!trap_PC_ReadToken( handle, &token ) ) {
			Com_Printf( "Corrupt spirit section in F2R file '%s'\n", F2RFile );
			return( qfalse );
		}

		SpiritScript = Spirit_LoadScript( token.string );

		Com_Printf( "'%s' contains spirit file: '%s'\n", F2RFile, token.string );
		
		if ( !SpiritScript ) {
			Com_Printf( "Failed to load spirit system %s for tag %s F2R file '%s'\n", token.string, tagname, F2RFile );
			return( qfalse );
		}

		if (F2RLinks >= MAX_LINKS) 
			return qfalse;

		Link = &F2RLinkCache[ F2RLinks++ ];
		strncpy(Link->tag , tagname, TAGLENGTH);
		Link->SpiritScript = SpiritScript;
		Link->next = *StoreLink;
		*StoreLink = Link;
	}

	return( qtrue );
}


/*
===============
ParseF2RFile_Animation

  Parses a single animation from an F2R file
===============
*/
static qboolean ParseF2RFile_Animation( const int handle, const char *F2RFile, F2RDef_t *F2RScript, const int animNumber ) {
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
	// force the allocation of an F2RScript here if we don't have it yet
	// we don't want to parse an F2R section yet though
	if( F2RScript->numAnims + 1 > ANI_NUM ) {
		Com_Printf( "More than %i animations in '%s'\n", ANI_NUM, F2RFile );
		return( qfalse );
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

	F2RScript->animations[animNumber-1] = animation;
	
	F2RScript->numAnims++;

	// parse the final spirit section

	F2RParseDepth++;		//Spirit script might load a new model

	if (ParseF2RFile_Spirit( handle, F2RFile, &F2RScript->links )) {
	
	F2RParseDepth--;

	};


	return( qtrue );
}

/*
===============
ParseF2RFile_Animations

  Parses "animations" section of F2R file
===============
*/
static qboolean ParseF2RFile_Animations( const int handle, const char *F2RFile, F2RDef_t *F2RScript  ) {
	pc_token_t token;

	if (!trap_PC_ReadToken( handle, &token )) {
		Com_Printf( "Corrupt animations section in F2R file '%s'\n", F2RFile );
		return( qfalse );
	}

	if ( token.string[0] != '{' ) {
		Com_Printf( "Missing { in animations section of F2R file '%s'\n", F2RFile );
		return( qfalse );
	}

	SetCurrentMemory( MEM_F2R );

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
	
	/* Allocate an f2r script, people shouldn't add empty f2r files */
	F2RScript = &F2RScripts[F2RParseDepth + F2RCount];
	if( F2R_Count()  < MAX_F2R ) {
		F2R_Init( F2RScript );
		Q_strncpyz( F2RScript->F2RFile, F2RFile, sizeof( F2RScript->F2RFile ) );
		F2RCount++;
	} else {
		CG_Error("ParseF2RFile: Failure. No F2RScript available" );
		return( NULL );			
	}

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

		if ( !Q_stricmp(token.string, "animations") ) {
			if( !ParseF2RFile_Animations( handle, F2RFile, F2RScript ) )
				break;
		} else if ( !Q_stricmp(token.string, "spirittags") ) {
			if( !ParseF2RFile_Spirit( handle, F2RFile, &F2RScript->links ) )
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

	if( F2RScript = Load_F2RFile( name ) ) {
		F2RScript->model = trap_R_RealRegisterModel( name );
		/* Add this script in the hash */
		F2RScript->next = F2RHash[F2RScript->model & (F2R_HASHSIZE-1)];
		F2RHash[F2RScript->model & (F2R_HASHSIZE-1)] =  F2RScript;
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
