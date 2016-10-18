void trap_R_RealAddLightToScene( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags ) {
#ifdef API_Q3
	Q_syscall( CG_R_ADDLIGHTTOSCENE, org, PASSFLOAT(radius), PASSFLOAT(r), PASSFLOAT(g), PASSFLOAT(b) );
#endif
#ifdef API_ET
	Q_syscall( CG_R_ADDLIGHTTOSCENE, org, PASSFLOAT( radius ), PASSFLOAT( intensity ),
		PASSFLOAT( r ), PASSFLOAT( g ), PASSFLOAT( b ), hShader, flags );
#endif
}

int trap_R_LerpTag( orientation_t *tag, const refEntity_t *refent, const char *tagName, int startIndex ) {
#ifdef API_Q3
	float backLerp = 1.f - refent->backlerp;
	return Q_syscall( CG_R_LERPTAG, tag, refent->hModel, refent->oldframe, refent->frame, PASSFLOAT(backLerp), tagName );
#endif
#ifdef API_ET
	return Q_syscall( CG_R_LERPTAG, tag, refent, tagName, startIndex );
#endif
}

void trap_S_ClearLoopingSounds( qboolean killall ) {
#ifdef API_Q3
	Q_syscall( CG_S_CLEARLOOPINGSOUNDS, killall );
#endif
#ifdef API_ET
	Q_syscall( CG_S_CLEARLOOPINGSOUNDS );
#endif
}

void trap_S_AddLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int volume, int soundTime ) {
#ifdef API_Q3
#ifdef DEBUGREG
	cgs.registeredSoundCounts[(int) sfx]++;
#endif
	Q_syscall( CG_S_ADDLOOPINGSOUND, entityNum, origin, velocity, sfx );
#endif
#ifdef API_ET
	Q_syscall( CG_S_ADDLOOPINGSOUND, origin, velocity, 1250, sfx, volume, soundTime );		// volume was previously removed from CG_S_ADDLOOPINGSOUND.  I added 'range'
#endif
}

void trap_S_AddRealLoopingSound( int entityNum, const vec3_t origin, const vec3_t velocity, sfxHandle_t sfx, int range, int volume, int soundTime ) {
#ifdef API_Q3
#ifdef DEBUGREG
	cgs.registeredSoundCounts[(int) sfx]++;
#endif
	Q_syscall( CG_S_ADDREALLOOPINGSOUND, entityNum, origin, velocity, sfx );
#endif
#ifdef API_ET
	Q_syscall( CG_S_ADDREALLOOPINGSOUND, origin, velocity, range, sfx, volume, soundTime );
#endif
}

void trap_S_StartBackgroundTrack( const char *intro, const char *loop, int fadeupTime ) {
#ifdef API_Q3
	Q_syscall( CG_S_STARTBACKGROUNDTRACK, intro, loop );
#endif
#ifdef API_ET
	Q_syscall( CG_S_STARTBACKGROUNDTRACK, intro, loop, fadeupTime );
#endif
}

void trap_SetUserCmdValue( int stateValue, int flags, float sensitivityScale, int mpIdentClient ) {
#ifdef API_Q3
	Q_syscall( CG_SETUSERCMDVALUE, stateValue, PASSFLOAT(sensitivityScale) );
#endif
#ifdef API_ET
	Q_syscall( CG_SETUSERCMDVALUE, stateValue, flags, PASSFLOAT(sensitivityScale), mpIdentClient );
#endif
}
