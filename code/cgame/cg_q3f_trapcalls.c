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
