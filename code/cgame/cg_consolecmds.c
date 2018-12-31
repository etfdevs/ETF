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

// cg_consolecmds.c -- text commands typed in at the local console, or
// executed by a key binding

#include "cg_local.h"
#include "../game/bg_q3f_playerclass.h"
#include "cg_q3f_grenades.h"
#include "cg_q3f_sounddict.h"
#include "cg_q3f_menu.h"
#include "cg_q3f_scriptlib.h"

extern int menuCount;

/*
=================
CG_MediaDump_f

Dump the usage counts of all registered media.
=================
*/
#ifdef DEBUGREG
static void CG_MD_Add64Bit( unsigned int count[], unsigned int inc )
{
	if( inc && (count[0] + inc) <= count[0] )
		count[1]++;
	count[0] += inc;
}
static float CG_MD_Div64Bit( unsigned int num, unsigned int denom[] )
{
	double dnum, ddenom;

	dnum = (double) num;
	ddenom = denom[1] ? ((double) denom[0] + (double) (1 << 32) * (double) denom[1]) : (double) denom[0];
	return( (float) (dnum / ddenom) );
}
static void CG_MediaDump_f( void )
{
	unsigned int index, count[2];
	int percentage;
	fileHandle_t dumpHandle;
	char *ptr;

	trap_FS_FOpenFile( "mediadump.cfg", &dumpHandle, FS_WRITE );

		// Models
	for( index = count[0] = count[1] = 0; index < 1024; index++ )
	{
		if( cgs.registeredModels[index] )
			CG_MD_Add64Bit( count, cgs.registeredModelCounts[index] );
	}
	for( index = 0; index < 1024; index++ )
	{
		if( cgs.registeredModels[index] )
		{
			percentage = (int) (100 * CG_MD_Div64Bit( cgs.registeredModelCounts[index], count ));
			ptr = va(	"%s%s: %d (%d%%, %d bytes)\n",
						cgs.registeredModelCounts[index] ? S_COLOR_YELLOW : S_COLOR_WHITE,
						cgs.registeredModels[index], cgs.registeredModelCounts[index],
						percentage, cgs.registeredModelSizes[index] );
			CG_Printf( ptr );
			trap_FS_Write( ptr, strlen( ptr ), dumpHandle );
		}
	}
	CG_Printf( "^7\n" );
	trap_FS_Write( "^7\n", 3, dumpHandle );

		// Shaders
	for( index = count[0] = count[1] = 0; index < 1024; index++ )
	{
		if( cgs.registeredShaders[index] )
			CG_MD_Add64Bit( count, cgs.registeredShaderCounts[index] );
	}
	for( index = 0; index < 1024; index++ )
	{
		if( cgs.registeredShaders[index] )
		{
			percentage = (int) (100 * CG_MD_Div64Bit( cgs.registeredShaderCounts[index], count ));
			ptr = va(	"%s%s: %d (%d%%, %d bytes)\n",
						cgs.registeredShaderCounts[index] ? S_COLOR_YELLOW : S_COLOR_WHITE,
						cgs.registeredShaders[index], cgs.registeredShaderCounts[index],
						percentage, cgs.registeredShaderSizes[index] );
			CG_Printf( ptr );
			trap_FS_Write( ptr, strlen( ptr ), dumpHandle );
		}
	}
	CG_Printf( "^7\n" );
	trap_FS_Write( "^7\n", 3, dumpHandle );

		// Skins
	for( index = count[0] = count[1] = 0; index < 1024; index++ )
	{
		if( cgs.registeredSkins[index] )
			CG_MD_Add64Bit( count, cgs.registeredSkinCounts[index] );
	}
	for( index = 0; index < 1024; index++ )
	{
		if( cgs.registeredSkins[index] )
		{
			percentage = (int) (100 * CG_MD_Div64Bit( cgs.registeredSkinCounts[index], count ));
			ptr = va(	"%s%s: %d (%d%%, %d bytes)\n",
						cgs.registeredSkinCounts[index] ? S_COLOR_YELLOW : S_COLOR_WHITE,
						cgs.registeredSkins[index], cgs.registeredSkinCounts[index],
						percentage, cgs.registeredSkinSizes[index] );
			CG_Printf( ptr );
			trap_FS_Write( ptr, strlen( ptr ), dumpHandle );
		}
	}
	CG_Printf( "^7\n" );
	trap_FS_Write( "^7\n", 3, dumpHandle );

		// Sounds
	for( index = count[0] = count[1] = 0; index < 1024; index++ )
	{
		if( cgs.registeredSounds[index] )
			CG_MD_Add64Bit( count, cgs.registeredSoundCounts[index] );
	}
	for( index = 0; index < 1024; index++ )
	{
		if( cgs.registeredSounds[index] )
		{
			percentage = (int) (100 * CG_MD_Div64Bit( cgs.registeredSoundCounts[index], count ));
			ptr = va(	"%s%s: %d (%d%%, %d bytes)\n",
						cgs.registeredSoundCounts[index] ? S_COLOR_YELLOW : S_COLOR_WHITE,
						cgs.registeredSounds[index], cgs.registeredSoundCounts[index],
						percentage, cgs.registeredSoundSizes[index] );
			CG_Printf( ptr );
			trap_FS_Write( ptr, strlen( ptr ), dumpHandle );
		}
	}
	CG_Printf( "^7\n" );
	trap_FS_Write( "^7\n", 3, dumpHandle );

	trap_FS_FCloseFile( dumpHandle );
}
#endif


/*
=================
CG_SizeUp_f

Keybinding command
=================
*/
static void CG_SizeUp_f (void) {
	trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer+10)));
}


/*
=================
CG_SizeDown_f

Keybinding command
=================
*/
static void CG_SizeDown_f (void) {
	trap_Cvar_Set("cg_viewsize", va("%i",(int)(cg_viewsize.integer-10)));
}


/*
===================
CG_Q3F_BeginAlias
CG_Q3F_EndAlias
===================
*/

#define MAXALIASCOMMANDS	5
static char currcommands[MAXALIASCOMMANDS][MAX_STRING_CHARS];

/*static qboolean CG_Q3F_ConsoleDown( void )
{
	int cmdnum;
	usercmd_t ucmd;

	cmdnum = trap_GetCurrentCmdNumber();
	trap_GetUserCmd( cmdnum, &ucmd );

	return( ucmd.buttons & BUTTON_TALK );
}*/

static void CG_Q3F_BeginAlias( void )
{
	// Begin an alias command. Check it's not already in progress
	// first, of course.

	char cmdbuff[MAX_STRING_CHARS];
	int index, freeindex;

//	if( CG_Q3F_ConsoleDown() )
//		return;

	if( trap_Argc() >= 2 )
	{
		trap_Argv( 1, cmdbuff, MAX_STRING_CHARS - 5 );

		for( freeindex = -1, index = 0; index < MAXALIASCOMMANDS; index++ )
		{
			if( !Q_stricmp( currcommands[index], cmdbuff ) )
				return;		// Command is already in progress
			if( !*currcommands[index] )
				freeindex = index;
		}
		if( freeindex < 0 )
		{
			CG_Printf( BOX_PRINT_MODE_CHAT, "Sorry, too many +alias commands.\n" );
			return;
		}
		Q_strncpyz( currcommands[freeindex], cmdbuff, MAX_STRING_CHARS );
		trap_SendConsoleCommand( va( "vstr %sdown\n", cmdbuff ) );
	}
	else CG_Printf( BOX_PRINT_MODE_CHAT, "+alias: no argument.\n" );
}

static void CG_Q3F_EndAlias( void )
{
	// Begin an alias command

	char cmdbuff[MAX_STRING_CHARS];
	int index;

//	if( CG_Q3F_ConsoleDown() )
//		return;

	if( trap_Argc() >= 2 )
	{
		trap_Argv( 1, cmdbuff, MAX_STRING_CHARS - 5 );
		for( index = 0; index < MAXALIASCOMMANDS; index++ )
		{
			if( !Q_stricmp( currcommands[index], cmdbuff ) ) {
				*currcommands[index] = 0;		// Remove command from list
				trap_SendConsoleCommand( va( "vstr %sup\n", cmdbuff ) );
			}
		}
	}
	else CG_Printf( BOX_PRINT_MODE_CHAT, "-alias: no argument.\n" );
}

static void CG_Q3F_PlayString( void )
{
	char strbuff[MAX_STRING_CHARS];

	trap_Args( strbuff, MAX_STRING_CHARS );
	CG_Q3F_ParseSoundDictionary();
	CG_Q3F_StartSoundString( strbuff );
}

/*
===================
CG_Q3F_Inventory

Print off a quick inventory of current ammo values
===================
*/

static void CG_Q3F_Inventory( void )
{
	char ammobuff[8][128];
	char messagebuff[1024];
	bg_q3f_playerclass_t *cls;
	bg_q3f_grenade_t *gren1, *gren2;
	int buffcount, ammo, index;
	char *ammoname;

	memset( ammobuff, 0, sizeof(ammobuff) );
	memset( messagebuff, 0, sizeof(messagebuff) );
	cls		= BG_Q3F_GetClass( &cg.snap->ps );
	gren1	= BG_Q3F_GetGrenade( cls->gren1type );
	gren2	= BG_Q3F_GetGrenade( cls->gren2type );
	buffcount = -1;
	ammoname = "";
	ammo = 0;

		// Work out all the ammo available
	for( index = 0; index < 8; index++ )
	{
		switch( index )
		{
			case 0:	ammo		= cg.snap->ps.ammo[AMMO_SHELLS];
					ammoname	= "Shell";
					break;
			case 1:	ammo		= cg.snap->ps.ammo[AMMO_NAILS];
					ammoname	= "Nail";
					break;
			case 2:	ammo		= cg.snap->ps.ammo[AMMO_ROCKETS];
					ammoname	= "Rocket";
					break;
			case 3:	ammo		= cg.snap->ps.ammo[AMMO_CELLS];
					ammoname	= "Cell";
					break;
			case 4:	ammo		= cg.snap->ps.ammo[AMMO_GRENADES] & 0xFF; 
					ammoname	= gren1->name;
					break;
			case 5:	ammo		= cg.snap->ps.ammo[AMMO_GRENADES] >> 8;
					ammoname	= gren2->name;
					break;
			case 6:	ammo		= cg.snap->ps.ammo[AMMO_MEDIKIT];
					ammoname	= "Medi-unit";
					break;
			case 7:	ammo		= cg.snap->ps.ammo[AMMO_CHARGE];
					ammoname	= "HE Charge";
					break;
		}

		if( ammo > 0 )
		{
			buffcount++;
			Q_strncpyz(	ammobuff[index], va( "%d %s%s", ammo, ammoname, ((ammo == 1) ? "" : "s")), 128 );
		}
	}

		// And print the message
	if( buffcount < 0 )
		Q_strcat( messagebuff, sizeof(messagebuff), "nothing" );
	for( index = 0; index < 8; index++ )
	{
		if( !*ammobuff[index] )
			continue;
		Q_strcat( messagebuff, sizeof(messagebuff), ammobuff[index] );
		if( buffcount > 1 )
			Q_strcat( messagebuff, sizeof(messagebuff), ", " );
		else if( buffcount == 1 )
			Q_strcat( messagebuff, sizeof(messagebuff), " and " );
		buffcount--;
	}
	CG_Printf( BOX_PRINT_MODE_CHAT, "You have %s.\n", messagebuff );
}

/*
===================
CG_Q3F_DumpLocation

Dump a target_location definition to a file
===================
*/

static void CG_Q3F_DumpLocation( void )
{
	char locfilename[MAX_QPATH];
	char locname[MAX_STRING_CHARS];
	char *extptr, *buffptr;
	fileHandle_t f;

		// Check for argument
	if( trap_Argc() < 2 )
	{
		CG_Printf( BOX_PRINT_MODE_CHAT, "Usage: dumploc <locationname>\n" );
		return;
	}
	trap_Args( locname, sizeof(locname) );

		// Open locations file
	Q_strncpyz( locfilename, cgs.mapname, sizeof(locfilename) );
	extptr = locfilename + strlen( locfilename ) - 4;
	if( extptr < locfilename || Q_stricmp( extptr, ".bsp" ) )
	{
		CG_Printf( BOX_PRINT_MODE_CHAT, "Unable to dump, unknown map name?\n" );
		return;
	}
	Q_strncpyz( extptr, ".loc", 5 );
	trap_FS_FOpenFile( locfilename, &f, FS_APPEND_SYNC );
	if( !f )
	{
		CG_Printf( BOX_PRINT_MODE_CHAT, "Failed to open '%s' for writing.\n", locfilename );
		return;
	}

		// Strip bad characters out
	for( buffptr = locname; *buffptr; buffptr++ )
	{
		if( *buffptr == '\n' )
			*buffptr = ' ';
		else if( *buffptr == '"' )
			*buffptr = '\'';
	}
		// Kill any trailing space as well
	if( *(buffptr - 1) == ' ' )
		*(buffptr - 1) = 0;

		// Build the entity definition
	buffptr = va(	"{\n\"classname\" \"target_location\"\n\"origin\" \"%i %i %i\"\n\"message\" \"%s\"\n}\n\n",
					(int) cg.snap->ps.origin[0], (int) cg.snap->ps.origin[1], (int) cg.snap->ps.origin[2], locname );

		// And write out/acknowledge
	trap_FS_Write( buffptr, strlen( buffptr ), f );
	trap_FS_FCloseFile( f );
	CG_Printf(	BOX_PRINT_MODE_CHAT, "Entity dumped to '%s' (%i %i %i).\n", locfilename,
				(int) cg.snap->ps.origin[0], (int) cg.snap->ps.origin[1], (int) cg.snap->ps.origin[2] );
}

/*
===================
CG_Q3F_DumpSpeaker

Dump a speaker definition to a soundscript
===================
*/

static void CG_Q3F_DumpSpeaker( void )
{
	char scriptName[MAX_QPATH];
	char soundfile[MAX_STRING_CHARS];
	char buff[1024], filebuff[2048];
	char *extptr, *buffptr;
	fileHandle_t f;
	int i;
	int wait, random;

	wait = random = 0;

		// Check for argument
	if( trap_Argc() < 2 || trap_Argc() > 4 )
	{
		CG_Printf( BOX_PRINT_MODE_CHAT, "Usage: dumpspeaker <soundfile> ( <wait=value>|<random=value> )\n" );
		return;
	}

	trap_Argv( 1, soundfile, sizeof(soundfile) );

	// parse the other parameters
	for( i = 2; i < trap_Argc(); i++ ) {
		char *valueptr=NULL;

		trap_Argv( i, buff, sizeof(buff) );

		for( buffptr = buff; *buffptr; buffptr++ ) {
			if( *buffptr == '=' ) {
				valueptr = buffptr + 1;
				break;
			}
		}

		Q_strncpyz( buff, buff, buffptr - buff + 1 );

		if( !Q_stricmp( buff, "wait" ) )
			wait = atoi( valueptr );
		else if( !Q_stricmp( buff, "random" ) )
			random = atoi( valueptr );
	}

		// Open locations file
	Q_strncpyz( scriptName, cgs.mapname, sizeof(scriptName) );
	extptr = scriptName + strlen( scriptName ) - 4;
	if( extptr < scriptName || Q_stricmp( extptr, ".bsp" ) ) {
		CG_Printf( BOX_PRINT_MODE_CHAT, "Unable to dump, unknown map name?\n" );
		return;
	}
	Q_strncpyz( extptr, ".sscr", 6 );
	trap_FS_FOpenFile( scriptName, &f, FS_APPEND_SYNC );
	if( !f ) {
		CG_Printf( BOX_PRINT_MODE_CHAT, "Failed to open '%s' for writing.\n", scriptName );
		return;
	}

		// Build the speaker definition
	Com_sprintf( filebuff, sizeof(filebuff), "speaker {\norigin %i %i %i\nnoise \"%s\"\n", (int)cg.snap->ps.origin[0], (int)cg.snap->ps.origin[1], (int)cg.snap->ps.origin[2], soundfile );
	if( wait ) {
		Com_sprintf( buff, sizeof(buff), "wait %i\n", wait );
		Q_strcat( filebuff, sizeof(filebuff), buff );
	}
	if( random ) {
		Com_sprintf( buff, sizeof(buff), "random %i\n", random );
		Q_strcat( filebuff, sizeof(filebuff), buff );
	}
	Q_strcat( filebuff, sizeof(filebuff), "}\n\n" );

		// And write out/acknowledge
	trap_FS_Write( filebuff, strlen( filebuff ), f );
	trap_FS_FCloseFile( f );
	CG_Printf(	BOX_PRINT_MODE_CHAT, "Speaker dumped to '%s' (%i %i %i).\n", scriptName,
				(int) cg.snap->ps.origin[0], (int) cg.snap->ps.origin[1], (int) cg.snap->ps.origin[2] );
}

/*
======================
CG_Q3F_PlaySound
======================
*/
void CG_Q3F_PlaySound( void )
{
	// Play a sound on the announcer channel (like \play)

	const char *s;
	sfxHandle_t h;

	s = CG_Argv( 1 );
	if( s && *s && (h = trap_S_RegisterSound( s, qfalse )) )
		trap_S_StartLocalSound( h, CHAN_ANNOUNCER );
}

/*
======================
CG_Q3F_Flyby
======================
*/
void CG_Q3F_Flyby ( void ) {

	if( cgs.flybyPathIndex == -1 )
		return;

	cg.renderingFlyBy = !cg.renderingFlyBy;

	trap_SendClientCommand( va( "flyby %s %i", cg.renderingFlyBy ? "start" : "stop", (int)(cg.time  + cl_timeNudge.value - 2 * cg.frametime) ) );

	cgs.campaths[cgs.flybyPathIndex].currtrajindex = -1; // little hack so it starts at traject 0

	memset( &cgs.campaths[cgs.flybyPathIndex].camtraj, 0, sizeof(cgs.campaths[cgs.flybyPathIndex].camtraj) );
}

/*
=============
CG_Viewpos_f

Debugging command to print the current position
=============
*/
static void CG_Viewpos_f (void) {
	CG_Printf ( BOX_PRINT_MODE_CHAT, "(%i %i %i) : %i\n", (int)cg.refdef.vieworg[0],
		(int)cg.refdef.vieworg[1], (int)cg.refdef.vieworg[2], 
		(int)cg.refdefViewAngles[YAW]);
}


static void CG_Q3F_ShowQuickMenu( void ) {

	CG_Printf(BOX_PRINT_MODE_CHAT, "Quickmenu down.\n" );

	if(trap_Key_GetCatcher()) {
		return;
	}

	if (cg.showQuickMenu)
		return;

	if(cg.time < cg.nextQuickMenu) {
		cg.showQuickMenu = qtrue;
		return;
	}

	cg.showQuickMenu = qtrue;
	cgs.cursorX = 10;
	cgs.cursorY = 376;
	HUD_Setup_Menu( "quickmenu" );
	trap_UI_Popup( 2 );		//2 = UIMENU_INGAME		
}

static void CG_Q3F_HideQuickMenu( void ) {

	CG_Printf(BOX_PRINT_MODE_CHAT, "Quickmenu up.\n" );

	if(cg.showQuickMenu) {
		cg.showQuickMenu = qfalse;
		trap_UI_Popup( 0 );		//0 = UIMENU_NONE	
		cg.nextQuickMenu = cg.time + 200; 
	}
}

/*static*/ void CG_ScoresDown_f( void ) {
	if ( cg.showScores  ) {
		cg.showScores = qfalse;
		cg.scoreFadeTime = cg.time;
	} else {
		cg.showScores = qtrue;
	}
}

static void CG_TellTarget_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

// RR2DO2
static void CG_TellTeamTarget_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		CG_Printf(BOX_PRINT_MODE_CHAT, "Nobody heard message.\n" );
		return;
	}

	if ( cg.snap->ps.persistant[PERS_TEAM] != (int)cgs.clientinfo[clientNum].team ) {
		CG_Printf(BOX_PRINT_MODE_CHAT, "Nobody heard message.\n" );
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}
// RR2DO2

static void CG_TellAttacker_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_LastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "tell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

#if 0
static void CG_VoiceTellTarget_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "vtell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}

// RR2DO2
static void CG_VoiceTellTeamTarget_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_CrosshairPlayer();
	if ( clientNum == -1 ) {
		CG_Printf(BOX_PRINT_MODE_CHAT, "Nobody heard message.\n" );
		return;
	}

	if ( cg.snap->ps.persistant[PERS_TEAM] != cgs.clientinfo[clientNum].team ) {
		CG_Printf(BOX_PRINT_MODE_CHAT, "Nobody heard message.\n" );
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "vtell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}
// RR2DO2
#endif

static void CG_Q3F_Echo_f( void ) {
	char	message[256];

	trap_Args( message, 256 );

	CG_Printf(BOX_PRINT_MODE_CHAT, "%s\n", message);
}

#if 0
static void CG_VoiceTellAttacker_f( void ) {
	int		clientNum;
	char	command[128];
	char	message[128];

	clientNum = CG_LastAttacker();
	if ( clientNum == -1 ) {
		return;
	}

	trap_Args( message, 128 );
	Com_sprintf( command, 128, "vtell %i %s", clientNum, message );
	trap_SendClientCommand( command );
}
#endif

static qboolean CG_Q3F_Special( void ) 
{
	// RR2DO2: prevent crash
	if(!cg.snap || !cg.snap->ps.persistant || (cg.snap->ps.pm_flags & PMF_FOLLOW) || (cg.snap->ps.pm_flags & PMF_CHASE) || (cg.snap->ps.powerups[PW_Q3F_CEASEFIRE]))
		return (qfalse);

	switch(cg.snap->ps.persistant[PERS_CURRCLASS])
	{
		case Q3F_CLASS_SNIPER:		if( cg_sniperAutoZoom.integer )
									{
										CG_Printf( BOX_PRINT_MODE_CHAT, "Autozoom disabled.\n" );
										trap_Cvar_Set( "cg_sniperAutoZoom", "0" );
									}
									else {
										CG_Printf( BOX_PRINT_MODE_CHAT, "Autozoom enabled.\n" );
										trap_Cvar_Set( "cg_sniperAutoZoom", "1" );
									}
									return( qtrue );
		case Q3F_CLASS_PARAMEDIC:	if(cg.snap->ps.weapon == WP_SUPERNAILGUN)
										cg.weaponSelect = WP_AXE;
									else
										cg.weaponSelect = WP_SUPERNAILGUN;
									return( qtrue );
		case Q3F_CLASS_MINIGUNNER:	if(cg.snap->ps.weapon == WP_MINIGUN)
										cg.weaponSelect = WP_SUPERSHOTGUN;
									else
										cg.weaponSelect = WP_MINIGUN;
									return( qtrue );
		case Q3F_CLASS_FLAMETROOPER:if(cg.snap->ps.weapon == WP_FLAMETHROWER)
										cg.weaponSelect = WP_NAPALMCANNON;
									else
										cg.weaponSelect = WP_FLAMETHROWER;
									return( qtrue );
		default:					return( qfalse );
	}
}

static void CG_Q3F_Special2( void ) {
	// RR2DO2: prevent crash
	if(!cg.snap || !cg.snap->ps.persistant || (cg.snap->ps.pm_flags & PMF_FOLLOW) || (cg.snap->ps.pm_flags & PMF_CHASE) || (cg.snap->ps.powerups[PW_Q3F_CEASEFIRE]))
		return;

	switch(cg.snap->ps.persistant[PERS_CURRCLASS]) {
	case Q3F_CLASS_SNIPER:
		break;
	case Q3F_CLASS_FLAMETROOPER:
		break;
	case Q3F_CLASS_GRENADIER:
		if (cg.snap->ps.stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_LAYCHARGE) ) {
           trap_SendConsoleCommand("charge cancel\n");
		} else {
			trap_SendConsoleCommand("charge menu\n");
		}
		break;
	case Q3F_CLASS_ENGINEER:
		/* Cancel, destroy or build the supplystation */
		if( cg.snap->ps.stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_BUILDING) ) {
			trap_SendConsoleCommand("build cancel\n");
		} else if( cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF00 ) {
			trap_SendConsoleCommand("destroy supplystation\n");
		} else {
			trap_SendConsoleCommand("build supplystation\n");
		}
		break;
	case Q3F_CLASS_AGENT:
		trap_SendConsoleCommand("invisible\n");
		break;
	}
}

static void CG_Q3F_BeginGesture (void ) {
	char buffer[64];
	if( trap_Argc() < 2 )
		return;
	trap_Argv( 1, buffer, sizeof(buffer) );
	cg.ucmd_flags &= ~UCMDF_GESTURE_MASK;
	if (!Q_stricmp( buffer, "look")) 
		cg.ucmd_flags |= GESTURE_LOOK;
	else if (!Q_stricmp( buffer, "becon")) 
		cg.ucmd_flags |= GESTURE_BECON;
	else if (!Q_stricmp( buffer, "stop")) 
		cg.ucmd_flags |= GESTURE_STOP;
	else if (!Q_stricmp( buffer, "point")) 
		cg.ucmd_flags |= GESTURE_POINT;
	else if (!Q_stricmp( buffer, "yes")) 
		cg.ucmd_flags |= GESTURE_WAVEYES;
	else if (!Q_stricmp( buffer, "no")) 
		cg.ucmd_flags |= GESTURE_WAVENO;
	else if (!Q_stricmp( buffer, "taunt")) 
		cg.ucmd_flags |= GESTURE_TAUNT;
}

static void CG_Q3F_EndGesture( void ) {
	cg.ucmd_flags &= ~UCMDF_GESTURE_MASK;
}


/*
==================
CG_StartOrbit_f
==================
*/

static void CG_StartOrbit_f( void ) {
	char var[MAX_TOKEN_CHARS];

	trap_Cvar_VariableStringBuffer( "developer", var, sizeof( var ) );
	if ( !atoi(var) ) {
		return;
	}

	if (cg_cameraOrbit.value != 0) {
		trap_Cvar_Set ("cg_cameraOrbit", "0");
		trap_Cvar_Set("cg_thirdPerson", "0");
	} else {
		trap_Cvar_Set("cg_cameraOrbit", "5");
		trap_Cvar_Set("cg_thirdPerson", "1");
		trap_Cvar_Set("cg_thirdPersonAngle", "0");
		trap_Cvar_Set("cg_thirdPersonRange", "100");
	}
}

/*static void CG_Camera_f( void ) {
	char name[1024];
	trap_Argv( 1, name, sizeof(name));
	if (trap_loadCamera(name)) {
		cg.cameraMode = qtrue;
		trap_startCamera(cg.time);
	} else {
		CG_Printf ("Unable to load camera %s\n",name);
	}
}*/

// StringPool
static void CG_StringReport( void ) {
	String_Report();
}

void CG_LoadF2R_f( void ) {
	// Reloads all already loaded F2R Scripts
	if ( cgs.sv_cheats == qfalse ) {
		CG_Printf( BOX_PRINT_MODE_CHAT, "loadf2r is cheat protected.\n" );
		return;
	}
	F2R_Reload();
}

void CG_LoadSpirit_f( void ) {
	// Reloads all already loaded Spirit Scripts
	if ( cgs.sv_cheats == qfalse ) {
		CG_Printf( BOX_PRINT_MODE_CHAT, "loadspirit is cheat protected.\n" );
		return;
	}
	Spirit_Reload();
}


void CG_ListF2R_f( void ) {
	int i;

	CG_Printf( BOX_PRINT_MODE_CHAT, "%i F2R Scripts loaded\n", F2R_Count() );
	CG_Printf( BOX_PRINT_MODE_CHAT, "================================================================\n", F2R_Count() );

	for( i = 0; i < F2R_Count(); i++ ) {
		F2RDef_t *F2RScript = F2R_Get( i );
		CG_Printf( BOX_PRINT_MODE_CHAT, "%i : : %s\n", F2RScript->model, F2RScript->F2RFile );
	}
}

static void CG_LoadTeamColours_f( void ) {
	// Refreshes the teamcolours from the skin.colours files
	int						classNum;
	qboolean				noErrors = qtrue;
	bg_q3f_playerclass_t	*cls;
	char					filename[MAX_QPATH];
	int						skinColourHandle;

	if ( cgs.sv_cheats == qfalse ) {
		CG_Printf( BOX_PRINT_MODE_CHAT, "loadf2r is cheat protected.\n" );
		return;
	}

	for( classNum = 0; classNum < Q3F_CLASS_MAX; classNum++ ) {

		if( !(cgs.classes & ( 1 << classNum ) ) )
			continue;

		noErrors = qtrue;

		cls = bg_q3f_classlist[classNum];

		//
		// Load team colours
		//
		Com_sprintf( filename, sizeof( filename ), "models/classes/%s/skin.colours", cls->commandstring );
		if( ( skinColourHandle = trap_PC_LoadSource( filename ) ) != NULL_FILE ) {
			pc_token_t token;
			int i, j, teamNum;

			for( teamNum = 0; teamNum < 4 && noErrors; teamNum++ ) {
				for( i = 0; i < 3 && noErrors; i++ ) {
					for( j = 0; j < 3 && noErrors; j++ ) {
						if( !trap_PC_ReadToken( skinColourHandle, &token ) ) {
							Com_Printf( "^3Skin colour load failure: %s\n", filename );
							noErrors = qfalse;
						} else {
							if( token.intvalue > 255 )
								token.intvalue = 255;
							else if( token.intvalue < 0 )
								token.intvalue = 0;
							cgs.media.skincolours[classNum][teamNum][i][j] = (byte)token.intvalue;
						}
					}
				}
			}

			trap_PC_FreeSource( skinColourHandle );
		} else {
			Com_Printf( "^3Skin colour load failure: %s\n", filename );
			noErrors = qfalse;
		}
	}
}

void CG_LoadHud_f( void) {
	char buff[1024];
	const char *hudSet;

	memset(buff, 0, sizeof(buff));

	// Initializing memory
	Memory_Init( MEM_CGAME );
	SetCurrentMemory( MEM_CGAME );

	Menu_Reset();
	
	trap_Cvar_VariableStringBuffer("cg_hudFiles", buff, sizeof(buff));

	hudSet = buff;
	if (hudSet[0] == '\0') {
		hudSet = "ui/hud.txt";
	}

	CG_LoadMenus(hudSet, qfalse);
	//menuScoreboard = NULL;
}

static void CG_ResetHud_f( void) {
	char buff[1024];
	const char *hudSet;
	fileHandle_t fh;

	memset(buff, 0, sizeof(buff));

	// Initializing memory
	Memory_Init( MEM_CGAME );
	SetCurrentMemory( MEM_CGAME );

	Menu_Reset();
	
	trap_Cvar_VariableStringBuffer("cg_hudFiles", buff, sizeof(buff));

	hudSet = buff;
	if (hudSet[0] == '\0') {
		hudSet = "ui/hud.txt";
	}

	CG_LoadMenus(hudSet, qtrue);
	//menuScoreboard = NULL;

	// Leaving edithud mode. Save to config.
//	trap_FS_FOpenFile( va( "ui/userhud%i.cfg", cg_userHud.integer ), &fh, FS_WRITE );
	Com_ExtractFilePath( hudSet, buff );
	trap_FS_FOpenFile( va( "%s/userhud%i.cfg", buff, cg_userHud.integer ), &fh, FS_WRITE );

	if( fh ) {
		int i;

		trap_FS_Write( "// generated by " GAME_NAME ", do not modify\n", 35, fh );

		for( i = 0; i < menuCount; i++ ) {
			menuDef_t *menu = Menu_Get( i );

			Com_sprintf( buff, sizeof(buff), "%s %f %f\n", menu->window.name, menu->window.rect.x, menu->window.rect.y );

			trap_FS_Write( buff, strlen(buff), fh );				
		}

		trap_FS_FCloseFile( fh );
	}
}

/*
==================
CG_EditHud_f
==================
*/
/*static void CG_EditHud_f( void ) {
	if (cgs.eventHandling == CGAME_EVENT_EDITHUD) {
		CG_EventHandling(CGAME_EVENT_NONE, qfalse);
		trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_CGAME);
	} else if (cgs.eventHandling == CGAME_EVENT_NONE) {
		CG_EventHandling(CGAME_EVENT_EDITHUD, qfalse);
		trap_Key_SetCatcher(KEYCATCH_CGAME);
	}
}*/

static void CG_Q3F_MessageMode( int mode ) {
	trap_SendConsoleCommand( "etf_starttalk\n");
	cg.q3f_messagemode_mode = mode;
	cg.q3f_messagemode_buffer[0] = 0;
	CG_EventHandling(CGAME_EVENT_MESSAGEMODE, qfalse);
}

static void CG_Q3F_MessageMode_f( void ) {
	CG_Q3F_MessageMode(Q3F_SAY_ALL);
}

static void CG_Q3F_MessageModeTeam_f( void ) {
	CG_Q3F_MessageMode(Q3F_SAY_TEAM);
}

static void CG_Q3F_MessageModeAttacker_f( void ) {
	CG_Q3F_MessageMode(Q3F_SAY_ATTACKER);
}

static void CG_Q3F_MessageModeTarget_f( void ) {
	CG_Q3F_MessageMode(Q3F_SAY_TARGET);
}

static void CG_Q3F_CustomMenuOpen_f( void ) {
	char buffer[MAX_QPATH];

	trap_Argv(1, buffer, MAX_QPATH);
	if (!buffer[0] && cgs.eventHandling == CGAME_EVENT_CUSTOMMENU) {
		CG_EventHandling( CGAME_EVENT_NONE, qfalse );
		return;
	}

	if(cgs.eventHandling != CGAME_EVENT_NONE)
		return;

	CG_Q3F_CustomMenuShow( buffer );
}

/*static void CG_Q3F_CustomMenuClose_f( void ) {
	if(cgs.eventHandling == CGAME_EVENT_CUSTOMMENU) {
		CG_EventHandling(CGAME_EVENT_NONE, qfalse);
	}
}*/

static void CG_Q3F_CleanHandling_f( void ) {
	CG_EventHandling(CGAME_EVENT_NONE, qfalse);
}

void HudScript_Hide( void ) {
	int i;
	char buffer[1024];
	menuDef_t* menu;

	trap_Argv(2, buffer, 1024);

	menu = Menus_FindByName(buffer);

	if(!menu) {
		return;
	}

	for (i = 0; i < menu->itemCount; i++) {
		menu->items[i]->window.flags &= ~WINDOW_VISIBLE;
	}
}

void HudScript_Show( void ) {
	int i;
	char buffer[1024];
	menuDef_t* menu;

	trap_Argv(2, buffer, 1024);

	menu = Menus_FindByName(buffer);

	if(!menu) {
		return;
	}

	for (i = 0; i < menu->itemCount; i++) {
		menu->items[i]->window.flags |= WINDOW_VISIBLE;
	}
}

void HudScript_Toggle( void ) {
	int i;
	char buffer[1024];
	menuDef_t* menu;

	trap_Argv(2, buffer, 1024);

	menu = Menus_FindByName(buffer);

	if(!menu) {
		return;
	}

	for (i = 0; i < menu->itemCount; i++) {
		menu->items[i]->window.flags ^= WINDOW_VISIBLE;
	}
}

void HudScript_ScrollUp( void ) {
	char menuname[128];
	char itemname[128];
	menuDef_t* menu;
	itemDef_t* item;
	listBoxDef_t *listPtr;

	trap_Argv(2, menuname, 128);
	menu = Menus_FindByName(menuname);

	if(!menu) {
		return;
	}

	trap_Argv(3, itemname, 128);
	item = Menu_FindItemByName(menu, itemname);

	if(!item) {
		return;
	}

	listPtr = (listBoxDef_t*)item->typeData;
	if (item->type == ITEM_TYPE_LISTBOX && listPtr) {
		listPtr->startPos--;
		if(listPtr->startPos < 0) {
			listPtr->startPos = 0;
		}
	}
}

int Item_ListBox_MaxScroll(itemDef_t *item);

void HudScript_ScrollDown( void ) {
	char menuname[128];
	char itemname[128];
	menuDef_t* menu;
	itemDef_t* item;
	listBoxDef_t *listPtr;

	trap_Argv(2, menuname, 128);
	menu = Menus_FindByName(menuname);

	if(!menu) {
		return;
	}

	trap_Argv(3, itemname, 128);
	item = Menu_FindItemByName(menu, itemname);

	if(!item) {
		return;
	}

	listPtr = (listBoxDef_t*)item->typeData;
	if (item->type == ITEM_TYPE_LISTBOX && listPtr) {
		int max = Item_ListBox_MaxScroll(item);
		listPtr->startPos++;
		if(listPtr->startPos > max) {
			listPtr->startPos = max;
		}
	}
}

hudCommandDef_t hudCommandList[] =
{
	{"show",		&HudScript_Show},						// group/name
	{"hide",		&HudScript_Hide},						// group/name
	{"toggle",		&HudScript_Toggle},						// group/name
	{"scrolldown",	&HudScript_ScrollDown},					// group/name
	{"scrollup",	&HudScript_ScrollUp},					// group/name
};

int hudCommandCount = sizeof(hudCommandList) / sizeof(commandDef_t);

static void CG_Q3F_HudScript_f( void ) {
	char buffer[1024];
	int i;

	trap_Argv(1, buffer, 1024);

	for (i = 0; i < hudCommandCount; i++) {
		if (Q_stricmp(buffer, hudCommandList[i].name) == 0) {
			(hudCommandList[i].handler());
			break;
		}
	}
}


static void CG_Q3F_RecordDemo_f( void ) {
	char buffer[256], str[128];

	if( cl_demorecording.integer )
	{
		trap_SendConsoleCommand("g_synchronousClients 0; wait\n");
		trap_SendConsoleCommand("stoprecord\n");
		return;
	}

	trap_Argv(1, str, 128);

	if(!*str) {
		Q_strncpyz(str, "$Y-$a-$d_$h$m-$s_$l", 128);
	}

	CG_ETF_DemoParseString(str, buffer, 256);

	trap_SendConsoleCommand("g_synchronousClients 1; wait\n");
	trap_SendConsoleCommand(va("record \"%s\"\n", buffer));
	trap_SendConsoleCommand("g_synchronousClients 0; wait\n");
}

static void CG_Q3F_ScreenshotTGA_f( void ) {
	char buffer[256], str[128];

	trap_Argv(1, str, 128);

	if(!*str) {
		Q_strncpyz(str, "$Y-$a-$d_$h$m-$s_$l", 128);
	}

	CG_ETF_DemoParseString(str, buffer, 256);
	trap_SendConsoleCommand(va("screenshot \"%s\"\n", buffer));
}

static void CG_Q3F_ScreenshotJPEG_f( void ) {
	char buffer[256], str[128];

	trap_Argv(1, str, 128);

	if(!*str) {
		Q_strncpyz(str, "$Y-$a-$d_$h$m-$s_$l", 128);
	}

	CG_ETF_DemoParseString(str, buffer, 256);
	trap_SendConsoleCommand(va("screenshotjpeg \"%s\"\n", buffer));
}

#ifdef _ETXREAL
static void CG_Q3F_ScreenshotPNG_f( void ) {
	char buffer[256], str[128];

	trap_Argv(1, str, 128);

	if(!*str) {
		Q_strncpyz(str, "$Y-$a-$d_$h$m-$s_$l", 128);
	}

	CG_ETF_DemoParseString(str, buffer, 256);
	trap_SendConsoleCommand(va("screenshotpng \"%s\"\n", buffer));
}
#endif

extern qboolean debugMode;

static void CG_Q3F_DebugHud_f( void ) {
	debugMode = qtrue;
}

static void CG_Q3F_Discard_f( void ) {
	int cells, shells, rockets;
	char buffer[256];
	bg_q3f_playerclass_t* cls = bg_q3f_classlist[cg.predictedPlayerState.persistant[PERS_CURRCLASS]];
	
	trap_Cvar_VariableStringBuffer(va("discard_%s_cells", cls->commandstring), buffer, 256);
	cells = atoi(buffer);

	trap_Cvar_VariableStringBuffer(va("discard_%s_shells", cls->commandstring), buffer, 256);
	shells = atoi(buffer);

	trap_Cvar_VariableStringBuffer(va("discard_%s_rockets", cls->commandstring), buffer, 256);
	rockets = atoi(buffer);

	if(cells == -1) {
		trap_Cvar_VariableStringBuffer("discard_cells", buffer, 256);
		cells = atoi(buffer);
	}

	if(shells == -1) {
		trap_Cvar_VariableStringBuffer("discard_shells", buffer, 256);
		shells = atoi(buffer);
	}

	if(rockets == -1) {
		trap_Cvar_VariableStringBuffer("discard_rockets", buffer, 256);
		rockets = atoi(buffer);
	}

	trap_SendConsoleCommand(va("discard_etf %d %d %d\n", cells, shells, rockets));
}

static void CG_Q3F_TraceSurface_f( void ) {
	trace_t trace;
	vec3_t forward;

	if ( cgs.sv_cheats == qfalse ) {
		CG_Printf( BOX_PRINT_MODE_CHAT, "tracesurface is cheat protected.\n" );
		return;
	}

	AngleVectors( cg.refdefViewAngles, forward, NULL, NULL );
	VectorMA(cg.refdef.vieworg, 8000, forward, forward );
	CG_Trace(&trace, cg.refdef.vieworg, NULL, NULL, forward , -1, ~0 );
	CG_Printf(BOX_PRINT_MODE_CHAT,"Trace end %10.1f %10.1f %10.1f | Entity %5d Contents:%08X Surface:%08X\n",
		trace.endpos[0],trace.endpos[1],trace.endpos[2],
		trace.entityNum,trace.contents,trace.surfaceFlags);
}

void CG_keyOn_f( void ) {
	if ( !cg.demoPlayback ) {
		CG_Printf( BOX_PRINT_MODE_CHAT, "^3*** NOT PLAYING A DEMO!!\n" );
		return;
	}

	if ( demo_infoWindow.integer > 0 ) {
		//CG_ShowHelp_On( &cg.demohelpWindow );
	}

	CG_EventHandling( CGAME_EVENT_DEMO, qtrue );
}

void CG_keyOff_f( void ) {
	if ( !cg.demoPlayback ) {
		return;
	}
	CG_EventHandling( CGAME_EVENT_NONE, qfalse );
}

void CG_ShowSpecs_f( void ) {
	if(cg.spectatorLen)
		Com_Printf("%s\n", cg.spectatorList);
	else
		Com_Printf("No spectators\n");
}


/////////////////////////////////////////////////////////////////////////////////////////


typedef struct {
	char	*cmd;
	void	(*function)(void);
} consoleCommand_t;

static consoleCommand_t	commands[] = {
	{ "testgun", CG_TestGun_f },
	{ "testmodel", CG_TestModel_f },
	{ "nextframe", CG_TestModelNextFrame_f },
	{ "prevframe", CG_TestModelPrevFrame_f },
	{ "nextskin", CG_TestModelNextSkin_f },
	{ "prevskin", CG_TestModelPrevSkin_f },
	{ "viewpos", CG_Viewpos_f },
	{ "+scores", CG_ScoresDown_f },
//	{ "-scores", CG_ScoresUp_f },
	{ "+newzoom", CG_ZoomDown_f },
	{ "-newzoom", CG_ZoomUp_f },
	{ "sizeup", CG_SizeUp_f },
	{ "sizedown", CG_SizeDown_f },
	{ "weapnext", CG_NextWeapon_f },
	{ "weapprev", CG_PrevWeapon_f },
	{ "weapon", CG_Weapon_f },
	{ "weaponslot", CG_WeaponSlot_f },
	{ "lastweapon", CG_LastWeapon_f },
	{ "tell_target", CG_TellTarget_f },
	{ "tell_teamtarget", CG_TellTeamTarget_f }, // RR2DO2
	{ "tell_attacker", CG_TellAttacker_f },
	{ "startOrbit", CG_StartOrbit_f },
//	{ "camera", CG_Camera_f },

	// Golliwog: replacement play command
	{ "playsound",	CG_Q3F_PlaySound },
	// Golliwog.

	// slothy
	{ "+quickmenu",	CG_Q3F_ShowQuickMenu    },
	{ "-quickmenu",	CG_Q3F_HideQuickMenu    },

	// Golliwog: Grenade handling functions
	{ "primeone",	CG_Q3F_GrenOnePrime		},
	{ "primetwo",	CG_Q3F_GrenTwoPrime		},
	{ "throwgren",	CG_Q3F_GrenThrow		},
	{ "+gren1",		CG_Q3F_GrenOnePlusPrime	},
	{ "-gren1",		CG_Q3F_GrenOneThrow		},
	{ "+gren2",		CG_Q3F_GrenTwoPlusPrime	},
	{ "-gren2",		CG_Q3F_GrenTwoThrow		},
	{ "toggleone",	CG_Q3F_GrenOneToggle	},
	{ "toggletwo",	CG_Q3F_GrenTwoToggle	},
	// Golliwog.

	// Golliwog: Alias functions
	{ "+alias",		CG_Q3F_BeginAlias	},
	{ "-alias",		CG_Q3F_EndAlias		},
	// Golliwog.

	// Golliwog: Inventory
	{ "inventory",	CG_Q3F_Inventory	},
	// Golliwog.

	{ "playstring",		CG_Q3F_PlayString		},
	{ "dumploc",		CG_Q3F_DumpLocation		},
	{ "dumpspeaker",	CG_Q3F_DumpSpeaker		},
	{ "menucmd",		CG_Q3F_MenuNumCommand	},

//	{ "special",	CG_Q3F_Special		},
	
	{ "special2",		CG_Q3F_Special2	},
	
	{ "+gesture",		CG_Q3F_BeginGesture	},
	{ "-gesture",		CG_Q3F_EndGesture	},

	{ "flyby",			CG_Q3F_Flyby		},
	{ "stringreport",	CG_StringReport		},
	{ "loadf2r",		CG_LoadF2R_f		},
	{ "loadspirit",		CG_LoadSpirit_f		},
	{ "listf2r",		CG_ListF2R_f		},

	{ "loadhud",		CG_LoadHud_f },
	{ "resethud",		CG_ResetHud_f },
/*	{ "edithud",		CG_EditHud_f },		*/
	{ "loadteamcolours", CG_LoadTeamColours_f },

	{ "matchLog",		CG_MatchLog_f		},

#ifdef DEBUGREG
	{ "mediadump", CG_MediaDump_f },
#endif

   //these used to end in _q3f.  renamed for ETF.
	{ "messagemode_etf",	CG_Q3F_MessageMode_f			},
	{ "messagemode2_etf",	CG_Q3F_MessageModeTeam_f		},
	{ "messagemode3_etf",	CG_Q3F_MessageModeTarget_f		},
	{ "messagemode4_etf",	CG_Q3F_MessageModeAttacker_f	},

	{ "echo_etf",			CG_Q3F_Echo_f				},
	{ "startspectate",		CG_Q3F_StartSpectate		},

	{ "hudScript",			CG_Q3F_HudScript_f			},

	{ "usermenu",			CG_Q3F_CustomMenuOpen_f		},
//	{ "CloseUsermenu",		CG_Q3F_CustomMenuClose_f	},
	{ "cg_cleanhandling",	CG_Q3F_CleanHandling_f		},

	{ "record_etf",			CG_Q3F_RecordDemo_f			},
	{ "screenshotJPEG_etf",	CG_Q3F_ScreenshotJPEG_f		},
	{ "screenshot_etf",		CG_Q3F_ScreenshotTGA_f		},
#ifdef _ETXREAL
	{ "screenshotPNG_etf",	CG_Q3F_ScreenshotPNG_f		},
#endif

	{ "discard",			CG_Q3F_Discard_f			},
	{ "generateTracemap",	CG_GenerateTracemap			},

	{ "tracesurface",		CG_Q3F_TraceSurface_f		},
	{ "debughud",			CG_Q3F_DebugHud_f			},

	{ "keyoff",				CG_keyOff_f					},
	{ "keyon",				CG_keyOn_f					},

	{ "showspecs",			CG_ShowSpecs_f				},

};

static const size_t numCG_Commands = ARRAY_LEN(commands);

/*
=================
CG_ConsoleCommand

The string has been tokenized and can be retrieved with
Cmd_Argc() / Cmd_Argv()
=================
*/
qboolean CG_ConsoleCommand( void ) {
	const char	*cmd;
	size_t i;

	// Golliwog: Don't allow any commands if initializing
	if( cgs.initPhase )
		return( qtrue );
	// Golliwog.

	cmd = CG_Argv(0);

		// Golliwog: special is client- or server-side, depending on class
	if( !Q_stricmp( cmd, "special" ) )
		return( CG_Q3F_Special() );

	for ( i = 0 ; i < numCG_Commands; i++ ) {
		if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
			commands[i].function();
			return qtrue;
		}
	}

	if( !Q_stricmp(cmd, "UpdateClassinfo") ) {
		if((cg.time - cg.classInfoTime) > 3000) {
			cg.classInfoTime = cg.time;
			trap_SendConsoleCommand("ClassinfoRequest\n");
		}
		return qtrue;
	}

	return qfalse;
}


/*
=================
CG_InitConsoleCommands

Let the client system know about all of our commands
so it can perform tab completion
=================
*/
void CG_InitConsoleCommands( void ) {
	size_t i;
	char *str;

	for ( i = 0 ; i < numCG_Commands; i++ ) {
		trap_AddCommand( commands[i].cmd );
	}

	//
	// the game server will interpret these commands, which will be automatically
	// forwarded to the server after they are not recognized locally
	//
	trap_AddCommand ("kill");
	trap_AddCommand ("say");
	trap_AddCommand ("say_team");
	trap_AddCommand ("tell");
//	trap_AddCommand ("vsay");
//	trap_AddCommand ("vsay_team");
//	trap_AddCommand ("vtell");
//	trap_AddCommand ("vtaunt");
//	trap_AddCommand ("vosay");
//	trap_AddCommand ("vosay_team");
//	trap_AddCommand ("votell");
	trap_AddCommand ("give");
	trap_AddCommand ("god");
	trap_AddCommand ("notarget");
	trap_AddCommand ("noclip");
	trap_AddCommand ("team");
	//trap_AddCommand ("flyby"); // RR2DO2
	trap_AddCommand ("follow");
	trap_AddCommand ("follownext" );
	trap_AddCommand ("followprev" );
	trap_AddCommand ("levelshot");
//	trap_AddCommand ("addbot");
	trap_AddCommand ("setviewpos");
	trap_AddCommand ("callvote");
	trap_AddCommand ("vote");
	trap_AddCommand ("loaddefered");	// spelled wrong, but not changing for demo

	// slothy
	trap_AddCommand ("kick");
	trap_AddCommand ("clientkick");

	trap_AddCommand ("kickall");
	trap_AddCommand ("kickbots");
	trap_AddCommand ("banUser");
	trap_AddCommand ("banClient");

	trap_AddCommand ("mute");
	trap_AddCommand ("unmute");

	trap_AddCommand ("ignore");
	trap_AddCommand ("unignore");

	// Golliwog: Custom commands
	trap_AddCommand( "channel" );		// The 'channel' command for team comms.
	trap_AddCommand( "disguise" );		// Agent disguises.
	trap_AddCommand( "invisible" );		// Agent becomes invisible.
	trap_AddCommand( "flaginfo" );		// Flag information.
	trap_AddCommand( "charge" );		// Lay charge.
	trap_AddCommand( "reload" );		// Reload weapon.
//	trap_AddCommand( "discard" );		// Discard worthless ammo. djbob: client intercepts now
	trap_AddCommand( "dropammo" );		// Drop specified ammo.
	trap_AddCommand( "dropammoto" );	// Drop ammo, leaving specified amount.
	trap_AddCommand( "saveme" );		// "Save me" command.
	trap_AddCommand( "armorme" );		// "Armor me" command.
	trap_AddCommand( "detpipe" );		// Detonate pipes.
	trap_AddCommand( "special" );		// Assorted 'special' functions
	trap_AddCommand( "scanner" );		// Toggle scanner
	trap_AddCommand( "changeclass" );	// Show class menu
	trap_AddCommand( "build" );			// Build options
	trap_AddCommand( "destroy" );		// Building destruction options
	trap_AddCommand( "changeteam" );	// Show team menu
	trap_AddCommand( "alias" );			// tjh's aliasing command
	trap_AddCommand( "unalias" );		// tjh's un-aliasing command
	trap_AddCommand( "dropflag" );		// Command to drop the flag
	trap_AddCommand( "adminpassword" );	// Authenticate yourself as an admin
	trap_AddCommand( "admin" );			// Execute an admin command.
	trap_AddCommand( "admin2" );		// Execute an admin command. (client only)
	trap_AddCommand( "playerstatus" );
	trap_AddCommand( "chase" );
	trap_AddCommand( "chasenext" );
	trap_AddCommand( "chaseprev" );
	trap_AddCommand( "waypoint" );		// Add a waypoint.
	// Golliwog.

	// djbob
	trap_AddCommand( "authrc" );		// Check rcon is valid, for hud display
	// djbob

	trap_AddCommand( "ready" );			// Ready...
	trap_AddCommand( "unready" );		// Unready...

	trap_AddCommand("etfmap");
	trap_AddCommand("etfdevmap");

	//trap_AddCommand("showspecs");		// lists spectators in console

	// Golliwog: Add all the class changing commands
	for( i = 0; i < Q3F_CLASS_MAX; i++ )
	{
		str = bg_q3f_classlist[i]->commandstring;
		if( str && *str )
			trap_AddCommand( str );
	}
	trap_AddCommand( "randompc" );

	// Golliwog.
	// Golliwog: Force key bindings to weapon select values (also performed
	// in UI, but the overkill is good, no doubt).
//	for( i = 1; i <= 10; i++ )
//		trap_SendConsoleCommand( va( "bind %d weapon %d\n", (i % 10), i ) );

	// Golliwog
}

