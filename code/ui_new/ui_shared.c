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
// string allocation/managment

#include "ui_shared.h"
#include "../game/bg_q3f_playerclass.h"

#ifdef UI_EXPORTS
#include "ui_public.h"
void			trap_GetClientState( uiClientState_t *state );
#endif

#define SCROLL_TIME_START					500
#define SCROLL_TIME_ADJUST				150
#define SCROLL_TIME_ADJUSTOFFSET	40
#define SCROLL_TIME_FLOOR					20

typedef struct scrollInfo_s {
	int nextScrollTime;
	int nextAdjustTime;
	int adjustValue;
	int scrollKey;
	float xStart;
	float yStart;
	itemDef_t *item;
	qboolean scrollDir;
} scrollInfo_t;

static scrollInfo_t scrollInfo;

typedef struct tooltipInfo_s {
	itemDef_t *item;
	int	showtime;
} tooltipInfo_t;

static tooltipInfo_t ttInfo;

static void (*captureFunc) (void *p) = NULL;
static void *captureData = NULL;
static itemDef_t *itemCapture = NULL;   // item that has the mouse captured ( if any )

displayContextDef_t *DC = NULL;

qboolean g_waitingForKey = qfalse;
static qboolean g_editingField = qfalse;

//static itemDef_t *g_bindItem = NULL;
static itemDef_t *g_editItem = NULL;

menuDef_t Menus[MAX_MENUS];      // defined menus
int menuCount = 0;               // how many

menuDef_t *menuStack[MAX_OPEN_MENUS];
int openMenuCount = 0;

qboolean debugMode = qfalse;

static int uimousex = 0;
static int uimousey = 0;

#define DOUBLE_CLICK_DELAY 300
static int lastListBoxClickTime = 0;

void Item_RunScript(itemDef_t *item, const char *s);
void Item_SetupKeywordHash(void);
void Menu_SetupKeywordHash(void);
int BindingIDFromName(const char *name);
//qboolean Item_Bind_HandleKey(itemDef_t *item, int key, qboolean down);
itemDef_t *Menu_SetPrevCursorItem(menuDef_t *menu);
itemDef_t *Menu_SetNextCursorItem(menuDef_t *menu);
static qboolean Menu_OverActiveItem(menuDef_t *menu, float x, float y);

// slothy
#ifdef CGAME
#include "../cgame/cg_local.h"
//#include "../game/bg_public.h"
#include "../cgame/cg_q3f_menu.h"
extern cg_t cg;
extern cgs_t cgs;
#else
void trap_GetClipboardData( char *buf, int bufsize );
#endif

// RR2DO2: font loading code
void UI_Q3F_LoadFontFile( const char *fontName, int pointSize, fontInfo_t *font ) {
	fileHandle_t	f;
	int				filelen;
	int				i;

	memset( font, 0, sizeof( fontInfo_t ) );

	filelen = DC->openFile( va("%s_%i.dat", fontName, pointSize), &f, FS_READ );

	// read font .dat file
	if( filelen >= 0 ) {
		DC->fRead( font, sizeof( fontInfo_t ), f );

		DC->closeFile( f );
	} else {
		return;
	}

	// swap bytes if needed for those pesky macs
	i = 1;
	if( !*(byte *) &i ) {
		font->glyphScale = FloatSwap( font->glyphScale );
		for( i = 0; i < GLYPHS_PER_FONT; i++ ) {
			font->glyphs[i].height = LongSwap( font->glyphs[i].height );
			font->glyphs[i].top = LongSwap( font->glyphs[i].top  );
			font->glyphs[i].bottom = LongSwap( font->glyphs[i].bottom );
			font->glyphs[i].pitch = LongSwap( font->glyphs[i].pitch );
			font->glyphs[i].xSkip = LongSwap( font->glyphs[i].xSkip );
			font->glyphs[i].imageWidth = LongSwap( font->glyphs[i].imageWidth );
			font->glyphs[i].imageHeight = LongSwap( font->glyphs[i].imageHeight );
			font->glyphs[i].s = FloatSwap( font->glyphs[i].s );
			font->glyphs[i].t = FloatSwap( font->glyphs[i].t );
			font->glyphs[i].s2 = FloatSwap( font->glyphs[i].s2 );
			font->glyphs[i].t2 = FloatSwap( font->glyphs[i].t2 );
		}
	}

	// load shaders
	for( i = 0; i < GLYPHS_PER_FONT; i++ ) {
		font->glyphs[i].glyph = DC->registerShaderNoMip( font->glyphs[i].shaderName );
	}
}

// RR2DO2

#ifndef CGAME
//#define MEM_POOL_SIZE  128 * 1024
//#else
#define MEM_POOL_SIZE  4096 * 1024
//#endif

static char		memoryPool[MEM_POOL_SIZE];
static int		allocPoint;
static qboolean	outOfMemory;


/*
===============
UI_Alloc
===============
*/				  
void *UI_Alloc( int size ) {
	char	*p; 

	if ( allocPoint + size > MEM_POOL_SIZE ) {
		outOfMemory = qtrue;
		if (DC->Print) {
			DC->Print("UI_Alloc: Failure. Out of memory!\n");
		}
		return NULL;
	}

	p = &memoryPool[allocPoint];

	allocPoint += ( size + 15 ) & ~15;

	return p;
}

/*
===============
UI_InitMemory
===============
*/
void UI_InitMemory( void ) {
	allocPoint = 0;
	outOfMemory = qfalse;
}

qboolean UI_OutOfMemory() {
	return outOfMemory;
}

#define HASH_TABLE_SIZE 2048
/*
================
return a hash value for the string
================
*/
static unsigned hashForString(const char *str) {
	int			i;
	unsigned	hash;
	char		letter;

	hash = 0;
	i = 0;
	while (str[i] != '\0') {
		letter = tolower(str[i]);
		hash+=(unsigned)(letter)*(i+119);
		i++;
	}
	hash &= (HASH_TABLE_SIZE-1);
	return hash;
}

typedef struct stringDef_s {
	struct stringDef_s *next;
	const char *str;
} stringDef_t;

static int strPoolIndex = 0;
static char strPool[STRING_POOL_SIZE];

static int strHandleCount = 0;
static stringDef_t *strHandle[HASH_TABLE_SIZE];

const char *String_Alloc(const char *p) {
	int len;
	unsigned hash;
	stringDef_t *str, *last;
	static const char *staticNULL = "";

	if (p == NULL) {
		return NULL;
	}

	if (*p == 0) {
		return staticNULL;
	}

	hash = hashForString(p);

	str = strHandle[hash];
	while (str) {
		if (strcmp(p, str->str) == 0) {
			return str->str;
		}
		str = str->next;
	}

	len = strlen(p);
	if (len + strPoolIndex + 1 < STRING_POOL_SIZE) {
		int ph = strPoolIndex;
		strcpy(&strPool[strPoolIndex], p);
		strPoolIndex += len + 1;

		str = strHandle[hash];
		last = str;
		while (str && str->next) {
			last = str;
			str = str->next;
		}

		str  = UI_Alloc(sizeof(stringDef_t));
		if(str == NULL) {
			Com_Printf( S_COLOR_RED "String_Alloc (%s) Failed! UI_Alloc out of memory" S_COLOR_WHITE "\n", p );
			return NULL;
		}
			
		str->next = NULL;
		str->str = &strPool[ph];
		if (last) {
			last->next = str;
		} else {
			strHandle[hash] = str;
		}
		return &strPool[ph];
	}

	//Increase STRING_POOL_SIZE.
	Com_Printf( S_COLOR_RED "String_Alloc (%s) Failed! String pool has been exhausted." S_COLOR_WHITE "\n", p );
	return NULL;
}

void String_Report() {
	float f;
	Com_Printf("Memory/String Pool Info\n");
	Com_Printf("----------------\n");
	f = strPoolIndex;
	f /= STRING_POOL_SIZE;
	f *= 100;
	Com_Printf("String Pool is %.1f%% full, %i bytes out of %i used.\n", f, strPoolIndex, STRING_POOL_SIZE);
	f = allocPoint;
	f /= MEM_POOL_SIZE;
	f *= 100;
	Com_Printf("Memory Pool is %.1f%% full, %i bytes out of %i used.\n", f, allocPoint, MEM_POOL_SIZE);
}

/*
=================
String_Init
=================
*/
extern void F2R_SetupKeywordHash( void );

void String_Init() {
	int i;
	for (i = 0; i < HASH_TABLE_SIZE; i++) {
		strHandle[i] = 0;
	}
	strHandleCount = 0;
	strPoolIndex = 0;
	menuCount = 0;
	openMenuCount = 0;
	UI_InitMemory();
	Item_SetupKeywordHash();
	Menu_SetupKeywordHash();
/*	if (DC && DC->getBindingBuf) {
		Controls_GetConfig();
	}*/
}
#endif

#ifdef CGAME
void CG_Menu_Init() {

	// Init some settings
	menuCount = 0;
	openMenuCount = 0;

	Item_SetupKeywordHash();
	Menu_SetupKeywordHash();

/*	if (DC && DC->getBindingBuf) {
		Controls_GetConfig();
	}*/
}
#endif

/*
=================
PC_SourceWarning
=================
*/
void PC_SourceWarning(int handle, char *format, ...) {
	int line;
	char filename[128];
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof(string), format, argptr);
	va_end (argptr);

	filename[0] = '\0';
	line = 0;
	trap_PC_SourceFileAndLine(handle, filename, &line);

	Com_Printf(S_COLOR_YELLOW "WARNING: %s, line %d: %s\n", filename, line, string);
}

/*
=================
PC_SourceError
=================
*/
void PC_SourceError(int handle, char *format, ...) {
	int line;
	char filename[128];
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof(string), format, argptr);
	va_end (argptr);

	filename[0] = '\0';
	line = 0;
	trap_PC_SourceFileAndLine(handle, filename, &line);

	Com_Printf(S_COLOR_RED "ERROR: %s, line %d: %s\n", filename, line, string);
}

/*
=================
LerpColor
=================
*/
void LerpColor(vec4_t a, vec4_t b, vec4_t c, float t)
{
	int i;

	// lerp and clamp each component
	for (i=0; i<4; i++)
	{
		c[i] = a[i] + t*(b[i]-a[i]);
		if (c[i] < 0)
			c[i] = 0;
		else if (c[i] > 1.0)
			c[i] = 1.0;
	}
}

/*
=================
Float_Parse
=================
*/
qboolean Float_Parse(char **p, float *f) {
	char	*token;
	token = COM_ParseExt(p, qfalse);
	if (token && token[0] != 0) {
		*f = atof(token);
		return qtrue;
	} else {
		return qfalse;
	}
}

/*
=================
PC_Float_Parse
=================
*/
qboolean PC_Float_Parse(int handle, float *f) {
	pc_token_t token;
	int negative = qfalse;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (token.string[0] == '-') {
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;
		negative = qtrue;
	}
	if (token.type != TT_NUMBER) {
		PC_SourceError(handle, "expected float but found %s\n", token.string);
		return qfalse;
	}
	if (negative)
		*f = -token.floatvalue;
	else
		*f = token.floatvalue;
	return qtrue;
}

/*
=================
Color_Parse
=================
*/
qboolean Color_Parse(char **p, vec4_t *c) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!Float_Parse(p, &f)) {
			return qfalse;
		}
		(*c)[i] = f;
	}
	return qtrue;
}

/*
=================
PC_Color_Parse
=================
*/
qboolean PC_Color_Parse(int handle, vec4_t *c) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		(*c)[i] = f;
	}
	return qtrue;
}

/*
=================
Int_Parse
=================
*/
qboolean Int_Parse(char **p, int *i) {
	char	*token;
	token = COM_ParseExt(p, qfalse);

	if (token && token[0] != 0) {
		*i = atoi(token);
		return qtrue;
	} else {
		return qfalse;
	}
}

/*
=================
PC_Int_Parse
=================
*/
qboolean PC_Int_Parse(int handle, int *i) {
	pc_token_t token;
	int negative = qfalse;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (token.string[0] == '-') {
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;
		negative = qtrue;
	}
	if (token.type != TT_NUMBER) {
		PC_SourceError(handle, "expected integer but found %s\n", token.string);
		return qfalse;
	}
	*i = token.intvalue;
	if (negative)
		*i = - *i;
	return qtrue;
}

/*
=================
Rect_Parse
=================
*/
qboolean Rect_Parse(char **p, rectDef_t *r) {
	if (Float_Parse(p, &r->x)) {
		if (Float_Parse(p, &r->y)) {
			if (Float_Parse(p, &r->w)) {
				if (Float_Parse(p, &r->h)) {
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

/*
=================
PC_Rect_Parse
=================
*/
qboolean PC_Rect_Parse(int handle, rectDef_t *r) {
	if (PC_Float_Parse(handle, &r->x)) {
		if (PC_Float_Parse(handle, &r->y)) {
			if (PC_Float_Parse(handle, &r->w)) {
				if (PC_Float_Parse(handle, &r->h)) {
					return qtrue;
				}
			}
		}
	}
	return qfalse;
}

/*
=================
String_Parse
=================
*/
qboolean String_Parse(char **p, const char **out) {
	char *token;

	token = COM_ParseExt(p, qfalse);
	if (token && token[0] != 0) {
		*(out) = String_Alloc(token);
		return qtrue;
	}
	return qfalse;
}

/*
=================
PC_String_Parse
=================
*/
qboolean PC_String_Parse(int handle, const char **out) {
	pc_token_t token;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	
	*(out) = String_Alloc(token.string);
    return qtrue;
}

/*
=================
PC_String_ParseNoAlloc

Same as one above, but uses a static buff and not the string memory pool
=================
*/
qboolean PC_String_ParseNoAlloc(int handle, char *out, size_t size) {
	pc_token_t token;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	
	Q_strncpyz( out, token.string, size );
    return qtrue;
}


/*
=================
PC_Script_Parse
=================
*/
qboolean PC_Script_Parse(int handle, const char **out) {
	char script[1024];
	pc_token_t token;

	memset(script, 0, sizeof(script));
	// scripts start with { and have ; separated command lists.. commands are command, arg.. 
	// basically we want everything between the { } as it will be interpreted at run time
  
	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (Q_stricmp(token.string, "{") != 0) {
	    return qfalse;
	}

	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token))
			return qfalse;

		if (Q_stricmp(token.string, "}") == 0) {
			*out = String_Alloc(script);
			return qtrue;
		}

		if (token.string[1] != '\0') {
			Q_strcat(script, 1024, va("\"%s\"", token.string));
		} else {
			Q_strcat(script, 1024, token.string);
		}
		Q_strcat(script, 1024, " ");
	}
	return qfalse;
}

// display, window, menu, item code
// 

/*
==================
Init_Display

Initializes the display with a structure to all the drawing routines
 ==================
*/
void Init_Display(displayContextDef_t *dc) {
	DC = dc;

	ttInfo.item = NULL;
}

/*
==================
Window_Init

Initializes a window structure ( windowDef_t ) with defaults
 
==================
*/
void Window_Init(Window *w) {
	memset(w, 0, sizeof(windowDef_t));
	w->borderSize = 1;
	w->foreColor[0] = w->foreColor[1] = w->foreColor[2] = w->foreColor[3] = 1.0;
	w->cinematic = -1;
}

void Fade(int *flags, float *f, float clamp, int *nextTime, int offsetTime, qboolean bFlags, float fadeAmount) {
	if (*flags & (WINDOW_FADINGOUT | WINDOW_FADINGIN)) {
		if (DC->realTime > *nextTime) {
			*nextTime = DC->realTime + offsetTime;
			if (*flags & WINDOW_FADINGOUT) {
				*f -= fadeAmount;
				if (bFlags && *f <= 0.0) {
					*flags &= ~(WINDOW_FADINGOUT | WINDOW_VISIBLE);
				}
			} else {
				*f += fadeAmount;
				if (*f >= clamp) {
					*f = clamp;
					if (bFlags) {
						*flags &= ~WINDOW_FADINGIN;
					}
				}
			}
		}
	}
}

void GradientBar_Paint(rectDef_t *rect, vec4_t color) {
	// gradient bar takes two paints
	DC->setColor( color );
	DC->drawHandlePic(rect->x, rect->y, rect->w, rect->h, DC->Assets.gradientBar);
	DC->setColor( NULL );
}

void Window_Paint(Window *w, float fadeAmount, float fadeClamp, float fadeCycle) {
	vec4_t color;
	rectDef_t fillRect = w->rect;

/*	if (debugMode) {
		color[0] = color[1] = color[2] = color[3] = 1;
		DC->drawRect(w->rect.x, w->rect.y, w->rect.w, w->rect.h, 1, color);
	}*/

	if (!w || (!w->style && !w->border)) {
		return;
	}

	if (w->border != 0) {
		fillRect.x += w->borderSize;
		fillRect.y += w->borderSize;
		fillRect.w -= w->borderSize + 1;
		fillRect.h -= w->borderSize + 1;
	}

	switch(w->style) {
	case WINDOW_STYLE_FILLED:
		// box, but possible a shader that needs filled
		if (w->background) {
			Fade(&w->flags, &w->backColor[3], fadeClamp, &w->nextTime, fadeCycle, qtrue, fadeAmount);
			DC->setColor(w->backColor);
			DC->drawHandlePic(fillRect.x, fillRect.y, fillRect.w, fillRect.h, w->background);
			DC->setColor(NULL);
		} else {
			DC->fillRect(fillRect.x, fillRect.y, fillRect.w, fillRect.h, w->backColor);
		}
		break;
	case WINDOW_STYLE_GRADIENT:
	  GradientBar_Paint(&fillRect, w->backColor);
	  // gradient bar
	  break;

	case WINDOW_STYLE_SHADER:
		if (w->flags & WINDOW_FORECOLORSET) {
			DC->setColor(w->foreColor);
		}
		DC->drawHandlePic(fillRect.x, fillRect.y, fillRect.w, fillRect.h, w->background);
		DC->setColor(NULL);
		break;
	case WINDOW_STYLE_SHADER_ADJUST:
		if (w->flags & WINDOW_FORECOLORSET) {
			DC->setColor(w->foreColor);
		}
		DC->drawAdjustedPic(fillRect.x, fillRect.y, fillRect.w, fillRect.h, w->background);
		DC->setColor(NULL);
		break;
	case WINDOW_STYLE_TEAMCOLOR:
		if (DC->getTeamColor) {
			DC->getTeamColor(&color);
			DC->fillRect(fillRect.x, fillRect.y, fillRect.w, fillRect.h, color);
		}
		break;
	case WINDOW_STYLE_CINEMATIC:
		if (w->cinematic == -1) {
			w->cinematic = DC->playCinematic(w->cinematicName, fillRect.x, fillRect.y, fillRect.w, fillRect.h);
			if (w->cinematic == -1) {
				w->cinematic = -2;
			}
		} 
		if (w->cinematic >= 0) {
			DC->runCinematicFrame(w->cinematic);
			DC->drawCinematic(w->cinematic, fillRect.x, fillRect.y, fillRect.w, fillRect.h);
		}
		break;
	}

	switch(w->border) {
	case WINDOW_BORDER_FULL:
		// full
		// HACK HACK HACK
		if (w->style == WINDOW_STYLE_TEAMCOLOR) {
			if (color[0] > 0) { 
				// red
				color[0] = 1;
				color[1] = color[2] = .5;

			} else {
				color[2] = 1;
				color[0] = color[1] = .5;
			}
			color[3] = 1;
			DC->drawRect(w->rect.x, w->rect.y, w->rect.w, w->rect.h, w->borderSize, color);
		} else {
			DC->drawRect(w->rect.x, w->rect.y, w->rect.w, w->rect.h, w->borderSize, w->borderColor);
		}
		break;
	case WINDOW_BORDER_HORZ:
		// top/bottom
		DC->setColor(w->borderColor);
		DC->drawTopBottom(w->rect.x, w->rect.y, w->rect.w, w->rect.h, w->borderSize);
		DC->setColor( NULL );
		break;
	case WINDOW_BORDER_VERT:
		// left right
		DC->setColor(w->borderColor);
		DC->drawSides(w->rect.x, w->rect.y, w->rect.w, w->rect.h, w->borderSize);
		DC->setColor( NULL );
		break;
	}
}

// Slothy
void Tooltip_Initialize(itemDef_t *item)
{ 
	item->text = NULL;
	item->textalignx = 3;
	item->textaligny = 10;
	item->textscale = .2f;
	item->window.border = WINDOW_BORDER_FULL;
	item->window.borderSize = 1.f;
	item->window.flags &= ~WINDOW_VISIBLE;
	item->window.flags |= (WINDOW_DRAWALWAYSONTOP|WINDOW_AUTOWRAPPED);
	VectorSet4( item->window.backColor, .9f, .9f, .75f, 1.f );
	VectorSet4( item->window.borderColor, 0.f, 0.f, 0.f, 1.f );
	VectorSet4( item->window.foreColor, 0.f, 0.f, 0.f, 1.f );
}

void Tooltip_ComputePosition(itemDef_t *item)
{
	menuDef_t *parent = (menuDef_t*)item->parent;

	Rectangle *itemRect = &item->window.rectClient;
	Rectangle *tipRect = &item->toolTipData->window.rectClient;

//	DC->textFont( item->toolTipData->font );

	// Set positioning based on item location
	tipRect->x = itemRect->x + (itemRect->w / 3);
	tipRect->y = itemRect->y + itemRect->h + 8;

	//tipRect->h = 14.0f;
	//tipRect->w = DC->textWidth( item->toolTipData->text, item->toolTipData->textscale, 0 ) + 6.0f;
	//tipRect->h = DC->multiLineTextHeight( item->toolTipData->text, item->toolTipData->textscale, 0 ) + 9.f;
	//tipRect->w = DC->multiLineTextWidth( item->toolTipData->text, item->toolTipData->textscale, 0 ) + 6.f;

	tipRect->h = DC->textHeight( item->toolTipData->text, item->toolTipData->textscale, 0, &parent->font) + 7.f;
	tipRect->w = DC->textWidth( item->toolTipData->text, item->toolTipData->textscale, 0, &parent->font) + 8.f;
	
	if((tipRect->w + tipRect->x) > 635.0f) tipRect->x -= (tipRect->w + tipRect->x) - 635.0f;

	item->toolTipData->parent = item->parent;
	item->toolTipData->type = ITEM_TYPE_TEXT;
	item->toolTipData->window.style = WINDOW_STYLE_FILLED;
	item->toolTipData->window.flags |= WINDOW_VISIBLE;
} 


void Item_SetScreenCoords(itemDef_t *item, float x, float y) {
  
  if (item == NULL) {
    return;
  }

  if (item->window.border != 0) {
    x += item->window.borderSize;
    y += item->window.borderSize;
  }


  item->window.rect.x = x + item->window.rectClient.x;
  item->window.rect.y = y + item->window.rectClient.y;
  item->window.rect.w = item->window.rectClient.w;
  item->window.rect.h = item->window.rectClient.h;

  	// Don't let tooltips draw off the screen.
	if(item->toolTipData) {
		Item_SetScreenCoords(item->toolTipData, x, y);
		{
			float val = (item->toolTipData->window.rect.x + item->toolTipData->window.rect.w) - 635.0f;
			if(val > 0.0f) {
				item->toolTipData->window.rectClient.x -= val;
				item->toolTipData->window.rect.x -= val;
			}
		}
	}


  // force the text rects to recompute
  item->textRect.w = 0;
  item->textRect.h = 0;
}

// FIXME: consolidate this with nearby stuff
void Item_UpdatePosition(itemDef_t *item) {
  float x, y;
  menuDef_t *menu;
  
  if (item == NULL || item->parent == NULL) {
    return;
  }

  if(item->toolTipData != NULL)
	  x = 1;

  menu = (menuDef_t *)item->parent;

  x = menu->window.rect.x;
  y = menu->window.rect.y;
  
  if (menu->window.border != 0) {
    x += menu->window.borderSize;
    y += menu->window.borderSize;
  }

  Item_SetScreenCoords(item, x, y);

}

// menus
void Menu_UpdatePosition(menuDef_t *menu) {
  int i;
  float x, y;

  if (menu == NULL) {
    return;
  }
  
  x = menu->window.rect.x;
  y = menu->window.rect.y;
  if (menu->window.border != 0) {
    x += menu->window.borderSize;
    y += menu->window.borderSize;
  }

  for (i = 0; i < menu->itemCount; i++) {
	  if(menu->items[i]->toolTipData != NULL)
	  	Tooltip_ComputePosition(menu->items[i]);

    Item_SetScreenCoords(menu->items[i], x, y);
  }
}

void Menu_PostParse(menuDef_t *menu) {
	if (!menu) {
		return;
	}

	Menu_UpdatePosition(menu);
}

itemDef_t *Menu_ClearFocus(menuDef_t *menu) {
	int i;
	itemDef_t *ret = NULL;

	if (!menu) {
		return NULL;
	}

	for (i = 0; i < menu->itemCount; i++) {
		if (menu->items[i]->window.flags & WINDOW_HASFOCUS) {
			ret = menu->items[i];
		} 
		menu->items[i]->window.flags &= ~WINDOW_HASFOCUS;
		if (menu->items[i]->leaveFocus) {
			Item_RunScript(menu->items[i], menu->items[i]->leaveFocus);
		}
	}
 
	return ret;
}

qboolean IsVisible(int flags) {
	return (flags & WINDOW_VISIBLE && !(flags & WINDOW_FADINGOUT));
}

qboolean Rect_ContainsPoint(rectDef_t *rect, float x, float y) {
	if (rect) {
		if (x > rect->x && x < rect->x + rect->w && y > rect->y && y < rect->y + rect->h) {
			return qtrue;
		}
	}
	return qfalse;
}

int Menu_ItemsMatchingGroup(menuDef_t *menu, const char *name) {
  int i;
  int count = 0;
  for (i = 0; i < menu->itemCount; i++) {
    if (Q_stricmp(menu->items[i]->window.name, name) == 0 || (menu->items[i]->window.group && Q_stricmp(menu->items[i]->window.group, name) == 0)) {
      count++;
    } 
  }
  return count;
}

itemDef_t *Menu_GetMatchingItemByNumber(menuDef_t *menu, int index, const char *name) {
  int i;
  int count = 0;
  for (i = 0; i < menu->itemCount; i++) {
    if (Q_stricmp(menu->items[i]->window.name, name) == 0 || (menu->items[i]->window.group && Q_stricmp(menu->items[i]->window.group, name) == 0)) {
      if (count == index) {
        return menu->items[i];
      }
      count++;
    } 
  }
  return NULL;
}



void Script_SetColor(itemDef_t *item, char **args) {
  const char *name;
  int i;
  float f;
  vec4_t *out;
  // expecting type of color to set and 4 args for the color
  if (String_Parse(args, &name)) {
      out = NULL;
      if (Q_stricmp(name, "backcolor") == 0) {
        out = &item->window.backColor;
        item->window.flags |= WINDOW_BACKCOLORSET;
      } else if (Q_stricmp(name, "forecolor") == 0) {
        out = &item->window.foreColor;
        item->window.flags |= WINDOW_FORECOLORSET;
      } else if (Q_stricmp(name, "bordercolor") == 0) {
        out = &item->window.borderColor;
      }

      if (out) {
        for (i = 0; i < 4; i++) {
          if (!Float_Parse(args, &f)) {
            return;
          }
          (*out)[i] = f;
        }
      }
  }
}

void Script_SetBackground(itemDef_t *item, char **args) {
	const char *name;

	// expecting name to set asset to
	if (String_Parse(args, &name)) {
		item->window.background = DC->registerShaderNoMip(name);
	}
}




itemDef_t *Menu_FindItemByName(menuDef_t *menu, const char *p) {
	int i;
	if (menu == NULL || p == NULL) {
		return NULL;
	}

	for (i = 0; i < menu->itemCount; i++) {
		if (Q_stricmp(p, menu->items[i]->window.name) == 0) {
			return menu->items[i];
		}
	}

	return NULL;
}

void Script_SetTeamColor(itemDef_t *item, char **args) {
	if (DC->getTeamColor) {
		int i;
		vec4_t color;

		DC->getTeamColor(&color);
		for (i = 0; i < 4; i++) {
			item->window.backColor[i] = color[i];
		}
	}
}

void Script_SetItemColor(itemDef_t *item, char **args) {
	const char *itemname;
	const char *name;
	vec4_t color;
	int i;
	vec4_t *out;

	// expecting type of color to set and 4 args for the color
	if (String_Parse(args, &itemname) && String_Parse(args, &name)) {
		itemDef_t *item2;
		int j;
		int count = Menu_ItemsMatchingGroup(item->parent, itemname);

		if (!Color_Parse(args, &color)) {
			return;
		}

		for (j = 0; j < count; j++) {
			item2 = Menu_GetMatchingItemByNumber(item->parent, j, itemname);
			if (item2 != NULL) {
				out = NULL;
				if (Q_stricmp(name, "backcolor") == 0) {
					out = &item2->window.backColor;
				} else if (Q_stricmp(name, "forecolor") == 0) {
					out = &item2->window.foreColor;
					item2->window.flags |= WINDOW_FORECOLORSET;
				} else if (Q_stricmp(name, "bordercolor") == 0) {
					out = &item2->window.borderColor;
				} else if (Q_stricmp(name, "outlinecolor") == 0) {
					out = &item2->window.outlineColor;
				}

				if (out) {
					for (i = 0; i < 4; i++) {
						(*out)[i] = color[i];
					}
				}
			}
		}
	}
}

void Menus_MoveToY(menuDef_t *menu, int newY) {
	int Yoffset, i;
	itemDef_t *item;

	Yoffset = newY - menu->window.rect.y;
	if(Yoffset == 0)
		return;

	menu->window.rect.y += Yoffset;

	for(i = 0; i < menu->itemCount; i++) {
		item = menu->items[i];
		item->window.rect.y += Yoffset;
	}
}

void Menu_ShowItemByName(menuDef_t *menu, const char *p, qboolean bShow) {
	itemDef_t *item;
	int i;
	int count = Menu_ItemsMatchingGroup(menu, p);
	for (i = 0; i < count; i++) {
		item = Menu_GetMatchingItemByNumber(menu, i, p);
		if (item != NULL) {
			if (bShow) {
				item->window.flags |= WINDOW_VISIBLE;
			} else {
				item->window.flags &= ~WINDOW_VISIBLE;
				// stop cinematics playing in the window
				if (item->window.cinematic >= 0) {
					DC->stopCinematic(item->window.cinematic);
					item->window.cinematic = -1;
				}
			}
		}
	}
}

void Menu_FadeItemByName(menuDef_t *menu, const char *p, qboolean fadeOut) {
	itemDef_t *item;
	int i;
	int count = Menu_ItemsMatchingGroup(menu, p);
	for (i = 0; i < count; i++) {
		item = Menu_GetMatchingItemByNumber(menu, i, p);
		if (item != NULL) {
			if (fadeOut) {
				item->window.flags |= (WINDOW_FADINGOUT | WINDOW_VISIBLE);
				item->window.flags &= ~WINDOW_FADINGIN;
			} else {
				item->window.flags |= (WINDOW_VISIBLE | WINDOW_FADINGIN);
				item->window.flags &= ~WINDOW_FADINGOUT;
			}
		}
	}
}

menuDef_t *Menus_FindByName(const char *p) {
	int i;
	for (i = 0; i < menuCount; i++) {
		if (Q_stricmp(Menus[i].window.name, p) == 0) {
			return &Menus[i];
		} 
	}
	return NULL;
}

void Menus_ShowByName(const char *p) {
	menuDef_t *menu = Menus_FindByName(p);
	if (menu) {
		Menus_Activate(menu);
	}
}

void Menus_OpenByName(const char *p) {
	Menus_ActivateByName(p);
}

static void Menu_RunCloseScript(menuDef_t *menu) {
	if (menu && menu->window.flags & WINDOW_VISIBLE && menu->onClose) {
		itemDef_t item;
		item.parent = menu;
		Item_RunScript(&item, menu->onClose);
	}
}

void Menus_CloseByName(const char *p) {
	menuDef_t *menu = Menus_FindByName(p);
	if (menu != NULL && IsVisible(menu->window.flags)) {
		Menu_RunCloseScript(menu);
		menu->window.flags &= ~(WINDOW_VISIBLE | WINDOW_HASFOCUS);
	}
}

void Menus_CloseAll() {
	int i;
	for (i = 0; i < menuCount; i++) {
		Menu_RunCloseScript(&Menus[i]);
		Menus[i].window.flags &= ~(WINDOW_HASFOCUS | WINDOW_VISIBLE);
	}
}

void Script_Show(itemDef_t *item, char **args) {
	const char *name;
	if (String_Parse(args, &name)) {
		Menu_ShowItemByName(item->parent, name, qtrue);
	}
}

void Script_Hide(itemDef_t *item, char **args) {
	const char *name;
	if (String_Parse(args, &name)) {
		Menu_ShowItemByName(item->parent, name, qfalse);
	}
}

void Script_FadeIn(itemDef_t *item, char **args) {
	const char *name;
	if (String_Parse(args, &name)) {
		Menu_FadeItemByName(item->parent, name, qfalse);
	}
}

void Script_FadeOut(itemDef_t *item, char **args) {
	const char *name;
	if (String_Parse(args, &name)) {
		Menu_FadeItemByName(item->parent, name, qtrue);
	}
}

void Menu_FadeMenuByName( const char *p, qboolean fadeOut ) {
	itemDef_t	*item;
	int		i;
	menuDef_t *menu = Menus_FindByName( p );

	if( menu ) {
		for( i = 0; i < menu->itemCount; i++ ) {
			item = menu->items[i];
			if( fadeOut ) {
				item->window.flags |= (WINDOW_FADINGOUT | WINDOW_VISIBLE);
				item->window.flags &= ~WINDOW_FADINGIN;
			} else {
				item->window.flags |= (WINDOW_VISIBLE | WINDOW_FADINGIN);
				item->window.flags &= ~WINDOW_FADINGOUT;
			}
		}
	}
 }

void Script_FadeInMenu(itemDef_t *item, char **args) {
	const char *name=NULL;
	if( String_Parse( args, &name ) ) {
		Menus_OpenByName(name);
		Menu_FadeMenuByName( name, qfalse );
	}
}

void Script_FadeOutMenu(itemDef_t *item, char **args) {
	const char *name=NULL;
	if( String_Parse( args, &name ) ) {
		Menus_CloseByName(name);
		Menu_FadeMenuByName( name, qtrue );
	}
}

void Script_ConditionalOpen(itemDef_t *item, char **args) {
	const char *cvar=NULL;
	const char *name1=NULL;
	const char *name2=NULL;
	float		val;
	char		buff[1024];
	int			testtype; // 0: check val not 0
						  // 1: check cvar not empty

	if ( String_Parse(args, &cvar) && Int_Parse(args, &testtype) && String_Parse(args, &name1) && String_Parse(args, &name2) ) {

		switch( testtype ) {
		default:
		case 0:
			val = DC->getCVarValue( cvar );
			if ( val == 0.f ) {
				Menus_OpenByName(name2);
			} else {
				Menus_OpenByName(name1);
			}
			break;
		case 1:
			DC->getCVarString( cvar, buff, sizeof(buff) );
			if( !buff[0] ) {
				Menus_OpenByName(name2);
			} else {
				Menus_OpenByName(name1);
			}
			break;
		}
	}
}

void Script_ConditionalScript(itemDef_t *item, char **args) {
	const char *cvar;
	const char *script1;
	const char *script2;
	const char *token;
	float		val;
	char		buff[1024];
	int			testtype; // 0: check val not 0
						  // 1: check cvar not empty
						  // 2: special
						  // 3: compare integer
						  // 4: compare strings
	int			testval;
	const char *teststr;

	if ( String_Parse(args, &cvar) &&
		 Int_Parse(args, &testtype) &&
		 String_Parse(args, &token) && ( token && *token == '(' ) &&
		 String_Parse(args, &script1) &&
		 String_Parse(args, &token) && ( token && *token == ')' ) &&
		 String_Parse(args, &token) && ( token && *token == '(' ) &&
		 String_Parse(args, &script2) &&
		 String_Parse(args, &token) && ( token && *token == ')' ) ) {

		switch( testtype ) {
		default:
		case 0:
			val = DC->getCVarValue( cvar );
			if ( val == 0.f ) {
				Item_RunScript( item, script2 );
			} else {
				Item_RunScript( item, script1 );
			}
			break;
		case 1:
			DC->getCVarString( cvar, buff, sizeof(buff) );
			if( !buff[0] ) {
				Item_RunScript( item, script2 );
			} else {
				Item_RunScript( item, script1 );
			}
			break;
		case 3:
			if( Int_Parse( args, &testval ) ) {
				val = DC->getCVarValue( cvar ); 
				if ( val != testval ) {
					Item_RunScript( item, script2 );
				} else {
					Item_RunScript( item, script1 );
				}
			}
			break;
		case 4:
			if( String_Parse( args, &teststr ) ) { 
				DC->getCVarString( cvar, buff, sizeof(buff) );
				if ( Q_stricmp(buff, teststr) != 0) {
					Item_RunScript( item, script2 );
				} else {
					Item_RunScript( item, script1 );
				}
			}
			break;
		case 2:
			// special tests
			if( !Q_stricmp( cvar, "isconnected" ) ) {
				qboolean ingame = qfalse;
#ifdef UI_EXPORTS
				uiClientState_t	cstate;
				trap_GetClientState( &cstate );

				if( cstate.connState == CA_DISCONNECTED)
					ingame = qfalse;
				else
					ingame = qtrue;
#endif
	
				if( !ingame ) {
					Item_RunScript( item, script1 );
				} else {
					Item_RunScript( item, script2 );
				}
			}
			break;
		}
	}
}

void Script_Open(itemDef_t *item, char **args) {
	const char *name;
	if (String_Parse(args, &name)) {
		Menus_OpenByName(name);
	}
}

void Script_Close(itemDef_t *item, char **args) {
	const char *name;
	if (String_Parse(args, &name)) {
		Menus_CloseByName(name);
	}
}

void Menu_TransitionItemByName(menuDef_t *menu, const char *p, rectDef_t rectFrom, rectDef_t rectTo, int time, float amt) {
  itemDef_t *item;
  int i;
  int count = Menu_ItemsMatchingGroup(menu, p);
  for (i = 0; i < count; i++) {
    item = Menu_GetMatchingItemByNumber(menu, i, p);
    if (item != NULL) {
      item->window.flags |= (WINDOW_INTRANSITION | WINDOW_VISIBLE);
      item->window.offsetTime = time;
			memcpy(&item->window.rectClient, &rectFrom, sizeof(rectDef_t));
			memcpy(&item->window.rectEffects, &rectTo, sizeof(rectDef_t));
			item->window.rectEffects2.x = abs(rectTo.x - rectFrom.x) / amt;
			item->window.rectEffects2.y = abs(rectTo.y - rectFrom.y) / amt;
			item->window.rectEffects2.w = abs(rectTo.w - rectFrom.w) / amt;
			item->window.rectEffects2.h = abs(rectTo.h - rectFrom.h) / amt;
      Item_UpdatePosition(item);
    }
  }
}


void Script_Transition(itemDef_t *item, char **args) {
  const char *name;
	rectDef_t rectFrom, rectTo;
  int time;
	float amt;

  if (String_Parse(args, &name)) {
    if ( Rect_Parse(args, &rectFrom) && Rect_Parse(args, &rectTo) && Int_Parse(args, &time) && Float_Parse(args, &amt)) {
      Menu_TransitionItemByName(item->parent, name, rectFrom, rectTo, time, amt);
    }
  }
}


void Menu_OrbitItemByName(menuDef_t *menu, const char *p, float x, float y, float cx, float cy, int time) {
  itemDef_t *item;
  int i;
  int count = Menu_ItemsMatchingGroup(menu, p);
  for (i = 0; i < count; i++) {
    item = Menu_GetMatchingItemByNumber(menu, i, p);
    if (item != NULL) {
      item->window.flags |= (WINDOW_ORBITING | WINDOW_VISIBLE);
      item->window.offsetTime = time;
      item->window.rectEffects.x = cx;
      item->window.rectEffects.y = cy;
      item->window.rectClient.x = x;
      item->window.rectClient.y = y;
      Item_UpdatePosition(item);
    }
  }
}


void Script_Orbit(itemDef_t *item, char **args) {
  const char *name;
  float cx, cy, x, y;
  int time;

  if (String_Parse(args, &name)) {
    if ( Float_Parse(args, &x) && Float_Parse(args, &y) && Float_Parse(args, &cx) && Float_Parse(args, &cy) && Int_Parse(args, &time) ) {
      Menu_OrbitItemByName(item->parent, name, x, y, cx, cy, time);
    }
  }
}

void Script_SetFocus(itemDef_t *item, char **args) {
	const char *name;
	itemDef_t *focusItem;

	if (String_Parse(args, &name)) {
		focusItem = Menu_FindItemByName(item->parent, name);
		
		if (focusItem && !(focusItem->window.flags & WINDOW_DECORATION) && !(focusItem->window.flags & WINDOW_HASFOCUS)) {
			Menu_ClearFocus(item->parent);
			focusItem->window.flags |= WINDOW_HASFOCUS;
			
			if (focusItem->onFocus) {
				Item_RunScript(focusItem, focusItem->onFocus);
			}

			if (DC->Assets.itemFocusSound) {
				DC->startLocalSound( DC->Assets.itemFocusSound, CHAN_LOCAL_SOUND );
			}
		}
	}
}

// RR2DO2
void Script_SetMenuFocus(itemDef_t *item, char **args) {
	const char *name;

	if (String_Parse(args, &name)) {
		menuDef_t *focusMenu = Menus_FindByName( name );
		
		if (focusMenu && !(focusMenu->window.flags & WINDOW_HASFOCUS)) {
			Menu_ClearFocus(item->parent);
			focusMenu->window.flags |= WINDOW_HASFOCUS;
		}
	}
}

// RR2DO2

void Script_SetPlayerModel(itemDef_t *item, char **args) {
	const char *name;

	if (String_Parse(args, &name)) {
		DC->setCVar("team_model", name);
	}
}

void Script_SetPlayerHead(itemDef_t *item, char **args) {
	const char *name;

	if (String_Parse(args, &name)) {
		DC->setCVar("team_headmodel", name);
	}
}

void Script_SetCvar(itemDef_t *item, char **args) {
	const char *cvar, *val;

	if (String_Parse(args, &cvar) && String_Parse(args, &val)) {
		DC->setCVar(cvar, val);
	}
	
}

void Script_Exec(itemDef_t *item, char **args) {
	const char *val;

	if (String_Parse(args, &val)) {
		DC->executeText(EXEC_APPEND, va("%s ; ", val));
	}
}

void Script_Play(itemDef_t *item, char **args) {
	const char *val;

	if (String_Parse(args, &val)) {
		DC->startLocalSound(DC->registerSound(val, qfalse), CHAN_LOCAL_SOUND);
	}
}

void Script_playLooped(itemDef_t *item, char **args) {
	const char *val;

	if (String_Parse(args, &val)) {
		DC->stopBackgroundTrack();
		DC->startBackgroundTrack(val, val, 0);
	}
}

void Script_SetOnKey(itemDef_t *item, char **args) {
	int i, key;
	const char* script;
	menuDef_t *menu = item->parent;

	if(!Int_Parse(args, &key)) {
		return;
	}

	if(!String_Parse(args, &script)) {
		return;
	}

	for(i = 0; i < menu->numKeyScripts; i++) {
		if(menu->keyScripts[i].key == key) {
			menu->keyScripts[i].script = script;
			return;
		}
	}

	if(menu->numKeyScripts == MAX_KEY_SCRIPTS) {
		return;
	}

	menu->keyScripts[menu->numKeyScripts].key =		key;
	menu->keyScripts[menu->numKeyScripts].script =	script;

	menu->numKeyScripts++;
}

void Script_ResetFeederScroll(itemDef_t *item, char **args) {
	listBoxDef_t *listPtr;
	const char *name;
	itemDef_t *focusItem;

	if (String_Parse(args, &name)) {
		focusItem = Menu_FindItemByName(item->parent, name);

		if(!focusItem || focusItem->type != ITEM_TYPE_LISTBOX) {
			return;
		}

		listPtr = (listBoxDef_t*)focusItem->typeData;
		listPtr->startPos = 0;
		listPtr->cursorPos = -1;
	}
}

void Script_SetItemBackground(itemDef_t *item, char **args) {
	const char *name, *pos;
	itemDef_t *menuItem;
	int i, count;
	
	if (!String_Parse(args, &name)) {
		return;
	}

	if (!String_Parse(args, &pos)) {
		return;
	}

	count = Menu_ItemsMatchingGroup(item->parent, name);

	for (i = 0; i < count; i++) {
		menuItem = Menu_GetMatchingItemByNumber(item->parent, i, name);

		if(!menuItem) {
			continue;
		}

		if(!Q_stricmp(pos, "primary")) {
			menuItem->window.background = menuItem->window.primbackground;
		}
		else if(!Q_stricmp(pos, "alt")) {
			menuItem->window.background = menuItem->window.altbackground;
		}
	}
}

void Script_SetEditFocus( itemDef_t *item, char **args ) {
	const char *name=NULL;
	itemDef_t *editItem;

	if (String_Parse(args, &name)) {
		editItem = Menu_FindItemByName(item->parent, name);
		if( editItem && (editItem->type == ITEM_TYPE_EDITFIELD || editItem->type == ITEM_TYPE_NUMERICFIELD) ) {
			editFieldDef_t *editPtr = (editFieldDef_t*)editItem->typeData;

			Menu_ClearFocus( item->parent );
			editItem->window.flags |= WINDOW_HASFOCUS;
			if( editItem->onFocus ) {
				Item_RunScript( editItem, editItem->onFocus );
			}
			if( DC->Assets.itemFocusSound ) {
				DC->startLocalSound( DC->Assets.itemFocusSound, CHAN_LOCAL_SOUND );
			}

			// NERVE - SMF - reset scroll offset so we can see what we're editing
			if ( editPtr )
				editPtr->paintOffset = 0;

			editItem->cursorPos = 0;
			g_editingField = qtrue;
			g_editItem = editItem;

			// the stupidest idea ever, let's just override the console, every ui element, user choice, etc
			// nuking this
			//%	DC->setOverstrikeMode(qtrue);
		}
	}
}

commandDef_t commandList[] =
{
	{"fadein", &Script_FadeIn},						// group/name
	{"fadeout", &Script_FadeOut},					// group/name
	{"show", &Script_Show},							// group/name
	{"hide", &Script_Hide},							// group/name
	{"setcolor", &Script_SetColor},					// works on this
	{"open", &Script_Open},							// menu
	{"close", &Script_Close},						// menu
	{"setbackground", &Script_SetBackground},		// works on this
	{"setitemcolor", &Script_SetItemColor},			// group/name
	{"setteamcolor", &Script_SetTeamColor},			// sets this background color to team color
	{"setfocus", &Script_SetFocus},					// focus item in this menu
	{"setplayermodel", &Script_SetPlayerModel},		// sets this background color to team color
	{"setplayerhead", &Script_SetPlayerHead},		// sets this background color to team color
	{"transition", &Script_Transition},				// group/name
	{"setcvar", &Script_SetCvar},					// group/name
	{"exec", &Script_Exec},							// group/name
	{"play", &Script_Play},							// group/name
	{"playlooped", &Script_playLooped},				// group/name
	{"orbit", &Script_Orbit},						// group/name
// RR2DO2
	{"setmenufocus", &Script_SetMenuFocus},			// focus menu
// RR2DO2
// djbob
	{"setonkey", &Script_SetOnKey},
	{"resetFeederScroll", &Script_ResetFeederScroll},
	{"setitembackground", &Script_SetItemBackground},		// works on this
// djbob
// slothy from ET
	{"setEditFocus", &Script_SetEditFocus},
	{"conditionalopen", &Script_ConditionalOpen},	// DHM - Nerve:: cvar menu menu 
													// opens first menu if cvar is true[non-zero], second if false
	{"conditionalscript", &Script_ConditionalScript},	// as conditonalopen, but then executes scripts
	{"fadeinmenu", &Script_FadeInMenu},			// menu
	{"fadeoutmenu", &Script_FadeOutMenu},		// menu
// end slothy
};

int scriptCommandCount = sizeof(commandList) / sizeof(commandDef_t);


void Item_RunScript(itemDef_t *item, const char *s) {
	char script[1024], *p;
	int i;
	qboolean bRan;

	memset(script, 0, sizeof(script));

	if (item && s && s[0]) {
		Q_strcat(script, 1024, s);
		p = script;

		while (1) {
			const char *command;

			// expect command then arguments, ; ends command, NULL ends script
			if (!String_Parse(&p, &command)) {
				return;
			}

			if (command[0] == ';' && command[1] == '\0') {
				continue;
			}

			bRan = qfalse;

			for (i = 0; i < scriptCommandCount; i++) {
				if (Q_stricmp(command, commandList[i].name) == 0) {
					(commandList[i].handler(item, &p));
					bRan = qtrue;
					break;
				}
			}

			// not in our auto list, pass to handler
			if (!bRan) {
				DC->runScript(&p);
			}
		}
	}
}


qboolean Item_EnableShowViaCvar( itemDef_t *item, int flag ) {
	char script[1024], *p;

	memset(script, 0, sizeof(script));

	if (item && item->enableCvar && *item->enableCvar && item->cvarTest && *item->cvarTest) {
		char buff[1024];

		DC->getCVarString(item->cvarTest, buff, sizeof(buff));

		Q_strcat(script, 1024, item->enableCvar);
		p = script;
		while (1) {
			const char *val;

			// expect value then ; or NULL, NULL ends list
			if (!String_Parse(&p, &val)) {
				return (item->cvarFlags & flag) ? qfalse : qtrue;
			}

			if (val[0] == ';' && val[1] == '\0') {
				continue;
			}

			// enable it if any of the values are true
			if (item->cvarFlags & flag) {
				if (Q_stricmp(buff, val) == 0) {
					return qtrue;
				}
			} else {
				// disable it if any of the values are true
				if (Q_stricmp(buff, val) == 0) {
					return qfalse;
				}
			}
		}
		return (item->cvarFlags & flag) ? qfalse : qtrue;
	}
	return qtrue;
}

// will optionaly set focus to this item 
qboolean Item_SetFocus(itemDef_t *item, float x, float y) {
	int i;
	itemDef_t *oldFocus;
	sfxHandle_t *sfx = &DC->Assets.itemFocusSound;
	qboolean playSound = qfalse;
	menuDef_t *parent;
	// sanity check, non-null, not a decoration and does not already have the focus
	if (item == NULL || item->window.flags & WINDOW_DECORATION || item->window.flags & WINDOW_HASFOCUS || !(item->window.flags & WINDOW_VISIBLE)) {
		return qfalse;
	}

	// items can be enabled and disabled based on cvars
	if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !Item_EnableShowViaCvar(item, CVAR_ENABLE)) {
		return qfalse;
	}

	if (item->cvarFlags & (CVAR_SHOW | CVAR_HIDE) && !Item_EnableShowViaCvar(item, CVAR_SHOW)) {
		return qfalse;
	}

	parent = (menuDef_t*)item->parent;

	oldFocus = Menu_ClearFocus(parent);

	if (item->type == ITEM_TYPE_TEXT) {
		rectDef_t r;
		r = item->textRect;
		r.y -= r.h;
		if (Rect_ContainsPoint(&r, x, y)) {
			item->window.flags |= WINDOW_HASFOCUS;
			if (item->focusSound) {
				sfx = &item->focusSound;
			}
			playSound = qtrue;
		} else {
			if (oldFocus) {
				oldFocus->window.flags |= WINDOW_HASFOCUS;
				if (oldFocus->onFocus) {
					Item_RunScript(oldFocus, oldFocus->onFocus);
				}
			}
		}
	} else {
	    item->window.flags |= WINDOW_HASFOCUS;
		if (item->onFocus) {
			Item_RunScript(item, item->onFocus);
		}
		if (item->focusSound) {
			sfx = &item->focusSound;
		}
		playSound = qtrue;
	}

	if (playSound && sfx && *sfx) {
		DC->startLocalSound( *sfx, CHAN_LOCAL_SOUND );
	}

	for (i = 0; i < parent->itemCount; i++) {
		if (parent->items[i] == item) {
			parent->cursorItem = i;
			break;
		}
	}

	return qtrue;
}

int Item_ListBox_MaxScroll(itemDef_t *item) {
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	int count = DC->feederCount(item->special, item);
	int max;

/*	if (item->window.flags & WINDOW_HORIZONTAL) {
		max = count - (item->window.rect.w / listPtr->elementWidth) + 1;
	}
	else {*/
		max = count - (item->window.rect.h / listPtr->elementHeight) + 1;
//	}
	if (max < 0) {
		return 0;
	}
	return max;
}

int Item_ListBox_ThumbPosition(itemDef_t *item) {
	float max, pos, size;
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;

	max = Item_ListBox_MaxScroll(item);
/*	if (item->window.flags & WINDOW_HORIZONTAL) {
		size = item->window.rect.w - (SCROLLBAR_SIZE * 2) - 2;
		if (max > 0) {
			pos = (size-SCROLLBAR_SIZE) / (float) max;
		} else {
			pos = 0;
		}
		pos *= listPtr->startPos;
		return item->window.rect.x + 1 + SCROLLBAR_SIZE + pos;
	}
	else {*/
		size = item->window.rect.h - (SCROLLBAR_SIZE * 2) - 2;
		if (max > 0) {
			pos = (size-SCROLLBAR_SIZE) / (float) max;
		} else {
			pos = 0;
		}
		pos *= listPtr->startPos;
		return item->window.rect.y + 1 + SCROLLBAR_SIZE + pos;
//	}
}

int Item_ListBox_ThumbDrawPosition(itemDef_t *item) {
	int min, max;

	if (itemCapture == item) {
/*		if (item->window.flags & WINDOW_HORIZONTAL) {
			min = item->window.rect.x + SCROLLBAR_SIZE + 1;
			max = item->window.rect.x + item->window.rect.w - 2*SCROLLBAR_SIZE - 1;
			if (DC->cursorx >= min + SCROLLBAR_SIZE/2 && DC->cursorx <= max + SCROLLBAR_SIZE/2) {
				return DC->cursorx - SCROLLBAR_SIZE/2;
			}
			else {
				return Item_ListBox_ThumbPosition(item);
			}
		}
		else {*/
			min = item->window.rect.y + SCROLLBAR_SIZE + 1;
			max = item->window.rect.y + item->window.rect.h - 2*SCROLLBAR_SIZE - 1;
			if (DC->cursory >= min + SCROLLBAR_SIZE/2 && DC->cursory <= max + SCROLLBAR_SIZE/2) {
				return DC->cursory - SCROLLBAR_SIZE/2;
			}
			else {
				return Item_ListBox_ThumbPosition(item);
			}
//		}
	}
	else {
		return Item_ListBox_ThumbPosition(item);
	}
}

float Item_Slider_ThumbPosition(itemDef_t *item, float scale) {
	float value, range, x, myscale;
	editFieldDef_t *editDef = item->typeData;

	if (item->text) {
		x = item->textRect.x + item->textRect.w + 8;
	} else {
		x = item->window.rect.x;
	}

	if (editDef == NULL && item->cvar) {
		return x;
	}

	myscale = scale;
	if(editDef->scale != 1)
		myscale = editDef->scale;
	value = DC->getCVarValue(item->cvar) * myscale;

	if (value < editDef->minVal) {
		value = editDef->minVal;
	} else if (value > editDef->maxVal) {
		value = editDef->maxVal;
	}

	range = editDef->maxVal - editDef->minVal;
	value -= editDef->minVal;
	value /= range;
	value *= (SLIDER_WIDTH);
	x += value;

	return (int)x;
}

int Item_Slider_OverSlider(itemDef_t *item, float x, float y) {
	rectDef_t r;

	r.x = Item_Slider_ThumbPosition(item, 1); // - (SLIDER_THUMB_WIDTH / 2); //; 
	// Slothy r.y = item->window.rect.y - 2;
	r.y = item->window.rect.y;
	r.w = SLIDER_THUMB_WIDTH;
	r.h = SLIDER_THUMB_HEIGHT;

	if (Rect_ContainsPoint(&r, x, y)) {
		return WINDOW_LB_THUMB;
	}
	return 0;
}

int Item_ListBox_OverLB(itemDef_t *item, float x, float y) {
	rectDef_t r;
	//listBoxDef_t *listPtr;
	int thumbstart;
	//int count;

	//count = DC->feederCount(item->special, item);
	//listPtr = (listBoxDef_t*)item->typeData;
/*	if (item->window.flags & WINDOW_HORIZONTAL) {
		// check if on left arrow
		r.x = item->window.rect.x;
		r.y = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE;
		r.h = r.w = SCROLLBAR_SIZE;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_LEFTARROW;
		}
		// check if on right arrow
		r.x = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_RIGHTARROW;
		}
		// check if on thumb
		thumbstart = Item_ListBox_ThumbPosition(item);
		r.x = thumbstart;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_THUMB;
		}
		r.x = item->window.rect.x + SCROLLBAR_SIZE;
		r.w = thumbstart - r.x;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_PGUP;
		}
		r.x = thumbstart + SCROLLBAR_SIZE;
		r.w = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_PGDN;
		}
	} else {*/
		r.x = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE;
		r.y = item->window.rect.y;
		r.h = r.w = SCROLLBAR_SIZE;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_LEFTARROW;
		}
		r.y = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_RIGHTARROW;
		}
		thumbstart = Item_ListBox_ThumbPosition(item);
		r.y = thumbstart;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_THUMB;
		}
		r.y = item->window.rect.y + SCROLLBAR_SIZE;
		r.h = thumbstart - r.y;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_PGUP;
		}
		r.y = thumbstart + SCROLLBAR_SIZE;
		r.h = item->window.rect.y + item->window.rect.h - SCROLLBAR_SIZE;
		if (Rect_ContainsPoint(&r, x, y)) {
			return WINDOW_LB_PGDN;
		}
//	}
	return 0;
}


void Item_ListBox_MouseEnter(itemDef_t *item, float x, float y) 
{
	rectDef_t r;
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
        
	item->window.flags &= ~(WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW | WINDOW_LB_THUMB | WINDOW_LB_PGUP | WINDOW_LB_PGDN);
	item->window.flags |= Item_ListBox_OverLB(item, x, y);

/*	if (item->window.flags & WINDOW_HORIZONTAL) {
		if (!(item->window.flags & (WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW | WINDOW_LB_THUMB | WINDOW_LB_PGUP | WINDOW_LB_PGDN))) {
			// check for selection hit as we have exausted buttons and thumb
			if (listPtr->elementStyle == LISTBOX_IMAGE) {
				r.x = item->window.rect.x;
				r.y = item->window.rect.y;
				r.h = item->window.rect.h - SCROLLBAR_SIZE;
				r.w = item->window.rect.w - listPtr->drawPadding;
				if (Rect_ContainsPoint(&r, x, y)) {
					listPtr->cursorPos =  (int)((x - r.x) / listPtr->elementWidth)  + listPtr->startPos;
					if (listPtr->cursorPos >= listPtr->endPos) {
						listPtr->cursorPos = listPtr->endPos;
					}
				}
			} else {
				// text hit.. 
			}
		}
	} else*/ if (!(item->window.flags & (WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW | WINDOW_LB_THUMB | WINDOW_LB_PGUP | WINDOW_LB_PGDN))) {
		r.x = item->window.rect.x;
		r.y = item->window.rect.y;
		r.w = item->window.rect.w - SCROLLBAR_SIZE;
		r.h = item->window.rect.h - listPtr->drawPadding;
		if (Rect_ContainsPoint(&r, x, y)) {
			listPtr->cursorPos =  (int)((y - 2 - r.y) / listPtr->elementHeight)  + listPtr->startPos;
			if (listPtr->cursorPos > listPtr->endPos) {
				listPtr->cursorPos = listPtr->endPos;
			}
		}
	}
}

void Item_MouseEnter(itemDef_t *item, float x, float y) {
	rectDef_t r;
	if (item) {
		r = item->textRect;
		r.y -= r.h;
		// in the text rect?

		// items can be enabled and disabled based on cvars
		if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !Item_EnableShowViaCvar(item, CVAR_ENABLE)) {
			return;
		}

		if (item->cvarFlags & (CVAR_SHOW | CVAR_HIDE) && !Item_EnableShowViaCvar(item, CVAR_SHOW)) {
			return;
		}

		if (Rect_ContainsPoint(&r, x, y)) {
			if (!(item->window.flags & WINDOW_MOUSEOVERTEXT)) {
				Item_RunScript(item, item->mouseEnterText);
				item->window.flags |= WINDOW_MOUSEOVERTEXT;
			}
			if (!(item->window.flags & WINDOW_MOUSEOVER)) {
				Item_RunScript(item, item->mouseEnter);
				item->window.flags |= WINDOW_MOUSEOVER;
			}

		} else {
			// not in the text rect
			if (item->window.flags & WINDOW_MOUSEOVERTEXT) {
				// if we were
				Item_RunScript(item, item->mouseExitText);
				item->window.flags &= ~WINDOW_MOUSEOVERTEXT;
			}
			if (!(item->window.flags & WINDOW_MOUSEOVER)) {
				Item_RunScript(item, item->mouseEnter);
				item->window.flags |= WINDOW_MOUSEOVER;
			}

			if (item->type == ITEM_TYPE_LISTBOX) {
				Item_ListBox_MouseEnter(item, x, y);
			}
		}
	}
}

void Item_MouseLeave(itemDef_t *item) {
  if (item) {
    if (item->window.flags & WINDOW_MOUSEOVERTEXT) {
      Item_RunScript(item, item->mouseExitText);
      item->window.flags &= ~WINDOW_MOUSEOVERTEXT;
    }
    Item_RunScript(item, item->mouseExit);
    item->window.flags &= ~(WINDOW_LB_RIGHTARROW | WINDOW_LB_LEFTARROW);
  }
}

itemDef_t *Menu_HitTest(menuDef_t *menu, float x, float y) {
  int i;
  for (i = 0; i < menu->itemCount; i++) {
    if (Rect_ContainsPoint(&menu->items[i]->window.rect, x, y)) {
      return menu->items[i];
    }
  }
  return NULL;
}

void Item_SetMouseOver(itemDef_t *item, qboolean focus) {
  if (item) {
    if (focus) {
      item->window.flags |= WINDOW_MOUSEOVER;
    } else {
      item->window.flags &= ~WINDOW_MOUSEOVER;
    }
  }
}


qboolean Item_OwnerDraw_HandleKey(itemDef_t *item, int key) {
  if (item && DC->ownerDrawHandleKey) {
    return DC->ownerDrawHandleKey(item->window.ownerDraw, item->window.ownerDrawFlags, &item->special, key);
  }
  return qfalse;
}

qboolean Item_Slider_HandleKey(itemDef_t *item, int key, qboolean down, float scale) {
	float x, value, width, work, myscale;

	//DC->Print("slider handle key\n");
	if (item->window.flags & WINDOW_HASFOCUS && item->cvar && Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory)) {
		if (key == K_MOUSE1 || key == K_ENTER || key == K_MOUSE2 || key == K_MOUSE3) {
			editFieldDef_t *editDef = item->typeData;
			if (editDef) {
				rectDef_t testRect;
				width = SLIDER_WIDTH;
				if (item->text) {
					x = item->textRect.x + item->textRect.w + 8;
				} else {
					x = item->window.rect.x;
				}

				x += SLIDER_END_WIDTH;
				testRect = item->window.rect;
				//testRect.x = ;
				testRect.w = SLIDER_WIDTH + SLIDER_END_WIDTH + SLIDER_END_WIDTH;
				//DC->Print("slider x: %f\n", testRect.x);
				//DC->Print("slider w: %f\n", testRect.w);
				if (Rect_ContainsPoint(&testRect, DC->cursorx, DC->cursory)) {
					work = DC->cursorx - x;
					value = work / width;
					if(value > 1.0f)
						value = 1.0f;
					if(value < 0.0f)
						value = 0.0f;
					value *= (editDef->maxVal - editDef->minVal);
					value += editDef->minVal;

					myscale = scale;
					if(editDef->scale != 1)
						myscale = editDef->scale;

					DC->setCVar(item->cvar, va("%f", value / myscale));
					return qtrue;
				}
			}
		}
	}
//	DC->Print("slider handle key exit\n");
	return qfalse;
}

void UI_CycleFloatList(configData_t* configData) {
	char buffer[1024];
	char *p, *pos[2];
	float numbers[16];
	int i, j;
	qboolean brk = qfalse;

	if(!configData)
		return;

	p = configData->string;
	Q_strncpyz(buffer, p, 1024);
	p = buffer;

	i = 0;

	while(qtrue) {
		pos[0] = p;
		p = strchr(p, ';');
		if(!p) {
			break;
		}
			
		*p++ = '\0';
		pos[1] = p;

		p = strchr(p, ';');
		if(!p) {
			brk = qtrue;
		}
		else {
			*p++ = '\0';
		}

		numbers[i] = atof(pos[1]);

		if(++i == 16)
			break;

		if(brk)
			break;
	}

	for(j = 0; j < i; j++) {
		if(fabs(numbers[j] - configData->value) < 0.01) {
			j++;
			j %= i;
			configData->value = numbers[j];
			return;
		}
	}

	if(i > 0) {
		configData->value = numbers[0];
	}
	else {
		configData->value = 0;
	}
}

void UI_CycleTextList(configData_t* configData) {
	char buffer[1024];
	char *p, *pos[2];
	char* texts[64];
	int i, j;
	qboolean brk = qfalse;

	if(!configData)
		return;

	p = configData->string;
	Q_strncpyz(buffer, p, 1024);
	p = buffer;

	i = 0;

	while(qtrue) {
		pos[0] = p;
		p = strchr(p, ';');
		if(!p) {
			break;
		}
			
		*p++ = '\0';
		pos[1] = p;

		p = strchr(p, ';');
		if(!p) {
			brk = qtrue;
		}
		else {
			*p++ = '\0';
		}

		texts[i] = pos[1];

		if(++i == 64)
			break;

		if(brk)
			break;
	}

	for(j = 0; j < i; j++) {
		if(!Q_stricmp(texts[j], configData->strvalue)) {
			j++;
			j %= i;
			configData->strvalue = String_Alloc(texts[j]);
			return;
		}
	}

	if(i > 0) {
		configData->strvalue = String_Alloc(texts[0]);
	}
	else {
		configData->strvalue = "";
	}
}

qboolean Item_ListBox_HandleKey(itemDef_t *item, int key, qboolean down, qboolean force) {
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	int count = DC->feederCount(item->special, item);
	int max, viewmax;

	if (force || (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory) && item->window.flags & WINDOW_HASFOCUS)) {
		max = Item_ListBox_MaxScroll(item);
			viewmax = (item->window.rect.h / listPtr->elementHeight);
			if ( key == K_UPARROW || key == K_KP_UPARROW ) 
			{
				if (!listPtr->notselectable) {
					listPtr->cursorPos--;
					if (listPtr->cursorPos < 0) {
						listPtr->cursorPos = 0;
					}
					if (listPtr->cursorPos < listPtr->startPos) {
						listPtr->startPos = listPtr->cursorPos;
					}
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					}
					item->cursorPos = listPtr->cursorPos;
					DC->feederSelection(item->special, item->cursorPos);
				}
				else {
					listPtr->startPos--;
					if (listPtr->startPos < 0)
						listPtr->startPos = 0;
				}
				return qtrue;
			}
			if ( key == K_DOWNARROW || key == K_KP_DOWNARROW ) 
			{
				if (!listPtr->notselectable) {
					listPtr->cursorPos++;
					if (listPtr->cursorPos < listPtr->startPos) {
						listPtr->startPos = listPtr->cursorPos;
					}
					if (listPtr->cursorPos >= count) {
						listPtr->cursorPos = count-1;
					}
					if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
						listPtr->startPos = listPtr->cursorPos - viewmax + 1;
					}
					item->cursorPos = listPtr->cursorPos;
					DC->feederSelection(item->special, item->cursorPos);
				}
				else {
					listPtr->startPos++;
					if (listPtr->startPos > max)
						listPtr->startPos = max;
				}
				return qtrue;
			}
		// mouse hit
		if (key == K_MOUSE1 || key == K_MOUSE2) {
			if (item->window.flags & WINDOW_LB_LEFTARROW) {
				listPtr->startPos--;
				if (listPtr->startPos < 0) {
					listPtr->startPos = 0;
				}
			} else if (item->window.flags & WINDOW_LB_RIGHTARROW) {
				// one down
				listPtr->startPos++;
				if (listPtr->startPos > max) {
					listPtr->startPos = max;
				}
			} else if (item->window.flags & WINDOW_LB_PGUP) {
				// page up
				listPtr->startPos -= viewmax;
				if (listPtr->startPos < 0) {
					listPtr->startPos = 0;
				}
			} else if (item->window.flags & WINDOW_LB_PGDN) {
				// page down
				listPtr->startPos += viewmax;
				if (listPtr->startPos > max) {
					listPtr->startPos = max;
				}
			} else if (item->window.flags & WINDOW_LB_THUMB) {
				// Display_SetCaptureItem(item);
			} else {
				int i;

				// Arnout: can't select something that doesn't exist
				if ( listPtr->cursorPos >= count ) {
					listPtr->cursorPos = count - 1;
				}

				// select an item
				if (DC->realTime < lastListBoxClickTime && listPtr->doubleClick) {
					Item_RunScript(item, listPtr->doubleClick);
				}
				lastListBoxClickTime = DC->realTime + DOUBLE_CLICK_DELAY;


				if(listPtr->elementStyle == LISTBOX_MULTI_CONTROLS) {
					configData_t* configData = DC->feederItemInfo(item->special, listPtr->cursorPos, item);
					if(configData) {

						for(i = 0; i < listPtr->numColumns; i++) {
							int x1 = (item->window.rect.x + listPtr->columnInfo[i].pos);

							if(DC->cursorx > x1 && 
								DC->cursorx <= (x1 + listPtr->columnInfo[i].width)) {

								if(configData->type == CONFIG_TYPE_SLIDER) {
									if(i >= 1) {
										itemDef_t slider;							
										editFieldDef_t range;

										memset(&slider, 0, sizeof(itemDef_t));
	
										slider.parent =				item->parent;
										slider.cvar =				configData->command;
										slider.window.flags =		WINDOW_HASFOCUS;

										slider.window.rect.x = item->window.rect.x + listPtr->columnInfo[1].pos + 8;
										slider.window.rect.y = item->window.rect.y + 4 + (listPtr->cursorPos-listPtr->startPos)*listPtr->elementHeight;
										slider.window.rect.w = SLIDER_WIDTH + SLIDER_END_WIDTH + SLIDER_END_WIDTH;
										slider.window.rect.h = listPtr->elementHeight;

										range.minVal = configData->primary;
										range.maxVal = configData->secondary;
										range.scale = 1.0;

										slider.typeData = (void*)&range;

										Item_Slider_HandleKey(&slider, key, down, configData->scale);

										configData->value = DC->getCVarValue(configData->command);
									} 
								} else if (configData->type == CONFIG_TYPE_YESNO) {
									if(i == 1) {
										configData->value = !configData->value;
									}
								} else if (configData->type == CONFIG_TYPE_FLOATLIST) {
									if(i == 1) {
										UI_CycleFloatList(configData);
									}
								} else if (configData->type == CONFIG_TYPE_TEXTLIST) {
									if(i == 1) {
										UI_CycleTextList(configData);
									}
								}
							}
						}
					}
				}

				if (item->cursorPos != listPtr->cursorPos) {
					item->cursorPos = listPtr->cursorPos;
					DC->feederSelection(item->special, item->cursorPos);
				}
			}
			return qtrue;
		}
		if ( key == K_HOME || key == K_KP_HOME) {
			// home
			listPtr->startPos = 0;
			return qtrue;
		}
		if ( key == K_END || key == K_KP_END) {
			// end
			listPtr->startPos = max;
			return qtrue;
		}
		if (key == K_PGUP || key == K_KP_PGUP ) {
			// page up
			if (!listPtr->notselectable) {
				listPtr->cursorPos -= viewmax;
				if (listPtr->cursorPos < 0) {
					listPtr->cursorPos = 0;
				}
				if (listPtr->cursorPos < listPtr->startPos) {
					listPtr->startPos = listPtr->cursorPos;
				}
				if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
					listPtr->startPos = listPtr->cursorPos - viewmax + 1;
				}
				item->cursorPos = listPtr->cursorPos;
				DC->feederSelection(item->special, item->cursorPos);
			}
			else {
				listPtr->startPos -= viewmax;
				if (listPtr->startPos < 0) {
					listPtr->startPos = 0;
				}
			}
			return qtrue;
		}
		if ( key == K_PGDN || key == K_KP_PGDN ) {
			// page down
			if (!listPtr->notselectable) {
				listPtr->cursorPos += viewmax;
				if (listPtr->cursorPos < listPtr->startPos) {
					listPtr->startPos = listPtr->cursorPos;
				}
				if (listPtr->cursorPos >= count) {
					listPtr->cursorPos = count-1;
				}
				if (listPtr->cursorPos >= listPtr->startPos + viewmax) {
					listPtr->startPos = listPtr->cursorPos - viewmax + 1;
				}
				item->cursorPos = listPtr->cursorPos;
				DC->feederSelection(item->special, item->cursorPos);
			}
			else {
				listPtr->startPos += viewmax;
				if (listPtr->startPos > max) {
					listPtr->startPos = max;
				}
			}
			return qtrue;
		}
	}
	return qfalse;
}

qboolean Item_CheckBox_HandleKey( itemDef_t *item, int key ) {
	if( Rect_ContainsPoint( &item->window.rect, DC->cursorx, DC->cursory ) && item->window.flags & WINDOW_HASFOCUS && item->cvar ) {
		if( key == K_MOUSE1 || key == K_ENTER || key == K_MOUSE2 || key == K_MOUSE3 ) {
			// ATVI Wolfenstein Misc #462
			// added the flag to toggle via action script only
//			if( !(item->cvarFlags & CVAR_NOTOGGLE) ) {
				if( item->type == ITEM_TYPE_TRICHECKBOX ) {
					int curvalue = DC->getCVarValue( item->cvar ) + 1;
					if( curvalue > 2 )
						curvalue = 0;
					DC->setCVar( item->cvar, va( "%i", curvalue ) );
				} else {
					DC->setCVar( item->cvar, va( "%i", !DC->getCVarValue( item->cvar ) ) );
				}
//			}
			return qtrue;
		}
	}
	return qfalse;
}

qboolean Item_YesNo_HandleKey(itemDef_t *item, int key) {

  if (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory) && item->window.flags & WINDOW_HASFOCUS && item->cvar) {
		if (key == K_MOUSE1 || key == K_ENTER || key == K_MOUSE2 || key == K_MOUSE3) {
	    DC->setCVar(item->cvar, va("%i", !DC->getCVarValue(item->cvar)));
		  return qtrue;
		}
  }

  return qfalse;

}

int Item_Multi_CountSettings(itemDef_t *item) {
	multiDef_t *multiPtr = (multiDef_t*)item->typeData;
	if (multiPtr == NULL) {
		return 0;
	}
	return multiPtr->count;
}

int Item_Multi_FindCvarByValue(itemDef_t *item) {
	char buff[1024];
	float value = 0;
	int i;
	multiDef_t *multiPtr = (multiDef_t*)item->typeData;
	if (multiPtr) {
		if (multiPtr->strDef) {
	    DC->getCVarString(item->cvar, buff, sizeof(buff));
		} else {
			value = DC->getCVarValue(item->cvar);
		}
		for (i = 0; i < multiPtr->count; i++) {
			if (multiPtr->strDef) {
				if (Q_stricmp(buff, multiPtr->cvarStr[i]) == 0) {
					return i;
				}
			} else {
 				if (multiPtr->cvarValue[i] == value) {
 					return i;
 				}
 			}
 		}
	}
	return 0;
}

const char *Item_Multi_Setting(itemDef_t *item) {
	char buff[1024];
	float value = 0;
	int i;
	multiDef_t *multiPtr = (multiDef_t*)item->typeData;
	if (multiPtr) {
		if (multiPtr->strDef) {
	    DC->getCVarString(item->cvar, buff, sizeof(buff));
		} else {
			value = DC->getCVarValue(item->cvar);
		}
		for (i = 0; i < multiPtr->count; i++) {
			if (multiPtr->strDef) {
				if (Q_stricmp(buff, multiPtr->cvarStr[i]) == 0) {
					return multiPtr->cvarList[i];
				}
			} else {
 				if (multiPtr->cvarValue[i] == value) {
					return multiPtr->cvarList[i];
 				}
 			}
 		}
	}
	return "";
}

qboolean Item_Multi_HandleKey(itemDef_t *item, int key) {
	multiDef_t *multiPtr = (multiDef_t*)item->typeData;
	if (multiPtr) {
	  if (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory) && item->window.flags & WINDOW_HASFOCUS && item->cvar) {
			if (key == K_MOUSE1 || key == K_ENTER ) {						// step forward
				int current = Item_Multi_FindCvarByValue(item) + 1;
				int max = Item_Multi_CountSettings(item);
				if ( current < 0 || current >= max ) {
					current = 0;
				}
				if (multiPtr->strDef) {
					DC->setCVar(item->cvar, multiPtr->cvarStr[current]);
				} else {
					float value = multiPtr->cvarValue[current];
					if (((float)((int) value)) == value) {
						DC->setCVar(item->cvar, va("%i", (int) value ));
					}
					else {
						DC->setCVar(item->cvar, va("%f", value ));
					}
				}
				return qtrue;
			}
			else if(key == K_MOUSE2 || key == K_MOUSE3) {					// step back
				signed int current = Item_Multi_FindCvarByValue(item) - 1;
				int max = Item_Multi_CountSettings(item) - 1;
				if ( current < 0 || current >= max ) {
					current = max;
				}
				if (multiPtr->strDef) {
					DC->setCVar(item->cvar, multiPtr->cvarStr[current]);
				} else {
					float value = multiPtr->cvarValue[current];
					if (((float)((int) value)) == value) {
						DC->setCVar(item->cvar, va("%i", (int) value ));
					}
					else {
						DC->setCVar(item->cvar, va("%f", value ));
					}
				}
				return qtrue;
			}
		}
	}
  return qfalse;
}

void Item_Action( itemDef_t *item ) {
	if ( item ) {
		Item_RunScript( item, item->action );
	}
}

#ifdef UI_EXPORTS
qboolean Item_TextField_HandleKey( itemDef_t *item, int key );
void Item_TextField_Paste( itemDef_t *item ) {
	int		pasteLen, i;
	char	buff[2048] = { 0 };

	trap_GetClipboardData( buff, sizeof( buff ) );

	if ( !*buff ) {
		return;
	}

	// send as if typed, so insert / overstrike works properly
	pasteLen = strlen( buff );
	for ( i = 0; i < pasteLen; i++ ) {
		Item_TextField_HandleKey( item, buff[i] | K_CHAR_FLAG );
	}
}
#endif

qboolean Item_TextField_HandleKey(itemDef_t *item, int key) {
	char buff[1024];
	int len;
	itemDef_t *newItem = NULL;
	editFieldDef_t *editPtr = (editFieldDef_t*)item->typeData;

	if (item->cvar) {

		memset(buff, 0, sizeof(buff));
		DC->getCVarString(item->cvar, buff, sizeof(buff));
		len = strlen(buff);
		if (editPtr->maxChars && len > editPtr->maxChars) {
			len = editPtr->maxChars;
		}

		// Gordon: make sure our cursorpos doesn't go oob, windows doesn't like negative memory copy operations :)
		if ( item->cursorPos < 0 || item->cursorPos > len ) {
			item->cursorPos = 0;
		}

		if ( key & K_CHAR_FLAG ) {
			key &= ~K_CHAR_FLAG;

#ifdef UI_EXPORTS
			if ( key == 'v' - 'a' + 1 ) {	// ctrl-v is paste
				Item_TextField_Paste( item );
				return qtrue;
			}
#endif


			if (key == 'h' - 'a' + 1 )	{	// ctrl-h is backspace
				if ( item->cursorPos > 0 ) {
					memmove( &buff[item->cursorPos - 1], &buff[item->cursorPos], len + 1 - item->cursorPos);
					item->cursorPos--;
					if (item->cursorPos < editPtr->paintOffset) {
						editPtr->paintOffset--;
					}
				}
				DC->setCVar(item->cvar, buff);
	    		return qtrue;
			}


			//
			// ignore any non printable chars
			//
			if ( key < 32) {
			    return qtrue;
		    }
			if ( key > 127) {
				return qtrue;
			}

			if (item->type == ITEM_TYPE_NUMERICFIELD) {
				if ((key < '0' || key > '9') && key != '.') {
					return qfalse;
				}
			}

			if (DC->getOverstrikeMode && !DC->getOverstrikeMode()) {
				if (( len == MAX_EDITFIELD - 1 ) || (editPtr->maxChars && len >= editPtr->maxChars)) {
					return qtrue;
				}
				memmove( &buff[item->cursorPos + 1], &buff[item->cursorPos], len + 1 - item->cursorPos );
			} else {
				if (editPtr->maxChars && item->cursorPos >= editPtr->maxChars) {
					return qtrue;
				}
			}

			buff[item->cursorPos] = key;

			DC->setCVar(item->cvar, buff);

			if (item->cursorPos < len + 1) {
				item->cursorPos++;
				if (editPtr->maxPaintChars && item->cursorPos > editPtr->maxPaintChars) {
					editPtr->paintOffset++;
				}
			}

		} else {

			if ( key == K_DEL || key == K_KP_DEL ) {
				if ( item->cursorPos < len ) {
					memmove( buff + item->cursorPos, buff + item->cursorPos + 1, len - item->cursorPos);
					DC->setCVar(item->cvar, buff);
				}
				return qtrue;
			}

			if ( key == K_RIGHTARROW || key == K_KP_RIGHTARROW ) 
			{
				if (editPtr->maxPaintChars && item->cursorPos >= editPtr->maxPaintChars && item->cursorPos < len) {
					item->cursorPos++;
					editPtr->paintOffset++;
					return qtrue;
				}
				if (item->cursorPos < len) {
					item->cursorPos++;
				} 
				return qtrue;
			}

			if ( key == K_LEFTARROW || key == K_KP_LEFTARROW ) 
			{
				if ( item->cursorPos > 0 ) {
					item->cursorPos--;
				}
				if (item->cursorPos < editPtr->paintOffset) {
					editPtr->paintOffset--;
				}
				return qtrue;
			}

			if ( key == K_HOME || key == K_KP_HOME) {// || ( tolower(key) == 'a' && trap_Key_IsDown( K_CTRL ) ) ) {
				item->cursorPos = 0;
				editPtr->paintOffset = 0;
				return qtrue;
			}

			if ( key == K_END || key == K_KP_END)  {// ( tolower(key) == 'e' && trap_Key_IsDown( K_CTRL ) ) ) {
				item->cursorPos = len;
				if(item->cursorPos > editPtr->maxPaintChars) {
					editPtr->paintOffset = len - editPtr->maxPaintChars;
				}
				return qtrue;
			}

			if ( key == K_INS || key == K_KP_INS ) {
				if(DC->keyIsDown(K_SHIFT)) {
#ifdef UI_EXPORTS
					Item_TextField_Paste( item );
#endif
				} else {
					DC->setOverstrikeMode(!DC->getOverstrikeMode());
				}
				return qtrue;
			}
		}

		if (key == K_TAB || key == K_DOWNARROW || key == K_KP_DOWNARROW) {
			newItem = Menu_SetNextCursorItem(item->parent);
			if (newItem && (newItem->type == ITEM_TYPE_EDITFIELD || newItem->type == ITEM_TYPE_NUMERICFIELD)) {
				g_editItem = newItem;
			}
		}

		if (key == K_UPARROW || key == K_KP_UPARROW) {
			newItem = Menu_SetPrevCursorItem(item->parent);
			if (newItem && (newItem->type == ITEM_TYPE_EDITFIELD || newItem->type == ITEM_TYPE_NUMERICFIELD)) {
				g_editItem = newItem;
			}
		}

		if ( key == K_ENTER && item->action)  {
			Item_Action(item);
			return qtrue;
		}

		if ( key == K_ENTER || key == K_KP_ENTER || key == K_ESCAPE)  {
			return qfalse;
		}

		return qtrue;
	}
	return qfalse;

}

static void Scroll_ListBox_AutoFunc(void *p) {
	scrollInfo_t *si = (scrollInfo_t*)p;
	if (DC->realTime > si->nextScrollTime) { 
		// need to scroll which is done by simulating a click to the item
		// this is done a bit sideways as the autoscroll "knows" that the item is a listbox
		// so it calls it directly
		Item_ListBox_HandleKey(si->item, si->scrollKey, qtrue, qfalse);
		si->nextScrollTime = DC->realTime + si->adjustValue; 
	}

	if (DC->realTime > si->nextAdjustTime) {
		si->nextAdjustTime = DC->realTime + SCROLL_TIME_ADJUST;
		if (si->adjustValue > SCROLL_TIME_FLOOR) {
			si->adjustValue -= SCROLL_TIME_ADJUSTOFFSET;
		}
	}
}

static void Scroll_ListBox_ThumbFunc(void *p) {
	scrollInfo_t *si = (scrollInfo_t*)p;
	rectDef_t r;
	int pos, max;

	listBoxDef_t *listPtr = (listBoxDef_t*)si->item->typeData;
/*	if (si->item->window.flags & WINDOW_HORIZONTAL) {
		if (DC->cursorx == si->xStart) {
			return;
		}
		r.x = si->item->window.rect.x + SCROLLBAR_SIZE + 1;
		r.y = si->item->window.rect.y + si->item->window.rect.h - SCROLLBAR_SIZE - 1;
		r.h = SCROLLBAR_SIZE;
		r.w = si->item->window.rect.w - (SCROLLBAR_SIZE*2) - 2;
		max = Item_ListBox_MaxScroll(si->item);
		//
		pos = (DC->cursorx - r.x - SCROLLBAR_SIZE/2) * max / (r.w - SCROLLBAR_SIZE);
		if (pos < 0) {
			pos = 0;
		}
		else if (pos > max) {
			pos = max;
		}
		listPtr->startPos = pos;
		si->xStart = DC->cursorx;
	}
	else */if (DC->cursory != si->yStart) {

		r.x = si->item->window.rect.x + si->item->window.rect.w - SCROLLBAR_SIZE - 1;
		r.y = si->item->window.rect.y + SCROLLBAR_SIZE + 1;
		r.h = si->item->window.rect.h - (SCROLLBAR_SIZE*2) - 2;
		r.w = SCROLLBAR_SIZE;
		max = Item_ListBox_MaxScroll(si->item);
		//
		pos = (DC->cursory - r.y - SCROLLBAR_SIZE/2) * max / (r.h - SCROLLBAR_SIZE);
		if (pos < 0) {
			pos = 0;
		}
		else if (pos > max) {
			pos = max;
		}
		listPtr->startPos = pos;
		si->yStart = DC->cursory;
	}

	if (DC->realTime > si->nextScrollTime) { 
		// need to scroll which is done by simulating a click to the item
		// this is done a bit sideways as the autoscroll "knows" that the item is a listbox
		// so it calls it directly
		// Arnout: clear doubleclicktime though!
		lastListBoxClickTime = 0;
		Item_ListBox_HandleKey(si->item, si->scrollKey, qtrue, qfalse);
		si->nextScrollTime = DC->realTime + si->adjustValue; 
	}

	if (DC->realTime > si->nextAdjustTime) {
		si->nextAdjustTime = DC->realTime + SCROLL_TIME_ADJUST;
		if (si->adjustValue > SCROLL_TIME_FLOOR) {
			si->adjustValue -= SCROLL_TIME_ADJUSTOFFSET;
		}
	}
}

static void Scroll_Slider_ThumbFunc(void *p) {
	float x, value, cursorx, myscale;
	scrollInfo_t *si = (scrollInfo_t*)p;
	editFieldDef_t *editDef = si->item->typeData;

	if (si->item->text) {
		x = si->item->textRect.x + si->item->textRect.w + 8;
	} else {
		x = si->item->window.rect.x;
	}

	x += SLIDER_END_WIDTH;

	cursorx = DC->cursorx;

	if (cursorx < x) {
		cursorx = x;
	} else if (cursorx > x + SLIDER_WIDTH) {
		cursorx = x + SLIDER_WIDTH;
	}

	myscale = 1.0;
	if(editDef->scale != 1)
		myscale = editDef->scale;

	value = cursorx - x;
	value /= SLIDER_WIDTH;
	value *= (editDef->maxVal - editDef->minVal);
	value += editDef->minVal;
	DC->setCVar(si->item->cvar, va("%f", value / myscale));


/*
				rectDef_t testRect;
				width = SLIDER_WIDTH;
				if (item->text) {
					x = item->textRect.x + item->textRect.w + 8;
				} else {
					x = item->window.rect.x;
				}

				x += SLIDER_END_WIDTH;
				testRect = item->window.rect;
//				testRect.x = x + SLIDER_END_WIDTH;
//				testRect.w = SLIDER_WIDTH;
				//DC->Print("slider x: %f\n", testRect.x);
				//DC->Print("slider w: %f\n", testRect.w);
				if (Rect_ContainsPoint(&testRect, DC->cursorx, DC->cursory)) {
					work = DC->cursorx - x;
					value = work / width;
					if(value > 1.0f)
						value = 1.0f;
					if(value < 0.0f)
						value = 0.0f;
					value *= (editDef->maxVal - editDef->minVal);
					value += editDef->minVal;
					DC->setCVar(item->cvar, va("%f", value / scale));
					DC->Print("Setting slider val to %f\n", value);
					return qtrue;
				}
*/

}

void Item_StartCapture(itemDef_t *item, int key) {
	int flags;
	switch (item->type) {
    case ITEM_TYPE_EDITFIELD:
    case ITEM_TYPE_NUMERICFIELD:

		case ITEM_TYPE_LISTBOX:
		{
			flags = Item_ListBox_OverLB(item, DC->cursorx, DC->cursory);
			if (flags & (WINDOW_LB_LEFTARROW | WINDOW_LB_RIGHTARROW)) {
				scrollInfo.nextScrollTime = DC->realTime + SCROLL_TIME_START;
				scrollInfo.nextAdjustTime = DC->realTime + SCROLL_TIME_ADJUST;
				scrollInfo.adjustValue = SCROLL_TIME_START;
				scrollInfo.scrollKey = key;
				scrollInfo.scrollDir = (flags & WINDOW_LB_LEFTARROW) ? qtrue : qfalse;
				scrollInfo.item = item;
				captureData = &scrollInfo;
				captureFunc = &Scroll_ListBox_AutoFunc;
				itemCapture = item;
			} else if (flags & WINDOW_LB_THUMB) {
				scrollInfo.scrollKey = key;
				scrollInfo.item = item;
				scrollInfo.xStart = DC->cursorx;
				scrollInfo.yStart = DC->cursory;
				captureData = &scrollInfo;
				captureFunc = &Scroll_ListBox_ThumbFunc;
				itemCapture = item;
			}
			break;
		}
		case ITEM_TYPE_SLIDER:
		{
			flags = Item_Slider_OverSlider(item, DC->cursorx, DC->cursory);
			if (flags & WINDOW_LB_THUMB) {
				scrollInfo.scrollKey = key;
				scrollInfo.item = item;
				scrollInfo.xStart = DC->cursorx;
				scrollInfo.yStart = DC->cursory;
				captureData = &scrollInfo;
				captureFunc = &Scroll_Slider_ThumbFunc;
				itemCapture = item;
			}
			break;
		}
	}
}

void Item_StopCapture(itemDef_t *item) {

}

qboolean Item_HandleKey(itemDef_t *item, int key, qboolean down) {

	if (itemCapture) {
		Item_StopCapture(itemCapture);
		itemCapture = NULL;
		captureFunc = NULL;
		captureData = NULL;
	} else {
		if (down && (key == K_MOUSE1 || key == K_MOUSE2 || key == K_MOUSE3)) {
			Item_StartCapture(item, key);
		}
	}

	if (!down) {
		return qfalse;
	}

  switch (item->type) {
    case ITEM_TYPE_BUTTON:
      return qfalse;
      break;
    case ITEM_TYPE_RADIOBUTTON:
      return qfalse;
      break;
    case ITEM_TYPE_CHECKBOX:
	case ITEM_TYPE_TRICHECKBOX:
		return Item_CheckBox_HandleKey( item, key );
		break;
    case ITEM_TYPE_EDITFIELD:
    case ITEM_TYPE_NUMERICFIELD:
      //return Item_TextField_HandleKey(item, key);
      return qfalse;
      break;
    case ITEM_TYPE_COMBO:
      return qfalse;
      break;
    case ITEM_TYPE_LISTBOX:
      return Item_ListBox_HandleKey(item, key, down, qfalse);
      break;
    case ITEM_TYPE_YESNO:
      return Item_YesNo_HandleKey(item, key);
      break;
    case ITEM_TYPE_MULTI:
      return Item_Multi_HandleKey(item, key);
      break;
    case ITEM_TYPE_OWNERDRAW:
      return Item_OwnerDraw_HandleKey(item, key);
      break;
    case ITEM_TYPE_SLIDER:
      return Item_Slider_HandleKey(item, key, down, 1);
      break;
    //case ITEM_TYPE_IMAGE:
    //  Item_Image_Paint(item);
    //  break;
    default:
      return qfalse;
      break;
  }

  //return qfalse;
}

itemDef_t *Menu_SetPrevCursorItem(menuDef_t *menu) {
	qboolean wrapped = qfalse;
	int oldCursor = menu->cursorItem;
  
	if (menu->cursorItem < 0) {
	    menu->cursorItem = menu->itemCount-1;
	    wrapped = qtrue;
	} 

	while (menu->cursorItem > -1) {
    
		menu->cursorItem--;
		if (menu->cursorItem < 0 ) {
			//djbob
			if(wrapped)
				break;	// 1 item menu would break before....

			wrapped = qtrue;
			menu->cursorItem = menu->itemCount -1;
		}

		if (Item_SetFocus(menu->items[menu->cursorItem], DC->cursorx, DC->cursory)) {
			Menu_HandleMouseMove(menu, menu->items[menu->cursorItem]->window.rect.x + 1, menu->items[menu->cursorItem]->window.rect.y + 1);      
			return menu->items[menu->cursorItem];
		}
	}
	menu->cursorItem = oldCursor;
	return NULL;
}

itemDef_t *Menu_SetNextCursorItem(menuDef_t *menu) {

	qboolean wrapped = qfalse;
	int oldCursor = menu->cursorItem;


	if (menu->cursorItem == -1) {
		menu->cursorItem = 0;
		wrapped = qtrue;
	}

	while (menu->cursorItem < menu->itemCount) {

		menu->cursorItem++;
		if (menu->cursorItem >= menu->itemCount) {
			//djbob
			if(wrapped)
				break;	// 1 item menu would break before....
			wrapped = qtrue;
			menu->cursorItem = 0;
		}
		
		if (Item_SetFocus(menu->items[menu->cursorItem], DC->cursorx, DC->cursory)) {
			Menu_HandleMouseMove(menu, menu->items[menu->cursorItem]->window.rect.x + 1, menu->items[menu->cursorItem]->window.rect.y + 1);
			return menu->items[menu->cursorItem];
		}    
	}

	menu->cursorItem = oldCursor;
	return NULL;
}

static void Window_CloseCinematic(windowDef_t *window) {
	if (window->style == WINDOW_STYLE_CINEMATIC && window->cinematic >= 0) {
		DC->stopCinematic(window->cinematic);
		window->cinematic = -1;
	}
}

static void Menu_CloseCinematics(menuDef_t *menu) {
	if (menu) {
		int i;
		Window_CloseCinematic(&menu->window);
		for (i = 0; i < menu->itemCount; i++) {
			Window_CloseCinematic(&menu->items[i]->window);
			if (menu->items[i]->type == ITEM_TYPE_OWNERDRAW) {
				DC->stopCinematic(0-menu->items[i]->window.ownerDraw);
			}
		}
	}
}

static void Display_CloseCinematics() {
	int i;
	for (i = 0; i < menuCount; i++) {
		Menu_CloseCinematics(&Menus[i]);
	}
}

void  Menus_Activate(menuDef_t *menu) {
	int i;
	int vis;
	for (i = 0; i < menuCount; i++) {
		Menus[i].window.flags &= ~WINDOW_HASFOCUS;
	}

	vis = menu->window.flags & WINDOW_VISIBLE;
	menu->window.flags |= (WINDOW_HASFOCUS | WINDOW_VISIBLE);
	if (menu->onOpen && !vis) {
		itemDef_t item;
		item.parent = menu;
		Item_RunScript(&item, menu->onOpen);
	}

	if (menu->soundName && *menu->soundName) {
//		DC->stopBackgroundTrack();					// you don't want to do this since it will reset s_rawend
		DC->startBackgroundTrack(menu->soundName, menu->soundName, 0);
	}

	Display_CloseCinematics();

}

int Display_VisibleMenuCount() {
	int i, count;
	count = 0;
	for (i = 0; i < menuCount; i++) {
		if (Menus[i].window.flags & (WINDOW_FORCED | WINDOW_VISIBLE)) {
			count++;
		}
	}
	return count;
}

void Menus_HandleOOBClick(menuDef_t *menu, int key, qboolean down) {
	if (menu) {
		int i;
		// basically the behaviour we are looking for is if there are windows in the stack.. see if 
		// the cursor is within any of them.. if not close them otherwise activate them and pass the 
		// key on.. force a mouse move to activate focus and script stuff 
		if (down && menu->window.flags & WINDOW_OOB_CLICK) {
			itemDef_t it;

			Menu_RunCloseScript(menu);
			menu->window.flags &= ~(WINDOW_HASFOCUS | WINDOW_VISIBLE);

			it.parent = menu;
			Item_RunScript(&it, menu->onOOBClick);
		}

		for (i = 0; i < menuCount; i++) {
			if ( Menu_OverActiveItem(&Menus[i], DC->cursorx, DC->cursory) ) {
				// RR2DO2: don't close windows that marked as keep open on focus lost
				if( !(menu->window.flags & WINDOW_KEEPOPENONFOCUSLOST) ) {
					Menu_RunCloseScript(menu);
					menu->window.flags &= ~WINDOW_VISIBLE;
				}
				menu->window.flags &= ~WINDOW_HASFOCUS;
				Menus_Activate(&Menus[i]);
				Menu_HandleMouseMove(&Menus[i], DC->cursorx, DC->cursory);
				Menu_HandleKey(&Menus[i], key, down);
				return;
			}
		}

		if (Display_VisibleMenuCount() == 0) {
			if (DC->Pause) {
				DC->Pause(qfalse);
			}
		}
		Display_CloseCinematics();
	}
}

static rectDef_t *Item_CorrectedTextRect(itemDef_t *item) {
	static rectDef_t rect;
	memset(&rect, 0, sizeof(rectDef_t));
	if (item) {
		rect = item->textRect;
		if (rect.w) {
			rect.y -= rect.h;
		}
	}
	return &rect;
}

void Menu_HandleKey(menuDef_t *menu, int key, qboolean down) {
	int i;
	itemDef_t *item = NULL;
	itemDef_t *wheelitem = NULL;
	qboolean inHandler = qfalse;

	if (inHandler) {
		return;
	}

	inHandler = qtrue;
	
#ifndef CGAME

	if (g_waitingForKey && down) {
		KeyBinder_HandleKey(key, down);
		inHandler = qfalse;
		return;
	}

#endif

	if (g_editingField && down) {
		if (!Item_TextField_HandleKey(g_editItem, key)) {
			g_editingField = qfalse;
			g_editItem = NULL;
			inHandler = qfalse;
			return;
		} else if (key == K_MOUSE1 || key == K_MOUSE2 || key == K_MOUSE3) {
			g_editingField = qfalse;
			g_editItem = NULL;
			Display_MouseMove(NULL, DC->cursorx, DC->cursory);
		} else if (key == K_TAB || key == K_UPARROW || key == K_DOWNARROW) {
			return;
		}
	}

	if (menu == NULL) {
		inHandler = qfalse;
		return;
	}

	// see if the mouse is within the window bounds and if so is this a mouse click
	if (down && !(menu->window.flags & WINDOW_POPUP) && !Rect_ContainsPoint(&menu->window.rect, DC->cursorx, DC->cursory)) {
		static qboolean inHandleKey = qfalse;
		if (!inHandleKey && (key == K_MOUSE1 || key == K_MOUSE2 || key == K_MOUSE3)) {
			inHandleKey = qtrue;
			inHandler = qfalse;
			Menus_HandleOOBClick(menu, key, down);
			inHandleKey = qfalse;
			return;
		}
	}

	// get the item with focus
	for (i = 0; i < menu->itemCount; i++) {
		if (menu->items[i]->window.flags & WINDOW_HASFOCUS) {
			item = menu->items[i];
		}
	}

	if (item != NULL) {
		if (Item_HandleKey(item, key, down)) {
			Item_Action(item);
			inHandler = qfalse;
			return;
		}
	}

	if (!down) {
		inHandler = qfalse;
		return;
	}

	// djbob OnKey events
	for(i = 0; i < menu->numKeyScripts; i++) {
		if(key == menu->keyScripts[i].key) {
			itemDef_t it;
			it.parent = menu;
			Item_RunScript(&it, menu->keyScripts[i].script);
		}
	}

	// default handling
	switch ( key ) {

		case K_F11:
			if (DC->getCVarValue("developer")) {
				debugMode ^= 1;
			}
			break;

		case K_F12:
			if (DC->getCVarValue("developer")) {
				DC->executeText(EXEC_APPEND, "screenshot_etf\n");
			}
			break;
		case K_KP_UPARROW:
		case K_UPARROW:
			Menu_SetPrevCursorItem(menu);
			break;

		case K_ESCAPE:
			if (!g_waitingForKey && menu->onESC) {
				itemDef_t it;
				it.parent = menu;
				Item_RunScript(&it, menu->onESC);
			}
			break;
		case K_TAB:
		case K_KP_DOWNARROW:
		case K_DOWNARROW:
			Menu_SetNextCursorItem(menu);
			break;

		case K_MOUSE1:
		case K_MOUSE2:
			if (item && down) {
				if (item->type == ITEM_TYPE_TEXT) {
					if (Rect_ContainsPoint(Item_CorrectedTextRect(item), DC->cursorx, DC->cursory)) {
						Item_Action(item);
					}
				} else if (item->type == ITEM_TYPE_EDITFIELD || item->type == ITEM_TYPE_NUMERICFIELD) {
					if (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory)) {
						item->cursorPos = 0;
						g_editingField = qtrue;
						g_editItem = item;
						// see elsewhere for venomous comment about this particular piece of "functionality"
						//%	DC->setOverstrikeMode(qtrue);
					}
				} else {
					if (Rect_ContainsPoint(&item->window.rect, DC->cursorx, DC->cursory)) {
						Item_Action(item);
					}
				}
			}
			break;

		case K_JOY1:
		case K_JOY2:
		case K_JOY3:
		case K_JOY4:
		case K_AUX1:
		case K_AUX2:
		case K_AUX3:
		case K_AUX4:
		case K_AUX5:
		case K_AUX6:
		case K_AUX7:
		case K_AUX8:
		case K_AUX9:
		case K_AUX10:
		case K_AUX11:
		case K_AUX12:
		case K_AUX13:
		case K_AUX14:
		case K_AUX15:
		case K_AUX16:
			break;
		case K_KP_ENTER:
		case K_ENTER:
			if (item) {
				if (item->type == ITEM_TYPE_EDITFIELD || item->type == ITEM_TYPE_NUMERICFIELD) {
					item->cursorPos = 0;
					g_editingField = qtrue;
					g_editItem = item;
					// see elsewhere for venomous comment about this particular piece of "functionality"
					//%	DC->setOverstrikeMode(qtrue);
				} else {
					Item_Action(item);
				}
			}
			break;
		case K_MWHEELUP:
		case K_MWHEELDOWN:
			{
				menuDef_t *wheelmenu = NULL;;
				
				if(!Rect_ContainsPoint(&menu->window.rect, DC->cursorx, DC->cursory) )
				{
					// find the menu this key was over
					for (i = 0; i < menuCount; i++) {
						if ( Menu_OverActiveItem(&Menus[i], DC->cursorx, DC->cursory) ) {
							wheelmenu = &Menus[i];
							break;
						}
					}
				}
				else
					wheelmenu = menu;

				if(!wheelmenu)
					break;

				for (i = 0; i < wheelmenu->itemCount; i++) {
					if(Rect_ContainsPoint(&wheelmenu->items[i]->window.rect, DC->cursorx, DC->cursory))
					{
						wheelitem = wheelmenu->items[i];
						if(wheelitem && wheelitem->type == ITEM_TYPE_LISTBOX) {
							listBoxDef_t *listPtr;
							int max = Item_ListBox_MaxScroll(wheelitem);
							int viewmax;

							listPtr = (listBoxDef_t*)wheelitem->typeData;

							viewmax = (wheelitem->window.rect.h / listPtr->elementHeight);

							if(key == K_MWHEELUP) {
								int scroll = viewmax < 6 ? 1 : 3;
								listPtr->startPos -= scroll;
								if (listPtr->startPos < 0) {
									listPtr->startPos = 0;
								}
							}
							else {
								int scroll = viewmax < 6 ? 1 : 3;
								listPtr->startPos += scroll;
								if (listPtr->startPos > max) {
									listPtr->startPos = max;
								}
							}

							break;
						}
					}
				}
				break;
			}
	}
	inHandler = qfalse;
}

void ToWindowCoords(float *x, float *y, windowDef_t *window) {
	if (window->border != 0) {
		*x += window->borderSize;
		*y += window->borderSize;
	} 
	*x += window->rect.x;
	*y += window->rect.y;
}

void Rect_ToWindowCoords(rectDef_t *rect, windowDef_t *window) {
	ToWindowCoords(&rect->x, &rect->y, window);
}

void Item_SetTextExtents(itemDef_t *item, int *width, int *height, const char *text) {
	fontStruct_t *parentfont;
	const char *textPtr = (text) ? text : item->text;
	menuDef_t *parent = (menuDef_t*)item->parent;

	parentfont = ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font;

	if (textPtr == NULL ) {
		return;
	}

	*width = item->textRect.w;
	*height = item->textRect.h;

	// keeps us from computing the widths and heights more than once
	if (*width == 0 || (item->type == ITEM_TYPE_OWNERDRAW && item->textalignment == ITEM_ALIGN_CENTER)) {
		int originalWidth = DC->textWidth(item->text, item->textscale, 0, &parent->font);

		// FIXME: djbob: think this is b0rking something up, need to review later
		if (item->type == ITEM_TYPE_OWNERDRAW/* && (item->textalignment == ITEM_ALIGN_CENTER || item->textalignment == ITEM_ALIGN_RIGHT)*/) {
//			originalWidth += DC->ownerDrawWidth(item->window.ownerDraw, item->textscale, &parent->font);
		} else if (item->type == ITEM_TYPE_EDITFIELD && item->textalignment == ITEM_ALIGN_CENTER && item->cvar) {
			char buff[256];
			DC->getCVarString(item->cvar, buff, 256);
			originalWidth += DC->textWidth(buff, item->textscale, 0, &parent->font);
		}

		*width = DC->textWidth(textPtr, item->textscale, 0, parentfont);
		*height = DC->textHeight(textPtr, item->textscale, 0, parentfont);
		item->textRect.w = *width;
		item->textRect.h = *height;
		item->textRect.x = item->textalignx;
		item->textRect.y = item->textaligny;
		if (item->textalignment == ITEM_ALIGN_RIGHT) {
			item->textRect.x = item->textalignx - *width;//originalWidth;
		} else if (item->textalignment == ITEM_ALIGN_CENTER) {
			item->textRect.x = item->textalignx - *width/*originalWidth*/ / 2;
		}

		ToWindowCoords(&item->textRect.x, &item->textRect.y, &item->window);
	}
}

void Item_TextColor(itemDef_t *item, vec4_t *newColor) {
	vec4_t lowLight;
	menuDef_t *parent = (menuDef_t*)item->parent;

	Fade(&item->window.flags, &item->window.foreColor[3], parent->fadeClamp, &item->window.nextTime, parent->fadeCycle, qtrue, parent->fadeAmount);

	if (item->window.flags & WINDOW_HASFOCUS && !(item->window.flags & WINDOW_NOPULSEONFOCUS)) {	// RR2DO2: added WINDOW_NOPULSEONFOCUS flag
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		LerpColor(parent->focusColor,lowLight,*newColor,0.5+0.5*sin(DC->realTime / PULSE_DIVISOR));
	} else if (item->textStyle == ITEM_TEXTSTYLE_BLINK && !((DC->realTime/BLINK_DIVISOR) & 1)) {
		lowLight[0] = 0.8 * item->window.foreColor[0]; 
		lowLight[1] = 0.8 * item->window.foreColor[1]; 
		lowLight[2] = 0.8 * item->window.foreColor[2]; 
		lowLight[3] = 0.8 * item->window.foreColor[3]; 
		LerpColor(item->window.foreColor,lowLight,*newColor,0.5+0.5*sin(DC->realTime / PULSE_DIVISOR));
	} else {
		memcpy(newColor, &item->window.foreColor, sizeof(vec4_t));
		// items can be enabled and disabled based on cvars
	}

	if (item->enableCvar && *item->enableCvar && item->cvarTest && *item->cvarTest) {
		if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !Item_EnableShowViaCvar(item, CVAR_ENABLE)) {
			memcpy(newColor, &parent->disableColor, sizeof(vec4_t));
		}
	}
}

void Item_Text_AutoWrapped_Paint(itemDef_t *item) {
	char text[1024];
	const char *p, *textPtr, *newLinePtr;
	char buff[1024];
	int width, height, len, textWidth, newLine;//, newLineWidth;
	float y;
	vec4_t color;
	menuDef_t *parent = (menuDef_t*)item->parent;

	textWidth = 0;
	newLinePtr = NULL;

	if (item->text == NULL) {
		if (item->cvar == NULL) {
			return;
		}
		else {
			DC->getCVarString(item->cvar, text, sizeof(text));
			textPtr = text;
		}
	}
	else {
		textPtr = item->text;
	}
	if (*textPtr == '\0') {
		return;
	}
	Item_TextColor(item, &color);
	Item_SetTextExtents(item, &width, &height, textPtr);

	y = item->textaligny;
	len = 0;
	buff[0] = '\0';
	newLine = 0;
	//newLineWidth = 0;
	p = textPtr;
	while (p) {
		if (*p == ' ' || *p == '\t' || *p == '\n' || *p == '\0') {
			newLine = len;
			newLinePtr = p+1;
			//newLineWidth = textWidth;
		}
		textWidth = DC->textWidth(buff, item->textscale, 0, &parent->font);
		if ( (newLine && textWidth > item->window.rect.w) || *p == '\n' || *p == '\0') {
			if (len) {
				//if (item->textalignment == ITEM_ALIGN_LEFT) {
					item->textRect.x = item->textalignx;
				/*} else if (item->textalignment == ITEM_ALIGN_RIGHT) {
					item->textRect.x = item->textalignx - newLineWidth;
				} else if (item->textalignment == ITEM_ALIGN_CENTER) {
					item->textRect.x = item->textalignx - newLineWidth / 2;
				}*/
				item->textRect.y = y;
				ToWindowCoords(&item->textRect.x, &item->textRect.y, &item->window);
				//
				buff[newLine] = '\0';
				DC->drawText(item->textRect.x, item->textRect.y, item->textscale, color, buff, 0, 0, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment);
			}
			if (*p == '\0') {
				break;
			}
			//
			y += height + 5;
			p = newLinePtr;
			len = 0;
			newLine = 0;
			//newLineWidth = 0;
			continue;
		}
		buff[len++] = *p++;
		buff[len] = '\0';
	}
}

void Item_Text_Wrapped_Paint(itemDef_t *item) {
	char text[1024];
	const char *p, *start, *textPtr;
	char buff[1024];
	int width, height;
	float x, y;
	vec4_t color;
	menuDef_t *parent = (menuDef_t*)item->parent;

	// now paint the text and/or any optional images
	// default to left

	if (item->text == NULL) {
		if (item->cvar == NULL) {
			return;
		}
		else {
			DC->getCVarString(item->cvar, text, sizeof(text));
			textPtr = text;
		}
	}
	else {
		textPtr = item->text;
	}
	if (*textPtr == '\0') {
		return;
	}

	Item_TextColor(item, &color);
	Item_SetTextExtents(item, &width, &height, textPtr);

	x = item->textRect.x;
	y = item->textRect.y;
	start = textPtr;
	p = strchr(textPtr, '\r');
	while (p && *p) {
		strncpy(buff, start, p-start+1);
		buff[p-start] = '\0';
		DC->drawText(x, y, item->textscale, color, buff, 0, 0, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment);
		y += height + 5;
		start += p - start + 1;
		p = strchr(p+1, '\r');
	}
	DC->drawText(x, y, item->textscale, color, start, 0, 0, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment);
}

void Item_Text_Paint(itemDef_t *item) {
	char text[1024];
	const char *textPtr;
	int height, width;
	vec4_t color;
	menuDef_t *parent = (menuDef_t*)item->parent;
	int align = ITEM_ALIGN_LEFT;

	if (item->window.flags & WINDOW_WRAPPED) {
		Item_Text_Wrapped_Paint(item);
		return;
	}
	if (item->window.flags & WINDOW_AUTOWRAPPED) {
		Item_Text_AutoWrapped_Paint(item);
		return;
	}

	if (item->text == NULL) {
		if (item->cvar == NULL) {
			return;
		}
		else {
			DC->getCVarString(item->cvar, text, sizeof(text));
			if( item->window.flags & WINDOW_TEXTASINT ) {
				COM_StripExtension(text, text, sizeof(text));
				item->textRect.w = 0;	// force recalculation
			} else if( item->window.flags & WINDOW_TEXTASFLOAT ) {
				char *s = va( "%.2f", atof(text) );
				Q_strncpyz( text, s, sizeof(text) );
				item->textRect.w = 0;	// force recalculation
			}
			textPtr = text;
			align = item->textalignment;
		}
	}
	else {
		textPtr = item->text;
	}

	// this needs to go here as it sets extents for cvar types as well
	Item_SetTextExtents(item, &width, &height, textPtr);

	if (*textPtr == '\0') {
		return;
	}


	Item_TextColor(item, &color);

	if (item->window.flags & WINDOW_GREY) {
		color[3] *= 0.3f;
	}

	DC->drawText(item->textRect.x, item->textRect.y, item->textscale, color, textPtr, 0, 0, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), align );
}

void Item_MLText_Paint(itemDef_t *item) {
	char text[1024];
	const char *textPtr;
	int height, width;
	vec4_t color;
	menuDef_t *parent = (menuDef_t*)item->parent;
	char *p, *s;
	float x, y;
	//char clrCode = 0;
	//qboolean useClr;
	fontStruct_t* font = ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font;

	if (item->text == NULL) {
		if (item->cvar == NULL) {
			return;
		}
		else {
			DC->getCVarString(item->cvar, text, sizeof(text));
			textPtr = text;
		}
	}
	else {
		textPtr = item->text;
	}

	// this needs to go here as it sets extents for cvar types as well
	Item_SetTextExtents(item, &width, &height, textPtr);

	if (!*textPtr) {
		return;
	}


	Item_TextColor(item, &color);

	if (item->window.flags & WINDOW_GREY) {
		color[3] *= 0.3f;
	}

	x = item->window.rect.x + item->textalignx;
	y = item->window.rect.y + item->textaligny;

	Q_strncpyz(text, textPtr, 1024);

	s = p = text;
	do {
		p++;

		/*if(Q_IsColorStringPtr(p)) {
			clrCode = *(p+1);
		}*/

		if(*p == '\n' || !*p) {
			float alignmentoffset;

			if(*p) {
				*p++ = '\0';
			}

			switch( item->textalignment )
			{
			case ITEM_ALIGN_LEFT: 
				alignmentoffset = 0; 
				break;
			case ITEM_ALIGN_CENTER: 
				alignmentoffset = -0.5f * DC->textWidth( s, item->textscale, 0, font ); 
				break;
			case ITEM_ALIGN_RIGHT: 
				alignmentoffset = -DC->textWidth( s, item->textscale, 0, font ); 
				break;
			default: 
				alignmentoffset = 0; 
				break;
			}

			DC->drawText( x + alignmentoffset, y, item->textscale, color, s, 0, 0, item->textStyle, font, ITEM_ALIGN_LEFT );

			/*if(clrCode) {
				useClr = qtrue;
			}*/

			y += item->special;
			if(!*p) {
				break;
			}
			s = p;
		}
	} while(*p); 	
}


//float			trap_Cvar_VariableValue( const char *var_name );
//void			trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize );

void Item_TextField_Paint(itemDef_t *item) {
	char buff[1024];
	vec4_t newColor, lowLight;
	int offset;
	menuDef_t *parent = (menuDef_t*)item->parent;
	editFieldDef_t *editPtr = (editFieldDef_t*)item->typeData;

	Item_Text_Paint(item);

	buff[0] = '\0';

	if (item->cvar) {
		DC->getCVarString(item->cvar, buff, sizeof(buff));
	} 

	parent = (menuDef_t*)item->parent;

	if (item->window.flags & WINDOW_HASFOCUS) {
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		LerpColor(parent->focusColor,lowLight,newColor,0.5+0.5*sin(DC->realTime / PULSE_DIVISOR));
	} else {
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}

	if(item->textStyle == ITEM_TEXTSTYLE_PAD && editPtr->maxChars) {
		int i;
		int end = strlen(buff);
		int extra = editPtr->maxChars - end;

		for(i = 0; i < extra; i++) {
			buff[end++] = '_';
		}

		buff[end] = '\0';
	}

	offset = (item->text && *item->text) ? 8 : 0;
	if (item->window.flags & WINDOW_HASFOCUS && g_editingField) {
		char cursor = DC->getOverstrikeMode() ? '_' : '|';
		DC->drawTextWithCursor(item->textRect.x + item->textRect.w + offset, item->textRect.y, item->textscale, newColor, buff + editPtr->paintOffset, item->cursorPos - editPtr->paintOffset , cursor, editPtr->maxPaintChars, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment );
	} else {
		DC->drawText(item->textRect.x + item->textRect.w + offset, item->textRect.y, item->textscale, newColor, buff + editPtr->paintOffset, 0, editPtr->maxPaintChars, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment );
	}

}

void Item_YesNo_Paint(itemDef_t *item) {
	vec4_t newColor, lowLight;
	float value;
	menuDef_t *parent = (menuDef_t*)item->parent;

	value = (item->cvar) ? DC->getCVarValue(item->cvar) : 0;

	if (item->window.flags & WINDOW_HASFOCUS) {
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		LerpColor(parent->focusColor,lowLight,newColor,0.5+0.5*sin(DC->realTime / PULSE_DIVISOR));
	} else {
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}

	if (item->text) {
		Item_Text_Paint(item);
		DC->drawText(item->textRect.x + item->textRect.w + 8, item->textRect.y, item->textscale, newColor, (value != 0) ? "Yes" : "No", 0, 0, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment);
	} else {
		DC->drawText(item->textRect.x, item->textRect.y, item->textscale, newColor, (value != 0) ? "Yes" : "No", 0, 0, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment);
	}
}

void Item_LED_Paint(itemDef_t *item) {
	float value;

	value = (item->cvar) ? DC->getCVarValue(item->cvar) : 0;
	DC->drawHandlePic(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h, value ? DC->Assets.LEDon : DC->Assets.LEDoff);
}

void Item_CheckBox_Paint( itemDef_t *item ) {
	vec4_t newColor, lowLight;
	float value;
	menuDef_t *parent = (menuDef_t*)item->parent;
	qboolean hasMultiText = qfalse;
	multiDef_t *multiPtr = (multiDef_t*)item->typeData;

	value = (item->cvar) ? DC->getCVarValue(item->cvar) : 0;

	if (item->window.flags & WINDOW_HASFOCUS) {
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		LerpColor(parent->focusColor,lowLight,newColor,0.5+0.5*sin(DC->realTime / PULSE_DIVISOR));
	} else {
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}

	if( multiPtr && multiPtr->count ) {
		hasMultiText = qtrue;
	}

	if (item->text) {
//		Item_Text_Paint( item );
		/*
		if( item->type == ITEM_TYPE_TRICHECKBOX && value == 2 )
			DC->drawHandlePic( item->textRect.x + item->textRect.w + 8, item->window.rect.y, item->window.rect.h, item->window.rect.h, DC->Assets.checkboxCheckNo );
		else if( value )
			DC->drawHandlePic( item->textRect.x + item->textRect.w + 8, item->window.rect.y, item->window.rect.h, item->window.rect.h, DC->Assets.checkboxCheck );
		else
			DC->drawHandlePic( item->textRect.x + item->textRect.w + 8, item->window.rect.y, item->window.rect.h, item->window.rect.h, DC->Assets.checkboxCheckNot );
		*/
		if( item->type == ITEM_TYPE_TRICHECKBOX && value == 2 )
			DC->drawHandlePic( item->window.rect.x, item->window.rect.y, item->window.rect.h, item->window.rect.h, DC->Assets.checkboxCheckNo );
		else if( value )
			DC->drawHandlePic( item->window.rect.x, item->window.rect.y, item->window.rect.h, item->window.rect.h, DC->Assets.checkboxCheck );
		else
			DC->drawHandlePic( item->window.rect.x, item->window.rect.y, item->window.rect.h, item->window.rect.h, DC->Assets.checkboxCheckNot );


		if( hasMultiText ) {
			vec4_t colour;

			Item_TextColor( item, &colour );
			DC->drawText( item->textRect.x + item->textRect.w + 8 + item->window.rect.h + 4, item->textRect.y, item->textscale,
						  colour, Item_Multi_Setting( item ), 0, 0, item->textStyle,
						  ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment );
		}
		else
		{
			vec4_t color;
			Item_TextColor(item, &color);

			if (item->window.flags & WINDOW_GREY) {
				color[3] *= 0.3f;
			}
			DC->drawText(item->window.rect.x + item->window.rect.h + 4 + item->textalignx, item->window.rect.y + item->textaligny, 
				item->textscale, color, item->text, 0, 0, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), 
				item->textalignment );
		}
	} else {
		if( item->type == ITEM_TYPE_TRICHECKBOX && value == 2 )
			DC->drawHandlePic( item->window.rect.x, item->window.rect.y, item->window.rect.h, item->window.rect.h, DC->Assets.checkboxCheckNo );
		else if( value )
			DC->drawHandlePic( item->window.rect.x, item->window.rect.y, item->window.rect.h, item->window.rect.h, DC->Assets.checkboxCheck );
		else
			DC->drawHandlePic( item->window.rect.x, item->window.rect.y, item->window.rect.h, item->window.rect.h, DC->Assets.checkboxCheckNot );

		if( hasMultiText ) {
			vec4_t colour;

			Item_TextColor( item, &colour );
			DC->drawText( item->window.rect.x + item->window.rect.h + 4, item->window.rect.y + item->textaligny, item->textscale,
						  colour, Item_Multi_Setting( item ), 0, 0, item->textStyle,
						  ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment );
		}
	}
}

void Item_Multi_Paint(itemDef_t *item) {
	vec4_t newColor, lowLight;
	const char *text = "";
	menuDef_t *parent = (menuDef_t*)item->parent;

	if (item->window.flags & WINDOW_HASFOCUS) {
		lowLight[0] = 0.8 * parent->focusColor[0]; 
		lowLight[1] = 0.8 * parent->focusColor[1]; 
		lowLight[2] = 0.8 * parent->focusColor[2]; 
		lowLight[3] = 0.8 * parent->focusColor[3]; 
		LerpColor(parent->focusColor,lowLight,newColor,0.5+0.5*sin(DC->realTime / PULSE_DIVISOR));
	} else {
		memcpy(&newColor, &item->window.foreColor, sizeof(vec4_t));
	}

	text = Item_Multi_Setting(item);

	if (item->text) {
		Item_Text_Paint(item);
		DC->drawText(item->textRect.x + item->textRect.w + 8, item->textRect.y, item->textscale, newColor, text, 0, 0, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment);
	} else {
		DC->drawText(item->textRect.x, item->textRect.y, item->textscale, newColor, text, 0, 0, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment);
	}
}

/*
=================
Controls_GetKeyAssignment
=================
*/
void Controls_GetKeyAssignment (const char *command, int *twokeys)
{
	int		count;
	int		j;
	char	b[256];

	twokeys[0] = twokeys[1] = -1;
	count = 0;

	for ( j = 0; j < 256; j++ )
	{
		DC->getBindingBuf( j, b, 256 );
		if ( *b == 0 ) {
			continue;
		}
		if ( !Q_stricmp( b, command ) ) {
			twokeys[count] = j;
			count++;
			if (count == 2) {
				break;
			}
		}
	}
}

void Item_Slider_Paint(itemDef_t *item, float scale) {
	float x, y;

	y = item->window.rect.y + 2;
	if (item->text) {
		Item_Text_Paint(item);
		x = item->textRect.x + item->textRect.w + 8;
	} else {
		x = item->window.rect.x;
	}

	DC->drawHandlePic( x, y, SLIDER_END_WIDTH, SLIDER_HEIGHT, DC->Assets.sliderEndLeft );
	DC->drawHandlePic( x + SLIDER_END_WIDTH, y, SLIDER_WIDTH, SLIDER_HEIGHT, DC->Assets.sliderBar );
	DC->drawHandlePic( x + SLIDER_WIDTH /*+ SLIDER_END_WIDTH*/, y, SLIDER_END_WIDTH, SLIDER_HEIGHT, DC->Assets.sliderEndRight );

	x = Item_Slider_ThumbPosition(item, scale);
	DC->drawHandlePic( x, y, SLIDER_THUMB_WIDTH, SLIDER_THUMB_HEIGHT, DC->Assets.sliderThumb );
}

void Q3F_DrawItemBorder(rectDef_t* r, int border, int set, menuDef_t* menu, float alpha) {
	float y2, y3, h2, w2, x;
	vec4_t alphaclr;

	alphaclr[0] = 1.f;
	alphaclr[1] = 1.f;
	alphaclr[2] = 1.f;
	alphaclr[3] = alpha;

	DC->setColor(alphaclr);

	x = r->x;

	y2 = r->y + border;
	y3 = r->y + r->h - border;
	h2 = r->h - (2 * border);
	w2 = r->w - (2 * border);

	if(menu->borderBitmaps[0])
		DC->drawAdjustedPic(x, r->y,	border, border, menu->borderBitmaps[0]);
	if(menu->borderBitmaps[1])
		DC->drawAdjustedPic(x, y2,		border, h2,		menu->borderBitmaps[1]);
	if(menu->borderBitmaps[2])
		DC->drawAdjustedPic(x, y3,		border, border, menu->borderBitmaps[2]);

	x += border;

	if(menu->borderBitmaps[3])
		DC->drawAdjustedPic(x, r->y,	w2,		border, menu->borderBitmaps[3]);
	if(menu->borderBitmaps[4])
		DC->drawAdjustedPic(x, y2,		w2,		h2,		menu->borderBitmaps[4]);
	if(menu->borderBitmaps[5])
		DC->drawAdjustedPic(x, y3,		w2,		border, menu->borderBitmaps[5]);

	x += r->w - (2*border);

	if(menu->borderBitmaps[6])
		DC->drawAdjustedPic(x, r->y,	border, border, menu->borderBitmaps[6]);
	if(menu->borderBitmaps[7])
		DC->drawAdjustedPic(x, y2,		border, h2,		menu->borderBitmaps[7]);
	if(menu->borderBitmaps[8])
		DC->drawAdjustedPic(x, y3,		border, border, menu->borderBitmaps[8]);

	DC->setColor(NULL);
}

void Item_Border_Paint(itemDef_t* item) {
	rectDef_t r;
	float alpha = 1.f;

	if(item->window.ownerDraw) {
		if(!DC->ownerDrawSize(item->window.ownerDraw, &item->window.rect, &r, item, &alpha))
			return;
	}
	else {
		r.x = item->window.rect.x;
		r.y = item->window.rect.y;
		r.w = item->window.rect.w;
		r.h = item->window.rect.h;
	}

	Fade(&item->window.flags, &item->window.foreColor[3], ((menuDef_t*)item->parent)->fadeClamp, &item->window.nextTime, ((menuDef_t*)item->parent)->fadeCycle, qtrue, ((menuDef_t*)item->parent)->fadeAmount);

	Q3F_DrawItemBorder(&r, item->special, item->textalignment, (menuDef_t*)item->parent, alpha);
}

qboolean Display_KeyBindPending() {
	return g_waitingForKey;
}

void AdjustFrom640(float *x, float *y, float *w, float *h) {
	//*x = *x * DC->scale + DC->bias;
	*x *= DC->xscale;
	*y *= DC->yscale;
	*w *= DC->xscale;
	*h *= DC->yscale;
}

void Item_Image_Paint(itemDef_t *item) {
	if (item == NULL) {
		return;
	}
	DC->drawHandlePic(item->window.rect.x+1, item->window.rect.y+1, item->window.rect.w-2, item->window.rect.h-2, item->asset);
}

const char* Item_Listbox_FeederTextForControl_Bind(int column, const configData_t* configData) {
	static char buf[32];

	if(!configData)
		return "";

	switch(column) {
	case 0:
		return configData->title;
	case 1:
		if(configData->primary != -1) {
			DC->keynumToStringBuf(configData->primary, buf, 32);
			return buf;
		}
		break;
	case 2:
		if(configData->secondary != -1) {
			DC->keynumToStringBuf(configData->secondary, buf, 32);
			return buf;
		}
		break;
	}

	return "";
}

const char* Item_Listbox_FeederTextForControl_YesNo(int column, const configData_t* configData) {
	switch(column) {
	case 0:
		return configData->title;
	case 1:
		return configData->value ? "Yes" : "No";
	}
	return "";
}

const char* Item_Listbox_FeederTextForControl_FloatList(int column, const configData_t* configData) {
	static char buffer[1024];
	char* pos[2];
	char* p;
	qboolean brk = qfalse;

	switch(column) {
	case 0:
		return configData->title;
	case 1:
		
		p = configData->string;
		Q_strncpyz(buffer, p, 1024);
		p = buffer;

		while(qtrue) {
			pos[0] = p;
			p = strchr(p, ';');
			if(!p) {
				break;
			}
			
			*p++ = '\0';
			pos[1] = p;

			p = strchr(p, ';');
			if(!p) {
				brk = qtrue;
			}
			else {
				*p++ = '\0';
			}

			if(fabs(atof(pos[1]) - configData->value) < 0.01) {
				return pos[0];
			}

			if(brk)
				break;
		}
		break;
	default:
		return "";
	}
	return "Custom";
}

const char* Item_Listbox_FeederTextForControl_TextList(int column, const configData_t* configData) {
	static char buffer[1024];
	char* pos[2];
	char* p;
	qboolean brk = qfalse;

	switch(column) {
	case 0:
		return configData->title;
	case 1:
		
		p = configData->string;
		Q_strncpyz(buffer, p, 1024);
		p = buffer;

		while(qtrue) {
			pos[0] = p;
			p = strchr(p, ';');
			if(!p) {
				break;
			}
			
			*p++ = '\0';
			pos[1] = p;

			p = strchr(p, ';');
			if(!p) {
				brk = qtrue;
			}
			else {
				*p++ = '\0';
			}

			if(!Q_stricmp(pos[1], configData->strvalue)) {
				return pos[0];
			}

			if(brk)
				break;
		}
		break;
	}
	return "Custom";
}

void Item_ListBox_Paint(itemDef_t *item) {
	float x, y, size, count, i, thumb;
	qhandle_t image;
	qhandle_t optionalImage;
	listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
	menuDef_t *parent = (menuDef_t*)item->parent;

	// the listbox is horizontal or vertical and has a fixed size scroll bar going either direction
	// elements are enumerated from the DC and either text or image handles are acquired from the DC as well
	// textscale is used to size the text, textalignx and textaligny are used to size image elements
	// there is no clipping available so only the last completely visible item is painted
	count = DC->feederCount(item->special, item);
	// default is vertical if horizontal flag is not here

	/*  Slothy - consider putting a border around usermenu
	if(count && (item->special == FEEDER_CUSTOMMENU))
	{
		vec4_t bgcolour, framecolour;

		if(!item->window.border) {
 			framecolour[0] = framecolour[1] = framecolour[2] = 0.5f;
			framecolour[3] = 0.7f;
			bgcolour[0] = bgcolour[1] = bgcolour[2] = 0.2;
			bgcolour[3] = 0.5f;

			DC->fillRect(item->window.rect.x, item->window.rect.y, item->window.rect.w, (count + 1) * listPtr->elementHeight, bgcolour);
			DC->drawRect(item->window.rect.x, item->window.rect.y, item->window.rect.w, (count + 1) * listPtr->elementHeight, 1, framecolour);
		}
	}
	*/

	if(Item_ListBox_MaxScroll(item)) {
		// draw scrollbar to right side of the window
		x = item->window.rect.x + item->window.rect.w - SCROLLBAR_SIZE - 1;
		y = item->window.rect.y + 1;
		DC->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarArrowUp);
		y += SCROLLBAR_SIZE - 1;

		listPtr->endPos = listPtr->startPos;
		size = item->window.rect.h - (SCROLLBAR_SIZE * 2);
		DC->drawHandlePic(x, y, SCROLLBAR_SIZE, size+1, DC->Assets.scrollBar);
		y += size - 1;
		DC->drawHandlePic(x, y, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarArrowDown);
		// thumb
		thumb = Item_ListBox_ThumbDrawPosition(item);
		if (thumb > y - SCROLLBAR_SIZE - 1) {
			thumb = y - SCROLLBAR_SIZE - 1;
		}
		DC->drawHandlePic(x, thumb, SCROLLBAR_SIZE, SCROLLBAR_SIZE, DC->Assets.scrollBarThumb);
	}

	// adjust size for item painting
	size = item->window.rect.h - 2;

	if(DC->feederPaintSpecial(item)) {
		return;
	} else if (listPtr->elementStyle == LISTBOX_IMAGE) {
		// fit = 0;
		x = item->window.rect.x + 1;
		y = item->window.rect.y + 1;
		for (i = listPtr->startPos; i < count; i++) {
			// always draw at least one
			// which may overdraw the box if it is too small for the element
			image = DC->feederItemImage(item->special, i, item);
			if (image) {
				DC->drawHandlePic(x+1, y+1, listPtr->elementWidth - 2, listPtr->elementHeight - 2, image);
			}

			if (i == item->cursorPos) {
				DC->drawRect(x, y, listPtr->elementWidth - 1, listPtr->elementHeight - 1, item->window.borderSize, item->window.borderColor);
			}

			listPtr->endPos++;
			size -= listPtr->elementWidth;
			if (size < listPtr->elementHeight) {
				listPtr->drawPadding = listPtr->elementHeight - size;
				break;
			}
			y += listPtr->elementHeight;
			// fit++;
		}
	} else if (listPtr->elementStyle == LISTBOX_MULTI_CONTROLS) {
		int max;

		x = item->window.rect.x + 1;
		y = item->window.rect.y + 1;
		
		// always draw at least one
		// which may overdraw the box if it is too small for the element
		if (listPtr->numColumns > 0) {
			max = listPtr->numColumns;
		}
		else {
			max = 1;
		}

		for (i = listPtr->startPos; i < count; i++) {
			const char *text;
			int j;
			configData_t* configData = DC->feederItemInfo(item->special, i, item);

			if (i == item->cursorPos) {
				DC->fillRect(x + 2, y + 2, item->window.rect.w - SCROLLBAR_SIZE - 4, listPtr->elementHeight, item->window.outlineColor);
				if( configData->type == CONFIG_TYPE_BIND && g_waitingForKey) {
					DC->drawRect(x + 2, y + 2, item->window.rect.w - SCROLLBAR_SIZE - 4, listPtr->elementHeight, 1, item->window.borderColor);
				}
			}
				
			for (j = 0; j < max; j++) {
				itemDef_t slider;
				editFieldDef_t range;

				switch(configData->type) {
				case CONFIG_TYPE_BIND:
					text = Item_Listbox_FeederTextForControl_Bind(j, configData);
					DC->drawText(x + 4 + listPtr->columnInfo[j].pos + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, text, 0, listPtr->columnInfo[j].maxChars, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment);
					break;
				case CONFIG_TYPE_YESNO:
					text = Item_Listbox_FeederTextForControl_YesNo(j, configData);
					DC->drawText(x + 4 + listPtr->columnInfo[j].pos + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, text, 0, listPtr->columnInfo[j].maxChars, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment);
					break;
				case CONFIG_TYPE_SLIDER:

					switch(j) {
					case 0:
						text = configData->title;
						DC->drawText(x + 4 + listPtr->columnInfo[j].pos + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, text, 0, listPtr->columnInfo[j].maxChars, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment);
						break;

					case 1:
						memset(&slider, 0, sizeof(itemDef_t));

						slider.parent =				item->parent;
						slider.cvar =				configData->command;
						slider.window.flags =		i == item->cursorPos ? WINDOW_HASFOCUS : 0;
						memcpy(&slider.window.foreColor, &item->window.foreColor, sizeof(vec4_t));

						slider.window.rect.x = x + 4 + listPtr->columnInfo[j].pos;
						slider.window.rect.y = y;

						range.minVal = configData->primary;
						range.maxVal = configData->secondary;
						range.scale = configData->scale;

						slider.typeData = (void*)&range;

						Item_Slider_Paint(&slider, configData->scale);
						break;
					}
					break;
				case CONFIG_TYPE_FLOATLIST:
					text = Item_Listbox_FeederTextForControl_FloatList(j, configData);
					DC->drawText(x + 4 + listPtr->columnInfo[j].pos + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, text, 0, listPtr->columnInfo[j].maxChars, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment);
					break;
				case CONFIG_TYPE_TEXTLIST:
					text = Item_Listbox_FeederTextForControl_TextList(j, configData);
					DC->drawText(x + 4 + listPtr->columnInfo[j].pos + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, text, 0, listPtr->columnInfo[j].maxChars, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment);
					break;
				}
			}

			size -= listPtr->elementHeight;
			if (size < listPtr->elementHeight) {
				listPtr->drawPadding = listPtr->elementHeight - size;
				break;
			}
			listPtr->endPos++;
			y += listPtr->elementHeight;
			// fit++;
		}
	} else {
		x = item->window.rect.x + 1;
		y = item->window.rect.y + 1;
		for (i = listPtr->startPos; i < count; i++) {
			const char *text;
			// always draw at least one
			// which may overdraw the box if it is too small for the element

			if (i == item->cursorPos) {
				DC->fillRect(x + 2, y + 2, item->window.rect.w - SCROLLBAR_SIZE - 4, listPtr->elementHeight, item->window.outlineColor);
			}
				
			if (listPtr->numColumns > 0) {
				int j;
				for (j = 0; j < listPtr->numColumns; j++) {
					text = DC->feederItemText(item->special, i, j, &optionalImage, item);
					if (optionalImage >= 0) {
						DC->drawHandlePic(x + listPtr->columnInfo[j].pos, y + 4/*+ listPtr->elementHeight */, listPtr->columnInfo[j].width - 4, listPtr->elementHeight - 2, optionalImage);
					} else if (text) {
						DC->drawText(x + 4 + listPtr->columnInfo[j].pos + item->textalignx, y + listPtr->elementHeight+ item->textaligny, item->textscale, item->window.foreColor, text, 0, listPtr->columnInfo[j].maxChars, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment);
					}
				}
			} else {
				text = DC->feederItemText(item->special, i, 0, &optionalImage, item);
				if (optionalImage >= 0) {
					//DC->drawHandlePic(x + 4 + listPtr->elementHeight, y, listPtr->columnInfo[j].width, listPtr->columnInfo[j].width, optionalImage);
				} else if (text) {
					DC->drawText(x + 4 + item->textalignx, y + listPtr->elementHeight + item->textaligny, item->textscale, item->window.foreColor, text, 0, 0, item->textStyle, ( ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font ), item->textalignment);
				}
			}

			size -= listPtr->elementHeight;
			if (size < listPtr->elementHeight) {
				listPtr->drawPadding = listPtr->elementHeight - size;
				break;
			}
			listPtr->endPos++;
			y += listPtr->elementHeight;
			// fit++;
		}
	}
}

void Item_OwnerDraw_Paint(itemDef_t *item) {
	if (item == NULL) {
		return;
	}

	if (DC->ownerDrawItem) {
		vec4_t color, lowLight;
		menuDef_t *parent = (menuDef_t*)item->parent;
		Fade(&item->window.flags, &item->window.foreColor[3], parent->fadeClamp, &item->window.nextTime, parent->fadeCycle, qtrue, parent->fadeAmount);
		memcpy(&color, &item->window.foreColor, sizeof(color));
		if (item->numColors > 0 && DC->getValue) {
			// if the value is within one of the ranges then set color to that, otherwise leave at default
			int i;
			float f = DC->getValue(item->window.ownerDraw);
			for (i = 0; i < item->numColors; i++) {
				if (f >= item->colorRanges[i].low && f <= item->colorRanges[i].high) {
					memcpy(&color, &item->colorRanges[i].color, sizeof(color));
					break;
				}
			}
		}

		if (item->window.flags & WINDOW_HASFOCUS) {
			lowLight[0] = 0.8 * parent->focusColor[0]; 
			lowLight[1] = 0.8 * parent->focusColor[1]; 
			lowLight[2] = 0.8 * parent->focusColor[2]; 
			lowLight[3] = 0.8 * parent->focusColor[3]; 
			LerpColor(parent->focusColor,lowLight,color,0.5+0.5*sin(DC->realTime / PULSE_DIVISOR));
		} else if (item->textStyle == ITEM_TEXTSTYLE_BLINK && !((DC->realTime/BLINK_DIVISOR) & 1)) {
			lowLight[0] = 0.8 * item->window.foreColor[0]; 
			lowLight[1] = 0.8 * item->window.foreColor[1]; 
			lowLight[2] = 0.8 * item->window.foreColor[2]; 
			lowLight[3] = 0.8 * item->window.foreColor[3]; 
			LerpColor(item->window.foreColor,lowLight,color,0.5+0.5*sin(DC->realTime / PULSE_DIVISOR));
		}

		if (item->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !Item_EnableShowViaCvar(item, CVAR_ENABLE)) {
			memcpy(color, parent->disableColor, sizeof(vec4_t));
		}
	
		// big ugly hack
		if (item->text && item->window.ownerDraw != CG_FORT_MENU_TITLE) {
			Item_Text_Paint(item);
			if (item->text[0]) {
				// +8 is an offset kludge to properly align owner draw items that have text combined with them
				DC->ownerDrawItem( item, item->textRect.x + item->textRect.w + 8, item->window.rect.y, item->window.rect.w, item->window.rect.h, 0, item->textaligny, item->window.ownerDraw, item->window.ownerDrawFlags, item->alignment, item->special, item->textscale, color, item->window.background, item->textStyle, item->textalignment );
			} else {
				DC->ownerDrawItem( item, item->textRect.x + item->textRect.w, item->window.rect.y, item->window.rect.w, item->window.rect.h, 0, item->textaligny, item->window.ownerDraw, item->window.ownerDrawFlags, item->alignment, item->special, item->textscale, color, item->window.background, item->textStyle, item->textalignment );
			}
		} else {
			DC->ownerDrawItem( item, item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h, item->textalignx, item->textaligny, item->window.ownerDraw, item->window.ownerDrawFlags, item->alignment, item->special, item->textscale, color, item->window.background, item->textStyle, item->textalignment );
		}
	}
}


void Item_Paint(itemDef_t *item) {
	vec4_t red;
	menuDef_t *parent;
	red[0] = red[3] = 1;
	red[1] = red[2] = 0;

	if (item == NULL) {
		return;
	}

	parent = (menuDef_t*)item->parent;

	if (item->window.flags & WINDOW_ORBITING) {
		if (DC->realTime > item->window.nextTime) {
			float rx, ry, a, c, s, w, h;
      
			item->window.nextTime = DC->realTime + item->window.offsetTime;
			// translate
			w = item->window.rectClient.w / 2;
			h = item->window.rectClient.h / 2;
			rx = item->window.rectClient.x + w - item->window.rectEffects.x;
			ry = item->window.rectClient.y + h - item->window.rectEffects.y;
			a = 3 * M_PI / 180;
			c = cos(a);
			s = sin(a);
			item->window.rectClient.x = (rx * c - ry * s) + item->window.rectEffects.x - w;
			item->window.rectClient.y = (rx * s + ry * c) + item->window.rectEffects.y - h;
			Item_UpdatePosition(item);
		}
	}


	if (item->window.flags & WINDOW_INTRANSITION) {
		if (DC->realTime > item->window.nextTime) {
			int done = 0;

			item->window.nextTime = DC->realTime + item->window.offsetTime;
			// transition the x,y
			if (item->window.rectClient.x == item->window.rectEffects.x) {
				done++;
			} else {
				if (item->window.rectClient.x < item->window.rectEffects.x) {
					item->window.rectClient.x += item->window.rectEffects2.x;
					if (item->window.rectClient.x > item->window.rectEffects.x) {
						item->window.rectClient.x = item->window.rectEffects.x;
						done++;
					}
				} else {
					item->window.rectClient.x -= item->window.rectEffects2.x;
					if (item->window.rectClient.x < item->window.rectEffects.x) {
						item->window.rectClient.x = item->window.rectEffects.x;
						done++;
					}
				}
			}
			if (item->window.rectClient.y == item->window.rectEffects.y) {
				done++;
			} else {
				if (item->window.rectClient.y < item->window.rectEffects.y) {
					item->window.rectClient.y += item->window.rectEffects2.y;
					if (item->window.rectClient.y > item->window.rectEffects.y) {
						item->window.rectClient.y = item->window.rectEffects.y;
						done++;
					}
				} else {
					item->window.rectClient.y -= item->window.rectEffects2.y;
					if (item->window.rectClient.y < item->window.rectEffects.y) {
						item->window.rectClient.y = item->window.rectEffects.y;
						done++;
					}
				}
			}
			if (item->window.rectClient.w == item->window.rectEffects.w) {
				done++;
			} else {
				if (item->window.rectClient.w < item->window.rectEffects.w) {
					item->window.rectClient.w += item->window.rectEffects2.w;
					if (item->window.rectClient.w > item->window.rectEffects.w) {
						item->window.rectClient.w = item->window.rectEffects.w;
						done++;
					}
				} else {
					item->window.rectClient.w -= item->window.rectEffects2.w;
					if (item->window.rectClient.w < item->window.rectEffects.w) {
						item->window.rectClient.w = item->window.rectEffects.w;
						done++;
					}
				}
			}
			if (item->window.rectClient.h == item->window.rectEffects.h) {
				done++;
			} else {
				if (item->window.rectClient.h < item->window.rectEffects.h) {
					item->window.rectClient.h += item->window.rectEffects2.h;
					if (item->window.rectClient.h > item->window.rectEffects.h) {
						item->window.rectClient.h = item->window.rectEffects.h;
						done++;
					}
				} else {
					item->window.rectClient.h -= item->window.rectEffects2.h;
					if (item->window.rectClient.h < item->window.rectEffects.h) {
						item->window.rectClient.h = item->window.rectEffects.h;
						done++;
					}
				}
			}

			Item_UpdatePosition(item);

			if (done == 4) {
				item->window.flags &= ~WINDOW_INTRANSITION;
			}
		}
	}

	if (item->window.ownerDrawFlags && DC->ownerDrawVisible) {
		if (!DC->ownerDrawVisible(item->window.ownerDrawFlags)) {
			return;
/*			item->window.flags &= ~WINDOW_VISIBLE;
		} else {
			item->window.flags |= WINDOW_VISIBLE;*/
		}
	}

	if (item->cvarFlags & (CVAR_SHOW | CVAR_HIDE)) {
		if (!Item_EnableShowViaCvar(item, CVAR_SHOW)) {
			return;
		}
	}

	if (!(item->window.flags & WINDOW_VISIBLE)) {
		return;
	}

#ifdef CGAME
	// following 2 are HUD-only

	// slothy - class test
	if((item->classLimit != CLASS_ALL) && DC->playerClass) {
		if(!(DC->playerClass & item->classLimit))
			return;
	}

	// slothy - weapon test
	if(item->weaponLimit) {
		int curweapon = 0;

		if(!cg.snap)
			return;

		switch(cg.snap->ps.weapon) {
			case 1 : curweapon = WEAPON_AXE; break;
			case 2 : curweapon = WEAPON_SHOTGUN; break;
			case 3 : curweapon = WEAPON_SUPERSHOTGUN; break;
			case 4 : curweapon = WEAPON_NAILGUN; break;
			case 5 : curweapon = WEAPON_SUPERNAILGUN; break;
			case 6 : curweapon = WEAPON_GRENADE_LAUNCHER; break;
			case 7 : curweapon = WEAPON_ROCKET_LAUNCHER; break;
			case 8 : curweapon = WEAPON_SNIPER_RIFLE; break;
			case 9 : curweapon = WEAPON_RAILGUN; break;
			case 10 : curweapon = WEAPON_FLAMETHROWER; break;
			case 11 : curweapon = WEAPON_MINIGUN; break;
			case 12 : curweapon = WEAPON_ASSAULTRIFLE; break;
			case 13 : curweapon = WEAPON_DARTGUN; break;
			case 14 : curweapon = WEAPON_PIPELAUNCHER; break;
			case 15 : curweapon = WEAPON_NAPALMCANNON; break;
			default: curweapon = 0; break;
		}
		if(!(curweapon & item->weaponLimit))
			return;
	}
#endif

	// paint the rect first.. 
	Window_Paint(&item->window, parent->fadeAmount , parent->fadeClamp, parent->fadeCycle);

/*	if (debugMode) {
		vec4_t color;
		rectDef_t *r = Item_CorrectedTextRect(item);
		color[1] = color[3] = 1;
		color[0] = color[2] = 0;
		DC->drawRect(r->x, r->y, r->w, r->h, 1, color);
	}*/

	//DC->drawRect(item->window.rect.x, item->window.rect.y, item->window.rect.w, item->window.rect.h, 1, red);

	switch (item->type) {
    case ITEM_TYPE_OWNERDRAW:
		Item_OwnerDraw_Paint(item);
		break;
    case ITEM_TYPE_TEXT:
    case ITEM_TYPE_BUTTON:
		Item_Text_Paint(item);
		break;
	case ITEM_TYPE_MLTEXT:
		Item_MLText_Paint(item);
		break;
    case ITEM_TYPE_RADIOBUTTON:
		break;
    case ITEM_TYPE_CHECKBOX:
	case ITEM_TYPE_TRICHECKBOX:
		Item_CheckBox_Paint(item);
		break;
    case ITEM_TYPE_EDITFIELD:
    case ITEM_TYPE_NUMERICFIELD:
		Item_TextField_Paint(item);
		break;
    case ITEM_TYPE_COMBO:
		break;
    case ITEM_TYPE_LISTBOX:
		Item_ListBox_Paint(item);
		break;
    case ITEM_TYPE_YESNO:
		Item_YesNo_Paint(item);
		break;
    case ITEM_TYPE_MULTI:
		Item_Multi_Paint(item);
		break;
    case ITEM_TYPE_SLIDER:
		Item_Slider_Paint(item, 1);
		break;
    case ITEM_TYPE_BORDER:
		Item_Border_Paint(item);
		break;
	case ITEM_TYPE_LED:
		Item_LED_Paint(item);
		break;
	default:
		break;
	}
}

void Menu_Init(menuDef_t *menu) {
	memset(menu, 0, sizeof(menuDef_t));
	menu->cursorItem = -1;
	menu->fadeAmount = DC->Assets.fadeAmount;
	menu->fadeClamp = DC->Assets.fadeClamp;
	menu->fadeCycle = DC->Assets.fadeCycle;
	menu->classLimit = CLASS_ALL;
	Window_Init(&menu->window);
}

itemDef_t *Menu_GetFocusedItem(menuDef_t *menu) {
  int i;
  if (menu) {
    for (i = 0; i < menu->itemCount; i++) {
      if (menu->items[i]->window.flags & WINDOW_HASFOCUS) {
        return menu->items[i];
      }
    }
  }
  return NULL;
}

menuDef_t *Menu_GetFocused() {
	int i;

	for (i = 0; i < menuCount; i++) {
		if ((Menus[i].window.flags & WINDOW_HASFOCUS) && (Menus[i].window.flags & WINDOW_VISIBLE)) {
	//		Com_Printf("Current focus menu: %s\n", Menus[i].window.name ? Menus[i].window.name : "<none>");
			return &Menus[i];
		}
	}

	return NULL;
}

void Menu_ScrollFeeder(menuDef_t *menu, int feeder, qboolean down) {
	if (menu) {
		int i;

		for (i = 0; i < menu->itemCount; i++) {
			if (menu->items[i]->special == feeder) {
				Item_ListBox_HandleKey(menu->items[i], (down) ? K_DOWNARROW : K_UPARROW, qtrue, qtrue);
				return;
			}
		}
	}
}

int Menu_GetFeederSelection(menuDef_t *menu, int feeder, const char *name) {
	if (menu == NULL) {
		if (name == NULL) {
			menu = Menu_GetFocused();
		} else {
			menu = Menus_FindByName(name);
		}
	}

	if (menu) {
		int i;

		for (i = 0; i < menu->itemCount; i++) {
			if (menu->items[i]->special == feeder && menu->items[i]->type == ITEM_TYPE_LISTBOX) {
				return menu->items[i]->cursorPos;
			}
		}
	}
	return -1;
}

void Menu_SetFeederSelection(menuDef_t *menu, int feeder, int index, const char *name) {
	if (menu == NULL) {
		if (name == NULL) {
			menu = Menu_GetFocused();
		} else {
			menu = Menus_FindByName(name);
		}
	}

	if (menu) {
		int i;

		for (i = 0; i < menu->itemCount; i++) {
			if (menu->items[i]->special == feeder && menu->items[i]->type == ITEM_TYPE_LISTBOX) {
				if (index == 0) {
					listBoxDef_t *listPtr = (listBoxDef_t*)menu->items[i]->typeData;
					listPtr->cursorPos = 0;
					listPtr->startPos = 0;
				}
				menu->items[i]->cursorPos = index;
				DC->feederSelection(menu->items[i]->special, menu->items[i]->cursorPos);
				return;
			}
		}
	}
}

void Menus_SetFeederSelection(int feeder, int index) {
	int i, j;

	for (i = 0; i < menuCount; i++) {
		for(j = 0; j < Menus[i].itemCount; j++ ) {
			if(Menus[i].items[j]->type == ITEM_TYPE_LISTBOX && Menus[i].items[j]->special == feeder) {
				Menus[i].items[j]->cursorPos = index;
				return;
			}
		}
	}
}

qboolean Menus_AnyFullScreenVisible() {
	int i;

	for (i = 0; i < menuCount; i++) {
		if (Menus[i].window.flags & WINDOW_VISIBLE && Menus[i].fullScreen) {
			return qtrue;
		}
	}

	return qfalse;
}

menuDef_t *Menus_ActivateByName(const char *p) {
	int i;
	menuDef_t *m = NULL;
	menuDef_t *focus = Menu_GetFocused();

	for (i = 0; i < menuCount; i++) {
		if (Q_stricmp(Menus[i].window.name, p) == 0) {
			m = &Menus[i];
			Menus_Activate(m);
			if (openMenuCount < MAX_OPEN_MENUS && focus != NULL) {
				menuStack[openMenuCount++] = focus;
			}
		} else {
			Menus[i].window.flags &= ~WINDOW_HASFOCUS;
		}
	}
	Display_CloseCinematics();

	return m;
}


void Item_Init(itemDef_t *item) {
	memset(item, 0, sizeof(itemDef_t));
	item->textscale = 0.55f;
	item->classLimit = CLASS_ALL;
	Window_Init(&item->window);
}

void Menu_HandleMouseMove(menuDef_t *menu, float x, float y) {
	int i, pass;
	qboolean focusSet = qfalse;

	itemDef_t *overItem;
	if (menu == NULL) {
		return;
	}

	if (!(menu->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED))) {
		return;
	}

	if (itemCapture) {
		//Item_MouseMove(itemCapture, x, y);
		return;
	}

	if (g_waitingForKey || g_editingField) {
		return;
	}

	// FIXME: this is the whole issue of focus vs. mouse over.. 
	// need a better overall solution as i don't like going through everything twice
	for (pass = 0; pass < 2; pass++) {
		for (i = 0; i < menu->itemCount; i++) {
			// turn off focus each item
			// menu->items[i].window.flags &= ~WINDOW_HASFOCUS;

			if (!(menu->items[i]->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED))) {
				continue;
			}

			// items can be enabled and disabled based on cvars
			if (menu->items[i]->cvarFlags & (CVAR_ENABLE | CVAR_DISABLE) && !Item_EnableShowViaCvar(menu->items[i], CVAR_ENABLE)) {
				continue;
			}

			if (menu->items[i]->cvarFlags & (CVAR_SHOW | CVAR_HIDE) && !Item_EnableShowViaCvar(menu->items[i], CVAR_SHOW)) {
				continue;
			}



			if (Rect_ContainsPoint(&menu->items[i]->window.rect, x, y)) {
				if (pass == 1) {
					overItem = menu->items[i];

					if (overItem->type == ITEM_TYPE_TEXT && overItem->text) {
						if (!Rect_ContainsPoint(Item_CorrectedTextRect(overItem), x, y)) {
							continue;
						}
					}

					// if we are over an item
					if (IsVisible(overItem->window.flags)) {
						// different one
						Item_MouseEnter(overItem, x, y);
						// Item_SetMouseOver(overItem, qtrue);

						// if item is not a decoration see if it can take focus
						if (!focusSet) {
							focusSet = Item_SetFocus(overItem, x, y);
						}
					}
				}
			} else if (menu->items[i]->window.flags & WINDOW_MOUSEOVER) {
				Item_MouseLeave(menu->items[i]);
				Item_SetMouseOver(menu->items[i], qfalse);
			}
		}
	}
}

void Menu_Paint(menuDef_t *menu, qboolean forcePaint) {
	int i;
	itemDef_t *item = NULL;

	if (!menu) {
		return;
	}

	if (!(menu->window.flags & WINDOW_VISIBLE) && !forcePaint) {
		return;
	}

	if (menu->window.ownerDrawFlags && !DC->ownerDrawVisible(menu->window.ownerDrawFlags)) {
		return;
	}
	
	// HUD items only

	// slothy - class test
	if((menu->classLimit != CLASS_ALL) && DC->playerClass) {
		if(!(DC->playerClass & menu->classLimit))
			return;
	}

#ifdef CGAME
	// slothy - weapon test
	if(menu->weaponLimit) {
		int curweapon = 0;

		if(!cg.snap)
			return;

		switch(cg.snap->ps.weapon) {
			case 1 : curweapon = WEAPON_AXE; break;
			case 2 : curweapon = WEAPON_SHOTGUN; break;
			case 3 : curweapon = WEAPON_SUPERSHOTGUN; break;
			case 4 : curweapon = WEAPON_NAILGUN; break;
			case 5 : curweapon = WEAPON_SUPERNAILGUN; break;
			case 6 : curweapon = WEAPON_GRENADE_LAUNCHER; break;
			case 7 : curweapon = WEAPON_ROCKET_LAUNCHER; break;
			case 8 : curweapon = WEAPON_SNIPER_RIFLE; break;
			case 9 : curweapon = WEAPON_RAILGUN; break;
			case 10 : curweapon = WEAPON_FLAMETHROWER; break;
			case 11 : curweapon = WEAPON_MINIGUN; break;
			case 12 : curweapon = WEAPON_ASSAULTRIFLE; break;
			case 13 : curweapon = WEAPON_DARTGUN; break;
			case 14 : curweapon = WEAPON_PIPELAUNCHER; break;
			case 15 : curweapon = WEAPON_NAPALMCANNON; break;
			default: curweapon = 0; break;
		}
		if(!(curweapon & menu->weaponLimit))
			return;
	}

	// special menu
	if(!Q_stricmp(menu->window.name, "menubox_vote"))
		MenuCheckVoteTally();
#endif

	if (forcePaint) {
		menu->window.flags |= WINDOW_FORCED;
	}

	// paint the background and or border
	Window_Paint(&menu->window, menu->fadeAmount, menu->fadeClamp, menu->fadeCycle );


	for (i = 0; i < menu->itemCount; i++) {
		Item_Paint(menu->items[i]);
		if(menu->items[i]->window.flags & WINDOW_MOUSEOVER) item = menu->items[i];
	}

	// OSP draw tooltip data if we have it
	if( DC->getCVarValue( "ui_showtooltips" ))
	{
		if(item != NULL)
		{
			if(item->toolTipData != NULL)
			{
				if(item->toolTipData->text != NULL)
				{
					if(ttInfo.item != item) {
						ttInfo.item = item;
						ttInfo.showtime = DC->realTime + 500;
					}
					else {
						if(DC->realTime > ttInfo.showtime) {
							Rectangle *r = &item->toolTipData->window.rect;
							r->x = DC->cursorx + 16;
							if((r->x + r->w) > 635)
								r->x = 635 - r->w;
							r->y = DC->cursory + 16;
							Item_Paint( item->toolTipData );
						}
					}
				}
			}
		}
	}

	/*
	if( DC->getCVarValue( "ui_showtooltips" ) &&
		item != NULL &&
		item->toolTipData != NULL &&
		item->toolTipData->text != NULL &&
		*item->toolTipData->text ) Item_Paint( item->toolTipData );
	*/

/*	if (debugMode) {
		vec4_t color;
		color[0] = color[2] = color[3] = 1;
		color[1] = 0;
		DC->drawRect(menu->window.rect.x, menu->window.rect.y, menu->window.rect.w, menu->window.rect.h, 1, color);
	}*/
}

/*
===============
Item_ValidateTypeData
===============
*/
void Item_ValidateTypeData(itemDef_t *item) {
	if (item->typeData) {
		return;
	}

	if (item->type == ITEM_TYPE_LISTBOX) {
		item->typeData = UI_Alloc(sizeof(listBoxDef_t));
		memset(item->typeData, 0, sizeof(listBoxDef_t));
	} else if (item->type == ITEM_TYPE_EDITFIELD || item->type == ITEM_TYPE_NUMERICFIELD || item->type == ITEM_TYPE_YESNO || item->type == ITEM_TYPE_SLIDER || item->type == ITEM_TYPE_TEXT) {
		item->typeData = UI_Alloc(sizeof(editFieldDef_t));
		memset(item->typeData, 0, sizeof(editFieldDef_t));
		if (item->type == ITEM_TYPE_EDITFIELD) {
			if (!((editFieldDef_t *) item->typeData)->maxPaintChars) {
				((editFieldDef_t *) item->typeData)->maxPaintChars = MAX_EDITFIELD;
			}
		}
		else if(item->type == ITEM_TYPE_SLIDER) {				// slothy - init slider scale
			((editFieldDef_t *) item->typeData)->scale = 1.0;
		}
	} else if (item->type == ITEM_TYPE_MULTI) {
		item->typeData = UI_Alloc(sizeof(multiDef_t));
	}
}

/*
Slothy
========================
Item_ValidateTooltipData
========================
*/
qboolean Item_ValidateTooltipData(itemDef_t *item)
{
	if(item->toolTipData != NULL) return(qtrue);

	item->toolTipData = UI_Alloc(sizeof(itemDef_t));
	if(item->toolTipData == NULL) return(qfalse);

	Item_Init(item->toolTipData);
	Tooltip_Initialize(item->toolTipData);

	return(qtrue);
}

/*
===============
Keyword Hash
===============
*/

#define KEYWORDHASH_SIZE	512

typedef struct keywordHash_s
{
	char *keyword;
	qboolean (*func)(itemDef_t *item, int handle);
	struct keywordHash_s *next;
} keywordHash_t;

int KeywordHash_Key(char *keyword) {
	int register hash, i;

	hash = 0;
	for (i = 0; keyword[i] != '\0'; i++) {
		if (keyword[i] >= 'A' && keyword[i] <= 'Z')
			hash += (keyword[i] + ('a' - 'A')) * (119 + i);
		else
			hash += keyword[i] * (119 + i);
	}
	hash = (hash ^ (hash >> 10) ^ (hash >> 20)) & (KEYWORDHASH_SIZE-1);
	return hash;
}

void KeywordHash_Add(keywordHash_t *table[], keywordHash_t *key) {
	int hash;

	hash = KeywordHash_Key(key->keyword);
/*
	if (table[hash]) {
		int collision = qtrue;
	}
*/
	key->next = table[hash];
	table[hash] = key;
}

keywordHash_t *KeywordHash_Find(keywordHash_t *table[], char *keyword)
{
	keywordHash_t *key;
	int hash;

	hash = KeywordHash_Key(keyword);
	for (key = table[hash]; key; key = key->next) {
		if (!Q_stricmp(key->keyword, keyword))
			return key;
	}
	return NULL;
}

/*
===============
Item Keyword Parse functions
===============
*/

// name <string>
qboolean ItemParse_name( itemDef_t *item, int handle ) {
	if (!PC_String_Parse(handle, &item->window.name)) {
		return qfalse;
	}
	return qtrue;
}

// name <string>
qboolean ItemParse_focusSound( itemDef_t *item, int handle ) {
	const char *temp;
	if (!PC_String_Parse(handle, &temp)) {
		return qfalse;
	}
	item->focusSound = DC->registerSound(temp, qfalse);
	return qtrue;
}


// text <string>
qboolean ItemParse_text( itemDef_t *item, int handle ) {
	if (!PC_String_Parse(handle, &item->text)) {
		return qfalse;
	}
	return qtrue;
}

// group <string>
qboolean ItemParse_group( itemDef_t *item, int handle ) {
	if (!PC_String_Parse(handle, &item->window.group)) {
		return qfalse;
	}
	return qtrue;
}

// asset_shader <string>
qboolean ItemParse_asset_shader( itemDef_t *item, int handle ) {
	const char *temp;

	if (!PC_String_Parse(handle, &temp)) {
		return qfalse;
	}
	item->asset = DC->registerShaderNoMip(temp);
	return qtrue;
}

// rect <rectangle>
qboolean ItemParse_rect( itemDef_t *item, int handle ) {
	if (!PC_Rect_Parse(handle, &item->window.rectClient)) {
		return qfalse;
	}
	return qtrue;
}

// style <integer>
qboolean ItemParse_style( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->window.style)) {
		return qfalse;
	}
	return qtrue;
}

// decoration
qboolean ItemParse_decoration( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_DECORATION;
	return qtrue;
}

// Slothy
// textasint
qboolean ItemParse_textasint( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_TEXTASINT;
	return qtrue;
}

// Slothy
// textasfloat
qboolean ItemParse_textasfloat( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_TEXTASFLOAT;
	return qtrue;
}


// notselectable
qboolean ItemParse_notselectable( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;
	Item_ValidateTypeData(item);
	listPtr = (listBoxDef_t*)item->typeData;
	if (item->type == ITEM_TYPE_LISTBOX && listPtr) {
		listPtr->notselectable = qtrue;
	}
	return qtrue;
}

// manually wrapped
qboolean ItemParse_wrapped( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_WRAPPED;
	return qtrue;
}

// auto wrapped
qboolean ItemParse_autowrapped( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_AUTOWRAPPED;
	return qtrue;
}


// horizontalscroll
/*qboolean ItemParse_horizontalscroll( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_HORIZONTAL;
	return qtrue;
}*/

// type <integer>
qboolean ItemParse_type( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->type)) {
		return qfalse;
	}
	Item_ValidateTypeData(item);
	return qtrue;
}

// elementwidth, used for listbox image elements
// uses textalignx for storage
qboolean ItemParse_elementwidth( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;

	Item_ValidateTypeData(item);
	listPtr = (listBoxDef_t*)item->typeData;
	if (!PC_Float_Parse(handle, &listPtr->elementWidth)) {
		return qfalse;
	}
	return qtrue;
}

// elementheight, used for listbox image elements
// uses textaligny for storage
qboolean ItemParse_elementheight( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;

	Item_ValidateTypeData(item);
	listPtr = (listBoxDef_t*)item->typeData;
	if (!PC_Float_Parse(handle, &listPtr->elementHeight)) {
		return qfalse;
	}
	return qtrue;
}

// feeder <float>
qboolean ItemParse_feeder( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->special)) {
		return qfalse;
	}
	return qtrue;
}

// elementtype, used to specify what type of elements a listbox contains
// uses textstyle for storage
qboolean ItemParse_elementtype( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;

	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;
	listPtr = (listBoxDef_t*)item->typeData;
	if (!PC_Int_Parse(handle, &listPtr->elementStyle)) {
		return qfalse;
	}
	return qtrue;
}

// columns sets a number of columns and an x pos and width per.. 
qboolean ItemParse_columns( itemDef_t *item, int handle ) {
	int num, i;
	listBoxDef_t *listPtr;

	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;
	listPtr = (listBoxDef_t*)item->typeData;
	if (PC_Int_Parse(handle, &num)) {
		if (num > MAX_LB_COLUMNS) {
			num = MAX_LB_COLUMNS;
		}
		listPtr->numColumns = num;
		for (i = 0; i < num; i++) {
			int pos, width, maxChars;

			if (PC_Int_Parse(handle, &pos) && PC_Int_Parse(handle, &width) && PC_Int_Parse(handle, &maxChars)) {
			listPtr->columnInfo[i].pos = pos;
				listPtr->columnInfo[i].width = width;
				listPtr->columnInfo[i].maxChars = maxChars;
			} else {
				return qfalse;
			}
		}
	} else {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_border( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->window.border)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_bordersize( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->window.borderSize)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_visible( itemDef_t *item, int handle ) {
	int i;

	if (!PC_Int_Parse(handle, &i)) {
		return qfalse;
	}
	if (i) {
		item->window.flags |= WINDOW_VISIBLE;
	}
	return qtrue;
}

qboolean ItemParse_ownerdraw( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->window.ownerDraw)) {
		return qfalse;
	}
	item->type = ITEM_TYPE_OWNERDRAW;
	return qtrue;
}

qboolean ItemParse_align( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->alignment)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_textalign( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->textalignment)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_textalignx( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->textalignx)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_textaligny( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->textaligny)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_textscale( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->textscale)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_textstyle( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->textStyle)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_backcolor( itemDef_t *item, int handle ) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		item->window.backColor[i]  = f;
	}
	return qtrue;
}

qboolean ItemParse_forecolor( itemDef_t *item, int handle ) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		item->window.foreColor[i]  = f;
		item->window.flags |= WINDOW_FORECOLORSET;
	}
	return qtrue;
}

qboolean ItemParse_bordercolor( itemDef_t *item, int handle ) {
	int i;
	float f;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		item->window.borderColor[i]  = f;
	}
	return qtrue;
}

qboolean ItemParse_outlinecolor( itemDef_t *item, int handle ) {
	if (!PC_Color_Parse(handle, &item->window.outlineColor)){
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_background( itemDef_t *item, int handle ) {
	const char *temp;

	if (!PC_String_Parse(handle, &temp)) {
		return qfalse;
	}
	item->window.primbackground = item->window.background = DC->registerShaderNoMip(temp);
	return qtrue;
}

qboolean ItemParse_altbackground( itemDef_t *item, int handle ) {
	const char *temp;

	if (!PC_String_Parse(handle, &temp)) {
		return qfalse;
	}
	item->window.altbackground = DC->registerShaderNoMip(temp);
	return qtrue;
}

qboolean ItemParse_cinematic( itemDef_t *item, int handle ) {
	if (!PC_String_Parse(handle, &item->window.cinematicName)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_doubleClick( itemDef_t *item, int handle ) {
	listBoxDef_t *listPtr;

	Item_ValidateTypeData(item);
	if (!item->typeData) {
		return qfalse;
	}

	listPtr = (listBoxDef_t*)item->typeData;

	if (!PC_Script_Parse(handle, &listPtr->doubleClick)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_onFocus( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->onFocus)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_leaveFocus( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->leaveFocus)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_mouseEnter( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->mouseEnter)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_mouseExit( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->mouseExit)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_mouseEnterText( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->mouseEnterText)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_mouseExitText( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->mouseExitText)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_action( itemDef_t *item, int handle ) {
	if (!PC_Script_Parse(handle, &item->action)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_special( itemDef_t *item, int handle ) {
	if (!PC_Float_Parse(handle, &item->special)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_cvarTest( itemDef_t *item, int handle ) {
	if (!PC_String_Parse(handle, &item->cvarTest)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_cvar( itemDef_t *item, int handle ) {
	editFieldDef_t *editPtr;

	Item_ValidateTypeData(item);
	if (!PC_String_Parse(handle, &item->cvar)) {
		return qfalse;
	}
	if (item->typeData) {
		editPtr = (editFieldDef_t*)item->typeData;
		editPtr->minVal = -1;
		editPtr->maxVal = -1;
		editPtr->defVal = -1;
	}
	return qtrue;
}

qboolean ItemParse_maxChars( itemDef_t *item, int handle ) {
	editFieldDef_t *editPtr;
	int maxChars;

	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;

	if (!PC_Int_Parse(handle, &maxChars)) {
		return qfalse;
	}
	editPtr = (editFieldDef_t*)item->typeData;
	editPtr->maxChars = maxChars;
	return qtrue;
}

qboolean ItemParse_maxPaintChars( itemDef_t *item, int handle ) {
	editFieldDef_t *editPtr;
	int maxChars;

	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;

	if (!PC_Int_Parse(handle, &maxChars)) {
		return qfalse;
	}
	editPtr = (editFieldDef_t*)item->typeData;
	editPtr->maxPaintChars = maxChars;
	return qtrue;
}



qboolean ItemParse_cvarFloat( itemDef_t *item, int handle ) {
	editFieldDef_t *editPtr;

	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;
	editPtr = (editFieldDef_t*)item->typeData;
	if (PC_String_Parse(handle, &item->cvar) &&
		PC_Float_Parse(handle, &editPtr->defVal) &&
		PC_Float_Parse(handle, &editPtr->minVal) &&
		PC_Float_Parse(handle, &editPtr->maxVal)) {
		return qtrue;
	}
	return qfalse;
}

qboolean ItemParse_cvarStrList( itemDef_t *item, int handle ) {
	pc_token_t token;
	multiDef_t *multiPtr;
	int pass;
	
	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;
	multiPtr = (multiDef_t*)item->typeData;
	multiPtr->count = 0;
	multiPtr->strDef = qtrue;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (*token.string != '{') {
		return qfalse;
	}

	pass = 0;
	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token)) {
			PC_SourceError(handle, "end of file inside menu item\n");
			return qfalse;
		}

		if (*token.string == '}') {
			return qtrue;
		}

		if (*token.string == ',' || *token.string == ';') {
			continue;
		}

		if (pass == 0) {
			multiPtr->cvarList[multiPtr->count] = String_Alloc(token.string);
			pass = 1;
		} else {
			multiPtr->cvarStr[multiPtr->count] = String_Alloc(token.string);
			pass = 0;
			multiPtr->count++;
			if (multiPtr->count >= MAX_MULTI_CVARS) {
				return qfalse;
			}
		}

	}

	return qfalse;
}

qboolean ItemParse_cvarFloatList( itemDef_t *item, int handle ) {
	pc_token_t token;
	multiDef_t *multiPtr;
	
	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;
	multiPtr = (multiDef_t*)item->typeData;
	multiPtr->count = 0;
	multiPtr->strDef = qfalse;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (*token.string != '{') {
		return qfalse;
	}

	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token)) {
			PC_SourceError(handle, "end of file inside menu item\n");
			return qfalse;
		}

		if (*token.string == '}') {
			return qtrue;
		}

		if (*token.string == ',' || *token.string == ';') {
			continue;
		}

		multiPtr->cvarList[multiPtr->count] = String_Alloc(token.string);
		if (!PC_Float_Parse(handle, &multiPtr->cvarValue[multiPtr->count])) {
			return qfalse;
		}

		multiPtr->count++;
		if (multiPtr->count >= MAX_MULTI_CVARS) {
			return qfalse;
		}

	}

	return qfalse;
}



qboolean ItemParse_addColorRange( itemDef_t *item, int handle ) {
	colorRangeDef_t color;

	if (PC_Float_Parse(handle, &color.low) &&
		PC_Float_Parse(handle, &color.high) &&
		PC_Color_Parse(handle, &color.color) ) {
		if (item->numColors < MAX_COLOR_RANGES) {
			memcpy(&item->colorRanges[item->numColors], &color, sizeof(color));
			item->numColors++;
		}
		return qtrue;
	}
	return qfalse;
}

qboolean ItemParse_ownerdrawFlag( itemDef_t *item, int handle ) {
	int i;
	if (!PC_Int_Parse(handle, &i)) {
		return qfalse;
	}
	item->window.ownerDrawFlags |= i;
	return qtrue;
}

qboolean ItemParse_enableCvar( itemDef_t *item, int handle ) {
	if (PC_Script_Parse(handle, &item->enableCvar)) {
		item->cvarFlags = CVAR_ENABLE;
		return qtrue;
	}
	return qfalse;
}

qboolean ItemParse_disableCvar( itemDef_t *item, int handle ) {
	if (PC_Script_Parse(handle, &item->enableCvar)) {
		item->cvarFlags = CVAR_DISABLE;
		return qtrue;
	}
	return qfalse;
}

qboolean ItemParse_showCvar( itemDef_t *item, int handle ) {
	if (PC_Script_Parse(handle, &item->enableCvar)) {
		item->cvarFlags = CVAR_SHOW;
		return qtrue;
	}
	return qfalse;
}

qboolean ItemParse_hideCvar( itemDef_t *item, int handle ) {
	if (PC_Script_Parse(handle, &item->enableCvar)) {
		item->cvarFlags = CVAR_HIDE;
		return qtrue;
	}
	return qfalse;
}

// RR2DO2
qboolean ItemParse_noPulseOnFocus( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_NOPULSEONFOCUS;
	return qtrue;
}

qboolean ItemParse_useAssetFont( itemDef_t *item, int handle ) {
	item->window.flags |= WINDOW_USEASSETFONT;
	return qtrue;
}
// RR2DO2


// djbob
qboolean ItemParse_anchorX( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->anchorx)) {
		return qfalse;
	}
	return qtrue;
}

qboolean ItemParse_anchorY( itemDef_t *item, int handle ) {
	if (!PC_Int_Parse(handle, &item->anchory)) {
		return qfalse;
	}
	return qtrue;
}
// djbob

// slothy
qboolean ItemParse_tooltip( itemDef_t *item, int handle )
{
	return(Item_ValidateTooltipData(item) && PC_String_Parse(handle, &item->toolTipData->text));
}

qboolean ItemParse_tooltipalignx( itemDef_t *item, int handle )
{
	return(Item_ValidateTooltipData(item) && PC_Float_Parse(handle, &item->toolTipData->textalignx));
}

qboolean ItemParse_tooltipaligny( itemDef_t *item, int handle )
{
	return(Item_ValidateTooltipData(item) && PC_Float_Parse(handle, &item->toolTipData->textaligny));
}

qboolean ItemParse_scale( itemDef_t *item, int handle )
{
	editFieldDef_t *editDef;

	Item_ValidateTypeData(item);
	if (!item->typeData)
		return qfalse;
	editDef = item->typeData;

	return PC_Float_Parse(handle, &editDef->scale);
}

qboolean ItemParse_class( itemDef_t *item, int handle )
{
	pc_token_t token;

	item->classLimit = 0;
	
	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (*token.string != '{') {
		return qfalse;
	}

	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token)) {
			PC_SourceError(handle, "end of file inside menu item\n");
			return qfalse;
		}

		if (*token.string == '}') {
			return qtrue;
		}

		if (*token.string == ',' || *token.string == ';' || *token.string == ' ') {
			continue;
		}

		if(strcmp(token.string, "recon") == 0)
			item->classLimit |= CLASS_RECON;
		else if(strcmp(token.string, "sniper") == 0)
			item->classLimit |= CLASS_SNIPER;
		else if(strcmp(token.string, "soldier") == 0)
			item->classLimit |= CLASS_SOLDIER;
		else if(strcmp(token.string, "grenadier") == 0)
			item->classLimit |= CLASS_GRENADIER;
		else if(strcmp(token.string, "paramedic") == 0)
			item->classLimit |= CLASS_PARAMEDIC;
		else if(strcmp(token.string, "minigunner") == 0)
			item->classLimit |= CLASS_MINIGUNNER;
		else if(strcmp(token.string, "flametrooper") == 0)
			item->classLimit |= CLASS_FLAMETROOPER;
		else if(strcmp(token.string, "agent") == 0)
			item->classLimit |= CLASS_AGENT;
		else if(strcmp(token.string, "engineer") == 0)
			item->classLimit |= CLASS_ENGINEER;
		else if(strcmp(token.string, "civilian") == 0)
			item->classLimit |= CLASS_CIVILIAN;
		else if(strcmp(token.string, "spectator") == 0)
			item->classLimit |= CLASS_SPECTATOR;
		else
		{
			PC_SourceError(handle, "unknown class identifier\n");
			return qfalse;
		}
	}

	return qfalse;
}


qboolean ItemParse_weapon( itemDef_t *item, int handle )
{
	pc_token_t token;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (*token.string != '{') {
		return qfalse;
	}

	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token)) {
			PC_SourceError(handle, "end of file inside menu item\n");
			return qfalse;
		}

		if (*token.string == '}') {
			return qtrue;
		}

		if (*token.string == ',' || *token.string == ';' || *token.string == ' ') {
			continue;
		}

		if(strcmp(token.string, "axe") == 0)
			item->weaponLimit |= WEAPON_AXE;
		else if(strcmp(token.string, "shotgun") == 0)
			item->weaponLimit |= WEAPON_SHOTGUN;
		else if(strcmp(token.string, "supershotgun") == 0)
			item->weaponLimit |= WEAPON_SUPERSHOTGUN;
		else if(strcmp(token.string, "nailgun") == 0)
			item->weaponLimit |= WEAPON_NAILGUN;
		else if(strcmp(token.string, "supernailgun") == 0)
			item->weaponLimit |= WEAPON_SUPERNAILGUN;
		else if(strcmp(token.string, "grenadelauncher") == 0)
			item->weaponLimit |= WEAPON_GRENADE_LAUNCHER;
		else if(strcmp(token.string, "rocketlauncher") == 0)
			item->weaponLimit |= WEAPON_ROCKET_LAUNCHER;
		else if(strcmp(token.string, "sniperrifle") == 0)
			item->weaponLimit |= WEAPON_SNIPER_RIFLE;
		else if(strcmp(token.string, "railgun") == 0)
			item->weaponLimit |= WEAPON_RAILGUN;
		else if(strcmp(token.string, "flamethrower") == 0)
			item->weaponLimit |= WEAPON_FLAMETHROWER;
		else if(strcmp(token.string, "minigun") == 0)
			item->weaponLimit |= WEAPON_MINIGUN;
		else if(strcmp(token.string, "assaultrifle") == 0)
			item->weaponLimit |= WEAPON_ASSAULTRIFLE;
		else if(strcmp(token.string, "dartgun") == 0)
			item->weaponLimit |= WEAPON_DARTGUN;
		else if(strcmp(token.string, "pipelauncher") == 0)
			item->weaponLimit |= WEAPON_PIPELAUNCHER;
		else if(strcmp(token.string, "napalmcannon") == 0)
			item->weaponLimit |= WEAPON_NAPALMCANNON;
		else
		{
			PC_SourceError(handle, va("unknown weapon identifier: %s\n", token.string));
			return qfalse;
		}
	}

	return qfalse;
}


// slothy


keywordHash_t itemParseKeywords[] = {
	{"name",				ItemParse_name,				NULL},
	{"text",				ItemParse_text,				NULL},
	{"group",				ItemParse_group,			NULL},
	{"asset_shader",		ItemParse_asset_shader,		NULL},
	{"rect",				ItemParse_rect,				NULL},
	{"style",				ItemParse_style,			NULL},
	{"decoration",			ItemParse_decoration,		NULL},
	{"notselectable",		ItemParse_notselectable,	NULL},
	{"wrapped",				ItemParse_wrapped,			NULL},
	{"autowrapped",			ItemParse_autowrapped,		NULL},
	{"type",				ItemParse_type,				NULL},
	{"elementwidth",		ItemParse_elementwidth,		NULL},
	{"elementheight",		ItemParse_elementheight,	NULL},
	{"feeder",				ItemParse_feeder,			NULL},
	{"elementtype",			ItemParse_elementtype,		NULL},
	{"columns",				ItemParse_columns,			NULL},
	{"border",				ItemParse_border,			NULL},
	{"bordersize",			ItemParse_bordersize,		NULL},
	{"visible",				ItemParse_visible,			NULL},
	{"ownerdraw",			ItemParse_ownerdraw,		NULL},
	{"align",				ItemParse_align,			NULL},
	{"textalign",			ItemParse_textalign,		NULL},
	{"textalignx",			ItemParse_textalignx,		NULL},
	{"textaligny",			ItemParse_textaligny,		NULL},
	{"textscale",			ItemParse_textscale,		NULL},
	{"textstyle",			ItemParse_textstyle,		NULL},
	{"backcolor",			ItemParse_backcolor,		NULL},
	{"forecolor",			ItemParse_forecolor,		NULL},
	{"bordercolor",			ItemParse_bordercolor,		NULL},
	{"outlinecolor",		ItemParse_outlinecolor,		NULL},
	{"background",			ItemParse_background,		NULL},
	{"altbackground",		ItemParse_altbackground,	NULL},
	{"onFocus",				ItemParse_onFocus,			NULL},
	{"leaveFocus",			ItemParse_leaveFocus,		NULL},
	{"mouseEnter",			ItemParse_mouseEnter,		NULL},
	{"mouseExit",			ItemParse_mouseExit,		NULL},
	{"mouseEnterText",		ItemParse_mouseEnterText,	NULL},
	{"mouseExitText",		ItemParse_mouseExitText,	NULL},
	{"action",				ItemParse_action,			NULL},
	{"special",				ItemParse_special,			NULL},
	{"cvar",				ItemParse_cvar,				NULL},
	{"maxChars",			ItemParse_maxChars,			NULL},
	{"maxPaintChars",		ItemParse_maxPaintChars,	NULL},
	{"focusSound",			ItemParse_focusSound,		NULL},
	{"cvarFloat",			ItemParse_cvarFloat,		NULL},
	{"cvarStrList",			ItemParse_cvarStrList,		NULL},
	{"cvarFloatList",		ItemParse_cvarFloatList,	NULL},
	{"addColorRange",		ItemParse_addColorRange,	NULL},
	{"ownerdrawFlag",		ItemParse_ownerdrawFlag,	NULL},
	{"enableCvar",			ItemParse_enableCvar,		NULL},
	{"cvarTest",			ItemParse_cvarTest,			NULL},
	{"disableCvar",			ItemParse_disableCvar,		NULL},
	{"showCvar",			ItemParse_showCvar,			NULL},
	{"hideCvar",			ItemParse_hideCvar,			NULL},
	{"cinematic",			ItemParse_cinematic,		NULL},
	{"doubleclick",			ItemParse_doubleClick,		NULL},
// RR2DO2
	{"noPulseOnFocus",		ItemParse_noPulseOnFocus,	NULL},
	{"useAssetFont",		ItemParse_useAssetFont,		NULL},
// RR2DO2
// djbob
	{"anchorx",				ItemParse_anchorX,			NULL},
	{"anchory",				ItemParse_anchorY,			NULL},
// Slothy
	{ "textasint",			ItemParse_textasint,		NULL },
	{ "textasfloat",		ItemParse_textasfloat,		NULL },
	{ "tooltip",			ItemParse_tooltip,			NULL },
	{ "tooltipalignx",		ItemParse_tooltipalignx,	NULL },
	{ "tooltipaligny",		ItemParse_tooltipaligny,	NULL },
	{ "scale",				ItemParse_scale,			NULL }, 
	{ "class",				ItemParse_class,			NULL },
	{ "weapon",				ItemParse_weapon,			NULL },
// djbob
	{NULL, NULL, NULL}
};

keywordHash_t *itemParseKeywordHash[KEYWORDHASH_SIZE];

/*
===============
Item_SetupKeywordHash
===============
*/
void Item_SetupKeywordHash(void) {
	int i;

	memset(itemParseKeywordHash, 0, sizeof(itemParseKeywordHash));
	for (i = 0; itemParseKeywords[i].keyword; i++) {
		KeywordHash_Add(itemParseKeywordHash, &itemParseKeywords[i]);
	}
}

/*
===============
Item_Parse
===============
*/
qboolean Item_Parse(int handle, itemDef_t *item) {
	pc_token_t token;
	keywordHash_t *key;


	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (*token.string != '{') {
		return qfalse;
	}
	while ( 1 ) {
		if (!trap_PC_ReadToken(handle, &token)) {
			PC_SourceError(handle, "end of file inside menu item\n");
			return qfalse;
		}

		if (*token.string == '}') {
			return qtrue;
		}

		key = KeywordHash_Find(itemParseKeywordHash, token.string);
		if (!key) {
			PC_SourceError(handle, "unknown menu item keyword %s", token.string);
			continue;
		}
		if ( !key->func(item, handle) ) {
			PC_SourceError(handle, "couldn't parse menu item keyword %s", token.string);
			return qfalse;
		}
	}
	return qfalse;
}


// Item_InitControls
// init's special control types
void Item_InitControls(itemDef_t *item) {
	if (item == NULL) {
		return;
	}
	if (item->type == ITEM_TYPE_LISTBOX) {
		listBoxDef_t *listPtr = (listBoxDef_t*)item->typeData;
		item->cursorPos = 0;
		if (listPtr) {
			listPtr->cursorPos = 0;
			listPtr->startPos = 0;
			listPtr->endPos = 0;
			listPtr->cursorPos = 0;
		}
	}
}

void Item_AutoAlign(itemDef_t* item) {
	menuDef_t *parent;
	fontStruct_t *parentfont;
	int oldx;

	if(item->text && *item->text) {
		parent = (menuDef_t*)item->parent;
		parentfont = ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font;

		oldx = item->textalignx;
		item->textalignx = item->window.rectClient.w - DC->textWidth(item->text, item->textscale, 0, parentfont);
		item->window.rectClient.w += oldx;
	}
}

/*
===============
Menu Keyword Parse functions
===============
*/

/*qboolean MenuParse_font( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	//if (!PC_String_Parse(handle, &menu->font)) {
		return qfalse;
	}
	if (!DC->Assets.fontRegistered) {
		DC->registerFont(menu->font, 48, &DC->Assets.textFont);
		DC->Assets.fontRegistered = qtrue;
	}
	return qtrue;
}*/
qboolean MenuParse_font( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	char buff[128];
	int pointSize;

	if (!PC_String_ParseNoAlloc(handle, buff, sizeof(buff)) || !PC_Int_Parse(handle,&pointSize)) {
		return qfalse;
	}

	DC->registerFont(buff, pointSize, &menu->font.textFont);
	menu->font.fontRegistered = qtrue;
	return qtrue;
}

qboolean MenuParse_smallfont( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	char buff[128];
	int pointSize;

	if (!PC_String_ParseNoAlloc(handle, buff, sizeof(buff)) || !PC_Int_Parse(handle,&pointSize)) {
		return qfalse;
	}

	DC->registerFont(buff, pointSize, &menu->font.smallFont);
	menu->font.fontRegistered = qtrue;
	return qtrue;
}

qboolean MenuParse_bigfont( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	char buff[128];
	int pointSize;

	if (!PC_String_ParseNoAlloc(handle, buff, sizeof(buff)) || !PC_Int_Parse(handle,&pointSize)) {
		return qfalse;
	}

	DC->registerFont(buff, pointSize, &menu->font.bigFont);
	menu->font.fontRegistered = qtrue;
	return qtrue;
}

qboolean MenuParse_name( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_String_Parse(handle, &menu->window.name)) {
		return qfalse;
	}
	if (Q_stricmp(menu->window.name, "main") == 0) {
		// default main as having focus
		//menu->window.flags |= WINDOW_HASFOCUS;
	}
	return qtrue;
}

qboolean MenuParse_fullscreen( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Int_Parse(handle, (int*)&menu->fullScreen)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_rect( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Rect_Parse(handle, &menu->window.rect)) {
		return qfalse;
	}

	menu->realRect.x = menu->window.rect.x;
	menu->realRect.y = menu->window.rect.y;
	menu->realRect.w = menu->window.rect.w;
	menu->realRect.h = menu->window.rect.h;

	return qtrue;
}

qboolean MenuParse_style( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Int_Parse(handle, &menu->window.style)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_visible( itemDef_t *item, int handle ) {
	int i;
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &i)) {
		return qfalse;
	}
	if (i) {
		menu->window.flags |= WINDOW_VISIBLE;
	}
	return qtrue;
}

qboolean MenuParse_onOpen( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Script_Parse(handle, &menu->onOpen)) {
		return qfalse;
	}

	// RR2DO2: warning, no "open" in "onOpen"
	if( strstr( menu->onOpen, "open" ) ) {
		// we have an "open" string, now see if it's a token
		// this is only a valid token if it's one of the following cases:
		// a) it's the first token
		// b) it's the first token after an ;
		char *token;
		char *buff = (char *)menu->onOpen;
		qboolean lasttokenwassemicolon = qtrue;

		while( 1 ) {
			token = COM_Parse( &buff );
			if ( !token[0] ) {
				break;
			}

			if ( !strcmp( token, "open" ) && lasttokenwassemicolon ) {
				DC->Print(S_COLOR_YELLOW"WARNING: Menu \'%s\' has \"open\" in \"onOpen\"!\n", menu->window.name);
				break;
			} else if ( !strcmp( token, ";" ) ) {
				lasttokenwassemicolon = qtrue;
			} else {
				lasttokenwassemicolon = qfalse;
			}
		}
	}
	
	return qtrue;
}

qboolean MenuParse_onClose( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Script_Parse(handle, &menu->onClose)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_onESC( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Script_Parse(handle, &menu->onESC)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_onKey( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if(menu->numKeyScripts == MAX_KEY_SCRIPTS) {
		return qfalse;
	}

	if (!PC_Int_Parse(handle, &menu->keyScripts[menu->numKeyScripts].key)) {
		return qfalse;
	}

	if (!PC_Script_Parse(handle, &menu->keyScripts[menu->numKeyScripts].script)) {
		return qfalse;
	}

	menu->numKeyScripts++;

	return qtrue;
}


qboolean MenuParse_border( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Int_Parse(handle, &menu->window.border)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_borderSize( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Float_Parse(handle, &menu->window.borderSize)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_backcolor( itemDef_t *item, int handle ) {
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		menu->window.backColor[i]  = f;
	}
	return qtrue;
}

qboolean MenuParse_forecolor( itemDef_t *item, int handle ) {
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		menu->window.foreColor[i]  = f;
		menu->window.flags |= WINDOW_FORECOLORSET;
	}
	return qtrue;
}

qboolean MenuParse_bordercolor( itemDef_t *item, int handle ) {
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		menu->window.borderColor[i]  = f;
	}
	return qtrue;
}

qboolean MenuParse_focuscolor( itemDef_t *item, int handle ) {
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;

	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		menu->focusColor[i]  = f;
	}
	return qtrue;
}

qboolean MenuParse_disablecolor( itemDef_t *item, int handle ) {
	int i;
	float f;
	menuDef_t *menu = (menuDef_t*)item;
	for (i = 0; i < 4; i++) {
		if (!PC_Float_Parse(handle, &f)) {
			return qfalse;
		}
		menu->disableColor[i]  = f;
	}
	return qtrue;
}


qboolean MenuParse_outlinecolor( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Color_Parse(handle, &menu->window.outlineColor)){
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_background( itemDef_t *item, int handle ) {
	const char *buff;
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_String_Parse(handle, &buff)) {
		return qfalse;
	}
	menu->window.background = DC->registerShaderNoMip(buff);
	return qtrue;
}

qboolean MenuParse_cinematic( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_String_Parse(handle, &menu->window.cinematicName)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_ownerdrawFlag( itemDef_t *item, int handle ) {
	int i;
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &i)) {
		return qfalse;
	}
	menu->window.ownerDrawFlags |= i;
	return qtrue;
}

qboolean MenuParse_ownerdraw( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &menu->window.ownerDraw)) {
		return qfalse;
	}
	return qtrue;
}


// decoration
qboolean MenuParse_popup( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	menu->window.flags |= WINDOW_POPUP;
	return qtrue;
}


qboolean MenuParse_outOfBounds( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	menu->window.flags |= WINDOW_OOB_CLICK;
	return qtrue;
}

qboolean MenuParse_soundLoop( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_String_Parse(handle, &menu->soundName)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_fadeClamp( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Float_Parse(handle, &menu->fadeClamp)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_fadeAmount( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Float_Parse(handle, &menu->fadeAmount)) {
		return qfalse;
	}
	return qtrue;
}


qboolean MenuParse_fadeCycle( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	if (!PC_Int_Parse(handle, &menu->fadeCycle)) {
		return qfalse;
	}
	return qtrue;
}

qboolean MenuParse_itemDef( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (menu->itemCount < MAX_MENUITEMS) {
		menu->items[menu->itemCount] = UI_Alloc(sizeof(itemDef_t));
		Item_Init(menu->items[menu->itemCount]);
		if (!Item_Parse(handle, menu->items[menu->itemCount])) {
			return qfalse;
		}
		Item_InitControls(menu->items[menu->itemCount]);
		menu->items[menu->itemCount]->parent = menu;
		if(menu->items[menu->itemCount]->textalignment == ITEM_ALIGN_AUTO)
			Item_AutoAlign(menu->items[menu->itemCount]);
		menu->itemCount++;
	}
	return qtrue;
}

// RR2DO2
qboolean MenuParse_drawAlwaysOnTop( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	menu->window.flags |= WINDOW_DRAWALWAYSONTOP;
	return qtrue;
}

qboolean MenuParse_keepOpenOnFocusLost( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	menu->window.flags |= WINDOW_KEEPOPENONFOCUSLOST;
	return qtrue;
}

qboolean MenuParse_onOOBClick( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	if (!PC_Script_Parse(handle, &menu->onOOBClick)) {
		return qfalse;
	}

	menu->window.flags |= WINDOW_OOB_CLICK;

	return qtrue;
}
// RR2DO2

// djbob
qboolean MenuParse_LoadBorderBitmap(itemDef_t *item, int handle) {
	menuDef_t *menu = (menuDef_t*)item;
	const char *tex;
	int number;

	if (!PC_Int_Parse(handle, &number)) {
		return qfalse;
	}

	if (!PC_String_Parse(handle, &tex)) {
		return qfalse;
	}

	menu->borderBitmaps[number] = DC->registerShaderNoMip(tex);
	return qtrue;
}

qboolean MenuParse_nonSticky( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;

	menu->nonSticky = qtrue;

	return qtrue;
}
// djbob

// slothy
qboolean MenuParse_class( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	itemDef_t newitem;
	qboolean res;

	res = ItemParse_class(&newitem, handle);
	menu->classLimit = newitem.classLimit;
	return res;
}

qboolean MenuParse_weapon( itemDef_t *item, int handle ) {
	menuDef_t *menu = (menuDef_t*)item;
	itemDef_t newitem;
	qboolean res;

	newitem.weaponLimit = 0;
	res = ItemParse_weapon(&newitem, handle);
	menu->weaponLimit = newitem.weaponLimit;
	item->weaponLimit = newitem.weaponLimit;
	return res;
}// slothy

keywordHash_t menuParseKeywords[] = {
	{"font", MenuParse_font, NULL},
	{"smallFont", MenuParse_smallfont, NULL},
	{"bigFont", MenuParse_bigfont, NULL},
	{"name", MenuParse_name, NULL},
	{"fullscreen", MenuParse_fullscreen, NULL},
	{"rect", MenuParse_rect, NULL},
	{"style", MenuParse_style, NULL},
	{"visible", MenuParse_visible, NULL},
	{"onOpen", MenuParse_onOpen, NULL},
	{"onClose", MenuParse_onClose, NULL},
	{"onESC", MenuParse_onESC, NULL},
	{"border", MenuParse_border, NULL},
	{"borderSize", MenuParse_borderSize, NULL},
	{"backcolor", MenuParse_backcolor, NULL},
	{"forecolor", MenuParse_forecolor, NULL},
	{"bordercolor", MenuParse_bordercolor, NULL},
	{"focuscolor", MenuParse_focuscolor, NULL},
	{"disablecolor", MenuParse_disablecolor, NULL},
	{"outlinecolor", MenuParse_outlinecolor, NULL},
	{"background", MenuParse_background, NULL},
	{"ownerdraw", MenuParse_ownerdraw, NULL},
	{"ownerdrawFlag", MenuParse_ownerdrawFlag, NULL},
	{"outOfBoundsClick", MenuParse_outOfBounds, NULL},
	{"soundLoop", MenuParse_soundLoop, NULL},
	{"itemDef", MenuParse_itemDef, NULL},
	{"cinematic", MenuParse_cinematic, NULL},
	{"popup", MenuParse_popup, NULL},
	{"fadeClamp", MenuParse_fadeClamp, NULL},
	{"fadeCycle", MenuParse_fadeCycle, NULL},
	{"fadeAmount", MenuParse_fadeAmount, NULL},
// RR2DO2
	{"drawAlwaysOnTop", MenuParse_drawAlwaysOnTop, NULL},
	{"keepOpenOnFocusLost", MenuParse_keepOpenOnFocusLost, NULL},
	{"onOOBClick", MenuParse_onOOBClick, NULL},
// RR2DO2
// djbob
	{"loadborderbitmap", &MenuParse_LoadBorderBitmap, NULL},	// border bitmaps
	{"nonSticky", MenuParse_nonSticky, NULL},
	{"onKey", MenuParse_onKey, NULL},
// djbob
//slothy
	{ "class",				MenuParse_class,			NULL },
	{ "weapon",				MenuParse_weapon,			NULL },
//slothy
	{NULL, NULL, NULL}
};

keywordHash_t *menuParseKeywordHash[KEYWORDHASH_SIZE];

/*
===============
Menu_SetupKeywordHash
===============
*/
void Menu_SetupKeywordHash(void) {
	int i;

	memset(menuParseKeywordHash, 0, sizeof(menuParseKeywordHash));
	for (i = 0; menuParseKeywords[i].keyword; i++) {
		KeywordHash_Add(menuParseKeywordHash, &menuParseKeywords[i]);
	}
}

/*
===============
Menu_Parse
===============
*/
qboolean Menu_Parse(int handle, menuDef_t *menu) {
	pc_token_t token;
	keywordHash_t *key;

	if (!trap_PC_ReadToken(handle, &token))
		return qfalse;
	if (*token.string != '{') {
		return qfalse;
	}
    
	while ( 1 ) {

		memset(&token, 0, sizeof(pc_token_t));
		if (!trap_PC_ReadToken(handle, &token)) {
			PC_SourceError(handle, "end of file inside menu\n");
			return qfalse;
		}

		if (*token.string == '}') {
			return qtrue;
		}

		key = KeywordHash_Find(menuParseKeywordHash, token.string);
		if (!key) {
			PC_SourceError(handle, "unknown menu keyword %s", token.string);
			continue;
		}
		if ( !key->func((itemDef_t*)menu, handle) ) {
			PC_SourceError(handle, "couldn't parse menu keyword %s", token.string);
			return qfalse;
		}
	}
	return qfalse;
}

/*
===============
Menu_New
===============
*/
void Menu_New(int handle) {
	menuDef_t *menu = &Menus[menuCount];

	if (menuCount < MAX_MENUS) {
		Menu_Init(menu);
		if (Menu_Parse(handle, menu)) {
			Menu_PostParse(menu);
			menuCount++;
		}
	}
}

int trap_Milliseconds( void );

#ifdef CGAME
//#define DEBUGTIME_ENABLED_2
#endif
#ifdef DEBUGTIME_ENABLED_2
#define DEBUGTIME2 Com_Printf("%i, ", elapsed = (trap_Milliseconds()-dbgTime) ); dbgCnt++; dbgTime+=elapsed;
#else
#define DEBUGTIME2
#endif

void Menu_PaintAll() {
	int i;
#ifdef DEBUGTIME_ENABLED_2
	int dbgTime=trap_Milliseconds(),elapsed;
	int dbgCnt=0;
#endif

#ifdef DEBUGTIME_ENABLED_2
	Com_Printf("\n");
#endif

	if (captureFunc) {
		captureFunc(captureData);
	}

	DEBUGTIME2

	for (i = 0; i < menuCount; i++) {
		// RR2DO2: draw alwaysontop menus the latest
		if( Menus[i].window.flags & WINDOW_DRAWALWAYSONTOP )
			continue;
		// RR2DO2
		Menu_Paint(&Menus[i], qfalse);
		DEBUGTIME2
	}

	// RR2DO2: now draw the windows that are on top
	for (i = 0; i < menuCount; i++) {
		if( Menus[i].window.flags & WINDOW_DRAWALWAYSONTOP ) {
			Menu_Paint(&Menus[i], qfalse);
			DEBUGTIME2
		}
	}
	// RR2DO2

	if (debugMode) {
		char* fps = va("fps: %i", (int)DC->FPS);
		vec4_t v = {1, 1, 1, 1};
		DC->drawText(640, 35, .38f, v, fps, 0, 0, 0, NULL, ITEM_ALIGN_RIGHT);
//		Com_Printf(va("%s\n", fps));
	}
}

void Menu_Reset() {
	menuCount = 0;
}

// RR2DO2
menuDef_t *Menu_Get( int menu_num ) {
	return &Menus[menu_num];
}
// RR2DO2

displayContextDef_t *Display_GetContext() {
	return DC;
}
 
//static float captureX;
//static float captureY;

void *Display_CaptureItem(int x, int y) {
	int i;

	for (i = 0; i < menuCount; i++) {
		// turn off focus each item
		// menu->items[i].window.flags &= ~WINDOW_HASFOCUS;
		if (Rect_ContainsPoint(&Menus[i].window.rect, x, y)) {
			return &Menus[i];
		}
	}
	return NULL;
}

qboolean Display_MouseMove(void *p, int x, int y) {
	int i;
	menuDef_t *menu = p;

	uimousex = x;
	uimousey = y;

	if (menu == NULL) {
		menu = Menu_GetFocused();

		if (menu) {
			if (menu->window.flags & WINDOW_POPUP) {
				Menu_HandleMouseMove(menu, x, y);
				return qtrue;
			}
		}
		for (i = 0; i < menuCount; i++) {
			Menu_HandleMouseMove(&Menus[i], x, y);
		}
	} else {
		int xlocked = 0;
		int ylocked = 0;

		menu->realRect.x += x;
		menu->realRect.y += y;

		if(!DC->keyIsDown(K_SHIFT)) {

			for( i = 0; i < menuCount; i++ ) {
				float diff = 0;
				qboolean adjust = qfalse;
	
				menuDef_t* m = &Menus[i];
				if(menu == m || !(m->window.style | WINDOW_VISIBLE) || m->nonSticky) {
					continue;
				}
	
				if(m->window.ownerDrawFlags && !DC->ownerDrawVisible(m->window.ownerDrawFlags)) {
					continue;
				}
	
				adjust = qfalse;
				if(	!xlocked ) {
					if(	(menu->window.rect.y + menu->window.rect.h >= m->window.rect.y) &&
						(menu->window.rect.y <= m->window.rect.y + m->window.rect.h)) {
	
						diff = menu->realRect.x - (m->window.rect.x + m->window.rect.w);
						if(fabs(diff) < 10) {
							adjust = qtrue;
						} else {
							diff = (menu->realRect.x + menu->realRect.w) - m->window.rect.x;
							if(fabs(diff) < 10) {
								adjust = qtrue;
							} else {
								diff = 0;
							}
						}
					}
					if(adjust) {
						xlocked = 1;
					}
					menu->window.rect.x = menu->realRect.x - diff;
				}
	
				diff = 0;
	
				adjust = qfalse;
				if(	!ylocked ) {
					if(	(menu->window.rect.x + menu->window.rect.w >= m->window.rect.x) &&
						(menu->window.rect.x <= m->window.rect.x + m->window.rect.w)) {
	
						diff = menu->realRect.y - (m->window.rect.y + m->window.rect.h);
						if(fabs(diff) < 10) {
							adjust = qtrue;
						} else {
							diff = (menu->realRect.y + menu->realRect.h) - m->window.rect.y;
							if(fabs(diff) < 10) {
								adjust = qtrue;
							} else {
								diff = 0;
							}
						}
					}
					if(adjust) {
						ylocked = 1;
					}
					menu->window.rect.y = menu->realRect.y - diff;
				}
	
				diff = 0;

				if(!xlocked && ylocked == 1) {
					if(	((menu->window.rect.y + menu->window.rect.h) == m->window.rect.y) ||
						(menu->window.rect.y == (m->window.rect.y + m->window.rect.h))) {
						diff = menu->realRect.x - m->window.rect.x;
						if(fabs(diff) < 10) {
							adjust = qtrue;
						} else {
							diff = (menu->realRect.x + menu->realRect.w) - (m->window.rect.x + m->window.rect.w);
							if(fabs(diff) < 10) {
								adjust = qtrue;
							} else {
								diff = 0;
							}
						}
					}
					if(adjust) {
						xlocked = 1;
					}
					menu->window.rect.x = menu->realRect.x - diff;
				}

				if(!ylocked && xlocked == 1) {
					if(	((menu->window.rect.x + menu->window.rect.w) == m->window.rect.x) ||
						(menu->window.rect.x == (m->window.rect.x + m->window.rect.w))) {
						diff = menu->realRect.y - m->window.rect.y;
						if(fabs(diff) < 10) {
							adjust = qtrue;
						} else {
							diff = (menu->realRect.y + menu->realRect.h) - (m->window.rect.y + m->window.rect.h);
							if(fabs(diff) < 10) {
								adjust = qtrue;
							} else {
								diff = 0;
							}
						}
					}
					if(adjust) {
						ylocked = 1;
					}
					menu->window.rect.y = menu->realRect.y - diff;
				}

				if(ylocked == 1) {
					ylocked = 2;
				}

				if(xlocked == 1) {
					xlocked = 2;
				}
			}
		} else {
			menu->window.rect.x = menu->realRect.x;
			menu->window.rect.y = menu->realRect.y;
		}

		if(menu->window.rect.x > 639) {
			menu->window.rect.x = 639;
		}
		if(menu->window.rect.y > 479) {
			menu->window.rect.y = 479;
		}

		Menu_UpdatePosition(menu);
	}
 	return qtrue;
}

int Display_CursorType(int x, int y) {
	int i;
	for (i = 0; i < menuCount; i++) {
		rectDef_t r2;
		r2.x = Menus[i].window.rect.x - 3;
		r2.y = Menus[i].window.rect.y - 3;
		r2.w = r2.h = 7;
		if (Rect_ContainsPoint(&r2, x, y)) {
			return CURSOR_SIZER;
		}
	}
	return CURSOR_ARROW;
}


void Display_HandleKey(int key, qboolean down, int x, int y) {
	menuDef_t *menu = Display_CaptureItem(x, y);
	if (menu == NULL) {  
		menu = Menu_GetFocused();
	}
	if (menu) {
		Menu_HandleKey(menu, key, down );
	}
}

static void Window_CacheContents(windowDef_t *window) {
	if (window) {
		if (window->cinematicName) {
			int cin = DC->playCinematic(window->cinematicName, 0, 0, 0, 0);
			DC->stopCinematic(cin);
		}
	}
}


static void Item_CacheContents(itemDef_t *item) {
	if (item) {
		Window_CacheContents(&item->window);
	}

}

static void Menu_CacheContents(menuDef_t *menu) {
	if (menu) {
		int i;
		Window_CacheContents(&menu->window);
		for (i = 0; i < menu->itemCount; i++) {
			Item_CacheContents(menu->items[i]);
		}

		if (menu->soundName && *menu->soundName) {
			DC->registerSound(menu->soundName, qfalse);
		}
	}

}

void Display_CacheAll() {
	int i;
	for (i = 0; i < menuCount; i++) {
		Menu_CacheContents(&Menus[i]);
	}
}


static qboolean Menu_OverActiveItem(menuDef_t *menu, float x, float y) {
 	if (menu && menu->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED)) {
		if (Rect_ContainsPoint(&menu->window.rect, x, y)) {
			int i;
			for (i = 0; i < menu->itemCount; i++) {
				// turn off focus each item
				// menu->items[i].window.flags &= ~WINDOW_HASFOCUS;

				if (!(menu->items[i]->window.flags & (WINDOW_VISIBLE | WINDOW_FORCED))) {
					continue;
				}

				if (menu->items[i]->window.flags & WINDOW_DECORATION) {
					continue;
				}

				if (Rect_ContainsPoint(&menu->items[i]->window.rect, x, y)) {
					itemDef_t *overItem = menu->items[i];
					if (overItem->type == ITEM_TYPE_TEXT && overItem->text) {
						if (Rect_ContainsPoint(Item_CorrectedTextRect(overItem), x, y)) {
							return qtrue;
						} else {
							continue;
						}
					} else {
						return qtrue;
					}
				}
			}
		}
	}
	return qfalse;
}

const char* esc_menu_tabs[8] = {
	"tab_chooseteam",
	"tab_chooseclass",
	"tab_missions",
	"tab_admin",
	"tab_vote",
	"tab_controls",
	"tab_serverinfo",
	"tab_maphelp",
};

void HUD_Setup_Menu(const char* init_tab) {
	DC->setCVar("hud_focustab", init_tab);
}
