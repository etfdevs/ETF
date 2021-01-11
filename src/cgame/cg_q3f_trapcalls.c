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

//static sfxHandle_t sfxErrHandle = NULL_SOUND;

void trap_R_AddPolyToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts ) {
	if( cg.drawFilter )
		return;

	trap_R_RealAddPolyToScene( hShader, numVerts, verts );
}

#ifdef API_ET
void trap_R_AddPolyBufferToScene( polyBuffer_t* pPolyBuffer ) {
	if( cg.drawFilter )
		return;

	trap_R_RealAddPolyBufferToScene( pPolyBuffer );
}
#endif

void trap_R_AddPolysToScene( qhandle_t hShader , int numVerts, const polyVert_t *verts, int numPolys ) {
	if( cg.drawFilter )
		return;

	trap_R_RealAddPolysToScene( hShader, numVerts, verts, numPolys );
}

void trap_R_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags ) {
	if( cg.drawFilter )
		return;

	trap_R_RealAddLightToScene( org, radius, intensity, r, g, b, hShader, flags );
}

#ifdef API_Q3
void trap_R_AddAdditiveLightToScene( const vec3_t org, float intensity, float r, float g, float b ) {
	if( cg.drawFilter )
		return;

	trap_R_RealAddAdditiveLightToScene( org, intensity, r, g, b );
}
#endif

sfxHandle_t	trap_S_RegisterSound( const char *sample, qboolean compressed ) {
	sfxHandle_t sfxHandle = 0;
	char	rawsoundname[1024];
	int		slnkHandle;

	COM_StripExtension( sample, rawsoundname, sizeof(rawsoundname) );
	Q_strcat(rawsoundname, sizeof(rawsoundname), ".slnk");
//	Com_sprintf( rawsoundname, sizeof(rawsoundname), "%s.slnk", rawsoundname );		// slothy - linux can't use same src & dest

	if( ( slnkHandle = trap_PC_LoadSource( rawsoundname ) ) != NULL_SOUND ) {
		pc_token_t token;

		if( !trap_PC_ReadToken( slnkHandle, &token ) ) {
			Com_Printf( "^3WARNING: Invalid soundlink '%s'\n", rawsoundname );
		} else {
			sfxHandle = trap_S_RealRegisterSound( token.string, compressed );
			if(!sfxHandle) {
				Com_Printf( "^3WARNING: soundlink '%s' contained invalid target '%s'\n", rawsoundname, token.string );	//slothy
			}
		}
		trap_PC_FreeSource( slnkHandle );
	}

	if( sfxHandle ) {
		return( sfxHandle );		// was redirected
	}
	else {
		sfxHandle = trap_S_RealRegisterSound( sample, compressed );
		return sfxHandle;		// original sample
	}
}
