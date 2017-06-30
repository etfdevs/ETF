
#define ITEM_TYPE_TEXT					0			// simple text
#define ITEM_TYPE_BUTTON				1			// button, basically text with a border
#define ITEM_TYPE_RADIOBUTTON			2			// toggle button, may be grouped
#define ITEM_TYPE_CHECKBOX				3			// check box
#define ITEM_TYPE_EDITFIELD				4			// editable text, associated with a cvar
#define ITEM_TYPE_COMBO					5			// drop down list
#define ITEM_TYPE_LISTBOX				6			// scrollable list
#define ITEM_TYPE_OWNERDRAW				8			// owner draw, name specs what it is
#define ITEM_TYPE_NUMERICFIELD			9			// editable text, associated with a cvar
#define ITEM_TYPE_SLIDER				10			// mouse speed, volume, etc.
#define ITEM_TYPE_YESNO					11			// yes no cvar setting
#define ITEM_TYPE_MULTI					12			// multiple list setting, enumerated
#define ITEM_TYPE_BORDER				14			//
#define ITEM_TYPE_MLTEXT				15			// multiline text view
#define ITEM_TYPE_LED					16			// little light
#define ITEM_TYPE_TRICHECKBOX			17			// tri-state check box -- slothy

#define ITEM_TYPE_PASSWORDFIELD			99

#define ITEM_ALIGN_LEFT					0			// left alignment
#define ITEM_ALIGN_CENTER				1			// center alignment
#define ITEM_ALIGN_RIGHT				2			// right alignment
#define ITEM_ALIGN_AUTO					3			// auto right adjust

#define ITEM_TEXTSTYLE_NORMAL			0			// normal text
#define ITEM_TEXTSTYLE_BLINK			1			// fast blinking
#define ITEM_TEXTSTYLE_PULSE			2			// slow pulsing
#define ITEM_TEXTSTYLE_SHADOWED			3			// drop shadow ( need a color for this )
#define ITEM_TEXTSTYLE_SHADOWEDMORE		6			// drop shadow ( need a color for this )
#define ITEM_TEXTSTYLE_PAD				8			// add _ for maxchars

#define WINDOW_BORDER_NONE				0			// no border
#define WINDOW_BORDER_FULL				1			// full border based on border color ( single pixel )
#define WINDOW_BORDER_HORZ				2			// horizontal borders only
#define WINDOW_BORDER_VERT				3			// vertical borders only

#define WINDOW_STYLE_EMPTY				0			// no background
#define WINDOW_STYLE_FILLED				1			// filled with background color
#define WINDOW_STYLE_GRADIENT			2			// gradient bar based on background color
#define WINDOW_STYLE_SHADER				3			// background shader
#define WINDOW_STYLE_TEAMCOLOR			4			// team color
#define WINDOW_STYLE_CINEMATIC			5			// cinematic
#define WINDOW_STYLE_SHADER_ADJUST		6

#define MENU_TRUE						1			// uh.. true
#define MENU_FALSE						0			// and false

#define HUD_VERTICAL					0
#define HUD_HORIZONTAL					1

// list box element types
#define LISTBOX_TEXT					0
#define LISTBOX_IMAGE					1
#define LISTBOX_MULTI_CONTROLS			2

// list feeders
#define FEEDER_MAPS						1		// text maps based on game type
#define FEEDER_SERVERS					2		// servers
#define FEEDER_ALLMAPS					3		// all maps available, in graphic format
#define FEEDER_PLAYER_LIST				4		// players
#define FEEDER_TEAM_LIST				5		// team members for team voting
#define FEEDER_MODS						6		// mods
#define FEEDER_DEMOS 					7		// demos
#define FEEDER_SCOREBOARD				8		// scoreboard
#define FEEDER_SERVERSTATUS				9		// server status
#define FEEDER_FINDPLAYER				10		// find player
#define FEEDER_CINEMATICS				11		// cinematics
#define FEEDER_BINDINGS					12
#define FEEDER_CONFIGS					13
#define FEEDER_CLASS_CONFIGS			14
#define FEEDER_SYSTEM					15
#define FEEDER_FAVSERVERS				16
#define FEEDER_CLASSINFO				17
#define FEEDER_ADMIN_PLAYERS			18
#define FEEDER_ADMIN_IPS				19
#define FEEDER_ADMIN_BANS				20
#define FEEDER_SERVER_MAPLIST			21
#define FEEDER_GENERAL_SETTINGS			22

#define FEEDER_SCOREBOARD_TEAM1			23		// slothy - individual team scores
#define FEEDER_SCOREBOARD_TEAM2			24
#define FEEDER_SCOREBOARD_TEAM3			25
#define FEEDER_SCOREBOARD_TEAM4			26

#define FEEDER_HUDS						27
#define FEEDER_HUDSVARIANT				28

// slothy
#define FEEDER_MAPBLURB					30
#define FEEDER_HUDDESCR					31

#define FEEDER_BIND_MOVE				33
#define FEEDER_BIND_SHOOT				34
#define FEEDER_BIND_MISC				35
#define FEEDER_BIND_COMS				36

#define FEEDER_WEAPONINFO				40
#define FEEDER_WEAPONS  				41
#define FEEDER_GRENADES  				42
#define FEEDER_ITEMS    				43
#define FEEDER_WEAPINFO    				44

#define FEEDER_SCOREBOARD_SPECS			45
#define FEEDER_CUSTOMMENU				46
#define FEEDER_STATS					47

#define FEEDER_SERVER_PLAYERS			48		// players connected to selected server

// slothy


// display flags
#define CG_SHOW_RED_TEAM_EXISTS			0x00000001
#define CG_SHOW_BLUE_TEAM_EXISTS		0x00000002
#define CG_SHOW_YELLOW_TEAM_EXISTS		0x00000004
#define CG_SHOW_GREEN_TEAM_EXISTS		0x00000008
#define CG_SHOW_PLAYER_HAS_SENTRYCAM	0x00000010
#define CG_SHOW_PLAYER_HAS_SENTRY		0x00000020
#define CG_SHOW_PLAYER_HAS_SUPSTATION	0x00000040
#define CG_SHOW_PLAYER_HAS_SCANNERON	0x00000080
#define CG_SHOW_ENEMY_USE_SUPSTATION	0x00000100
#define CG_SHOW_FRIENDLY_USE_SUPSTATION 0x00000200

#define CG_SHOW_HEALTHCRITICAL			0x00000400
#define CG_SHOW_LANPLAYONLY				0x00000800
#define CG_SHOW_HEALTHOK				0x00001000
#define CG_SHOW_2DONLY					0x00002000
#define CG_SHOW_WEAPONSWITCH			0x00004000

#define CG_SHOW_ON_RED_TEAM				0x00008000
#define CG_SHOW_ON_BLUE_TEAM			0x00010000
#define CG_SHOW_ON_YELLOW_TEAM			0x00020000
#define CG_SHOW_ON_GREEN_TEAM			0x00040000
#define CG_SHOW_PLAYER_IS_AGENT			0x00080000
#define	CG_SHOW_CHATEDIT				0x00100000
#define	CG_SHOW_ON_TEAM					0x00200000
#define CG_SHOW_ESC_MENU				0x00400000
#define	CG_SHOW_GREN_PRIMED				0x00800000
#define CG_SHOW_GREN_PRIMED2			0x01000000
#define CG_SHOW_ALERT_ICON				0x02000000
#define CG_SHOW_NO_SCOREBOARD			0x04000000

//#define CG_SHOW_NOT_ON_RED_TEAM			0x10000000
//#define CG_SHOW_NOT_ON_BLUE_TEAM		0x20000000
//#define CG_SHOW_NOT_ON_YELLOW_TEAM		0x40000000
//#define CG_SHOW_NOT_ON_GREEN_TEAM		0x80000000
#define CG_SHOW_RED_TEAM_NOT_EXISTS			0x10000000
#define CG_SHOW_BLUE_TEAM_NOT_EXISTS		0x20000000
#define CG_SHOW_YELLOW_TEAM_NOT_EXISTS		0x40000000
#define CG_SHOW_GREEN_TEAM_NOT_EXISTS		0x80000000


#define UI_SHOW_FAVORITESERVERS			0x00000001
#define UI_SHOW_DEMOAVAILABLE			0x00000002
#define UI_SHOW_NOTFAVORITESERVERS		0x00000004

#define UI_SHOW_RED_TEAM_EXISTS			0x00000010
#define UI_SHOW_BLUE_TEAM_EXISTS		0x00000020
#define UI_SHOW_GREEN_TEAM_EXISTS		0x00000040
#define UI_SHOW_YELLOW_TEAM_EXISTS		0x00000080

// owner draw types
// ideally these should be done outside of this file but
// this makes it much easier for the macro expansion to
// convert them for the designers ( from the .menu files )
#define CG_OWNERDRAW_BASE		1
#define CG_PLAYER_ARMOR_ICON	1
#define CG_PLAYER_ARMOR_VALUE	2
#define CG_PLAYER_ARMOR_BAR		3
#define CG_PLAYER_HEAD			4
#define CG_PLAYER_HEALTH		5
#define CG_PLAYER_HEALTH_BAR	6
#define CG_PLAYER_AMMOSLOT_ROCKET_VALUE	7
#define CG_PLAYER_AMMOSLOT_SHELLS_VALUE	8
#define CG_PLAYER_AMMOSLOT_NAILS_VALUE	9
#define CG_PLAYER_AMMOSLOT_CELLS_VALUE	10

#define CG_PLAYER_AMMO_ICON			11		// Main slot
#define CG_PLAYER_AMMO_VALUE		12
#define CG_PLAYER_AMMO_CLIP_VALUE	13

#define CG_PLAYER_PRIMARY_GRENADE_VALUE		14
#define CG_PLAYER_SECONDARY_GRENADE_VALUE	15

#define CG_SYSTEM_LAGOMETER	16
#define CG_SYSTEM_FPS		17
#define CG_SYSTEM_TIMER		18

#define CG_ENGINEER_SENTRYCAM			19
#define CG_ENGINEER_SENTRYHEALTH		20
#define CG_ENGINEER_SENTRYBULLETS		21
#define CG_ENGINEER_SENTRYROCKETS		22
#define CG_ENGINEER_SUPSTATIONHEALTH	23

#define CG_RECON_SCANNER	24
#define CG_PLAYER_ITEM		25
#define CG_PLAYER_SCORE		26

#define CG_BLUE_SCORE		27
#define CG_RED_SCORE		28
#define CG_YELLOW_SCORE		29
#define CG_GREEN_SCORE		30

#define CG_RED_NAME			31
#define CG_BLUE_NAME		32
#define CG_YELLOW_NAME		33
#define CG_GREEN_NAME		34

#define CG_RED_TEAMCOUNT	35
#define CG_BLUE_TEAMCOUNT	36
#define CG_YELLOW_TEAMCOUNT	37
#define CG_GREEN_TEAMCOUNT	38

#define CG_RED_TEAMPING		39
#define CG_BLUE_TEAMPING	40
#define CG_YELLOW_TEAMPING	41
#define CG_GREEN_TEAMPING	42

#define CG_PLAYER_LOCATION	43
#define CG_TEAM_COLOR		44

#define CG_GAME_TYPE		47

#define CG_GAME_STATUS		54
#define CG_KILLER			55
#define CG_PLAYER_AMMO_ICON2D	57
#define CG_ACCURACY			58
#define CG_ASSISTS			59
#define CG_DEFEND			60
#define CG_EXCELLENT		61
#define CG_IMPRESSIVE		62
#define CG_PERFECT			63
#define CG_GAUNTLET			64
#define CG_SPECTATORS		65
// djbob
#define CG_CHATBOX_CONTENT			75
#define CG_TEAMCHATBOX_CONTENT		76
#define CG_CENTERPRINTBOX_CONTENT	77
#define CG_CROSSHAIRINFO_BOX		78
#define CG_CHATEDIT_CONTENT			79

#define CG_WEAPONSWITCH_BOX			80
#define CG_CLASS_ICON				81
#define CG_AGENTDISGUISE_INFO		82
#define CG_MENUBOX_CONTENT			83
#define CG_MENUBOX_TITLE			84
#define CG_GREN_TIMER				85
#define	CG_ALERT_ICON				86
#define CG_GREN_ANALOGUE			87
#define CG_GREN_TIMER_DIGITS		88
#define CG_DRAWATTACKER				89

// Slothy
#define CG_ACTIVE_WEAPON_ICON		90
#define CG_AGENTDISGUISE_FULLINFO	91
#define CG_PLAYER_PIPES				92
#define	CG_TEAM_PIPES				93

#define CG_SYSTEM_SPEED				95
// end Slothy

#define CG_OBITUARIES_CONTENT		96

#define CG_PLAYER_AMMOSLOT_MEDIKIT_VALUE	97
#define CG_PLAYER_AMMOSLOT_CHARGE_VALUE	98

// need to keep in order
#define CG_FORTMENU_OPTION_1		101
#define CG_FORTMENU_OPTION_2		102
#define CG_FORTMENU_OPTION_3		103
#define CG_FORTMENU_OPTION_4		104
#define CG_FORTMENU_OPTION_5		105
#define CG_FORTMENU_OPTION_6		106
#define CG_FORTMENU_OPTION_7		107
#define CG_FORTMENU_OPTION_8		108
#define CG_FORTMENU_OPTION_9		109
#define CG_FORTMENU_OPTION_0		110

#define CG_FORTMENU_NUMBER_1		111
#define CG_FORTMENU_NUMBER_2		112
#define CG_FORTMENU_NUMBER_3		113
#define CG_FORTMENU_NUMBER_4		114
#define CG_FORTMENU_NUMBER_5		115
#define CG_FORTMENU_NUMBER_6		116
#define CG_FORTMENU_NUMBER_7		117
#define CG_FORTMENU_NUMBER_8		118
#define CG_FORTMENU_NUMBER_9		119
#define CG_FORTMENU_NUMBER_0		120

#define CG_FORTMENU_RIGHT_1		121
#define CG_FORTMENU_RIGHT_2		122
#define CG_FORTMENU_RIGHT_3		123
#define CG_FORTMENU_RIGHT_4		124
#define CG_FORTMENU_RIGHT_5		125
#define CG_FORTMENU_RIGHT_6		126
#define CG_FORTMENU_RIGHT_7		127
#define CG_FORTMENU_RIGHT_8		128
#define CG_FORTMENU_RIGHT_9		129
#define CG_FORTMENU_RIGHT_0		130

#define CG_FORT_MENU_BORDER		131
#define CG_FORT_MENU_TITLE		132

#define CG_SCOREBOARD_TITLE			133
#define CG_SCOREBOARD_TEAMSCORES	134

#define CG_HUDICONS				135
#define CG_LEVELSHOT			136
#define CG_LOADSTATUS			137
#define CG_LOADINFO				138
#define CG_MAPNAME				139
#define CG_MOTD					141
#define CG_MAPINFO				142
#define CG_LOADPROGRESS			143

#define CG_POWERUP_ICON			160
#define CG_CHARGE_ICON			161
#define CG_POWERUP_ICONREV		162
// djbob


#define UI_OWNERDRAW_BASE			200
#define UI_HANDICAP					200
#define UI_EFFECTS					201
//#define UI_GAMETYPE					202
#define UI_MAPPREVIEW				203
#define UI_NETSOURCE				204
#define UI_NETMAPPREVIEW			205
#define UI_ALLMAPS_SELECTION		206
#define UI_VOTE_KICK				207
#define UI_BOTNAME					208
#define UI_CROSSHAIR				209
#define UI_SELECTEDPLAYER			210
#define UI_MAPCINEMATIC				211
//#define UI_NETGAMETYPE				212
#define UI_NETMAPCINEMATIC			213
#define UI_SERVERREFRESHDATE		214
#define UI_SERVERMOTD				215
#define UI_GLINFO					216
#define UI_KEYBINDSTATUS			217
#define UI_JOINGAMETYPE				218
#define UI_PREVIEWCINEMATIC			219
#define UI_STARTMAPCINEMATIC		220
#define UI_MAPS_SELECTION			221
// RR2DO2
#define UI_FORT_BACKMODELS			222			// rotating models in the background of the FORT menu
#define UI_SERVERREFRESHPROGRESS	223
// RR2DO2
//djbob
#define UI_GAMEINDEX_MAPNAME		224
#define UI_CLASSCONFIG_ACTIVE		225
#define UI_CDKEY_OK					226

#define UI_SPINNING_DUCK			227

#define UI_FLAGBACK					229

// slothy
#define	UI_PBSTATUS					230
#define UI_HUD_PREVIEW				231
#define UI_CLASS_MODEL				232
#define UI_CLASS_TITLE				233

#define UI_WEAPON_PREVIEW			235

// Shuriken
#define CG_ARMORTYPE_ICON			250
#define CG_ARMORTYPE_INFO			251
#define CG_ARMORCLASS_ICON          252

#define UI_FAV_SERVER_0				1000
#define UI_FAV_SERVER_1				1001
#define UI_FAV_SERVER_2				1002
#define UI_FAV_SERVER_3				1003
#define UI_FAV_SERVER_4				1004
#define UI_FAV_SERVER_5				1005
#define UI_FAV_SERVER_6				1006
#define UI_FAV_SERVER_7				1007
#define UI_FAV_SERVER_8				1008
#define UI_FAV_SERVER_9				1009




#define HUD_CLASS_TITLE				2000
#define HUD_CLASS_MODEL				2001
#define HUD_CLASS_STAT_HEALTH		2002
#define HUD_CLASS_STAT_ARMOUR		2003
#define HUD_CLASS_STAT_SPEED		2004
#define HUD_CLASS_CHOOSE_BUTTON		2005
#define HUD_CLASS_INV				2006
#define HUD_CLASS_INFO				2007
#define HUD_CLASS_HEADMODEL			2008
#define HUD_CLASS_DISABLED			2009

#define HUD_MAP_BLURB				2010
#define HUD_MAP_LVLSHOT				2011
#define HUD_MAP_NAME				2012

#define HUD_VOTE_STRING				2013
#define HUD_VOTE_TALLY_YES			2014
#define HUD_VOTE_TALLY_NO			2015

#define HUD_TEAM_NAME_RED			2016
#define HUD_TEAM_NAME_BLUE			2017
#define HUD_TEAM_NAME_YELLOW		2018
#define HUD_TEAM_NAME_GREEN			2019

#define HUD_ENDGAME_TEAMSCORES		2020
#define HUD_CHAT					2021

#define HUD_MAPVOTE_NAME_0			2050
#define HUD_MAPVOTE_NAME_1			2051
#define HUD_MAPVOTE_NAME_2			2052
#define HUD_MAPVOTE_NAME_3			2053
#define HUD_MAPVOTE_NAME_4			2054
#define HUD_MAPVOTE_NAME_5			2055
#define HUD_MAPVOTE_NAME_6			2056
#define HUD_MAPVOTE_NAME_7			2057
#define HUD_MAPVOTE_NAME_8			2058
#define HUD_MAPVOTE_NAME_9			2059

#define HUD_MAPVOTE_TALLY_0			2060
#define HUD_MAPVOTE_TALLY_1			2061
#define HUD_MAPVOTE_TALLY_2			2062
#define HUD_MAPVOTE_TALLY_3			2063
#define HUD_MAPVOTE_TALLY_4			2064
#define HUD_MAPVOTE_TALLY_5			2065
#define HUD_MAPVOTE_TALLY_6			2066
#define HUD_MAPVOTE_TALLY_7			2067
#define HUD_MAPVOTE_TALLY_8			2068
#define HUD_MAPVOTE_TALLY_9			2069

#define HUD_MAPVOTE_BAR_0			2070
#define HUD_MAPVOTE_BAR_1			2071
#define HUD_MAPVOTE_BAR_2			2072
#define HUD_MAPVOTE_BAR_3			2073
#define HUD_MAPVOTE_BAR_4			2074
#define HUD_MAPVOTE_BAR_5			2075
#define HUD_MAPVOTE_BAR_6			2076
#define HUD_MAPVOTE_BAR_7			2077
#define HUD_MAPVOTE_BAR_8			2078
#define HUD_MAPVOTE_BAR_9			2079

#define	HUD_MAPVOTE_LEVELSHOT		2080
#define HUD_MAPVOTE_BLURB			2081

// slothy
#define HUD_CLASS_CHOOSE_BTNTEXT	2500
#define HUD_TEAM_CHOOSE_BUTTON		2501
#define HUD_TEAM_CHOOSE_TEXT		2502
#define HUD_CLASS_CHOOSE_BTNFLAT	2505
#define HUD_CLASS_CHOOSE_BTN		2506
#define HUD_FOLLOW_TEXT				2507
#define HUD_CHASE_TEXT				2508
#define HUD_MATCH_STRING			2509
#define HUD_FLAG_INFO				2510
// slothy

//djbob

/*#define VOICECHAT_GFORTLAG			"gFORTlag"				// command someone to get the flag
#define VOICECHAT_OFFENSE			"offense"				// command someone to go on offense
#define VOICECHAT_DEFEND			"defend"				// command someone to go on defense
#define VOICECHAT_DEFENDFLAG		"defendflag"			// command someone to defend the flag
#define VOICECHAT_PATROL			"patrol"				// command someone to go on patrol (roam)
#define VOICECHAT_CAMP				"camp"					// command someone to camp (we don't have sounds for this one)
#define VOICECHAT_FOLLOWME			"followme"				// command someone to follow you
#define VOICECHAT_RETURNFLAG		"returnflag"			// command someone to return our flag
#define VOICECHAT_FOLLOWFLAGCARRIER	"followflagcarrier"		// command someone to follow the flag carrier
#define VOICECHAT_YES				"yes"					// yes, affirmative, etc.
#define VOICECHAT_NO				"no"					// no, negative, etc.
#define VOICECHAT_ONGFORTLAG			"ongFORTlag"				// I'm getting the flag
#define VOICECHAT_ONOFFENSE			"onoffense"				// I'm on offense
#define VOICECHAT_ONDEFENSE			"ondefense"				// I'm on defense
#define VOICECHAT_ONPATROL			"onpatrol"				// I'm on patrol (roaming)
#define VOICECHAT_ONCAMPING			"oncamp"				// I'm camping somewhere
#define VOICECHAT_ONFOLLOW			"onfollow"				// I'm following
#define VOICECHAT_ONFOLLOWCARRIER	"onfollowcarrier"		// I'm following the flag carrier
#define VOICECHAT_ONRETURNFLAG		"onreturnflag"			// I'm returning our flag
#define VOICECHAT_INPOSITION		"inposition"			// I'm in position
#define VOICECHAT_IHAVEFLAG			"ihaveflag"				// I have the flag
#define VOICECHAT_BASEATTACK		"baseattack"			// the base is under attack
#define VOICECHAT_ENEMYHASFLAG		"enemyhasflag"			// the enemy has our flag (CTF)
#define VOICECHAT_STARTLEADER		"startleader"			// I'm the leader
#define VOICECHAT_STOPLEADER		"stopleader"			// I resign leadership
#define VOICECHAT_TRASH				"trash"					// lots of trash talk
#define VOICECHAT_WHOISLEADER		"whoisleader"			// who is the team leader
#define VOICECHAT_WANTONDEFENSE		"wantondefense"			// I want to be on defense
#define VOICECHAT_WANTONOFFENSE		"wantonoffense"			// I want to be on offense
#define VOICECHAT_KILLINSULT		"kill_insult"			// I just killed you
#define VOICECHAT_TAUNT				"taunt"					// I want to taunt you
#define VOICECHAT_DEATHINSULT		"death_insult"			// you just killed me
#define VOICECHAT_KILLGAUNTLET		"kill_gauntlet"			// I just killed you with the gauntlet
#define VOICECHAT_PRAISE			"praise"				// you did something good*/

// djbob
#define BINDING_TABLE_LOOK	0
#define BINDING_TABLE_MOVE	1
#define BINDING_TABLE_SHOOT	2
#define BINDING_TABLE_MISC	3
#define BINDING_TABLE_COMS	4

#define SYSTEM_TABLE_GRAPHICS	0
#define SYSTEM_TABLE_DISPLAY	1
#define SYSTEM_TABLE_SOUND		2
#define SYSTEM_TABLE_NETWORK	3

#define CONFIG_READ_NORMAL	0
#define CONFIG_READ_CLASS	1

#define ANCHOR_NONE				0
#define ANCHOR_AUTOSIZE_WIDTH	1
#define ANCHOR_AUTOSIZE_HEIGHT	1
#define ANCHOR_RIGHT			2
#define ANCHOR_BOTTOM			2
#define ANCHOR_CENTER			4
#define ANCHOR_MIDDLE			4

#define FORTCOLOR				.68 .04 .04

#define FORT_ITEM_BORDER(FORT_BRD_X, FORT_BRD_Y, FORT_BRD_WIDTH, FORT_BRD_HEIGHT) \
		FORT_GROUP_ITEM_BORDER(FORT_BRD_X, FORT_BRD_Y, FORT_BRD_WIDTH, FORT_BRD_HEIGHT, "")

#define FORT_GROUP_ITEM_BORDER(FORT_BRD_X, FORT_BRD_Y, FORT_BRD_WIDTH, FORT_BRD_HEIGHT, FORT_GROUP) \
		FORT_GROUP_NAMED_ITEM_BORDER(FORT_BRD_X, FORT_BRD_Y, FORT_BRD_WIDTH, FORT_BRD_HEIGHT, FORT_GROUP, "")

#define FORT_GROUP_NAMED_ITEM_BORDER(FORT_BRD_X, FORT_BRD_Y, FORT_BRD_WIDTH, FORT_BRD_HEIGHT, FORT_GROUP, FORT_NAME) \
		itemDef {											\
			visible 1										\
			rect FORT_BRD_X FORT_BRD_Y 8 8					\
			decoration										\
			background "ui/gfx/hud/black_top_left"			\
			group FORT_GROUP									\
			name FORT_NAME									\
			style WINDOW_STYLE_SHADER_ADJUST				\
		}													\
															\
		itemDef {											\
			visible 1										\
			rect $evalint( FORT_BRD_X + 8 ) FORT_BRD_Y FORT_BRD_WIDTH 8	\
			decoration										\
			background "ui/gfx/hud/black_fill_top"		\
			group FORT_GROUP									\
			name FORT_NAME									\
			style WINDOW_STYLE_SHADER_ADJUST				\
		}													\
															\
		itemDef {											\
			visible 1										\
			rect $evalint( FORT_BRD_X + 8 + FORT_BRD_WIDTH ) FORT_BRD_Y 8 8			\
			decoration										\
			background "ui/gfx/hud/black_top_right"		\
			group FORT_GROUP									\
			name FORT_NAME									\
			style WINDOW_STYLE_SHADER_ADJUST				\
		}													\
															\
		itemDef {											\
			visible 1										\
			rect $evalint( FORT_BRD_X + 8 + FORT_BRD_WIDTH ) $evalint( FORT_BRD_Y + 8 ) 8 FORT_BRD_HEIGHT	\
			decoration										\
			background "ui/gfx/hud/black_fill_right"	\
			group FORT_GROUP									\
			name FORT_NAME									\
			style WINDOW_STYLE_SHADER_ADJUST						\
		}													\
															\
		itemDef {											\
			visible 1										\
			rect $evalint( FORT_BRD_X + 8 + FORT_BRD_WIDTH ) $evalint( FORT_BRD_Y + 8 + FORT_BRD_HEIGHT ) 8 8			\
			decoration										\
			background "ui/gfx/hud/black_bot_right"		\
			group FORT_GROUP									\
			name FORT_NAME									\
			style WINDOW_STYLE_SHADER_ADJUST						\
		}													\
															\
		itemDef {											\
			visible 1										\
			rect $evalint( FORT_BRD_X + 8 ) $evalint( FORT_BRD_Y + 8 + FORT_BRD_HEIGHT ) FORT_BRD_WIDTH 8	\
			decoration										\
			background "ui/gfx/hud/black_fill_bot"		\
			group FORT_GROUP									\
			name FORT_NAME									\
			style WINDOW_STYLE_SHADER_ADJUST						\
		}													\
															\
		itemDef {											\
			visible 1										\
			rect FORT_BRD_X $evalint( FORT_BRD_Y + 8 + FORT_BRD_HEIGHT ) 8 8			\
			decoration										\
			background "ui/gfx/hud/black_bot_left"		\
			group FORT_GROUP									\
			name FORT_NAME									\
			style WINDOW_STYLE_SHADER_ADJUST						\
		}													\
															\
		itemDef {											\
			visible 1										\
			rect FORT_BRD_X $evalint( FORT_BRD_Y + 8 ) 8 FORT_BRD_HEIGHT	\
			decoration										\
			background "ui/gfx/hud/black_fill_left"		\
			group FORT_GROUP									\
			name FORT_NAME									\
			style WINDOW_STYLE_SHADER_ADJUST						\
		}


#define FORT_ITEM_BORDER_FILLED_NEW(FORT_BRD_X, FORT_BRD_Y, FORT_BRD_WIDTH, FORT_BRD_HEIGHT) \
		itemDef {											\
			visible 1										\
			rect FORT_BRD_X FORT_BRD_Y 8 8					\
			decoration										\
			background "ui/gfx/hud/top_left"			\
			style WINDOW_STYLE_SHADER_ADJUST						\
		}													\
															\
		itemDef {											\
			visible 1										\
			rect $evalint( FORT_BRD_X + 8 ) FORT_BRD_Y FORT_BRD_WIDTH 8	\
			decoration										\
			background "ui/gfx/hud/fill_top"			\
			style WINDOW_STYLE_SHADER_ADJUST						\
		}													\
															\
		itemDef {											\
			visible 1										\
			rect $evalint( FORT_BRD_X + 8 + FORT_BRD_WIDTH ) FORT_BRD_Y 8 8			\
			decoration										\
			background "ui/gfx/hud/top_right"			\
			style WINDOW_STYLE_SHADER_ADJUST						\
		}													\
															\
		itemDef {											\
			visible 1										\
			rect $evalint( FORT_BRD_X + 8 + FORT_BRD_WIDTH ) $evalint( FORT_BRD_Y + 8 ) 8 FORT_BRD_HEIGHT	\
			decoration										\
			background "ui/gfx/hud/fill_right"	\
			style WINDOW_STYLE_SHADER_ADJUST						\
		}													\
															\
		itemDef {											\
			visible 1										\
			rect $evalint( FORT_BRD_X + 8 + FORT_BRD_WIDTH ) $evalint( FORT_BRD_Y + 8 + FORT_BRD_HEIGHT ) 8 8			\
			decoration										\
			background "ui/gfx/hud/bot_right"			\
			style WINDOW_STYLE_SHADER_ADJUST						\
		}													\
															\
		itemDef {											\
			visible 1										\
			rect $evalint( FORT_BRD_X + 8 ) $evalint( FORT_BRD_Y + 8 + FORT_BRD_HEIGHT ) FORT_BRD_WIDTH 8	\
			decoration										\
			background "ui/gfx/hud/fill_bot"			\
			style WINDOW_STYLE_SHADER_ADJUST						\
		}													\
															\
		itemDef {											\
			visible 1										\
			rect FORT_BRD_X $evalint( FORT_BRD_Y + 8 + FORT_BRD_HEIGHT ) 8 8			\
			decoration										\
			background "ui/gfx/hud/bot_left"			\
			style WINDOW_STYLE_SHADER_ADJUST						\
		}													\
															\
		itemDef {											\
			visible 1										\
			rect FORT_BRD_X $evalint( FORT_BRD_Y + 8 ) 8 FORT_BRD_HEIGHT	\
			decoration										\
			background "ui/gfx/hud/fill_left"			\
			style WINDOW_STYLE_SHADER_ADJUST						\
		}													\
															\
  		itemDef {															\
			visible 1														\
			rect $evalint( FORT_BRD_X + 8 ) $evalint( FORT_BRD_Y + 8 ) FORT_BRD_WIDTH FORT_BRD_HEIGHT	\
			decoration														\
			background "ui/gfx/hud/fill_center_grey"					\
			style WINDOW_STYLE_SHADER_ADJUST										\
		}

#define FORT_MENU_ITEM_OPTION(Y_COORD, NUMBER)									\
		itemDef {																\
			visible MENU_TRUE													\
			rect 48 Y_COORD 200 20												\
			decoration															\
			ownerDraw $evalint( CG_FORTMENU_OPTION_1 + NUMBER )					\
			textscale .27														\
			textstyle ITEM_TEXTSTYLE_SHADOWED									\
			forecolor 1 1 1 1													\
		}

#define FORT_MENU_ITEM_NUMBER(Y_COORD, NUMBER)						\
		itemDef {													\
			visible MENU_TRUE										\
			rect 24 Y_COORD 20 20									\
			decoration												\
			textscale .27											\
			textstyle ITEM_TEXTSTYLE_SHADOWED						\
			textalign ITEM_ALIGN_CENTER								\
			ownerDraw $evalint( CG_FORTMENU_NUMBER_1 + NUMBER )		\
			forecolor 1 1 1 1										\
		}

#define FORT_MENU_ITEM_RIGHT(Y_COORD, NUMBER)					\
		itemDef {												\
			visible MENU_TRUE									\
			rect 168 Y_COORD 20 20								\
			decoration											\
			ownerDraw $evalint( CG_FORTMENU_RIGHT_1 + NUMBER )	\
			textalign ITEM_ALIGN_CENTER							\
			textscale .27										\
			textstyle ITEM_TEXTSTYLE_SHADOWED					\
			forecolor 1 1 1 1									\
		}










#define FORT_BUTTON(FORT_BTNNAME, FORT_BTNTEXT, FORT_BTN_X, FORT_BTN_Y, FORT_BTN_WIDTH, FORT_GROUP, FORT_BTN_ACTIONS) \
		itemDef {																		\
			name FORT_BTNNAME															\
			visible 1																	\
			rect FORT_BTN_X FORT_BTN_Y 16 32												\
			background		"ui/gfx/named_button_left"									\
			altbackground	"ui/gfx/named_button_left_focus"							\
			style WINDOW_STYLE_SHADER_ADJUST													\
			group FORT_GROUP																\
			type		ITEM_TYPE_BUTTON												\
			noPulseOnFocus																\
			mouseEnter	{ setitembackground FORT_BTNNAME alt }							\
			mouseExit	{ setitembackground FORT_BTNNAME primary }						\
			action		{ play "ui/sound/menu4.wav"; FORT_BTN_ACTIONS }					\
		}																				\
		itemDef {																		\
			name FORT_BTNNAME															\
			visible 1																	\
			rect $evalint( FORT_BTN_X + FORT_BTN_WIDTH - 16 ) FORT_BTN_Y 16 32				\
			background		"ui/gfx/named_button_right"									\
			altbackground	"ui/gfx/named_button_right_focus"							\
			style WINDOW_STYLE_SHADER_ADJUST													\
			group FORT_GROUP																\
			type		ITEM_TYPE_BUTTON												\
			noPulseOnFocus																\
			mouseEnter	{ setitembackground FORT_BTNNAME alt }							\
			mouseExit	{ setitembackground FORT_BTNNAME primary }						\
			action		{ play "ui/sound/menu4.wav"; FORT_BTN_ACTIONS }					\
		}																				\
		itemDef {																		\
			visible 1																	\
			rect $evalint( FORT_BTN_X + 16 ) FORT_BTN_Y $evalint( FORT_BTN_WIDTH - 32 ) 32	\
			name FORT_BTNNAME															\
			text FORT_BTNTEXT															\
			textalignx $evalint( ( FORT_BTN_WIDTH - 32 ) / 2 )							\
			textaligny 21																\
			textscale .27																\
			textalign	ITEM_ALIGN_CENTER												\
			type		ITEM_TYPE_BUTTON												\
			background		"ui/gfx/named_button_center"								\
			altbackground	"ui/gfx/named_button_center_focus"							\
			style WINDOW_STYLE_SHADER_ADJUST													\
			noPulseOnFocus																\
			useAssetFont																\
			mouseEnter	{ setitembackground FORT_BTNNAME alt }							\
			mouseExit	{ setitembackground FORT_BTNNAME primary }						\
			action		{ play "ui/sound/menu4.wav"; FORT_BTN_ACTIONS }					\
			group FORT_GROUP																\
		}

#define FORT_BUTTON_SPECIAL(FORT_BTNNAME, FORT_BTNTEXT, FORT_BTN_X, FORT_BTN_Y, FORT_BTN_WIDTH, FORT_GROUP, FORT_BTN_ACTIONS, FORT_EXTRASTUFF) \
		itemDef {																		\
			name FORT_BTNNAME															\
			group FORT_GROUP																\
			visible 1																	\
			rect FORT_BTN_X FORT_BTN_Y 16 32												\
			background		"ui/gfx/named_button_left"									\
			altbackground	"ui/gfx/named_button_left_focus"							\
			style WINDOW_STYLE_SHADER_ADJUST													\
			type		ITEM_TYPE_BUTTON												\
			noPulseOnFocus																\
			mouseEnter	{ setitembackground FORT_BTNNAME alt }							\
			mouseExit	{ setitembackground FORT_BTNNAME primary }						\
			action		{ play "ui/sound/menu4.wav"; FORT_BTN_ACTIONS }					\
			FORT_EXTRASTUFF																\
		}																				\
		itemDef {																		\
			visible 1																	\
			rect $evalint( FORT_BTN_X + 16 ) FORT_BTN_Y $evalint( FORT_BTN_WIDTH - 32 ) 32	\
			name FORT_BTNNAME															\
			group FORT_GROUP																\
			text FORT_BTNTEXT															\
			textalignx $evalint( ( FORT_BTN_WIDTH - 32 ) / 2 )							\
			textaligny 21																\
			textscale .27																\
			textalign	ITEM_ALIGN_CENTER												\
			type		ITEM_TYPE_BUTTON												\
			background		"ui/gfx/named_button_center"								\
			altbackground	"ui/gfx/named_button_center_focus"							\
			style WINDOW_STYLE_SHADER_ADJUST													\
			noPulseOnFocus																\
			useAssetFont																\
			FORT_EXTRASTUFF																\
			mouseEnter	{ setitembackground FORT_BTNNAME alt }							\
			mouseExit	{ setitembackground FORT_BTNNAME primary }						\
			action		{ play "ui/sound/menu4.wav"; FORT_BTN_ACTIONS }					\
		}																				\
		itemDef {																		\
			name FORT_BTNNAME															\
			group FORT_GROUP																\
			visible 1																	\
			rect $evalint( FORT_BTN_X + FORT_BTN_WIDTH - 16 ) FORT_BTN_Y 16 32				\
			background		"ui/gfx/named_button_right"									\
			altbackground	"ui/gfx/named_button_right_focus"							\
			style WINDOW_STYLE_SHADER_ADJUST													\
			type		ITEM_TYPE_BUTTON												\
			noPulseOnFocus																\
			mouseEnter	{ setitembackground FORT_BTNNAME alt }							\
			mouseExit	{ setitembackground FORT_BTNNAME primary }						\
			action		{ play "ui/sound/menu4.wav"; FORT_BTN_ACTIONS }					\
			FORT_EXTRASTUFF																\
		}

#define FORT_BUTTONTT(FORT_BTNNAME, FORT_BTNTEXT, FORT_TEXTY_OFFSET, FORT_BTN_X, FORT_BTN_Y, FORT_BTN_WIDTH, FORT_BTN_HEIGHT, FORT_BTN_TEXTSIZE, FORT_GROUP, FORT_BTN_ACTIONS, FORT_BTN_TOOLTIP) \
		itemDef {																		\
			name FORT_BTNNAME															\
			group FORT_GROUP															\
			visible 1																	\
			rect FORT_BTN_X FORT_BTN_Y 16 FORT_BTN_HEIGHT								\
			background		"ui/gfx/button_left"									\
			altbackground	"ui/gfx/button_left_hi"							\
			style WINDOW_STYLE_SHADER_ADJUST											\
			type		ITEM_TYPE_BUTTON												\
			tooltip		FORT_BTN_TOOLTIP												\
			noPulseOnFocus																\
			mouseEnter	{ setitembackground FORT_BTNNAME alt }							\
			mouseExit	{ setitembackground FORT_BTNNAME primary }						\
			action		{ play "ui/sound/menu4.wav"; FORT_BTN_ACTIONS }					\
		}																				\
		itemDef {																		\
			name FORT_BTNNAME															\
			group FORT_GROUP															\
			visible 1																	\
			rect $evalint( FORT_BTN_X + FORT_BTN_WIDTH - 16 ) FORT_BTN_Y 16 FORT_BTN_HEIGHT				\
			background		"ui/gfx/button_right"									\
			altbackground	"ui/gfx/button_right_hi"							\
			style WINDOW_STYLE_SHADER_ADJUST											\
			type		ITEM_TYPE_BUTTON												\
			tooltip		FORT_BTN_TOOLTIP												\
			noPulseOnFocus																\
			mouseEnter	{ setitembackground FORT_BTNNAME alt }							\
			mouseExit	{ setitembackground FORT_BTNNAME primary }						\
			action		{ play "ui/sound/menu4.wav"; FORT_BTN_ACTIONS }					\
		}																				\
		itemDef {																		\
			visible 1																	\
			rect $evalint( FORT_BTN_X + 15 ) FORT_BTN_Y $evalint( FORT_BTN_WIDTH - 30 ) FORT_BTN_HEIGHT	\
			name FORT_BTNNAME															\
			group FORT_GROUP															\
			text FORT_BTNTEXT															\
			textalignx $evalint( ( FORT_BTN_WIDTH - 32 ) / 2 )							\
			textaligny FORT_TEXTY_OFFSET												\
			textscale FORT_BTN_TEXTSIZE													\
			textalign	ITEM_ALIGN_CENTER												\
			type		ITEM_TYPE_BUTTON												\
			background		"ui/gfx/button_center"								\
			altbackground	"ui/gfx/button_center_hi"							\
			style WINDOW_STYLE_SHADER_ADJUST											\
			noPulseOnFocus																\
			tooltip		FORT_BTN_TOOLTIP												\
			useAssetFont																\
			mouseEnter	{ setitembackground FORT_BTNNAME alt }							\
			mouseExit	{ setitembackground FORT_BTNNAME primary }						\
			action		{ play "ui/sound/menu4.wav"; FORT_BTN_ACTIONS }					\
		}

#define FORT_BUTTON_SPECIALTT(FORT_BTNNAME, FORT_BTNTEXT, FORT_TEXTY_OFFSET, FORT_BTN_X, FORT_BTN_Y, FORT_BTN_WIDTH, FORT_BTN_HEIGHT, FORT_BTN_TEXTSIZE, FORT_GROUP, FORT_BTN_ACTIONS, FORT_EXTRASTUFF, FORT_BTN_TOOLTIP) \
		itemDef {																		\
			name FORT_BTNNAME															\
			group		FORT_GROUP														\
			visible 1																	\
			rect FORT_BTN_X FORT_BTN_Y 16 FORT_BTN_HEIGHT												\
			background		"ui/gfx/button_left"									\
			altbackground	"ui/gfx/button_left_hi"							\
			style WINDOW_STYLE_SHADER_ADJUST													\
			group FORT_GROUP																\
			type		ITEM_TYPE_BUTTON												\
			noPulseOnFocus																\
			mouseEnter	{ setitembackground FORT_BTNNAME alt }							\
			mouseExit	{ setitembackground FORT_BTNNAME primary }						\
			action		{ play "ui/sound/menu4.wav"; FORT_BTN_ACTIONS }					\
			FORT_EXTRASTUFF																\
		}																				\
		itemDef {																		\
			visible 1																	\
			rect $evalint( FORT_BTN_X + 16 ) FORT_BTN_Y $evalint( FORT_BTN_WIDTH - 32 ) FORT_BTN_HEIGHT	\
			name FORT_BTNNAME															\
			group		FORT_GROUP														\
			text FORT_BTNTEXT															\
			textalignx $evalint( ( FORT_BTN_WIDTH - 32 ) / 2 )							\
			textaligny FORT_TEXTY_OFFSET																\
			textscale FORT_BTN_TEXTSIZE																\
			textalign	ITEM_ALIGN_CENTER												\
			type		ITEM_TYPE_BUTTON												\
			background		"ui/gfx/button_center"								\
			altbackground	"ui/gfx/button_center_hi"							\
			style WINDOW_STYLE_SHADER_ADJUST													\
			tooltip		FORT_BTN_TOOLTIP													\
			noPulseOnFocus																\
			useAssetFont																\
			FORT_EXTRASTUFF																\
			mouseEnter	{ setitembackground FORT_BTNNAME alt }							\
			mouseExit	{ setitembackground FORT_BTNNAME primary }						\
			action		{ play "ui/sound/menu4.wav"; FORT_BTN_ACTIONS }					\
			group FORT_GROUP																\
		}																				\
		itemDef {																		\
			name FORT_BTNNAME															\
			group		FORT_GROUP														\
			visible 1																	\
			rect $evalint( FORT_BTN_X + FORT_BTN_WIDTH - 16 ) FORT_BTN_Y 16 FORT_BTN_HEIGHT				\
			background		"ui/gfx/button_right"									\
			altbackground	"ui/gfx/button_right_hi"							\
			style WINDOW_STYLE_SHADER_ADJUST													\
			group FORT_GROUP																\
			type		ITEM_TYPE_BUTTON												\
			noPulseOnFocus																\
			mouseEnter	{ setitembackground FORT_BTNNAME alt }							\
			mouseExit	{ setitembackground FORT_BTNNAME primary }						\
			action		{ play "ui/sound/menu4.wav"; FORT_BTN_ACTIONS }					\
			FORT_EXTRASTUFF																\
		}

#define FORT_BUTTONDUMMY(FORT_GROUP, FORT_BTNNAME, FORT_BTN_X, FORT_BTN_Y, FORT_BTN_WIDTH, FORT_BTN_HEIGHT)			\
		itemDef {																		\
			group		FORT_GROUP														\
			visible 1																	\
			rect FORT_BTN_X FORT_BTN_Y 16 FORT_BTN_HEIGHT									\
			background		"ui/gfx/button_left"									\
			altbackground	"ui/gfx/button_left_hi"							\
			style WINDOW_STYLE_SHADER_ADJUST											\
			noPulseOnFocus																\
		}																				\
		itemDef {																		\
			group		FORT_GROUP														\
			visible 1																	\
			rect $evalint( FORT_BTN_X + 16 ) FORT_BTN_Y $evalint( FORT_BTN_WIDTH - 32 ) FORT_BTN_HEIGHT	\
			background		"ui/gfx/button_center"								\
			altbackground	"ui/gfx/button_center_hi"							\
			style WINDOW_STYLE_SHADER_ADJUST											\
			noPulseOnFocus																\
		}																				\
		itemDef {																		\
			group		FORT_GROUP														\
			visible 1																	\
			rect $evalint( FORT_BTN_X + FORT_BTN_WIDTH - 16 ) FORT_BTN_Y 16 FORT_BTN_HEIGHT	\
			background		"ui/gfx/button_right"									\
			altbackground	"ui/gfx/button_right_hi"							\
			style WINDOW_STYLE_SHADER_ADJUST											\
			noPulseOnFocus																\
		}

#define FORT_GROUPBOX(FORT_SUBTEXT, FORT_GROUP, FORT_WND_X, FORT_WND_Y, FORT_WND_WIDTH, FORT_WND_HEIGHT, FORT_WND_BORDERCOLOR, FORT_WND_SUBCOLOR, FORT_WND_FORECOLOR, FORT_BACKCOLOR, FORT_BACKSTYLE) \
		itemDef {																		\
			group		FORT_GROUP														\
			rect		FORT_WND_X FORT_WND_Y FORT_WND_WIDTH FORT_WND_HEIGHT				\
			style		FORT_BACKSTYLE													\
			backcolor	FORT_BACKCOLOR													\
			border		WINDOW_BORDER_FULL												\
			bordercolor	FORT_WND_BORDERCOLOR												\
			visible		1																\
			decoration																	\
		}																				\
		itemDef {																		\
			group		FORT_GROUP														\
			rect		$evalint( FORT_WND_X + 2 ) $evalint( FORT_WND_Y + 2) $evalint( FORT_WND_WIDTH - 2) 12		\
			text		FORT_SUBTEXT													\
			textscale	.19																\
			textalignx	3																\
			textaligny	10																\
			style		WINDOW_STYLE_FILLED												\
			backcolor	FORT_WND_SUBCOLOR												\
			forecolor	FORT_WND_FORECOLOR												\
			visible		1																\
			decoration																	\
		}
