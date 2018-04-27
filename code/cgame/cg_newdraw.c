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

#include "cg_local.h"
#include "cg_q3f_scanner.h"
#include "cg_q3f_menu.h"
#include "../game/bg_local.h"
#include "../ui_new/ui_shared.h"
#include "../game/bg_q3f_weapon.h"
#include "../game/bg_q3f_util.h"

// slothy
#define	DAMAGE_Q3F_SHELL			0x00000040	// Shell damage
#define DAMAGE_Q3F_NAIL				0x00000080	// Nail damage
#define DAMAGE_Q3F_EXPLOSION		0x00000100	// Explosive damage
#define DAMAGE_Q3F_SHOCK			0x00000200	// Electrical damage
#define	DAMAGE_Q3F_FIRE				0x00000400	// Fire damage
#define	DAMAGE_Q3F_MASK				(DAMAGE_Q3F_SHELL|DAMAGE_Q3F_NAIL|DAMAGE_Q3F_EXPLOSION|DAMAGE_Q3F_SHOCK|DAMAGE_Q3F_FIRE)

extern displayContextDef_t cgDC;
extern int menuCount;

/*
================
CG_DrawAttacker

================
*/
static void CG_DrawAttacker( rectDef_t* rect, float scale, int style, fontStruct_t* parentFont ) {
	vec3_t		angles;
	const char	*info;
	const char	*name;
	int			clientNum;
	static int	lastAttackerClass = 0;
	static int	lastAttackerNum = 0;

	if(!cg_drawAttacker.integer)
		return;

	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return;
	}

	if ( !cg.attackerTime ) {
		lastAttackerNum = 0;
		return;
	}

	clientNum = cg.predictedPlayerState.persistant[PERS_ATTACKER];
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS || clientNum == cg.snap->ps.clientNum ) {
		return;
	}

	if ( !cgs.clientinfo[ clientNum ].infoValid ) {
		cg.attackerTime = 0;
		return;
	}

	if( clientNum != lastAttackerNum) {
		lastAttackerNum = clientNum;
		lastAttackerClass = cgs.clientinfo[ clientNum ].cls;
	} else {
		if(lastAttackerClass != cgs.clientinfo[ clientNum ].cls) {
			cg.attackerTime = 0;
			return;
		}
	}

	if ( cg.time - cg.attackerTime > ATTACKER_HEAD_TIME ) {
		cg.attackerTime = 0;
		return;
	}

	angles[PITCH] = 0;
	angles[YAW] = 180;
	angles[ROLL] = 0;
	CG_DrawHead( rect->x, rect->y, rect->w, rect->h, clientNum, angles );

	info = CG_ConfigString( CS_PLAYERS + clientNum );
	name = Info_ValueForKey(  info, "n" );
	
	CG_Text_Paint(	rect->x + rect->w - (CG_Text_Width( name, scale, 0, parentFont ) * 0.5f),
					rect->y + rect->h + CG_Text_Height( name, scale, 0, parentFont),
					scale, colorWhite, name, 0, 0, style, parentFont, ITEM_ALIGN_CENTER );
}

static void CG_DrawPlayerArmorIcon( rectDef_t *rect ) {
	// ENSI PENTAGRAM LOGO if PW_PENTAGRAM
	CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.hud_armourShader );	// RR2DO2: another id bug
}

static void CG_DrawPlayerArmorValue(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	char	num[16];
	int		value;
	playerState_t	*ps;

	ps = &cg.snap->ps;

	if ( ps->powerups[PW_PENTAGRAM] )
		value = 666;
	else
		value = ps->stats[STAT_ARMOR];

	if (shader) {
		trap_R_SetColor( color );
		CG_DrawPic(rect->x, rect->y, rect->w, rect->h, shader);
		trap_R_SetColor( NULL );
	} else {
		Com_sprintf (num, sizeof(num), "%i", value);
		//value = CG_Text_Width(num, scale, 0, font);
		CG_Text_Paint(rect->x /*+ (rect->w - value) / 2*/ + text_x, rect->y + rect->h + text_y, scale, color, num, 0, 0, textStyle, font, textalignment);
	}
}

static void CG_DrawPlayerArmorBar(rectDef_t *rect) {
	vec4_t			hcolor;
	playerState_t	*ps;
	qboolean		pentagram;

	ps = &cg.snap->ps;

	pentagram = (ps->powerups[PW_PENTAGRAM] > 0) ? qtrue : qfalse;

	// get the armour colour
	hcolor[0] = hcolor[1] = hcolor[2] = 0.0f;
	hcolor[3] = 1.0f;
	if ( pentagram ) {
		hcolor[0] = 1.0f;
	}
	else {
		if ( ps->stats[STAT_ARMORTYPE] < Q3F_ARMOUR_YELLOW )
			hcolor[1] = 1.0f;
		else if ( ( ps->stats[STAT_ARMORTYPE] >= Q3F_ARMOUR_YELLOW ) && ( ps->stats[STAT_ARMORTYPE] < Q3F_ARMOUR_RED ) )
			hcolor[0] = hcolor[1] = 1.0f;
		else if ( ps->stats[STAT_ARMORTYPE] >= Q3F_ARMOUR_RED )
			hcolor[0] = 1.0f;
	}
	CG_Q3F_DrawProgress( rect->x, rect->y, rect->w, rect->h, 
						 bg_q3f_classlist[cg.predictedPlayerState.persistant[PERS_CURRCLASS]]->maxarmour,
						 bg_q3f_classlist[cg.predictedPlayerState.persistant[PERS_CURRCLASS]]->maxarmour,
						 pentagram ? 666 : ps->stats[STAT_ARMOR],
						 cgs.media.hud_armourShader,
						 //pentagram ? cgs.media.hud_armourShader : cgs.media.hud_armourShader,
						 hcolor);
}

// RR2DO2: some glaring bugs with player being hit is rescale head were in here
static void CG_DrawPlayerHead(rectDef_t *rect, qboolean draw2D) {
	vec3_t		angles;
	float		size, stretch;
	float		frac;
	float		x = rect->x;
	float		y = rect->y;

	VectorClear( angles );

	if ( cg.damageTime && cg.time - cg.damageTime < DAMAGE_TIME ) {
		frac = (float)(cg.time - cg.damageTime ) / DAMAGE_TIME;
		size = rect->w * ( 1.5 - frac * 0.5 );

		stretch = size - rect->w;
		// kick in the direction of damage
		x -= stretch * 0.5 + cg.damageX * stretch * 0.5;
		y -= stretch * 0.5 + cg.damageX * stretch * 0.5;	

		cg.headStartYaw = 180 + cg.damageX * 45;

		cg.headEndYaw = 180 + 20 * cos( Q_flrand(-1.0f, 1.0f)*M_PI );
		cg.headEndPitch = 5 * cos( Q_flrand(-1.0f, 1.0f)*M_PI );

		cg.headStartTime = cg.time;
		cg.headEndTime = cg.time + 100 + Q_flrand(0.0f, 1.0f) * 2000;
	} else {
		if ( cg.time >= cg.headEndTime ) {
			// select a new head angle
			cg.headStartYaw = cg.headEndYaw;
			cg.headStartPitch = cg.headEndPitch;
			cg.headStartTime = cg.headEndTime;
			cg.headEndTime = cg.time + 100 + Q_flrand(0.0f, 1.0f) * 2000;

			cg.headEndYaw = 180 + 20 * cos( Q_flrand(-1.0f, 1.0f)*M_PI );
			cg.headEndPitch = 5 * cos( Q_flrand(-1.0f, 1.0f)*M_PI );
		}

		size = rect->w;
	}

	// if the server was frozen for a while we may have a bad head start time
	if ( cg.headStartTime > cg.time ) {
		cg.headStartTime = cg.time;
	}

	frac = ( cg.time - cg.headStartTime ) / (float)( cg.headEndTime - cg.headStartTime );
	frac = frac * frac * ( 3 - 2 * frac );
	angles[YAW] = cg.headStartYaw + ( cg.headEndYaw - cg.headStartYaw ) * frac;
	angles[PITCH] = cg.headStartPitch + ( cg.headEndPitch - cg.headStartPitch ) * frac;

	CG_DrawHead( x, y, size, size, cg.snap->ps.clientNum, angles );
}

static void CG_DrawPlayerLocation( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	float alpha = color[3];

	if( cgs.currentLocationTime <= cg.time || cg.hyperspace )
	{
		if( !(cgs.currentLocation = CG_Q3F_GetLocation( cg.predictedPlayerState.origin, qtrue )) )
			cgs.currentLocation = "Unknown Location";
		cgs.currentLocationTime = cg.time + 500;
	}

	CG_Q3F_GetTeamColor( color, cg.snap->ps.persistant[PERS_TEAM] );
	color[3] = alpha;

	CG_Text_Paint( rect->x + text_x, rect->y + rect->h + text_y, scale, color, cgs.currentLocation, 0, 0, textStyle, font, textalignment );
}

static void CG_DrawPlayerScore( rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char num[16];
	int value = cg.snap->ps.persistant[PERS_SCORE];

	if (shader) {
		trap_R_SetColor( color );
		CG_DrawPic(rect->x, rect->y, rect->w, rect->h, shader);
		trap_R_SetColor( NULL );
	} else {
		Com_sprintf (num, sizeof(num), "%i", value);
		value = CG_Text_Width(num, scale, 0, font);
		CG_Text_Paint(rect->x + (rect->w - value) / 2 + text_x, rect->y + rect->h + text_y, scale, color, num, 0, 0, textStyle, font, textalignment);
	}
}

static void CG_DrawPlayerItem( rectDef_t *rect, float scale, qboolean draw2D) {
	int		value;
	vec3_t origin, angles;

	value = cg.snap->ps.stats[STAT_HOLDABLE_ITEM];
	if ( value ) {
		CG_RegisterItemVisuals( value );

		if (qtrue) {
			CG_RegisterItemVisuals( value );
			CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cg_items[ value ].icon );
		} else {
 			VectorClear( angles );
			origin[0] = 90;
			origin[1] = 0;
			origin[2] = -10;
			angles[YAW] = ( cg.time & 2047 ) * 360 / 2048.0;
			CG_Draw3DModel(rect->x, rect->y, rect->w, rect->h, cg_items[ value ].models[0], 0, origin, angles );
		}
	}
}

static void CG_DrawPlayerHealth(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	playerState_t	*ps;
	int value;
	char num[16];

	ps = &cg.snap->ps;

	value = ps->stats[STAT_HEALTH];

	if (shader) {
		trap_R_SetColor( color );
		CG_DrawPic(rect->x, rect->y, rect->w, rect->h, shader);
		trap_R_SetColor( NULL );
	} else {
		Com_sprintf (num, sizeof(num), "%i", value);
		//value = CG_Text_Width(num, scale, 0, font);
		CG_Text_Paint(rect->x/* + (rect->w - value) / 2*/ + text_x, rect->y + rect->h + text_y, scale, color, num, 0, 0, textStyle, font, textalignment);
	}
}

static void CG_DrawPlayerHealthBar(rectDef_t *rect, vec4_t color ) {
	playerState_t	*ps;

	ps = &cg.snap->ps;

	CG_Q3F_DrawProgress( rect->x, rect->y, rect->w, rect->h, 
						 bg_q3f_classlist[cg.predictedPlayerState.persistant[PERS_CURRCLASS]]->maxhealth,
						 bg_q3f_classlist[cg.predictedPlayerState.persistant[PERS_CURRCLASS]]->maxhealth/*+50*/,
						 ps->stats[STAT_HEALTH],
						 cgs.media.hud_healthShader,
						 color);
}

static void CG_DrawTeamScore( q3f_team_t team, rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char num[16];

	if ( cg.teamScores[team - Q3F_TEAM_RED] == SCORE_NOT_PRESENT ) {
		Com_sprintf (num, sizeof(num), "-");
	}
	else {
		Com_sprintf (num, sizeof(num), "%i", ( cg.teamScores[team - Q3F_TEAM_RED] > 9999 ? 9999 : cg.teamScores[team - Q3F_TEAM_RED] ) );
	}
	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, num, 0, 0, textStyle, font, textalignment);
}

static void CG_DrawTeamName( q3f_team_t team, rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char buffer[64];
	trap_Cvar_VariableStringBuffer(va("cg_%steam", teamnames[team-1]), buffer, 64);
	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, buffer, 0, 0, textStyle, font, textalignment);
}

static void CG_DrawTeamCount( q3f_team_t team, rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char num[16];

	if( cg.teamCounts[team - Q3F_TEAM_RED] ) {
		qboolean	balancewarningflash;

		balancewarningflash = cg.time & 512;

		if( !balancewarningflash || (cg.teamBalanceWarnings & 0x0007) != team )
		{
			// (not) low player count
			if( balancewarningflash && (cg.teamBalanceWarnings & 0x00F8) == ( team << 4) )
			{
				// High player count
				color[0] = color[1] = color[3] = 1.0;
				color[2] = 0;
			}
		}
		Com_sprintf( num, sizeof(num), "%i", cg.teamCounts[team - Q3F_TEAM_RED] );
	} else {
		Com_sprintf( num, sizeof(num), "0" );
	}

	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, num , 0, 0, textStyle, font, textalignment);
}

static void CG_DrawAmmoSlotValue( int ammoslot, rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char			num[16];

	Com_sprintf( num, sizeof(num), "%i", cg.snap->ps.ammo[ammoslot] );

	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, num , 0, 0, textStyle, font, textalignment);
}

static void CG_DrawCurrentAmmoIcon( rectDef_t *rect ) {
	vec4_t			hcolor;
	centity_t		*cent;

	if(cg.snap->ps.weaponstate == WEAPON_RELOADING) {
		if((cg.time / 200) & 1) {
			return;
		}
	}

	cent = &cg_entities[cg.snap->ps.clientNum];

	switch( Q3F_GetAmmoTypeForWeapon( cent->currentState.weapon ) ) {
	case AMMO_SHELLS:	VectorSet4( hcolor, 1.f, 1.f, 0.f, 1.f );
						trap_R_SetColor( hcolor );
						CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.hud_icon_shells );
						trap_R_SetColor( NULL );
						break;
	case AMMO_NAILS:	VectorSet4( hcolor, 0.f, 1.f, 0.f, 1.f );
						trap_R_SetColor( hcolor );
						CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.hud_icon_nails );
						trap_R_SetColor( NULL );
						break;
	case AMMO_ROCKETS:	VectorSet4( hcolor, 1.f, 0.f, 0.f, 1.f );
						trap_R_SetColor( hcolor );
						CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.hud_icon_rockets );
						trap_R_SetColor( NULL );
						break;
	case AMMO_CELLS:	VectorSet4( hcolor, 1.f, 0.f, 1.f, 1.f );
						trap_R_SetColor( hcolor );
						CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.hud_icon_cells );
						trap_R_SetColor( NULL );
						break;
	}

	if( cent->currentState.weapon == WP_AXE ) {
		switch(cg.predictedPlayerState.persistant[PERS_CURRCLASS]) {
		case Q3F_CLASS_PARAMEDIC:	VectorSet4( hcolor, 0.4f, 0.75f, 0.13f, 1.f );
									trap_R_SetColor( hcolor );
									CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.hud_healthShader );
									trap_R_SetColor( NULL );
									break;
		case Q3F_CLASS_ENGINEER:	VectorSet4( hcolor, 1.f, 0.f, 1.f, 1.f );
									trap_R_SetColor( hcolor );
									CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.hud_icon_cells );
									trap_R_SetColor( NULL );
									break;
		}
	}
}

static void CG_DrawCurrentAmmoValue( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char			num[16];
	int				ammocount;
	bg_q3f_weapon_t	*wp;
	centity_t		*cent;
	playerState_t	*ps;

	cent = &cg_entities[cg.snap->ps.clientNum];
	ps = &cg.snap->ps;

	// Golliwog: Some weapons use more than one ammo type, or more than one unit.
	wp = BG_Q3F_GetWeapon( cent->currentState.weapon );
	ammocount = ps->ammo[wp->ammotype];
	if( wp->numammo )
		ammocount /= wp->numammo;

	if( cent->currentState.weapon == WP_AXE ) {
		switch(cg.predictedPlayerState.persistant[PERS_CURRCLASS]) {
		case Q3F_CLASS_PARAMEDIC:	ammocount = cg.snap->ps.ammo[AMMO_MEDIKIT];
									break;
		case Q3F_CLASS_ENGINEER:	ammocount = cg.snap->ps.ammo[AMMO_CELLS];
									break;
		default:					ammocount = -1;
									break;
		}
	}

	if( ammocount >= 0 ) {
		Com_sprintf( num, sizeof(num), "%i", ammocount );

		CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, num , 0, 0, textStyle, font, textalignment);
	}
}

static void CG_DrawCurrentClipValue( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char			num[16];
	int				clipcount = -1;
	playerState_t	*ps;

	ps = &cg.snap->ps;

	clipcount = Q3F_GetClipValue( cg.predictedPlayerState.weapon, ps );

	if( clipcount >= 0 ) {
		Com_sprintf( num, sizeof(num), "%i", clipcount );

		CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, num , 0, 0, textStyle, font, textalignment);
	}
}

static void CG_DrawGrenadeValue( int grenNum, rectDef_t *rect, float scale, vec4_t color, vec4_t backcolor, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char					num[16];
	int						grenadecount = 0;
	bg_q3f_playerclass_t	*cls;
	qboolean				bc = qfalse;

	cls = BG_Q3F_GetClass( &cg.snap->ps );

	if( grenNum == 1 ) {
		grenadecount = cg.snap->ps.ammo[AMMO_GRENADES] & 0xFF;

		if ( cg.grenadeprimeTime && cg.grenadetype == cls->gren1type && ( cg.time & 256 ) ) {
			bc = qtrue; //VectorSet( color, 1.f, 1.f, 1.f );
		}
	} else if( grenNum == 2 ) {
		grenadecount = ( cg.snap->ps.ammo[AMMO_GRENADES] & 0xFF00 ) >> 8;

		if ( cg.grenadeprimeTime && cg.grenadetype == cls->gren2type && ( cg.time & 256 ) ) {
			bc = qtrue; //VectorSet( color, 1.f, 1.f, 1.f );
		}
	}

	Com_sprintf( num, sizeof(num), "%i", grenadecount );

	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, bc ? backcolor: color, num , 0, 0, textStyle, font, textalignment);
}

void CG_DrawChargeCountdown(rectDef_t * rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t * font)
{
	char            num[16];
	int				count = 0;
	vec4_t          clr, newclr;

	VectorCopy4(color, newclr);

	if(cg.snap->ps.ammoclip[3] > 0 && cg.snap->ps.ammoclip[2] >= 0)
	{
		count = cg.snap->ps.ammoclip[2] > 0 ? cg.snap->ps.ammoclip[2] : cg.snap->ps.ammoclip[3];
		Com_sprintf(num, 16, "%i secs...", count);
		if(count <= 10)
		{
			clr[0] = 0.8 * colorRed[0];
			clr[1] = 0.8 * colorRed[1];
			clr[2] = 0.8 * colorRed[2];
			clr[3] = 0.8 * colorRed[3];
			LerpColor(colorRed, clr, newclr, 0.5 + 0.5 * sin(cgDC.realTime / PULSE_DIVISOR));
		}
		CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, newclr, num, 0, 0, textStyle, font, textalignment);
	}
}

void CG_DrawChargeModel( rectDef_t *rect )
{
	int ammo = cg.snap->ps.ammo[AMMO_CHARGE];
	static qhandle_t model = 0;
	vec3_t mins, maxs, origin, angles;
	refdef_t refdef;
	refEntity_t ent;
	float x, y, w, h;
	float len;

	if(cg.predictedPlayerState.persistant[PERS_CURRCLASS] != Q3F_CLASS_GRENADIER)
		return;

	if(!model) {
		model = trap_R_RealRegisterModel("models/objects/charge/charge.md3");
		if(!model)
			model = -1;
	}

	if(cg.snap->ps.ammoclip[3] > 0 && cg.snap->ps.ammoclip[2] >= 0)
	{
		CG_DrawChargeCountdown(rect, 0.25f, colorWhite, ITEM_TEXTSTYLE_SHADOWED, ITEM_ALIGN_CENTER, 0, -20, NULL);
	}

	if((model > 0) && (ammo || (cg.snap->ps.ammoclip[3] > 0 && cg.snap->ps.ammoclip[2] >= 0))) {
		VectorClear( angles );
		// offset the origin y and z to center the flag
		trap_R_ModelBounds( model, mins, maxs );
		origin[2] = -0.5 * ( mins[2] + maxs[2] );
		origin[1] = 0.5 * ( mins[1] + maxs[1] );
		// calculate distance so the flag nearly fills the box
		// assume heads are taller than wide
		len = 1.5 * ( maxs[2] - mins[2] );		
		origin[0] = len / 0.268;	// len / tan( fov/2 )
		angles[YAW] = 60 * sin( 1 * 30 + cg.time / 2000.0 );
		angles[PITCH] = 300;
		//		CG_Draw3DModel( x, y, w, h, cm, 0, origin, angles );

		x	= rect->x;
		y	= rect->y;
		w	= rect->w;
		h	= rect->h;

		CG_AdjustFrom640( &x, &y, &w, &h );

		memset( &refdef, 0, sizeof( refdef ) );
		memset( &ent, 0, sizeof( ent ) );
		AnglesToAxis( angles, ent.axis );
		VectorCopy( origin, ent.origin );
		ent.hModel = model;
		ent.renderfx = RF_NOSHADOW|RF_MINLIGHT|RF_DEPTHHACK;		// no stencil shadows
		ent.shaderRGBA[0] = 2;
		ent.shaderRGBA[1] = 2;
		ent.shaderRGBA[2] = 2;
		ent.shaderRGBA[3] = 0xFF;

		trap_R_ClearScene();
		trap_R_AddRefEntityToScene( &ent, NULL );
		refdef.rdflags = RDF_NOWORLDMODEL;
		AxisClear( refdef.viewaxis );
		refdef.fov_x = 30;
		refdef.fov_y = 30;
		refdef.x = x;
		refdef.y = y;
		refdef.width = w;
		refdef.height = h;
		refdef.time = cg.time;
		trap_R_RenderScene( &refdef );
	}
}


/*void CG_DrawChargeIcon( rectDef_t *rect )
{
	int ammo = cg.snap->ps.ammo[AMMO_CHARGE];

	if(cg.predictedPlayerState.persistant[PERS_CURRCLASS] != Q3F_CLASS_GRENADIER)
		return;

	if( ammo )
		//CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.hud_icon_charge );
		;
}*/


static void CG_DrawTeamPing( q3f_team_t team, rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char num[16];

	if( cg.teamCounts[team - Q3F_TEAM_RED] ) {
		qboolean	balancewarningflash;

		balancewarningflash = cg.time & 512;

		if( !balancewarningflash || (cg.teamBalanceWarnings & 0x0007) != team  )
		{
			// (not) low player count
			if( balancewarningflash && (cg.teamBalanceWarnings & 0x00F8) == ( team  << 4) )
			{
				// High player count
				color[0] = color[1] = color[3] = 1.0;
				color[2] = 0;
			}
		}
		Com_sprintf( num, sizeof(num), "%i", cg.teamPings[team - Q3F_TEAM_RED] );
	} else {
		Com_sprintf( num, sizeof(num), "0" );
	}

	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, num , 0, 0, textStyle, font, textalignment);
}

static void CG_DrawTeamColor( rectDef_t *rect, vec4_t color ) {
	CG_DrawTeamBackground( rect->x, rect->y, rect->w, rect->h, color[3], cg.snap->ps.persistant[PERS_TEAM] );
}

#define	LAG_SAMPLES		128

typedef struct {
	int		frameSamples[LAG_SAMPLES];
	int		frameCount;
	int		snapshotFlags[LAG_SAMPLES];
	int		snapshotSamples[LAG_SAMPLES];
	int		snapshotCount;
} lagometer_t;

lagometer_t		lagometer;

/*
==============
CG_AddLagometerFrameInfo

Adds the current interpolate / extrapolate bar for this frame
==============
*/
void CG_AddLagometerFrameInfo( void ) {
	int			offset;

	offset = cg.time - cg.latestSnapshotTime;
	lagometer.frameSamples[ lagometer.frameCount & ( LAG_SAMPLES - 1) ] = offset;
	lagometer.frameCount++;
}

/*
==============
CG_AddLagometerSnapshotInfo

Each time a snapshot is received, log its ping time and
the number of snapshots that were dropped before it.

Pass NULL for a dropped packet.
==============
*/
void CG_AddLagometerSnapshotInfo( snapshot_t *snap ) {
	// dropped packet
	if ( !snap ) {
		lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = -1;
		lagometer.snapshotCount++;
		return;
	}

	// add this snapshot's info
	lagometer.snapshotSamples[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->ping;
	lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] = snap->snapFlags;
	if ( snap->ps.persistant[PERS_FLAGS] & PF_SKIPPEDFRAME )
		lagometer.snapshotFlags[ lagometer.snapshotCount & ( LAG_SAMPLES - 1) ] |= SNAPFLAG_NOUSERCMD;
	lagometer.snapshotCount++;
}

/*
==============
CG_DrawDisconnect

Should we draw something differnet for long lag vs no packets?
==============
*/
static void CG_DrawDisconnect( rectDef_t *rect ) {
	int			cmdNum;
	usercmd_t	cmd;

	if ( cg.demoPlayback && ( cg_timescale.value != 1.0f || cgs.demoPaused || cgs.timescaleUpdate > cg.time ) ) {
		return;
	}

	// ydnar: don't draw if the server is respawning
	if ( cg.serverRespawning ) {
		return;
	}

	// draw the phone jack if we are completely past our buffers
	cmdNum = trap_GetCurrentCmdNumber() - CMD_BACKUP + 1;
	trap_GetUserCmd( cmdNum, &cmd );
	if ( cmd.serverTime <= cg.snap->ps.commandTime
		|| cmd.serverTime > cg.time ) {	// special check for map_restart
		return;
	}

	// also add text in center of screen
	CG_LowPriority_Printf(BOX_PRINT_MODE_CENTER, "Connection Interrupted");

	// blink the icon
	if ( ( cg.time >> 9 ) & 1 ) {
		return;
	}

	CG_DrawPic( rect->x, rect->y, rect->w, rect->h, cgs.media.disconnectIcon );
}

#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300

static void CG_DrawLagometer( rectDef_t *rect ) {
	int		a/*, x, y*/, i;
	float	v;
	float	ax, ay, aw, ah, mid, range;
	int		color;
	float	vscale;
	vec4_t	hcolor;

	// slothy
	if(cg_lagometer.integer == 0)
		return;

	VectorSet4( hcolor, 1, 1, 1, 1 );

	//
	// draw the graph
	//

	trap_R_SetColor( NULL );

	if ( cg_lagometer.integer ) {
		ax = rect->x;
		ay = rect->y;
		aw = rect->w;
		ah = rect->h;
		CG_AdjustFrom640( &ax, &ay, &aw, &ah );

		color = -1;
		range = ah / 3;
		mid = ay + range;

		vscale = range / MAX_LAGOMETER_RANGE;

		// draw the frame interpoalte / extrapolate graph
		for ( a = 0 ; a < aw ; a++ ) {
			i = ( lagometer.frameCount - 1 - a ) & (LAG_SAMPLES - 1);
			v = lagometer.frameSamples[i];
			v *= vscale;
			if ( v > 0 ) {
				if ( color != COLOR_YELLOW ) {
					color = COLOR_YELLOW;
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
				}
				if ( v > range ) {
					v = range;
				}
				trap_R_DrawStretchPic ( ax + aw - a, mid - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
			} else if ( v < 0 ) {
				if ( color != COLOR_BLUE ) {
					color = COLOR_BLUE;
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_BLUE)] );
				}
				v = -v;
				if ( v > range ) {
					v = range;
				}
				trap_R_DrawStretchPic( ax + aw - a, mid, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
			}
		}
		// draw the snapshot latency / drop graph
		range = ah / 2;
		vscale = range / MAX_LAGOMETER_PING;

		for ( a = 0 ; a < aw ; a++ ) {
			i = ( lagometer.snapshotCount - 1 - a ) & (LAG_SAMPLES - 1);
			v = lagometer.snapshotSamples[i];
			if ( v > 0 ) {
				if ( lagometer.snapshotFlags[i] & SNAPFLAG_NOUSERCMD ) {
					if ( color != COLOR_ORANGE ) {
						color = COLOR_ORANGE;	// ORANGE for missing UCMDS
						trap_R_SetColor( g_color_table[ColorIndex(COLOR_ORANGE)] );
					}
				} else if ( lagometer.snapshotFlags[i] & SNAPFLAG_RATE_DELAYED ) {
					if ( color != COLOR_YELLOW ) {
						color = COLOR_YELLOW;	// YELLOW for rate delay
						trap_R_SetColor( g_color_table[ColorIndex(COLOR_YELLOW)] );
					}
				} else {
					if ( color != COLOR_GREEN ) {
						color = COLOR_GREEN;
						trap_R_SetColor( g_color_table[ColorIndex(COLOR_GREEN)] );
					}
				}
				v = v * vscale;
				if ( v > range ) {
					v = range;
				}
				trap_R_DrawStretchPic( ax + aw - a, ay + ah - v, 1, v, 0, 0, 0, 0, cgs.media.whiteShader );
			} else if ( v < 0 ) {
				if ( color != COLOR_RED ) {
					color = COLOR_RED;		// RED for dropped snapshots
					trap_R_SetColor( g_color_table[ColorIndex(COLOR_RED)] );
				}
				trap_R_DrawStretchPic( ax + aw - a, ay + ah - range, 1, range, 0, 0, 0, 0, cgs.media.whiteShader );
			}
		}
		trap_R_SetColor( NULL );
		if ( cg_nopredict.integer || cgs.synchronousClients ) {
			CG_Text_Paint( rect->x + ( rect->w / 2 ), rect->y + ( rect->h / 2 ) + 0.1f * rect->h, 0.2f, hcolor, "snc", 0, 0, 0, NULL, ITEM_ALIGN_CENTER );
		}
	} else {
		CG_Text_Paint( rect->x + ( rect->w / 2 ), rect->y + ( rect->h / 2 ) + 0.1f * rect->h, 0.2f, hcolor, "off", 0, 0, 0, NULL, ITEM_ALIGN_CENTER );
	}
	CG_DrawDisconnect( rect );
}

static void CG_DrawSpeedometer( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int linespace ) {
	char		num[32];
	float		z;
	int			lineheight;

	// slothy
	if(cg_drawSpeedometer.integer == 0)
		return;

	lineheight = linespace;
	if(!lineheight)
		lineheight = 10;

	z = cg.predictedPlayerState.velocity[2];
	cg.predictedPlayerState.velocity[2] = 0.0;
	Com_sprintf( num, sizeof(num), "Vel: %4.0f", VectorLength(cg.predictedPlayerState.velocity) );
	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, num , 0, 0, textStyle, font, textalignment);
	cg.predictedPlayerState.velocity[2] = z;
}	

#define	FPS_FRAMES	4

static void CG_DrawFPS( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char		num[32];
	static int	previousTimes[FPS_FRAMES];
	static int	index;
	int			i, total;
	int			fps;
	static		int	previous;
	int			t, frameTime;

	// slothy
	if(cg_drawFPS.integer == 0)
		return;

	// don't use serverTime, because that will be drifting to
	// correct for internet lag changes, timescales, timedemos, etc
	t = trap_Milliseconds();
	frameTime = t - previous;
	previous = t;

	previousTimes[index % FPS_FRAMES] = frameTime;
	index++;
	if ( index > FPS_FRAMES ) {
		// average multiple frames together to smooth changes out a bit
		total = 0;
		for ( i = 0 ; i < FPS_FRAMES ; i++ ) {
			total += previousTimes[i];
		}
		if ( !total ) {
			total = 1;
		}
		fps = 1000 * FPS_FRAMES / total;

		Com_sprintf( num, sizeof(num), "FPS: %4.0f", (float)fps );
//		Com_sprintf( num, sizeof(num), "Vel: %f", VectorLength(cg.predictedPlayerState.velocity) );
		CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, num , 0, 0, textStyle, font, textalignment);
	}
}	

static void CG_DrawTimer( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char		num[32];
	int			mins, seconds, tens;
	int			msec;

	// Slothy
	if(cg_drawTimer.integer == 0)
		return;

	if (cg.matchState == MATCH_STATE_WARMUP && cg.warmup > 0 ) {
		msec = (cg.warmup - cg.time) / 1000;
		Com_sprintf( num, sizeof(num), "WU: %i", msec + 1 );
		if ( msec < 10 && msec & 1)
			color = colorRed;
	} else {
		if( cg.matchState > MATCH_STATE_PLAYING || cg_drawTimer.integer > 1 || !cgs.timelimit )
			msec = cg.time - cgs.levelStartTime;
		else {
			msec = cgs.levelStartTime + cgs.timelimit * 60000 - cg.time;
		if( msec < 0 )
			msec = 0;
		}
        seconds = msec / 1000;
		mins = seconds / 60;
		seconds -= mins * 60;
		tens = seconds / 10;
		seconds -= tens * 10;

		Com_sprintf( num, sizeof(num), "T: %i:%i%i", mins, tens, seconds );
	}
	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, num , 0, 0, textStyle, font, textalignment);
}

static void CG_DrawSentryCam( rectDef_t *rect ) {
	refdef_t sentrycam_refdef;
	float x, y, w, h, x2, y2, w2, h2;
	vec4_t scr_color, lensflare_blinded;

	if( cgs.eventHandling == CGAME_EVENT_EDITHUD ) {
		return;
	}

	w2 = w = rect->w;
	h2 = h = rect->h;//w / cgs.glconfig.windowAspect;
	x2 = x = rect->x;
	y2 = y = rect->y;//( cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF00 ? 374 : 389 ) - h;

	if(cg.sentryCamTime_end) {
		float elapsed = cg.time - cg.sentryCamTime_end;

		if(elapsed > Q3F_SENTRYCAM_BLINKTIME/2) {
			float adjust = ((elapsed*2/Q3F_SENTRYCAM_BLINKTIME)-1)*(w2/2.f);
			x += adjust;
			w -= adjust*2;

			y += h2/2;
			h = 2;

			CG_DrawPic( x, y, w, h, cgs.media.sentryTvFx);
		}
		else
		{
			float adjust = ((2*elapsed/Q3F_SENTRYCAM_BLINKTIME))*(h2/2.f);
			y += adjust;
			h -= adjust*2;

			CG_DrawPic( x, y, w, h, cgs.media.sentryTvFx);
		}

		if(elapsed > Q3F_SENTRYCAM_BLINKTIME) {
			cg.sentryCamTime_end = 0;
		}

		return;
	}
    
	if(!cg.sentryCam_on) {
		cg.sentryCam_on = qtrue;
		cg.sentryCamTime = cg.time;
	}

	if(cg.sentryCamTime) {
		float elapsed = cg.time - cg.sentryCamTime;

		if(elapsed > Q3F_SENTRYCAM_BLINKTIME/2) {
			float adjust = (2-(2*elapsed/Q3F_SENTRYCAM_BLINKTIME))*(h2/2.f);
			y += adjust;
			h -= adjust*2;

			CG_DrawPic( x, y, w, h, cgs.media.sentryTvFx);
		} else {
			float adjust = (1-(elapsed*2/Q3F_SENTRYCAM_BLINKTIME))*(w2/2.f);
			x += adjust;
			w -= adjust*2;

			y += h2/2;
			h = 2;

			CG_DrawPic( x, y, w, h, cgs.media.sentryTvFx);
		}

		if(elapsed > Q3F_SENTRYCAM_BLINKTIME) {
			cg.sentryCamTime = 0;
		}
	} else {
		memcpy( &sentrycam_refdef, &cg.refdef, sizeof( cg.refdef ) );
		cg.currentrefdef = &sentrycam_refdef;

		CG_AdjustFrom640( &x2, &y2, &w2, &h2 );

		sentrycam_refdef.width = w2;
		sentrycam_refdef.width &= ~1;

		sentrycam_refdef.height = h2;
		sentrycam_refdef.height &= ~1;

		sentrycam_refdef.x = x2;
		sentrycam_refdef.y = y2;
  
		VectorCopy( cg.sentrycam_origin, sentrycam_refdef.vieworg );
		AnglesToAxis( cg.sentrycam_angles, sentrycam_refdef.viewaxis  );

		VectorMA( sentrycam_refdef.vieworg, 52, sentrycam_refdef.viewaxis[2], sentrycam_refdef.vieworg );

		sentrycam_refdef.fov_x = 90;
		sentrycam_refdef.fov_y = 73.7398f;
		//sentrycam_refdef.fov_x = 110;
		//sentrycam_refdef.fov_y = 90.1264f;

		// reset lensflare blinding
	
		VectorSet( lensflare_blinded, 0.f, 0.f, 0.f );
		lensflare_blinded[3] = 0.f;
  
		// set flare rendering to sentrycam refdef
		CG_SetFlareRenderer( &sentrycam_refdef, &lensflare_blinded );

		// RR2DO2: draw fake sky
		scr_color[0] = 0.f;
		scr_color[1] = 0.f;
		scr_color[2] = 0.f;
		scr_color[3] = .5f;
		//CG_DrawPic( x, y, w, h, cgs.media.teleportEffectShader );
		CG_FillRect( x, y, w, h, scr_color );

		cg.rendering2ndRefDef = qtrue;

		// RR2DO2: Render sunflares - NOTE: don't forget the 2nd line!
		CG_RenderSunflares();
		CG_SetFlareFader( NULL, NULL );
		// RR2DO2

		CG_AddPacketEntities();						// alter calcViewValues, so predicted player state is correct
//		CG_AddMarks();
		CG_AddLocalEntities();
//		CG_AddAtmosphericEffects();	                  // Keeger:  new atmospherics
		Spirit_AddParticles();
		cg.rendering2ndRefDef = qfalse;

		// set flare rendering to main refdef
		CG_SetFlareRenderer( &cg.refdef, &cg.lensflare_blinded );

		trap_R_RenderScene( &sentrycam_refdef );

		// Lensflare blinding
		if( lensflare_blinded[3] ) {
			CG_FillRectAdditive( x, y, w, h, lensflare_blinded );
		}

		// Set to NULL so we don't try to draw the cam when the sentrycam is gone.
		cg.sentrycam_entityState = NULL;
  
		cg.currentrefdef = &cg.refdef;

		if( cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF ) {
			if( cg.sentryHealth != (cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF) ) {

				if( cg.sentryHealth > (cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF) ) {
					cg.sentryDamageTime = cg.time + 1000;
				}

				cg.sentryHealth = (cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF);
			}

			//FIXME: get the actual sentry max health, and work as a percentage
			if(cg.sentryHealth <= 100) {
				vec4_t  alpha = {1.0f, 1.0f, 1.0f, 0.0f};
				alpha[3] = 1 - ((cg.sentryHealth + (rand()%10)) / 110.0f);

//				CG_Printf("alpha: %f\n", alpha[3]);

				trap_R_SetColor( alpha );
				CG_DrawPic( x, y, w, h, cgs.media.sentryTvFx);
				trap_R_SetColor( NULL );
			}
		}
	}
}

static void CG_DrawSentryHealth( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char	num[16];

	if( cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF ) {
		if( cg.sentryHealth != (cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF) ) {

			if( cg.sentryHealth > (cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF) ) {
				cg.sentryDamageTime = cg.time + 1000;
			}

			cg.sentryHealth = (cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF);
		}

		if( cg.sentryDamageTime >= cg.time ) {
			VectorSet( color, 1.f, 0.f, 0.f );
		} else if( cg.sentryHealth < 75 ) {
			VectorSet( color, 1.f, 1.f, 0.f );
		}

		Com_sprintf( num, sizeof(num), "%i", cg.sentryHealth );
		CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, num , 0, 0, textStyle, font, textalignment);
	}
}

static void CG_DrawSentryBullets( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char	num[16];

	if( cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF ) {
		Com_sprintf( num, sizeof(num), "%i", cg.snap->ps.ammo[AMMO_Q3F_ENGDATA2] & 0xFF );
		CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, num , 0, 0, textStyle, font, textalignment);
	}
}

static void CG_DrawSentryRockets( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char	num[16];

	if( cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF ) {
		Com_sprintf( num, sizeof(num), "%i", ((cg.snap->ps.ammo[AMMO_Q3F_ENGDATA2] & 0xFF00) >> 8) );
		CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, num , 0, 0, textStyle, font, textalignment);
	}
}

static void CG_DrawSupStationHealth( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char	num[16];

	if( cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF00 ) {
		if( cg.supplystationHealth != ((cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF00) >> 8) ) {

			if( cg.supplystationHealth > ((cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF00) >> 8) ) {
				cg.supplystationDamageTime = cg.time + 1000;
			}

			cg.supplystationHealth = ((cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF00) >> 8);
		}

		if( cg.supplystationDamageTime >= cg.time ) {
			VectorSet( color, 1.f, 0.f, 0.f );
		} else if( cg.supplystationHealth < 75 ) {
			VectorSet( color, 1.f, 1.f, 0.f );
		}

		Com_sprintf( num, sizeof(num), "%i", cg.supplystationHealth );
		CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, num , 0, 0, textStyle, font, textalignment);
	}
}

int CG_GetCurrentPowerUp( int *pups ) {
	int		sorted[MAX_POWERUPS];
	int		sortedTime[MAX_POWERUPS];
	int		i, j, k;
	int		active;
	playerState_t	*ps;
	int		t;

	ps = &cg.snap->ps;

	if ( ps->stats[STAT_HEALTH] <= 0 ) {
		return -1;
	}

	// sort the list by time remaining
	active = 0;
	for ( i = PW_QUAD ; i <= PW_FLIGHT ; i++ ) { // TODO PENTAGRAM
		if ( !ps->powerups[ i ] ) {
			continue;
		}
		t = ps->powerups[ i ] - cg.time;
		// ZOID--don't draw if the power up has unlimited time (999 seconds)
		// This is true of the CTF flags
		if ( t <= 0 || t >= 999000) {
			continue;
		}

		// insert into the list
		for ( j = 0 ; j < active ; j++ ) {
			if ( sortedTime[j] >= t ) {
				for ( k = active - 1 ; k >= j ; k-- ) {
					sorted[k+1] = sorted[k];
					sortedTime[k+1] = sortedTime[k];
				}
				break;
			}
		}
		sorted[j] = i;
		sortedTime[j] = t;
		active++;
	}

	if(active) {
		for(j = 0; j < active; j++) {
			pups[j] = sorted[j];
		}
		return active;
	} else {
		return -1;
	}
}

float CG_GetValue(int ownerDraw) {
	switch (ownerDraw) {
	case CG_PLAYER_SCORE:
				return cg.snap->ps.persistant[PERS_SCORE];
				break;
	case CG_PLAYER_HEALTH:
				return cg.snap->ps.stats[STAT_HEALTH];
				break;
	case CG_RED_SCORE:
				return cg.teamScores[0];
				break;
	case CG_BLUE_SCORE:
				return cg.teamScores[1];
				break;
	case CG_YELLOW_SCORE:
				return cg.teamScores[2];
				break;
	case CG_GREEN_SCORE:
				return cg.teamScores[3];
				break;
	default:
				break;
	}
	return -1;
}

// THINKABOUTME: should these be exclusive or inclusive.. 
// 
qboolean CG_OwnerDrawVisible(int flags) {
	if( cgs.eventHandling == CGAME_EVENT_EDITHUD ) {
		return qtrue;
	}

	if( flags & CG_SHOW_CHATEDIT ) {
		if (cgs.eventHandling != CGAME_EVENT_MESSAGEMODE )
			return qfalse;
	}

	if( flags & CG_SHOW_GREN_PRIMED ) {
		if( (cg.grenadeHudTime + Q3F_GRENADE_PRIME_TIME < cg.time) || !cg_showGrenadeTimer1.integer ) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_GREN_PRIMED2 ) {
		if( (cg.grenadeHudTime + Q3F_GRENADE_PRIME_TIME < cg.time) || !cg_showGrenadeTimer2.integer ) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_ALERT_ICON ) {
		if( cg.q3f_alerticontime[0] < cg.time ) {
			return qfalse;
		}
	}

	// RR2DO2: kind of a hack, see if this team has a score present, if so, it exists
	if( flags & CG_SHOW_RED_TEAM_EXISTS ) {
		if( !(cgs.teams & ( 1 << Q3F_TEAM_RED )) ) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_BLUE_TEAM_EXISTS ) {
		if( !(cgs.teams & ( 1 << Q3F_TEAM_BLUE )) ) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_YELLOW_TEAM_EXISTS ) {
		if( !(cgs.teams & ( 1 << Q3F_TEAM_YELLOW )) ) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_GREEN_TEAM_EXISTS ) {
		if( !(cgs.teams & ( 1 << Q3F_TEAM_GREEN )) ) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_RED_TEAM_NOT_EXISTS ) {
		if( (cgs.teams & ( 1 << Q3F_TEAM_RED )) ) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_BLUE_TEAM_NOT_EXISTS ) {
		if( (cgs.teams & ( 1 << Q3F_TEAM_BLUE )) ) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_YELLOW_TEAM_NOT_EXISTS ) {
		if( (cgs.teams & ( 1 << Q3F_TEAM_YELLOW )) ) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_GREEN_TEAM_NOT_EXISTS ) {
		if( (cgs.teams & ( 1 << Q3F_TEAM_GREEN )) ) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_ON_TEAM ) {
		if(cg.snap->ps.persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR || 
			cg.snap->ps.stats[STAT_HEALTH] <= 0 ||
			cg.showScores) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_NO_SCOREBOARD ) {
		if ( cg.showScores || cg.snap->ps.stats[STAT_HEALTH] <= 0 ) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_PLAYER_HAS_SENTRYCAM ) {

		if(!((cg.sentrycam_entityState || cg.sentryCamTime_end) && cg_showSentryCam.integer)) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_PLAYER_HAS_SENTRY ) {
		if( !(cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF) ) {
			return qfalse;
		}	
	}

	if( flags & CG_SHOW_PLAYER_HAS_SUPSTATION ) {
		if( !(cg.snap->ps.ammo[AMMO_Q3F_ENGDATA1] & 0xFF00) ) {
			return qfalse;
		}	
	}

	if( flags & CG_SHOW_PLAYER_HAS_SCANNERON ) {
		if( !(cg.snap->ps.stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_SCANNER)) ) {
			return qfalse;
		}	
	}

	if( flags & CG_SHOW_ENEMY_USE_SUPSTATION ) {
		if( !(cg.supplyStationUsedTime > cg.time && cg.supplyStationUserIsEnemy) ) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_FRIENDLY_USE_SUPSTATION ) {
		if( !(cg.supplyStationUsedTime > cg.time && !(cg.supplyStationUserIsEnemy)) ) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_HEALTHCRITICAL ) {
		if( cg.snap->ps.stats[STAT_HEALTH] >= 25) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_HEALTHOK ) {
		if( cg.snap->ps.stats[STAT_HEALTH] < 25) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_WEAPONSWITCH ) {
		if( cg.weaponSelectTime + WEAPON_SELECT_TIME <= cg.time) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_ON_RED_TEAM ) {
		if(cgs.clientinfo[cg.snap->ps.clientNum].team != Q3F_TEAM_RED) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_ON_BLUE_TEAM ) {
		if(cgs.clientinfo[cg.snap->ps.clientNum].team != Q3F_TEAM_BLUE) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_ON_YELLOW_TEAM ) {
		if(cgs.clientinfo[cg.snap->ps.clientNum].team != Q3F_TEAM_YELLOW) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_ON_GREEN_TEAM ) {
		if(cgs.clientinfo[cg.snap->ps.clientNum].team != Q3F_TEAM_GREEN) {
			return qfalse;
		}
	}

	/* Ensiform - Enable me when this gets added to menudef.h
	if( flags & CG_SHOW_ON_SPECTATOR_TEAM ) {
		if(cgs.clientinfo[cg.snap->ps.clientNum].team != Q3F_TEAM_SPECTATOR) {
			return qfalse;
		}
	}
	*/

/*  Slothy remove at Shurikens request, make way for CG_SHOW_(color)_TEAM_DOES_NOT_EXIST

	if( flags & CG_SHOW_NOT_ON_RED_TEAM ) {
		if(cgs.clientinfo[cg.snap->ps.clientNum].team == Q3F_TEAM_RED) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_NOT_ON_BLUE_TEAM ) {
		if(cgs.clientinfo[cg.snap->ps.clientNum].team == Q3F_TEAM_BLUE) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_NOT_ON_YELLOW_TEAM ) {
		if(cgs.clientinfo[cg.snap->ps.clientNum].team == Q3F_TEAM_YELLOW) {
			return qfalse;
		}
	}

	if( flags & CG_SHOW_NOT_ON_GREEN_TEAM ) {
		if(cgs.clientinfo[cg.snap->ps.clientNum].team == Q3F_TEAM_GREEN) {
			return qfalse;
		}
	}
*/
	if( flags & CG_SHOW_PLAYER_IS_AGENT ) {
		if(cgs.clientinfo[cg.snap->ps.clientNum].cls != Q3F_CLASS_AGENT) {
			return qfalse;
		}
	}

	return qtrue;
}

/*static void CG_DrawAreaSystemChat(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, systemChat, 0, 0, textStyle, font, textalignment);
}

static void CG_DrawAreaTeamChat(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color,teamChat1, 0, 0, textStyle, font, textalignment);
}

static void CG_DrawAreaChat(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, teamChat2, 0, 0, textStyle, font, textalignment);
}*/

const char *CG_GetKillerText() {
	const char *s = "";
	if ( cg.killerName[0] ) {
		s = va("Fragged by %s", cg.killerName );
	}
	return s;
}

static void CG_DrawKiller(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	// fragged by ... line
	if ( cg.killerName[0] ) {
		int x = rect->x + rect->w / 2;
		CG_Text_Paint(x - CG_Text_Width(CG_GetKillerText(), scale, 0, font) / 2 + text_x, rect->y + rect->h + text_y, scale, color, CG_GetKillerText(), 0, 0, textStyle, font, textalignment);
	}
	
}

/*
const char *CG_GetGameStatusText() {
	const char *s = "";
	if ( cgs.gametype < GT_TEAM) {
		if (cg.snap->ps.persistant[PERS_TEAM] != TEAM_SPECTATOR ) {
			s = va("%s place with %i",CG_PlaceString( cg.snap->ps.persistant[PERS_RANK] + 1 ),cg.snap->ps.persistant[PERS_SCORE] );
		}
	} else {
		if ( cg.teamScores[0] == cg.teamScores[1] ) {
			s = va("Teams are tied at %i", cg.teamScores[0] );
		} else if ( cg.teamScores[0] >= cg.teamScores[1] ) {
			s = va("Red leads Blue, %i to %i", cg.teamScores[0], cg.teamScores[1] );
		} else {
			s = va("Blue leads Red, %i to %i", cg.teamScores[1], cg.teamScores[0] );
		}
	}
	return s;
}*/

static char teampositionsbuff[MAX_STRING_CHARS];

static void CG_CalculateTeamTieString( int *order, int first, int last )
{
	// Return a string like "Red, Blue and Yellow have 5"

	int i;
	char *s;
	char namebuffer[64];

	for( i = first; i <= last; i++ )
	{
		switch( order[i] )
		{
			case Q3F_TEAM_RED:		s = "Red";		break;
			case Q3F_TEAM_BLUE:		s = "Blue";		break;
			case Q3F_TEAM_YELLOW:	s = "Yellow";	break;
			case Q3F_TEAM_GREEN:	s = "Green";	break;
			default:				s = "?";
		}
		trap_Cvar_VariableStringBuffer(va("cg_%steam", s), namebuffer, 64);
		Q_strcat( teampositionsbuff, sizeof(teampositionsbuff), namebuffer );
		if( i < last )
			Q_strcat( teampositionsbuff, sizeof(teampositionsbuff), (i < (last - 1)) ? ", " : " and " );
	}

	if( last > first ) {
		if( cg.teamScores[last - Q3F_TEAM_RED] == cg.teamScores[first - Q3F_TEAM_RED] ) {
			s = " are tied at ";
		} else if( last - first > 1 ) {
			s = first ? " each have " : " lead with ";
		} else {
			s = first ? " both have " : " are tied at ";
		}
	} else {
		s = first ? " has " : " leads with ";
	}
	Q_strcat( teampositionsbuff, sizeof(teampositionsbuff), s );
	Q_strcat( teampositionsbuff, sizeof(teampositionsbuff), va( "%d", cg.teamScores[order[first]-Q3F_TEAM_RED] ) );
}

const char *CG_Q3F_TeamStatus( void ) {
	// Calculate the order of teams, and a nice string.
	int order[4];

	int i, j, temp;

		// Find our teams
	for( i = 0; i < 4; i++ )
		order[i] = (cgs.teams & (1<<(Q3F_TEAM_RED+i))) ? (Q3F_TEAM_RED+i) : Q3F_TEAM_NUM_TEAMS;

		// Sort  them
	for( i = 0; i < 3; i++ )
	{
		if( order[i+1] < Q3F_TEAM_NUM_TEAMS &&
			cg.teamScores[order[i]-Q3F_TEAM_RED] < cg.teamScores[order[i+1]-Q3F_TEAM_RED] )
		{
			temp = order[i];
			order[i] = order[i+1];
			order[i+1] = temp;
			i = -1;		// Reset loop (it's a swap-sort)
		}
	}

	memset( teampositionsbuff, 0, sizeof(teampositionsbuff) );

		// Make a string
	for( i = 0; i < 4 && order[i] < Q3F_TEAM_NUM_TEAMS; i = j )
	{
		for( j = i+1; j < 4 &&
			(cg.teamScores[order[j]-Q3F_TEAM_RED] >= cg.teamScores[order[i]-Q3F_TEAM_RED]) &&
			(order[j] < Q3F_TEAM_NUM_TEAMS) ; j++ );
			// i to j-1 are on the same score, so...
		CG_CalculateTeamTieString( order, i, j-1 );
		if( j < 4 && order[j] < Q3F_TEAM_NUM_TEAMS )
			Q_strcat( teampositionsbuff, sizeof(teampositionsbuff), ", " );
	}

	return( teampositionsbuff );
}
	
static void CG_DrawGameStatus(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, CG_Q3F_TeamStatus(), 0, 0, textStyle, font, textalignment);
}

const char *CG_GameTypeString() {
	if ( cgs.gametype == GT_FORTS ) {
		return GAME_NAME_CAP;  //"Q3F"; keeg
	}
	return "";
}
static void CG_DrawGameType(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, CG_GameTypeString(), 0, 0, textStyle, font, textalignment);
}

static void CG_Text_Paint_Limit(float *maxX, float x, float y, float scale, vec4_t color, const char* text, float adjust, int limit, fontStruct_t *parentfont) {
	int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph;

	if (text) {
	// TTimo: FIXME
	//    const unsigned char *s = text; // bk001206 - unsigned
		const char *s = text;
		float max = *maxX;
		float useScale;
		
		fontInfo_t *font = &cgDC.Assets.font.textFont;
		
		if (scale <= cg_smallFont.value) {
			font = &cgDC.Assets.font.smallFont;
		} else if (scale > cg_bigFont.value) {
			font = &cgDC.Assets.font.bigFont;
		}
		
		useScale = scale * font->glyphScale;
		trap_R_SetColor( color );
		len = strlen(text);					 
	
		if (limit > 0 && len > limit) {
			len = limit;
		}
	
		count = 0;
	
		while (s && *s && count < len) {
			glyph = &font->glyphs[(int)*s]; // TTimo: FIXME: getting nasty warnings without the cast, hopefully this doesn't break the VM build
	
			if ( Q_IsColorStringPtr( s ) ) {
				memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
				newColor[3] = color[3];
				trap_R_SetColor( newColor );
				s += 2;
				continue;
			} else {
				float yadj = useScale * glyph->top;
			
				if (CG_Text_Width(s, scale, 1, parentfont) + x > max) {
					*maxX = 0;
					break;
				}
		    
				CG_Text_PaintChar(x, y - yadj, 
									glyph->imageWidth,
									  glyph->imageHeight,
										useScale, 
										  glyph->s,
											glyph->t,
											  glyph->s2,
												glyph->t2,
												  glyph->glyph);
				x += (glyph->xSkip * useScale) + adjust;
				*maxX = x;
				count++;
				s++;
			}
		}
		trap_R_SetColor( NULL );
	}

}

#define PIC_WIDTH 12

void CG_DrawTeamSpectators(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, fontStruct_t *parentfont) {
	if (cg.spectatorLen) {
		float maxX;

		if (cg.spectatorWidth == -1) {
			cg.spectatorWidth = 0;
			cg.spectatorPaintX = rect->x + 1;
			cg.spectatorPaintX2 = -1;
		}

		if (cg.spectatorOffset > cg.spectatorLen) {
			cg.spectatorOffset = 0;
			cg.spectatorPaintX = rect->x + 1;
			cg.spectatorPaintX2 = -1;
		}

		if (cg.time > cg.spectatorTime) {
			cg.spectatorTime = cg.time + 10;
			if (cg.spectatorPaintX <= rect->x + 2) {
				if (cg.spectatorOffset < cg.spectatorLen) {
					cg.spectatorPaintX += CG_Text_Width(&cg.spectatorList[cg.spectatorOffset], scale, 1, parentfont) - 1;
					cg.spectatorOffset++;
				} else {
					cg.spectatorOffset = 0;
					if (cg.spectatorPaintX2 >= 0) {
						cg.spectatorPaintX = cg.spectatorPaintX2;
					} else {
						cg.spectatorPaintX = rect->x + rect->w - 2;
					}
					cg.spectatorPaintX2 = -1;
				}
			} else {
				cg.spectatorPaintX--;
				if (cg.spectatorPaintX2 >= 0) {
					cg.spectatorPaintX2--;
				}
			}
		}

		maxX = rect->x + rect->w - 2;
		CG_Text_Paint_Limit(&maxX, cg.spectatorPaintX, rect->y + rect->h - 3, scale, color, &cg.spectatorList[cg.spectatorOffset], 0, 0, parentfont); 
		if (cg.spectatorPaintX2 >= 0) {
			float maxX2 = rect->x + rect->w - 2;
			CG_Text_Paint_Limit(&maxX2, cg.spectatorPaintX2, rect->y + rect->h - 3, scale, color, cg.spectatorList, 0, cg.spectatorOffset, parentfont); 
		}
		if (cg.spectatorOffset && maxX > 0) {
			// if we have an offset ( we are skipping the first part of the string ) and we fit the string
			if (cg.spectatorPaintX2 == -1) {
						cg.spectatorPaintX2 = rect->x + rect->w - 2;
			}
		} else {
			cg.spectatorPaintX2 = -1;
		}
	}
}

qboolean CG_GetWeaponSwitchBoxExtents(rectDef_t* in, rectDef_t* out, int anchorx, int anchory ) {
	int i, count = 0;
	rectDef_t newRect; 
	bg_q3f_playerclass_t *cls;

	out->h = in->h;
	out->w = in->w;
	out->x = in->x;
	out->y = in->y;

	cls = BG_Q3F_GetClass(&(cg.snap->ps));

	for ( i = 0 ; i < Q3F_NUM_WEAPONSLOTS ; i++ ) {
		if( cls->weaponslot[i]) {
			count++;
		}
	}

	if(count == 0)
		return qfalse;

	newRect.x = out->x;
	newRect.y = out->y;
	newRect.w = (count * (out->h+4))-4;
	newRect.h = out->h;

	CG_Item_AutoAnchor(out, &newRect, anchorx, anchory);

	return qtrue;
}


void CG_DrawWeaponSwitchBox (rectDef_t* rect, int anchorx, int anchory, qhandle_t bg, float inset) {
	rectDef_t r;
	int i;
	bg_q3f_playerclass_t	*cls;
	bg_q3f_weapon_t			*wp;
	int clsnum;
	vec4_t greyed = { 1.0f, 1.0f, 1.0f, 0.3f};
	vec4_t littlegreyed = { 1.0f, 1.0f, 1.0f, 0.8f};
	vec4_t redgreyed = { 1.0f, 0.0f, 0.0f, 0.8f};
	vec4_t white = { 1.0f, 1.0f, 1.0f, 1.0f};
	vec4_t color;
	float* scale;
	qboolean wdisabled = qfalse;


	if(!CG_GetWeaponSwitchBoxExtents(rect, &r, anchorx, anchory))
		return;

	cls = BG_Q3F_GetClass(&(cg.snap->ps));
	clsnum = cg.snap->ps.persistant[PERS_CURRCLASS];

	for ( i = 1 ; i < Q3F_NUM_WEAPONSLOTS ; i++ ) {
		if ( !cls->weaponslot[i] ) {
			continue;
		}
		wdisabled = qfalse;

		//CG_RegisterWeapon( cls->weaponslot[i] );

		wp = BG_Q3F_GetWeapon( cls->weaponslot[i]);
		if ( cg.snap->ps.ammo[ wp->ammotype ] < wp->numammo && wp->numammo) {
			memcpy(&color, &redgreyed, sizeof(vec4_t));
			wdisabled = qtrue;
		}
		else if ( cls->weaponslot[i] != cg.weaponSelect ) {
			memcpy(&color, &greyed, sizeof(vec4_t));
		}
		else {
			memcpy(&color, &white, sizeof(vec4_t));
		}

		if(cgs.eventHandling != CGAME_EVENT_EDITHUD) {
			scale = CG_FadeColor(cg.weaponSelectTime, WEAPON_SELECT_TIME);
			if(!scale)
				break;
			color[3] *= scale[3];
		}

		trap_R_SetColor(color);

		// draw weapon icon
		CG_DrawPic( r.x, r.y, r.h, r.h, bg);
		CG_DrawPic( r.x+(inset*0.5f), r.y+(inset*0.5f), r.h-inset, r.h-inset, CG_Q3F_GetWeaponStruct(clsnum, cls->weaponslot[i])->weaponIcon);


		if(wdisabled) {
			memcpy(&color, &littlegreyed, sizeof(vec4_t));			
			trap_R_SetColor(color);
			// Ensiform: Don't change the value of this variable
			CG_DrawPic( r.x+5, r.y+5, r.h-10, r.h-10, cgDC.Assets.xammo);
			//inset = 10;
			//CG_DrawPic( r.x+(inset*0.5f), r.y+(inset*0.5f), r.h-inset, r.h-inset, cgDC.Assets.xammo);
			//CG_DrawPic( r.x, r.y, r.h, r.h, cgDC.Assets.xammo);
		}

		trap_R_SetColor(NULL);

		r.x += r.h+4;
	}
}

static char *sentrynames[5][4] = {
	{ "bored",		"rusty",			"useless",		"pathetic"	},		// "Bored" data.
	{ "feeble",		"puny",				"basic",		"standard"	},
	{ "potent",		"dual-barrel",		"upgraded",		"enhanced"	},
	{ "terrifying",	"rocket-equipped",	"devastating",	"lethal"	},
	{ "diabolical",	"stupendous",		"outrageous",	"nitro-boosted"	},
};

vec3_t idteamcolours[Q3F_TEAM_NUM_TEAMS] = {
	{ 1, 1, 1 },		// Q3F_TEAM_FREE
	{ 1, 0.3f, 0.3f },	// Q3F_TEAM_RED
	{ 0.5f, 0.5f, 1 },	// Q3F_TEAM_BLUE
	{ 1, 1, 0.3f },		// Q3F_TEAM_YELLOW
	{ 0.3f, 1, 0.3f },	// Q3F_TEAM_GREEN
	{ 1, 1, 1 },		// Q3F_TEAM_SPECTATOR
};
	
static void CG_DrawClassIcon (rectDef_t* rect ) {
	vec4_t colour;
	clientInfo_t* ci;
	int cls;
	int index;
	centity_t* agentdata = NULL;

	if( cg.snap->ps.eFlags & (EF_Q3F_DISGUISE) ) {
		for( index = 0; index < MAX_ENTITIES; index++ ) {
			agentdata = &cg_entities[index];

			if( (agentdata->currentState.eType == ET_Q3F_AGENTDATA) &&
				agentdata->currentValid &&
				(agentdata->currentState.otherEntityNum == cg.snap->ps.clientNum) )
				break;		// We've found one.
		}

		if( index == MAX_ENTITIES )
			agentdata = NULL;	// We might not have the control ent yet, or it's finished
	} 

	ci = &cgs.clientinfo[ cg.snap->ps.clientNum ];
		
	cls = agentdata && agentdata->currentState.torsoAnim ? agentdata->currentState.torsoAnim : ci->cls;

	VectorCopy( CG_TeamColor( agentdata && agentdata->currentState.weapon ? agentdata->currentState.weapon : (int)ci->team ), colour );
	colour[3] = .75f;
	CG_FillRect( rect->x, rect->y, rect->w, rect->h, colour );
	CG_DrawPic( rect->x + 1, rect->y + 1, rect->w - 2, rect->h - 2, *CG_Q3F_ModelIcon( cls ) );
}

static void CG_DrawAgentDisguiseFullInfo(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	char* text = NULL;
	float* newcolor;
	vec4_t clr;
	int index;
	centity_t* agentdata = NULL;
	clientInfo_t* ci;
	char fullinfo[128];
	char team[14];
	char cl[16];
	int cls;

	newcolor = color;

	if( cg.snap->ps.eFlags & (EF_Q3F_DISGUISE|EF_Q3F_INVISIBLE) ) {
		// We don't draw, but we might want a agent effect instead.

		for( index = 0; index < MAX_ENTITIES; index++ ) {
			agentdata = &cg_entities[index];

			if( (agentdata->currentState.eType == ET_Q3F_AGENTDATA) &&
				agentdata->currentValid &&
				(agentdata->currentState.otherEntityNum == cg.snap->ps.clientNum) )
				break;		// We've found one.
		}

		if( index == MAX_ENTITIES )
			agentdata = NULL;	// We might not have the control ent yet, or it's finished
	} 

	if( agentdata ) {
		ci = &cgs.clientinfo[ cg.snap->ps.clientNum ];
		switch(agentdata->currentState.weapon ? agentdata->currentState.weapon : (int)ci->team)
		{
		case 1: strcpy(team, "^1red^7"); break;
		case 2: strcpy(team, "^4blue^7"); break;
		case 3: strcpy(team, "^3yellow^7"); break;
		case 4: strcpy(team, "^2green^7"); break;
		}

		cls = agentdata->currentState.torsoAnim ? agentdata->currentState.torsoAnim : ci->cls;
		switch(cls) {
			case 1: strcpy(cl, "recon"); break;
			case 2: strcpy(cl, "sniper"); break;
			case 3: strcpy(cl, "soldier"); break;
			case 4: strcpy(cl, "grenadier"); break;
			case 5: strcpy(cl, "paramedic"); break;
			case 6: strcpy(cl, "minigunner"); break;
			case 7: strcpy(cl, "flametrooper"); break;
			case 8: strcpy(cl, "agent"); break;
			case 9: strcpy(cl, "engineer"); break;
			case 10: strcpy(cl, "civilian"); break;
		}

		if((agentdata->currentState.modelindex2 & Q3F_AGENT_INVISMASK) == Q3F_AGENT_INVIS &&
		agentdata->currentState.origin2[0] < (cg.time + Q3F_AGENT_INVISIBLE_TIME) ) {
			text = "Invisible";
			clr[0] = clr[1] = clr[2] = 1.0f;
			clr[3] = (sin( (M_PI / 180) * (cg.time / 10.f)) + 1) *0.5f;

			newcolor = clr;
		} else if((agentdata->currentState.modelindex2 & Q3F_AGENT_DISGUISEMASK) == Q3F_AGENT_DISGUISE) {
			if( agentdata->currentState.time2 < cg.time ) {
				Com_sprintf(fullinfo, 128, "^7Disguised as %s %s", team, cl);
				text = fullinfo;
			} else if ( agentdata->currentState.time < cg.time ) {
				text = "Disguising";
			}
		}
		else
			text = "No Disguise";
	} else {
		text = "No Disguise";
	}

	if(text)
		CG_Text_Paint_MaxWidth( rect->x + text_x, rect->y + text_y, scale, newcolor, text, 0, 0, textStyle, font, textalignment, rect->w);
}

static void CG_DrawAgentDisguiseInfo(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font) {
	float* newcolor;
	vec4_t clr;
	int index;
	centity_t* agentdata = NULL;
	char* text = NULL;

	newcolor = color;

	if( cg.snap->ps.eFlags & (EF_Q3F_DISGUISE|EF_Q3F_INVISIBLE) ) {
		// We don't draw, but we might want a agent effect instead.

		for( index = 0; index < MAX_ENTITIES; index++ ) {
			agentdata = &cg_entities[index];

			if( (agentdata->currentState.eType == ET_Q3F_AGENTDATA) &&
				agentdata->currentValid &&
				(agentdata->currentState.otherEntityNum == cg.snap->ps.clientNum) )
				break;		// We've found one.
		}

		if( index == MAX_ENTITIES )
			agentdata = NULL;	// We might not have the control ent yet, or it's finished
	}

	if(	agentdata &&
		(agentdata->currentState.modelindex2 & Q3F_AGENT_INVISMASK) == Q3F_AGENT_INVIS &&
		agentdata->currentState.origin2[0] < (cg.time + Q3F_AGENT_INVISIBLE_TIME) ) {
		text = "Invisible";
		clr[0] = clr[1] = clr[2] = 1.0f;
		clr[3] = (sin( (M_PI / 180) * (cg.time / 10.f)) + 1) *0.5f;

		newcolor = clr;
	} else if( agentdata && (agentdata->currentState.modelindex2 & Q3F_AGENT_DISGUISEMASK) == Q3F_AGENT_DISGUISE) {
		if( agentdata->currentState.time2 < cg.time ) {
			text = "^1In Disguise";
		} else if ( agentdata->currentState.time < cg.time ) {
			text = "^3Disguising";
		}
	} else {
		text = "^2No Disguise";
	}

	if(text)
		CG_Text_Paint_MaxWidth( rect->x + text_x, rect->y + text_y, scale, newcolor, text, 0, 0, textStyle, font, textalignment, rect->w);
	else
	{
//		Com_Printf("Agent: bad agentdata\n");
		CG_Text_Paint_MaxWidth( rect->x + text_x, rect->y + text_y, scale, newcolor, "^2No Disguise", 0, 0, textStyle, font, textalignment, rect->w);
	}
}

static void CG_DrawTeamPipes( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char					num[16];
		
	Com_sprintf(num, 16, "%i", cg.snap->ps.ammoclip[1]);
	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, num , 0, 0, textStyle, font, textalignment);
}

static void CG_DrawPlayerPipes( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char					num[16];
		
	Com_sprintf(num, 16, "%i", cg.snap->ps.ammoclip[0]);
	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, num , 0, 0, textStyle, font, textalignment);
}

static void CG_DrawVoteString( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	const char *s;

	s = CG_ConfigString(CS_VOTE_STRING);

	if(s[0])
		CG_Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, s, 0, 0, textStyle, font, textalignment);
}

static void CG_DrawFlagInfo( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int special ) {
	char *strstart, *strend;
	int  y;

	if(cg.time > cg.fi_endtime) {
		cg.fi_endtime = 0;
		return;
	}

	y = rect->y + text_y;

	strstart = cg.finfo;
	strend = strchr(strstart, '\n');
	while(strend) {
		*strend = 0;
		CG_Text_Paint(rect->x + text_x, y, scale, color, strstart, 0, 0, textStyle, font, textalignment);
		*strend++ = '\n';
		y += special;
		strstart = strend;
		strend = strchr(strstart, '\n');
	}
}

static void CG_DrawGrenTimerDigits(rectDef_t *rect, float scale, vec4_t color, int textStyle, 
									  int textalignment, float text_x, float text_y, fontStruct_t *font) {
	char buffer[32];
	float left;

	left = (cg.grenadeHudTime + Q3F_GRENADE_PRIME_TIME - cg.time) * 0.001f;
	if(left < 0) {
		return;
	}

	Com_sprintf(buffer, 32, "%.3f", left);
	
	CG_Text_Paint_MaxWidth( rect->x + text_x, rect->y + text_y, scale, color, buffer, 0, 0, textStyle, font, textalignment, rect->w);
}

void CG_ExpandingTextBox_AutoWrap( char* instr, float scale, fontStruct_t*font, float w, int size) {
	char buffer[1024];
	char	*s, *p, *c, *ls;
	
	strcpy(buffer, instr);

	memset(instr, 0, size);

	c = s = instr;
	p = buffer;
	ls = NULL;
	while(*p) {
		*c = *p++;

		if(*c == ' ') {
			ls = c;
		} // store last space, to try not to break mid word

		c++;

		if(*p == '\n') {
			s = c+1;
		} else if(CG_Text_Width(s, scale, 0, font) > w) {
			if(ls) {
				*ls = '\n';
				s = ls+1;
			} else {
				*c = *(c-1);
				*(c-1) = '\n';
				s = c++;
			}

			ls = NULL;
		}
	}

	if(c != buffer && (*(c-1) != '\n')) {
		*c++ = '\n';
	}
	
	*c = '\0';
}

void CG_ExpandingTextBox_AutoWrapHeight( char* instr, float scale, fontStruct_t*font, float h, int size) {
	char buffer[1024];
	char* lines[64];
	char clrcodes[64];
	char clrcode = 0;
	int line, index, i;
	char *s, *c;

	strcpy(buffer, instr);

	memset(clrcodes, 0, sizeof(clrcodes));
	memset(lines, 0, sizeof(lines));
	memset(instr, 0, size);

	c = s = buffer;
	line = 0;

	while(*s && line < 63) {
		if(*s == '\n') {
			lines[line++] = c;
			*s++ = '\0';
			c = s;
		} else if(Q_IsColorStringPtr(s)) {
/*			if(!clrcodes[line]) {
				clrcodes[line] = *(s+1);
			}*/
			clrcodes[line+1] = *(s+1);
			s++;
		} else {
			s++;
		}
	}
	lines[line++] = c;

	for(index = 0; index < line; index++) {
		float h2 = 0;
		for(i = index; i < line; i++) {
			h2 += CG_Text_Height(lines[i], scale, 0, font);
			if(h2 > h) {
				break;
			}
		}
		if(h2 <= h) {
			for(i = 0; i < index; i++) {
				if(clrcodes[i]) {
					clrcode = clrcodes[i];
				}
			}

			for(i = index; i < line; i++) {
				char buff[256];

				if(clrcodes[i]) {
					clrcode = clrcodes[i];
				}
				if(clrcode) {
					Q_strncpyz(buff, va((i == index ? "^%c%s" : "\n^%c%s"), clrcode, lines[i]), 256);
				} else {
					Q_strncpyz(buff, va((i == index ? "%s" : "\n%s"), lines[i]), 256);
				}

				Q_strcat(instr, size, buff);
			}
			break;
		}
	}
}

qboolean CG_GetExpandingTextBox_Extents(rectDef_t* in, rectDef_t* out, float scale, fontStruct_t* font, int anchorx, int anchory, int border, char* instr, int ownerDraw) {
	float		textwidth, textheight, tw;
	rectDef_t	newRect;
	char		*p, *s;
	char		buffer[1024];

	if(!instr || !*instr) {
		return qfalse;
	}

	out->x = in->x;
	out->y = in->y;
	out->w = in->w;
	out->h = in->h;

	if(!(anchorx & 1)) {
		if(ownerDraw == CG_CHATBOX_CONTENT) {
			if(!cg.basicChatLinesWrapped) {
				CG_ExpandingTextBox_AutoWrap( instr, scale, font, out->w - (2*border), sizeof(buffer) );
				cg.basicChatLinesWrapped = qtrue;
				strcpy(cg.basicChatBuffer, instr);
			}
		} else if(ownerDraw == CG_TEAMCHATBOX_CONTENT) {
			if(!cg.teamChatLinesWrapped) {
				CG_ExpandingTextBox_AutoWrap( instr, scale, font, out->w - (2*border), sizeof(buffer) );
				cg.teamChatLinesWrapped = qtrue;
				strcpy(cg.teamChatBuffer, instr);
			}
		} else if(ownerDraw == CG_CENTERPRINTBOX_CONTENT) {
			if(!cg.centerprintWrapped) {
				CG_ExpandingTextBox_AutoWrap( instr, scale, font, out->w - (2*border), sizeof(buffer) );
				cg.centerprintWrapped = qtrue;
				strcpy(cg.centreprintBuffer, instr);
			}
		} else {
			CG_ExpandingTextBox_AutoWrap( instr, scale, font, out->w - (2*border), sizeof(buffer) );
		}
	}

	if(!(anchory & 1)) {
		CG_ExpandingTextBox_AutoWrapHeight( instr, scale, font, out->h - (2*border), sizeof(buffer) );
	}

	strcpy(buffer, instr);

	textwidth = 0;
	textheight = 0;

	s = p = buffer;
	while(*p && *s) {
		if(*p == '\n') {
			*p++ = '\0';
			tw = CG_Text_Width(s, scale, 0, font);
			if(tw > textwidth) {
				textwidth = tw;
			}
			textheight += CG_Text_Height(s, scale, 0, font)+2;
			s = p;
		}
		else {
			p++;
		}
	}

	newRect.x = out->x;
	newRect.y = out->y;
	newRect.h = textheight + (2 * border);
	newRect.w = textwidth + (2 * border);

	CG_Item_AutoAnchor(out, &newRect, anchorx, anchory);

	return qtrue;
}
	
qboolean CG_GetExpandingTextBox_Text( int ownerdraw, char* out, float* alpha, qboolean* changed) {
	int i;
	float*	clr;
	char* p = out;
	char* level;

	*changed = qfalse;

	if(cgs.eventHandling == CGAME_EVENT_EDITHUD) {
		return qfalse;
	}

	switch( ownerdraw ) {
	case CG_CHATBOX_CONTENT:
		*changed = cg.basicChatLinesChanged;

		for(i = 0; i < MAX_BASICCHAT_STRINGS; i++) {
			if(cg.basicChatTimes[i] <= cg.time) {
				break;
			}
		}

		if(i != cg.basicChatLinesUsed) {
			*changed = qtrue;
		}

		if(*changed) {
			cg.basicChatLinesUsed = i;
			cg.basicChatLinesWrapped = qfalse;
		}

		if(!i) {
			return qfalse;
		}

		if(!changed) {
			strcpy(out, cg.basicChatBuffer);
			return qtrue;
		}

		*p = '\0';
		for(i-- ; i >= 0; i--) {
			strcpy(p, &cg.basicChat[i*MAX_SAY_TEXT]);
			p += strlen(p);
			*p++ = '\n';
		}
		*p = '\0';

		strcpy(cg.basicChatBuffer, p);
		cg.basicChatLinesChanged = qfalse;
		cg.basicChatLinesWrapped = qfalse;

		*alpha = 1.0f;
		return qtrue;
	case CG_TEAMCHATBOX_CONTENT:
		*changed = cg.teamChatLinesChanged;

		for(i = 0; i < MAX_TEAMCHAT_STRINGS; i++) {
			if(cg.teamChatTimes[i] <= cg.time) {
				break;
			}
		}

		if(i != cg.teamChatLinesUsed) {
			*changed = qtrue;
		}

		if(*changed) {
			cg.teamChatLinesUsed = i;
			cg.teamChatLinesWrapped = qfalse;
		}

		if(!i) {
			return qfalse;
		}

		if(!changed) {
			strcpy(out, cg.teamChatBuffer);
			return qtrue;
		}

		*p = '\0';
		for(i-- ; i >= 0; i--) {
			strcpy(p, &cg.teamChat[i*MAX_SAY_TEXT]);
			p += strlen(p);
			*p++ = '\n';
		}
		*p = '\0';

		strcpy(cg.teamChatBuffer, p);
		cg.teamChatLinesChanged = qfalse;
		cg.teamChatLinesWrapped = qfalse;

		*alpha = 1.0f;
		return qtrue;
	case CG_OBITUARIES_CONTENT:
		*changed = cg.teamChatLinesChanged;

		for(i = 0; i < MAX_TEAMCHAT_STRINGS; i++) {
			if(cg.teamChatTimes[i] <= cg.time) {
				break;
			}
		}

		if(i != cg.teamChatLinesUsed) {
			*changed = qtrue;
		}

		if(*changed) {
			cg.teamChatLinesUsed = i;
			cg.teamChatLinesWrapped = qfalse;
		}

		if(!i) {
			return qfalse;
		}

		if(!changed) {
			strcpy(out, cg.teamChatBuffer);
			return qtrue;
		}

		*p = '\0';
		for(i-- ; i >= 0; i--) {
			strcpy(p, &cg.teamChat[i*MAX_SAY_TEXT]);
			p += strlen(p);
			*p++ = '\n';
		}
		*p = '\0';

		strcpy(cg.teamChatBuffer, p);
		cg.teamChatLinesChanged = qfalse;
		cg.teamChatLinesWrapped = qfalse;

		*alpha = 1.0f;
		return qtrue;
	case CG_CENTERPRINTBOX_CONTENT:
		
		*changed = qtrue;

		if(cg.centerPrintTime > cg.time) {
			if(cg.centerprintWrapped) {
				strcpy(p, cg.centreprintBuffer);
			} else {
				strcpy(p, cg.centerPrintText);
			}

			if(!cg.centerPrintSolid) {
				clr = CG_FadeColor( cg.centerPrintTime-5000, 5000 );
				if ( !clr || !clr[3] ) {
					return qfalse;
				}

				*alpha = clr[3];
			} else {
				*alpha = 1.f;
			}
			return qtrue;
		}
		return qfalse;
	case CG_CROSSHAIRINFO_BOX:

		*changed = qtrue;

		if( !cg_drawCrosshairNames.integer ||
			cg.renderingThirdPerson ||
			cg.renderingFlyBy ||
			cg.rendering2ndRefDef ||
			cg.snap->ps.powerups[PW_Q3F_FLASH] ||
			cg.snap->ps.powerups[PW_Q3F_CONCUSS] ||
			cg.gasEndTime > cg.time ||
			cg.scopeEnabled || 
			cg.scopeTime > cg.time ) {
			return qfalse;
		}

		// scan the known entities to see if the crosshair is sighted on one
		CG_ScanForCrosshairEntity();

		clr = CG_FadeColor( cg.crosshairClientTime, 1000 );
		if ( !clr || !clr[3] ) {
			return qfalse;
		}

		*alpha = clr[3];

		*p = '\0';

		if( cg.crosshairSentryLevel && cg.crosshairSentryLevel != 99 )
		{
			// Sentry ID

			level = sentrynames[cg.crosshairSentryBored ? 0 : cg.crosshairSentryLevel][cg.crosshairSentrySet];

			strcpy( p,	va( "%s^7's %s autosentry", cgs.clientinfo[cg.crosshairClientNum].name, level ) );
			p += strlen(p);
			*p++ = '\n';

			if( cg.snap->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_ENGINEER )
			{
				strcpy( p, va(	((cg.crosshairSentryLevel >= 3) ? "L%d Health: %d ^8Shells: %d ^1Rockets: %d" : "L%d Health: %d ^8Shells: %d"),
									cg.crosshairSentryLevel, cg.crosshairSentryHealth, cg.crosshairSentryShells, cg.crosshairSentryRockets ));
				p += strlen(p);
				*p++ = '\n';
			}
		}
		else if( cg.crosshairSupplyLevel /*== 1*/ && cg.crosshairSupplyLevel != 99 )
		{
			// Supply Station ID

			strcpy( p,  va( "%s^7's supply station", cgs.clientinfo[cg.crosshairClientNum].name ) );
			p += strlen(p);
			*p++ = '\n';

			if(cg.snap->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_ENGINEER)
			{
				strcpy(p,
						va(	((cg.crosshairSupplyLevel >= 3) ? "L%d Health: %d\n^8Shells: %d ^2Nails: %d ^1Rockets: %d ^6Cells: %d\n^5Armor: %d ^dGrenades: %d" :
																"L%d Health: %d\n^8Shells: %d ^2Nails: %d ^1Rockets: %d ^6Cells: %d\n^5Armor: %d"), cg.crosshairSupplyLevel, cg.crosshairSupplyHealth,
							cg.crosshairSupplyShells, cg.crosshairSupplyNails, cg.crosshairSupplyRockets, cg.crosshairSupplyCells, cg.crosshairSupplyArmor, cg.crosshairSupplyGrenades));
				p += strlen(p);
				*p++ = '\n';
			}
		}
		else {
			char buffer[256];
			// Client ID

			strcpy(buffer, cgs.clientinfo[ cg.crosshairClientNum ].name);
			
			Q_CleanStr(buffer);

			Com_sprintf(p, 256, "%s%s", CG_Q3F_GetTeamColorString(cgs.clientinfo[ cg.crosshairClientNum ].team), buffer);

			if(cg.predictedPlayerState.persistant[PERS_CURRCLASS] == Q3F_CLASS_PARAMEDIC
				&& CG_AlliedTeam( cgs.clientinfo[cg.crosshairClientNum].team, cg.predictedPlayerState.persistant[PERS_TEAM] ) &&
				//&& cgs.clientinfo[cg.crosshairClientNum].team == cg.predictedPlayerState.persistant[PERS_TEAM] &&
				cgs.clientinfo[cg.crosshairClientNum].infoValid )
			{
			
				strcat(p, va(S_COLOR_WHITE ": %d health",cgs.clientinfo[cg.crosshairClientNum].health));
			}
			else if(cg.predictedPlayerState.persistant[PERS_CURRCLASS] == Q3F_CLASS_ENGINEER
				&& CG_AlliedTeam( cgs.clientinfo[cg.crosshairClientNum].team, cg.predictedPlayerState.persistant[PERS_TEAM] ) &&
				//&& cgs.clientinfo[cg.crosshairClientNum].team == cg.predictedPlayerState.persistant[PERS_TEAM] &&
				cgs.clientinfo[cg.crosshairClientNum].infoValid )
			{
				strcat(p, va(S_COLOR_WHITE ": %d armor",cgs.clientinfo[cg.crosshairClientNum].armor));
			}
			p += strlen(p);
			*p++ = '\n';
		}

		*p = '\0';

		return qtrue;
	case CG_CHATEDIT_CONTENT:
		*p = '\0';

		switch(cg.q3f_messagemode_mode) {
		case Q3F_SAY_ALL:
			strcpy(p, "Say: ");
			break;
		case Q3F_SAY_TEAM:
			strcpy(p, "Say Team: ");
			break;
		case Q3F_SAY_ATTACKER:
			strcpy(p, "Say Attacker: ");
			break;
		case Q3F_SAY_TARGET:
			strcpy(p, "Say Target: ");
			break;
		}

		*alpha = 1.f;

		if(!cg.q3f_messagemode_buffer[0]) {
			return qtrue;
		}

		strcat(p, cg.q3f_messagemode_buffer);
		return qtrue;
	}
	return qfalse;
}

void CG_DrawExpandingTextBox (int ownerdraw, rectDef_t *rect, float scale, vec4_t color, int textStyle, 
							  int textalignment, float text_x, float text_y, fontStruct_t *font, int anchorx, int anchory, int border) {
	char		buffer[1024];
	char		*s, *p;
	rectDef_t	r;
	vec4_t		newcolor;
	qboolean	useClr = qfalse;
	char		clrCode = 0;
	char		cacheClrCode = 0;
	qboolean	changed;

	newcolor[0] = newcolor[1] = newcolor[2] = 1.0f;

	if(!CG_GetExpandingTextBox_Text(ownerdraw, buffer, &newcolor[3], &changed)) {
		return;
	}

	if(!CG_GetExpandingTextBox_Extents(rect, &r, scale, font, anchorx, anchory, border, buffer, ownerdraw)) {
		return;
	}

	r.x += text_x + border;
	r.y += text_y + border;
	
	s = p = buffer;
	while(*p) {
		if(Q_IsColorStringPtr(p)) {
			clrCode = *(p+1);
			p++;
		} else if(*p == '\n') {
			*p++ = '\0';

			CG_Text_Paint_MaxWidth( r.x , r.y, scale, newcolor, useClr ? va("^%c%s", cacheClrCode, s) : s, 0, 0, textStyle, font, textalignment, r.w);

			if(clrCode) {
				cacheClrCode = clrCode;
				useClr = qtrue;
			}

			r.y += CG_Text_Height(s, scale, 0, font)+2;
			s = p;
		}
		else {
			p++;
		}
	}
}

void CG_Q3F_DrawGrenadeTimer( rectDef_t* rect, vec4_t color ) {
	float pc;
	vec4_t clrbk;
	clrbk[0] = color[0];
	clrbk[1] = color[1];
	clrbk[2] = color[2];
	clrbk[3] = color[3]*0.5f;

	pc = (cg.grenadeHudTime + Q3F_GRENADE_PRIME_TIME - cg.time) / (float)Q3F_GRENADE_PRIME_TIME;
	if( pc < 0 ) {
		return;
	}
	if(pc > 1.f) {
		pc = 1.f;
	}

	CG_FillRect(rect->x, rect->y, rect->w, rect->h, clrbk);
	CG_FillRect(rect->x, rect->y, rect->w * pc, rect->h, color);
}

#if 0
void LoadGrenadeBar(void) {
	menuDef {
		name "q3f-1h_small_healthbar"
		fullScreen MENU_FALSE				
		rect 0 395 160 20
		visible MENU_TRUE
		ownerDrawFlag CG_SHOW_ON_TEAM

 	   	font "fonts/veramobd" 26
  	  	smallFont "fonts/veramobd" 20
  	  	bigFont "fonts/veramobd" 32

		itemDef {
			name "wholeback"
			rect 0 41 169 43
			visible MENU_TRUE
			decoration
			backcolor 0 0 0 .35
			style WINDOW_STYLE_FILLED
		}

		itemDef {
			name "back_healthbarTOP"
			rect 40 41 125 4
			visible MENU_TRUE
			decoration
			backcolor 0 0 0 .35
			style WINDOW_STYLE_FILLED
		}

		itemDef {
			name "back_healthbarMIDDLE"
			rect 40 57 125 4
			visible MENU_TRUE
			decoration
			backcolor 0 0 0 .35
			style WINDOW_STYLE_FILLED
		}		

		itemDef {
			name "back_healthbarRIGHT"
			rect 165 41 4 44
			visible MENU_TRUE
			decoration
			backcolor 0 0 0 .35
			style WINDOW_STYLE_FILLED
		}
		
		itemDef {
			name "back_healthbarLEFT"
			rect 0 41 40 32
			visible MENU_TRUE
			decoration
			backcolor 0 0 0 .35
			style WINDOW_STYLE_FILLED
		}		
		
		itemDef {
			name "back_healthbarBOTTOM"
			rect 0 73 165 12
			visible MENU_TRUE
			decoration
			backcolor 0 0 0 .35
			style WINDOW_STYLE_FILLED
		}		
		
		itemDef {
			name "playerhealth"
			rect 35 47 36 10
			visible MENU_TRUE
			decoration
			forecolor .8 .8 0 1
			textscale .3
			textalign ITEM_ALIGN_RIGHT
			textstyle ITEM_TEXTSTYLE_SHADOWED
			ownerDraw CG_PLAYER_HEALTH
		}

		itemDef {
			name "healthbar"
			rect 40 45 125 12
			visible MENU_TRUE
			decoration
			ownerDraw CG_PLAYER_HEALTH_BAR
		}
		
		itemDef {
			name "playerarmor"
			rect 35 62 36 10
			visible MENU_TRUE
			decoration
			forecolor .8 .8 0 1
			textscale .3
			textalign ITEM_ALIGN_RIGHT
			textstyle ITEM_TEXTSTYLE_SHADOWED
			ownerDraw CG_PLAYER_ARMOR_VALUE
		}

		itemDef {
			name "armorbar"
			rect 40 61 125 12
			visible MENU_TRUE
			decoration
			ownerDraw CG_PLAYER_ARMOR_BAR
		}	
	}
}
#endif

/*static void CG_DrawPlayerGrenadeBar(rectDef_t *rect, vec4_t color ) {
	CG_Q3F_DrawProgress( rect->x, rect->y, rect->w, rect->h, 
						 Q3F_GRENADE_PRIME_TIME,
						 Q3F_GRENADE_PRIME_TIME,
						 cg.grenadeHudTime + Q3F_GRENADE_PRIME_TIME - cg.time,
						 cgs.media.hud_healthShader,
						 color);
}*/

qboolean CG_Q3f_GetAlertIconExtents(rectDef_t* in, rectDef_t* out, int anchorx, int anchory, int border ) {
	int i, count = 0;
	rectDef_t	newRect;

	out->h = in->h;
	out->w = in->w;
	out->x = in->x;
	out->y = in->y;

	if(cgs.eventHandling == CGAME_EVENT_EDITHUD) {
		newRect.x = out->x;
		newRect.y = out->y;
		newRect.w = out->h;
		newRect.h = out->h;
	} else if(!cg_visualAids.integer) {
		return qfalse;
	} else {
		for(i = 0; i < MAX_Q3F_ALERTS; i++) {
			if(cg.q3f_alerticontime[i] > cg.time ) {
				count++;
			}
		}

		if(count == 0) {
			return qfalse;
		}

		newRect.x = out->x;
		newRect.y = out->y;
		newRect.w = (count * ((out->h-(2*border))+4))-4+(2*border);
		newRect.h = out->h;
	}

	CG_Item_AutoAnchor(out, &newRect, anchorx, anchory);

	return qtrue;
}

qboolean CG_GetAttackerBoxExtents(rectDef_t* in, rectDef_t* out) {
	int clientNum;

	out->h = in->h;
	out->w = in->w;
	out->x = in->x;
	out->y = in->y;

	if ( cg.predictedPlayerState.stats[STAT_HEALTH] <= 0 ) {
		return qfalse;
	}

	if ( !cg.attackerTime ) {
		return qfalse;
	}

	clientNum = cg.predictedPlayerState.persistant[PERS_ATTACKER];
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS || clientNum == cg.snap->ps.clientNum ) {
		return qfalse;
	}

	if ( cg.time - cg.attackerTime > ATTACKER_HEAD_TIME ) {
		cg.attackerTime = 0;
		return qfalse;
	}

	return qtrue;
}

qboolean CG_Q3F_GetPowerupIconExtents(rectDef_t* in, rectDef_t* out, int anchorx, int anchory, int border ) {
	int j;
	rectDef_t	newRect;
	int	powerups[MAX_POWERUPS];

	out->h = in->h;
	out->w = in->w;
	out->x = in->x;
	out->y = in->y;

	if(cgs.eventHandling == CGAME_EVENT_EDITHUD) {
		newRect.x = out->x;
		newRect.y = out->y;
		newRect.w = out->h;
		newRect.h = out->h;
	} else {
		j = CG_GetCurrentPowerUp(powerups); 

		if(j == -1) {
			return qfalse;
		}

		--j;
		out->x -= (j * out->h);
		out->w += (j + out->h);
		newRect.x = out->x;
		newRect.y = out->y;
		newRect.w = out->w;
		newRect.h = out->h;
	}

	CG_Item_AutoAnchor(out, &newRect, anchorx, anchory);

	return qtrue;
}

void CG_Q3F_AddAlertIcon(vec3_t origin, q3f_alert_t alert) {
	int i, j;
	vec3_t dist;

	if(alert >= Q3F_ALERT_MAX) {
		return;
	}

	if(!origin) {
		return;
	}

	VectorSubtract(cg.snap->ps.origin, origin, dist);
	if(VectorLength(dist) > 1024) {
		return;
	}

	for(i = 0; i < MAX_Q3F_ALERTS; i++) {
		if(cg.q3f_alerticons[i] == alert) {
			for(j = i+1; j < MAX_Q3F_ALERTS; j++) {
				cg.q3f_alerticons[j-1] =	cg.q3f_alerticons[j];
				cg.q3f_alerticontime[j-1] =	cg.q3f_alerticontime[j];
			}
			cg.q3f_alerticons[MAX_Q3F_ALERTS-1] =		Q3F_ALERT_NULL;
			cg.q3f_alerticontime[MAX_Q3F_ALERTS-1] =	0;
		}
	}

	for(i = MAX_Q3F_ALERTS-1; i > 0; i--) {
		cg.q3f_alerticons[i] =	cg.q3f_alerticons[i-1];
		cg.q3f_alerticontime[i] =	cg.q3f_alerticontime[i-1];
	}

	cg.q3f_alerticons[0] = alert;
	cg.q3f_alerticontime[0] = cg.time + 3000;
}

const char* Q3F_Alert_Shaders[Q3F_ALERT_MAX] = {
	"",
	"ui/gfx/hud/icons/waves.tga",
	"ui/gfx/hud/icons/pain.tga",
	"ui/gfx/hud/icons/trigger.tga",
	"ui/gfx/hud/icons/build.tga",
	"ui/gfx/hud/icons/HE_set.tga",
	"ui/gfx/hud/icons/HE_blow.tga",
	"ui/gfx/hud/icons/jumpad.tga",
	"ui/gfx/hud/icons/sentry.tga",
	"ui/gfx/hud/icons/explosion.tga",
	"ui/gfx/hud/icons/fire.tga",
	"ui/gfx/hud/icons/grenbounce.tga",
	"ui/gfx/hud/icons/gunfire.tga",
	"ui/gfx/hud/icons/minigun.tga",
	"ui/gfx/hud/icons/nailgren.tga",
	"ui/gfx/hud/icons/door.tga",
	"ui/gfx/hud/icons/lift.tga"
};

void CG_Q3F_DrawAlertIcon( rectDef_t* rect, int anchorx, int anchory, int border ) {
	rectDef_t r;
	int i;

	if(!CG_Q3f_GetAlertIconExtents(rect, &r, anchorx, anchory, border)) {
		return;
	}

	r.x += border;
	r.y += border;

	for(i = 0; i < MAX_Q3F_ALERTS; i++) {
		if(cg.q3f_alerticontime[i] < cg.time) {
			break;
		}

		CG_DrawPic(r.x, r.y, r.h-(2*border), r.h-(2*border), trap_R_RegisterShaderNoMip(Q3F_Alert_Shaders[cg.q3f_alerticons[i]]));

		r.x += (r.h-(2*border))+4;
	}
}

const vec3_t q3f_timer_positions[] = {
	{4, 0,	0},
	{4, 0,	1},
	{4, -1, 1},
	{4, -1, 0},
	{4, -1, -1},
	{4, 0,	-1},
	{4, 1,	-1},
	{4, 1,	0},
	{4, 1,	1},
	{4, 1,	1},
};

void CG_DrawAnalogueShader( rectDef_t *rect, vec4_t color, qhandle_t shader, int msec, int msecPerRound ) {
	float			x, y, w, h;
	refdef_t		refdef;
	polyVert_t		verts[10];
	int				numDrawVerts, i;
	int				frac;
	float			adjust;

	trap_R_SetColor(color);

	x = y = frac = 0;

	if(msec < 0) {
		return;
	}

	if(msec > msecPerRound) {
		msec = msecPerRound;
	}

	// djbob: bleh this is silly, but it just aint workin how it should.....
	for(adjust = msec; adjust > msecPerRound/8.f; adjust -= msecPerRound/8.f) {
		frac++;
	}

	if(frac < 0 || frac > 7) {
		return;
	}

	// Drawing it as a filled face
	memset( &refdef, 0, sizeof( refdef ) );

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	x = rect->x;
	y = rect->y;
	w = rect->w;
	h = rect->h;

	CG_AdjustFrom640( &x, &y, &w, &h );

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

	trap_R_ClearScene();

	for( i = 0; i < 10; i++ ) {
		verts[i].st[0] = 0;
		verts[i].st[1] = 0;
		verts[i].modulate[0] = 255 * color[0];
		verts[i].modulate[1] = 255 * color[1];
		verts[i].modulate[2] = 255 * color[2];
		verts[i].modulate[3] = 255 * color[3];
	}

	// Setup base uv data
	/*
	0:0		0.5:0		1:0

	0:0.5	0.5:0.5		1:0.5

	0:1		0.5:1		1:1	
	*/

	verts[0].st[0] = 0.5f;
	verts[0].st[1] = 0.5f;
	verts[1].st[0] = 0.5f;
	verts[1].st[1] = 0;
	verts[2].st[0] = 1;
	verts[2].st[1] = 0;
	verts[3].st[0] = 1;
	verts[3].st[1] = 0.5f;
	verts[4].st[0] = 1;
	verts[4].st[1] = 1;
	verts[5].st[0] = 0.5f;
	verts[5].st[1] = 1;
	verts[6].st[0] = 0;
	verts[6].st[1] = 1;
	verts[7].st[0] = 0;
	verts[7].st[1] = 0.5f;
	verts[8].st[0] = 0;
	verts[8].st[1] = 0;
	verts[9].st[0] = 0.5f;
	verts[9].st[1] = 0;

	adjust = (msec - (( frac * msecPerRound ) / 8.f)) / (msecPerRound / 8.f);

	numDrawVerts = frac + 3;
	for( i = 0; i < numDrawVerts; i++ ) {
		VectorCopy(q3f_timer_positions[i], verts[i].xyz);
	}

	switch(frac) {
	case 0:
		verts[2].xyz[1] = -adjust;
		verts[2].st[0] = 0.5f + 0.5f * adjust;
		break;

	case 1:
		verts[3].xyz[2] = 1-adjust;
		verts[3].st[1] = 0.5f * adjust;
		break;

	case 2:
		verts[4].xyz[2] = -adjust;
		verts[4].st[1] = 0.5f + 0.5f * adjust;
		break;

	case 3:
		verts[5].xyz[1] = -1+adjust;
		verts[5].st[0] = 1 - 0.5f * adjust;
		break;

	case 4:
		verts[6].xyz[1] = adjust;
		verts[6].st[0] = 0.5f - 0.5f * adjust;
		break;

	case 5:
		verts[7].xyz[2] = -1+adjust;
		verts[7].st[1] = 1 - 0.5f * adjust;
		break;

	case 6:
		verts[8].xyz[2] = adjust;
		verts[8].st[1] = 0.5f - 0.5f * adjust;
		break;

	case 7:
		verts[9].xyz[1] = 1-adjust;
		verts[9].st[0] = 0.5f * adjust;
		break;
	}

	trap_R_AddPolyToScene( shader, numDrawVerts, verts );
	trap_R_RenderScene( &refdef );

	trap_R_SetColor(NULL);
}

void CG_Q3F_DrawPowerupIcon( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int anchorx, int anchory, int border ) {
	rectDef_t r;
	int i, j, time;
	gitem_t* pw;
	char buffer[64];
	float y, x;
	int	powerups[MAX_POWERUPS];

	if(!CG_Q3F_GetPowerupIconExtents(rect, &r, anchorx, anchory, border)) {
		return;
	}

	r.x += border;
	r.y += border;

	i = CG_GetCurrentPowerUp(powerups);

	r.h -= 2*border;
	r.w = r.h;

	for(j = 0; j < i; j++) {
		time = (cg.snap->ps.powerups[powerups[j]] - cg.time);
		if(time <= 0) {
			return;
		}

		pw = BG_FindItemForPowerup(powerups[j]);
		if(!pw) {
			continue;
		}

		Com_sprintf(buffer, 64, "%d", (int)time/1000);

		y = r.y + ((r.h + CG_Text_Height(buffer, scale, 0, font)) * 0.5);
		x = r.x + (r.w * 0.5f);

		CG_DrawAnalogueShader(&r, colorWhite, trap_R_RegisterShaderNoMip(pw->icon), time, 30000);

		CG_Text_Paint(x, y, scale, color, buffer, 0, 0, textStyle, font, ITEM_ALIGN_CENTER);

		r.x += r.h;
	}
}



void CG_Q3F_DrawPowerupIconRev( rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int anchorx, int anchory, int border ) {
	rectDef_t r;
	int i, j, time;
	gitem_t* pw;
	char buffer[64];
	float y, x;
	int	powerups[MAX_POWERUPS];

	if(!CG_Q3F_GetPowerupIconExtents(rect, &r, anchorx, anchory, border)) {
		return;
	}

	r.x = rect->x;
	r.x += border;
	r.y += border;

	i = CG_GetCurrentPowerUp(powerups);

	r.h -= 2*border;
	r.w = r.h;

	for(j = 0; j < i; j++) {
		time = (cg.snap->ps.powerups[powerups[j]] - cg.time);
		if(time <= 0) {
			return;
		}

		pw = BG_FindItemForPowerup(powerups[j]);
		if(!pw) {
			continue;
		}

		Com_sprintf(buffer, 64, "%d", (int)time/1000);

		y = r.y + ((r.h + CG_Text_Height(buffer, scale, 0, font)) * 0.5);
		x = r.x + (r.w * 0.5f);

		CG_DrawAnalogueShader(&r, colorWhite, trap_R_RegisterShaderNoMip(pw->icon), time, 30000);

		CG_Text_Paint(x, y, scale, color, buffer, 0, 0, textStyle, font, ITEM_ALIGN_CENTER);

		r.x += r.h;
	}
}

void CG_Q3F_DrawScoreboardTitle(rectDef_t *rect, float scale, vec4_t color, int textStyle, 
									  int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	char buffer[256];
	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
		strcpy(buffer, "Final Scores For ");
	} else {
		strcpy(buffer, "Current Scores For ");
	}

	if(!cgs.mapInfoLoaded || (cgs.mapinfo.mapName == NULL)) {
		Q_strcat(buffer, 256, "Unknown Map");
		CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, buffer, 0, 0, textStyle, font, textalignment);
	} else {
		int i;

		for( i = 0; i < cgs.mapinfo.numGameIndicies; i++) {
			if(cgs.mapinfo.gameIndiciesInfo[i].number == cgs.gameindex) {
				Q_strcat(buffer, 256, cgs.mapinfo.gameIndiciesInfo[i].name+3);
				CG_Text_Paint( rect->x + text_x, rect->y + text_y, scale, color, buffer, 0, 0, textStyle, font, textalignment);
				return;
			}
		}
		Q_strcat(buffer, 256, cgs.mapinfo.mapName);
		CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, buffer, 0, 0, textStyle, font, textalignment);
	}
}


/*
=================
CG_Q3F_DrawHUDIcons
=================
*/

void CG_Q3F_DrawHUDIcons(rectDef_t *rect, float tscale, vec4_t color, int textStyle, 
									  int textalignment, float text_x, float text_y, fontStruct_t *font, float scale ) {
	// Fill all required slots with various HUD icons.

	int slot;
	float x, y, w, h;
	centity_t *cent;

	for( slot = 0; slot < Q3F_SLOT_MAX; slot++ ) {
		cent = cg.hudslots[slot];
		if( !cent ) {
			continue;
		}

		if( cent->currentState.eFlags & EF_VOTED) {
		} else {
			if( !cent->currentState.modelindex ) {
				continue;
			}
		}

		x	= rect->x + ((64 * (slot / 5)) * scale);
		y	= rect->y + ((64 * (slot % 5)) * scale);
		w	= 64 * cent->currentState.apos.trBase[0] * scale;	// 64 * scale (0 to 1)
		h	= 64 * cent->currentState.apos.trBase[0] * scale;

		if( cent->currentState.eFlags & EF_TALK ) {
			// It's a flat shader
			CG_AdjustFrom640( &x, &y, &w, &h );

			trap_R_DrawStretchPic(	(x + 32) - (w / 2), (y + 32) - (h / 2),
									w, h, 0, 0, 1, 1,
									cgs.gameShaders[cent->currentState.modelindex] );
		} else if( cent->currentState.eFlags & EF_VOTED ) {
			int secs, mins;
			vec4_t clr;
			char buf[16];

			secs = cent->currentState.modelindex;
			mins = cent->currentState.constantLight;

			clr[0] = cent->currentState.apos.trDelta[0];
			clr[1] = cent->currentState.apos.trDelta[1];
			clr[2] = cent->currentState.apos.trDelta[2];
			clr[3] = 1;

			Com_sprintf(buf, 16, mins >= 10 ? "%i:" : "0%i:", mins);
			Q_strcat(buf, 16, va(secs >= 10 ? "%i" : "0%i", secs));
			
			y += (h - CG_Text_Height(buf, tscale, 0, font)) / 2;

			CG_Text_Paint( x + text_x, y + text_y, tscale, clr, buf, 0, 0, textStyle, font, textalignment);

		} else {
			// It's a floating model
			vec3_t mins, maxs, origin, angles;
			clipHandle_t cm = cgs.gameModels[cent->currentState.modelindex];
			refdef_t refdef;
			refEntity_t ent;
			float len;

			VectorClear( angles );
			// offset the origin y and z to center the flag
			trap_R_ModelBounds( cm, mins, maxs );
			origin[2] = -0.5 * ( mins[2] + maxs[2] );
			origin[1] = 0.5 * ( mins[1] + maxs[1] );
			// calculate distance so the flag nearly fills the box
			// assume heads are taller than wide
			len = 0.5 * ( maxs[2] - mins[2] );		
			origin[0] = len / 0.268;	// len / tan( fov/2 )
			angles[YAW] = 60 * sin( slot * 30 + cg.time / 2000.0 );
	//		CG_Draw3DModel( x, y, w, h, cm, 0, origin, angles );

			CG_AdjustFrom640( &x, &y, &w, &h );

			memset( &refdef, 0, sizeof( refdef ) );
			memset( &ent, 0, sizeof( ent ) );
			AnglesToAxis( angles, ent.axis );
			VectorCopy( origin, ent.origin );
			ent.hModel = cm;
			ent.renderfx = RF_NOSHADOW|RF_MINLIGHT|RF_DEPTHHACK;		// no stencil shadows
			ent.shaderRGBA[0] = cent->currentState.origin2[0] * 0xff;
			ent.shaderRGBA[1] = cent->currentState.origin2[1] * 0xff;
			ent.shaderRGBA[2] = cent->currentState.origin2[2] * 0xff;
			ent.shaderRGBA[3] = 0xFF;

			trap_R_ClearScene();
			trap_R_AddRefEntityToScene( &ent, NULL );
			refdef.rdflags = RDF_NOWORLDMODEL;
			AxisClear( refdef.viewaxis );
			refdef.fov_x = 30;
			refdef.fov_y = 30;
			refdef.x = x;
			refdef.y = y;
			refdef.width = w;
			refdef.height = h;
			refdef.time = cg.time;
			trap_R_RenderScene( &refdef );
		}
	}
}


static void CG_Q3F_DrawLevelshot( rectDef_t* rect ) {
	if(!cgs.mapInfoLoaded) {
		CG_DrawPic(rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShaderNoMip("menu/art/unknownmap_sm"));
		CG_DrawPic(rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShader( "levelShotDetail" ));
		return;
	}

	if(cgs.mapinfo.levelShot == -1) {
		cgs.mapinfo.levelShot = trap_R_RegisterShaderNoMip(cgs.mapinfo.imageName);
	}

	CG_DrawPic(rect->x, rect->y, rect->w, rect->h, cgs.mapinfo.levelShot ? cgs.mapinfo.levelShot : trap_R_RegisterShaderNoMip("menu/art/unknownmap_sm"));
	CG_DrawPic(rect->x, rect->y, rect->w, rect->h, trap_R_RegisterShader( "levelShotDetail" ));
}

static void CG_Q3F_DrawLoadStatus(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	if(!cgs.initPhase) {
		CG_Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, "Awaiting snapshot...", 0, 0, textStyle, font, textalignment);
	}
}

static void CG_Q3F_DrawMapName(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	if(!cgs.mapInfoLoaded || (cgs.mapinfo.mapName == NULL)) {
		CG_Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, "Unknown Map", 0, 0, textStyle, font, textalignment);
	} else {
		int i;

		for( i = 0; i < cgs.mapinfo.numGameIndicies; i++) {
			if(cgs.mapinfo.gameIndiciesInfo[i].number == cgs.gameindex) {
				CG_Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, cgs.mapinfo.gameIndiciesInfo[i].name+3, 0, 0, textStyle, font, textalignment);
				return;
			}
		}
		CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, cgs.mapinfo.mapName, 0, 0, textStyle, font, textalignment);
	}
}

static void CG_Q3F_DrawMapInfo(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	if(!cgs.mapInfoLoaded || (cgs.mapinfo.mapName == NULL)) {
		CG_Text_Paint(rect->x + text_x, rect->y + text_y, scale, color, "Unknown Map", 0, 0, textStyle, font, textalignment);
	} else {
		int i;

		for( i = 0; i < cgs.mapinfo.numGameIndicies; i++) {
			if(cgs.mapinfo.gameIndiciesInfo[i].number == cgs.gameindex) {
				char buffer[1024];
				char *s, *p;
				float y;
				
				Q_strncpyz(buffer, cgs.mapinfo.gameIndiciesInfo[i].description, 1024);

				CG_ExpandingTextBox_AutoWrap(buffer, scale, font, rect->w, sizeof(buffer));

				y = rect->y + text_y;

				s = p = buffer;
	
				while(*p) {
					if(*p == '\n') {
						*p++ = '\0';

						CG_Text_Paint( rect->x + text_x, y, scale, color, s, 0, 0, textStyle, font, textalignment);

						y += CG_Text_Height(s, scale, 0, font)+2;
						s = p;
					}
					else {
						p++;
					}
				}

				return;
			}
		}
	}
}

static void CG_Q3F_DrawMotd(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	const char* s;

	s = CG_ConfigString(CS_MOTD);

	if(*s) {
		CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, s, 0, 0, textStyle, font, textalignment);
	}
}

static void CG_Q3F_DrawLoadInfo(rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font, int special ) {
	int y = rect->y + text_y;	// + rect->h;
	int index;
	int maxy = rect->y + rect->h;
	int strstart = cgs.teamChatPos;
	int numlines = (rect->h - text_y) / special;
	int count = 0;

	index = strstart - numlines;
	if(index < 0)
		index += TEAMCHAT_HEIGHT;

	while ((count < TEAMCHAT_HEIGHT) && (cgs.teamChatMsgs[index % TEAMCHAT_HEIGHT][0] == 0)) {
		index++;
		count++;
	}

	// Draw load log.
	count = 0;
	while( (count < TEAMCHAT_HEIGHT) && ((y + special) <=  maxy)) {
		CG_Text_Paint_MaxWidth(rect->x + text_x, y, scale, color, cgs.teamChatMsgs[index % TEAMCHAT_HEIGHT], 0, 0, textStyle, font, textalignment, rect->w);
		y += special;
		index++;
		count++;
	}
}

void CG_DrawMapLoadProgress( rectDef_t* rect ) {
	vec4_t		hcolor = { 0.f, 0.f, 0.f, 0.5f };
	float		valuefact;

	valuefact = 1 - (((float)cgs.initPhase - 1) / INITPHASE_NUM);

	CG_FillRect( rect->x + rect->w*(1-valuefact), rect->y, rect->w * valuefact, rect->h, hcolor );
}

void CG_DrawArmorIcon( rectDef_t* rect ) {
	float x, y, w, h;
	qhandle_t	*pic = NULL;

	// setup the refdef
	x = rect->x;
	y = rect->y;
	w = rect->w;
	h = rect->h;
	CG_AdjustFrom640( &x, &y, &w, &h );

	switch(cg.snap->ps.stats[STAT_ARMORTYPE]) {
		case Q3F_ARMOUR_GREEN : pic = &cgDC.Assets.ArmorColor[0]; break;
		case Q3F_ARMOUR_RED   : pic = &cgDC.Assets.ArmorColor[1]; break;
		case Q3F_ARMOUR_YELLOW: pic = &cgDC.Assets.ArmorColor[2]; break;
	}

	if(pic)
		trap_R_DrawStretchPic(x, y, w, h, 0, 0, 1, 1, *pic);
}

void CG_DrawArmorClassIcon( rectDef_t* rect ) {
	float x, y, w, h;
	qhandle_t	*pic = NULL;

	// setup the refdef
	x = rect->x;
	y = rect->y;
	w = rect->w;
	h = rect->h;
	CG_AdjustFrom640( &x, &y, &w, &h );

	if(cg.snap->ps.stats[STAT_Q3F_ARMOURCLASS]) {
		if(cg.snap->ps.stats[STAT_Q3F_ARMOURCLASS] & DAMAGE_Q3F_SHELL)
			pic = &cgDC.Assets.ArmorTypes[2];
		else if(cg.snap->ps.stats[STAT_Q3F_ARMOURCLASS] & DAMAGE_Q3F_NAIL)
			pic = &cgDC.Assets.ArmorTypes[4];
		else if(cg.snap->ps.stats[STAT_Q3F_ARMOURCLASS] & DAMAGE_Q3F_EXPLOSION)
			pic = &cgDC.Assets.ArmorTypes[1];
		else if(cg.snap->ps.stats[STAT_Q3F_ARMOURCLASS] & DAMAGE_Q3F_SHOCK)
			pic = &cgDC.Assets.ArmorTypes[3];
		else if(cg.snap->ps.stats[STAT_Q3F_ARMOURCLASS] & DAMAGE_Q3F_FIRE)
			pic = &cgDC.Assets.ArmorTypes[0];
	}

	if(pic)
		trap_R_DrawStretchPic(x, y, w, h, 0, 0, 1, 1, *pic);
}

void CG_DrawArmorType( rectDef_t *rect, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font ) {
	float x, y, w, h;
	char txt[30];

	// setup the refdef
	x = rect->x;
	y = rect->y;
	w = rect->w;
	h = rect->h;
	CG_AdjustFrom640( &x, &y, &w, &h );

	strcpy(txt, "no ");

	switch(cg.snap->ps.stats[STAT_ARMORTYPE]) {
		case Q3F_ARMOUR_GREEN : strcpy(txt, "green "); break;
		case Q3F_ARMOUR_YELLOW: strcpy(txt, "yellow "); break;
		case Q3F_ARMOUR_RED   : strcpy(txt, "red "); break;
	}

	if(cg.snap->ps.stats[STAT_Q3F_ARMOURCLASS]) {
		if(cg.snap->ps.stats[STAT_Q3F_ARMOURCLASS] & DAMAGE_Q3F_SHELL)
			strcat(txt, "shell");
		else if(cg.snap->ps.stats[STAT_Q3F_ARMOURCLASS] & DAMAGE_Q3F_NAIL)
			strcat(txt, "nail");
		else if(cg.snap->ps.stats[STAT_Q3F_ARMOURCLASS] & DAMAGE_Q3F_EXPLOSION)
			strcat(txt, "explosion");
		else if(cg.snap->ps.stats[STAT_Q3F_ARMOURCLASS] & DAMAGE_Q3F_SHOCK)
			strcat(txt, "shock");
		else if(cg.snap->ps.stats[STAT_Q3F_ARMOURCLASS] & DAMAGE_Q3F_FIRE)
			strcat(txt, "fire");
	}
	else
		strcat(txt, "armor");

	CG_Text_Paint(rect->x + text_x, rect->y + rect->h + text_y, scale, color, txt, 0, 0, textStyle, font, textalignment);
}

void CG_DrawActiveWeapon (rectDef_t* rect, int anchorx, int anchory, qhandle_t bg) {
	float i;
	//bg_q3f_playerclass_t	*cls;
	int clsnum;
	static const vec4_t greyed = { 1.0f, 1.0f, 1.0f, 0.3f};
	vec4_t color;
	bg_q3f_weapon_t			*wp;

	//cls = BG_Q3F_GetClass(&(cg.snap->ps));
	clsnum = cg.snap->ps.persistant[PERS_CURRCLASS];
	if(clsnum == 0)
		return;

	if(cg.snap->ps.weapon == 0)
		return;

//	CG_AdjustFrom640( &rect->x, &rect->y, &rect->w, &rect->h );

	VectorCopy4( colorWhite, color );
	if((cg.snap->ps.weaponstate == WEAPON_RELOADING) || (cg.snap->ps.weaponstate == WEAPON_RDROPPING))
	{
		// blergh make it pulse
		i = (cg.snap->ps.weaponTime / PULSE_DIVISOR); 
		color[3] = 0.5 + 0.5 * sin(i);
	}
	else
	{
		wp = BG_Q3F_GetWeapon(cg.snap->ps.weapon);
		//wp = BG_Q3F_GetWeapon( cg.weaponSelect );

		if ( cg.snap->ps.ammo[ wp->ammotype ] < wp->numammo && wp->numammo) {
			VectorCopy4( greyed, color );
		}
		else {
			// check for clip empty
			if (wp->clipsize > 0) {
				// find out how much in clip
				if(Q3F_GetClipValue(cg.snap->ps.weapon, &cg.snap->ps) == 0)	{
					VectorCopy4( greyed, color );
				}
			}
		}
	}

	trap_R_SetColor(color);

	rect->x += anchorx;
	rect->y += anchory;

	// draw weapon icon
	if(bg)
		CG_DrawPic( rect->x, rect->y, rect->h, rect->h, bg);

	CG_DrawPic( rect->x, rect->y, rect->h, rect->w, CG_Q3F_GetWeaponStruct(clsnum, cg.snap->ps.weapon)->weaponIcon);

	trap_R_SetColor(NULL);
}

//
void CG_OwnerDraw( itemDef_t *item, float x, float y, float w, float h, float text_x, float text_y, int ownerDraw, int ownerDrawFlags, int align, float special, float scale, vec4_t color, qhandle_t shader, int textStyle, int textalignment) {
	rectDef_t rect;
	menuDef_t *parent;
	fontStruct_t *parentfont;
	int anchorx, anchory;

	if ( cg_drawStatus.integer == 0 ) {
		return;
	}

	parent = (menuDef_t*)item->parent;
	parentfont = ( item->window.flags & WINDOW_USEASSETFONT ) ? NULL : &parent->font;

	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	anchorx = item->anchorx;
	anchory = item->anchory;

	switch (ownerDraw) {
		// Player stats
		case CG_DRAWATTACKER:
			CG_DrawAttacker(&rect, scale, textStyle, parentfont);
			break;
		case CG_PLAYER_ARMOR_ICON:
			CG_DrawPlayerArmorIcon(&rect);
			break;
		case CG_PLAYER_ARMOR_VALUE:
			CG_DrawPlayerArmorValue(&rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_PLAYER_ARMOR_BAR:
			CG_DrawPlayerArmorBar(&rect);
			break;
		case CG_PLAYER_HEAD:
			CG_DrawPlayerHead(&rect, ownerDrawFlags & CG_SHOW_2DONLY);
			break;
		case CG_PLAYER_ITEM:
			CG_DrawPlayerItem(&rect, scale, ownerDrawFlags & CG_SHOW_2DONLY);
			break;
		case CG_PLAYER_SCORE:
			CG_DrawPlayerScore(&rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_PLAYER_HEALTH:
			CG_DrawPlayerHealth( &rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_PLAYER_HEALTH_BAR:
			CG_DrawPlayerHealthBar( &rect, color );
			break;

		// Ammo stats
		case CG_PLAYER_AMMOSLOT_ROCKET_VALUE:
			CG_DrawAmmoSlotValue( AMMO_ROCKETS, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_PLAYER_AMMOSLOT_SHELLS_VALUE:
			CG_DrawAmmoSlotValue( AMMO_SHELLS, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_PLAYER_AMMOSLOT_NAILS_VALUE:
			CG_DrawAmmoSlotValue( AMMO_NAILS, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_PLAYER_AMMOSLOT_CELLS_VALUE:
			CG_DrawAmmoSlotValue( AMMO_CELLS, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_PLAYER_AMMOSLOT_MEDIKIT_VALUE:
			CG_DrawAmmoSlotValue( AMMO_MEDIKIT, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_PLAYER_AMMOSLOT_CHARGE_VALUE:
			CG_DrawAmmoSlotValue( AMMO_CHARGE, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;

		case CG_PLAYER_AMMO_ICON:
			CG_DrawCurrentAmmoIcon( &rect );
			break;
		case CG_PLAYER_AMMO_VALUE:
			CG_DrawCurrentAmmoValue( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_PLAYER_AMMO_CLIP_VALUE:
			CG_DrawCurrentClipValue( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;

		// Grenade stats
		case CG_PLAYER_PRIMARY_GRENADE_VALUE:
			CG_DrawGrenadeValue( 1, &rect, scale, color, item->window.backColor, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_PLAYER_SECONDARY_GRENADE_VALUE:
			CG_DrawGrenadeValue( 2, &rect, scale, color, item->window.backColor, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_CHARGE_ICON:
			CG_DrawChargeModel( &rect );
			break;

		//case CG_CHARGE_ICON2:
		//	CG_DrawChargeIcon( &rect );
		//	break;/

		// Team stats
		case CG_RED_SCORE:
			CG_DrawTeamScore( Q3F_TEAM_RED, &rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_BLUE_SCORE:
			CG_DrawTeamScore( Q3F_TEAM_BLUE, &rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_YELLOW_SCORE:
			CG_DrawTeamScore( Q3F_TEAM_YELLOW, &rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_GREEN_SCORE:
			CG_DrawTeamScore( Q3F_TEAM_GREEN, &rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_RED_NAME:
			CG_DrawTeamName( Q3F_TEAM_RED, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_BLUE_NAME:
			CG_DrawTeamName( Q3F_TEAM_BLUE, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_YELLOW_NAME:
			CG_DrawTeamName( Q3F_TEAM_YELLOW, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_GREEN_NAME:
			CG_DrawTeamName( Q3F_TEAM_GREEN, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_RED_TEAMCOUNT:
			CG_DrawTeamCount( Q3F_TEAM_RED, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_BLUE_TEAMCOUNT:
			CG_DrawTeamCount( Q3F_TEAM_BLUE, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_YELLOW_TEAMCOUNT:
			CG_DrawTeamCount( Q3F_TEAM_YELLOW, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_GREEN_TEAMCOUNT:
			CG_DrawTeamCount( Q3F_TEAM_GREEN, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_RED_TEAMPING:
			CG_DrawTeamPing( Q3F_TEAM_RED, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_BLUE_TEAMPING:
			CG_DrawTeamPing( Q3F_TEAM_BLUE, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_YELLOW_TEAMPING:
			CG_DrawTeamPing( Q3F_TEAM_YELLOW, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_GREEN_TEAMPING:
			CG_DrawTeamPing( Q3F_TEAM_GREEN, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;

		// System stats
		case CG_SYSTEM_LAGOMETER:
			CG_DrawLagometer( &rect );
			break;
		case CG_SYSTEM_FPS:
			CG_DrawFPS( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_SYSTEM_SPEED:
			CG_DrawSpeedometer( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont, item->special);
			break;
		case CG_SYSTEM_TIMER:
			CG_DrawTimer( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;

		// Special engineer items
		case CG_ENGINEER_SENTRYCAM:
			CG_DrawSentryCam( &rect );
			break;
		case CG_ENGINEER_SENTRYHEALTH:
			CG_DrawSentryHealth( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_ENGINEER_SENTRYBULLETS:
			CG_DrawSentryBullets( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_ENGINEER_SENTRYROCKETS:
			CG_DrawSentryRockets( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_ENGINEER_SUPSTATIONHEALTH:
			CG_DrawSupStationHealth( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;

		// Special recon items
		case CG_RECON_SCANNER:
			CG_DrawScanner( &rect );
			break;

		case CG_PLAYER_LOCATION:
			CG_DrawPlayerLocation(&rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_TEAM_COLOR:
			CG_DrawTeamColor(&rect, color);
			break;
		case CG_GAME_TYPE:
			CG_DrawGameType(&rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_GAME_STATUS:
			CG_DrawGameStatus(&rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_KILLER:
			CG_DrawKiller(&rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_SPECTATORS:
			CG_DrawTeamSpectators(&rect, scale, color, shader, parentfont );
			break;

//	djbob
		case CG_CHATBOX_CONTENT:
		case CG_TEAMCHATBOX_CONTENT:
		case CG_CENTERPRINTBOX_CONTENT:
		case CG_CROSSHAIRINFO_BOX:
		case CG_CHATEDIT_CONTENT:
			CG_DrawExpandingTextBox ( ownerDraw, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont, anchorx, anchory, item->special );
			break;

		case CG_WEAPONSWITCH_BOX:
			CG_DrawWeaponSwitchBox( &rect, anchorx, anchory, item->window.background, item->special);
			break;
		case CG_AGENTDISGUISE_INFO:
			CG_DrawAgentDisguiseInfo( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_CLASS_ICON:
			CG_DrawClassIcon( &rect );
			break;
		case CG_MENUBOX_CONTENT:
			CG_Q3F_DrawMenuBox();
			break;
//		case CG_MENUBOX_TITLE:
//			CG_Q3F_DrawMenuBoxTitle( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont);
//			break;
		case CG_GREN_TIMER:
			if(cg.grenadeHudTime != 0)		// slothy - do not display grentimer at map start
				CG_Q3F_DrawGrenadeTimer( &rect, color );
			break;
		case CG_GREN_ANALOGUE:
			if(cg.grenadeHudTime != 0)		// slothy - do not display grentimer at map start
				//CG_DrawPlayerGrenadeBar( &rect, color );
				CG_DrawAnalogueShader( &rect, color, cgs.media.hudTimerShader, cg.grenadeHudTime + Q3F_GRENADE_PRIME_TIME - cg.time, Q3F_GRENADE_PRIME_TIME);
			break;
		case CG_ALERT_ICON:
			CG_Q3F_DrawAlertIcon( &rect, anchorx, anchory, item->special );
			break;
		case CG_POWERUP_ICON:
			CG_Q3F_DrawPowerupIcon( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont, anchorx, anchory, item->special);
			break;
		case CG_POWERUP_ICONREV:
			CG_Q3F_DrawPowerupIconRev( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont, anchorx, anchory, item->special);
			break;
		case CG_GREN_TIMER_DIGITS:
			if(cg.grenadeHudTime != 0)		// slothy - do not display grentimer at map start
				CG_DrawGrenTimerDigits( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;

		case CG_FORTMENU_OPTION_1:
		case CG_FORTMENU_OPTION_2:
		case CG_FORTMENU_OPTION_3:
		case CG_FORTMENU_OPTION_4:
		case CG_FORTMENU_OPTION_5:
		case CG_FORTMENU_OPTION_6:
		case CG_FORTMENU_OPTION_7:
		case CG_FORTMENU_OPTION_8:
		case CG_FORTMENU_OPTION_9:
		case CG_FORTMENU_OPTION_0:
			CG_Q3F_Menu_DrawItem( 1, ownerDraw - CG_FORTMENU_OPTION_1, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_FORTMENU_RIGHT_1:
		case CG_FORTMENU_RIGHT_2:
		case CG_FORTMENU_RIGHT_3:
		case CG_FORTMENU_RIGHT_4:
		case CG_FORTMENU_RIGHT_5:
		case CG_FORTMENU_RIGHT_6:
		case CG_FORTMENU_RIGHT_7:
		case CG_FORTMENU_RIGHT_8:
		case CG_FORTMENU_RIGHT_9:
		case CG_FORTMENU_RIGHT_0:
			CG_Q3F_Menu_DrawItem( 2, ownerDraw - CG_FORTMENU_RIGHT_1, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_FORTMENU_NUMBER_1:
		case CG_FORTMENU_NUMBER_2:
		case CG_FORTMENU_NUMBER_3:
		case CG_FORTMENU_NUMBER_4:
		case CG_FORTMENU_NUMBER_5:
		case CG_FORTMENU_NUMBER_6:
		case CG_FORTMENU_NUMBER_7:
		case CG_FORTMENU_NUMBER_8:
		case CG_FORTMENU_NUMBER_9:
		case CG_FORTMENU_NUMBER_0:
			CG_Q3F_Menu_DrawItem( 0, ownerDraw - CG_FORTMENU_NUMBER_1, &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_FORT_MENU_TITLE:
			CG_Q3F_DrawMenuTitle( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont, item->text);
			break;
		case CG_SCOREBOARD_TITLE:
			CG_Q3F_DrawScoreboardTitle( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_SCOREBOARD_TEAMSCORES:
			CG_Q3F_DrawScoreboardTeamScores( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		case CG_HUDICONS:
			CG_Q3F_DrawHUDIcons( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont, item->special );
			break;

		case CG_LEVELSHOT:
			CG_Q3F_DrawLevelshot( &rect );
			break;

		case CG_LOADSTATUS:
			CG_Q3F_DrawLoadStatus(&rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont );
			break;

		case CG_LOADINFO:
			CG_Q3F_DrawLoadInfo(&rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont, item->special );
			break;

		case CG_MAPNAME:
			CG_Q3F_DrawMapName(&rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont );
			break;

		case CG_MAPINFO:
			CG_Q3F_DrawMapInfo(&rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont );
			break;

		case CG_LOADPROGRESS:
			CG_DrawMapLoadProgress(&rect);
			break;

		case CG_MOTD:
			CG_Q3F_DrawMotd(&rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont );
			break;

//		case CG_FORTLOGO:
//			CG_Q3F_DrawFortLogo( &rect );
//			break;

// slothy & shuriken
		case CG_ARMORTYPE_ICON:
			CG_DrawArmorIcon( &rect );
			break;

		case CG_ARMORTYPE_INFO:
			CG_DrawArmorType( &rect, scale, color, shader, textStyle, textalignment, text_x, text_y, parentfont );
			break;

		case CG_ARMORCLASS_ICON:
			CG_DrawArmorClassIcon( &rect );
			break;

		case CG_ACTIVE_WEAPON_ICON:
			CG_DrawActiveWeapon( &rect, anchorx, anchory, item->window.background);
			break;

		case CG_AGENTDISGUISE_FULLINFO:
			CG_DrawAgentDisguiseFullInfo( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;

		case CG_PLAYER_PIPES:
			CG_DrawPlayerPipes( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;
		
		case CG_TEAM_PIPES:
			CG_DrawTeamPipes( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;

		case HUD_VOTE_STRING:
			CG_DrawVoteString ( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont );
			break;

		case HUD_FLAG_INFO:
			if(cg.fi_endtime)
				CG_DrawFlagInfo( &rect, scale, color, textStyle, textalignment, text_x, text_y, parentfont, item->special );
			break;
// end slothy

	//	djbob
		default:
			break;
	}
}

void CG_MouseEvent(int x, int y) {
	int n;

	switch(cgs.eventHandling) {
	case CGAME_EVENT_EDITHUD:
		cgs.cursorX+= x;
		if (cgs.cursorX < 0)
			cgs.cursorX = 0;
		else if (cgs.cursorX > 640)
			cgs.cursorX = 640;

		cgs.cursorY += y;
		if (cgs.cursorY < 0)
			cgs.cursorY = 0;
		else if (cgs.cursorY > 480)
			cgs.cursorY = 480;

		n = Display_CursorType(cgs.cursorX, cgs.cursorY);
		cgs.activeCursor = 0;

		if (n == CURSOR_ARROW) {
			cgs.activeCursor = cgs.media.selectCursor;
		} else if (n == CURSOR_SIZER) {
			cgs.activeCursor = cgs.media.sizeCursor;
		}

		if (cgs.capturedItem) {
			Display_MouseMove(cgs.capturedItem, x, y);
		} else {
			Display_MouseMove(NULL, cgs.cursorX, cgs.cursorY);
		}
		break;
	case CGAME_EVENT_CUSTOMMENU:
	case CGAME_EVENT_MESSAGEMODE:
	case CGAME_EVENT_MENUMODE:
		break;

	case CGAME_EVENT_DEMO:
		cgs.cursorX += x;
		if ( cgs.cursorX < 0 ) {
			cgs.cursorX = 0;
		} else if ( cgs.cursorX > 640 ) {
			cgs.cursorX = 640;
		}

		cgs.cursorY += y;
		if ( cgs.cursorY < 0 ) {
			cgs.cursorY = 0;
		} else if ( cgs.cursorY > 480 ) {
			cgs.cursorY = 480;
		}

		//if ( x != 0 || y != 0 ) {
		//	cgs.cursorUpdate = cg.time + 5000;
		//}
		break;



	default:
		if ( (cg.predictedPlayerState.pm_type == PM_NORMAL ||
			  cg.predictedPlayerState.pm_type == PM_SPECTATOR ||
			  cg.predictedPlayerState.pm_type == PM_ADMINSPECTATOR) &&
			cg.showScores == qfalse ) {
			trap_Key_SetCatcher( trap_Key_GetCatcher() & ~KEYCATCH_CGAME );
			return;
		}
		break;
	}
}

/*
==================
CG_EventHandling
==================
 type 0 - no event handling
      2 - hud editor

*/
void CG_EventHandling(int type, qboolean fForced) {
	int oldtype = cgs.eventHandling;

	if ( cg.demoPlayback && type == CGAME_EVENT_NONE && !fForced ) {
		type = CGAME_EVENT_DEMO;
	}

	//Immediatly set type to prevent recursions
	cgs.eventHandling = type;
	/* Always init when setting none */
	if(type == CGAME_EVENT_NONE) {
		CG_Q3F_MenuCancel( qfalse );
		trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_CGAME);
		trap_Cvar_Set( "cl_bypassmouseinput", "0" );

		if( cg.demoPlayback ) {
			// Forcefully disable the help menu
		}
	}

	if ( oldtype == type )
		return;

	if( oldtype == CGAME_EVENT_EDITHUD ) {
		// Leaving edithud mode. Save to config.
		fileHandle_t fh;

		char buff[1024];
		const char *hudSet;

		trap_Cvar_VariableStringBuffer("cg_hudFiles", buff, sizeof(buff));

		hudSet = buff;
		if (hudSet[0] == '\0') {
			hudSet = "ui/hud.txt";
		}
		//trap_FS_FOpenFile( va( "ui/userhud%i.cfg", cg_userHud.integer ), &fh, FS_WRITE );
		Com_ExtractFilePath( hudSet, buff );
		trap_FS_FOpenFile( va( "%s/userhud%i.cfg", buff, cg_userHud.integer ), &fh, FS_WRITE );

		if( fh ) {
			int i;
			char tbuff[50];
			//keeg test this!
			Com_sprintf( tbuff, sizeof(tbuff), "// generated by %s, do not modify\n", GAME_NAME );
			trap_FS_Write( tbuff, strlen(tbuff), fh );

			for( i = 0; i < menuCount; i++ ) {
				char linebuff[256];
				menuDef_t *menu = Menu_Get( i );

				Com_sprintf( linebuff, sizeof(linebuff), "%s %f %f\n", menu->window.name, menu->window.rect.x, menu->window.rect.y );

				trap_FS_Write( linebuff, strlen(linebuff), fh );
			}
			trap_FS_FCloseFile( fh );
		}
	/* Clean up old event handlings */
	} else if (oldtype == CGAME_EVENT_MESSAGEMODE) {
		trap_SendConsoleCommand( "etf_stoptalk\n" );
	} else if (oldtype == CGAME_EVENT_MENUMODE) {
		trap_Cvar_Set( "cl_bypassmouseinput", "0" );
		CG_Q3F_MenuCancel( qfalse );
	} else if (oldtype == CGAME_EVENT_CUSTOMMENU) {
		trap_Cvar_Set( "cl_bypassmouseinput", "0" );
	}

	/* Setup the new event handling */
	 if (type == CGAME_EVENT_MENUMODE) {
		trap_Cvar_Set( "cl_bypassmouseinput", "1" );
		trap_Key_SetCatcher( KEYCATCH_CGAME );
	} else if (type == CGAME_EVENT_CUSTOMMENU) {
		trap_Cvar_Set( "cl_bypassmouseinput", "1" );
		trap_Key_SetCatcher( KEYCATCH_CGAME );
	} else if (type == CGAME_EVENT_MESSAGEMODE) {
		trap_Key_SetCatcher( KEYCATCH_CGAME );
	} else if ( type == CGAME_EVENT_DEMO ) {
		//cgs.fResize = qfalse;
		//cgs.fSelect = qfalse;
		//cgs.cursorUpdate = cg.time + 10000;
		cgs.timescaleUpdate = cg.time + 4000;
		if(cg.showScores)
		{
			cg.showScores = qfalse;
			cg.scoreFadeTime = cg.time;
		}
		trap_Key_SetCatcher( KEYCATCH_CGAME );
	}
}

void CG_KeyEvent(int key, qboolean down) {
	char *p;
	switch(cgs.eventHandling) {
		// Demos get their own keys
	case CGAME_EVENT_DEMO:
		CG_DemoClick( key, down );
		return;
	case CGAME_EVENT_EDITHUD:
		if ( cgs.capturedItem && key == K_MOUSE2 && !down) {
			cgs.capturedItem = NULL;
		}

		if (!down) {
			return;
		}

		Display_HandleKey(key, down, cgs.cursorX, cgs.cursorY);

		if (cgs.capturedItem) {
			cgs.capturedItem = NULL;
		} else {
			if (key == K_MOUSE2 && down) {
				cgs.capturedItem = Display_CaptureItem(cgs.cursorX, cgs.cursorY);
			}
		}
		break;
	case CGAME_EVENT_MESSAGEMODE:
		if(!down) 
			return;

		p = cg.q3f_messagemode_buffer;

		while(*p) {
			p++;
		}

		if( key == K_ENTER || key == K_KP_ENTER ) {
			switch(cg.q3f_messagemode_mode) {
			case Q3F_SAY_ALL:
				p = "say";
				break;
			case Q3F_SAY_TEAM:
				p = "say_team";
				break;
			case Q3F_SAY_ATTACKER:
				p = "tell_attacker";
				break;
			case Q3F_SAY_TARGET:
				p = "tell_target";
				break;
			default:
				p = "say";
				break;
			}
			trap_SendConsoleCommand(va("%s \"%s\"\n", p, cg.q3f_messagemode_buffer));
			cg.q3f_messagemode_buffer[0] = 0;
			CG_EventHandling(CGAME_EVENT_NONE, qfalse);
		}

		if( key == K_BACKSPACE ) {
			if( p != cg.q3f_messagemode_buffer ) {
				*--p = '\0';
			}
			return;
		}

		if(!(key & K_CHAR_FLAG)) {
			return;
		}

		key &= ~K_CHAR_FLAG;

		if( key < 32 ) {
			return;
		}
		if( key > 127) {
			return;
		}

		if( p - cg.q3f_messagemode_buffer >= (MAX_SAY_TEXT - 1) ) {
			return;
		}
		
		*p++ = key;
		*p = '\0';
		break;
	case CGAME_EVENT_MENUMODE:
		if (!down)
			break;
		if (key >= '0' && key <='9') {
			if (key == '0') 
				CG_Q3F_MenuChoice( 10 );
			else
				CG_Q3F_MenuChoice( key - '0');
		}
		break;
	case CGAME_EVENT_CUSTOMMENU:
		if (!down)
			break;
		CG_Q3F_CustomMenuKeyEvent( key );
		break;
	default:
		if ( ( cg.predictedPlayerState.pm_type == PM_NORMAL ||
			(cg.predictedPlayerState.pm_type == PM_SPECTATOR && cg.showScores == qfalse) ||
			(cg.predictedPlayerState.pm_type == PM_ADMINSPECTATOR && cg.showScores == qfalse))) {

			CG_EventHandling(CGAME_EVENT_NONE, qfalse);
			trap_Key_SetCatcher(trap_Key_GetCatcher() & ~KEYCATCH_CGAME );
			return;
		}
		break;
	}
}

int CG_ClientNumFromName(const char *p) {
	int i;
	for (i = 0; i < cgs.maxclients; i++) {
		if (cgs.clientinfo[i].infoValid && Q_stricmp(cgs.clientinfo[i].name, p) == 0) {
			return i;
		}
	}
	return -1;
}

void CG_RunMenuScript(char **args) {

}

/*
===================
CG_Q3F_GetTeamColor
===================
*/
void CG_Q3F_GetTeamColor (vec4_t hcolor, q3f_team_t teamnum) {
	hcolor[0] = hcolor[1] = hcolor[2] = 0.0;
	hcolor[3] = 1.0;
	switch( teamnum )
	{
		case Q3F_TEAM_RED:
			hcolor[0] = 1.0;
			break;
		case Q3F_TEAM_BLUE:
			hcolor[2] = 1.0;
			break;
		case Q3F_TEAM_YELLOW:
			hcolor[0] = 1.0;
			hcolor[1] = 1.0;
			break;
		case Q3F_TEAM_GREEN:
			hcolor[1] = 1.0;
			break;
		default:
			hcolor[0] = 1.0;
			hcolor[1] = 1.0;
			hcolor[2] = 1.0;
		break;
	}
}

void CG_Q3F_GetTeamColor2 (vec4_t *hcolor ) {
	(*hcolor)[0] = (*hcolor)[1] = (*hcolor)[2] = 0.0;
	(*hcolor)[3] = 1.0;
	switch( cg.snap->ps.persistant[PERS_TEAM] )
	{
		case Q3F_TEAM_RED:
			(*hcolor)[0] = 1.0;
			break;
		case Q3F_TEAM_BLUE:
			(*hcolor)[2] = 1.0;
			break;
		case Q3F_TEAM_YELLOW:
			(*hcolor)[0] = 1.0;
			(*hcolor)[1] = 1.0;
			break;
		case Q3F_TEAM_GREEN:
			(*hcolor)[1] = 1.0;
			break;
		default:
			(*hcolor)[0] = 1.0;
			(*hcolor)[1] = 1.0;
			(*hcolor)[2] = 1.0;
		break;
	}
}