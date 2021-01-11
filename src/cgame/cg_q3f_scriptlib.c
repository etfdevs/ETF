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

// 
// string allocation/managment

#include "cg_local.h"
#include "cg_q3f_scriptlib.h"
#include "../ui_new/ui_shared.h"

//////////////////////////////////
// Memory Handling

static const char *memNames[] = {
	"F2R",
	"Spirit",
	"CGame"
};

static memory_t memories[MEM_NUMMEMS];

static memory_t *currentMemory;
static int currentMemoryIndex;

// Memory Pool
#define F2R_MP_SIZE		32*1024
#define SPIRIT_MP_SIZE	64*1024
#define CGAME_MP_SIZE	4096*1024

static int memPoolSizes[] = {
	F2R_MP_SIZE,
	SPIRIT_MP_SIZE,
	CGAME_MP_SIZE
};

static char memPool_F2R[F2R_MP_SIZE];
static char memPool_SPIRIT[SPIRIT_MP_SIZE];
static char memPool_CGAME[CGAME_MP_SIZE];

static char *memPools[] = {
	memPool_F2R,
	memPool_SPIRIT,
	memPool_CGAME
};


// String Pool
#define F2R_SP_SIZE		64*1024
#define SPIRIT_SP_SIZE	64*1024
#define CGAME_SP_SIZE	2048*1024

static int stringPoolSizes[] = {
	F2R_SP_SIZE,
	SPIRIT_SP_SIZE,
	CGAME_SP_SIZE
};

static char stringPool_F2R[F2R_SP_SIZE];
static char stringPool_SPIRIT[SPIRIT_SP_SIZE];
static char stringPool_CGAME[SPIRIT_SP_SIZE];

static char *stringPools[] = {
	stringPool_F2R,
	stringPool_SPIRIT,
	stringPool_CGAME
};

/*
===============
UI_Alloc
===============
*/
void *UI_Alloc( int size ) {
	char	*p; 

	if ( currentMemory->allocPoint + size > currentMemory->memPoolSize ) {
		currentMemory->outOfMemory = qtrue;
		CG_Error("UI_Alloc: Failure. Memory Pool \'%s\' Out of memory!", memNames[currentMemoryIndex] );
		return NULL;
	}

	p = &currentMemory->memPool[currentMemory->allocPoint];

	currentMemory->allocPoint += ( size + 15 ) & ~15;

	return p;
}

qboolean UI_OutOfMemory() {
	return currentMemory->outOfMemory;
}

//#define HASH_TABLE_SIZE 2048
/*
================
return a hash value for the string
================
*/
static unsigned hashForString(const char *str) {
	int		i;
	unsigned	hash;
	char	letter;

	hash = 0;
	i = 0;
	while (str[i] != '\0') {
		letter = tolower(str[i]);
		hash+=(unsigned)(letter)*(i+119);
		i++;
	}
	hash &= (HASH_TABLE_SIZE-1);
	return hash;
}

/*typedef struct stringDef_s {
	struct stringDef_s *next;
	const char *str;
} stringDef_t;

static int strPoolIndex = 0;
static char strPool[STRING_POOL_SIZE];

static int strHandleCount = 0;
static stringDef_t *strHandle[HASH_TABLE_SIZE];*/


const char *String_Alloc(const char *p) {
	int len;
	unsigned hash;
	stringDef_t *str, *last;
	static const char *staticNULL = "";

	if (p == NULL) {
		return NULL;
	}

	if (*p == 0) {
		return staticNULL;
	}

	hash = hashForString(p);

	str = currentMemory->strHandle[hash];
	while (str) {
		if (strcmp(p, str->str) == 0) {
			return str->str;
		}
		str = str->next;
	}

	len = strlen(p);
	if (len + currentMemory->strPoolIndex + 1 < currentMemory->strPoolSize) {
		int ph = currentMemory->strPoolIndex;
		strcpy(&currentMemory->strPool[currentMemory->strPoolIndex], p);
		currentMemory->strPoolIndex += len + 1;

		str = currentMemory->strHandle[hash];
		last = str;
		while (str && str->next) {
			last = str;
			str = str->next;
		}

		str  = UI_Alloc(sizeof(stringDef_t));
		str->next = NULL;
		str->str = &currentMemory->strPool[ph];
		if (last) {
			last->next = str;
		} else {
			currentMemory->strHandle[hash] = str;
		}
		return &currentMemory->strPool[ph];
	}
	return NULL;
}

void String_Report() {
	float f;
	int i;
	Com_Printf("Memory/String Pool Info\n");
	Com_Printf("----------------\n");
	for( i = 0; i < MEM_NUMMEMS; i++ ) {
		SetCurrentMemory( i );
		f = currentMemory->strPoolIndex;
		f /= currentMemory->strPoolSize;
		f *= 100;
		Com_Printf("String Pool \'%s\' is %.1f%% full, %i bytes out of %i used.\n", memNames[i], f, currentMemory->strPoolIndex, currentMemory->strPoolSize);
		f = currentMemory->allocPoint;
		f /= currentMemory->memPoolSize;
		f *= 100;
		Com_Printf("Memory Pool \'%s\' is %.1f%% full, %i bytes out of %i used.\n", memNames[i], f, currentMemory->allocPoint, currentMemory->memPoolSize);
	}
}

/*
=================
SetCurrentMemory
=================
*/
void SetCurrentMemory( const int memory ) {
	if ( memory < 0 || memory >= MEM_NUMMEMS )
		return;

	currentMemory = &memories[memory];
	currentMemoryIndex = memory;
}


/*
=================
StringPool_Init

  Initializes a stringpool
=================
*/
static void StringPool_Init( memory_t *memory, const int size, char *strPool ) {
	int i;

	memory->strPoolSize = size;
	memory->strPool = strPool;

	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		memory->strHandle[i] = 0;
	}

	memory->strHandleCount = 0;
	memory->strPoolIndex = 0;
}


/*
=================
MemPool_Init

  Initializes a memorypool
=================
*/
static void MemPool_Init( memory_t *memory, const int size, char *memPool ) {
	memory->memPoolSize = size;
	memory->memPool = memPool;

	memory->allocPoint = 0;
	memory->outOfMemory = qfalse;
}

/*
=================
String_Init
=================
*/
void String_Init() {
	int i;

	// Initializing memory
	for( i = 0; i < MEM_NUMMEMS; i++ ) {
		Memory_Init( i );
	}

	// Initializing keyword hashes
	Spirit_SetupKeywordHash();

	CG_Menu_Init();
}

/*
=================
Memory_Init

  Initializes one string and memory pool
=================
*/
void Memory_Init( const int memory ) {
	if ( memory < 0 || memory >= MEM_NUMMEMS )
		return;

	// Initializing stringpool and memory

	SetCurrentMemory( memory );
	StringPool_Init( currentMemory, stringPoolSizes[memory], stringPools[memory] );
	MemPool_Init( currentMemory, memPoolSizes[memory], memPools[memory] );
}

/*
=================
PC_SourceWarning
=================
*/
/*void PC_SourceWarning(int handle, char *format, ...) {
	int line;
	char filename[128];
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	vsprintf (string, format, argptr);
	Q_vsnprintf (string, sizeof(string), format, argptr);
	va_end (argptr);

	filename[0] = '\0';
	line = 0;
	trap_PC_SourceFileAndLine(handle, filename, &line);

	Com_Printf(S_COLOR_YELLOW "WARNING: %s, line %d: %s\n", filename, line, string);
}*/

/*
=================
PC_SourceError
=================
*/
/*void PC_SourceError(int handle, char *format, ...) {
	int line;
	char filename[128];
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	vsprintf (string, format, argptr);
	va_end (argptr);

	filename[0] = '\0';
	line = 0;
	trap_PC_SourceFileAndLine(handle, filename, &line);

	Com_Printf(S_COLOR_RED "ERROR: %s, line %d: %s\n", filename, line, string);
}*/

/*
=================
Float_Parse
=================
*/
/*qboolean Float_Parse(char **p, float *f) {
	char	*token;
	token = COM_ParseExt(p, qfalse);
	if (token && token[0] != 0) {
		*f = atof(token);
		return qtrue;
	} else {
		return qfalse;
	}
}*/

/*
=================
PC_Float_Parse
=================
*/
/*qboolean PC_Float_Parse(int handle, float *f) {
	pc_token_t token;
	int negative = qfalse;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (token.string[0] == '-') {
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;
		negative = qtrue;
	}
	if (token.type != TT_NUMBER) {
		PC_SourceError(handle, "expected float but found %s", token.string);
		return qfalse;
	}
	if (negative)
		*f = -token.floatvalue;
	else
		*f = token.floatvalue;
	return qtrue;
}*/

/*
=================
Vec_Parse
=================
*/
qboolean Vec_Parse(char **p, vec3_t *c) {
	int i;
	float f;

	for (i = 0; i < 3; i++) {
		if (!Float_Parse(p, &f)) {
			return qfalse;
		}
		(*c)[i] = f;
	}
	return qtrue;
}

/*
=================
PC_Vec_Parse
=================
*/
qboolean PC_Vec_Parse(int handle, vec3_t *c) {
	int i;
	float f;

	for (i = 0; i < 3; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		(*c)[i] = f;
	}
	return qtrue;
}

/*
=================
Color_Parse
=================
*/
/*qboolean Color_Parse(char **p, vec4_t *c) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!Float_Parse(p, &f)) {
			return qfalse;
		}
		(*c)[i] = f;
	}
	return qtrue;
}*/

/*
=================
PC_Color_Parse
=================
*/
/*qboolean PC_Color_Parse(int handle, vec4_t *c) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		(*c)[i] = f;
	}
	return qtrue;
}*/

/*
=================
Int_Parse
=================
*/
/*qboolean Int_Parse(char **p, int *i) {
	char	*token;
	token = COM_ParseExt(p, qfalse);

	if (token && token[0] != 0) {
		*i = atoi(token);
		return qtrue;
	} else {
		return qfalse;
	}
}*/

/*
=================
PC_Int_Parse
=================
*/
/*qboolean PC_Int_Parse(int handle, int *i) {
	pc_token_t token;
	int negative = qfalse;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (token.string[0] == '-') {
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;
		negative = qtrue;
	}
	if (token.type != TT_NUMBER) {
		PC_SourceError(handle, "expected integer but found %s", token.string);
		return qfalse;
	}
	*i = token.intvalue;
	if (negative)
		*i = - *i;
	return qtrue;
}*/

/*
=================
String_Parse
=================
*/
/*qboolean String_Parse(char **p, const char **out) {
	char *token;

	token = COM_ParseExt(p, qfalse);
	if (token && token[0] != 0) {
		*(out) = String_Alloc(token);
		return qtrue;
	}
	return qfalse;
}*/

/*
=================
PC_String_Parse
=================
*/
/*qboolean PC_String_Parse(int handle, const char **out) {
	pc_token_t token;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	
	*(out) = String_Alloc(token.string);
    return qtrue;
}*/

/*
=================
PC_String_ParseNoAlloc

Same as one above, but uses a static buff and not the string memory pool
=================
*/
/*
qboolean PC_String_ParseNoAlloc(int handle, char *out) {
	pc_token_t token;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	
	Q_strncpyz( out, token.string, sizeof( out ) );
    return qtrue;
}
*/

/*
=================
PC_Script_Parse
=================
*/
/*qboolean PC_Script_Parse(int handle, const char **out) {
	char script[1024];
	pc_token_t token;

	memset(script, 0, sizeof(script));
	// scripts start with { and have ; separated command lists.. commands are command, arg.. 
	// basically we want everything between the { } as it will be interpreted at run time
  
	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (Q_stricmp(token.string, "{") != 0) {
	    return qfalse;
	}

	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;

		if (Q_stricmp(token.string, "}") == 0) {
			*out = String_Alloc(script);
			return qtrue;
		}

		if (token.string[1] != '\0') {
			Q_strcat(script, 1024, va("\"%s\"", token.string));
		} else {
			Q_strcat(script, 1024, token.string);
		}
		Q_strcat(script, 1024, " ");
	}
	//return qfalse;
}*/

