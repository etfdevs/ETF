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

#ifndef __CG_Q3F_SPIRIT_H
#define __CG_Q3F_SPIRIT_H

#include "tr_types.h"

#define SPIRIT_SCRIPT_LOOPEDSOUND	0x1

typedef struct SpiritScript_s {
	char filename[256];
	struct SpiritSystem_s *SpiritSystem;
	int flags;
	struct SpiritScript_s *link;
	qhandle_t sound;
} SpiritScript_t;

void InitParticles( void );
int Particle_Count(void);
void Spirit_AddParticles(void);
void Spirit_PrepareFrame(void);

void Spirit_SetCustomColor( const vec3_t color );
void Spirit_SetCustomShader( const qhandle_t shader );
void Spirit_RunModel( const SpiritScript_t *SpiritScript, const refEntity_t *re, const char * tagname, int key );
void Spirit_RunScript( const SpiritScript_t *SpiritScript, const vec3_t origin, const vec3_t oldorigin, const vec3_t axis[3], int key );
qboolean Spirit_UpdateScript( const SpiritScript_t *SpiritScript, const vec3_t origin, const vec3_t axis[3], int key );
qboolean Spirit_UpdateModel( const SpiritScript_t *SpiritScript, const refEntity_t *re, const char * tagname, int key );

SpiritScript_t *Spirit_LoadScript( const char *filename );
void Spirit_AddStandAlone( char * script,vec3_t origin, vec3_t dir, float rotation );

void Spirit_SetupKeywordHash( void );
int Spirit_ScriptCount( );
int Spirit_SystemCount( );
void Spirit_Reset( );
void Spirit_Reload( );

#endif
