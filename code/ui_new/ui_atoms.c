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

//
/**********************************************************************
	UI_ATOMS.C

	User interface building blocks and support functions.
**********************************************************************/
#include "ui_local.h"

#define UI_MAXPRINTMSG 8192
void QDECL Com_DPrintf( const char *fmt, ... ) {
	va_list argptr;
	char msg[UI_MAXPRINTMSG];
	int developer;

	developer = trap_Cvar_VariableValue( "developer" );
	if ( !developer ) {
		return;
	}

	va_start( argptr, fmt );
	Q_vsnprintf( msg, sizeof( msg ), fmt, argptr );
	va_end( argptr );

	trap_Print( msg );
}

void NORETURN QDECL Com_Error( int level, const char *error, ... ) {
	va_list		argptr;
	char		text[UI_MAXPRINTMSG];

	va_start (argptr, error);
	Q_vsnprintf (text, sizeof(text), error, argptr);
	va_end (argptr);

	trap_Error( text );
}

void QDECL Com_Printf( const char *msg, ... ) {
	va_list		argptr;
	char		text[UI_MAXPRINTMSG];

	va_start (argptr, msg);
	Q_vsnprintf (text, sizeof(text), msg, argptr);
	va_end (argptr);

	trap_Print( text );
}

qboolean newUI = qfalse;


/*
=================
UI_ClampCvar
=================
*/
float UI_ClampCvar( float min, float max, float value )
{
	if ( value < min ) return min;
	if ( value > max ) return max;
	return value;
}

const char *UI_Argv( int arg ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Argv( arg, buffer, sizeof( buffer ) );

	return buffer;
}

const char *UI_Cvar_VariableString( const char *var_name ) {
	static char	buffer[MAX_STRING_CHARS];

	trap_Cvar_VariableStringBuffer( var_name, buffer, sizeof( buffer ) );

	return buffer;
}

static void	UI_Cache_f( void) {
	Display_CacheAll();
}

static qboolean UI_AdvancedConsole(void)
{
	/*
	 * tjh:  alias/unalias/+action support added
	 */

	char	buf1[MAX_STRING_CHARS + 4];
	char	buf2[MAX_STRING_CHARS + 4];
	char	*cmd = buf1 + 1;

	trap_Argv(0, cmd, MAX_STRING_CHARS);

	// alias
	if (Q_stricmp(cmd, "alias") == 0)
	{
		buf1[0] = '/';
		trap_Argv(1, buf1 + 1, MAX_STRING_CHARS);
		trap_Argv(2, buf2, MAX_STRING_CHARS);

		if (buf1[1] == '\0')
		{
			trap_Print("usage: alias <name> <command>\n");
			return qtrue;
		}
		else if (buf2[0] == '\0')
		{
			trap_Cvar_VariableStringBuffer(buf1, buf2, MAX_STRING_CHARS);
			Com_Printf("\"%s\" is:\"%s\"\n", buf1 + 1, buf2);
			return qtrue;
		}
		else
		{
			// FixMe: Would like to add alias to command completion list
			// need to be in cgame to do this afaik

			// set alias cvar
			trap_Cvar_Set(buf1, "");
			trap_Cvar_Set(buf1, buf2);

			// test if +action and set /cmd+ cvar to 0.0
			// done for completeness, I doubt it's needed
			if (buf1[1] == '+')
			{
				strcat(buf1, "+");
				trap_Cvar_SetValue(buf1, 0.0);
			}

			return qtrue;
		}
	}

	// unalias: this doesn't really do much useful
	if (Q_stricmp(cmd, "unalias") == 0)
	{
		buf1[0] = '/';
		trap_Argv(1, buf1 + 1, MAX_STRING_CHARS);
		
		if (buf1[1] == '\0')
		{
			trap_Print("usage: unalias <name>\n");
			return qtrue;
		}
		else
		{
			// FixMe: Would like to remove alias from command completion list
			trap_Cvar_Reset(buf1);
			return qtrue;
		}
	}

	// try to find alias cvar for this command
	buf1[0] = '/';
	trap_Cvar_VariableStringBuffer(buf1, buf2, MAX_STRING_CHARS);

	if (buf2[0] != '\0')
	{
		// found cvar "/cmd"

		// stick '\n' on the end of the string to make sure its executed
		strcat(buf2, "\n");

		if (buf1[1] == '+')			// test if its a +action
		{
			// test if not pressed using cvar "/cmd+" == 0
			strcat(buf1, "+");	
			if (trap_Cvar_VariableValue(buf1) == 0.0)
			{
				// now mark action as pressed
				trap_Cvar_SetValue(buf1, 1.0);

				// execute alias				
				trap_Cmd_ExecuteText(EXEC_APPEND, buf2);
			}
		}
		else if (buf1[1] == '-')	// test if its a -action
		{
			// test if pressed using cvar "/cmd+" != 0
			buf1[1] = '+';
			strcat(buf1, "+");
			if (trap_Cvar_VariableValue(buf1) != 0.0)
			{
				// now mark action as not pressed
				trap_Cvar_SetValue(buf1, 0.0);

				// execute alias
				trap_Cmd_ExecuteText(EXEC_APPEND, buf2);
			}
			
		}
		else						// must be a normal alias then, execute
		{
			trap_Cmd_ExecuteText(EXEC_APPEND, buf2);
		}

		return qtrue;
	}

	return qfalse;
}

void UI_AddToTextBox(char* text, char* buffer, int* times, int max, int bufferwidth) {
	int i;

	for(i = max-1; i > 0 ; i--) {
		times[i] = times[i-1];
		Q_strncpyz(&buffer[i*bufferwidth], &buffer[(i-1)*bufferwidth], bufferwidth);
	}

	times[0] = uiInfo.uiDC.realTime + (trap_Cvar_VariableValue("con_notifytime_etf")*1000);
	Q_strncpyz(buffer, text, bufferwidth);
}

void UI_Q3F_MapSelectInit(void) {
	// We've just recieved a configstring from the server, split it up and process it
	char buffer[512];
	char *p, *s;

	uiInfo.mapSelectCount = 0;
	memset(uiInfo.mapSelectNames, 0, sizeof(uiInfo.mapSelectNames));
	memset(uiInfo.mapSelectTally, 0, sizeof(uiInfo.mapSelectTally));

	trap_GetConfigString(CS_FORTS_MAPVOTENAMES, buffer, 512);

	s = buffer;
	p = strchr(s, ' ');
	while(p) {
		*p++ = '\0';
		Q_strncpyz(uiInfo.mapSelectNames[uiInfo.mapSelectCount++], s, 64); 
		s = p;
		p = strchr(s, ' ');
	}
	if(*s) {
		Q_strncpyz(uiInfo.mapSelectNames[uiInfo.mapSelectCount++], s, 64); 
	}
}

void UI_Q3F_MapSelectTally(void) {
	// Update the tally of votes

	char buffer[512];
	char *p, *s;
	int i = 0;

	memset(uiInfo.mapSelectTally, 0, sizeof(uiInfo.mapSelectTally));

	trap_GetConfigString(CS_FORTS_MAPVOTETALLY, buffer, 512);

	s = buffer;
	p = strchr(s, ' ');
	while(p) {
		*p++ = '\0';
		uiInfo.mapSelectTally[i++] = atoi(s);
		s = p;
		p = strchr(s, ' ');
	}
	if(*s) {
		uiInfo.mapSelectTally[i++] = atoi(s);
	}
}

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

static void UI_ETF_DemoParseString(const char* in, char* out, int size) {
	qtime_t time;
	char tmp[2] = {0, 0};
	const char* p;
	char* c;
	int yr;

	trap_RealTime(&time);

	out[0] = '\0';
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
					Q_strcat( out, size, "ui" );
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

static void UI_HudIngame_f( void ) {
	char buffer[64];
	trap_GetConfigString( CS_INTERMISSION, buffer, sizeof(buffer) );

	if ( atoi( buffer ) ) {
		HUD_Setup_Menu("tab_scores");
		UI_ShowEndGame();
	} else {
		UI_ShowInGame();
	}
}

static void UI_RemapShader_f( void ) {
	if (trap_Argc() == 4) {
		char shader1[MAX_QPATH];
		char shader2[MAX_QPATH];
		Q_strncpyz(shader1, UI_Argv(1), sizeof(shader1));
		Q_strncpyz(shader2, UI_Argv(2), sizeof(shader2));
		trap_R_RemapShader(shader1, shader2, UI_Argv(3));
	}
}

static void UI_Chat_f( void ) {
	int i, j;
	char buffer[256];
	char chatbuffer[256];
	char *p, *s;

	*chatbuffer = '\0';
	for (i = 1, j = trap_Argc(); i < j; i++) {
		trap_Argv(i, buffer, sizeof(buffer));
		if(i != 1) {
			Q_strcat(chatbuffer, sizeof(chatbuffer), " ");
		}
		Q_strcat(chatbuffer, sizeof(chatbuffer), buffer);
	}
	Q_strcat(chatbuffer, sizeof(chatbuffer), "\n");
	
	s = chatbuffer;
	for( p = s; *p;) {
		if(*p == '\n') {
			*p++ = '\0';
			UI_AddToTextBox(chatbuffer, uiInfo.Q3F_uiChat, uiInfo.Q3F_uiChatTimes, MAX_UICHAT_STRINGS, MAX_SAY_TEXT);
			s = p;
		}
		else {
			p++;
		}
	}
}

static void UI_ScoreComplete_f( void ) {
	uiInfo.ScoreFetched = qtrue;
}

static void UI_UpdateMapVotes_f( void ) {
	UI_ParseMapInfo();
	UI_Q3F_MapSelectInit();
}

static void UI_Q3F_ScreenshotTGA_f( void ) {
	char buffer[256], str[128];

	trap_Argv(1, str, 128);

	if(!*str) {
		Q_strncpyz(str, "$Y-$a-$d_$h$m-$s_$l", 128);
	}

	UI_ETF_DemoParseString(str, buffer, 256);
	trap_Cmd_ExecuteText(EXEC_APPEND, va("screenshot \"%s\"\n", buffer));
}

static void UI_Q3F_ScreenshotJPEG_f( void ) {
	char buffer[256], str[128];

	trap_Argv(1, str, 128);

	if(!*str) {
		Q_strncpyz(str, "$Y-$a-$d_$h$m-$s_$l", 128);
	}

	UI_ETF_DemoParseString(str, buffer, 256);
	trap_Cmd_ExecuteText(EXEC_APPEND, va("screenshotjpeg \"%s\"\n", buffer));
}

#ifdef _ETXREAL
static void UI_Q3F_ScreenshotPNG_f( void ) {
	char buffer[256], str[128];

	trap_Argv(1, str, 128);

	if(!*str) {
		Q_strncpyz(str, "$Y-$a-$d_$h$m-$s_$l", 128);
	}

	UI_ETF_DemoParseString(str, buffer, 256);
	trap_Cmd_ExecuteText(EXEC_APPEND, va("screenshotpng \"%s\"\n", buffer));
}
#endif

static void UI_ETFMap_f( void ) {
	char strmap[MAX_QPATH], index[10];
	trap_Argv(1, strmap, sizeof(strmap));
	if(!*strmap) {
		trap_Print("Usage: etfmap <map name> [game index]\n");
		return;
	}

	trap_Argv(2, index, sizeof(index)); 
	if(!*index) {
		trap_Print("No gameindex specified, defaulting to 1\n");
		Q_strncpyz(index, "1", sizeof(index));
	}

	trap_Cvar_Set("g_gameindex", index);
	trap_Cvar_Set("g_antilag", index);
	trap_Cmd_ExecuteText(EXEC_APPEND, va("map \"%s\"\n", strmap));
}

static void UI_ETFDevMap_f( void ) {
	char strmap[MAX_QPATH], index[10];
	trap_Argv(1, strmap, sizeof(strmap));
	if(!*strmap) {
		trap_Print("Usage: etfdevmap <map name> [game index]\n");
		return;
	}

	trap_Argv(2, index, sizeof(index)); 
	if(!*index) {
		trap_Print("No gameindex specified, defaulting to 1\n");
		Q_strncpyz(index, "1", sizeof(index));
	}

	trap_Cvar_Set("g_gameindex", index);
	trap_Cvar_Set("g_antilag", index);
	trap_Cmd_ExecuteText(EXEC_APPEND, va("devmap \"%s\"\n", strmap));
}

/*
really don't need this, gfxinfo contains everything and glconfig should be safe was probably for testing during migration
static void UI_GLConfig_f( void ) {
	const char *tc_table[] =
	{
		"None",
		"GL_S3_s3tc",
		"GL_EXT_texture_compression_s3tc",
	};

	const char *dt_table[] = {
		"Integrated with window system",
		"Non-3Dfx standalone",
		"3Dfx standalone"
	};

	const char *hw_table[] = {
		"Generic",
		"Voodoo Banshee or Voodoo3",
		"Riva 128",
		"Rage Pro",
		"Permidia 2"
	};

	Com_Printf( "vendor: %s\n", uiInfo.uiDC.glconfig.vendor_string );
	Com_Printf( "renderer: %s\n", uiInfo.uiDC.glconfig.renderer_string );
	Com_Printf( "version: %s\n", uiInfo.uiDC.glconfig.version_string );
	Com_Printf( "extensions (likely truncated): %s\n", uiInfo.uiDC.glconfig.extensions_string );
	Com_Printf( "maxTextureSize: %i\n", uiInfo.uiDC.glconfig.maxTextureSize );
	Com_Printf( "maxActiveTextures: %i\n", uiInfo.uiDC.glconfig.maxActiveTextures );
	Com_Printf( "colorBits: %i\n", uiInfo.uiDC.glconfig.colorBits );
	Com_Printf( "depthBits: %i\n", uiInfo.uiDC.glconfig.depthBits );
	Com_Printf( "stencilBits: %i\n", uiInfo.uiDC.glconfig.stencilBits );
	Com_Printf( "driverType: %s\n", dt_table[uiInfo.uiDC.glconfig.driverType] );
	Com_Printf( "hardwareType: %s\n", hw_table[uiInfo.uiDC.glconfig.hardwareType] );
	Com_Printf( "deviceSupportsGamma: %s\n", ( uiInfo.uiDC.glconfig.deviceSupportsGamma ? "Yes" : "No" ) );
	Com_Printf( "textureCompression: %s\n", tc_table[uiInfo.uiDC.glconfig.textureCompression] );
	Com_Printf( "textureEnvAddAvailable: %s\n", ( uiInfo.uiDC.glconfig.textureEnvAddAvailable ? "Yes" : "No" ) );
	Com_Printf( "deviceSupportsGamma: %i\n", ( uiInfo.uiDC.glconfig.deviceSupportsGamma ? "Yes" : "No" ) );
	Com_Printf( "vidWidth: %i\n", uiInfo.uiDC.glconfig.vidWidth );
	Com_Printf( "vidHeight: %i\n", uiInfo.uiDC.glconfig.vidHeight );
	Com_Printf( "windowAspect: %f\n", uiInfo.uiDC.glconfig.windowAspect );
	Com_Printf( "displayFrequency: %i\n", uiInfo.uiDC.glconfig.displayFrequency );
	Com_Printf( "isFullscreen: %s\n", ( uiInfo.uiDC.glconfig.isFullscreen ? "Yes" : "No" ) );
	Com_Printf( "stereoEnabled: %s\n", ( uiInfo.uiDC.glconfig.stereoEnabled ? "Yes" : "No" ) );
}*/

static void UI_Fontinfo_f( void ) {
	int i;
	char buff[2];

	Com_Printf( "Font info for: '%s', glyphScale '%f'\n", uiInfo.uiDC.Assets.font.textFont.name, uiInfo.uiDC.Assets.font.textFont.glyphScale );

	buff[1] = 0;

	for ( i = 0; i < GLYPHS_PER_FONT; i++ ) {
		if ( ( i >= GLYPH_CHARSTART && i <= GLYPH_CHAREND ) || ( i >= GLYPH_CHARSTART2 && i <= GLYPH_CHAREND2 ) ) {
			buff[0] = i;
			Com_Printf( "%i: %s :: height: %i top: %i bottom: %i pitch: %i xSkip: %i imageWidth: %i imageHeight: %i s: %f t: %f s2: %f t2: %f glyph: %i shaderName: %s\n",
								i,
								buff,
								uiInfo.uiDC.Assets.font.textFont.glyphs[i].height,			// number of scan lines
								uiInfo.uiDC.Assets.font.textFont.glyphs[i].top,				// top of glyph in buffer
								uiInfo.uiDC.Assets.font.textFont.glyphs[i].bottom,			// bottom of glyph in buffer
								uiInfo.uiDC.Assets.font.textFont.glyphs[i].pitch,			// width for copying
								uiInfo.uiDC.Assets.font.textFont.glyphs[i].xSkip,			// x adjustment
								uiInfo.uiDC.Assets.font.textFont.glyphs[i].imageWidth,		// width of actual image
								uiInfo.uiDC.Assets.font.textFont.glyphs[i].imageHeight,		// height of actual image
								uiInfo.uiDC.Assets.font.textFont.glyphs[i].s,				// x offset in image where glyph starts
								uiInfo.uiDC.Assets.font.textFont.glyphs[i].t,				// y offset in image where glyph starts
								uiInfo.uiDC.Assets.font.textFont.glyphs[i].s2,
								uiInfo.uiDC.Assets.font.textFont.glyphs[i].t2,
								uiInfo.uiDC.Assets.font.textFont.glyphs[i].glyph,			// handle to the shader with the glyph
								uiInfo.uiDC.Assets.font.textFont.glyphs[i].shaderName );
		}
	}
}

typedef struct {
	const char	*cmd;
	void	(*function)(void);
} consoleCommand_t;

static const consoleCommand_t commands[] = {
	{ "hud_ingame", UI_HudIngame_f },
	{ "ui_report", UI_Report },
	{ "ui_load", UI_Load },
	{ "readbindings", UI_ReadBindings, },
	{ "remapShader", UI_RemapShader_f },
	{ "ui_chat", UI_Chat_f },
	{ "ui_cache", UI_Cache_f },
	{ "hud_iplist", HUD_BuildPlayerIPList },
	{ "hud_banlist", HUD_BuildPlayerBANList },
	{ "ui_scoreclear", HUD_ClearScoreInfo },
	{ "ui_stats", UI_ParseStats },
	{ "ui_awards", UI_ParseAwards },
	{ "ui_teamscoredump", HUD_ParseTeamScoreInfo },
	{ "ui_scoredump", HUD_ParseScoreInfo },
	{ "ui_scorecomplete", UI_ScoreComplete_f },
	{ "ui_updatemapvotes", UI_UpdateMapVotes_f },
	{ "ui_updatemapvotetally", UI_Q3F_MapSelectTally },
	{ "screenshot_etf", UI_Q3F_ScreenshotTGA_f },
	{ "screenshotJPEG_etf", UI_Q3F_ScreenshotJPEG_f },
#ifdef _ETXREAL
	{ "screenshotPNG_etf", UI_Q3F_ScreenshotPNG_f },
#endif
	{ "etfmap", UI_ETFMap_f },
	{ "etfdevmap", UI_ETFDevMap_f },
	//{ "glconfig", UI_GLConfig_f },
	{ "fontinfo", UI_Fontinfo_f },
};

static const size_t numUI_Commands = ARRAY_LEN(commands);

/*
=================
UI_ConsoleCommand
=================
*/
qboolean UI_ConsoleCommand( int realTime ) {
	const char	*cmd;
	size_t i;

	uiInfo.uiDC.frameTime = realTime - uiInfo.uiDC.realTime;
	uiInfo.uiDC.realTime = realTime;

	cmd = UI_Argv( 0 );

	if( UI_AdvancedConsole() )
		return( qtrue );

	// ensure minimum menu data is available
	//Menu_Cache();

	// Unused command but we still want to consume it if it is sent
	if ( !Q_stricmp( cmd, "ui_cdkey" ) ) {
		return qtrue;
	}

	for ( i = 0 ; i < numUI_Commands; i++ ) {
		if ( !Q_stricmp( cmd, commands[i].cmd ) ) {
			commands[i].function();
			return qtrue;
		}
	}

	return qfalse;
}

/*
================
UI_AdjustFrom640

Adjusted for resolution and screen aspect ratio
================
*/
void UI_AdjustFrom640( float *x, float *y, float *w, float *h ) {
	// expect valid pointers
#if 0
	*x = *x * uiInfo.uiDC.scale + uiInfo.uiDC.bias;
	*y *= uiInfo.uiDC.scale;
	*w *= uiInfo.uiDC.scale;
	*h *= uiInfo.uiDC.scale;
#endif

	*x *= uiInfo.uiDC.xscale;
	*y *= uiInfo.uiDC.yscale;
	*w *= uiInfo.uiDC.xscale;
	*h *= uiInfo.uiDC.yscale;
}

void UI_DrawNamedPic( float x, float y, float width, float height, const char *picname ) {
	qhandle_t	hShader;

	hShader = trap_R_RegisterShaderNoMip( picname );
	UI_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 1, 1, hShader );
}

void UI_DrawHandlePic( float x, float y, float w, float h, qhandle_t hShader ) {
	float s0;
	float s1;
	float t0;
	float t1;

	if ( w < 0 ) {   // flip about vertical
		w  = -w;
		s0 = 1;
		s1 = 0;
	} else {
		s0 = 0;
		s1 = 1;
	}

	if ( h < 0 ) {   // flip about horizontal
		h  = -h;
		t0 = 1;
		t1 = 0;
	} else {
		t0 = 0;
		t1 = 1;
	}

	UI_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, w, h, s0, t0, s1, t1, hShader );
}

void UI_DrawAdjustedPic( float x, float y, float w, float h, qhandle_t hShader ) {
	UI_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, w, h, 0.02f, 0.02f, .98f, .98f, hShader );
}

/*
================
UI_FillRect

Coordinates are 640*480 virtual values
=================
*/
void UI_FillRect( float x, float y, float width, float height, const float *color ) {
	trap_R_SetColor( color );

	UI_AdjustFrom640( &x, &y, &width, &height );
	trap_R_DrawStretchPic( x, y, width, height, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );

	trap_R_SetColor( NULL );
}

void UI_DrawSides(float x, float y, float w, float h) {
	UI_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, 1, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
	trap_R_DrawStretchPic( x + w - 1, y, 1, h, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}

void UI_DrawTopBottom(float x, float y, float w, float h) {
	UI_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, w, 1, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
	trap_R_DrawStretchPic( x, y + h - 1, w, 1, 0, 0, 0, 0, uiInfo.uiDC.whiteShader );
}
/*
================
UI_DrawRect

Coordinates are 640*480 virtual values
=================
*/
void UI_DrawRect( float x, float y, float width, float height, const float *color ) {
	trap_R_SetColor( color );

	UI_DrawTopBottom(x, y, width, height);
	UI_DrawSides(x, y, width, height);

	trap_R_SetColor( NULL );
}

void UI_SetColor( const float *rgba ) {
	trap_R_SetColor( rgba );
}

void UI_UpdateScreen( void ) {
	trap_UpdateScreen();
}


/*void UI_DrawTextBox (int x, int y, int width, int lines)
{
	UI_FillRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorBlack );
	UI_DrawRect( x + BIGCHAR_WIDTH/2, y + BIGCHAR_HEIGHT/2, ( width + 1 ) * BIGCHAR_WIDTH, ( lines + 1 ) * BIGCHAR_HEIGHT, colorWhite );
}*/

qboolean UI_CursorInRect (int x, int y, int width, int height)
{
	if (uiInfo.uiDC.cursorx < x ||
		uiInfo.uiDC.cursory < y ||
		uiInfo.uiDC.cursorx > x+width ||
		uiInfo.uiDC.cursory > y+height)
		return qfalse;

	return qtrue;
}

// RR2DO2
/*
===================
UI_Q3F_DrawProgress

===================
*/
void UI_Q3F_DrawProgress( rectDef_t *rect, int value, int maxvalue, vec4_t color, qhandle_t shader ) {
	int			width;

	//let's find out the width of the bar
	width = (value * rect->w) / maxvalue;

	if ( width > rect->w )
		width = rect->w;

	if ( width < 0 )
		width = 0;

	if( shader ) {
		trap_R_SetColor( color );
		UI_DrawHandlePic( rect->x, rect->y, width, rect->h, shader);
		trap_R_SetColor( NULL );
	} else {
		UI_FillRect( rect->x, rect->y, width, rect->h, color );
	}
}
// RR2DO2
