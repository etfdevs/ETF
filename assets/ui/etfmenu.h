#define POPUPMENUITEM(X, Y, ITEMTEXT, ITEMNAME, TARGET) \
	itemDef {										\
		name		ITEMNAME					\
		rect		X Y $evalint(MENU_WIDTH - 22) 14	\
		visible		MENU_TRUE							\
		text		ITEMTEXT							\
		textalignx	3									\
		textaligny	11									\
		textscale	.22									\
		forecolor	1 1 1 1								\
		type		ITEM_TYPE_BUTTON					\
		textstyle	ITEM_TEXTSTYLE_SHADOWED				\
		backcolor	.3 .3 .3 0							\
		style		WINDOW_STYLE_FILLED					\
		noPulseOnFocus									\
		action { uiScript openFORTBackModels TARGET ; close MENU_NAME }	\
		mouseEnter { setitemcolor ITEMNAME backcolor .8 .1 .1 .9 }	\
		mouseExit  { setitemcolor ITEMNAME backcolor .3 .3 .3 0 }		\
	}

#define POPUPMENUITEMACTION(X, Y, ITEMTEXT, ITEMNAME, ACTION) \
	itemDef {										\
		name		ITEMNAME					\
		rect		X Y $evalint(MENU_WIDTH - 22) 14	\
		visible		MENU_TRUE							\
		text		ITEMTEXT							\
		textalignx	3									\
		textaligny	11									\
		textscale	.22									\
		forecolor	1 1 1 1								\
		type		ITEM_TYPE_BUTTON					\
		textstyle	ITEM_TEXTSTYLE_SHADOWED				\
		backcolor	.3 .3 .3 0							\
		style		WINDOW_STYLE_FILLED					\
		noPulseOnFocus									\
		action {ACTION }	\
		mouseEnter { setitemcolor ITEMNAME backcolor .8 .1 .1 .9 }	\
		mouseExit  { setitemcolor ITEMNAME backcolor .3 .3 .3 0 }		\
	}


#define MENUSCREENSTYLE \
		name			MENU_NAME								\
		visible			MENU_FALSE								\
		fullscreen		MENU_FALSE								\
		rect			MENU_X MENU_Y MENU_WIDTH MENU_HEIGHT	\
		focusColor		.85 .41 .38 1							\
		disableColor	.5 .5 .5 1								\
		keepOpenOnFocusLost										\
		style			WINDOW_STYLE_FILLED						\
		backcolor		.05 .05 .05 0.5          

    
#define MENUMAINRECT(MENU_HEADER) \
	itemDef {									\
		rect		0 0 $evalint(MENU_WIDTH) $evalint(MENU_HEIGHT)	\
		style		WINDOW_STYLE_FILLED			\
		backcolor	0 0 0 .4					\
		border		WINDOW_BORDER_FULL			\
		bordercolor	.5 .5 .5 .5					\
		visible		1							\
		decoration								\
	}											\
	itemDef {									\
		rect		2 2 $evalint(MENU_WIDTH - 2) 19		\
		text		MENU_HEADER							\
		textscale	.24									\
		textalignx	10									\
		textaligny	14									\
		style		WINDOW_STYLE_GRADIENT				\
		backcolor	.6 0 0 .7							\
		forecolor	1 1 1 1								\
		visible		1									\
		decoration										\
	}													\
	itemDef {											\
		name			"closebutton"						\
		rect			$evalint(MENU_WIDTH - 21) 2 20 20	\
		visible 		MENU_TRUE							\
		background 		"ui/gfx/xmark.tga"					\
		altbackground	"ui/gfx/xmark_hi.tga"				\
		style 			WINDOW_STYLE_SHADER_ADJUST			\
		type			ITEM_TYPE_BUTTON					\
		mouseEnter		{ setitembackground closebutton alt }		\
		mouseExit		{ setitembackground closebutton primary }	\
		action			{ uiScript closeFORTBackModels ; conditionalScript isconnected 2 ( "setmenufocus menubar" ) ( "setmenufocus ingame" ) ; close MENU_NAME }	\
	}

#define MENUMAINRECTSMALL(MENU_HEADER) \
	itemDef {									\
		rect		0 0 $evalint(MENU_WIDTH) $evalint(MENU_HEIGHT)	\
		style		WINDOW_STYLE_FILLED			\
		backcolor	0 0 0 .4					\
		border		WINDOW_BORDER_FULL			\
		bordercolor	.5 .5 .5 .5					\
		visible		1							\
		decoration								\
	}											\
	itemDef {									\
		rect		2 2 $evalint(MENU_WIDTH - 2) 16		\
		text		MENU_HEADER							\
		textscale	.24									\
		textalignx	10									\
		textaligny	13									\
		style		WINDOW_STYLE_GRADIENT				\
		backcolor	.6 0 0 .7							\
		forecolor	1 1 1 1								\
		visible		1									\
		decoration										\
	}													\
	itemDef {											\
		name			"closebutton"						\
		rect			$evalint(MENU_WIDTH - 18) 2 16 16	\
		visible 		MENU_TRUE							\
		background 		"ui/gfx/xmark.tga"					\
		altbackground	        "ui/gfx/xmark_hi.tga"				\
		style 			WINDOW_STYLE_SHADER_ADJUST			\
		type			ITEM_TYPE_BUTTON					\
		mouseEnter		{ setitembackground closebutton alt }		\
		mouseExit		{ setitembackground closebutton primary }	\
		action			{ uiScript closeFORTBackModels ; conditionalScript isconnected 2 ( "setmenufocus menubar" ) ( "setmenufocus ingame" ) ; close MENU_NAME }	\
	}

#define POPUPMENUSTYLE \
		name			MENU_NAME								\
		visible			MENU_FALSE								\
		fullscreen		MENU_FALSE								\
		rect			MENU_X MENU_Y MENU_WIDTH MENU_HEIGHT	\
		focusColor		1 1 1 1									\
		disableColor	.3 .3 .3 1								\
		drawAlwaysOnTop											\
 	   	font			"fonts/veramobd" 26						\
  	  	smallFont		"fonts/veramobd" 20						\
  	  	bigFont			"fonts/veramobd" 32						\
		onESC			{ close MENU_NAME }						\
		onClose			{ conditionalScript isconnected 2 ( "setmenufocus menubar" ) ( "setmenufocus ingame" ) }				


#define POPUPMENUBG \
	itemDef {																		\
		name		"menu_play_background"											\
		visible		MENU_TRUE														\
		rect		16 16 $evalint(MENU_WIDTH - 32) $evalint(MENU_HEIGHT - 16)		\
		background	"ui/gfx/back"													\
		style		WINDOW_STYLE_SHADER_ADJUST										\
		decoration																	\
	}

#define POPUPMENUBORDERS \
	itemDef {												\
		visible		MENU_TRUE								\
		rect		0 16 16 $evalint(MENU_HEIGHT - 16)		\
		background	"ui/gfx/left"							\
		style		WINDOW_STYLE_SHADER_ADJUST				\
		decoration											\
	}														\
	itemDef {												\
		visible		MENU_TRUE								\
		rect		0 0 16 16								\
		background	"ui/gfx/topleft"						\
		style		WINDOW_STYLE_SHADER_ADJUST				\
		decoration											\
	}														\
	itemDef {												\
		visible		MENU_TRUE								\
		rect		16 0 $evalint(MENU_WIDTH - 32) 16		\
		background	"ui/gfx/top"							\
		style		WINDOW_STYLE_SHADER_ADJUST				\
		decoration											\
	}														\
	itemDef {												\
		visible		MENU_TRUE								\
		rect		$evalint(MENU_WIDTH - 16) 0 16 16		\
		background	"ui/gfx/topright"						\
		style		WINDOW_STYLE_SHADER_ADJUST				\
		decoration											\
		}													\
	itemDef {												\
		visible		MENU_TRUE								\
		rect		$evalint(MENU_WIDTH - 16) 16 16 $evalint(MENU_HEIGHT - 16)	\
		background	"ui/gfx/right"							\
		style		WINDOW_STYLE_SHADER_ADJUST				\
		decoration											\
	}

#define POPUPMENUBGINGAME \
	itemDef {																		\
		name		"menu_play_background"											\
		visible		MENU_TRUE														\
		rect		16 0 $evalint(MENU_WIDTH - 32) $evalint(MENU_HEIGHT - 16)		\
		background	"ui/gfx/back"													\
		style		WINDOW_STYLE_SHADER_ADJUST										\
		decoration																	\
	}

#define POPUPMENUBORDERSINGAME \
	itemDef {												\
		visible		MENU_TRUE								\
		rect		0 0 16 $evalint(MENU_HEIGHT - 16)		\
		background	"ui/gfx/rev_left"							\
		style		WINDOW_STYLE_SHADER_ADJUST				\
		decoration											\
	}														\
	itemDef {												\
		visible		MENU_TRUE								\
		rect		0 $evalint(MENU_HEIGHT - 16) 16 16								\
		background	"ui/gfx/botleft"						\
		style		WINDOW_STYLE_SHADER_ADJUST				\
		decoration											\
	}														\
	itemDef {												\
		visible		MENU_TRUE								\
		rect		16 $evalint(MENU_HEIGHT - 16) $evalint(MENU_WIDTH - 32) 16		\
		background	"ui/gfx/bottom"							\
		style		WINDOW_STYLE_SHADER_ADJUST				\
		decoration											\
	}														\
	itemDef {												\
		visible		MENU_TRUE								\
		rect		$evalint(MENU_WIDTH - 16) $evalint(MENU_HEIGHT - 16) 16 16		\
		background	"ui/gfx/botright"						\
		style		WINDOW_STYLE_SHADER_ADJUST				\
		decoration											\
		}													\
	itemDef {												\
		visible		MENU_TRUE								\
		rect		$evalint(MENU_WIDTH - 16) 0 16 $evalint(MENU_HEIGHT - 16)	\
		background	"ui/gfx/rev_right"							\
		style		WINDOW_STYLE_SHADER_ADJUST				\
		decoration											\
	}

#define SIMPLEGROUPBOX(BOXTEXT, FORT_WND_X, FORT_WND_Y, FORT_WND_WIDTH, FORT_WND_HEIGHT ) \
		itemDef {																		\
			rect		FORT_WND_X FORT_WND_Y FORT_WND_WIDTH FORT_WND_HEIGHT				\
			style		WINDOW_STYLE_FILLED													\
			backcolor	0 0 0 .2													\
			border		WINDOW_BORDER_FULL												\
			bordercolor	.5 .5 .5 .5												\
			visible		1																\
			decoration																	\
		}																				\
		itemDef {																		\
			rect		$evalint( FORT_WND_X + 2 ) $evalint( FORT_WND_Y + 2) $evalint( FORT_WND_WIDTH - 2) 12		\
			text		BOXTEXT													\
			textscale	.19																\
			textalignx	3																\
			textaligny	10																\
			style		WINDOW_STYLE_FILLED												\
			backcolor	.27 .28 .31 .8												\
			forecolor	.8 .8 .8 1												\
			visible		1																\
			decoration																	\
		}

#define SIMPLEGROUPBOXG(GROUPNAME, BOXTEXT, FORT_WND_X, FORT_WND_Y, FORT_WND_WIDTH, FORT_WND_HEIGHT ) \
		itemDef {                     \
                        group           GROUPNAME \
			rect		FORT_WND_X FORT_WND_Y FORT_WND_WIDTH FORT_WND_HEIGHT				\
			style		WINDOW_STYLE_FILLED													\
			backcolor	0 0 0 .2													\
			border		WINDOW_BORDER_FULL												\
			bordercolor	.5 .5 .5 .5												\
			visible		1																\
			decoration																	\
		}																				\
		itemDef {																		\
                        group           GROUPNAME \
			rect		$evalint( FORT_WND_X + 2 ) $evalint( FORT_WND_Y + 2) $evalint( FORT_WND_WIDTH - 2) 12		\
			text		BOXTEXT													\
			textscale	.19																\
			textalignx	3																\
			textaligny	10																\
			style		WINDOW_STYLE_FILLED												\
			backcolor	.27 .28 .31 .8												\
			forecolor	.8 .8 .8 1												\
			visible		1																\
			decoration																	\
		}



#define SIMPLEGROUPBOXBG(BOXTEXT, FORT_WND_X, FORT_WND_Y, FORT_WND_WIDTH, FORT_WND_HEIGHT, BG_COLOR ) \
		itemDef {																		\
			rect		FORT_WND_X FORT_WND_Y FORT_WND_WIDTH FORT_WND_HEIGHT				\
			style		WINDOW_STYLE_FILLED													\
			backcolor	BG_COLOR													\
			border		WINDOW_BORDER_FULL												\
			bordercolor	.5 .5 .5 .5												\
			visible		1																\
			decoration																	\
		}																				\
		itemDef {																		\
			rect		$evalint( FORT_WND_X + 2 ) $evalint( FORT_WND_Y + 2) $evalint( FORT_WND_WIDTH - 2) 12		\
			text		BOXTEXT													\
			textscale	.19																\
			textalignx	3																\
			textaligny	10																\
			style		WINDOW_STYLE_FILLED												\
			backcolor	.27 .28 .31 .8												\
			forecolor	.8 .8 .8 1												\
			visible		1																\
			decoration																	\
		}
