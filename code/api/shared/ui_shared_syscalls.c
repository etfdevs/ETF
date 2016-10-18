void trap_R_AddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags ) {
#ifdef API_Q3
	Q_syscall( UI_R_ADDLIGHTTOSCENE, org, PASSFLOAT(radius), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b) );
#endif
#ifdef API_ET
	Q_syscall( UI_R_ADDLIGHTTOSCENE, org, PASSFLOAT( radius ), PASSFLOAT( intensity ),
		PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ), hShader, flags );
#endif
}

int trap_CM_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex ) {
#ifdef API_Q3
	return Q_syscall( UI_CM_LERPTAG, tag, refent->hModel, refent->oldframe, refent->frame, PASSFLOAT(1.0 - refent->backlerp), tagName );
#endif
#ifdef API_ET
	return Q_syscall( UI_CM_LERPTAG, tag, refent, tagName, 0 );			// NEFVE - SMF - fixed
#endif
}

void trap_S_StartBackgroundTrack( const char *intro, const char *loop, int fadeupTime ) {
#ifdef API_Q3
	Q_syscall( UI_S_STARTBACKGROUNDTRACK, intro, loop );
#endif
#ifdef API_ET
	Q_syscall( UI_S_STARTBACKGROUNDTRACK, intro, loop, fadeupTime );
#endif
}
