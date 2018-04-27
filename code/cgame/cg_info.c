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

// cg_info.c -- display information while data is being loading

#include "cg_local.h"


static const char *MonthAbbrev[] = {
	"Jan","Feb","Mar",
		"Apr","May","Jun",
		"Jul","Aug","Sep",
		"Oct","Nov","Dec"
};

static const char *DayAbbrev[] = {
	"Sun",
		"Mon","Tue","Wed",
		"Thu","Fri","Sat"
};

void CG_ETF_DemoParseString(const char* in, char* out, int size) {
	qtime_t time;
	char tmp[2] = {0, 0};
	const char* p;
	char* c;
	int yr;

	trap_RealTime(&time);

	out[0] = 0;
	yr = time.tm_year;
	while(yr >= 100)
		yr-=100;

	for(p = in; *p; p++) {
		if(*p == '$') {
			p++;
			switch(*p) {
				case '\0':
					p--;
					break;
				case 'M':
					Q_strcat(out, size, MonthAbbrev[time.tm_mon]);
					break;
				case 'D':
					Q_strcat(out, size, DayAbbrev[time.tm_wday]);
					break;
				case 'Y':
					Q_strcat(out, size, va("%i", time.tm_year + 1900));
					break;

				case 'a':
					Q_strcat(out, size, time.tm_mon+1 >= 10 ? va("%i", time.tm_mon+1) : va("0%i", time.tm_mon+1));
					break;
				case 'd':
					Q_strcat(out, size, time.tm_mday >= 10 ? va("%i", time.tm_mday) : va("0%i", time.tm_mday));
					break;
				case 'y':
					Q_strcat(out, size, yr >= 10 ? va("%i", yr) : va("0%i", yr));
					break;

				case 'm':
					Q_strcat(out, size, time.tm_min >= 10 ? va("%i", time.tm_min) : va("0%i", time.tm_min));
					break;
				case 's':
					Q_strcat(out, size, time.tm_sec >= 10 ? va("%i", time.tm_sec) : va("0%i", time.tm_sec));
					break;
				case 'h':
					Q_strcat(out, size, time.tm_hour >= 10 ? va("%i", time.tm_hour) : va("0%i", time.tm_hour));
					break;

				case 'l':
					if(cgs.mapInfoLoaded && *cgs.mapinfo.mapName) {
						char buffer[ MAX_STRING_CHARS ];
						char* mp;

						Q_strncpyz( buffer, cgs.mapinfo.mapName, sizeof( buffer ) );

						for( mp = buffer; *mp; mp++ ) {
							if( *mp == ':' || *mp == '\\' || *mp == '/' || *mp == '|' || *mp == '<' || *mp == '>' || *mp == '"' || *mp == '?' || *mp == ' ' ) {
								*mp = '_';
							}
						}

						Q_strcat( out, size, buffer );
					} else {
						Q_strcat(out, size, "Unknown Map");
					}
					break;

				case '$':
					tmp[0] = '$';
					Q_strcat(out, size, tmp);
					break;
			}
		} else {
			tmp[0] = *p;
			Q_strcat(out, size, tmp);
		}
	}

	for(c = out; *c; c++) {
		if(*c == '/' || *c == '\\') {
			*c = '_';
		}
	}
}

void CG_MatchLogAddLine( const char *line ) {
	//char endline = '\n';
	if (cg.matchLogFileHandle <= 0)
		return;
	trap_FS_Write(line, strlen(line), cg.matchLogFileHandle);
}

void CG_MatchLog_f( void ) {
	char buf[MAX_QPATH];
	fileHandle_t copyHandle, bakHandle;
	int i;

	trap_Argv(1, buf, sizeof(buf));
	if (!Q_stricmp(buf, "start")) {
		if (cg.matchLogFileHandle <= 0) {
			trap_Argv( 2, buf, sizeof(buf) );
			if(!buf[0]) {
				Q_strncpyz( buf, "$l-$d_$M_$Y-$h_$m_$s.txt", sizeof(buf));
			}
			CG_ETF_DemoParseString(buf, cg.matchLogFileName, sizeof(cg.matchLogFileName));
			Q_strncpyz( buf, cg.matchLogFileName, sizeof(buf));
			Q_strncpyz( cg.matchLogFileName, "matchlogs/",sizeof(cg.matchLogFileName));
			Q_strcat( cg.matchLogFileName, sizeof( cg.matchLogFileName), buf);
		} else {
			trap_FS_FCloseFile( cg.matchLogFileHandle );
			cg.matchLogFileHandle = 0;
		}
		i = trap_FS_FOpenFile( cg.matchLogFileName, &copyHandle, FS_READ );
		if (i > 0) {
			Q_strncpyz( buf, cg.matchLogFileName, sizeof(buf) );
			Q_strcat( buf, sizeof(buf), ".bak");
			trap_FS_FOpenFile( buf, &bakHandle, FS_WRITE );
			if (bakHandle > 0 ) {
				Com_Printf( "Creating matchlog backup to %s\n", buf);
				while (i > (int)sizeof(buf) ) {
					trap_FS_Read( buf, sizeof(buf), copyHandle );
					trap_FS_Write( buf, sizeof(buf), bakHandle );
					i -= sizeof( buf );
				}
				if ( i > 0 ) {
					trap_FS_Read( buf, i, copyHandle );
					trap_FS_Write( buf, i, bakHandle );
				}
				trap_FS_FCloseFile( bakHandle );
			}
		}
		if ( copyHandle > 0 )
			trap_FS_FCloseFile( copyHandle );
		trap_FS_FOpenFile( cg.matchLogFileName, &cg.matchLogFileHandle, FS_WRITE );
		if ( cg.matchLogFileHandle <= 0 ) {
			cg.matchLogFileHandle = 0;
			Com_Printf( "Failed to open matchlog %s\n", cg.matchLogFileName);
		} else {
			int index = cgs.gameindex < MAX_GAMEINDICIES ? cgs.gameindex : 0;
			Com_Printf( "Matchlog start to %s\n", cg.matchLogFileName);
			CG_MatchLogAddLine("matchlog start\n");
			CG_MatchLogAddLine(va( "info map %s %s\n", 
				cgs.mapinfo.mapLoadName ? cgs.mapinfo.mapLoadName : "etf_mapunkown",
				cgs.mapinfo.mapName ? cgs.mapinfo.mapName : "Unkown map"
			));
			if (cgs.mapinfo.gameIndiciesInfo[index].name[0])
				CG_MatchLogAddLine(va( "info gameindex %s\n", cgs.mapinfo.gameIndiciesInfo[index].name));
			if (cgs.mapinfo.gameIndiciesInfo[index].description)
				CG_MatchLogAddLine(va( "info description %s\n", cgs.mapinfo.gameIndiciesInfo[index].description));
			CG_ETF_DemoParseString( "info date $d $M $Y\n", buf, sizeof(buf));
			CG_MatchLogAddLine(buf);
			CG_ETF_DemoParseString( "info time $h:$m:$s\n", buf, sizeof(buf));
			CG_MatchLogAddLine(buf);
		}
	} else if (!Q_stricmp(buf, "stop")) {
		if ( cg.matchLogFileHandle <= 0 ) {
            Com_Printf( "Match logging isn't active.\n");
		} else {
			CG_MatchLogAddLine("matchlog stop\n");
			trap_FS_FCloseFile( cg.matchLogFileHandle );
			cg.matchLogFileHandle = 0;
            Com_Printf( "Matchlog stopped to %s\n", cg.matchLogFileName);
		}
	}
}

void CG_ScoresDown_f( void );

// Demo playback key catcher support
void CG_DemoClick( int key, qboolean down ) {
	int milli = trap_Milliseconds();

	// Avoid active console keypress issues
	if ( !down && !cgs.fKeyPressed[key] ) {
		return;
	}

	cgs.fKeyPressed[key] = down;

	switch ( key )
	{
	case K_ESCAPE:
		//CG_ShowHelp_Off( &cg.demohelpWindow );
		CG_keyOff_f();
		return;

	case K_TAB:
		if ( demo_scoresToggle.integer ) {
			if ( down )
				CG_ScoresDown_f();
		} else {
			if( down )
				CG_ScoresDown_f();
			else
				CG_ScoresDown_f();
		}
		//	CG_ScoresDown_f();
		//} else { CG_ScoresDown_f();}
		return;

		// Help info
	case K_BACKSPACE:
		if ( !down ) {
			/*if ( cg.demohelpWindow != SHOW_ON ) {
				CG_ShowHelp_On( &cg.demohelpWindow );
			} else {
				CG_ShowHelp_Off( &cg.demohelpWindow );
			}*/
		}
		return;

		// Screenshot keys
	case K_F11:
		if ( !down ) {
			trap_SendConsoleCommand( "screenshot_etf\n" );
			//trap_SendConsoleCommand( va( "screenshot%s\n", ( ( cg_useScreenshotJPEG.integer ) ? "JPEG" : "" ) ) );
		}
		return;
	case K_F12:
		if ( !down ) {
			trap_SendConsoleCommand( "screenshotJPEG_etf\n" );
			//CG_autoScreenShot_f();
		}
		return;

	case K_HOME:
		if ( !down ) {
			trap_Cvar_Set( "cg_draw2D", ( ( cg_draw2D.integer == 0 ) ? "1" : "0" ) );
		}
		return;

	case K_END:
		if ( !down ) {
			trap_Cvar_Set( "cg_drawGun", ( ( cg_drawGun.integer == 0 ) ? "1" : "0" ) );
		}
		return;

	case K_MOUSE3:
		// DO NOTHING to prevent accidental presses
		return;

		// Window controls
#if 0
	case K_SHIFT:
	case K_CTRL:
	case K_MOUSE4:
		cgs.fResize = down;
		return;
	case K_MOUSE1:
		cgs.fSelect = down;
		return;
	case K_MOUSE2:
		if ( !down ) {
			CG_mvSwapViews_f();             // Swap the window with the main view
		}
		return;
	case K_INS:
	case K_KP_PGUP:
		if ( !down ) {
			CG_mvShowView_f();              // Make a window for the client
		}
		return;
	case K_DEL:
	case K_KP_PGDN:
		if ( !down ) {
			CG_mvHideView_f();              // Delete the window for the client
		}
		return;
	case K_MOUSE3:
		if ( !down ) {
			CG_mvToggleView_f();            // Toggle a window for the client
		}
		return;
#endif

		// Third-person controls
	case K_ENTER:
		if ( !down ) {
			trap_Cvar_Set( "cg_thirdperson", ( ( cg_thirdPerson.integer == 0 ) ? "1" : "0" ) );
		}
		return;
	case K_UPARROW:
		if ( milli > cgs.thirdpersonUpdate ) {
			float range = cg_thirdPersonRange.value;

			cgs.thirdpersonUpdate = milli + DEMO_THIRDPERSONUPDATE;
			range -= ( ( range >= 4 * DEMO_RANGEDELTA ) ? DEMO_RANGEDELTA : ( range - DEMO_RANGEDELTA ) );
			trap_Cvar_Set( "cg_thirdPersonRange", va( "%f", range ) );
		}
		return;
	case K_DOWNARROW:
		if ( milli > cgs.thirdpersonUpdate ) {
			float range = cg_thirdPersonRange.value;

			cgs.thirdpersonUpdate = milli + DEMO_THIRDPERSONUPDATE;
			range += ( ( range >= 120 * DEMO_RANGEDELTA ) ? 0 : DEMO_RANGEDELTA );
			trap_Cvar_Set( "cg_thirdPersonRange", va( "%f", range ) );
		}
		return;
	case K_RIGHTARROW:
		if ( milli > cgs.thirdpersonUpdate ) {
			float angle = cg_thirdPersonAngle.value - DEMO_ANGLEDELTA;

			cgs.thirdpersonUpdate = milli + DEMO_THIRDPERSONUPDATE;
			if ( angle < 0 ) {
				angle += 360.0f;
			}
			trap_Cvar_Set( "cg_thirdPersonAngle", va( "%f", angle ) );
		}
		return;
	case K_LEFTARROW:
		if ( milli > cgs.thirdpersonUpdate ) {
			float angle = cg_thirdPersonAngle.value + DEMO_ANGLEDELTA;

			cgs.thirdpersonUpdate = milli + DEMO_THIRDPERSONUPDATE;
			if ( angle >= 360.0f ) {
				angle -= 360.0f;
			}
			trap_Cvar_Set( "cg_thirdPersonAngle", va( "%f", angle ) );
		}
		return;

	case 'p':
		if ( !down ) {
			if ( cgs.demoPaused ) {
				cgs.demoPaused = qfalse;
				trap_Cvar_Set( "cl_freezedemo", "0" ); // Unpause
				if( cgs.oldtimescale != cg_timescale.value ) {
					if ( cgs.oldtimescale >= 0.1f ) {
						trap_Cvar_Set( "timescale", va( "%f", cgs.oldtimescale ) );
						cgs.timescaleUpdate = cg.time + (int)( 1000.0f * cgs.oldtimescale );
					} else {
						trap_Cvar_Set( "timescale", "1" );
						cgs.timescaleUpdate = cg.time + 1000;
					}
				}
			} else {
				cgs.oldtimescale = cg_timescale.value;
				cgs.demoPaused = qtrue;
				trap_Cvar_Set( "cl_freezedemo", "1" );
				// Try to stop the demo as much as possible.. it will continue to play in the background ever so slowly...
				trap_Cvar_Set( "timescale", "0.00000001" );
				cgs.timescaleUpdate = cg.time + 1000; // Not really used in this case
			}
		}
		return;

		// Timescale controls
	case K_KP_5:
	case K_KP_INS:
	case K_SPACE:
		if ( !down ) {
			cgs.demoPaused = qfalse;
			trap_Cvar_Set( "cl_freezedemo", "0" ); // Unpause
			trap_Cvar_Set( "timescale", "1" );
			cgs.timescaleUpdate = cg.time + 1000;
		}
		return;
	case K_KP_DOWNARROW:
		if ( !down ) {
			float tscale = cg_timescale.value;

			if ( cgs.demoPaused && cgs.oldtimescale >= 0.1f )
				tscale = cgs.oldtimescale;

			if ( tscale <= 1.1f ) {
				if ( tscale > 0.1f ) {
					tscale -= 0.1f;
				}
			} else { tscale -= 1.0;}
			cgs.demoPaused = qfalse;
			trap_Cvar_Set( "cl_freezedemo", "0" ); // Unpause
			trap_Cvar_Set( "timescale", va( "%f", tscale ) );
			cgs.timescaleUpdate = cg.time + (int)( 1000.0f * tscale );
		}
		return;
	case K_MWHEELDOWN:
		if ( !cgs.fKeyPressed[K_SHIFT] ) {
			if ( !down ) {
				//CG_ZoomOut_f();
			}
			return;
		}       // Roll over into timescale changes
	case K_KP_LEFTARROW:
		if ( !down ) {
			float tscale;

			if ( cgs.demoPaused && cgs.oldtimescale >= 0.1f )
				tscale = cgs.oldtimescale;
			else
				tscale = cg_timescale.value;

			if ( tscale > 0.1f ) {
				cgs.demoPaused = qfalse;
				trap_Cvar_Set( "cl_freezedemo", "0" ); // Unpause
				trap_Cvar_Set( "timescale", va( "%f", tscale - 0.1f ) );
				cgs.timescaleUpdate = cg.time + (int)( 1000.0f * tscale - 0.1f );
			}
		}
		/*if ( !down && cg_timescale.value > 0.1f ) {
			cgs.demoPaused = qfalse;
			trap_Cvar_Set( "cl_freezedemo", "0" ); // Unpause
			trap_Cvar_Set( "timescale", va( "%f", cg_timescale.value - 0.1f ) );
			cgs.timescaleUpdate = cg.time + (int)( 1000.0f * cg_timescale.value - 0.1f );
		}*/
		return;
	case K_KP_UPARROW:
		if ( !down ) {
			float tscale;

			if ( cgs.demoPaused && cgs.oldtimescale >= 0.1f )
				tscale = cgs.oldtimescale;
			else
				tscale = cg_timescale.value;

			cgs.demoPaused = qfalse;
			trap_Cvar_Set( "cl_freezedemo", "0" ); // Unpause
			trap_Cvar_Set( "timescale", va( "%f", tscale + 1.0f ) );
			cgs.timescaleUpdate = cg.time + (int)( 1000.0f * tscale + 1.0f );
		}
		return;
	case K_MWHEELUP:
		if ( !cgs.fKeyPressed[K_SHIFT] ) {
			if ( !down ) {
				//CG_ZoomIn_f();
			}
			return;
		}       // Roll over into timescale changes
	case K_KP_RIGHTARROW:
		if ( !down ) {
			float tscale;

			if ( cgs.demoPaused && cgs.oldtimescale >= 0.1f )
				tscale = cgs.oldtimescale;
			else
				tscale = cg_timescale.value;

			cgs.demoPaused = qfalse;
			trap_Cvar_Set( "cl_freezedemo", "0" ); // Unpause
			trap_Cvar_Set( "timescale", va( "%f", tscale + 0.1f ) );
			cgs.timescaleUpdate = cg.time + (int)( 1000.0f * tscale + 0.1f );
		}
		return;

		// AVI recording controls
	case K_F1:
		if ( down ) {
			cgs.aviDemoRate = demo_avifpsF1.integer;
		} else { trap_Cvar_Set( "cl_avidemo", demo_avifpsF1.string );}
		return;
	case K_F2:
		if ( down ) {
			cgs.aviDemoRate = demo_avifpsF2.integer;
		} else { trap_Cvar_Set( "cl_avidemo", demo_avifpsF2.string );}
		return;
	case K_F3:
		if ( down ) {
			cgs.aviDemoRate = demo_avifpsF3.integer;
		} else { trap_Cvar_Set( "cl_avidemo", demo_avifpsF3.string );}
		return;
	case K_F4:
		if ( down ) {
			cgs.aviDemoRate = demo_avifpsF4.integer;
		} else { trap_Cvar_Set( "cl_avidemo", demo_avifpsF4.string );}
		return;
	case K_F5:
		if ( down ) {
			cgs.aviDemoRate = demo_avifpsF5.integer;
		} else { trap_Cvar_Set( "cl_avidemo", demo_avifpsF5.string );}
		return;
	case K_F6:
		if ( down ) {
			cgs.aviDemoRate = demo_avifpsF6.integer;
		} else { trap_Cvar_Set( "cl_avidemo", demo_avifpsF6.string );}
		return;
	}
}
