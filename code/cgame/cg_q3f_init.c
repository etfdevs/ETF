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
**	cg_q3f_init.c
**
**	Replacement map initialization code.
*/

#include "cg_local.h"
#include "../game/bg_q3f_weapon.h"
#include "cg_q3f_scriptlib.h"
#include "cg_q3f_grenades.h"

//Keeg
extern qboolean initTrails;
void CG_ClearTrails (void);

/******************************************************************************
*****	Init functions used internally
****/

static int CG_Q3F_LoadMapExt( char *newext, int *fhandle )
{
	// Works out and opens a 'companion' file for the map

	char buff[512];
	char *src, *dest, *lastExt, *maxdest;

	for(	src = cgs.mapname, lastExt = NULL, dest = buff, maxdest = &buff[511];
			dest < maxdest && *src; )
	{
		if( *src == '.' )
			lastExt = dest;
		*dest++ = *src++;
	}

		// Can't append the extension
	if( lastExt == NULL ||
		strlen( newext ) + lastExt >= maxdest )
		return( -1 );

	for( dest = lastExt + 1; *newext; )
		*dest++ = *newext++;

	return( trap_FS_FOpenFile( buff, fhandle, FS_READ ) );
}

void CG_Q3F_SetInitPhase( int newPhase )
{
	// Reset the init fields.

	cgs.initPhase	= newPhase;
	cgs.initIndex	= 0;
	cg.infoScreenText[0] = 0;

	if( cgs.initFHandle )
	{
		trap_FS_FCloseFile( cgs.initFHandle );
		cgs.initFHandle = 0;
	}
}

void CG_Q3F_InitLog( const char *prefix, const char *entry, const char *suffix )
{
	// Add an entry to the init log.

	char time[16];
	int passedtime;
	char *src, *dest, *max;
#ifdef _DEBUG
	int fhandle;
#endif
	dest = cgs.teamChatMsgs[cgs.teamChatPos];
	max = dest + TEAMCHAT_BYTEWIDTH;

	passedtime = trap_Milliseconds() - cgs.initTime;
	Com_sprintf( time, sizeof(time), "%3i:%03i ", passedtime / 1000, passedtime % 1000 );

	for( src = (char *) time; src && dest < max && *src; )
		*dest++ = *src++;
	for( src = (char *) prefix; src && dest < max && *src; )
		*dest++ = *src++;
	for( src = (char *) entry; src && dest < max && *src; )
		*dest++ = *src++;
	for( src = (char *) suffix; src && dest < max && *src; )
		*dest++ = *src++;
	*dest = 0;

#ifdef _DEBUG
	if( trap_FS_FOpenFile( "init.log", &fhandle, FS_APPEND ) >= 0 )
	{
		src = cgs.teamChatMsgs[cgs.teamChatPos];
		trap_FS_Write( src, dest - src, fhandle );
		trap_FS_Write( "\r\n", 2, fhandle );
		trap_FS_FCloseFile( fhandle );
	}
#endif
		// Bump the 'start and end' positions.
	cgs.teamLastChatPos = (cgs.teamLastChatPos + 1) % TEAMCHAT_HEIGHT;
	if( cgs.teamLastChatPos == cgs.teamChatPos )
		cgs.teamChatPos = (cgs.teamChatPos + 1) % TEAMCHAT_HEIGHT;
}

extern displayContextDef_t cgDC;
qboolean BG_LoadMapInfoFromFile( char *filename, displayContextDef_t* DC, mapInfo* miList, int* index );

void CG_Q3F_LoadingMapInfo(void)
{
	char buffer[128];
	int dummy = 0;

	if( cgs.mapInfoLoaded) {
		return;
	}

	COM_StripExtension( COM_SkipPath( cgs.mapname ), buffer, sizeof(buffer) );

	cgs.mapInfoLoaded = BG_LoadMapInfoFromFile( va("%s/%s%s", MAPINFODIR, buffer, MAPINFOEXT), &cgDC, &cgs.mapinfo, &dummy);
}

void CG_Q3F_RenderLoadingScreen(void) {
	menuDef_t*	menu;

	menu = Menus_FindByName("infoscreen");
	if(!menu) {
		return;
	}

	Menu_Paint(menu, qtrue);

	if( cg.demoPlayback || cg_classicinit.integer ) {
		// trap_UpdateScreen() calls DrawActiveFrame again, we shortcut
		// it so it doesn't cause an infinite loop.
		
		cgs.initDemoFrameRender = qtrue;
		trap_UpdateScreen();
		cgs.initDemoFrameRender = qfalse;
	}
	cgs.initScreenRendered = qtrue;
}


/******************************************************************************
*****	Data blocks.
****/

typedef struct {
	sfxHandle_t *storage;
	const char *name;
} initSound_t;

static initSound_t initSounds[] = {
	{ &cgs.media.oneMinuteSound,			"sound/feedback/1_minute.wav"				},
	{ &cgs.media.fiveMinuteSound,			"sound/feedback/5_minute.wav"				},
	{ &cgs.media.suddenDeathSound,			"sound/feedback/sudden_death.wav"			},
/*	{ &cgs.media.oneFragSound,				"sound/feedback/1_frag.wav"					},
	{ &cgs.media.twoFragSound,				"sound/feedback/2_frags.wav"				},
	{ &cgs.media.threeFragSound,			"sound/feedback/3_frags.wav"				},*/
	{ &cgs.media.count3Sound,				"sound/feedback/three.wav"					},
	{ &cgs.media.count2Sound,				"sound/feedback/two.wav"					},
	{ &cgs.media.count1Sound,				"sound/feedback/one.wav"					},
	{ &cgs.media.countFightSound,			"sound/feedback/fight.wav"					},
	{ &cgs.media.countPrepareSound,			"sound/feedback/prepare.wav"				},
	{ &cgs.media.hitTeamSound,				"sound/feedback/hit_teammate.wav"			},
	{ &cgs.media.tracerSound,				"sound/weapons/machinegun/buletby1.wav"		},
	{ &cgs.media.selectSound,				"sound/weapons/foley/change.wav"			},
	{ &cgs.media.wearOffSound,				"sound/items/wearoff.wav"					},
	{ &cgs.media.useNothingSound,			"sound/items/use_nothing.wav"				},
	{ &cgs.media.gasSmokeSound,				"sound/weapons/grenade/gren_smoke.wav"		},
	{ &cgs.media.gibSound,					"sound/player/gibsplt1.wav"					},
	{ &cgs.media.gibBounce1Sound,			"sound/player/gibimp1.wav"					},
	{ &cgs.media.gibBounce2Sound,			"sound/player/gibimp2.wav"					},
	{ &cgs.media.gibBounce3Sound,			"sound/player/gibimp3.wav"					},
	{ &cgs.media.brassBounce1Sound,			"sound/weapons/foley/brass1.wav"			},
	{ &cgs.media.brassBounce2Sound,			"sound/weapons/foley/brass2.wav"			},
	{ &cgs.media.brassBounce3Sound,			"sound/weapons/foley/brass3.wav"			},
	{ &cgs.media.teleInSound,				"sound/world/telein.wav"					},
	{ &cgs.media.teleOutSound,				"sound/world/teleout.wav"					},
	{ &cgs.media.respawnSound,				"sound/items/respawn1.wav"					},
	{ &cgs.media.noAmmoSound,				"sound/weapons/foley/noammo.wav"			},
	{ &cgs.media.talkSound,					"sound/player/talk.wav"						},
	{ &cgs.media.landSound,					"sound/player/land1.wav"					},
	{ &cgs.media.hitSound,					"sound/feedback/hit.wav"					},
	{ &cgs.media.excellentSound,			"sound/feedback/excellent.wav"				},
	{ &cgs.media.deniedSound,				"sound/feedback/denied.wav"					},
	{ &cgs.media.humiliationSound,			"sound/feedback/humiliation.wav"			},
//	{ &cgs.media.takenLeadSound,			"sound/feedback/takenlead.wav"				},
//	{ &cgs.media.tiedLeadSound,				"sound/feedback/tiedlead.wav"				},
//	{ &cgs.media.lostLeadSound,				"sound/feedback/lostlead.wav"				},
	{ &cgs.media.watrInSound,				"sound/player/watr_in.wav"					},
	{ &cgs.media.watrOutSound,				"sound/player/watr_out.wav"					},
	{ &cgs.media.watrUnSound,				"sound/player/watr_un.wav"					},
	{ &cgs.media.jumpPadSound,				"sound/world/jumppad.wav"					},
	{ &cgs.media.flightSound,				"sound/items/flight.wav"					},
	{ &cgs.media.medkitSound,				"sound/items/use_medkit.wav"				},
	{ &cgs.media.quadSound,					"sound/items/damage3.wav"					},
	{ &cgs.media.sfx_ric1,					"sound/weapons/impact/impact_stone1.wav"	},
	{ &cgs.media.sfx_ric2,					"sound/weapons/impact/impact_stone1.wav"	},
	{ &cgs.media.sfx_ric3,					"sound/weapons/impact/impact_stone1.wav"	},
	{ &cgs.media.sfx_rockexp,				"sound/weapons/rocket/rocket_hit.wav"		},
//	{ &cgs.media.sfx_plasmaexp,				"sound/weapons/plasma/plasmx1a.wav"			},
	{ &cgs.media.sfx_pulseexp,				"sound/weapons/explosive/gren_pulse.wav"	},
	{ &cgs.media.sfx_railhit,				"sound/weapons/impact/impact_railgun.wav"	},
	{ &cgs.media.sfx_minigun_windup,		"sound/weapons/minigun/minigun_windup.wav"	},
	{ &cgs.media.sfx_minigun_winddown,		"sound/weapons/minigun/minigun_winddown.wav"},
	{ &cgs.media.sfx_minigun_loop,			"sound/weapons/minigun/minigun_spin.wav"	},
	//{ &cgs.media.sfx_minigun_fire,			"sound/weapons/minigun/minigun_fire.wav"	},
	{ &cgs.media.sfx_flamethrower_windup,	"sound/weapons/flamer/flamer_start.wav"		},
	{ &cgs.media.sfx_flamethrower_winddown,	"sound/weapons/flamer/flamer_stop.wav"		},
	{ &cgs.media.sfx_flamethrower_fire,		"sound/weapons/flamer/flamer_burn.wav"		},
	{ &cgs.media.sfx_flamethrower_firewater,"sound/world/underwater02.wav"				},
	{ &cgs.media.chargebeep1,				"sound/weapons/explosive/he_charge_set.wav"	},
	{ &cgs.media.chargebeep2,				"sound/weapons/explosive/he_charge_beep.wav"},
	{ &cgs.media.sfx_chargeexp,				"sound/weapons/explosive/he_charge.wav"		},
	{ &cgs.media.discardSound,				"sound/weapons/foley/backpack.wav"			},
	{ &cgs.media.cockSound,					"sound/weapons/foley/cock_shell.wav"		},
	{ &cgs.media.cockGrenSound,				"sound/weapons/foley/cock_rocket.wav"		},
	{ &cgs.media.regenSound,				"sound/items/regen.wav"						},
	{ &cgs.media.protectSound,				"sound/items/protect3.wav"					},
	{ &cgs.media.n_healthSound,				"sound/items/n_health.wav"					},
	{ &cgs.media.hgrenb1aSound,				"sound/weapons/explosive/gren_bounce1.wav"	},
	{ &cgs.media.hgrenb2aSound,				"sound/weapons/explosive/gren_bounce2.wav"	},

	{ NULL,									"" },
};

typedef struct {
	qhandle_t *storage;
	const char *name;
	int type;
} initGraphic_t;

#define	INITRT_SHADER		0
#define	INITRT_SHADERNOMIP	1
#define	INITRT_MODEL		2

static initGraphic_t initGraphics[] = {
	{	&cgs.media.numberShaders[0],		"gfx/2d/numbers/zero_32b",				INITRT_SHADERNOMIP	},
	{	&cgs.media.numberShaders[1],		"gfx/2d/numbers/one_32b",				INITRT_SHADERNOMIP	},
	{	&cgs.media.numberShaders[2],		"gfx/2d/numbers/two_32b",				INITRT_SHADERNOMIP	},
	{	&cgs.media.numberShaders[3],		"gfx/2d/numbers/three_32b",				INITRT_SHADERNOMIP	},
	{	&cgs.media.numberShaders[4],		"gfx/2d/numbers/four_32b",				INITRT_SHADERNOMIP	},
	{	&cgs.media.numberShaders[5],		"gfx/2d/numbers/five_32b",				INITRT_SHADERNOMIP	},
	{	&cgs.media.numberShaders[6],		"gfx/2d/numbers/six_32b",				INITRT_SHADERNOMIP	},
	{	&cgs.media.numberShaders[7],		"gfx/2d/numbers/seven_32b",				INITRT_SHADERNOMIP	},
	{	&cgs.media.numberShaders[8],		"gfx/2d/numbers/eight_32b",				INITRT_SHADERNOMIP	},
	{	&cgs.media.numberShaders[9],		"gfx/2d/numbers/nine_32b",				INITRT_SHADERNOMIP	},
	{	&cgs.media.numberShaders[10],		"gfx/2d/numbers/minus_32b",				INITRT_SHADERNOMIP	},
/*	{	&cgs.media.botSkillShaders[0],		"menu/art/skill1.tga",					INITRT_SHADER		},
	{	&cgs.media.botSkillShaders[1],		"menu/art/skill2.tga",					INITRT_SHADER		},
	{	&cgs.media.botSkillShaders[2],		"menu/art/skill3.tga",					INITRT_SHADER		},
	{	&cgs.media.botSkillShaders[3],		"menu/art/skill4.tga",					INITRT_SHADER		},
	{	&cgs.media.botSkillShaders[4],		"menu/art/skill5.tga",					INITRT_SHADER		},*/
//	{	&cgs.media.viewBloodShader,			"viewBloodBlend",						INITRT_SHADER		},
//	{	&cgs.media.deferShader,				"gfx/2d/defer.tga",						INITRT_SHADERNOMIP	},
/*	{	&cgs.media.scoreboardName,			"menu/tab/name.tga",					INITRT_SHADERNOMIP	},
	{	&cgs.media.scoreboardPing,			"menu/tab/ping.tga",					INITRT_SHADERNOMIP	},
	{	&cgs.media.scoreboardScore,			"menu/tab/score.tga",					INITRT_SHADERNOMIP	},
	{	&cgs.media.scoreboardTime,			"menu/tab/time.tga",					INITRT_SHADERNOMIP	},*/
	{	&cgs.media.smokePuffShader,			"smokePuff",							INITRT_SHADER		},
	{	&cgs.media.shotgunSmokePuffShader,	"shotgunSmokePuff",						INITRT_SHADER		},
	{	&cgs.media.flameShader,				"rocketExplosion",						INITRT_SHADER		},
	{	&cgs.media.bloodTrailShader,		"etf_bloodTrail",							INITRT_SHADER		},
	{	&cgs.media.lagometerShader,			"gfx/hud/hud_lagometer",				INITRT_SHADER		},
	{	&cgs.media.lagometermaskShader,		"gfx/hud/hud_lagometermask",			INITRT_SHADER		},
	{	&cgs.media.connectionShader,		"disconnected",							INITRT_SHADER		},
	{	&cgs.media.waterBubbleShader,		"waterBubble",							INITRT_SHADER		},
	{	&cgs.media.tracerShader,			"gfx/misc/tracer",						INITRT_SHADER		},
	{	&cgs.media.tracer2Shader,			"gfx/misc/tracer2",						INITRT_SHADER		},
//	{	&cgs.media.selectShader,			"gfx/2d/select",						INITRT_SHADER		},
/*	{	&cgs.media.crosshairShader[0],		"gfx/2d/crosshaira",					INITRT_SHADER		},
	{	&cgs.media.crosshairShader[1],		"gfx/2d/crosshairb",					INITRT_SHADER		},
	{	&cgs.media.crosshairShader[2],		"gfx/2d/crosshairc",					INITRT_SHADER		},
	{	&cgs.media.crosshairShader[3],		"gfx/2d/crosshaird",					INITRT_SHADER		},
	{	&cgs.media.crosshairShader[4],		"gfx/2d/crosshaire",					INITRT_SHADER		},
	{	&cgs.media.crosshairShader[5],		"gfx/2d/crosshairf",					INITRT_SHADER		},
	{	&cgs.media.crosshairShader[6],		"gfx/2d/crosshairg",					INITRT_SHADER		},
	{	&cgs.media.crosshairShader[7],		"gfx/2d/crosshairh",					INITRT_SHADER		},
	{	&cgs.media.crosshairShader[8],		"gfx/2d/crosshairi",					INITRT_SHADER		},
	{	&cgs.media.crosshairShader[9],		"gfx/2d/crosshairj",					INITRT_SHADER		}, */
	{	&cgs.media.scannerShader,			"gfx/scanner_bg",						INITRT_SHADER		},
	{	&cgs.media.scannerblipShader,		"ui/gfx/hud/scanner_center",			INITRT_SHADERNOMIP	},
	{	&cgs.media.scannerupShader,			"ui/gfx/hud/scanner_above",				INITRT_SHADERNOMIP	},
	{	&cgs.media.scannerdownShader,		"ui/gfx/hud/scanner_below",				INITRT_SHADERNOMIP	},
	{	&cgs.media.backTileShader,			"gfx/2d/backtile",						INITRT_SHADER		},
	{	&cgs.media.noammoShader,			"icons/noammo",							INITRT_SHADER		},
	{	&cgs.media.hud_armourShader,		"gfx/hud/hud_armour",					INITRT_SHADER		},
	{	&cgs.media.hud_healthShader,		"gfx/hud/hud_health",					INITRT_SHADER		},
	{	&cgs.media.hud_icon_shells,			"ui/gfx/hud/icons/icon_shells",			INITRT_SHADERNOMIP	},
	{	&cgs.media.hud_icon_nails,			"ui/gfx/hud/icons/icon_nails",			INITRT_SHADERNOMIP	},
	{	&cgs.media.hud_icon_rockets,		"ui/gfx/hud/icons/icon_rockets",		INITRT_SHADERNOMIP	},
	{	&cgs.media.hud_icon_cells,			"ui/gfx/hud/icons/icon_cells",			INITRT_SHADERNOMIP	},
	{	&cgs.media.hud_sniperscope,			"sniperscopeShader",					INITRT_SHADERNOMIP	},
	{	&cgs.media.hud_sniperscopeXhair,	"sniperscopeXHairShader",				INITRT_SHADERNOMIP	},
	{	&cgs.media.hud_binoculars,			"binocularShader",						INITRT_SHADERNOMIP	},
	{	&cgs.media.hud_binocularsXhair,		"binocularsXHairShader",				INITRT_SHADERNOMIP	},
	{	&cgs.media.hud_binocularsTarget,	"binocularsTargetShader",				INITRT_SHADERNOMIP	},
	{	&cgs.media.quadShader,				"powerups/quad",						INITRT_SHADER		},
	{	&cgs.media.quadWeaponShader,		"powerups/quadWeapon",					INITRT_SHADER		},
	{	&cgs.media.redQuadWeaponShader,		"powerups/redQuadWeapon",				INITRT_SHADER		},
	{	&cgs.media.yellowQuadWeaponShader,	"powerups/yellowQuadWeapon",			INITRT_SHADER		},
	{	&cgs.media.greenQuadWeaponShader,	"powerups/greenQuadWeapon",				INITRT_SHADER		},
	{	&cgs.media.battleSuitShader,		"powerups/battleSuit",					INITRT_SHADER		},
	{	&cgs.media.battleWeaponShader,		"powerups/battleWeapon",				INITRT_SHADER		},
	{	&cgs.media.invisShader,				"powerups/invisibility",				INITRT_SHADER		},
	{	&cgs.media.regenShader,				"powerups/regen",						INITRT_SHADER		},
	{	&cgs.media.onFireShader0,			"modelonfire0",							INITRT_SHADER		},
	{	&cgs.media.onFireShader1,			"modelonfire1",							INITRT_SHADER		}, 
	{	&cgs.media.hastePuffShader,			"hasteSmokePuff",						INITRT_SHADER		},
	{	&cgs.media.friendShader,			"icons/ident_arrow",					INITRT_SHADER		},
	{	&cgs.media.redQuadShader,			"powerups/redquad",						INITRT_SHADER		},
	{	&cgs.media.yellowQuadShader,		"powerups/yellowquad",					INITRT_SHADER		},
	{	&cgs.media.greenQuadShader,			"powerups/greenquad",					INITRT_SHADER		},
	{	&cgs.media.teamStatusBar,			"gfx/2d/colorbar.tga",					INITRT_SHADER		},
/*	{	&cgs.media.armorGreenModel,			"models/powerups/armor/armor_gre.md3",	INITRT_MODEL		},
	{	&cgs.media.armorGreenIcon,			"icons/iconr_green",					INITRT_SHADERNOMIP	},
	{	&cgs.media.armorYellowModel,		"models/powerups/armor/armor_yel.md3",	INITRT_MODEL		},
	{	&cgs.media.armorYellowIcon,			"icons/iconr_yellow",					INITRT_SHADERNOMIP	},
	{	&cgs.media.armorRedModel,			"models/powerups/armor/armor_red.md3",	INITRT_MODEL		},
	{	&cgs.media.armorRedIcon,			"icons/iconr_red",						INITRT_SHADERNOMIP	},*/
	{	&cgs.media.machinegunBrassModel,	"models/weapons2/shells/m_shell.md3",	INITRT_MODEL		},
	{	&cgs.media.shotgunBrassModel,		"models/weapons2/shells/s_shell.md3",	INITRT_MODEL		},
	{	&cgs.media.gibAbdomen,				"models/gibs/abdomen.md3",				INITRT_MODEL		},
	{	&cgs.media.gibArm,					"models/gibs/arm.md3",					INITRT_MODEL		},
	{	&cgs.media.gibChest,				"models/gibs/chest.md3",				INITRT_MODEL		},
	{	&cgs.media.gibFist,					"models/gibs/fist.md3",					INITRT_MODEL		},
	{	&cgs.media.gibFoot,					"models/gibs/foot.md3",					INITRT_MODEL		},
	{	&cgs.media.gibForearm,				"models/gibs/forearm.md3",				INITRT_MODEL		},
	{	&cgs.media.gibIntestine,			"models/gibs/intestine.md3",			INITRT_MODEL		},
	{	&cgs.media.gibLeg,					"models/gibs/leg.md3",					INITRT_MODEL		},
	{	&cgs.media.gibSkull,				"models/gibs/skull.md3",				INITRT_MODEL		},
	{	&cgs.media.gibBrain,				"models/gibs/brain.md3",				INITRT_MODEL		},
	{	&cgs.media.smoke2,					"models/weapons2/shells/s_shell.md3",	INITRT_MODEL		},
	{	&cgs.media.balloonShader,			"sprites/balloon3",						INITRT_SHADER		},
	{	&cgs.media.bloodExplosionShader,	"bloodExplosion",						INITRT_SHADER		},
	{	&cgs.media.bulletFlashModel,		"models/weaphits/bullet.md3",			INITRT_MODEL		},
	{	&cgs.media.sphereFlashModel,		"models/weaphits/sphere.md3",			INITRT_MODEL		},
	{	&cgs.media.napalmFlameShader,		"gfx/napalmFlame",						INITRT_SHADER		},
	{	&cgs.media.flameEffectShader,		"models/objects/flame",					INITRT_SHADER		},
	{	&cgs.media.flameModel,				"models/objects/flame.md3",				INITRT_MODEL		},
//	{	&cgs.media.flameShader,				"models/weaphits/fthrow/fthrow_flame",	INITRT_SHADER		},
	{	&cgs.media.bulletExplosionShaders[0],"bulletExplosion2",					INITRT_SHADER		},
	{	&cgs.media.bulletExplosionShaders[1],"bulletExplosion1",					INITRT_SHADER		},
	{	&cgs.media.bulletExplosionShaders[2],"bulletExplosion0",					INITRT_SHADER		},
	{	&cgs.media.pulseExplosionShader,	"pulseExplosion",						INITRT_SHADER		},
	{	&cgs.media.pulse3DExplosionShader,	"pulseExplosion3D",						INITRT_SHADER		},
	{	&cgs.media.pulseRingShader,			"pulseRing",							INITRT_SHADER		},
	{	&cgs.media.pulseBeamShader,			"pulseBeam",							INITRT_SHADER		},
	{	&cgs.media.medalExcellent,			"medal_excellent",						INITRT_SHADERNOMIP	},
	{	&cgs.media.medalGauntlet,			"medal_gauntlet",						INITRT_SHADERNOMIP	},
	//{	&cgs.media.medalDefend,				"medal_defend",							INITRT_SHADERNOMIP	},
	//{	&cgs.media.medalAssist,				"medal_assist",							INITRT_SHADERNOMIP	},
	//{	&cgs.media.medalCapture,			"medal_capture",						INITRT_SHADERNOMIP	},
	{	&cgs.media.savemeShader,			"icons/saveme",							INITRT_SHADERNOMIP	},
	{	&cgs.media.armormeShader,			"icons/armorme",						INITRT_SHADERNOMIP	},
	{	&cgs.media.sniperDot,				"gfx/sniperdot",						INITRT_SHADER		},
	{	&cgs.media.sniperLaser,				"gfx/sniperlaser",						INITRT_SHADER		},
	{	&cgs.media.bulletMarkShader,		"gfx/damage/bullet_mrk",				INITRT_SHADER		},
	{	&cgs.media.burnMarkShader,			"gfx/damage/burn_med_mrk",				INITRT_SHADER		},
	{	&cgs.media.holeMarkShader,			"gfx/damage/hole_lg_mrk",				INITRT_SHADER		},
	{	&cgs.media.energyMarkShader,		"gfx/damage/plasma_mrk",				INITRT_SHADER		},
	{	&cgs.media.shadowMarkShader,		"markShadow",							INITRT_SHADER		},
	{	&cgs.media.wakeMarkShader,			"wake",									INITRT_SHADER		},
	{	&cgs.media.bloodMarkShader,			"bloodMark",							INITRT_SHADER		},
	{	&cgs.media.footprintRightMarkShader, "footprintRightMark",					INITRT_SHADER		},
	{	&cgs.media.footprintLeftMarkShader,	"footprintLeftMark",					INITRT_SHADER		},
	{	&cgs.media.minigunSmokeTag1,		"models/weapons2/minigun/minigun_smoke_startup.md3",	INITRT_MODEL	},
	{	&cgs.media.minigunSmokeTag2,		"models/weapons2/minigun/minigun_smoke_firing.md3",		INITRT_MODEL	},
	{	&cgs.media.minigunFlashTag,			"models/weapons2/minigun/minigun_flash_flame.md3",		INITRT_MODEL	},

	// lens flares
	{	&cgs.media.flare1,					"gfx/flares/flare1",					INITRT_SHADER		},
	{	&cgs.media.flare2,					"gfx/flares/flare2",					INITRT_SHADER		},
	{	&cgs.media.flare3,					"gfx/flares/flare3",					INITRT_SHADER		},
	{	&cgs.media.flare4,					"gfx/flares/flare4",					INITRT_SHADER		},

	// Sun lens flare
	{	&cgs.media.bluedisc,				"gfx/flares/bluedisc",					INITRT_SHADER		},
	{	&cgs.media.bluediscweak,			"gfx/flares/bluediscweak",				INITRT_SHADER		},
	{	&cgs.media.bluegradient,			"gfx/flares/bluegradient",				INITRT_SHADER		},
	{	&cgs.media.browndisc,				"gfx/flares/browndisc",					INITRT_SHADER		},
	{	&cgs.media.brownring,				"gfx/flares/brownring",					INITRT_SHADER		},
	{	&cgs.media.greenring,				"gfx/flares/greenring",					INITRT_SHADER		},
	{	&cgs.media.rainbowring,				"gfx/flares/rainbowring",				INITRT_SHADER		},
	{	&cgs.media.whitegradient,			"gfx/flares/whitegradient",				INITRT_SHADER		},
	{	&cgs.media.whiteredring2,			"gfx/flares/whiteredring2",				INITRT_SHADER		},
	{	&cgs.media.whiteredring66,			"gfx/flares/whiteredring66",			INITRT_SHADER		},
	{	&cgs.media.whitering,				"gfx/flares/whitering",					INITRT_SHADER		},

	{	&cgs.media.backpack,				"models/objects/backpack/backpack_small.md3",	INITRT_MODEL		},

	{	&cgs.media.hudTimerShader,			"ui/gfx/hud/timer.tga",					INITRT_SHADERNOMIP	},

	{	&cgs.media.repairmeShader,			"icons/repairme",						INITRT_SHADERNOMIP	},
	{	&cgs.media.upgrademeShader,			"icons/upgrademe",						INITRT_SHADERNOMIP	},
	{	&cgs.media.refillmeShader,			"icons/refillme",						INITRT_SHADERNOMIP	},

	{	&cgs.media.disconnectIcon,			"gfx/2d/net",							INITRT_SHADER	},

#ifdef RIMLIGHTING_OUTLINE
	{	&cgs.media.outlineShader,			"_outline",								INITRT_SHADERNOMIP	},
#endif

	{	NULL,								"",										0					},
};

typedef struct {
	SpiritScript_t **storage;
	const char * name;
} initSpiritDef_t;


static initSpiritDef_t initSpiritDefs[] = {
	{ &cgs.spirit.diseased,				"spirit/playerfx/diseased.spirit" },
	{ &cgs.spirit.gassed,				"spirit/playerfx/gassed.spirit" },
	{ &cgs.spirit.flashed,				"spirit/playerfx/flashed.spirit" },
	{ &cgs.spirit.stunned,				"spirit/playerfx/stunned.spirit" },
	{ &cgs.spirit.tranqed,				"spirit/playerfx/tranqed.spirit" },
	{ &cgs.spirit.legshot,				"spirit/playerfx/legshot.spirit" },
	{ &cgs.spirit.forcefieldspark,		"spirit/forcefieldspark.spirit" },
	{ &cgs.spirit.spawn,				"spirit/spawn.spirit" },
	{ &cgs.spirit.watersplash,			"spirit/watersplash.spirit" },
	{ &cgs.spirit.explosion_concussion,	"spirit/explosions/explosion_concussion.spirit" },
	{ &cgs.spirit.explosion_flash,		"spirit/explosions/explosion_flash.spirit" },
	{ &cgs.spirit.explosion_simple,		"spirit/explosions/explosion_simple.spirit" },
	{ &cgs.spirit.explosion_normal,		"spirit/explosions/explosion_normal.spirit" },
	{ &cgs.spirit.explosion_he,			"spirit/explosions/explosion_he.spirit" },
	{ &cgs.spirit.explosion_napalm,		"spirit/explosions/explosion_napalm.spirit" },
	{ &cgs.spirit.explosion_quad,		"spirit/explosions/explosion_quad.spirit" },
	{ NULL,								"" },
};

/******************************************************************************
*****	Initialization phases
****/

static void CG_Q3F_InitPhaseMapInfo()
{
	// Pull data out of the seperate mapinfo.

	char values[3][1024];

	/*cg_q3f_mapinfo_t infoFields[] = {
		{ "longname",		values[0], sizeof(values[0]) },
		{ "mapinfo",		values[1], sizeof(values[1]) },
		{ "shaderremap",	values[2], sizeof(values[2]) },
	};*/
	cg_q3f_mapinfo_t infoFields[3];

	infoFields[0].key = "longname";
	infoFields[0].value = values[0];
	infoFields[0].valueSize = sizeof(values[0]);

	infoFields[1].key = "mapinfo";
	infoFields[1].value = values[1];
	infoFields[1].valueSize = sizeof(values[1]);

	infoFields[2].key = "shaderremap";
	infoFields[2].value = values[2];
	infoFields[2].valueSize = sizeof(values[2]);

	CG_Q3F_InitLog( "Map Information", "", "..." );
	CG_Q3F_RenderLoadingScreen();
	CG_Q3F_GetMapInfo( cgs.mapname, infoFields, 3, cgs.gameindex );

	if( *values[0] )	cgs.longName	= CG_Q3F_AddString( values[0] );
	if( *values[1] )	cgs.description	= CG_Q3F_AddString( values[1] );
	if( *values[2] )	cgs.shaderRemap	= CG_Q3F_AddString( values[2] );

	cgs.initTime = trap_Milliseconds();
	CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
}

static void CG_Q3F_InitPhaseMapBSP()
{
	// Load the world BSP data.

		CG_Q3F_InitLog( "Map Data", "", "..." );
		CG_Q3F_RenderLoadingScreen();
		trap_CM_LoadMap( cgs.mapname );
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
}

static void CG_Q3F_InitPhaseMapRender()
{
	// Load the world render data (the worldmodel itself?).
		CG_Q3F_InitLog( "World", "", "..." );
		CG_Q3F_RenderLoadingScreen();
		trap_R_LoadWorldMap( cgs.mapname );
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
}

static void CG_Q3F_InitPhaseMapEntities()
{
	// Load the cgame entity data.

		CG_Q3F_InitLog( "Entities", "", "..." );
		CG_Q3F_RenderLoadingScreen();
		CG_Q3F_ParseEntities();
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
}

static void CG_Q3F_InitPhaseSPD()
{
	// Load the world SPD

		CG_Q3F_InitLog( "Spline Data", "", "..." );
		CG_Q3F_RenderLoadingScreen();
		cgs.camNumPaths = BG_Q3F_LoadCamPaths( cgs.mapname, (void *)cgs.campaths );
		cgs.flybyPathIndex = BG_Q3F_LocateFlybyPath( cgs.camNumPaths, (void *)cgs.campaths );
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
}


static void CG_Q3F_InitPhaseSoundStatic()
{
	// Load all static (i.e. always used) sounds.

	initSound_t *snd;

	snd = &initSounds[cgs.initIndex++];

	if( snd->storage )
	{
		CG_Q3F_InitLog( "Static Sound ", snd->name, "..." );
		CG_Q3F_RenderLoadingScreen();
		if( !(*snd->storage = trap_S_RegisterSound( snd->name, qfalse )) )
			CG_Q3F_InitLog( "ERROR: ", snd->name, " not loaded." );
	}
	else {
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
	}
}

static const char *voiceSounds[] = {
	"sound/voices/spencer/def_depdisp.wav",
	"sound/voices/spencer/def_deppipe.wav",
	"sound/voices/spencer/def_depsen.wav",
	"sound/voices/spencer/def_dropflag.wav",
	"sound/voices/spencer/def_fixsenty.wav",
	"sound/voices/spencer/def_flag.wav",
	"sound/voices/spencer/def_flagdanger.wav",
	"sound/voices/spencer/def_flagprimexit.wav",
	"sound/voices/spencer/def_flagsafe.wav",
	"sound/voices/spencer/def_flagsecexit.wav",
	"sound/voices/spencer/def_iam.wav",
	"sound/voices/spencer/def_incflag1.wav",
	"sound/voices/spencer/def_incflag2.wav",
	"sound/voices/spencer/def_incprimrte.wav",
	"sound/voices/spencer/def_incsecrte.wav",
	"sound/voices/spencer/def_needsupp.wav",
	"sound/voices/spencer/def_obj.wav",
	"sound/voices/spencer/def_wpt.wav",
	"sound/voices/spencer/gen_anytime.wav",
	"sound/voices/spencer/gen_ceasefire.wav",
	"sound/voices/spencer/gen_firehole.wav",
	"sound/voices/spencer/gen_giveammo.wav",
	"sound/voices/spencer/gen_gogogo.wav",
	"sound/voices/spencer/gen_goodbye1.wav",
	"sound/voices/spencer/gen_goodbye2.wav",
	"sound/voices/spencer/gen_halt.wav",
	"sound/voices/spencer/gen_hello1.wav",
	"sound/voices/spencer/gen_hello2.wav",
	"sound/voices/spencer/gen_inpos.wav",
	"sound/voices/spencer/gen_isbasesec.wav",
	"sound/voices/spencer/gen_moveout.wav",
	"sound/voices/spencer/gen_movepls.wav",
	"sound/voices/spencer/gen_no1.wav",
	"sound/voices/spencer/gen_no2.wav",
	"sound/voices/spencer/gen_noprob.wav",
	"sound/voices/spencer/gen_objcplt.wav",
	"sound/voices/spencer/gen_objfld.wav",
	"sound/voices/spencer/gen_oops.wav",
	"sound/voices/spencer/gen_pass.wav",
	"sound/voices/spencer/gen_reportin.wav",
	"sound/voices/spencer/gen_sorry.wav",
	"sound/voices/spencer/gen_stop.wav",
	"sound/voices/spencer/gen_unlucky.wav",
	"sound/voices/spencer/gen_wait.wav",
	"sound/voices/spencer/gen_waitord.wav",
	"sound/voices/spencer/gen_watchfire.wav",
	"sound/voices/spencer/gen_yes1.wav",
	"sound/voices/spencer/gen_yes2.wav",
	"sound/voices/spencer/off_attobj.wav",
	"sound/voices/spencer/off_attsentry.wav",
	"sound/voices/spencer/off_attwpt.wav",
	"sound/voices/spencer/off_carrsupp.wav",
	"sound/voices/spencer/off_coverme.wav",
	"sound/voices/spencer/off_defhvy.wav",
	"sound/voices/spencer/off_deflight.wav",
	"sound/voices/spencer/off_dephe.wav",
	"sound/voices/spencer/off_flagget.wav",
	"sound/voices/spencer/off_flaggive.wav",
	"sound/voices/spencer/off_flaghave.wav",
	"sound/voices/spencer/off_flagtake.wav",
	"sound/voices/spencer/off_imatt.wav",
	"sound/voices/spencer/off_needsupp.wav",
	"sound/voices/spencer/off_spotpipe.wav",
	"sound/voices/spencer/off_spotsen.wav",
	"sound/voices/spencer/tap_alright.wav",
	"sound/voices/spencer/tap_aw.wav",
	"sound/voices/spencer/tap_goaway.wav",
	"sound/voices/spencer/tap_goodgame1.wav",
	"sound/voices/spencer/tap_goodgame2.wav",
	"sound/voices/spencer/tap_myflag1.wav",
	"sound/voices/spencer/tap_myflag2.wav",
	"sound/voices/spencer/tap_nicecapture1.wav",
	"sound/voices/spencer/tap_nicecapture2.wav",
	"sound/voices/spencer/tap_nicemove1.wav",
	"sound/voices/spencer/tap_nicemove2.wav",
	"sound/voices/spencer/tap_niceshot.wav",
	"sound/voices/spencer/tap_sneakybastard.wav",
	"sound/voices/spencer/tap_thatsucks1.wav",
	"sound/voices/spencer/tap_thatsucks2.wav",
	"sound/voices/spencer/tap_thegreatest.wav",
	"sound/voices/spencer/tap_wellplayed1.wav",
	"sound/voices/spencer/tap_wellplayed2.wav",
	"sound/voices/spencer/tap_werock1.wav",
	"sound/voices/spencer/tap_werock2.wav",
	"sound/voices/spencer/tap_yourmine.wav",
	"sound/voices/spencer/tap_yourock.wav"
};

static const size_t numVoiceSounds = ARRAY_LEN( voiceSounds );

static void CG_Q3F_InitPhaseSoundVoiceComms(void)
{
	if( cgs.initIndex < numVoiceSounds )
	{
		CG_Q3F_InitLog( "VO Sound ", voiceSounds[cgs.initIndex], "..." );
		CG_Q3F_RenderLoadingScreen();
		if( !trap_S_RegisterSound( voiceSounds[cgs.initIndex], qfalse ) )
			CG_Q3F_InitLog( "ERROR: ", voiceSounds[cgs.initIndex], " not loaded." );
		cgs.initIndex++;
	}
	else CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
}

static void CG_Q3F_InitPhaseSoundFootsteps()
{
	// Load footstep sounds

	char *type;
	int id;

	id = cgs.initIndex / 4;		// 4 step sounds per type.
	if( id >= FOOTSTEP_TOTAL )
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
	else {
		switch( id )
		{
			case FOOTSTEP_NORMAL:	type = "step";		break;
			case FOOTSTEP_BOOT:		type = "carpet";	break;
			case FOOTSTEP_FLESH:	type = "gravel";	break;
			case FOOTSTEP_MECH:		type = "clank";		break;
			case FOOTSTEP_ENERGY:	type = "step";		break;
			case FOOTSTEP_SPLASH:	type = "water";		break;
			case FOOTSTEP_METAL:	type = "clank";		break;
			default:				type = "none";		break;
		}
		Com_sprintf( cgs.initBuff, sizeof(cgs.initBuff), "sound/player/footsteps/%s%i.wav", type, (cgs.initIndex & 3) + 1 );
		CG_Q3F_InitLog( "Footstep Sound ", cgs.initBuff, "..." );
		CG_Q3F_RenderLoadingScreen();
		if( !(cgs.media.footsteps[id][cgs.initIndex & 3] = trap_S_RegisterSound( cgs.initBuff, qfalse )) )
			CG_Q3F_InitLog( "FS ERROR: ", cgs.initBuff, " not loaded." );
		cgs.initIndex++;
	}
}

static void CG_Q3F_InitPhaseSoundItems()
{
	if( cgs.initIndex < bg_numItems )
	{
		if( CG_ConfigString( CS_ITEMS )[cgs.initIndex] )
		{
			CG_Q3F_InitLog( "Sound ", bg_itemlist[cgs.initIndex].pickup_name, "..." );
			CG_Q3F_RenderLoadingScreen();
			CG_RegisterItemSounds( cgs.initIndex );
		}
		cgs.initIndex++;
	}
	else CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
}

static void CG_Q3F_InitPhaseSoundDynamic()
{
	const char *soundName;

	while( cgs.initIndex < MAX_SOUNDS )
	{
		soundName = CG_ConfigString( CS_SOUNDS + cgs.initIndex );
		if( !soundName[0] )
		{
			if( cgs.initIndex != 0 )
				CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
			else cgs.initIndex++;
			break;
		}
		else if( soundName[0] != '*' )
		{
			CG_Q3F_InitLog( "Dynamic Sound ", soundName, "..." );
			CG_Q3F_RenderLoadingScreen();
			if ( !( cgs.gameSounds[cgs.initIndex++] = trap_S_RegisterSound( soundName, qfalse ) ) )
				CG_Q3F_InitLog( "ERROR: ", soundName, " not loaded." );
			break;
		}
		else {
			cgs.initIndex++;
		}
	}
}

static void CG_Q3F_InitPhaseGraphicStatic()
{
	// Load all static (i.e. always used) graphics.

	initGraphic_t *graphic;

	graphic = &initGraphics[cgs.initIndex++];

	if( graphic->storage )
	{
		switch( graphic->type )
		{
			case INITRT_SHADER:
			case INITRT_SHADERNOMIP:	CG_Q3F_InitLog( "Shader ", graphic->name, "..." );
										break;
			case INITRT_MODEL:			CG_Q3F_InitLog( "Model ", graphic->name, "..." );
			default:					break;
		}
		CG_Q3F_RenderLoadingScreen();
		switch( graphic->type )
		{
			case INITRT_SHADER:			*graphic->storage = trap_R_RegisterShader( graphic->name );
										break;
			case INITRT_SHADERNOMIP:	*graphic->storage = trap_R_RegisterShaderNoMip( graphic->name );
										break;
			case INITRT_MODEL:			*graphic->storage = trap_R_RegisterModel( graphic->name );
										break;
		}
		if( !*graphic->storage )
			CG_Q3F_InitLog( "ERROR: ", graphic->name, " not loaded." );
	} else {
		//keeg:  load xhairs
		int i;
		for ( i = 0 ; i < NUM_CROSSHAIRS ; i++ ) {
			cgs.media.crosshairShader[i] = trap_R_RegisterShader( va("gfx/2d/crosshair%c", 'a'+i) );
			cg.crosshairShaderAlt[i] = trap_R_RegisterShader( va("gfx/2d/crosshair%c_alt", 'a'+i) );
		}
		CG_InitFlameChunks();
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
	}
}

static void CG_Q3F_InitPhaseGraphicItems()
{
	// Load all item-related graphics

	if( cgs.initIndex > bg_numItems )
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
	else {
		if( !cgs.initIndex )
			Q_strncpyz( cgs.initBuff, CG_ConfigString( CS_ITEMS), sizeof(cgs.initBuff) );
		if( cgs.initBuff[cgs.initIndex] == '1')
		{
			CG_Q3F_InitLog( "Item ", bg_itemlist[cgs.initIndex].pickup_name, "..." );
			CG_Q3F_RenderLoadingScreen();
			CG_RegisterItemVisuals( cgs.initIndex );
		}
		cgs.initIndex++;
	}
}

static void CG_Q3F_InitPhaseGraphicWorld()
{
	// Load all world graphics

	vec3_t mins, maxs;
	int j;

	if( !cgs.initIndex &&
		!(cgs.initIndex = trap_CM_NumInlineModels()) )
	{
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
		return;
	}
	cgs.initIndex--;

	Com_sprintf( cgs.initBuff, sizeof(cgs.initBuff), "*%i", cgs.initIndex );
	cgs.inlineDrawModel[cgs.initIndex] = trap_R_RegisterModel( cgs.initBuff );
	trap_R_ModelBounds( cgs.inlineDrawModel[cgs.initIndex], mins, maxs );
	for ( j = 0 ; j < 3 ; j++ )
		cgs.inlineModelMidpoints[cgs.initIndex][j] = mins[j] + 0.5 * ( maxs[j] - mins[j] );

		// We're counting down, abort this if there's no more models left, otherwise it'll
		// get stuck in an infinite loop.
	if( !cgs.initIndex )
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
}

static void CG_Q3F_InitPhaseGraphicModels()
{
	// Load all dynamic models

	const char *modelName;

	if( cgs.initIndex >= MAX_MODELS )
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
	else {
		modelName = CG_ConfigString( CS_MODELS + cgs.initIndex );
		if( modelName[0] )
		{
			CG_Q3F_InitLog( "Custom Model ", modelName, "..." );
			CG_Q3F_RenderLoadingScreen();
			cgs.gameModels[cgs.initIndex] = trap_R_RegisterModel( modelName );
		}
		cgs.initIndex++;
	}
}

static void CG_Q3F_InitPhaseGraphicShaders()
{
	// Load all dynamic shaders

	const char *shaderName;
	int shaderOffset;

	if( cgs.initIndex >= MAX_SHADERS )
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
	else {
		shaderName = CG_ConfigString( CS_SHADERS + cgs.initIndex );
		if( shaderName[0] )
		{
			shaderOffset = (shaderName[0] == '*') ? 1 : 0;
			CG_Q3F_InitLog( "Custom Shader ", shaderName + shaderOffset, "..." );
			CG_Q3F_RenderLoadingScreen();
			if( shaderOffset )
				cgs.gameShaders[cgs.initIndex] = trap_R_RegisterShaderNoMip( shaderName );
			else cgs.gameShaders[cgs.initIndex] = trap_R_RegisterShader( shaderName );
		}
		cgs.initIndex++;
	}
}

static void CG_Q3F_InitPhaseSpiritStatic()
{
	initSpiritDef_t * spiritdef = &initSpiritDefs[cgs.initIndex];
	if (spiritdef->storage) 
	{
		cgs.initIndex++;
		CG_Q3F_InitLog( "Script ", spiritdef->name, "..." );
		CG_Q3F_RenderLoadingScreen();
		*spiritdef->storage = Spirit_LoadScript( spiritdef->name );
		if ( !*spiritdef->storage ) {
			CG_Q3F_InitLog( "ERROR: ", spiritdef->name, " not loaded." );
			CG_Q3F_RenderLoadingScreen();
			return;
		}
	} else
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
};

static void CG_Q3F_InitPhaseSpiritDynamic()
{
	const char *scriptName;

	while( cgs.initIndex < MAX_SPIRITSCRIPTS )
	{
		scriptName = CG_ConfigString( CS_SPIRITSCRIPTS + cgs.initIndex );
		if( !scriptName[0] )
		{
			if( cgs.initIndex != 0 )
				CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
			else cgs.initIndex++;
			break;
		}
		else if( scriptName[0] != '*' )
		{
			CG_Q3F_InitLog( "Dynamic Spirit Script ", scriptName, "..." );
			CG_Q3F_RenderLoadingScreen();
			if( !(cgs.gameSpiritScript[cgs.initIndex++] = Spirit_LoadScript( scriptName) ) )
				CG_Q3F_InitLog( "ERROR: ", scriptName, " not loaded." );
			break;
		}
		else {
			cgs.initIndex++;
		}
	}
};


static void CG_Q3F_InitPhaseClasses()
{
	int i;
	bg_q3f_playerclass_t *cls;

	if ( cgs.initIndex == Q3F_CLASS_NULL ) {
		cgs.teams =  atoi( CG_ConfigString( CS_TEAMMASK ));
		sscanf(	CG_ConfigString( CS_TEAMALLIED ), "%i %i %i %i",
			&cg.teamAllies[0], &cg.teamAllies[1],
			&cg.teamAllies[2], &cg.teamAllies[3] );
		cgs.classes = atoi( CG_ConfigString( CS_CLASSMASK ));
		cgs.initIndex++;
	} else if( cgs.initIndex >= Q3F_CLASS_MAX) {
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
	} else {
		if( !(cgs.classes & ( 1 << cgs.initIndex ) ) ) {
			cgs.initIndex++;
			return;
		}

		// Log the registering
		cls = bg_q3f_classlist[ cgs.initIndex ];
		CG_Q3F_InitLog( "Class ", cls->title, "..." );
		CG_Q3F_RenderLoadingScreen();

		// Register the class's model
		if( !CG_Q3F_RegisterClassModels( cgs.initIndex ) ) {
			CG_Q3F_InitLog( "ERROR: ", cls->title, " art not precached properly." );
			CG_Q3F_RenderLoadingScreen();
		}
        
		// Register the class's weapons
		for( i = 0; i < Q3F_NUM_WEAPONSLOTS; i++ ) {
			if( !cls->weaponslot[i] )
				continue;

			if( cls->weaponslot[i] >= WP_NUM_WEAPONS )
				continue;

			if( cls->weaponslot[i] == WP_AXE ) {
				switch( cgs.initIndex ) {
				default:
					CG_RegisterWeapon( WP_AXE );
					break;
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

		// Register Gren1 visuals
		if ( cls->gren1type != Q3F_GREN_NONE )
			CG_Q3F_RegisterGrenade( cls->gren1type );

		// Register Gren2 visuals
		if ( cls->gren2type != Q3F_GREN_NONE )
			CG_Q3F_RegisterGrenade( cls->gren2type );

		// Register HE charge visuals
		if ( cgs.initIndex == Q3F_CLASS_GRENADIER )
			CG_Q3F_RegisterGrenade( Q3F_GREN_CHARGE );

		if( !CG_Q3F_RegisterClassSounds( cgs.initIndex ) ) {
			CG_Q3F_InitLog( "ERROR: ", cls->title, " sounds not precached properly." );
			CG_Q3F_RenderLoadingScreen();
		}
		// Register special media for classes
		switch ( cgs.initIndex ) {
		default:
		case Q3F_CLASS_RECON:
		case Q3F_CLASS_SNIPER:
		case Q3F_CLASS_SOLDIER:
		case Q3F_CLASS_GRENADIER:
		case Q3F_CLASS_PARAMEDIC:
		case Q3F_CLASS_MINIGUNNER:
		case Q3F_CLASS_FLAMETROOPER:
			break;
		case Q3F_CLASS_AGENT:
			cgs.media.agentShader = trap_R_RegisterShader( "gfx/agenteffect" );
			break;
		case Q3F_CLASS_ENGINEER:
			CG_Q3F_RegisterSentry();
			CG_Q3F_RegisterSupplyStation();
			break;
		case Q3F_CLASS_CIVILIAN:
			break;
		}
		cgs.initIndex++;
	}
}

void CG_ParseTeamNameinfo( void );

static void CG_Q3F_InitPhaseClients()
{
	// Initialize all client information
	const char *clientInfo;

	if( cgs.initIndex >= MAX_CLIENTS ) {
		CG_ParseTeamNameinfo();
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
	} else {
		clientInfo = CG_ConfigString( CS_PLAYERS + cgs.initIndex );
		if( clientInfo[0] )
		{
			Com_sprintf( cgs.initBuff, sizeof(cgs.initBuff), "%i", cgs.initIndex );
			CG_Q3F_InitLog( "Client ", cgs.initBuff, "..." );
			CG_Q3F_RenderLoadingScreen();
			CG_NewClientInfo( cgs.initIndex );
		}
		cgs.initIndex++;
	}
}

static void CG_Q3F_InitPhaseUIScripting()
{
	// Initialize all ui script stuff
		CG_Q3F_InitLog( "UI Scripting", "", "..." );
		CG_Q3F_RenderLoadingScreen();
		SetCurrentMemory( MEM_CGAME );
		CG_AssetCache();
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
}

static void CG_Q3F_InitPhaseShaderRemap()
{
	// Remap shaders based on the 'remapshader' mapinfo directive.

	int index, splitIndex, destIndex;
	char from[1024], to[1024];

	if( !cgs.shaderRemap )
		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
	else {
		for(	splitIndex = index = cgs.initIndex + 1;
				cgs.shaderRemap[index] && cgs.shaderRemap[index] != ',';
				index++ )
		{
			if( cgs.shaderRemap[index] == '=' )
				splitIndex = index;
		}
	
		if( splitIndex > cgs.initIndex && index > (splitIndex + 1) &&
			splitIndex - cgs.initIndex < sizeof(from) - 1 &&
			index - splitIndex < sizeof(to) - 2 )
		{
			// Something to remap.

			for( destIndex = cgs.initIndex; destIndex < splitIndex; destIndex++ )
				from[destIndex - cgs.initIndex] = cgs.shaderRemap[destIndex];
			from[destIndex - cgs.initIndex] = 0;
			for( destIndex = splitIndex + 1; destIndex < index; destIndex++ )
				to[destIndex - splitIndex - 1] = cgs.shaderRemap[destIndex];
			to[destIndex - splitIndex - 1] = 0;
			CG_Q3F_InitLog( "Remapping ", from, "..." );
			CG_Q3F_RenderLoadingScreen();

			trap_R_RemapShader( from, to, 0 );
		}

		if( !cgs.shaderRemap[index] )
			CG_Q3F_SetInitPhase( cgs.initPhase + 1 );
		else cgs.initIndex = index + 1;
	}
}

static void CG_Q3F_InitPhaseSnapshot()
{
	// Wait for a snapshot.
	// ALWAYS THE LAST PHASE

//	if( cg.snap )
	if ( !cg.snap || ( cg.snap->snapFlags & SNAPFLAG_NOT_ACTIVE ) )
	{
		if( !cgs.initIndex )
		{
			CG_Q3F_InitLog( "Waiting for snapshot", "", "..." );
			CG_Q3F_RenderLoadingScreen();
			cgs.initIndex++;
		}
		CG_ProcessSnapshots();
		CG_Q3F_RenderLoadingScreen();
	}
	else
	{
		cg.loading = qfalse;	// future players will be deferred
		CG_InitLocalEntities();
		CG_InitMarkPolys();
		InitParticles();
		cg.infoScreenText[0] = 0;	// remove the last loading update
		CG_SetConfigValues();		// Make sure we have update values (scores)
		CG_StartMusic();
//		CG_LoadingString( "" );
		CG_ShaderStateChanged();
		trap_S_ClearLoopingSounds( qtrue );
		CG_Q3F_LoadMapConfig();		// Golliwog: Load in a config based on mapname
		trap_Cvar_Set( "init", "0" );
		cg.ScoreSnapshotTaken = qfalse;

		CG_Q3F_SetInitPhase( cgs.initPhase + 1 );

		CG_Printf( BOX_PRINT_MODE_CHAT, "...loaded %i F2R Scripts,  Spirit %i Scripts %i Systems\n", F2R_Count(), Spirit_ScriptCount(), Spirit_SystemCount() );	// RR2DO2: it _is_ possible that more systems are
																														// loaded after this, but this is the best place to																														// print these stats//		CG_Q3F_Flyby();	
//		FIXME: enable when flyby is no longer b0rked
	}

	cgs.initScreenRendered = qtrue;		// We lie, since we're going to be busy-waiting for a snapshot.
}


/******************************************************************************
*****	Init functions called externally.
****/

void CG_InitWorldText( void );

void CG_Q3F_Init( int serverMessageNum, int serverCommandSequence, int clientNum, qboolean demoPlayback )
{
	// Perform required initialization before anything else happens.

	const char *s;
	qtime_t now;
	int i;

		// clear everything
	memset( &cgs, 0, sizeof( cgs ) );
	memset( &cg, 0, sizeof( cg ) );
	memset( cg_entities, 0, sizeof(cg_entities) );
	memset( cg_weapons, 0, sizeof(cg_weapons) );
	memset( cg_items, 0, sizeof(cg_items) );
//	memset( cgs.media.modelcache, 0, sizeof(cgs.media.modelcache) );
//	memset( cgs.media.skincache, 0, sizeof(cgs.media.skincache) );
	CG_Q3F_WaypointInit();

	CG_InitWorldText();

	String_Init();

	// Ensiform: Set this early.
	cg.demoPlayback = demoPlayback;
	
  	// OSP - sync to main refdef  keeg brought in to stop tracemap crash in cgame
   //keeg note:  could just change the other code to use cg.refdef but talk to RR2 first
	cg.refdef_current = &cg.refdef;

	CG_RegisterCvars();

	CG_LoadHudMenu();      // load new hud stuff

	cg.clientNum = clientNum;

	cgs.processedSnapshotNum = serverMessageNum;
	cgs.serverCommandSequence = serverCommandSequence;

	// load a few needed things before we do any screen updates
	cgs.media.charsetShader			= trap_R_RegisterShader( "gfx/2d/bigchars" );
	cgs.media.whiteShader			= trap_R_RegisterShader( "white" );
	cgs.media.whiteAdditiveShader	= trap_R_RegisterShader( "additivewhite" );
	cgs.media.charsetProp			= trap_R_RegisterShaderNoMip( "menu/art/font1_prop.tga" );
	cgs.media.charsetPropGlow		= trap_R_RegisterShaderNoMip( "menu/art/font1_prop_glo.tga" );
	cgs.media.charsetPropB			= trap_R_RegisterShaderNoMip( "menu/art/font2_prop.tga" );
	//cgs.media.logodisc				= trap_R_RegisterModel( "ui/models/logodisc.md3" );

	//keeg for ET flamethrower
	cgs.media.sparkFlareShader = trap_R_RegisterShader( "sparkFlareParticle" );
	cgs.media.flamethrowerFireStream = trap_R_RegisterShader( "flamethrowerFireStream" );
	cgs.media.flamethrowerBlueStream = trap_R_RegisterShader( "gfx/flamethrower/nozzleflame" );

	if ( !cgs.media.grenadePrimeSound &&
		cg_grenadePrimeSound.string &&
		*cg_grenadePrimeSound.string )
		cgs.media.grenadePrimeSound = trap_S_RegisterSound( cg_grenadePrimeSound.string, qfalse );

	//keeg 
	CG_ClearTrails();
	CG_InitSmokeSprites();

	CG_InitConsoleCommands();

	cg.weaponSelect = WP_NAILGUN;

	cg.drawFilter = qfalse;

	cg.supplyStationUsedTime = 0;	// Don't want this to be uninitialized
	cg.usingSupplystation = 0;

	// get the rendering configuration from the client system
	trap_GetGlconfig( &cgs.glconfig );
	cgs.screenXScale = cgs.glconfig.vidWidth / 640.0;
	cgs.screenYScale = cgs.glconfig.vidHeight / 480.0;

	// get the gamestate from the client system
	trap_GetGameState( &cgs.gameState );

	// check version
	s = CG_ConfigString( CS_GAME_VERSION );
	if ( strcmp( s, GAME_VERSION ) ) {
		CG_Error( "Client/Server game mismatch: %s/%s", GAME_VERSION, s );
	}

	// Canabis: Check FORTS version to give client a warning.
	s = CG_ConfigString( CS_FORTS_VERSION );
	if ( s && strcmp( s, FORTS_VERSION ) ) {
		if ( demoPlayback )
			CG_Error( "Client/Demo ETF version mismatch: %s/%s", FORTS_VERSION, s );
		else
			Com_Printf("Your version: %s doesn't match with Server version: %s\nThis could give problems.\n",
						FORTS_VERSION, s );
	}

	trap_Cvar_Set( "cg_etfVersion", FORTS_VERSION );	// So server can check, in case of force version

	trap_Cvar_Set( "init", "1" );
	// Golliwog.

	s = CG_ConfigString( CS_LEVEL_START_TIME );
	cgs.levelStartTime = atoi( s );

	CG_ParseServerinfo();
	CG_ParseSysteminfo();

	cgs.teamChatPos = cgs.teamLastChatPos = 0;

	// we're not gassed yet
	cg.gasEndTime = 0;
	for( i = 0; i < MAX_CLIENTS; i++ )
	{
		cg.gasPlayerClass[i] = cg.gasPlayerTeam[i] = 0xFF;	// "No effect"
	}

	// not reloading
	cg.reloadendtime = 0;

		// Start the main init going.
#ifdef _DEBUG
	if( (trap_FS_FOpenFile( "init.log", &cgs.initFHandle, FS_WRITE )) >= 0 )
		trap_FS_FCloseFile( cgs.initFHandle );
#endif
	trap_RealTime( &now );
	Com_sprintf( cgs.initBuff, sizeof(cgs.initBuff), "%02i:%02i:%02i %02i/%02i/%04i", now.tm_hour, now.tm_min, now.tm_sec, now.tm_mday, now.tm_mon + 1, now.tm_year + 1900 );
	CG_Q3F_InitLog( "Init starting: ", cgs.initBuff, "" );

	// KYRO HAVE BAD DRIVERS!!!
	if(!Q_stricmp(cgs.glconfig.renderer_string, "PowerVR KYRO")) {
		int dValue;
		char buffer[256];
		
		trap_Cvar_VariableStringBuffer("cg_drawSkyPortal", buffer, 256);

		dValue = atoi(buffer);

		if(dValue == 1) {
			trap_Print("^1Disabling Portal Skies On PowerVR Kyro, Set cg_drawSkyPortal To 2 To Force On");
		}
	}

	CG_Q3F_SetInitPhase( INITPHASE_NONE + 1 );

	cgs.mapInfoLoaded = qfalse;
	CG_Q3F_LoadingMapInfo();

	// remap skyportal shaders if needed
	if( !cg_drawSkyPortal.integer )
		CG_Q3F_RemapSkyShader();

	// remove console printing to upper left (sets time to show -ve)
	trap_Cvar_Set ( "con_notifytime", "-2" );

	// RR2DO2 - start loading media if demo or classicinit
	if( cg.demoPlayback || cg_classicinit.integer )
		CG_Q3F_InitUpdate();
}

void CG_Q3F_InitUpdate()
{
	static qboolean isUpdating = qfalse;

	if ( isUpdating ) {
		// don't allow recursive calling
		return;
	}

	isUpdating = qtrue;

	// Call the appropriate init code block.
	// Keeps calling until something is rendered (since A: Long operations won't return
	// anyway, and B: some operations cycle through lots of empty entries quickly).

	cgs.initScreenRendered = qfalse;

	while( !cgs.initScreenRendered )
	{
		switch( cgs.initPhase )
		{
			case INITPHASE_MAPINFO:				CG_Q3F_InitPhaseMapInfo();				break;
			case INITPHASE_MAPBSP:				CG_Q3F_InitPhaseMapBSP();				break;
			case INITPHASE_MAPRENDER:			CG_Q3F_InitPhaseMapRender();			break;
			case INITPHASE_MAPENTITIES:			CG_Q3F_InitPhaseMapEntities();			break;
			case INITPHASE_SPD:					CG_Q3F_InitPhaseSPD();					break;
			case INITPHASE_SOUND_STATIC:		CG_Q3F_InitPhaseSoundStatic();			break;
			case INITPHASE_SOUND_FOOTSTEPS:		CG_Q3F_InitPhaseSoundFootsteps();		break;
			case INITPHASE_SOUND_ITEMS:			CG_Q3F_InitPhaseSoundItems();			break;
			case INITPHASE_SOUND_DYNAMIC:		CG_Q3F_InitPhaseSoundDynamic();			break;
			case INITPHASE_SOUND_VOICECOMMS:	CG_Q3F_InitPhaseSoundVoiceComms();		break;
			case INITPHASE_GRAPHIC_STATIC:		CG_Q3F_InitPhaseGraphicStatic();		break;
			case INITPHASE_GRAPHIC_ITEMS:		CG_Q3F_InitPhaseGraphicItems();			break;
			case INITPHASE_GRAPHIC_WORLD:		CG_Q3F_InitPhaseGraphicWorld();			break;
			case INITPHASE_GRAPHIC_MODELS:		CG_Q3F_InitPhaseGraphicModels();		break;
			case INITPHASE_GRAPHIC_SHADERS:		CG_Q3F_InitPhaseGraphicShaders();		break;
			case INITPHASE_SPIRIT_STATIC:		CG_Q3F_InitPhaseSpiritStatic();			break;
			case INITPHASE_SPIRIT_DYNAMIC:		CG_Q3F_InitPhaseSpiritDynamic();		break;
			case INITPHASE_CLASSES:				CG_Q3F_InitPhaseClasses();				break;
			case INITPHASE_CLIENTS:				CG_Q3F_InitPhaseClients();				break;
			case INITPHASE_UISCRIPTING:			CG_Q3F_InitPhaseUIScripting();			break;
			case INITPHASE_SHADERREMAP:			CG_Q3F_InitPhaseShaderRemap();			break;
			case INITPHASE_SNAPSHOT:			CG_Q3F_InitPhaseSnapshot();				break;
			default:							CG_Q3F_SetInitPhase( INITPHASE_NONE );
												cgs.initScreenRendered = qtrue;
		}

		// Shortcut cycle while playing back demos.
		if( ( cg.demoPlayback || cg_classicinit.integer ) && cgs.initPhase != INITPHASE_NONE && cgs.initPhase != INITPHASE_SNAPSHOT )
			cgs.initScreenRendered = qfalse;
	}

	isUpdating = qfalse;
}
