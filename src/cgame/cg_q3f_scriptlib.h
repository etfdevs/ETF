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

#ifndef __CG_Q3F_SCRIPTLIB_H
#define __CG_Q3F_SCRIPTLIB_H

#include "q_shared.h"
#include "cg_local.h"

//////////////////////////////////////////////////////
// Memory settings

#define HASH_TABLE_SIZE 2048

typedef struct stringDef_s {
	struct stringDef_s *next;
	const char *str;
} stringDef_t;

typedef struct {
	char *memPool;
	int allocPoint, outOfMemory;
	int memPoolSize;

	int strPoolIndex;
	char *strPool;
	int strPoolSize;

	int strHandleCount;
	stringDef_t *strHandle[HASH_TABLE_SIZE];
} memory_t;

enum {
	// Memory types
	MEM_F2R,
	MEM_SPIRIT,
	MEM_CGAME,

	MEM_NUMMEMS
};

//////////////////////////////////////////////////////

#define MAX_SCRIPT_ARGS 12

/*typedef struct {
	const char *command;
	const char *args[MAX_SCRIPT_ARGS];
} scriptDef_t;*/

void PC_SourceWarning(int handle, char *format, ...);
void PC_SourceError(int handle, char *format, ...);
const char *String_Alloc(const char *p);
void Memory_Init( const int memory );
void String_Init();
void String_Report();
void SetCurrentMemory( const int memory );
qboolean Float_Parse(char **p, float *f);
qboolean Color_Parse(char **p, vec4_t *c);
qboolean Int_Parse(char **p, int *i);
qboolean String_Parse(char **p, const char **out);
qboolean Script_Parse(char **p, const char **out);
qboolean PC_String_ParseNoAlloc(int handle, char *out, size_t size);
qboolean PC_Float_Parse(int handle, float *f);
qboolean PC_Vec_Parse(int handle, vec3_t *c);
qboolean PC_Color_Parse(int handle, vec4_t *c);
qboolean PC_Int_Parse(int handle, int *i);
qboolean PC_String_Parse(int handle, const char **out);
qboolean PC_String_ParseNoAlloc(int handle, char *out, size_t size);
qboolean PC_Script_Parse(int handle, const char **out);

void *UI_Alloc( int size );
qboolean UI_OutOfMemory();

int trap_PC_AddGlobalDefine( char *define );
int trap_PC_LoadSource( const char *filename );
int trap_PC_FreeSource( int handle );
int trap_PC_ReadToken( int handle, pc_token_t *pc_token );
int trap_PC_SourceFileAndLine( int handle, char *filename, int *line );

#endif
