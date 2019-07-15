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

#ifndef __UI_SHARED_H
#define __UI_SHARED_H

#include "q_shared.h"
#include "tr_types.h"
#include "keycodes.h"

#include "../../ui/menudef.h"

#if defined(_MSC_VER)
//disable deprecated warnings
#pragma warning( disable : 4996 )
#endif

#define MAX_MENUNAME 32
#define MAX_ITEMTEXT 64
#define MAX_ITEMACTION 64
#define MAX_MENUDEFFILE 4096
#define MAX_MENUFILE 32768
#define MAX_MENUS 80
#define MAX_MENUITEMS 512
#define MAX_COLOR_RANGES 10
#define MAX_OPEN_MENUS 16

#define WINDOW_MOUSEOVER			0x00000001	// mouse is over it, non exclusive
#define WINDOW_HASFOCUS				0x00000002	// has cursor focus, exclusive
#define WINDOW_VISIBLE				0x00000004	// is visible
#define WINDOW_GREY					0x00000008	// is visible but grey ( non-active )
#define WINDOW_DECORATION			0x00000010	// for decoration only, no mouse, keyboard, etc.. 
#define WINDOW_FADINGOUT			0x00000020	// fading out, non-active
#define WINDOW_FADINGIN				0x00000040	// fading in
#define WINDOW_MOUSEOVERTEXT		0x00000080	// mouse is over it, non exclusive
#define WINDOW_INTRANSITION			0x00000100	// window is in transition
#define WINDOW_FORECOLORSET			0x00000200	// forecolor was explicitly set ( used to color alpha images or not )
#define WINDOW_HORIZONTAL			0x00000400	// for list boxes and sliders, vertical is default this is set of horizontal
#define WINDOW_LB_LEFTARROW			0x00000800	// mouse is over left/up arrow
#define WINDOW_LB_RIGHTARROW		0x00001000	// mouse is over right/down arrow
#define WINDOW_LB_THUMB				0x00002000	// mouse is over thumb
#define WINDOW_LB_PGUP				0x00004000	// mouse is over page up
#define WINDOW_LB_PGDN				0x00008000	// mouse is over page down
#define WINDOW_ORBITING				0x00010000	// item is in orbit
#define WINDOW_OOB_CLICK			0x00020000	// close on out of bounds click
#define WINDOW_WRAPPED				0x00040000	// manually wrap text
#define WINDOW_AUTOWRAPPED			0x00080000	// auto wrap text
#define WINDOW_FORCED				0x00100000	// forced open
#define WINDOW_POPUP				0x00200000	// popup
#define WINDOW_BACKCOLORSET			0x00400000	// backcolor was explicitly set 
#define WINDOW_TIMEDVISIBLE			0x00800000	// visibility timing ( NOT implemented )
// RR2DO2
#define WINDOW_NOPULSEONFOCUS		0x01000000	// doesn't pulse text when focussed
#define WINDOW_DRAWALWAYSONTOP		0x02000000	// window draws always on top
#define WINDOW_KEEPOPENONFOCUSLOST	0x04000000	// window doesn't close when other window gets the focus
#define WINDOW_USEASSETFONT			0x08000000	// item doesn't use the font of it's parent menudef but the one in the assetdef
// RR2DO2

// Slothy	
#define WINDOW_TEXTASINT			0x10000000
#define WINDOW_TEXTASFLOAT			0x20000000

#define CLASS_RECON					0x00000001
#define CLASS_SNIPER				0x00000002
#define CLASS_SOLDIER				0x00000004
#define CLASS_GRENADIER				0x00000008
#define CLASS_PARAMEDIC				0x00000010
#define CLASS_MINIGUNNER			0x00000020
#define CLASS_FLAMETROOPER			0x00000040
#define CLASS_AGENT					0x00000080
#define CLASS_ENGINEER				0x00000100
#define CLASS_CIVILIAN				0x00000200
#define CLASS_SPECTATOR				0x00000400
#define CLASS_ALL					(CLASS_RECON | CLASS_SNIPER | CLASS_SOLDIER | CLASS_GRENADIER | CLASS_PARAMEDIC | CLASS_MINIGUNNER | CLASS_FLAMETROOPER | CLASS_AGENT | CLASS_ENGINEER | CLASS_CIVILIAN | CLASS_SPECTATOR)

#define WEAPON_AXE					0x00000001
#define WEAPON_SHOTGUN				0x00000002
#define WEAPON_SUPERSHOTGUN			0x00000004
#define WEAPON_NAILGUN				0x00000008
#define WEAPON_SUPERNAILGUN			0x00000010
#define WEAPON_GRENADE_LAUNCHER		0x00000020
#define WEAPON_ROCKET_LAUNCHER		0x00000040
#define WEAPON_SNIPER_RIFLE			0x00000080
#define WEAPON_RAILGUN				0x00000100
#define WEAPON_FLAMETHROWER			0x00000200
#define WEAPON_MINIGUN				0x00000400
#define WEAPON_ASSAULTRIFLE			0x00000800
#define WEAPON_DARTGUN				0x00001000
#define WEAPON_PIPELAUNCHER			0x00002000
#define WEAPON_NAPALMCANNON			0x00004000
#define WEAPON_ALL					(WEAPON_AXE | WEAPON_SHOTGUN | WEAPON_SUPERSHOTGUN | WEAPON_NAILGUN | WEAPON_SUPERNAILGUN | WEAPON_GRENADE_LAUNCHER | WEAPON_ROCKET_LAUNCHER | WEAPON_SNIPER_RIFLE | WEAPON_RAILGUN | WEAPON_FLAMETHROWER | WEAPON_MINIGUN | WEAPON_ASSAULTRIFLE | WEAPON_DARTGUN | WEAPON_PIPELAUNCHER | WEAPON_NAPALMCANNON)

// end Slothy


// CGAME cursor type bits
#define CURSOR_NONE					0x00000001
#define CURSOR_ARROW				0x00000002
#define CURSOR_SIZER				0x00000004

#define STRING_POOL_SIZE 2048*1024

#define MAX_STRING_HANDLES 4096

#define MAX_SCRIPT_ARGS 12
#define MAX_EDITFIELD 256

//slothy - location and name of hud index files
#define HUDINFODIR		"ui/hud"
#define HUDINFOEXT		".cfg"

#define WEAPONSHOTDIR	"ui/weapons"
//end slothy

#define ART_FX_BASE			"menu/art/fx_base"
#define ART_FX_BLUE			"menu/art/fx_blue"
#define ART_FX_CYAN			"menu/art/fx_cyan"
#define ART_FX_GREEN		"menu/art/fx_grn"
#define ART_FX_RED			"menu/art/fx_red"
#define ART_FX_TEAL			"menu/art/fx_teal"
#define ART_FX_WHITE		"menu/art/fx_white"
#define ART_FX_YELLOW		"menu/art/fx_yel"

#define ASSET_SCROLLBAR             "ui/gfx/scroll_center_fill"
#define ASSET_SCROLLBAR_HOR         "ui/gfx/scroll_center_fill_hor"
#define ASSET_SCROLLBAR_ARROWDOWN   "ui/gfx/scroll_bottom"
#define ASSET_SCROLLBAR_ARROWUP     "ui/gfx/scroll_top"
#define ASSET_SCROLLBAR_ARROWLEFT   "ui/gfx/scroll_left"
#define ASSET_SCROLLBAR_ARROWRIGHT  "ui/gfx/scroll_right"
#define ASSET_SCROLL_THUMB          "ui/gfx/scroll_button"
#define SCROLLBAR_SIZE				16.0
#define ASSET_GRADIENTBAR			"ui/assets/gradientbar1.tga"		// slothy
#define ASSET_BUTTONLEFT			"ui/gfx/button_left.tga"			// slothy
#define ASSET_BUTTONMID				"ui/gfx/button_center.tga"		// slothy
#define ASSET_BUTTONRIGHT			"ui/gfx/button_right.tga"		// slothy
#define ASSET_BUTTONLEFTHI			"ui/gfx/button_left_hi.tga"			// slothy
#define ASSET_BUTTONMIDHI			"ui/gfx/button_center_hi.tga"		// slothy
#define ASSET_BUTTONRIGHTHI			"ui/gfx/button_right_hi.tga"		// slothy

#define ASSET_SLIDER_BAR			"ui/gfx/slider_fill"
#define ASSET_SLIDER_THUMB			"ui/gfx/slider_adjuster"
#define ASSET_SLIDER_END_L			"ui/gfx/slider_end_L"
#define ASSET_SLIDER_END_R			"ui/gfx/slider_end_R"
#define SLIDER_WIDTH				96.0
#define SLIDER_HEIGHT				16.0
#define SLIDER_END_WIDTH			16.0
#define SLIDER_THUMB_WIDTH			16.0
#define SLIDER_THUMB_HEIGHT			16.0

#define ASSET_LED_ON			"ui/gfx/led_on.tga"
#define ASSET_LED_OFF			"ui/gfx/led_off.tga"

// slothy
#define ASSET_CHECKBOX_CHECK	 "ui/gfx/check.tga"
#define ASSET_CHECKBOX_CHECK_NOT "ui/gfx/check_not.tga"
#define ASSET_CHECKBOX_CHECK_NO	 "ui/gfx/check_no.tga"

#define MODEL_AGENT				"ui/classes/agent.tga"
#define MODEL_ENGINEER			"ui/classes/engineer.tga"
#define MODEL_FLAMETROOPER		"ui/classes/flametrooper.tga"
#define MODEL_GRENADIER			"ui/classes/grenadier.tga"
#define MODEL_MINIGUNNER		"ui/classes/minigunner.tga"
#define MODEL_PARAMEDIC			"ui/classes/paramedic.tga"
#define MODEL_RECON				"ui/classes/recon.tga"
#define MODEL_SNIPER			"ui/classes/sniper.tga"
#define MODEL_SOLDIER			"ui/classes/soldier.tga"
#define MODEL_CIVILIAN			"ui/classes/civilian.tga"
// slothy end

/* not used
#define Q3F_BORDER_TOP_LEFT		"ui/gfx/hud/top_left"
#define Q3F_BORDER_TOP			"ui/gfx/hud/fill_top"
#define Q3F_BORDER_TOP_RIGHT	"ui/gfx/hud/top_right"
#define Q3F_BORDER_RIGHT		"ui/gfx/hud/fill_right"
#define Q3F_BORDER_BOTTOM_RIGHT "ui/gfx/hud/bot_right"
#define Q3F_BORDER_BOTTOM		"ui/gfx/hud/fill_bot"
#define Q3F_BORDER_BOTTOM_LEFT	"ui/gfx/hud/bot_left"
#define Q3F_BORDER_LEFT			"ui/gfx/hud/fill_left"
#define Q3F_BORDER_FILL			"ui/gfx/hud/fill_center_grey"
*/

#define	NUM_CROSSHAIRS				10

typedef struct {
	const char *command;
	const char *args[MAX_SCRIPT_ARGS];
} scriptDef_t;


typedef struct {
	float x;    // horiz position
	float y;    // vert position
	float w;    // width
	float h;    // height;
} rectDef_t;

typedef rectDef_t Rectangle;

// FIXME: do something to separate text vs window stuff
typedef struct {
  Rectangle rect;                 // client coord rectangle
  Rectangle rectClient;           // screen coord rectangle
  const char *name;               //
  const char *group;              // if it belongs to a group
  const char *cinematicName;		  // cinematic name
  int cinematic;								  // cinematic handle
  int style;                      //
  int border;                     //
  int ownerDraw;									// ownerDraw style
	int ownerDrawFlags;							// show flags for ownerdraw items
  float borderSize;               // 
  int flags;                      // visible, focus, mouseover, cursor
  Rectangle rectEffects;          // for various effects
  Rectangle rectEffects2;         // for various effects
  int offsetTime;                 // time based value for various effects
  int nextTime;                   // time next effect should cycle
  vec4_t foreColor;               // text color
  vec4_t backColor;               // border color
  vec4_t borderColor;             // border color
  vec4_t outlineColor;            // border color
  qhandle_t background;           // background asset  
  qhandle_t primbackground;       // 1st background asset  
  qhandle_t altbackground;        // 2nd background asset  
} windowDef_t;

typedef windowDef_t Window;

typedef struct {
	vec4_t	color;
	float		low;
	float		high;
} colorRangeDef_t;

// FIXME: combine flags into bitfields to save space
// FIXME: consolidate all of the common stuff in one structure for menus and items
// THINKABOUTME: is there any compelling reason not to have items contain items
// and do away with a menu per say.. major issue is not being able to dynamically allocate 
// and destroy stuff.. Another point to consider is adding an alloc free call for vm's and have 
// the engine just allocate the pool for it based on a cvar
// many of the vars are re-used for different item types, as such they are not always named appropriately
// the benefits of c++ in DOOM will greatly help crap like this
// FIXME: need to put a type ptr that points to specific type info per type
// 
#define MAX_LB_COLUMNS 16

typedef struct columnInfo_s {
	int pos;
	int width;
	int maxChars;
} columnInfo_t;

typedef struct listBoxDef_s {
	int startPos;
	int endPos;
	int drawPadding;
	int cursorPos;
	float elementWidth;
	float elementHeight;
	int elementStyle;
	int numColumns;
	columnInfo_t columnInfo[MAX_LB_COLUMNS];
	const char *doubleClick;
	qboolean notselectable;
} listBoxDef_t;

typedef struct editFieldDef_s {
  float minVal;                  //	edit field limits
  float maxVal;                  //
  float defVal;                  //
	float range;									 // 
  int maxChars;                  // for edit fields
  int maxPaintChars;             // for edit fields
	int paintOffset;							 // 

	float scale;				// slothy - for slider scale support
} editFieldDef_t;

#define MAX_MULTI_CVARS 32

typedef struct multiDef_s {
	const char *cvarList[MAX_MULTI_CVARS];
	const char *cvarStr[MAX_MULTI_CVARS];
	float cvarValue[MAX_MULTI_CVARS];
	int count;
	qboolean strDef;
} multiDef_t;

#define CVAR_ENABLE			0x00000001
#define CVAR_DISABLE		0x00000002
#define CVAR_SHOW			0x00000004
#define CVAR_HIDE			0x00000008

typedef struct itemDef_s {
	Window window;                 // common positional, border, style, layout info
	Rectangle textRect;            // rectangle the text ( if any ) consumes     
	int type;                      // text, button, radiobutton, checkbox, textfield, listbox, combo
	int alignment;                 // left center right
	int textalignment;             // ( optional ) alignment for text within rect based on text width
	float textalignx;              // ( optional ) text alignment x coord
	float textaligny;              // ( optional ) text alignment x coord
	float textscale;               // scale percentage from 72pts
	int textStyle;                 // ( optional ) style, normal and shadowed are it for now
	const char *text;              // display text
	void *parent;                  // menu owner
	qhandle_t asset;               // handle to asset
	const char *mouseEnterText;    // mouse enter script
	const char *mouseExitText;     // mouse exit script
	const char *mouseEnter;        // mouse enter script
	const char *mouseExit;         // mouse exit script 
	const char *action;            // select script
	const char *onFocus;           // select script
	const char *leaveFocus;        // select script
	const char *cvar;              // associated cvar 
	const char *cvarTest;          // associated cvar for enable actions
	const char *enableCvar;			   // enable, disable, show, or hide based on value, this can contain a list
	int cvarFlags;								 //	what type of action to take on cvarenables
	sfxHandle_t focusSound;
	int numColors;								 // number of color ranges
	colorRangeDef_t colorRanges[MAX_COLOR_RANGES];
	float special;								 // used for feeder id's etc.. diff per type
	int cursorPos;                 // cursor position in characters
	int anchorx;
	int anchory;
	void *typeData;								 // type specific data ptr's	

	// Slothy
	struct itemDef_s *toolTipData;	// OSP - Tag an item to this item for auto-help popups

	int classLimit;								// slothy - show items only for specific classes
	int weaponLimit;							// slothy - show item only for specific weapon

} itemDef_t;

typedef struct fontStruct_s {
	fontInfo_t	textFont;
	fontInfo_t	smallFont;
	fontInfo_t	bigFont;	
	qboolean	fontRegistered;
} fontStruct_t;

#define MAX_KEY_SCRIPTS 20

typedef struct {
	int			key;
	const char*	script;
} keyScript_t;

typedef struct {
	Window window;
	//const char *font;							// font
	fontStruct_t font;
	qboolean fullScreen;						// covers entire screen 
	int itemCount;								// number of items;
	int fontIndex;								// 
	int cursorItem;								// which item as the cursor
	int fadeCycle;								//
	float fadeClamp;							//
	float fadeAmount;							//
	const char *onOpen;							// run when the menu is first opened
	const char *onClose;						// run when the menu is closed
	const char *onESC;							// run when the menu is closed
	const char *onOOBClick;						// RR2DO2: run when an oob click event happens and menu has 'outOfBoundsClick' set
	const char *soundName;						// background loop sound for menu

	vec4_t focusColor;							// focus color for items
	vec4_t disableColor;						// focus color for items
	itemDef_t *items[MAX_MENUITEMS];			// items this menu contains   
	qhandle_t borderBitmaps[9];

	// djbob: for edithud
	qboolean	nonSticky;
	rectDef_t	realRect;

	int classLimit;								// slothy - show entire menu only for specific classes
	int weaponLimit;							// slothy - show item only for specific weapon

	keyScript_t keyScripts[MAX_KEY_SCRIPTS];
	int			numKeyScripts;
} menuDef_t;

typedef struct {
	const char *fontStr;
	const char *cursorStr;
	fontStruct_t font;
	qhandle_t cursor;
	qhandle_t scrollBarArrowUp;
	qhandle_t scrollBarArrowDown;
	qhandle_t scrollBarArrowLeft;
	qhandle_t scrollBarArrowRight;
	qhandle_t scrollBar;
	qhandle_t scrollBar_hor;
	qhandle_t scrollBarThumb;
	qhandle_t buttonMiddle;
	qhandle_t buttonInside;
	qhandle_t solidBox;
	qhandle_t sliderBar;
	qhandle_t sliderThumb;
	qhandle_t sliderEndLeft;
	qhandle_t sliderEndRight;	
	sfxHandle_t menuEnterSound;
	sfxHandle_t menuExitSound;
	sfxHandle_t menuBuzzSound;
	sfxHandle_t itemFocusSound;
	float fadeClamp;
	int fadeCycle;
	float fadeAmount;
	float shadowX;
	float shadowY;
	vec4_t shadowColor;
	float shadowFadeClamp;
	//qboolean fontRegistered;

	// player settings
	qhandle_t fxBasePic;
	qhandle_t fxPic[7];
	qhandle_t	crosshairShader[NUM_CROSSHAIRS];
	qhandle_t	crosshairAltShader[NUM_CROSSHAIRS];  //keeg for et new xhair

//	qhandle_t Q3F_BM_InnerModel;
//	qhandle_t Q3F_BM_ContentModel;
//	qhandle_t Q3F_BM_MiddleModel;
//	qhandle_t Q3F_BM_OuterModel;

	qhandle_t LEDon;
	qhandle_t LEDoff;

	//slothy
	qhandle_t checkboxCheck;
	qhandle_t checkboxCheckNot;
	qhandle_t checkboxCheckNo;

	// slothy - cache armours
	qhandle_t ArmorTypes[5];
	qhandle_t ArmorColor[3];

	// slothy - ported from ET
	qhandle_t gradientBar;

	// slothy - for painting buttons
	qhandle_t btnleft[2];			// 0 = default, 1 = highlight
	qhandle_t btnmid[2];
	qhandle_t btnright[2];

	// slothy - server browser
	qhandle_t lock;
	qhandle_t pureon;
	qhandle_t pureoff;

	qhandle_t xammo;
} cachedAssets_t;

typedef struct {
	const char *name;
	void (*handler) (itemDef_t *item, char** args);
} commandDef_t;

typedef struct {
	const char *name;
	void (*handler) ();
} hudCommandDef_t;

// djbob

#define CONFIG_TYPE_BIND		0
#define CONFIG_TYPE_YESNO		1
#define CONFIG_TYPE_SLIDER		2
#define CONFIG_TYPE_FLOATLIST	3
#define CONFIG_TYPE_TEXTLIST	4

typedef struct {
	const char* title;
	const char* command;
	float		primary;
	float		secondary;
	float		value;
	int			type;
	float		scale;
	char*		string;
	const char*	strvalue;
} configData_t;

// djbob

typedef struct {
	qhandle_t (*registerShaderNoMip) (const char *p);
	void (*setColor) (const vec4_t v);
	void (*drawHandlePic) (float x, float y, float w, float h, qhandle_t asset);
	void (*drawAdjustedPic) (float x, float y, float w, float h, qhandle_t asset);
	void (*drawStretchPic) (float x, float y, float w, float h, float s1, float t1, float s2, float t2, qhandle_t hShader );
	void (*drawText) (float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style, fontStruct_t *parentfont, int textalignment );
	int (*textWidth) (const char *text, float scale, int limit, fontStruct_t *parentfont);
	int (*textHeight) (const char *text, float scale, int limit, fontStruct_t *parentfont);
	qhandle_t (*registerModel) (const char *p);
	void (*modelBounds) (qhandle_t model, vec3_t min, vec3_t max);
	void (*fillRect) ( float x, float y, float w, float h, const vec4_t color );
	void (*drawRect) ( float x, float y, float w, float h, float size, const vec4_t color );
	void (*drawSides) (float x, float y, float w, float h, float size );
	void (*drawTopBottom) (float x, float y, float w, float h, float size) ;
	void (*clearScene) ();
	void (*addRefEntityToScene) (const refEntity_t *re );
	void (*addLightToScene) ( const vec3_t org, float radius, float intensity, float r, float g, float b, qhandle_t hShader, int flags );
	void (*renderScene) ( const refdef_t *fd );
	void (*registerFont) (const char *pFontname, int pointSize, fontInfo_t *font );
	void (*ownerDrawItem) ( itemDef_t *item, float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment );
	float (*getValue) (int ownerDraw);
	qboolean (*ownerDrawVisible) (int flags);
	void (*runScript)(char **p);
	void (*getTeamColor)(vec4_t *color);
	void (*getCVarString)(const char *cvar, char *buffer, int bufsize);
	float (*getCVarValue)(const char *cvar);
	void (*setCVar)(const char *cvar, const char *value);
	void (*drawTextWithCursor)(float x, float y, float scale, vec4_t color, const char *text, int cursorPos, char cursor, int limit, int style, fontStruct_t *parentfont, int textalignment);
	void (*setOverstrikeMode)(qboolean b);
	qboolean (*getOverstrikeMode)();
	void (*startLocalSound)( sfxHandle_t sfx, int channelNum );
	qboolean (*ownerDrawHandleKey)(int ownerDraw, int flags, float *special, int key);
	int (*feederCount)(float feederID, itemDef_t* item);
	const char *(*feederItemText)(float feederID, int index, int column, qhandle_t *handle, itemDef_t* item);
	configData_t* (*feederItemInfo)(float feederID, int index, itemDef_t* item);
	qhandle_t (*feederItemImage)(float feederID, int index, itemDef_t* item);
	qboolean (*feederPaintSpecial)(itemDef_t* item);
	void (*feederSelection)(float feederID, int index);
	void (*keynumToStringBuf)( int keynum, char *buf, int buflen );
	void (*getBindingBuf)( int keynum, char *buf, int buflen );
	void (*setBinding)( int keynum, const char *binding );
	void (*executeText)(int exec_when, const char *text );	
	void (*Error)(int level, const char *error, ...);
	void (*Print)(const char *msg, ...);
	void (*Pause)(qboolean b);
	int (*ownerDrawWidth)(int ownerDraw, float scale, fontStruct_t *parentfont);
	qboolean (*ownerDrawSize)(int ownerDraw, rectDef_t* in, rectDef_t* out, itemDef_t* item, float* alpha);
	sfxHandle_t (*registerSound)(const char *name, qboolean compressed);
	void (*startBackgroundTrack)( const char *intro, const char *loop, int fadeupTime );
	void (*stopBackgroundTrack)();
	int (*playCinematic)(const char *name, float x, float y, float w, float h);
	void (*stopCinematic)(int handle);
	void (*drawCinematic)(int handle, float x, float y, float w, float h);
	void (*runCinematicFrame)(int handle);
	int (*openFile)( const char *qpath, fileHandle_t *f, fsMode_t mode );
	void (*fRead)( void *buffer, int len, fileHandle_t f );
	void (*fWrite)( const void *buffer, int len, fileHandle_t f );
	void (*closeFile)( fileHandle_t f );
	qboolean (*keyIsDown)( int keynum );
	void (*adjustFrom640)( float *x, float *y, float *w, float *h );


	float			yscale;
	float			xscale;
	float			bias;
	int				realTime;
	int				frameTime;
	int				cursorx;
	int				cursory;
	qboolean		debug;

	cachedAssets_t	Assets;

	glconfig_t		glconfig;
	qhandle_t		whiteShader;
	qhandle_t		cursor;
	float			FPS;

	// slothy
	int				playerClass;
	int				weapon;

	int				hudPreviewCin;
	char			curHud[64];
	int				curHudInt;
	char			curHudVariant[64];
	int				curHudVarInt;

	qhandle_t		weapPreview;
	const char *	(*curWeapSource);
	int				curWeapInt;
	int				lastFeeder;

} displayContextDef_t;

const char *String_Alloc(const char *p);
void String_Init();
#ifdef CGAME
void CG_Menu_Init();
#endif
void String_Report();
void Init_Display(displayContextDef_t *dc);
void Display_ExpandMacros(char * buff);
void Menu_Init(menuDef_t *menu);
void Item_Init(itemDef_t *item);
void Menu_UpdatePosition(menuDef_t *menu);
void Menu_PostParse(menuDef_t *menu);
menuDef_t *Menu_GetFocused();
void Menu_HandleKey(menuDef_t *menu, int key, qboolean down);
void Menu_HandleMouseMove(menuDef_t *menu, float x, float y);
void Menu_ScrollFeeder(menuDef_t *menu, int feeder, qboolean down);
qboolean Float_Parse(char **p, float *f);
qboolean Color_Parse(char **p, vec4_t *c);
qboolean Int_Parse(char **p, int *i);
qboolean Rect_Parse(char **p, rectDef_t *r);
qboolean String_Parse(char **p, const char **out);
qboolean Script_Parse(char **p, const char **out);
qboolean PC_Float_Parse(int handle, float *f);
qboolean PC_Color_Parse(int handle, vec4_t *c);
qboolean PC_Int_Parse(int handle, int *i);
qboolean PC_Rect_Parse(int handle, rectDef_t *r);
qboolean PC_String_Parse(int handle, const char **out);
qboolean PC_Script_Parse(int handle, const char **out);
int Menu_Count();
void Menu_New(int handle);
void Menu_PaintAll();
menuDef_t *Menus_ActivateByName(const char *p);
void Menu_Reset();
void  Menus_Activate(menuDef_t *menu);
// RR2DO2
menuDef_t *Menu_Get( int menu_num );
// RR2DO2

void Menus_MoveToY(menuDef_t *menu, int newY);
void Item_RunScript(itemDef_t *item, const char *s);
itemDef_t *Menu_ClearFocus(menuDef_t *menu);
void Menu_ShowItemByName(menuDef_t *menu, const char *p, qboolean bShow);
void Script_SetFocus(itemDef_t *item, char **args);

displayContextDef_t *Display_GetContext();
void *Display_CaptureItem(int x, int y);
qboolean Display_MouseMove(void *p, int x, int y);
int Display_CursorType(int x, int y);
qboolean Display_KeyBindPending();
void Menus_OpenByName(const char *p);
menuDef_t *Menus_FindByName(const char *p);
itemDef_t *Menu_FindItemByName(menuDef_t *menu, const char *p);
void Menus_ShowByName(const char *p);
void Menus_CloseByName(const char *p);
void Display_HandleKey(int key, qboolean down, int x, int y);
void LerpColor(vec4_t a, vec4_t b, vec4_t c, float t);
void Menus_CloseAll();
void Menu_Paint(menuDef_t *menu, qboolean forcePaint);
int Menu_GetFeederSelection(menuDef_t *menu, int feeder, const char *name);
void Menu_SetFeederSelection(menuDef_t *menu, int feeder, int index, const char *name);
void Menus_SetFeederSelection(int feeder, int index);
void Display_CacheAll();
qboolean Menus_AnyFullScreenVisible();

qboolean IsVisible(int flags);

void *UI_Alloc( int size );
void UI_InitMemory( void );
qboolean UI_OutOfMemory();

// RR2DO2
void UI_Q3F_LoadFontFile( const char *fontName, int pointSize, fontInfo_t *font );

/*void Controls_GetConfig( void );
void Controls_SetConfig(qboolean restart);
void Controls_SetDefaults( void );*/
void Controls_GetKeyAssignment (const char *command, int *twokeys);

int			trap_PC_AddGlobalDefine( char *define );
int			trap_PC_RemoveAllGlobalDefines( void );
int			trap_PC_LoadSource( const char *filename );
int			trap_PC_FreeSource( int handle );
int			trap_PC_ReadToken( int handle, pc_token_t *pc_token );
int			trap_PC_SourceFileAndLine( int handle, char *filename, int *line );
int			trap_PC_UnReadToken( int handle );

// djbob
extern qboolean g_waitingForKey;
extern qboolean	KeyBinder_HandleKey(int key, qboolean down);

void HUD_Setup_Menu(const char* init_tab);

extern const char* esc_menu_tabs[8];

#endif
