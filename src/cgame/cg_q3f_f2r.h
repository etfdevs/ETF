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

#ifndef __CG_Q3F_F2R_H
#define __CG_Q3F_F2R_H

#define MAX_F2R 196

#define TAGLENGTH	40
#ifdef CGAME
typedef struct F2RLink_s {
	SpiritScript_t		*SpiritScript;
	int					key;
	char				tag[TAGLENGTH];
	struct F2RLink_s	*next;
} F2RLink_t;
#endif

typedef struct F2RDef_s {
	char			F2RFile[256];
	qhandle_t		model;
#ifdef CGAME
	F2RLink_t		*links;
#endif
	animation_t		*animations[ANI_NUM];	// dynamically allocated pool
	int				numAnims;
	struct F2RDef_s	*next;
} F2RDef_t;

F2RDef_t *F2R_New(int handle);
int F2R_Count( );
F2RDef_t *F2R_Get( int f2rnum );
F2RDef_t *F2R_GetForModel( qhandle_t model );
int F2R_NumGet( F2RDef_t *F2RScript );
void F2R_Reset( );
void F2R_Reload( );

F2RDef_t *Parse_F2RFile( const char *F2RFile );
F2RDef_t *Load_F2RFile( const char *modelFile );
qhandle_t trap_R_RegisterModel( const char *name );			// RR2DO2: this is a wrapper function around trap_R_RealRegisterModel now

#endif
