// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_servercmds.c -- reliably sequenced text commands sent by the server
// these are processed at snapshot transition time, so there will definately
// be a valid snapshot this frame

#include "cg_local.h"
#include "cg_q3f_menu.h"
#include "cg_q3f_sounddict.h"
#include "cg_q3f_mapselect.h"
#include "cg_q3f_grenades.h"
#include "../game/bg_q3f_weapon.h"
#include "../game/bg_q3f_tea.h"
#include "../../ui/menudef.h"

void CG_IntermissionScoreDump();

/*
=================
CG_ParseStats

=================
*/
static void CG_ParseStats( void ) {
	const char	*buf;

	buf = CG_Argv(1);
	trap_SendConsoleCommand(va("ui_stats %s\n", buf));
}

/*
=================
CG_ParseAwards

=================
*/
static void CG_ParseAwards( void ) {
	int		j;
	char	buf[1400];

	Com_sprintf(buf, 1400, "\"%s\"", CG_Argv(1) );
	Q_strcat(buf, 1400, va(" \"%s\"", CG_Argv(2)));
	Q_strcat(buf, 1400, va(" \"%s\"", CG_Argv(3)));
	Q_strcat(buf, 1400, va(" \"%s\"", CG_Argv(4)));

	for(j = 0; j < 31; j++) {
		Q_strcat(buf, 1400, va(" \"%s\"", CG_Argv(5 + j)) );
	}

	trap_SendConsoleCommand(va("ui_awards %s\n", buf));
}


/*
=================
CG_ParseScores

=================
*/
static void CG_ParseScores( void ) {
	int		i;

	cg.numScores = atoi( CG_Argv( 1 ) );
	if ( cg.numScores > MAX_CLIENTS ) {
		cg.numScores = MAX_CLIENTS;
	}

	cg.teamScores[0] = atoi( CG_Argv( 2 ) );
	cg.teamScores[1] = atoi( CG_Argv( 3 ) );
	cg.teamScores[2] = atoi( CG_Argv( 4 ) );
	cg.teamScores[3] = atoi( CG_Argv( 5 ) );

	memset( cg.scores, 0, sizeof( cg.scores ) );
	for ( i = 0 ; i < cg.numScores ; i++ ) {
		cg.scores[i].client =	atoi(CG_Argv(i * 5 + 6));
		cg.scores[i].score =	atoi(CG_Argv(i * 5 + 7));
		cg.scores[i].ping =		atoi(CG_Argv(i * 5 + 8));
		cg.scores[i].time =		atoi(CG_Argv(i * 5 + 9));
		cg.scores[i].flags =	atoi(CG_Argv(i * 5 + 10));

		if ( cg.scores[i].client < 0 || cg.scores[i].client >= MAX_CLIENTS ) {
			cg.scores[i].client = 0;
		}
		cgs.clientinfo[ cg.scores[i].client ].score = cg.scores[i].score;

		cg.scores[i].team = cgs.clientinfo[cg.scores[i].client].team;
	}

	CG_SortScoreboard();
	
	if(cg.intermissionStarted) {
		CG_IntermissionScoreDump();
	}
}

/*
=================
CG_ParseTeamInfo

=================
*/
static void CG_ParseTeamInfo( void ) {
	int		i;
	int		client;

	numSortedTeamPlayers = atoi( CG_Argv( 1 ) );

	if( numSortedTeamPlayers < 0 || numSortedTeamPlayers > TEAM_MAXOVERLAY ) {
		CG_Error("CG_ParseTeamInfo: numSortedTeamPlayers out of range (%d)", numSortedTeamPlayers);
	}

	for ( i = 0 ; i < numSortedTeamPlayers ; i++ ) {
		client = atoi( CG_Argv( i * 3 + 2 ) );
		if  ( client < 0 || client >= MAX_CLIENTS ) {
			CG_Error("CG_ParseTeamInfo: bad client number: %d", client);
			continue;
		}

		sortedTeamPlayers[i] = client;

		cgs.clientinfo[ client ].health =	atoi( CG_Argv( i * 3 + 3 ) );
		cgs.clientinfo[ client ].armor =	atoi( CG_Argv( i * 3 + 4 ) );
	}
}


/*
================
CG_ParseServerinfo

This is called explicitly when the gamestate is first received,
and whenever the server updates any serverinfo flagged cvars
================
*/

void CG_ParseTeamNameinfo( void ) {
	const char	*info;

	info = CG_ConfigString( CS_TEAMNAMES );
	Q_strncpyz( cgs.redTeam,	Info_ValueForKey( info, "g_etf_redTeam" ),		sizeof(cgs.redTeam) );
	Q_strncpyz( cgs.blueTeam,	Info_ValueForKey( info, "g_etf_blueTeam" ),		sizeof(cgs.blueTeam) );
	Q_strncpyz( cgs.yellowTeam, Info_ValueForKey( info, "g_etf_yellowTeam" ),	sizeof(cgs.yellowTeam) );
	Q_strncpyz( cgs.greenTeam,	Info_ValueForKey( info, "g_etf_greenTeam" ),	sizeof(cgs.greenTeam) );

	trap_Cvar_Set("cg_redTeam",		cgs.redTeam);
	trap_Cvar_Set("cg_blueTeam",	cgs.blueTeam);
	trap_Cvar_Set("cg_greenTeam",	cgs.greenTeam);
	trap_Cvar_Set("cg_yellowTeam",	cgs.yellowTeam);
}

void CG_ParseServerinfo( void ) {
	const char	*info;
	char	*mapname;

	info = CG_ConfigString( CS_SERVERINFO );
	cgs.gametype = atoi( Info_ValueForKey( info, "g_gametype" ) );
	trap_Cvar_Set("g_gametype", va("%i", cgs.gametype));
	cgs.dmflags = atoi( Info_ValueForKey( info, "dmflags" ) );
	cgs.teamflags = atoi( Info_ValueForKey( info, "teamflags" ) );
//	cgs.fraglimit = atoi( Info_ValueForKey( info, "fraglimit" ) );
//	cgs.capturelimit = atoi( Info_ValueForKey( info, "capturelimit" ) );
	cgs.timelimit = atoi( Info_ValueForKey( info, "timelimit" ) );
	cgs.maxclients = atoi( Info_ValueForKey( info, "sv_maxclients" ) );
	cgs.gameindex = atoi( Info_ValueForKey( info, "g_gameindex" ) );
	mapname = Info_ValueForKey( info, "mapname" );
	Q_strncpyz( cgs.rawmapname, mapname, sizeof(cgs.rawmapname) );  //keeg for tracemap support
	Com_sprintf( cgs.mapname, sizeof( cgs.mapname ), "maps/%s.bsp", mapname );

	cg.teamScores[0] = atoi( Info_ValueForKey( info, "score_red" ) );
	cg.teamScores[1] = atoi( Info_ValueForKey( info, "score_blue" ) );
	cg.teamScores[2] = atoi( Info_ValueForKey( info, "score_yellow" ) );
	cg.teamScores[3] = atoi( Info_ValueForKey( info, "score_green" ) );

	// we'll need this for deciding whether or not to predict weapon effects
	cgs.unlagged = atoi( Info_ValueForKey( info, "g_unlagged" ) );
}



static void CG_ParseMatchState( void ) {
	int	oldstate = cg.matchState;

	cg.matchState = atoi (CG_ConfigString( CS_MATCH_STATE ));
	if (cg.matchState == oldstate)
		return;
	if (cg.matchState == MATCH_STATE_WARMUP) {
       trap_S_StartLocalSound( cgs.media.countPrepareSound, CHAN_ANNOUNCER );
	} else if (cg.matchState == MATCH_STATE_PLAYING) {
		trap_S_StartLocalSound( cgs.media.countFightSound, CHAN_ANNOUNCER );
		CG_LowPriority_Printf( BOX_PRINT_MODE_CENTER, "FIGHT!" );
	}

}

/*
==================
CG_ParseWarmup
==================
*/
static void CG_ParseWarmup( void ) {
	cg.warmupCount = -1;
	cg.warmup = atoi( CG_ConfigString( CS_WARMUP ));
}

/*
=====================
CG_ParseScores
=====================
*/
static void CG_ParseTeamScores( void ) {
	const char	*info;

	info = CG_ConfigString( CS_SERVERINFO );

	cg.teamScores[0] = atoi( Info_ValueForKey( info, "score_red" ) );
	cg.teamScores[1] = atoi( Info_ValueForKey( info, "score_blue" ) );
	cg.teamScores[2] = atoi( Info_ValueForKey( info, "score_yellow" ) );
	cg.teamScores[3] = atoi( Info_ValueForKey( info, "score_green" ) );
}

/*
================
CG_SetConfigValues

Called on load to set the initial values from configure strings
================
*/
void CG_SetConfigValues( void ) {
//	const char *s;

	CG_ParseTeamScores();
	cgs.levelStartTime = atoi( CG_ConfigString( CS_LEVEL_START_TIME ) );

	CG_ParseWarmup();
	CG_ParseMatchState();
	if ( cg_atmosphericEffects.integer )
		//CG_Q3F_EffectParse( CG_ConfigString( CS_Q3F_ATMOSEFFECT ) );
      CG_EffectParse( CG_ConfigString( CS_FORTS_ATMOSEFFECT ) );  //keeger new atmospherics
	cg.ceaseFire = atoi( CG_ConfigString( CS_FORTS_CEASEFIRE ) ) ? qtrue : qfalse;	// Golliwog: Can't shoot/gren/pickup if set
}

/*
=====================
CG_ShaderStateChanged
=====================
*/
void CG_ShaderStateChanged(void) {
	char originalShader[MAX_QPATH];
	char newShader[MAX_QPATH];
	char timeOffset[16];
	const char *o;
	char *n,*t;

	o = CG_ConfigString( CS_SHADERSTATE );
	while (o && *o) {
		n = strstr(o, "=");
		if (n && *n) {
			strncpy(originalShader, o, n-o);
			originalShader[n-o] = 0;
			n++;
			t = strstr(n, ":");
			if (t && *t) {
				strncpy(newShader, n, t-n);
				newShader[t-n] = 0;
			} else {
				break;
			}
			t++;
			o = strstr(t, "@");
			if (o) {
				strncpy(timeOffset, t, o-t);
				timeOffset[o-t] = 0;
				o++;
				trap_R_RemapShader( originalShader, newShader, timeOffset );
			}
		} else {
			break;
		}
	}
}

void CG_ETF_RegisterClass( int classNum ) {
	int i;
	bg_q3f_playerclass_t *cls;

	// Not enabled
	if ( !( cgs.classes & ( 1 << classNum ) ) ) {
		return;
	}

	// Already registered
	if ( cgs.media.modelcache[classNum][0] && cgs.media.f2rcache[classNum][0] ) {
		return;
	}

	cls = bg_q3f_classlist[classNum];

	// Register the class's model
	if ( !CG_Q3F_RegisterClassModels( classNum ) ) {
	}

	// Register the class's weapons
	for ( i = 0; i < Q3F_NUM_WEAPONSLOTS; i++ ) {
		if ( !cls->weaponslot[i] )
			continue;

		if ( cls->weaponslot[i] >= WP_NUM_WEAPONS )
			continue;

		if ( cls->weaponslot[i] == WP_AXE ) {
			switch ( classNum ) {
			case Q3F_CLASS_PARAMEDIC:
				CG_RegisterExtendedWeapon( Q3F_WP_BIOAXE );
				break;
			case Q3F_CLASS_AGENT:
				CG_RegisterExtendedWeapon( Q3F_WP_KNIFE );
				break;
			case Q3F_CLASS_ENGINEER:
				CG_RegisterExtendedWeapon( Q3F_WP_WRENCH );
				break;
			}
		} else {
			CG_RegisterWeapon( cls->weaponslot[i] );
		}
	}

	if ( !CG_Q3F_RegisterClassSounds( classNum ) ) {
	}
	// Register special media for classes
	// FIXME - make this a bit nicer
	switch ( classNum ) {
	case Q3F_CLASS_RECON:
		break;
	case Q3F_CLASS_SNIPER:
		break;
	case Q3F_CLASS_ENGINEER:
		//Always load the rocket launcher for sentries too
		CG_RegisterWeapon( WP_ROCKET_LAUNCHER );

		cgs.media.sentryBase = trap_R_RegisterModel( "models/objects/sentry/sentry_base.md3" );
		cgs.media.sentryTurret1 = trap_R_RegisterModel( "models/objects/sentry/sentry_turret.md3" );
		cgs.media.sentryTurret2 = trap_R_RegisterModel( "models/objects/sentry/sentry_turret_l2.md3" );
		cgs.media.sentryTurret3 = trap_R_RegisterModel( "models/objects/sentry/sentry_turret_l3.md3" );
		cgs.media.sentryCannon1 = trap_R_RegisterModel( "models/objects/sentry/sentry_turret_inner.md3" );
		cgs.media.sentryCannon2 = trap_R_RegisterModel( "models/objects/sentry/sentry_turret_inner_l2.md3" );
		cgs.media.sentryCannon3 = trap_R_RegisterModel( "models/objects/sentry/sentry_turret_inner_l3.md3" );
		cgs.media.sentryBarrel = trap_R_RegisterModel( "models/objects/sentry/sentry_minigun.md3" );
		cgs.media.sentryRocketLauncher = trap_R_RegisterModel( "models/objects/sentry/sentry_rocketl.md3" );
		cgs.media.sentryFlash = trap_R_RegisterModel( "models/objects/sentry/sentry_flash.md3" );
		cgs.media.sentryBits[0] = trap_R_RegisterModel( "models/objects/sentry/sentry_bit1.md3" );
		cgs.media.sentryBits[1] = trap_R_RegisterModel( "models/objects/sentry/sentry_bit2.md3" );
		cgs.media.sentryBits[2] = trap_R_RegisterModel( "models/objects/sentry/sentry_bit3.md3" );
		cgs.media.sentryBits[3] = trap_R_RegisterModel( "models/objects/sentry/sentry_bit4.md3" );
		cgs.media.supplystationBase = trap_R_RegisterModel( "models/objects/supplystation/supply.md3" );
		cgs.media.supplystationHUD = trap_R_RegisterModel( "models/objects/supplystation/supply_hud.md3" );
		cgs.media.supplystationBits[0] = trap_R_RegisterModel( "models/objects/supplystation/supplystation_bit1.md3" );
		cgs.media.supplystationBits[1] = trap_R_RegisterModel( "models/objects/supplystation/supplystation_bit2.md3" );
		cgs.media.supplystationBits[2] = trap_R_RegisterModel( "models/objects/supplystation/supplystation_bit3.md3" );
		cgs.media.sentrySpinupSound = trap_S_RegisterSound( "sound/movers/motors/motor_start_01.wav", qfalse );
		cgs.media.sentryFireSound = trap_S_RegisterSound( "sound/weapons/deploy/sentry_fire.wav", qfalse );
		cgs.media.sentryStartSound = trap_S_RegisterSound( "sound/weapons/deploy/sentry_seek.wav", qfalse );
		cgs.media.sentryStopSound = trap_S_RegisterSound( "sound/weapons/deploy/sentry_reset.wav", qfalse );
		cgs.media.sentryBuildSound = trap_S_RegisterSound( "sound/weapons/deploy/sentry_build.wav", qtrue );
		cgs.media.sentryUpgradeSound = trap_S_RegisterSound( "sound/weapons/deploy/sentry_upgrade.wav", qtrue );
		cgs.media.sentryExplodeSound = trap_S_RegisterSound( "sound/weapons/deploy/sentry_explode.wav", qtrue );
		cgs.media.supplyBuildSound = trap_S_RegisterSound( "sound/weapons/deploy/supply_build.wav", qtrue );
		cgs.media.supplyPopup = trap_S_RegisterSound( "sound/weapons/deploy/supply_out.wav", qfalse );
		cgs.media.supplyRetract = trap_S_RegisterSound( "sound/weapons/deploy/supply_in.wav", qfalse );
		cgs.media.supplyExplodeSound = trap_S_RegisterSound( "sound/weapons/deploy/supply_explode.wav", qtrue );
		// start hack of the year!
		if ( r_vertexLight.integer )
			trap_Cvar_Set( "r_vertexlight", "0" );
		cgs.media.sentryConstruct_Base = trap_R_RegisterShader( "models/objects/sentry/texture_sentry_base_construct" );
		cgs.media.sentryConstructShader_1 = trap_R_RegisterShader( "models/objects/sentry/texture_sentry_level1_construct" );
		cgs.media.sentryConstructShader_2 = trap_R_RegisterShader( "models/objects/sentry/texture_sentry_level2_construct" );
		cgs.media.supplystationConstruct_Base = trap_R_RegisterShader( "models/objects/supplystation/base_construct" );
		cgs.media.supplystationConstruct_Screen = trap_R_RegisterShader( "models/objects/supplystation/screen_construct" );
		cgs.media.sentryTvFx = trap_R_RegisterShader( "gfx/sfx/sentryCamTvBlur" );
		// exit hack of the year!
		if ( r_vertexLight.integer )
			trap_Cvar_Set( "r_vertexlight", "1" );
		break;
	case Q3F_CLASS_CIVILIAN:
		break;
	case Q3F_CLASS_GRENADIER:
		break;
	case Q3F_CLASS_PARAMEDIC:
		break;
	case Q3F_CLASS_FLAMETROOPER:
		cgs.media.sfx_napalmExplode = trap_S_RegisterSound( "sound/weapons/explosive/gren_napalm_start.wav", qfalse );
		cgs.media.sfx_napalmBurn = trap_S_RegisterSound( "sound/weapons/explosive/gren_napalm_loop.wav", qfalse );
		cgs.media.sfx_napalmWater = trap_S_RegisterSound( "sound/weapons/explosive/gren_napalm_water.wav", qfalse );
		break;
	case Q3F_CLASS_AGENT:
		break;
	case Q3F_CLASS_SOLDIER:
		//Always load the nailgun for soldier's too because of nail model so we dont hitch on nailgren
		CG_RegisterWeapon( WP_NAILGUN );
		break;
	case Q3F_CLASS_MINIGUNNER:
		break;
	}
}

/*
================
CG_ConfigStringModified

================
*/
static void CG_ConfigStringModified( void ) {
	const char	*str;
	int		num;

	num = atoi( CG_Argv( 1 ) );

	// get the gamestate from the client system, which will have the
	// new configstring already integrated
	trap_GetGameState( &cgs.gameState );

	// look up the individual string that was modified
	str = CG_ConfigString( num );

	// do something with it if necessary
	if ( num == CS_MUSIC ) {
		CG_StartMusic();
	} else if ( num == CS_SERVERINFO ) {
		CG_ParseServerinfo();
	} else if ( num == CS_TEAMNAMES ) {
		CG_ParseTeamNameinfo();
	} else if ( num == CS_WARMUP ) {
		CG_ParseWarmup();
	} else if ( num == CS_MATCH_STATE ) {
		CG_ParseMatchState();
	} else if ( num == CS_LEVEL_START_TIME ) {
		cgs.levelStartTime = atoi( str );
	} else if ( num == CS_VOTE_TIME ) {
		cgs.voteTime = atoi( str );
		cgs.voteModified = qtrue;
	} else if ( num == CS_VOTE_YES ) {
		cgs.voteYes = atoi( str );
		cgs.voteModified = qtrue;
	} else if ( num == CS_VOTE_NO ) {
		cgs.voteNo = atoi( str );
		cgs.voteModified = qtrue;
	} else if ( num == CS_VOTE_STRING ) {
		Q_strncpyz( cgs.voteString, str, sizeof( cgs.voteString ) );
	} else if ( num == CS_INTERMISSION ) {
		cg.intermissionStarted = atoi( str );
		if(cg.intermissionStarted) {
			CG_SetupIntermissionMenu();
		} else {
			CG_EventHandling(CGAME_EVENT_NONE, qfalse);
			trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_CGAME );
		}
	} else if ( num >= CS_MODELS && num < CS_MODELS+MAX_MODELS ) {
		cgs.gameModels[ num-CS_MODELS ] = trap_R_RegisterModel( str );
	} else if ( num >= CS_SOUNDS && num < CS_SOUNDS+MAX_SOUNDS ) {
		if ( str[0] != '*' ) {	// player specific sounds don't register here
			cgs.gameSounds[ num-CS_SOUNDS] = trap_S_RegisterSound( str, qfalse );
		}
	} else if( num >= CS_SHADERS && num < CS_SHADERS+MAX_SHADERS ) {
		cgs.gameShaders[ num - CS_SHADERS ] = str[0] == '*'
												? trap_R_RegisterShader( str + 1 )
												: trap_R_RegisterShaderNoMip( str );
	} else if( num >= CS_SPIRITSCRIPTS && num < CS_SPIRITSCRIPTS+MAX_SPIRITSCRIPTS ) {
		cgs.gameSpiritScript[ num - CS_SPIRITSCRIPTS ] = 
			Spirit_LoadScript( str );
	} else if ( num >= CS_PLAYERS && num < CS_PLAYERS+MAX_CLIENTS ) {
		CG_NewClientInfo( num - CS_PLAYERS );
		CG_BuildSpectatorString();
	}
	else if ( num == CS_SHADERSTATE ) {
		CG_ShaderStateChanged();
	} else if( num == CS_FORTS_ATMOSEFFECT ) {
		//CG_Q3F_EffectParse( str );
		CG_EffectParse( str );  //Keeger atmospheric code
	} else if( num == CS_FORTS_CEASEFIRE ) {
		cg.ceaseFire = atoi( str ) ? qtrue : qfalse;
	} else if( num == CS_FORTS_MAPVOTENAMES ) {
		trap_SendConsoleCommand( "ui_updatemapvotes\n" );
//		CG_Q3F_MapSelectInit( str );
	} else if( num == CS_FORTS_MAPVOTETALLY ) {
		trap_SendConsoleCommand( "ui_updatemapvotetally\n" );
//		CG_Q3F_MapSelectTally( str );
	} else if( num == CS_FORTS_TEAMPINGS ) {
		sscanf(	str, "%i %i %i %i %i %i %i %i %i",
				&cg.teamCounts[0], &cg.teamPings[0],
				&cg.teamCounts[1], &cg.teamPings[1],
				&cg.teamCounts[2], &cg.teamPings[2],
				&cg.teamCounts[3], &cg.teamPings[3],
				&cg.teamBalanceWarnings );
	} else if ( num == CS_Q3F_CVARLIMITS ) {
		CG_Q3F_UpdateCvarLimits();
	} else if ( num == CS_TEAMMASK ) {
		cgs.teams = atoi( str );
	} else if ( num == CS_TEAMALLIED ) {
		sscanf(	str, "%i %i %i %i",
			&cg.teamAllies[0], &cg.teamAllies[1],
			&cg.teamAllies[2], &cg.teamAllies[3] );
	} else if ( num == CS_CLASSMASK && !cgs.initing ) {
		int oldClasses = cgs.classes;
		cgs.classes = atoi( CG_ConfigString( CS_CLASSMASK ) );
		if ( oldClasses != cgs.classes ) {
			int i;
			for ( i = Q3F_CLASS_RECON; i < Q3F_CLASS_MAX; i++ ) {
				CG_ETF_RegisterClass( i );
			}
		}
	}
}

/*
===============
CG_MapRestart

The server has issued a map_restart, so the next snapshot
is completely new and should not be interpolated to.

A tournement restart will clear everything, but doesn't
require a reload of all the media
===============
*/
static void CG_MapRestart( void ) {
	if ( cg_showmiss.integer ) {
		CG_Printf( BOX_PRINT_MODE_CHAT, "CG_MapRestart\n" );
	}

	if ( cg.matchLogFileHandle > 0 ) {
		CG_MatchLogAddLine("info map_restart\n");
	}


	CG_ClearFlameChunks();
	CG_InitSmokeSprites();
	CG_InitLocalEntities();
	CG_InitMarkPolys();
	
	cg.timelimitWarnings = 0;

	cg.intermissionStarted = qfalse;

	cgs.voteTime = 0;

	cg.mapRestart = qtrue;

	CG_StartMusic();

	trap_S_ClearLoopingSounds(qtrue);

	trap_R_ClearDecals();

	// we really should clear more parts of cg here and stop sounds

	trap_Cvar_Set("cg_thirdPerson", "0");

	CG_LoadHud_f();

	CG_EventHandling(CGAME_EVENT_NONE, qtrue);
}

/*
=================
CG_RemoveChatEscapeChar
=================
*/
static void CG_RemoveChatEscapeChar( char *text ) {
	int i, l;

	l = 0;
	for ( i = 0; text[i]; i++ ) {
		if (text[i] == '\x19')
			continue;
		text[l++] = text[i];
	}
	text[l] = '\0';
}

static void CG_Q3F_PipeCommand(const char* cmd) {
	char buffer[1024];
	int i, cnt;

	cnt = trap_Argc();

	Q_strncpyz(buffer, cmd, 1024);

	for(i = 1; i < cnt; i++) {
		char buff[64];
		trap_Argv(i, buff, 64);

		Q_strcat(buffer, 1024, va(" %s", buff));
	}

	trap_SendConsoleCommand(va("%s\n", buffer));
}

/*
=================
CG_ServerCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
static void CG_ServerCommand( void ) {
	const char	*cmd;
	char		text[MAX_SAY_TEXT];

	cmd = CG_Argv(0);

	if ( !cmd[0] ) {
		// server claimed the command
		return;
	}

	if ( !strcmp( cmd, "cp" ) ) {
		if ( cg.matchLogFileHandle > 0 ) {
			CG_MatchLogAddLine(va("print %s", CG_Argv(1)));
		}
		CG_Printf( BOX_PRINT_MODE_CENTER, "%s", CG_Argv(1) );
		CG_Printf( BOX_PRINT_MODE_CHAT, "%s", CG_Argv(1) );
		return;
	}

	if ( !strcmp( cmd, "cpb" ) ) {
		CG_Printf( BOX_PRINT_MODE_CENTER, "%s", CG_Argv(1) );
		return;
	}

	if ( !strcmp( cmd, "cs" ) ) {
		CG_ConfigStringModified();
		return;
	}

	if ( !strcmp( cmd, "print" ) ) {
		CG_Printf( BOX_PRINT_MODE_CHAT, "%s", CG_Argv(1) );
		if ( cg.matchLogFileHandle > 0 ) {
			CG_MatchLogAddLine(va("print %s", CG_Argv(1)));
		}
		return;
	}


	// Golliwog: No means of playing sounds to player only?
	if( !strcmp( cmd, "play" ) ) {
		CG_Q3F_PlayLocalSound( CG_Argv(1) );
		return;
	}

	/*if( !strcmp( cmd, "tplay" ) ) {
		if ( cg_teamChatSounds.integer >= 1 )
			CG_Q3F_PlayLocalSound( CG_Argv(1) );
		return;
	}*/

	if( !strcmp( cmd, "playstring" ) ) {
		char strbuff[MAX_STRING_CHARS];
		trap_Args( strbuff, MAX_STRING_CHARS );
		CG_Q3F_ParseSoundDictionary();
		CG_Q3F_StartSoundString( strbuff );
		return;
	}

	if( !strcmp( cmd, "tplaystring" ) ) {
		char strbuff[MAX_STRING_CHARS];
		if( cg_teamChatSounds.integer >= 2 ) {
			trap_Args( strbuff, MAX_STRING_CHARS );
			CG_Q3F_ParseSoundDictionary();
			CG_Q3F_StartSoundString( strbuff );
		}
		return;
	}
	// Golliwog.

	// Golliwog: Process menu commands
	if( !strcmp( cmd, "menu" ) ) {
		CG_Q3F_MenuCommand();
		return;
	}
	// Golliwog.

	// Golliwog: The player is being warned about teamkilling
	if( !strcmp( cmd, "tkwarn" ) ) {
		cg.teamKillWarnTime = cg.time + 2000;
		return;
	}
	// Golliwog.

	// RR2DO2: enemy is using supplystation
	if ( !strcmp( cmd, "enesup" ) ) {
		cg.supplyStationUsedTime = cg.time + 1000;
		cg.supplyStationUserIsEnemy = qtrue;

		CG_Printf( BOX_PRINT_MODE_CHAT, "%s", CG_Argv(1) );
		return;
	}

	// friendly is using supplystation
	if ( !strcmp( cmd, "frisup" ) ) {
		cg.supplyStationUsedTime = cg.time + 1000;
		cg.supplyStationUserIsEnemy = qfalse;
		return;
	}
	// RR2DO2

	if ( !strcmp( cmd, "chat" ) ) {
		if ( cg.matchLogFileHandle > 0 ) {
			CG_MatchLogAddLine(va("chat %s\n", CG_Argv(1)));
		}
		if ( !cg_teamChatsOnly.integer ) {
			Q_strncpyz( text, CG_Argv(1), MAX_SAY_TEXT );
			CG_RemoveChatEscapeChar( text );
			CG_Printf( BOX_PRINT_MODE_CHAT_REAL, "%s\n", text );

			cmd = CG_Argv(2);
			if ( cg_teamChatSounds.integer >= 1 && atoi(cmd) )
				CG_Q3F_PlayLocalSound( cmd );
			else
				trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
		}
		return;
	}

	if ( !strcmp( cmd, "tchat" ) ) {
		//cmd = CG_Argv(1);
		//if( strcmp( cmd + strlen(cmd) - strlen(Q3F_MESSAGE_BLANK), Q3F_MESSAGE_BLANK ) )
		//	trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
		Q_strncpyz( text, CG_Argv(1), MAX_SAY_TEXT );
		CG_RemoveChatEscapeChar( text );
		CG_Printf( BOX_PRINT_MODE_TEAMCHAT, "%s\n", text );

		cmd = CG_Argv(2);
		if ( cg_teamChatSounds.integer >= 1 && atoi(cmd) )
			CG_Q3F_PlayLocalSound( cmd );
		else
			trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
		return;
	}

	if ( !strcmp( cmd, "scores" ) ) {
		CG_ParseScores();
		return;
	}

	if ( !strcmp( cmd, "stats") ) {
		CG_ParseStats();
		return;
	}

	if ( !strcmp( cmd, "awards") ) {
		CG_ParseAwards();
		return;
	}

	if ( !strcmp( cmd, "tinfo" ) ) {
		CG_ParseTeamInfo();
		return;
	}

	if ( !strcmp( cmd, "map_restart" ) ) {
		CG_MapRestart();
		return;
	}

	if ( Q_stricmp (cmd, "remapShader") == 0 ) {
		if (trap_Argc() == 4) {
			trap_R_RemapShader(CG_Argv(1), CG_Argv(2), CG_Argv(3));
		}
		return;
	}

	if( !strcmp( cmd, "mapquery" ) ) {
		CG_Q3F_MapSelectRespond();
		return;
	}

	if( !strcmp( cmd, "waypoint" ) ) {
		CG_Q3F_WaypointCommand();
		return;
	}

	// clientLevelShot is sent before taking a special screenshot for
	// the menu system during development
	if ( !strcmp( cmd, "clientLevelShot" ) ) {
		CG_Printf( BOX_PRINT_MODE_CHAT, "Illegal client game command: %s\n", cmd );
		//cg.levelShot = qtrue;
		return;
	}

	// RR2DO2
	if ( !strcmp( cmd, "flyby" ) ) {
		CG_Q3F_Flyby();
		return;
	}

	if ( !strcmp( cmd, "classinfo") ) {
		cg.classInfoTime = cg.time;
		CG_Q3F_StoreClassinfo();
		return;
	}

	if ( !strcmp( cmd, "hud_auth_admin") ) {
		char str[16];
		trap_Argv(1, str, 16);
	
		trap_Cvar_Set( "hud_admin_auth", str);
		return;
	}	

	if ( !strcmp( cmd, "hud_auth_rcon") ) {
		char str[16];
		trap_Argv(1, str, 16);
	
		trap_Cvar_Set( "hud_rcon_auth", str);
		return;
	}

	if ( !strcmp( cmd, "hud_auth_shoutcast") ) {
		char str[16];
		trap_Argv(1, str, 16);
	
		trap_Cvar_Set( "hud_shoutcast_auth", str);
		return;
	}

	if ( !strcmp( cmd, "hud_iplist" )) {
		CG_Q3F_PipeCommand("hud_iplist");
		return;
	}

	if ( !strcmp( cmd, "hud_banlist" )) {
		CG_Q3F_PipeCommand("hud_banlist");
		return;
	}

	//keeg cpm and spawnserver don't seem to be used in ETF...so just ignore
	// Ensiform: spawnserver COULD be, to allow telling clients of a restart
	if ( !strcmp( cmd, "cpm" )) {
		return;
	}

	if ( !strcmp( cmd, "spawnserver" )) {
		CG_Printf( BOX_PRINT_MODE_CENTER, "%s", CG_Argv(1) );

		// hack here
		cg.serverRespawning = qtrue;
		return;
	}
	//end-keeg

   // slothy
   if( !strcmp( cmd, "finfo")) {
	   trap_Argv(1, cg.finfo, MAX_SAY_TEXT);
	   cg.fi_endtime = cg.time + 5000;
	   return;
   }
   
	CG_Printf( BOX_PRINT_MODE_CHAT, "Unknown client game command: %s\n", cmd );
}


/*
====================
CG_ExecuteNewServerCommands

Execute all of the server commands that were received along
with this this snapshot.
====================
*/
void CG_ExecuteNewServerCommands( int latestSequence ) {
	while ( cgs.serverCommandSequence < latestSequence ) {
		if ( trap_GetServerCommand( ++cgs.serverCommandSequence ) ) {
			CG_ServerCommand();
		}
	}
}
