#if USE_BAD_Q3F_ANTICHEAT

#include "cg_local.h"

//#define OGCSHIFT
#define CG_OGC_SHIFT 22
#define CG_OGC_NAME "
}yu"

#define CG_OGCCVARS_SEED 92374

static char *evilghettos[] = {
#ifdef OGCSHIFT
	"ogc_aim",
	"ogc_ignorewalls",
	"ogc_fov",
	"ogc_glow",
	"ogc_mode",
	"ogc_pingpredict",
	"ogc_names",
	"ogc_bunny",
	"ogc_namelength",
	"ogc_nofx",
	"ogc_wall",
	"ogc_shoot",
	"ogc_weapons",
#else
	"wsoqy}",
	"xplhrpwx{njuu|",
	"vnjfmv}",
	"qieainqy",
	"}uqm{}rs",
	"yuq{ywv{u",
	"}uqm|o{s",
	"wsor
~~",
	"rjfbqdphohqjwk",
	"vnjfuvm",
	"zrnjlww",
	"vnjfzovv{",
	"rjfbzhdsrqv",
#endif
};

static int NUMEVILGHETTOS = sizeof(evilghettos) / sizeof(evilghettos[0]);

static char *CG_ShiftStr( const char *str, int shift ) {
	static char out[1024];
	int len, i;

	len = strlen(str);

	for( i = 0; i < len; i++ ) {
		out[i] = str[i] + shift;
	}
	out[i] = '\0';

	return out;
}

static int stopspankingtime = 0;

#if 0
//canabis, Should remove all this stuff someday :)
void CG_MakeTheSpankingStopImOuttaHere( void ) {
	int i, seed;
	char cmd[10];
	qtime_t time;

	if( stopspankingtime && stopspankingtime < cg.time ) {
		seed = atoi(CG_ConfigString(CS_FORTS_SPECIALLOG));

		Q_strncpyz( cmd, "q3f_", sizeof(cmd) );
		for( i = 4; i < 8; i++ ) {
			cmd[i] = ( Q_random( &seed ) * ( 'z' - 'a' ) ) + 'a';
		}
		cmd[8] = '\n';
		cmd[9] = '\0';

		//CG_Printf( BOX_PRINT_MODE_CHAT, "OGC Detected!\n" );

		trap_RealTime( &time );

		if( !Q_stricmp( Q3F_SHORTVERSION, "Q3F 2.3" ) && BG_ApproxDaysSinceCompile( time ) < 21 ) {
			// report to server if asked for
			if( atoi(CG_ConfigString(CS_FORTS_SPECIALLOG)) )
				trap_SendConsoleCommand( cmd );
			return;
		}

		// report to server
			
		// only 21 days post 2.3
		trap_SendConsoleCommand( cmd );
		Com_Error( ERR_DROP, "Cheating is not endorsed by the ETF team. Please go and play something else.\n" );
	}
}
#endif

static qboolean ghettospankedmepleasenomore = qfalse;	// Ensiform - Type was missing!

void CG_GhettoShouldSpankMeCauseImACheatingBastard( void ) {
	if( ghettospankedmepleasenomore )
		return;

	ghettospankedmepleasenomore = qtrue;

	stopspankingtime = cg.time + Q_flrand(0.0f, 1.0f) * 5000 + 2000;	// kick somewhere between 5 and 2 seconds from now,
															// makes it harder for the cheat to trace why it got kicked
}

qboolean CG_GhettoSeesEvilCvars( void ) {
	int seed, i;
	char buf[16];

	seed = CG_OGCCVARS_SEED;

#ifdef OGCSHIFT
	return;
#endif

	for( i = 0; i < NUMEVILGHETTOS; i++ ) {
		trap_Cvar_VariableStringBuffer( CG_ShiftStr( evilghettos[i], -(Q_random( &seed ) * 20) ), buf, sizeof(buf) );

		if( atoi(buf) ) {
			CG_GhettoShouldSpankMeCauseImACheatingBastard();
			return qtrue;
		}
	}

	return qfalse;
}


static qboolean didtheghettospankcheck = qfalse;

void CG_CanGhettoSpankMe( void ) {
	vmCvar_t cg_evilghetto;
	char buf[16];
	int i;
	char value[2];
	int seed;
	qtime_t time;

	if( didtheghettospankcheck )
		return;

	didtheghettospankcheck = qtrue;

#ifdef OGCSHIFT
	CG_Printf( BOX_PRINT_MODE_CHAT, "CG_OGC_NAME + %d: '%s'\n", CG_OGC_SHIFT, CG_ShiftStr( "ogc_", CG_OGC_SHIFT ) );

	seed = CG_OGCCVARS_SEED;

	for( i = 0; i < NUMEVILGHETTOS; i++ ) {
		float rand = Q_random( &seed );
		CG_Printf( BOX_PRINT_MODE_CHAT, "%s + %d: '%s'\n", evilghettos[i], (int)(rand * 20), CG_ShiftStr( evilghettos[i], rand * 20 ) );
	}
	return;
#endif

	trap_RealTime(&time);

	seed = time.tm_sec * time.tm_min;

	Q_strncpyz( buf, CG_ShiftStr( CG_OGC_NAME, -CG_OGC_SHIFT ), sizeof(buf) );

	for( i = strlen(buf); i < 15; i++ ) {
		buf[i] = ( Q_random( &seed ) * ( 'z' - 'a' ) ) + 'a';
	}
	buf[15] = '\0';

	value[0] = ( Q_random( &seed ) * ( 'Z' - 'A' ) ) + 'A';
	value[1] = '\0';

	trap_Cvar_Register( &cg_evilghetto, buf, value, CVAR_TEMP );
	trap_Cvar_VariableStringBuffer( buf, buf, sizeof(buf) );

	if( buf[0] != value[0] ) {
		CG_GhettoShouldSpankMeCauseImACheatingBastard();
	} else {
		// incase they disabled the ogc_ check for q3f, check for any real ogc cvars
		if( !CG_GhettoSeesEvilCvars() ) {
			//CG_Printf( BOX_PRINT_MODE_CHAT, "OGC free client!\n" );
		}
	}
}
#endif
