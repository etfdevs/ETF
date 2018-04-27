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

/*
**	cg_q3f_menu.c
**
**	The UI menu system seems a little hard to use - there's no direct communication between
**	game/cgame and UI, as far as I can tell (i.e. to tell the UI to throw up menu X). Also,
**	We want these things to be purely text-based. (so as not to interfere with panicked
**	fights).
*/

#include "cg_q3f_menu.h"
#include "cg_local.h"
#include "../game/bg_q3f_playerclass.h"
#include "cg_q3f_mapselect.h"

static int menustate, menutime, menuchoice, menuchoicetime, menuslidetime;
static qboolean isinvismenu;	// hack for supplystation menu

typedef enum { 
	MOS_INACTIVE,
	MOS_ACTIVE,
	MOS_DISABLED,
} menuoption_state_t;

static char menustrings[10][128]; 
static char menustrings2[10][128];
static char menucommands[10][128];
static menuoption_state_t menuoptionstate[10];

static char menuname[20];
static char menu_menuname[20];

static float menurange;
static vec3_t menulocation;

void CG_Q3F_MenuInit()
{
	// Reset a menu to 'off', and clear out the buffers

	if( menustate == Q3F_MENUSTATE_READY ) {
		menuchoicetime	= cg.time + Q3F_MENU_CHOICETIME;	// Prevent choice if menu changes
	}
	if( menustate == Q3F_MENUSTATE_NONE ) {
//		menuslidetime = cg.time + cg_menuSlideTime.integer;
	}

	menustate = Q3F_MENUSTATE_READY;
	memset( menuoptionstate,	0, sizeof(menuoptionstate)	);
	memset( menustrings,		0, sizeof(menustrings)		);
	memset( menustrings2,		0, sizeof(menustrings2)		);
	memset( menucommands,		0, sizeof(menucommands)		);
	memset( menu_menuname,		0, sizeof(menu_menuname)	);
	menurange = 0;
	isinvismenu = qfalse;

	if (cgs.eventHandling == CGAME_EVENT_NONE) {
		CG_EventHandling(CGAME_EVENT_MENUMODE, qfalse);
	}
}

void CG_Q3F_MenuAddOption(int num, char *text, char *text2, char *command) {
	if( num < 0 || num > 9 ) {
		return;
	}

	Q_strncpyz( menustrings[num], text, 128 );
	Q_strncpyz( menucommands[num], command, 128 );
	if( text2 ) {
		Q_strncpyz( menustrings2[num], text2, 128 );
	}

	menuoptionstate[num] = MOS_ACTIVE;
}

void CG_Q3F_MenuSetDisabled(int num) {
	if( num < 0 || num > 9 ) {
		return;
	}

	menuoptionstate[num] = MOS_DISABLED;
}

void CG_Q3F_MenuCancel( qboolean command ) {
	// 'Cancel' a menu

	char cmdbuff[20];
	int index;

	if( menustate == Q3F_MENUSTATE_FADING || menustate == Q3F_MENUSTATE_NONE ) {
		return;			// Already ended
	}

	if( command && trap_Argc() > 2 ) {
		for( index = trap_Argc() - 1; index >= 2; index-- ) {
			trap_Argv( 2, cmdbuff, 20 );
			if( !Q_stricmp( cmdbuff, menuname ) )
				break;		// Not the current menu
		}
		if( index < 2 ) {
			return;			// We tried all the names without a break, cancel
		}
	}
	*menuname = 0;		// No more menu name

	menustate	= Q3F_MENUSTATE_FADING;
	menuchoice	= -1;
	menutime	= cg.time;
	menurange	= 0;

	if (cgs.eventHandling == CGAME_EVENT_MENUMODE) {
		CG_EventHandling(CGAME_EVENT_NONE, qfalse);
	}

	// stop using supplystation
	cg.usingSupplystation = 0;
}

qboolean CG_Q3F_MenuChoice( int choice )
{
	// 'choose' a menu option from a weapon option... returns
	// qtrue if it selected something, or qfalse to indicate
	// continue processing.

	if( CG_Q3F_MapSelectChoice( choice ) ) {	// Map select nabbed it
		return( qtrue );
	}
	if( menustate != Q3F_MENUSTATE_READY ) {
		return( qfalse );			// Not taking commands
	}
	if( choice < 1 || choice > 10 || !*menucommands[choice-1] ){
		return( qfalse );			// Not a valid choice
	}
	if( menuchoicetime > cg.time ) {
		return( qtrue );			// Prevent selecting the wrong choice on another menu
	}

	menustate		= Q3F_MENUSTATE_FADING;
	menutime		= cg.time;
	menuchoicetime	= cg.time + Q3F_MENU_CHOICETIME;
	menuchoice		= choice - 1;

	if (cgs.eventHandling == CGAME_EVENT_MENUMODE) {
		CG_EventHandling(CGAME_EVENT_NONE, qfalse);
	}

	if( !Q_stricmp( "cancel", menucommands[menuchoice] ) ) {
		CG_Q3F_MenuCancel( qfalse );
	} else if ( !Q_stricmp( "spectate", menucommands[menuchoice] ) ) {
		CG_Q3F_StartSpectate();
	} /* else if ( !Q_stricmp( "spectatemenu", menucommands[menuchoice] ) ) {
		CG_Q3F_SpectatorTypeMenu();
	} */ else if ( !Q_stricmp( "flyby", menucommands[menuchoice] ) ) {
		CG_Q3F_Flyby();
	} else { 
		trap_SendConsoleCommand( va("%s\n", menucommands[menuchoice]) );
	}

	// hack for supplystation
	if( cg.usingSupplystation == 1 || cg.usingSupplystation == 3 ) {
		cg.usingSupplystation = 2;
	}

	return( qtrue );
}

void CG_Q3F_MenuNumCommand( void ) {
	// 'menucmd' has been invoked, get the number and send to MenuChoice

	char buff[3];
	int num;

	trap_Argv( 1, buff, sizeof(num) );
	num = atoi( buff );
	CG_Q3F_MenuChoice( num );
}

/*
**	Menu setups
*/

static char colourcodes[4] = { COLOR_RED, COLOR_BLUE, COLOR_YELLOW, COLOR_GREEN };
static void TeamMenu() {
	// Throw up a list of teams

	int numteams, index;
	char buffer[128];
	int count, i;
	clientInfo_t* ci;

	if(!cg_oldSkoolMenu.integer) {
		HUD_Setup_Menu("tab_chooseteam");
		trap_UI_Popup( 2 );		//2 = UIMENU_INGAME
//		trap_SendConsoleCommand( "hud_ingame\n" );
		return;
	}
	
	CG_Q3F_MenuInit();
	numteams = trap_Argc() - 2;

	for( index = 0; index < numteams; index++ ) {		
		trap_Argv( index + 2, buffer, 126 );
		if( *buffer ) {
			char buf[32];
			char teamnamebuffer[64];
			count = 0;
			for ( i = 0 ; i < MAX_CLIENTS ; i++ ) {
				ci = &cgs.clientinfo[i];
				if ( !ci->infoValid ) {
					continue;
				}
				if ( ci->team == Q3F_TEAM_RED + index ) {
					count++;
				}
			}

         //keeg:  fixed the com_sprintf line to use a string not a char, and the menu add option to reference 
         //the right team name string...
			trap_Cvar_VariableStringBuffer(va("cg_%steam", teamnames[index]), teamnamebuffer, 64);
			Com_sprintf(buf, sizeof(buf), "^%c%s", colourcodes[index], teamnamebuffer);
			CG_Q3F_MenuAddOption(index, buf, va("^%c%i", colourcodes[index], count), va("team %s\n", teamnames[index]));
		} else {
			CG_Q3F_MenuSetDisabled(index);
		}
	}

	for( ; index < 4; index++ ) {
		CG_Q3F_MenuSetDisabled(index);
	}

	// if they REALLY can't decide
	CG_Q3F_MenuAddOption(5, "AUTO JOIN", NULL,	"team auto\n");
	CG_Q3F_MenuAddOption(6, "SPECTATE", NULL,	"team s\n");

	if( cgs.flybyPathIndex >= 0 && !cg.renderingFlyBy ) {
		CG_Q3F_MenuAddOption(7, "FLYBY", NULL,	"flyby");
	}

	CG_Q3F_MenuAddOption(9, "CANCEL", NULL,	"cancel");

	Q_strncpyz(menu_menuname, "menubox_changeteam", 20);
}

void CG_Q3F_StartSpectate(void) {
	CG_Q3F_MenuCancel( qfalse );
	trap_SendConsoleCommand( "team s; wait; stopfollow\n" );
}

static void SpectatorTeamMenu() {
	// Throw up a list of teams

	int numteams, index;
	qboolean isAdmin;
	char buff[2];
	char buffer[128];
	int count, i;
	clientInfo_t* ci;

	if(!cg_oldSkoolMenu.integer) {
		// call up our initial class menu
		HUD_Setup_Menu("tab_chooseteam");
		trap_UI_Popup( 2 );		//2 = UIMENU_INGAME
//		trap_SendConsoleCommand( "hud_ingame\n" );
		return;
	}
	
	CG_Q3F_MenuInit();
	numteams = trap_Argc() - 3;

	for( index = 0; index < numteams; index++ ) {
		trap_Argv( index + 2, buffer, 126 );
		if( *buffer ) {
//			char buf[32];
			char teamnamebuffer[64];

			count = 0;
			for ( i = 0 ; i < MAX_CLIENTS ; i++ ) {
				ci = &cgs.clientinfo[i];
				if ( !ci->infoValid ) {
					continue;
				}
				if ( ci->team == Q3F_TEAM_RED+index ) {
					count++;
				}
			}

			trap_Cvar_VariableStringBuffer(va("cg_%steam", teamnames[index]), teamnamebuffer, 64);
			CG_Q3F_MenuAddOption(index, teamnamebuffer, va("^%c%i", colourcodes[index], count), va("team %s\n", teamnames[index]));
/*			Com_sprintf( buf, sizeof(buf), "^%c%s", colourcodes[index], teamnames[index] );
			CG_Q3F_MenuAddOption(index, buf, va("^%c%i", colourcodes[index], count), va("team %s", teamnames[index]));*/
		} else {
			CG_Q3F_MenuSetDisabled(index);
		}
	}

	for( ; index < 4; index++ ) {
		CG_Q3F_MenuSetDisabled(index);
	}

	// if they REALLY can't decide
	CG_Q3F_MenuAddOption(4, "AUTO JOIN", NULL,	"team auto\n");


	trap_Argv( trap_Argc() - 1, buff, 2 );
	isAdmin = atoi( buff );

	// if they want to spectate
	if ( g_spectatorMode.value == 0 || ( g_spectatorMode.value == 1 && isAdmin ) ) {
		CG_Q3F_MenuAddOption(5, "SPECTATE", NULL,	"team s;spectatemenu\n");
	} else {
		CG_Q3F_MenuSetDisabled(5);
	}

	if( cgs.flybyPathIndex >= 0 && !cg.renderingFlyBy  ) {
		CG_Q3F_MenuAddOption(6, "FLYBY", NULL,	"flyby");
	} else {
		CG_Q3F_MenuSetDisabled(6);
	}

	Q_strncpyz(menu_menuname, "menubox_specteam", 20);
}

void CG_Q3F_SpectatorTypeMenu() {
	// Throw up a list of teams

	int numteams, index;
	qboolean isAdmin;
	char *charptr;
	char buff[2];
	char teams[4][128];
	char buffer[128];

	CG_Q3F_MenuInit();
	numteams = trap_Argc() - 2;

	for( index = 0; index < numteams; index++ ) {
		trap_Argv( index + 2, teams[index], 128 );
	}

	for( index = 0; index < numteams; index++ )
	{
		// Store each team using the name supplied by the server.

		if( *teams[index]  ) {
			char teamnamebuffer[64];

			trap_Cvar_VariableStringBuffer(va("cg_%steam", teamnames[index]), teamnamebuffer, 64);
			Q_strncpyz( buffer + 6, teamnames[index], 122 );

//			buffer[0] = '^';
//			buffer[1] = colourcodes[index];
			buffer[0] = 'C';
			buffer[1] = 'h';
			buffer[2] = 'a';
			buffer[3] = 's';
			buffer[4] = 'e';
			buffer[5] = ' ';
			for( charptr = buffer; *charptr; charptr++ ) {
				if( *charptr == '~' ) {
					*charptr = ' ';
				}
			}
			CG_Q3F_MenuAddOption(index, teamnamebuffer, NULL, va("chase %s\n", teamnames[index]));
//			CG_Q3F_MenuAddOption(index, buffer, NULL, va("chase %s", teamnames[index] ));
		} else {
			CG_Q3F_MenuSetDisabled(index);
		}
	}

	for( index = 0; index < numteams; index++ ) {
		// Store each team using the name supplied by the server.

		if( *teams[index]  ) {
			char teamnamebuffer[64];

			trap_Cvar_VariableStringBuffer(va("cg_%steam", teamnames[index]), teamnamebuffer, 64);
			Q_strncpyz( buffer + 7, teamnamebuffer, 121 );

//			buffer[0] = '^';
//			buffer[1] = colourcodes[index];
			buffer[0] = 'F';
			buffer[1] = 'o';
			buffer[2] = 'l';
			buffer[3] = 'l';
			buffer[4] = 'o';
			buffer[5] = 'w';
			buffer[6] = ' ';
			for( charptr = buffer; *charptr; charptr++ ) {
				if( *charptr == '~' ) {
					*charptr = ' ';
				}
			}
			CG_Q3F_MenuAddOption(index+4, teamnamebuffer, NULL, va("follow %s\n", teamnames[index]));
//			CG_Q3F_MenuAddOption(index+4, buffer, NULL, va("follow %s", teamnames[index] ));
		} else {
			CG_Q3F_MenuSetDisabled(index+4);
		}
	}

	trap_Argv( trap_Argc() - 1, buff, 2 );
	isAdmin = atoi( buff );

	// if they want to spectate
	if ( g_spectatorMode.value == 0 || ( g_spectatorMode.value == 1 && isAdmin ) ) {
		CG_Q3F_MenuAddOption( 9, "Free Flight", NULL, "spectate");
	} else {
		CG_Q3F_MenuSetDisabled(9);
	}

	Q_strncpyz(menu_menuname, "menubox_specopts", 20);
}

void CG_Q3F_StoreClassinfo() {
	int i, count;
	char allowClasses[11];
	char classLimits[21];
	char currentClasses[21];
	char buffer[128];
	char *p, *c, *s;

	for( i = 0; i < 10; i++ ) {
		allowClasses[i]				= '0';
		classLimits[(i*2)]			= '0';
		classLimits[(i*2) + 1]		= '0';
		currentClasses[(i*2)]		= '0';
		currentClasses[(i*2) + 1]	= '0';
	}
	allowClasses[10]	= '\0';
	classLimits[20]		= '\0';
	currentClasses[20]	= '\0';

	count = trap_Argc();
	if( count <= 0 ) {
		
		trap_Cvar_Set("hud_allowClasses",	allowClasses);
		trap_Cvar_Set("hud_currentClasses", currentClasses);
		trap_Cvar_Set("hud_maxClasses",		classLimits);

		return;			// No info
	}

	for( i = 0; i <= count; i++ ) {
		trap_Argv( i + 1, buffer, 128 );

//		CG_Printf(BOX_PRINT_MODE_CHAT, va("%s\n", buffer));
		
		if(*buffer) {
			allowClasses[i] = '1';
		} else {
			continue;
		}

		for(p = buffer; *p; p++) {
			if(*p == '*') {
				*p++ = '\0';
				
				s = p;
				if(c = strchr(s, '/')) {
					*c++ = '\0';
				}

				currentClasses[(i*2)] =		s[0];
				currentClasses[(i*2)+1] =	s[1];
								
				if(c) {
					classLimits[(i*2)] =	c[0];
					classLimits[(i*2)+1] =	c[1];
				}
				break;
			}
		}
	}

	trap_Cvar_Set("hud_allowClasses",	allowClasses);
	trap_Cvar_Set("hud_currentClasses", currentClasses);
	trap_Cvar_Set("hud_maxClasses",		classLimits);
}

static void ClassMenu() {
	char allowClasses[11];
	char classLimits[21];
	char currentClasses[21];
	int i;
	bg_q3f_playerclass_t *cls;
//	char buffer[128];

	trap_Cvar_VariableStringBuffer("hud_allowClasses",		allowClasses, 11);
	trap_Cvar_VariableStringBuffer("hud_currentClasses",	currentClasses, 21);
	trap_Cvar_VariableStringBuffer("hud_maxClasses",		classLimits, 21);

	if(!cg_oldSkoolMenu.integer) {
		// call up our initial class menu
		HUD_Setup_Menu( "tab_chooseclass");
		trap_UI_Popup( 2 );		//2 = UIMENU_INGAME		
//		trap_SendConsoleCommand( "hud_ingame\n" );
		return;
	}
	
	CG_Q3F_MenuInit();
	Q_strncpyz(menu_menuname, "menubox_changeclass", 20);

	for(i = 0; i < 10; i++) {
		if(allowClasses[i] == '1') {
			char buf[2][3];
			int current, max;

			cls = bg_q3f_classlist[i + Q3F_CLASS_RECON];

			buf[0][0] = currentClasses[(i * 2)];
			buf[0][1] = currentClasses[(i * 2)+1];
			buf[0][2] = '\0';

			buf[1][0] = classLimits[(i * 2)];
			buf[1][1] = classLimits[(i * 2)+1];
			buf[1][2] = '\0';

			current	= atoi(buf[0]);
			max		= atoi(buf[1]);

			CG_Q3F_MenuAddOption( i, cls->title, va(max ? "%i/%i" : "%i", current, max), va("%s\n", cls->commandstring));
		} else {
			CG_Q3F_MenuSetDisabled( i );
		}
	}

	if( allowClasses[9] != '1' && cg.snap->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_NULL ) {
		CG_Q3F_MenuAddOption( 9, "Cancel", NULL, "cancel");
	}
}

static void DisguiseMenu() {
	// Throw up a 'disguise' menu

	CG_Q3F_MenuInit();

	CG_Q3F_MenuAddOption( 0, "Change Class",	NULL, "disguise menu class\n");
	CG_Q3F_MenuAddOption( 1, "Change Team",		NULL, "disguise menu team\n");
	CG_Q3F_MenuAddOption( 2, "Remove disguise",	NULL, "disguise reset\n");
	CG_Q3F_MenuAddOption( 3, ((cg.snap->ps.eFlags & EF_Q3F_INVISIBLE) ? "Deactivate invisibility" : "Activate invisibility"),	NULL, "invisible\n");
	CG_Q3F_MenuAddOption( 9, "Cancel", NULL, "cancel");

	Q_strncpyz(menu_menuname, "menubox_agentopts", 20);
}

static void DisguiseTeamMenu()
{
	// Throw up a list of teams

	int numteams, index;
	char buffer[128];
	int count, i;
	clientInfo_t* ci;

	CG_Q3F_MenuInit();
	numteams = trap_Argc() - 2;

	for( index = 0; index < numteams; index++ )
	{
		trap_Argv( index + 2, buffer, 126 );
		if( *buffer ) {
//			char buf[32];
			char teamnamebuffer[64];

			count = 0;
			for ( i = 0 ; i < MAX_CLIENTS ; i++ ) {
				ci = &cgs.clientinfo[i];
				if ( !ci->infoValid ) {
					continue;
				}
				if ( ci->team == Q3F_TEAM_RED+index ) {
					count++;
				}
			}
			trap_Cvar_VariableStringBuffer(va("cg_%steam", teamnames[index]), teamnamebuffer, 64);
			CG_Q3F_MenuAddOption(index, teamnamebuffer, va("^%c%i", colourcodes[index], count), va("disguise team %s\n", teamnames[index]));
/*			Com_sprintf(buf, sizeof(buf), "^%c%s", colourcodes[index], teamnames[index]);
			CG_Q3F_MenuAddOption(index, buf, va("^%c%i", colourcodes[index], count), va("disguise team %s", teamnames[index]));*/
		} else {
			CG_Q3F_MenuSetDisabled(index);
		}
	}

	// Always allow a cancel menu
	CG_Q3F_MenuAddOption( 9, "Cancel", NULL, "cancel");

	Q_strncpyz(menu_menuname, "menubox_agentteam", 20);
}

static void DisguiseClassMenu()
{
	// Accepts a bitmask of acceptable classes.

	int index, count;
	char buffer[128];

	count = trap_Argc() - 2;
	if( count <= 0 )
		return;			// No class menu

	CG_Q3F_MenuInit();

	for( index = 0; index < count; index++ ) {
		trap_Argv( index + 2, buffer, 128 );
		if( *buffer ) {
			CG_Q3F_MenuAddOption( index, buffer, NULL, va( "disguise class %s\n", bg_q3f_classlist[index+1]->commandstring) );
		} else {
			CG_Q3F_MenuSetDisabled( index );
		}
	}

	// If we don't have a Civilian listed, allow us to Cancel
	if( !*menustrings[9] ) {
		CG_Q3F_MenuAddOption( 9, "Cancel", NULL, "cancel" );
	}

	Q_strncpyz(menu_menuname, "menubox_agentclass", 20);
}

/*static void MapHelpMenu()
{
	// Throw up a 'maphelp' menu
	char buff[1280];
	int line, width;
	char *srcptr, *ptr;
	
	CG_Q3F_MenuInit();

	Q_strncpyz( menutitle, "Map Help", 128 );

	trap_Argv( 2, buff, 1280 );
	srcptr = buff;
	for( line = 0; line < 9 && *srcptr; line++ )
	{
		ptr = menustrings[line];
		width = 0;
		while( *srcptr )
		{
			if( *srcptr == '\\' && *(srcptr+1)== 'n' )
			{
				srcptr += 2;
				break;			// Next line
			}
			if( *srcptr == '\n' )
			{
				srcptr++;
				break;
			}
			if( width < 128 )		// Prevent overflows :)
				*ptr++ = *srcptr;
			srcptr++;
		}
	}

	Q_strncpyz( menustrings[9],		"Close Help", 128 );
	Q_strncpyz( menucommands[9],	"cancel", 128 );
}*/

static unsigned char chargetimes[] = { 5, 10, 20, 30, 60, 120, 180 };
void ChargeMenu()
{
	// Menu options to lay a charge

	int line;

	CG_Q3F_MenuInit();

	for( line = 0; line < sizeof(chargetimes); line++ ) {
		CG_Q3F_MenuAddOption( line, va("%d seconds", chargetimes[line]), NULL, va("charge %d\n", chargetimes[line]) );
	}

	if (cg.snap->ps.stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_LAYCHARGE) ) {
		CG_Q3F_MenuAddOption( 8, "Stop Laying", NULL, "charge cancel\n" );
	}
	else {
		CG_Q3F_MenuSetDisabled( 8 );
	}
	CG_Q3F_MenuAddOption( 9, "Cancel",		NULL, "cancel" );

	Q_strncpyz(menu_menuname, "menubox_charge", 20);
}

static void DropammoMenu()
{
	CG_Q3F_MenuInit();

	CG_Q3F_MenuAddOption(0, "10 Shells", NULL, "dropammo 10 shells\n");
	CG_Q3F_MenuAddOption(1, "Free Shells", NULL, "dropammo shells\n");

	CG_Q3F_MenuAddOption(2, "50 Nails", NULL, "dropammo 50 nails\n");
	CG_Q3F_MenuAddOption(3, "Free Nails", NULL, "dropammo nails\n");

	CG_Q3F_MenuAddOption(4, "10 Rockets", NULL, "dropammo 10 rockets\n");
	CG_Q3F_MenuAddOption(5, "Free Rockets", NULL, "dropammo rockets\n");

	CG_Q3F_MenuAddOption(6, "50 Cells", NULL, "dropammo 50 cells\n");
	CG_Q3F_MenuAddOption(7, "Free Cells", NULL, "dropammo cells\n");

	CG_Q3F_MenuAddOption(9, "Cancel", NULL, "cancel");

	Q_strncpyz(menu_menuname, "menubox_drop", 20);
}

static void ReadyMenu()
{
	// don't show if another menu is already active
	if(menustate == Q3F_MENUSTATE_READY)
		return;

	CG_Q3F_MenuInit();

	CG_Q3F_MenuAddOption(0, "I'm ready", NULL, "ready\n");
	CG_Q3F_MenuAddOption(1, "Not yet ready", NULL, "cancel");

	Q_strncpyz(menu_menuname, "menubox_ready", 20);
}

static void VoteMenu()
{
	CG_Q3F_MenuInit();

	CG_Q3F_MenuAddOption(0, "Vote: Yes", "0", "vote yes\n");
	CG_Q3F_MenuAddOption(1, "Vote: No", "0", "vote no\n");

	CG_Q3F_MenuAddOption(2, "Don't vote", NULL, "cancel");
	Q_strncpyz(menu_menuname, "menubox_vote", 20);
}

void MenuCheckVoteTally() 
{
	const char *buffer;

	buffer = CG_ConfigString(CS_VOTE_YES);
	if(buffer && buffer[0])
		Q_strncpyz(menustrings2[0], buffer, 128);

	buffer = CG_ConfigString(CS_VOTE_NO);
	if(buffer && buffer[0])
		Q_strncpyz(menustrings2[1], buffer, 128);
}

static void BuildMenu()
{
	// Automatically work out what items can be built based on
	// cells and current structures.

	CG_Q3F_MenuInit();

	if( cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF ) {
		CG_Q3F_MenuAddOption( 7, "Destroy autosentry", NULL, "destroy autosentry\n" );
		CG_Q3F_MenuSetDisabled( 0 );
	}
	else if( cg.snap->ps.ammo[AMMO_CELLS] >= 130 ) {
		CG_Q3F_MenuAddOption( 0, "Build autosentry (130 cells)", NULL, "build autosentry\n" );
		CG_Q3F_MenuSetDisabled( 7 );
	} else {
		CG_Q3F_MenuSetDisabled( 0 );
		CG_Q3F_MenuSetDisabled( 7 );
	}

	if( cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF00 ) {
		CG_Q3F_MenuAddOption( 8, "Destroy supply station", NULL, "destroy supplystation\n" );
		CG_Q3F_MenuSetDisabled( 1 );
	}
	else if( cg.snap->ps.ammo[AMMO_CELLS] >= 100 ) {
		CG_Q3F_MenuAddOption( 1, "Build supply station (100 cells)", NULL, "build supplystation\n" );
		CG_Q3F_MenuSetDisabled( 8 );
	} else {
		CG_Q3F_MenuSetDisabled( 1 );
		CG_Q3F_MenuSetDisabled( 8 );
	}

	CG_Q3F_MenuAddOption( 9, "Cancel", NULL, "cancel" );

	Q_strncpyz(menu_menuname, "menubox_build", 20);
}

qboolean CG_Q3F_ShowingSentryUpgradeMenu()
{
	// Return true if showing an 'upgrade sentry' menu.

	if( menustate == Q3F_MENUSTATE_READY && !strcmp( "upgradeautosentry", menuname ) )
		return( qtrue );
	return( qfalse );
}

qboolean CG_Q3F_ShowingSupplyStationUpgradeMenu()
{
	// Return true if showing an 'upgrade supply station' menu.

	if( menustate == Q3F_MENUSTATE_READY && !strcmp( "upgradesupplystation", menuname ) && cg.usingSupplystation == 3 )
		return( qtrue );
	return( qfalse );
}

static void UpgradeSentryMenu()
{
	// upgrade options for the sentry.

	char buff[5];
	int id/*, health, nails, rockets, level*/;
	centity_t *cent;

	if( trap_Argc() < 3 )
		return;
	trap_Argv( 2, buff, 5 );
	id = atoi( buff );
/*	trap_Argv( 3, buff, 5 );
	health = atoi( buff );
	trap_Argv( 4, buff, 5 );
	nails = atoi( buff );
	trap_Argv( 5, buff, 5 );
	rockets = atoi( buff );
	trap_Argv( 6, buff, 5 );
	level = atoi( buff );*/

	cent = &cg_entities[id];
	if( !cent->currentValid ) {
		return;
	}
	if( Distance( cg_entities[cg.snap->ps.clientNum].lerpOrigin, cent->lerpOrigin ) > 80 ) {
		return;		// Already out of range
	}

	CG_Q3F_MenuInit();
	VectorCopy( cent->lerpOrigin, menulocation );
	menurange = 80;

/*	if( level >= 3 ) {
		CG_Q3F_MenuAddOption( 0, va( "^%cL%d, %d health, %d nail%s, %d rockets", COLOR_YELLOW, level, health, nails, (nails == 1 ? "" : "s"), rockets, (rockets == 1 ? "" : "s") ), NULL, "");
	} else {
		CG_Q3F_MenuAddOption( 0, va( "^%cL%d, %d health, %d bullet%s", COLOR_YELLOW, level, health, nails, (nails == 1 ? "" : "s") ), NULL, "");		
	}*/



//	CG_Q3F_MenuAddOption( 2, "Repair autosentry", NULL, va( "build repair %d\n", id ));
//	CG_Q3F_MenuAddOption( 3, "Refill autosentry", NULL, va( "build refill %d\n", id ));
	CG_Q3F_MenuAddOption( 0, "Aim left", NULL, va( "build rotate 45 %d\n", id ));
	CG_Q3F_MenuAddOption( 1, "Aim right", NULL, va( "build rotate -45 %d\n", id ));
	CG_Q3F_MenuAddOption( 2, "Turn around", NULL, va( "build rotate 180 %d\n", id ));

	CG_Q3F_MenuAddOption( 4, "Dismantle sentry", NULL, va( "build dismantle %d\n", id ));

	CG_Q3F_MenuAddOption( 9, "Cancel", NULL, "cancel");

	Q_strncpyz(menu_menuname, "menubox_upgrade", 20);
}

static void SupplyStationMenu()
{
	// Throw some options for users to select

	char buff[5];
	int id/*, shells, nails, rockets, cells, armour*/;
	centity_t *cent;

	if( trap_Argc() < 3 )
		return;
	trap_Argv( 2, buff, 5 );
	id = atoi( buff );

	cent = &cg_entities[id];
	if( !cent->currentValid )
		return;
	if( Distance( cg_entities[cg.snap->ps.clientNum].lerpOrigin, cent->lerpOrigin ) > 80 )
		return;		// Already out of range

	CG_Q3F_MenuInit();
	VectorCopy( cent->lerpOrigin, menulocation );
	menurange = 80;

	Q_strncpyz( menucommands[0], va( "supply ammo %d", id ), 128 );
	Q_strncpyz( menucommands[1], va( "supply armor %d", id ), 128 );
	Q_strncpyz( menucommands[2], va( "supply grenade %d", id ), 128 );

	// Start using supplystation
	cg.usingSupplystation = 1;

	isinvismenu = qtrue;
}

static void UpgradeSupplyStationMenu()
{
	// upgrade options for the supply station.

	char buff[5];
	int id/*, health, shells, nails, rockets, cells, armour*/;
	centity_t *cent;

	if( trap_Argc() < 3 )
		return;
	trap_Argv( 2, buff, 5 );
	id = atoi( buff );

	cent = &cg_entities[id];
	if( !cent->currentValid )
		return;
	if( Distance( cg_entities[cg.snap->ps.clientNum].lerpOrigin, cent->lerpOrigin ) > 50 )
		return;		// Already out of range

	CG_Q3F_MenuInit();
	VectorCopy( cent->lerpOrigin, menulocation );
	menurange = 50;

//	Q_strncpyz( menucommands[0], va( "build repair %d", id ), 128 );
//	Q_strncpyz( menucommands[1], va( "build refill %d", id ), 128 );
//	Q_strncpyz( menucommands[2], va( "build dismantle %d", id ), 128 );

	Q_strncpyz( menucommands[0], va( "build dismantle %d", id ), 128 );

	// Start upgrading supplystation
	cg.usingSupplystation = 3;

	isinvismenu = qtrue;
}


/*
** Automatically invoke a menu if we're in an intermediate state
*/

void CG_Q3F_AutoTeamMenu()
{
	if ( cg.renderingFlyBy || cg.scoreBoardShowing ) 
	{
		cg.nextMenuAutoTime = cg.time + 1000;
		return;
	}
	
	if( cg.nextMenuAutoTime >= 0 && cg.nextMenuAutoTime <= cg.time  &&
		cg.snap && cg.snap->ps.pm_type != PM_ADMINSPECTATOR ) 
	{
		if( cg.snap->ps.persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR ) {
			if (!( cg.snap->ps.persistant[PERS_FLAGS] & PF_JOINEDTEAM) ) {
				CG_LowPriority_Printf(BOX_PRINT_MODE_CENTER, "Press ESC to join a team");
			} 
		} else if( cg.snap->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL ) {
			CG_LowPriority_Printf(BOX_PRINT_MODE_CENTER, "Press ESC to choose a class");
		}
		cg.nextMenuAutoTime = cg.time + 4000;
	}
}

/*
**	The 'dispatcher' function
*/

void CG_Q3F_MenuCommand()
{
	// The server has sent a 'menu' command

	char cmdbuff[32];

	// Stop using supplystation (bit hackish)
	cg.usingSupplystation = 0;

	trap_Argv( 1, cmdbuff, 32 );
	if( !Q_stricmp( "team", cmdbuff ) ) {
		TeamMenu();
	} else if( !Q_stricmp( "spectatorteam", cmdbuff ) ) {
		SpectatorTeamMenu();
	} else if( !Q_stricmp( "spectatortype", cmdbuff ) ) {
		CG_Q3F_SpectatorTypeMenu();
	} else if( !Q_stricmp( "class", cmdbuff ) ) {
		ClassMenu();
	} else if( !Q_stricmp( "cancel", cmdbuff ) ) {
		CG_Q3F_MenuCancel( qtrue );
	} else if( !Q_stricmp( "disguise", cmdbuff ) ) {
		DisguiseMenu();
	} else if( !Q_stricmp( "disguiseclass", cmdbuff ) ) {
		DisguiseClassMenu();
	} else if( !Q_stricmp( "disguiseteam", cmdbuff ) ) {
		DisguiseTeamMenu();
/*	} else if( !Q_stricmp( "maphelp", cmdbuff ) ) {
		MapHelpMenu();*/
	} else if( !Q_stricmp( "charge", cmdbuff ) ) {
		ChargeMenu();
	} else if( !Q_stricmp( "build", cmdbuff ) ) {
		BuildMenu();
	} else if( !Q_stricmp( "upgradeautosentry", cmdbuff ) ) {
		UpgradeSentryMenu();
	} else if( !Q_stricmp( "supply", cmdbuff ) ) {
		SupplyStationMenu();
	} else if( !Q_stricmp( "upgradesupplystation", cmdbuff ) ) {
		UpgradeSupplyStationMenu();
	} else if( !Q_stricmp( "dropammo", cmdbuff ) ) {
		DropammoMenu();
	} else if( !Q_stricmp( "ready", cmdbuff ) ) {
		ReadyMenu();
	} else if( !Q_stricmp( "vote", cmdbuff ) ) {
		VoteMenu();
	} 
	else {
		CG_Printf( BOX_PRINT_MODE_CHAT, "Unknown menu command '%s'.\n", cmdbuff );
		return;
	}
	Q_strncpyz( menuname, cmdbuff, 20 );	// Set the current menu for reference
}

void CG_Q3F_DrawMenuBox() {
	float alpha = 1.f;
	menuDef_t *menu;

	if( menustate == Q3F_MENUSTATE_NONE ) {
		return;
	}

	// Don't bother rendering during demo playback (as cgame_event_demo takes precedence in terms of key nabbing)
	if( !isinvismenu && cg.demoPlayback ) {
		return;
	}

	if( menustate == Q3F_MENUSTATE_READY && menurange ) {
		// Check we're still in range
		if( Distance( menulocation, cg_entities[cg.snap->ps.clientNum].lerpOrigin ) > menurange ) {
			CG_Q3F_MenuCancel( qfalse );
		}
	}

	if( menustate == Q3F_MENUSTATE_FADING ) {
		alpha = 1.0 - ((float) (cg.time - menutime)) / Q3F_MENU_FADETIME;
		if( alpha <= 0 ) {
			menustate = Q3F_MENUSTATE_NONE;
			return;
		}
	}

	// hack for supplystation
	if( isinvismenu ) {
		return;
	}

	if(!*menu_menuname) {
		return;
	}

	menu = Menus_FindByName(menu_menuname);
	if(!menu) {
		menustate = Q3F_MENUSTATE_NONE;
		return;
	}

	Menu_Paint(menu, qtrue);
}

void CG_Q3F_Menu_DrawItem(int pos, int optnum, rectDef_t *rect, float scale, vec4_t color, int textStyle, 
									  int textalignment, float text_x, float text_y, fontStruct_t *font) {
	char buffer[128];
	vec4_t newcolor;
	float alpha;

	newcolor[0] = color[0];
	newcolor[1] = color[1];
	newcolor[2] = color[2];
	newcolor[3] = color[3];

	if( menuoptionstate[optnum] == MOS_INACTIVE ) {
		return;
	} 
	
	if( menuoptionstate[optnum] == MOS_DISABLED ) {
		newcolor[3] *= 0.5f;
	}

	if(pos == 0) {
		Com_sprintf(buffer, 128, "%i", (optnum+1)%10);
	} else if(pos == 1) {
		if( menuoptionstate[optnum] == MOS_DISABLED ) {
			Q_strncpyz(buffer, "Disabled", 128);
		} else {
			Q_strncpyz(buffer, menustrings[optnum], 128);
		} 
	} else {
		Q_strncpyz(buffer, menustrings2[optnum], 128);
	}

	if( menustate == Q3F_MENUSTATE_FADING ) {
		alpha = 1.0 - ((float) (cg.time - menutime)) / Q3F_MENU_FADETIME;
		if( alpha <= 0 ) {
			menustate = Q3F_MENUSTATE_NONE;
			return;
		}
	} else { 
		alpha = 1.0;
	}

	newcolor[3] *= alpha;

	CG_Text_Paint_MaxWidth( rect->x + text_x, rect->y + text_y, scale, newcolor, buffer, 0, 0, textStyle, font, textalignment, rect->w);
}

qboolean CG_Q3F_GetMenuAlpha(rectDef_t* in, rectDef_t* out, float* alpha) {
	
	if( menustate == Q3F_MENUSTATE_NONE ) {
		return qfalse;
	}

	if( menustate == Q3F_MENUSTATE_FADING ) {
		*alpha = 1.0 - ((float) (cg.time - menutime)) / Q3F_MENU_FADETIME;
		if( *alpha <= 0 ) {
			return qfalse;
		}
	} else {
		*alpha = 1.f;
	}

	memcpy(out, in, sizeof(rectDef_t));

	return qtrue;
}

void CG_Q3F_DrawMenuTitle(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, const char* text) {
	vec4_t newcolor;
	float alpha;

	newcolor[0] = color[0];
	newcolor[1] = color[1];
	newcolor[2] = color[2];
	newcolor[3] = color[3];

	if( menustate == Q3F_MENUSTATE_NONE ) {
		return;
	}

	if( menustate == Q3F_MENUSTATE_FADING ) {
		alpha = 1.0 - ((float) (cg.time - menutime)) / Q3F_MENU_FADETIME;
		if( alpha <= 0 ) {
			return;
		}
	} else {
		alpha = 1.f;
	}

	newcolor[3] *= alpha;

	CG_Text_Paint( rect->x + text_x, rect->y + text_y, scale, newcolor, text, 0, 0, textStyle, font, textalignment);
}

int CG_Q3F_GetMenuState() {
	return menustate;
}

