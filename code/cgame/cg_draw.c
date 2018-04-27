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

// cg_draw.c -- draw all of the graphical elements during
// active (after loading) gameplay

#include "cg_local.h"
#include "../game/bg_local.h"
#include "../game/bg_q3f_weapon.h"
#include "../game/bg_q3f_util.h"
#include "../api/et/ui_public.h"
#include "cg_q3f_menu.h"
#include "cg_q3f_scanner.h"
#include "../game/bg_q3f_playerclass.h"
#include "cg_q3f_mapselect.h"
#include "../ui_new/ui_shared.h"

extern displayContextDef_t cgDC;
extern int menuCount;

int drawTeamOverlayModificationCount = -1;

int sortedTeamPlayers[TEAM_MAXOVERLAY];
int	numSortedTeamPlayers;

/*
=========================
CG_Q3F_GetTeamColorString
=========================
*/
const char *CG_Q3F_GetTeamColorString( int teamnum )
{
	static char	str[3];
	char	*s;

	switch( teamnum )
	{
		case Q3F_TEAM_RED:		s = "^1";	break;
		case Q3F_TEAM_BLUE:		s = "^4";	break;
		case Q3F_TEAM_YELLOW:	s = "^3";	break;
		case Q3F_TEAM_GREEN:	s = "^2";	break;
		default:				s = "^7";
	}

	Com_sprintf( str, sizeof( str ), "%s", s );
	return str;
}

int CG_Text_Width(const char *text, float scale, int limit, fontStruct_t *parentfont) {
	int count,len;
	float out;
	glyphInfo_t *glyph;
	float useScale;
	const /*unsigned*/ char *s = text;
	fontInfo_t *font;

	if( !(parentfont) || !(parentfont->fontRegistered) ) {
		if (scale <= cg_smallFont.value) {
			font = &cgDC.Assets.font.smallFont;
		} else if (scale > cg_bigFont.value) {
			font = &cgDC.Assets.font.bigFont;
		} else {
			font = &cgDC.Assets.font.textFont;
		}
	} else {
		if (scale <= cg_smallFont.value) {
			font = &parentfont->smallFont;
		} else if (scale > cg_bigFont.value) {
			font = &parentfont->bigFont;
		} else {
			font = &parentfont->textFont;
		}
	}

	useScale = scale * font->glyphScale;

	out = 0;
	if (text) {
		len = strlen(text);

		if (limit > 0 && len > limit) {
			len = limit;
		}

		count = 0;

		while (s && *s && count < len) {
			if(((*s >= GLYPH_CHARSTART) && (*s <= GLYPH_CHAREND)) || ((*s >= GLYPH_CHARSTART2) && (*s <= GLYPH_CHAREND2))) {
				if ( Q_IsColorStringPtr( s ) ) {
					s += 2;
					continue;
				} else {
					glyph = &font->glyphs[(unsigned char)*s];
					out += glyph->xSkip * useScale;
					s++;
					count++;
				}
			}
		}
	}

	return out;
}

void CG_Text_Width_To_Max(char *text, float scale, int max, fontStruct_t *parentfont) {
	int count;
	float out;
	glyphInfo_t *glyph;
	float useScale;
	char *s = text;
	fontInfo_t *font;

	if( !(parentfont) || !(parentfont->fontRegistered) ) {
		if (scale <= cg_smallFont.value) {
			font = &cgDC.Assets.font.smallFont;
		} else if (scale > cg_bigFont.value) {
			font = &cgDC.Assets.font.bigFont;
		} else {
			font = &cgDC.Assets.font.textFont;
		}
	} else {
		if (scale <= cg_smallFont.value) {
			font = &parentfont->smallFont;
		} else if (scale > cg_bigFont.value) {
			font = &parentfont->bigFont;
		} else {
			font = &parentfont->textFont;
		}
	}

	useScale = scale * font->glyphScale;

	out = 0;
	if (text) {
		count = 0;
		if ( out > max ) {
			*s = '\0';
			return;
		}
		while (s && *s) {
			if(((*s >= GLYPH_CHARSTART) && (*s <= GLYPH_CHAREND)) || ((*s >= GLYPH_CHARSTART2) && (*s <= GLYPH_CHAREND2))) {
				if ( Q_IsColorStringPtr(s) ) {
					s += 2;
					continue;
				} else {
					glyph = &font->glyphs[(unsigned char)*s];
					out += glyph->xSkip * useScale;
					s++;
					count++;
				}

				if ( out >= max ) {
					*s = '\0';
					return;
				}
			}
		}
	}
}

int CG_Text_Height(const char *text, float scale, int limit, fontStruct_t *parentfont) {
	int len, count;
	float max;
	glyphInfo_t *glyph;
	float useScale;
	const char *s = text;
	fontInfo_t *font;

	if( !(parentfont) || !(parentfont->fontRegistered) ) {
		if (scale <= cg_smallFont.value) {
			font = &cgDC.Assets.font.smallFont;
		} else if (scale > cg_bigFont.value) {
			font = &cgDC.Assets.font.bigFont;
		} else {
			font = &cgDC.Assets.font.textFont;
		}
	} else {
		if (scale <= cg_smallFont.value) {
			font = &parentfont->smallFont;
		} else if (scale > cg_bigFont.value) {
			font = &parentfont->bigFont;
		} else {
			font = &parentfont->textFont;
		}
	}

	useScale = scale * font->glyphScale;

	max = 0;

	if (text) {
		len = strlen(text);

		if (limit > 0 && len > limit) {
			len = limit;
		}

		count = 0;

		while (s && *s && count < len) {
			if(((*s >= GLYPH_CHARSTART) && (*s <= GLYPH_CHAREND)) || ((*s >= GLYPH_CHARSTART2) && (*s <= GLYPH_CHAREND2))) {
				if ( Q_IsColorStringPtr(s) ) {
					s += 2;
					continue;
				} else {
					glyph = &font->glyphs[(unsigned char)*s];
					if (max < glyph->height) {
						max = glyph->height;
					}
					s++;
					count++;
				}
			}
		}
	}
	return max * useScale;
}

void CG_Text_PaintChar(float x, float y, float width, float height, float scale, float s, float t, float s2, float t2, qhandle_t hShader) {
	float w, h;
	w = width * scale;
	h = height * scale;
	CG_AdjustFrom640( &x, &y, &w, &h );
	trap_R_DrawStretchPic( x, y, w, h, s, t, s2, t2, hShader );
}

void CG_Text_Paint_MaxWidth(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style, fontStruct_t *parentfont, int textalignment, int maxwidth) {
	static char buffer[1024];

	if(!maxwidth) {
		return;
	}

	Q_strncpyz(buffer, text, 1024);

	CG_Text_Width_To_Max(buffer, scale, maxwidth, parentfont);
	CG_Text_Paint(x, y, scale, color, buffer, adjust, limit, style, parentfont, textalignment);
}

void CG_Text_Paint(float x, float y, float scale, vec4_t color, const char *text, float adjust, int limit, int style, fontStruct_t *parentfont, int textalignment) {
	int len, count;
	vec4_t newColor;
	glyphInfo_t *glyph;
	float useScale;
	fontInfo_t *font;

	if( !(parentfont) || !(parentfont->fontRegistered) ) {
		if (scale <= cg_smallFont.value) {
			font = &cgDC.Assets.font.smallFont;
		} else if (scale > cg_bigFont.value) {
			font = &cgDC.Assets.font.bigFont;
		} else {
			font = &cgDC.Assets.font.textFont;
		}
	} else {
		if (scale <= cg_smallFont.value) {
			font = &parentfont->smallFont;
		} else if (scale > cg_bigFont.value) {
			font = &parentfont->bigFont;
		} else {
			font = &parentfont->textFont;
		}
	}


	useScale = scale * font->glyphScale;
	if (text) {
		const /*unsigned*/ char *s = text;
		// RR2DO2: for alignment
		int	alignmentoffset;

		switch( textalignment )
		{
		case ITEM_ALIGN_LEFT: 
			alignmentoffset = 0; 
			break;
		case ITEM_ALIGN_CENTER: 
			alignmentoffset = -0.5f * CG_Text_Width( text, scale, limit, parentfont ); 
			break;
		case ITEM_ALIGN_RIGHT: 
			alignmentoffset = -CG_Text_Width( text, scale, limit, parentfont ); 
			break;
		default: 
			alignmentoffset = 0; 
			break;
		}
		// RR2DO2

		trap_R_SetColor( color );
		memcpy(&newColor[0], &color[0], sizeof(vec4_t));
		len = strlen(text);

		if (limit > 0 && len > limit) {
			len = limit;
		}

		count = 0;

		while (s && *s && count < len) {
			if(((*s >= GLYPH_CHARSTART) && (*s <= GLYPH_CHAREND)) || ((*s >= GLYPH_CHARSTART2) && (*s <= GLYPH_CHAREND2))) {
				glyph = &font->glyphs[(unsigned char)*s];

				if ( Q_IsColorStringPtr( s ) ) {
					memcpy( newColor, g_color_table[ColorIndex(*(s+1))], sizeof( newColor ) );
					newColor[3] = color[3];
					trap_R_SetColor( newColor );
					s += 2;
					continue;
				} else {
					float yadj = useScale * glyph->top;

					if (style == ITEM_TEXTSTYLE_SHADOWED || style == ITEM_TEXTSTYLE_SHADOWEDMORE) {
						int ofs = style == ITEM_TEXTSTYLE_SHADOWED ? 1 : 2;

						colorBlack[3] = newColor[3];
						trap_R_SetColor( colorBlack );
						CG_Text_PaintChar(x + ofs + alignmentoffset, y - yadj + ofs, 
															glyph->imageWidth,
															glyph->imageHeight,
															useScale, 
															glyph->s,
															glyph->t,
															glyph->s2,
															glyph->t2,
															glyph->glyph);
						colorBlack[3] = 1.0;
						trap_R_SetColor( newColor );
					}
					CG_Text_PaintChar(x + alignmentoffset, y - yadj, 
														glyph->imageWidth,
														glyph->imageHeight,
														useScale, 
														glyph->s,
														glyph->t,
														glyph->s2,
														glyph->t2,
														glyph->glyph);
					x += (glyph->xSkip * useScale) + adjust;
				}
				s++;
				count++;
			}
		}
		trap_R_SetColor( NULL );
	}
}

/* RR2DO2: now has variable font size
==============
CG_DrawField

Draws large numbers for status bar and powerups
==============
*/
/*void CG_DrawField (int x, int y, int width, int value, int charwidth, int charheight) {
	char	num[16], *ptr;
	int		l;
	int		frame;

	if ( width < 1 ) {
		return;
	}

	// draw number string
	if ( width > 5 ) {
		width = 5;
	}

	switch ( width ) {
	case 1:
		value = value > 9 ? 9 : value;
		value = value < 0 ? 0 : value;
		break;
	case 2:
		value = value > 99 ? 99 : value;
		value = value < -9 ? -9 : value;
		break;
	case 3:
		value = value > 999 ? 999 : value;
		value = value < -99 ? -99 : value;
		break;
	case 4:
		value = value > 9999 ? 9999 : value;
		value = value < -999 ? -999 : value;
		break;
	}

	Com_sprintf (num, sizeof(num), "%i", value);
	l = strlen(num);
	if (l > width)
		l = width;
	x += 2 + charwidth*(width - l);

	ptr = num;
	while (*ptr && l)
	{
		if (*ptr == '-')
			frame = STAT_MINUS;
		else
			frame = *ptr -'0';

		CG_DrawPic( x,y, charwidth, charheight, cgs.media.numberShaders[frame] );
		x += charwidth;
		ptr++;
		l--;
	}
}*/

/*
================
CG_Draw3DModel

================
*/
void CG_Draw3DModel( float x, float y, float w, float h, qhandle_t model, qhandle_t skin, vec3_t origin, vec3_t angles ) {
	refdef_t		refdef;
	refEntity_t		ent;

	if ( !cg_draw3dIcons.integer || !cg_drawIcons.integer ) {
		return;
	}

	CG_AdjustFrom640( &x, &y, &w, &h );

	memset( &refdef, 0, sizeof( refdef ) );

	memset( &ent, 0, sizeof( ent ) );
	AnglesToAxis( angles, ent.axis );
	VectorCopy( origin, ent.origin );
	ent.hModel = model;
	ent.customSkin = skin;
	ent.renderfx = RF_NOSHADOW;		// no stencil shadows

	refdef.rdflags = RDF_NOWORLDMODEL;

	AxisClear( refdef.viewaxis );

	refdef.fov_x = 30;
	refdef.fov_y = 30;

	refdef.x = x;
	refdef.y = y;
	refdef.width = w;
	refdef.height = h;

	refdef.time = cg.time;

	trap_R_ClearScene();
	trap_R_AddRefEntityToScene( &ent, NULL );
	trap_R_RenderScene( &refdef );
}

/*
================
CG_DrawHead

Used for both the status bar and the scoreboard
================
*/
void CG_DrawHead( float x, float y, float w, float h, int clientNum, vec3_t headAngles ) {
	clipHandle_t	cm;
	clientInfo_t	*ci;
	float			len;
	vec3_t			origin;
	vec3_t			mins, maxs;
   int index = 0, cls = 0;
	centity_t* agentdata=NULL;
   int tc=0;
   vec4_t colour;

	ci = &cgs.clientinfo[ clientNum ];

	if( !ci->infoValid || ci->cls < Q3F_CLASS_RECON || ci->cls > Q3F_CLASS_CIVILIAN )
	{
		CG_DrawPic( x, y, w, h, trap_R_RegisterShaderNoMip("gfx/2d/defer.tga"));
		return;
	}

   //Keeg: if it's an agent and he's disguised, we want to display his disguised head
	if( cg.snap->ps.eFlags & (EF_Q3F_DISGUISE) ) {
		for( index = 0; index < MAX_ENTITIES; index++ ) {
			agentdata = &cg_entities[index];

			if( (agentdata->currentState.eType == ET_Q3F_AGENTDATA) &&
				agentdata->currentValid &&
				(agentdata->currentState.otherEntityNum == clientNum) )
				break;		// We've found one.
		}

		if( index == MAX_ENTITIES )
			agentdata = NULL;	// We might not have the control ent yet, or it's finished
	} else {
		agentdata = NULL;
	}

   cls = agentdata && agentdata->currentState.torsoAnim ? agentdata->currentState.torsoAnim : ci->cls;
   tc = agentdata && agentdata->currentState.weapon ? agentdata->currentState.weapon : (int)ci->team;  //team color
	
   if ( cg_draw3dIcons.integer ) {
		cm = *CG_Q3F_HeadModel( cls );
		if ( !cm ) {
			return;
		}

		// offset the origin y and z to center the head
		trap_R_ModelBounds( cm, mins, maxs );

		origin[2] = -0.5 * ( mins[2] + maxs[2] );
		origin[1] = 0.5 * ( mins[1] + maxs[1] );

		// calculate distance so the head nearly fills the box
		// assume heads are taller than wide
		len = 0.7 * ( maxs[2] - mins[2] );		
		origin[0] = len / 0.268;	// len / tan( fov/2 )


	   VectorCopy( CG_TeamColor( tc ), colour );
	   colour[3] = .75f;
	   CG_FillRect( x, y, w, h, colour );
		CG_Draw3DModel( x, y, w, h, *CG_Q3F_HeadModel( cls ), 0/**CG_Q3F_HeadSkin( cls )*/, origin, headAngles ); // headskin was tc rather than cls
	} else if ( cg_drawIcons.integer ) {
		VectorCopy( CG_TeamColor( tc ), colour );
		colour[3] = .75f;
		CG_FillRect( x, y, w, h, colour );
		CG_DrawPic( x + 1, y + 1, w - 2, h - 2, *CG_Q3F_ModelIcon( cls ) );
	}
}

/*
================
CG_DrawTeamBackground

================
*/
void CG_DrawTeamBackground( int x, int y, int w, int h, float alpha, int team )
{
	vec4_t		hcolor;

	hcolor[3] = alpha;
	if ( team == Q3F_TEAM_RED ) {
		hcolor[0] = 1;
		hcolor[1] = 0;
		hcolor[2] = 0;
	} else if ( team == Q3F_TEAM_BLUE ) {
		hcolor[0] = 0;
		hcolor[1] = 0;
		hcolor[2] = 1;
	} else if ( team == Q3F_TEAM_YELLOW ) {
		hcolor[0] = 1;
		hcolor[1] = 1;
		hcolor[2] = 0;
	} else if ( team == Q3F_TEAM_GREEN ) {
		hcolor[0] = 0;
		hcolor[1] = 1;
		hcolor[2] = 0;
	} else {
		return; // no team
	}
	trap_R_SetColor( hcolor );
	CG_DrawPic( x, y, w, h, cgs.media.teamStatusBar );
	trap_R_SetColor( NULL );
}

/* RR2DO2
===================
CG_Q3F_DrawProgress

===================
*/
void CG_Q3F_DrawProgress( int x, int y, int maxwidth, int height, int maxvalue, int absmaxvalue, int value, qhandle_t icon, vec4_t iconcolor )
{
	vec4_t		hcolor;
	int			width, drawx, drawy;
	float		valuefact;

	// only draw if we can draw something
	if ( absmaxvalue == 0 )
		return;

	//let's find out the width of the bar
	width = (value * maxwidth) / absmaxvalue;

	if( width > maxwidth )
		width = maxwidth;
	else if( width < 0 )
		width = 0;

	//let's find out the colour
	// 0% = red
	// 50% = yellow
	// >100% = green
	valuefact = ((float) value) / (float) maxvalue;
	hcolor[2] = 0;
	hcolor[3] = 0.3f;
	if ( valuefact <= 0 ) {
		hcolor[0] = 1;
		hcolor[1] = 0;
	}
	else if ( valuefact < 0.5f ) {
		hcolor[0] = 1;
		hcolor[1] = valuefact*2;
	}
	else if ( valuefact == 0.5f ) {
		hcolor[0] = 1;
		hcolor[1] = 1;
	}
	else if ( valuefact < 1.0f ) {
		hcolor[0] = 1 - ( ( valuefact - 0.5f ) * 2 );
		hcolor[1] = 1;
	}
	else if ( valuefact == 1.0f ) {
		hcolor[0] = 0;
		hcolor[1] = 1;
	}
	else {
		hcolor[0] = hcolor[2] = (cg.time & 256) ? 1 : 0;
		hcolor[1] = 1;
	}

	CG_FillRect( x, y, width, height, hcolor );

	// now let's draw the icon

	// first find out it's draw size and location
	// default size is (height-2)^2

	if ( width < height ) {
		drawx = x + 2;
		width -= 4;
		drawy = y + ((height - width) / 2);
	}
	else {
		drawx = x + width - (height - 2);
		width = height - 4;
		drawy = y + 2;
	}

	if ( width < 0 )
		width = 0;

	// then draw it
	trap_R_SetColor( iconcolor );
	CG_DrawPic( drawx, drawy, width, width, icon );
	trap_R_SetColor( NULL );
}


/*
==================
CG_DrawSnapshot
==================
*/
/*static void CG_DrawSnapshot( float y ) {
	CG_LowPriority_Printf(BOX_PRINT_MODE_CENTER, va( "time:%i snap:%i cmd:%i", cg.snap->serverTime, cg.latestSnapshotNum, cgs.serverCommandSequence ));
}*/

/*
==================
CG_Q3F_DrawLocation
==================
*/
/*static float CG_Q3F_DrawLocation( float y )
{
	vec4_t color;

	if( cgs.currentLocationTime <= cg.time || cg.hyperspace )
	{
		if( !(cgs.currentLocation = CG_Q3F_GetLocation( cg.predictedPlayerState.origin, qtrue )) )
			cgs.currentLocation = "Unknown Location";
		cgs.currentLocationTime = cg.time + 500;
	}

	CG_Q3F_GetTeamColor( color, cg.snap->ps.persistant[PERS_TEAM] );
	trap_R_SetColor( color );
	CG_DrawSmallStringColor( 638 - SMALLCHAR_WIDTH * strlen( cgs.currentLocation ), y + 2, cgs.currentLocation, color );

	return( y + SMALLCHAR_HEIGHT + 4 );
}*/


/*
=================
CG_DrawParticleCount
=================
*/
/*static void CG_DrawParticleCount( float y ) {
	// RR2DO2: Show Particle count
	CG_LowPriority_Printf(BOX_PRINT_MODE_CENTER, va( "Particles: %i", Particle_Count() ));
}*/

//===========================================================================================

/*
===================
CG_DrawReward
===================
*/
static void CG_DrawReward( void ) { 
	float	*color;
	int		i, count;
	float	x, y;
	char	buf[32];

	if ( !cg_drawRewards.integer ) {
		return;
	}
	color = CG_FadeColor( cg.rewardTime, REWARD_TIME );
	if ( !color ) {
		if (cg.rewardStack > 0) {
			for(i = 0; i < cg.rewardStack; i++) {
				cg.rewardSound[i] = cg.rewardSound[i+1];
				cg.rewardShader[i] = cg.rewardShader[i+1];
				cg.rewardCount[i] = cg.rewardCount[i+1];
			}
			cg.rewardTime = cg.time;
			cg.rewardStack--;
			color = CG_FadeColor( cg.rewardTime, REWARD_TIME );
			trap_S_StartLocalSound(cg.rewardSound[0], CHAN_ANNOUNCER);
		} else {
			return;
		}
	}

	trap_R_SetColor( color );

	if ( cg.rewardCount[0] >= 10 ) {
		y = 56;
		x = 320 - ICON_SIZE/2;
		CG_DrawPic( x, y, ICON_SIZE-4, ICON_SIZE-4, cg.rewardShader[0] );
		Com_sprintf(buf, sizeof(buf), "%d", cg.rewardCount[0]);
		x = ( SCREEN_WIDTH - CG_Text_Width( buf, 0.27f, 0, NULL) ) / 2.f;
		CG_Text_Paint( x, y + (ICON_SIZE * 0.5f), 0.27f, color, buf, 0, 0, 0, NULL, ITEM_ALIGN_LEFT);
	}
	else {
		count = cg.rewardCount[0];

		y = 56;
		x = 320 - count * ICON_SIZE/2;
		for ( i = 0 ; i < count ; i++ ) {
			CG_DrawPic( x, y, ICON_SIZE-4, ICON_SIZE-4, cg.rewardShader[0] );
			x += ICON_SIZE;
		}
	}
	trap_R_SetColor( NULL );
}


/*#define	MAX_LAGOMETER_PING	900
#define	MAX_LAGOMETER_RANGE	300*/

/*
================================================================================

CROSSHAIR

================================================================================
*/

qboolean CG_AlliedTeam( int team, int isAlliedTeam) {
	team--;
	if (team < 0 || team > 3)
		return qfalse;
	return cg.teamAllies[team] & (1 << isAlliedTeam);
}

/*
=================
CG_ScanForFriendlyCrosshairEntity
=================
*/
static qboolean CG_ScanForFriendlyCrosshairEntity( void ) {
	trace_t		trace;
	vec3_t		start, end;
	int			content;
	centity_t	*cent;
	clientInfo_t *ci;

	VectorCopy( cg.refdef.vieworg, start );
	VectorMA( start, 1024, cg.refdef.viewaxis[0], end );	// Golliwog: Was 8192

	CG_Trace( &trace, start, vec3_origin, vec3_origin, end, 
		cg.snap->ps.clientNum, CONTENTS_SOLID|CONTENTS_BODY );

	// if the player is in fog, don't show it
	content = CG_PointContents( trace.endpos, 0 );
	if ( content & CONTENTS_FOG ) {
		return qfalse;
	}

	// Golliwog: ID sentries too
	cent = &cg_entities[trace.entityNum];
	if( cent->currentState.eType == ET_Q3F_SENTRY )
	{
		// If they're gassed, ID's are out.
		if( cg.gasEndTime )
			return qfalse;
		ci = &cgs.clientinfo[cent->currentState.clientNum];
		if (CG_AlliedTeam( cg.snap->ps.persistant[PERS_TEAM], ci->team) && cent->currentState.legsAnim)
			return qtrue;
		return qfalse;
	}
	// Golliwog.

	// Ensiform: ID supplystations too
	cent = &cg_entities[trace.entityNum];
	if( cent->currentState.eType == ET_Q3F_SUPPLYSTATION )
	{
		// If they're gassed, ID's are out.
		if( cg.gasEndTime )
			return qfalse;
		ci = &cgs.clientinfo[cent->currentState.clientNum];
		if (CG_AlliedTeam( cg.snap->ps.persistant[PERS_TEAM], ci->team) && cent->currentState.legsAnim)
			return qtrue;
		return qfalse;
	}
	// Ensiform.

	// Golliwog: If they're not a client, don't show them
	if( cent->currentState.eType != ET_PLAYER )
		return qfalse;

	ci = &cgs.clientinfo[trace.entityNum];

	// if the player is invisible, don't show it
	if ( cent->currentState.powerups & ( 1 << PW_INVIS ) )
		return qfalse;

	// If there's extended agent invisibility, ditto
	if( cent->currentState.eFlags & EF_Q3F_INVISIBLE )
		return qfalse;

	// If they're gassed, ID's are out.
	if( cg.gasEndTime )
		return qfalse;

	// Golliwog: If the player is an agent, find another client to emulate
	if (!CG_AlliedTeam( cg.snap->ps.persistant[PERS_TEAM], ci->team))
	{
		if( cent->currentState.otherEntityNum2 == Q3F_CLASS_AGENT && cent->currentState.eFlags & EF_Q3F_DISGUISE )
			return qtrue;
		return qfalse;
	}

	return qtrue;
}

/*
=================
CG_DrawCrosshair
=================
*/
static void CG_DrawCrosshair(void) {
	float		w, h;
	qhandle_t	hShader;
	float		x, y;
	int			ca;
	int			x2, y2;
	vec4_t		hcolor;
	qboolean	isSpec = qfalse;

	CG_Q3F_ConcussionEffect2(&x2, &y2);

	if (  cg.scopeEnabled || 
		 cg.scopeTime > cg.time ) {
		return;
	}

	if(	cg.snap->ps.persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR ||
		cg.snap->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL )
		isSpec = qtrue;

	if ( cg.renderingThirdPerson || cg.renderingFlyBy || cg.rendering2ndRefDef ) {
		return;
	}
	// set color based on health
	if ( cg_crosshairHealth.integer && !isSpec) {
		CG_ColorForHealth( hcolor );
		trap_R_SetColor( hcolor );				// slothy - old health support
	} else {
		trap_R_SetColor( cg.xhairColor );
	}

	// green cursor for friends
	if(cg_friendlyCrosshair.integer && !isSpec) {
		if(CG_ScanForFriendlyCrosshairEntity()) {
			trap_R_SetColor(colorGreen);
		}
	}
	w = h = cg_crosshairSize.value;
	x = cg_crosshairX.integer + x2;
	y = cg_crosshairY.integer + y2;

	CG_AdjustFrom640( &x, &y, &w, &h );

	if(!isSpec)
		ca = cg_drawCrosshair.integer;
	else
		ca = 0;				// xhair nr 0 for specs

	if (ca < 0)
		ca = 0;

	hShader = cgs.media.crosshairShader[ ca % NUM_CROSSHAIRS ];

   trap_R_DrawStretchPic( x + cg.refdef.x + 0.5 * (cg.refdef_current->width - w), 
        y + cg.refdef.y + 0.5 * (cg.refdef_current->height - h),
        w, h, 0, 0, 1, 1, hShader );
   if ( cg.crosshairShaderAlt[ ca % NUM_CROSSHAIRS ] ) {
	   w = h = cg_crosshairSize.value;
	   x = cg_crosshairX.integer + x2;
	   y = cg_crosshairY.integer + y2;
	   CG_AdjustFrom640( &x, &y, &w, &h );

	   if(cg_crosshairHealth.integer) {
			trap_R_SetColor(hcolor);				// slothy - old health support
	   } else {
		   trap_R_SetColor(cg.xhairColorAlt);		// slothy
	   }

	   trap_R_DrawStretchPic( x + cg.refdef.x + 0.5 * (cg.refdef_current->width - w), 
            y + cg.refdef.y + 0.5 * (cg.refdef_current->height - h),
            w, h, 0, 0, 1, 1, cg.crosshairShaderAlt[ ca % NUM_CROSSHAIRS ] );
   }
   trap_R_SetColor( NULL );
}

//==============================================================================

/*
=================
CG_DrawVote
=================
*/
/*static void CG_DrawVote(void) {
	char	*s;
	int		sec;

	if ( !cgs.voteTime ) {
		return;
	}

	// play a talk beep whenever it is modified
	if ( cgs.voteModified ) {
		cgs.voteModified = qfalse;
		trap_S_StartLocalSound( cgs.media.talkSound, CHAN_LOCAL_SOUND );
	}

	sec = ( VOTE_TIME - ( cg.time - cgs.voteTime ) ) / 1000;
	if ( sec < 0 ) {
		sec = 0;
	}
	s = va("VOTE(%i):%s yes:%i no:%i", sec, cgs.voteString, cgs.voteYes, cgs.voteNo );
	CG_DrawSmallString( 0, 58, s, 1.0F );
}*/

static int CG_DrawScoreboard(qboolean force) {
	menuDef_t* menu;

	if(!force) {
		if(trap_Key_GetCatcher() & KEYCATCH_UI) {
			return qfalse;
		}

		// don't draw scoreboard during death while warmup up
		if ( cg.warmup && !cg.showScores ) {
			return qfalse;
		}

		if ( !(cg.showScores || cg.predictedPlayerState.pm_type == PM_DEAD )) {
			return qfalse;
		}
	}

	menu = Menus_FindByName("scoreboard");
	if(!menu) {
		return qfalse;
	}

	Menu_Paint(menu, qtrue);

	return qtrue;
}


/*
=================
CG_DrawIntermission
=================
*/
static void CG_DrawIntermission( void ) {
}

/*
=================
CG_DrawFollow
=================
*/
static qboolean CG_DrawFollow( void ) {
	if ( !(cg.snap->ps.pm_flags & PMF_FOLLOW) ) {
		return qfalse;
	}

	CG_LowPriority_Printf( BOX_PRINT_MODE_CENTER, va("following %s", cgs.clientinfo[ cg.snap->ps.clientNum ].name ));

	return qtrue;
}

/*
=================
CG_DrawChase
=================
*/
static qboolean CG_DrawChase( void ) {
	if ( !(cg.snap->ps.pm_flags & PMF_CHASE) ) {
		return qfalse;
	}

	CG_LowPriority_Printf( BOX_PRINT_MODE_CENTER, va("chasing %s", cgs.clientinfo[ cg.snap->ps.clientNum ].name ));

	return qtrue;
}



/*
=================
CG_DrawAmmoWarning
=================
*/
static void CG_DrawAmmoWarning( void ) {
	char buffer[64];

	if ( cg_drawAmmoWarning.integer == 0 ) {
		return;
	}

	if( cg.ceaseFire )
		return;

	*buffer = '\0';

	if( cg.lowAmmoWarning & 3 )
	{
		if ( cg.lowAmmoWarning == 2 ) {
			Q_strcat(buffer, 64, "OUT OF AMMO");
		} else {
			Q_strcat(buffer, 64, "LOW AMMO WARNING");
		}
	} else if( cg.lowAmmoWarning & 12 ) {
		if ( cg.lowAmmoWarning == 8 ) {
			Q_strcat(buffer, 64, "OUT OF CELLS");
		} else {
			Q_strcat(buffer, 64, "LOW CELLS WARNING");
		}
	}
	
	if(*buffer)
		CG_LowPriority_Printf(BOX_PRINT_MODE_CENTER, buffer);
}

/*
=================
CG_DrawTKWarning
=================
*/
static void CG_DrawTKWarning( void ) {
	if(cg.teamKillWarnTime < cg.time) {
		return;
	}

	CG_LowPriority_Printf(BOX_PRINT_MODE_CENTER, "WARNING: You Killed A Teammate");
}

/*
==================
CG_Q3F_DrawCeaseFire
==================
*/

static void CG_Q3F_DrawCeaseFire( void )
{
	if( cg.ceaseFire ) {
		CG_LowPriority_Printf( BOX_PRINT_MODE_CENTER, "CEASE FIRE!" );
	}
}


/*
=================
CG_ScanForCrosshairEntity
=================
*/
void CG_ScanForCrosshairEntity( void ) {
	trace_t		trace;
	vec3_t		start, end;
	int			content, agentteam, agentclass, mode;
	centity_t	*cent;
	clientInfo_t *ci;

	if( cg.time - cg.lastCrosshairCheck < 200 ) {
		return;
	}
	cg.lastCrosshairCheck = cg.time;

	VectorCopy( cg.refdef.vieworg, start );
	VectorMA( start, 1024, cg.refdef.viewaxis[0], end );	// Golliwog: Was 8192

	CG_Trace( &trace, start, vec3_origin, vec3_origin, end, 
		cg.snap->ps.clientNum, CONTENTS_SOLID|CONTENTS_BODY );

	// if the player is in fog, don't show it
	content = CG_PointContents( trace.endpos, 0 );
	if ( content & CONTENTS_FOG ) {
		return;
	}

	// Golliwog: ID sentries too
	cent = &cg_entities[trace.entityNum];
	if( cent->currentState.eType == ET_Q3F_SENTRY && cent->currentState.legsAnim != 99 )
	{
		// If they're gassed, ID's are out.
		if( cg.gasEndTime )
			return;
		ci = &cgs.clientinfo[cent->currentState.clientNum];
		if(	CG_AlliedTeam(cg.snap->ps.persistant[PERS_TEAM], ci->team) &&
			cent->currentState.legsAnim &&
			!CG_Q3F_ShowingSentryUpgradeMenu() )
		{
			// Only on same team

			cg.crosshairSentryLevel = cent->currentState.legsAnim;
			cg.crosshairSentryHealth = cent->currentState.torsoAnim;
			cg.crosshairSentryShells = cent->currentState.otherEntityNum;
			cg.crosshairSentryRockets = cent->currentState.otherEntityNum2;
			cg.crosshairSentryBored = cent->currentState.eFlags & EF_Q3F_SENTRYBORED;
			if(	cent->currentState.clientNum != cg.crosshairClientNum ||
				(cg.crosshairClientTime + 7000) < cg.time ||
				cent->currentState.legsAnim != cg.crosshairSentryLevel )
				cg.crosshairSentrySet = rand() & 3;			// We need a new 'set' of descriptions
			cg.crosshairClientNum = cent->currentState.clientNum;
			cg.crosshairClientTime = cg.time;
			cg.crosshairSupplyLevel = 0;
		}
		return;
	}
	// Golliwog.

	if( cent->currentState.eType == ET_Q3F_SUPPLYSTATION && cent->currentState.legsAnim != 99 )
	{
		// If they're gassed, ID's are out.
		if( cg.gasEndTime )
			return;
		ci = &cgs.clientinfo[cent->currentState.clientNum];
		if(	CG_AlliedTeam(cg.snap->ps.persistant[PERS_TEAM], ci->team) &&
			cent->currentState.legsAnim &&
			!CG_Q3F_ShowingSupplyStationUpgradeMenu() )
		{
			// Only on same team

			cg.crosshairSupplyLevel = cent->currentState.legsAnim;
			cg.crosshairSupplyHealth = cent->currentState.torsoAnim;
			cg.crosshairSupplyShells = (int)cent->currentState.origin2[0];
			cg.crosshairSupplyNails = (int)cent->currentState.origin2[1];
			cg.crosshairSupplyRockets = (int)cent->currentState.origin2[2];
			cg.crosshairSupplyCells = (int)cent->currentState.angles2[0];
			cg.crosshairSupplyArmor = (int)cent->currentState.angles2[1];
			cg.crosshairSupplyGrenades = (int)cent->currentState.angles2[2];
			cg.crosshairClientNum = cent->currentState.clientNum;
			cg.crosshairClientTime = cg.time;
			cg.crosshairSentryLevel = 0;
		}
		return;
	}

	// Golliwog: If they're not a client, don't show them
	if( cent->currentState.eType != ET_PLAYER )
		return;

	ci = &cgs.clientinfo[trace.entityNum];

	// if the player is invisible, don't show it
	if ( cent->currentState.powerups & ( 1 << PW_INVIS ) ) {
		return;
	}

	// If there's extended agent invisibility, ditto
	if( cent->currentState.eFlags & EF_Q3F_INVISIBLE )
		return;

	// If they're gassed, ID's are out.
	if( cg.gasEndTime )
		return;

	// Golliwog: If the player is an agent, find another client to emulate
	if(	!CG_AlliedTeam(cg.snap->ps.persistant[PERS_TEAM], ci->team))
	{
		if( cent->currentState.otherEntityNum2 == Q3F_CLASS_AGENT && cent->currentState.eFlags & EF_Q3F_DISGUISE )
		{
			// Search for agentdata
			for( content = 0; content < MAX_GENTITIES; content++ )
			{
				if( (cg_entities[ content ].currentState.eType == ET_Q3F_AGENTDATA) &&
					cg_entities[ content ].currentValid &&
					(cg_entities[ content ].currentState.otherEntityNum == trace.entityNum) )
					break;
			}
			if( content < MAX_GENTITIES )
			{
				// They're disguised, see what as.

				agentclass = cg_entities[ content ].currentState.torsoAnim;
				agentteam  = cg_entities[ content ].currentState.weapon;

				if( !agentclass )
					agentclass = Q3F_CLASS_AGENT;		// This is obvious, really.
					
				if( !agentteam )
					agentteam = ci->team;

				for( mode = 0; mode <= 2 && agentteam; mode++ )
				{
					// Scan through, relaxing rules each time.
					for( content = 0; content < MAX_CLIENTS; content++ )
					{
						if( !cgs.clientinfo[content].infoValid || content == cg.snap->ps.clientNum )
							continue;	// Not valid, or ourself
						if( cg_entities[content].currentState.otherEntityNum2 != agentclass && mode < 1 )
							continue;	// Not the same class
						if( cgs.clientinfo[content].team != (q3f_team_t)agentteam && mode < 2 )
							continue;	// Not the same team
						break;
					}
					if( content < MAX_CLIENTS )
					{
						cg.crosshairClientNum = content;		// Set our victim
						cg.crosshairClientTime = cg.time;
						cg.crosshairSentryLevel = 0;
						cg.crosshairSupplyLevel = 0;
						return;
					}
				}
				cg.crosshairClientNum = cg.snap->ps.clientNum;		// Set our victim
				cg.crosshairClientTime = cg.time;
				cg.crosshairSentryLevel = 0;
				cg.crosshairSupplyLevel = 0;
				return;		// Update as the local client himself - stupid, but better than showing the spies real data.
			}
			return;		// Something went badly wrong (agent disguise data unavailable?), don't ID at all.
		}
	}
	// Golliwog.

	// update the fade timer
	cg.crosshairClientNum = trace.entityNum;
	cg.crosshairClientTime = cg.time;
	cg.crosshairSentryLevel = 0;
	cg.crosshairSupplyLevel = 0;
}


/*
=================
CG_DrawWarmup
=================
*/
static void CG_DrawWarmup( void ) {
	int			sec;
	const char	*s;

	switch (cg.matchState) {
	case MATCH_STATE_WAITING:
		CG_LowPriority_Printf(BOX_PRINT_MODE_CENTER, "Waiting for players to join");
		return;
	case MATCH_STATE_READYUP:
		CG_LowPriority_Printf(BOX_PRINT_MODE_CENTER, "Waiting for players to ready up");
		return;
	case MATCH_STATE_WARMUP:
		break;
	default:
		return;
	}

	sec = cg.warmup;
	sec = ( sec - cg.time ) / 1000;
	if ( sec < 0 ) {
		cg.warmup = 0;
		sec = 0;
	}
	s = va(GAME_NAME_CAP" in: %i seconds", sec + 1 );
	if ( sec > 10 )
		CG_LowPriority_Printf(BOX_PRINT_MODE_CENTER, s);
	else {
		if ( sec < 10 && (sec & 1) ) {
			CG_Printf(BOX_PRINT_MODE_CENTER, S_COLOR_RED "%s", s );
		} else {
			CG_Printf(BOX_PRINT_MODE_CENTER, s);
		}
		cg.centerPrintTime = cg.time + 800;
	}
	if ( sec != cg.warmupCount ) {
		cg.warmupCount = sec;
		switch ( sec ) {
		case 0:
			trap_S_StartLocalSound( cgs.media.count1Sound, CHAN_ANNOUNCER );
			break;
		case 1:
			trap_S_StartLocalSound( cgs.media.count2Sound, CHAN_ANNOUNCER );
			break;
		case 2:
			trap_S_StartLocalSound( cgs.media.count3Sound, CHAN_ANNOUNCER );
			break;
		default:
			break;
		}
	}
}

/*
===================
CG_Q3F_DrawMapInfo
===================
*/

/*void CG_Q3F_DrawMapInfo()
{
	// Draw map information if the user hasn't selected their team/class yet

	char *lineptr, *lineendptr;
	int x, y;
	float colour[4];
	qboolean lastline;

	if(	( cg.snap->ps.persistant[PERS_TEAM] != Q3F_TEAM_SPECTATOR &&
		cg.snap->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_NULL ) ||
		( cg.snap->ps.persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR &&
		( cg.isSpectating || cg.renderingFlyBy ) ) )
	{
		if( cg.time >= cg.mapInfoFadeTime )
			return;
		colour[3] = ((float) cg.mapInfoFadeTime - cg.time) / 500;
	}
	else {
		cg.mapInfoFadeTime = cg.time + 500;
		colour[3] = 1;
	}

	if( cgs.mapinfowrapwidth != 40 )
	{
		cgs.mapinfowrapwidth = 40;
		memset( cgs.mapinfowrapped, 0, sizeof(cgs.mapinfowrapped) );
		cgs.mapinfowrapheight = CG_Q3F_WrapText( cgs.mapinfowrapped, cgs.mapinfo, cgs.mapinfowrapwidth, sizeof(cgs.mapinfowrapped) );
	}

	lineptr = cgs.mapinfowrapped;
	colour[0] = colour[1] = colour[2] = 1.0;

	y = 200 - (SMALLCHAR_HEIGHT * cgs.mapinfowrapheight >> 1);
	if( y < Q3F_MENU_STARTY )
		y = Q3F_MENU_STARTY;		// Try and keep it lined up with the menu
	while( *lineptr )
	{
		lineendptr = lineptr;
		while( *lineendptr && *lineendptr != '\n' )
			lineendptr++;
		lastline = *lineendptr != '\n';

		x = 576 - SMALLCHAR_WIDTH * (lineendptr - lineptr);	// Find where to draw from
		*lineendptr = 0;
		CG_DrawStringExt( x, y, lineptr, colour, qfalse, qtrue, SMALLCHAR_WIDTH, SMALLCHAR_HEIGHT, (640/SMALLCHAR_WIDTH) );
		if( lastline )
			break;
		*lineendptr = '\n';		// Restore for next time we draw the string
		lineptr = lineendptr + 1;
		y += SMALLCHAR_HEIGHT;
	}
}*/

static void CG_Q3F_DrawScope( void ) {
	vec4_t color;

	if(cg_hideScope.integer)
		return;

	if(cg.snap->ps.persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR || cg.snap->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL)
		return;

	if( cg.refdef.fov_x < 70 ) {
		// fade in or show
		if( !cg.scopeEnabled ) {
			cg.scopeTime = cg.time + Q3F_SCOPE_FADEINTIME;
			cg.scopeEnabled = qtrue;
		}

		if( cg.scopeTime > cg.time )
			VectorSet4( color, 0.f, 0.f, 0.f, 1.f - ( ( cg.scopeTime - cg.time ) / Q3F_SCOPE_FADEINTIME ) );
		else
			VectorSet4( color, 0.f, 0.f, 0.f, 1.f );
	} else if( cg.scopeEnabled ) {
		// start fadeout
		cg.scopeTime = cg.time + Q3F_SCOPE_FADEOUTTIME;
		cg.scopeEnabled = qfalse;

		VectorSet4( color, 0.f, 0.f, 0.f, 1.f );
		
	} else if( cg.scopeTime > cg.time ) {
		// fading out
		VectorSet4( color, 0.f, 0.f, 0.f, ( cg.scopeTime - cg.time ) / Q3F_SCOPE_FADEOUTTIME );
	}

	if( cg.scopeEnabled || cg.scopeTime > cg.time ) {
		trap_R_SetColor( color );

		if( cg.snap->ps.weapon == WP_SNIPER_RIFLE || cg.snap->ps.weapon == WP_ASSAULTRIFLE ) {
			float x, y, w, h;

			x = y = 0.f;
			w = 320.f;
			h = 240.f;

			CG_AdjustFrom640( &x, &y, &w, &h );
			trap_R_DrawStretchPic( x, y, w, h, 0.01f, 0.01f, 0.99f, 0.99f, cgs.media.hud_sniperscope );
			trap_R_DrawStretchPic( x + w, y, w, h, 0.99f, 0.01f, 0.01f, 0.99f, cgs.media.hud_sniperscope );
			trap_R_DrawStretchPic( x, y + h, w, h, 0.01f, 0.99f, 0.99f, 0.01f, cgs.media.hud_sniperscope );
			trap_R_DrawStretchPic( x + w, y + h, w, h, 0.99f, 0.99f, 0.01f, 0.01f, cgs.media.hud_sniperscope );

			x = 128.f;
			y = 48.f;
			w = h = 384.f;

			VectorSet( color, 0.f, 0.8f, 0.f );
			trap_R_SetColor( color );

			CG_AdjustFrom640( &x, &y, &w, &h );
			trap_R_DrawStretchPic( x, y, w, h, 0, 0, 1, 1, cgs.media.hud_sniperscopeXhair );

		} else {
			float x, y, w, h;

			x = y = 0.f;
			w = 320.f;
			h = 240.f;

			CG_AdjustFrom640( &x, &y, &w, &h );
			trap_R_DrawStretchPic( x, y, w, h, 0.01f, 0.01f, 0.99f, 0.99f, cgs.media.hud_binoculars );
			trap_R_DrawStretchPic( x + w, y, w, h, 0.99f, 0.01f, 0.01f, 0.99f, cgs.media.hud_binoculars );
			trap_R_DrawStretchPic( x, y + h, w, h, 0.01f, 0.99f, 0.99f, 0.01f, cgs.media.hud_binoculars );
			trap_R_DrawStretchPic( x + w, y + h, w, h, 0.99f, 0.99f, 0.01f, 0.01f, cgs.media.hud_binoculars );

			x = 24.f;
			y = 112.f;
			w = 64.f;
			h = 256.f;

			VectorSet( color, 0.f, 0.8f, 0.f );
			trap_R_SetColor( color );

			CG_AdjustFrom640( &x, &y, &w, &h );
			trap_R_DrawStretchPic( x, y, w, h, 0, 0, 1, 1, cgs.media.hud_binocularsTarget );

			x = 128.f;
			y = 48.f;
			w = h = 384.f;

			CG_AdjustFrom640( &x, &y, &w, &h );
			trap_R_DrawStretchPic( x, y, w, h, 0, 0, 1, 1, cgs.media.hud_binocularsXhair );

			x = 560.f;
			y = 112.f;
			w = 64.f;
			h = 256.f;

			CG_AdjustFrom640( &x, &y, &w, &h );
			trap_R_DrawStretchPic( x, y, w, h, 1, 1, 0, 0, cgs.media.hud_binocularsTarget );
		}

		trap_R_SetColor( NULL );
	}
}

//==================================================================================

void CG_DrawObits()
{
	int x, y, i;
	int todelete = 0;

	if(cg.numObits == 0)
		return;

	y = ui_altObitsY.integer + (12 * cg.numObits);
	i = cg.numObits - 1;

	trap_R_SetColor( colorWhite );

	while(i >= 0) {
		x = ui_altObitsX.integer - cg.obits[i].modlen;

		CG_Text_Paint (x, y, 0.2f, colorWhite, cg.obits[i].mod, 0, 0, 0, NULL, ITEM_ALIGN_LEFT);

		if(cg.obits[i].attacklen) {
			int ax = x - cg.obits[i].attacklen - 4;
			CG_Text_Paint (ax, y, 0.2f, colorWhite, cg.obits[i].attacker, 0, 0, 0, NULL, ITEM_ALIGN_LEFT);
		}

		x += (2 * cg.obits[i].modlen);
		x += 4;

		CG_Text_Paint (x, y, 0.2f, colorWhite, cg.obits[i].victim, 0, 0, 0, NULL, ITEM_ALIGN_LEFT);

		if(cg.obits[i].endtime < cg.time) {
			++todelete;
		}
		--i;
		y -= 12;
	}

	trap_R_SetColor( NULL );

	if(todelete > 0) {
		cg.numObits -= todelete;
		if(cg.numObits > 0)
			memmove(&cg.obits[0], &cg.obits[todelete], sizeof(altObit_t) * cg.numObits);
		memset(&cg.obits[cg.numObits], 0, sizeof(altObit_t) * todelete);
	}
}

//==================================================================================

//bani
void CG_DrawDemoRecording( void ) {
	char status[1024];
	char demostatus[128];
	char wavestatus[128];

	// tjw:	cl_demorecording, cl_demooffset, and cl_demofilename cvars
	//		are always referencing garbage in OSX.
	//		The etmain client doesn't seem to have this problem and the
	//		game engine shows the correct values...  Very strange. 
	//		BTW, this is a dirty hack.
 #if defined(__MACOS__)
	{
		char demorecording[2] = {"0"};
		trap_Cvar_VariableStringBuffer("cl_demorecording", demorecording, sizeof(demorecording));
		if(demorecording[0] == '1') {
			CG_Text_Paint( 5, cg_recording_statusline.integer, 0.2f, colorWhite, "recording demo, /stoprecord to finish", 0, 0, 0, NULL, ITEM_ALIGN_LEFT );
		}
		return;
	}
#endif

	if( !cl_demorecording.integer && !cl_waverecording.integer ) {
		return;
	}

	if( !cg_recording_statusline.integer ) {
		return;
	}

	if( cl_demorecording.integer ) {
		Com_sprintf( demostatus, sizeof( demostatus ), " demo %s: %ik ", cl_demofilename.string, cl_demooffset.integer / 1024 );
	} else {
		strncpy( demostatus, "", sizeof( demostatus ) );
	}

	if( cl_waverecording.integer ) {
		Com_sprintf( wavestatus, sizeof( demostatus ), " audio %s: %ik ", cl_wavefilename.string, cl_waveoffset.integer / 1024 );
	} else {
		strncpy( wavestatus, "", sizeof( wavestatus ) );
	}

	Com_sprintf( status, sizeof( status ), "RECORDING%s%s", demostatus, wavestatus );

	CG_Text_Paint( 5, cg_recording_statusline.integer, 0.2f, colorWhite, status, 0, 0, 0, NULL, ITEM_ALIGN_LEFT );
}

void CG_demoAviFPSDraw( void ) {
	qboolean fKeyDown = cgs.fKeyPressed[K_F1] | cgs.fKeyPressed[K_F2] | cgs.fKeyPressed[K_F3] | cgs.fKeyPressed[K_F4] | cgs.fKeyPressed[K_F5];

	if ( cg.demoPlayback && fKeyDown && cgs.aviDemoRate >= 0 ) {
		CG_Text_Paint( 42, 425, 0.2f, colorWhite,
						  ( ( cgs.aviDemoRate > 0 ) ? va( "^3Record AVI @ ^7%d^2fps", cgs.aviDemoRate ) : "^1Stop AVI Recording" ),
						  0, 0, 0, NULL, ITEM_ALIGN_LEFT );
	}
}

void CG_demoTimescaleDraw( void ) {
	if ( cg.demoPlayback && demo_drawTimeScale.integer != 0 ) {
		if( !cgs.demoPaused && cgs.timescaleUpdate > cg.time ) {
			char *s = va( "^3TimeScale: ^7%.1f", cg_timescale.value );

			CG_Text_Paint( 42, 400, 0.2f, colorWhite, s, 0, 0, 0, NULL, ITEM_ALIGN_LEFT );
		} else if( cgs.demoPaused ) {
			CG_Text_Paint( 42, 400, 0.2f, colorWhite, "^3TimeScale: ^1PAUSED^7", 0, 0, 0, NULL, ITEM_ALIGN_LEFT );
		}
	}
}

/*
=================
CG_Draw2D
=================
*/

void CG_DrawOnScreenText(void);

static void CG_Draw2D( void ) {

	// if we are taking a levelshot for the menu, don't draw anything
	if ( cg.levelShot ) {
		return;
	}

	if( cg.showScores || cg.snap->ps.pm_type == PM_INTERMISSION) {
		if ( cg.scoresRequestTime + 2000 < cg.time ) {
			// the scores are more than two seconds out of data,
			// so request new ones
			cg.scoresRequestTime = cg.time;
			if(!cg.demoPlayback) // Added
				trap_SendClientCommand( "score" );
		}
	}

	if(trap_Key_GetCatcher() & KEYCATCH_UI) {
		return;
	}

	CG_DrawOnScreenText();

	if ( !cg_draw2D.integer) {
		return;
	}

	if ( cg.snap->ps.pm_type == PM_INTERMISSION ) {
//		CG_SetupIntermissionMenu();
		CG_DrawIntermission();
		CG_Q3F_DrawMenuBox();
		return;
	}

	// draw everything + cursor
	if( cgs.eventHandling == CGAME_EVENT_EDITHUD ) {
		Menu_PaintAll();

		trap_R_SetColor( NULL );
		if (menuCount > 0) {
			CG_DrawPic( cgDC.cursorx-16, cgDC.cursory-6, 32, 32, cgDC.Assets.cursor);	// RR2DO2: Q3F Cursor art
		}
		return;
	}

	cg.scoreBoardShowing = CG_DrawScoreboard(qfalse);
	if ( cg_drawStatus.integer ) {
		Menu_PaintAll();
	}

	CG_Q3F_DrawMenuBox();

	if ( cg.snap->ps.persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR || cg.snap->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL) {	// RR2DO2
		CG_DrawCrosshair();
	} else {
		// don't draw any status if dead or the scoreboard is being explicitly shown
		if ( !cg.showScores && cg.snap->ps.stats[STAT_HEALTH] > 0 ) {
			CG_DrawAmmoWarning();
			CG_DrawTKWarning();
			CG_DrawCrosshair();
			CG_DrawReward();
			CG_Q3F_DrawCeaseFire();
		}
	}
	
	if ( cg_drawFollowText.integer ) {
		if ( !CG_DrawFollow() ) {
			if ( !CG_DrawChase() ) {
				CG_DrawWarmup();
			}
		}
	} else {
		CG_DrawWarmup();
	}

	if(cg_altObits.integer) {
		CG_DrawObits();
	}

	CG_demoAviFPSDraw();
	CG_demoTimescaleDraw();

	CG_DrawDemoRecording();
}

/*
=====================
CG_DrawActive

Perform all drawing needed to completely fill the screen
=====================
*/
void CG_DrawActive( stereoFrame_t stereoView ) {
	float		separation;
	vec3_t		baseOrg;//, ttOrigin, ttAngles;
	int			flashtime;
	float		flashcol[4];

	// optionally draw the info screen instead
	if ( !cg.snap ) {
		CG_Q3F_RenderLoadingScreen();
		return;
	}

	switch ( stereoView ) {
	case STEREO_CENTER:
		separation = 0;
		break;
	case STEREO_LEFT:
		separation = -cg_stereoSeparation.value / 2;
		break;
	case STEREO_RIGHT:
		separation = cg_stereoSeparation.value / 2;
		break;
	default:
		separation = 0;
		CG_Error( "CG_DrawActive: Undefined stereoView" );
	}


	// clear around the rendered view if sized down
	CG_TileClear();

	// offset vieworg appropriately if we're doing stereo separation
	VectorCopy( cg.refdef.vieworg, baseOrg );
	if ( separation != 0 ) {
		VectorMA( cg.refdef.vieworg, -separation, cg.refdef.viewaxis[1], cg.refdef.vieworg );
	}

	//Keeger  polybus hook
	CG_PB_RenderPolyBuffers();

	// RR2DO2: Render sunflares - NOTE: don't forget the 2nd line!
	CG_RenderSunflares();
	CG_SetFlareFader( NULL, NULL );
	// RR2DO2

	// Golliwog: Blind the user if required
	if( cg.snap->ps.powerups[PW_Q3F_FLASH] > cg.time && cg.snap->ps.stats[STAT_HEALTH] > 0 )
	{
		flashtime = cg.snap->ps.powerups[PW_Q3F_FLASH] - cg.time;
		flashcol[0] = flashcol[1] = flashcol[2] = 1;
		flashcol[3] = (flashtime > 500) ? 1 : (((float)flashtime) / 500.0f);
		if( flashcol[3] < 1 )
		{
			// draw 3D view
			trap_R_RenderScene( &cg.refdef );

			// restore original viewpoint if running stereo
			if ( separation != 0 ) {
				VectorCopy( baseOrg, cg.refdef.vieworg );
			}
		}
		CG_Q3F_Waypoint();
		CG_FillRect( 0, 0, 640, 480, flashcol );
	}
	else {
		// draw 3D view

		trap_R_RenderScene( &cg.refdef );
		CG_Q3F_Waypoint();

		// restore original viewpoint if running stereo
		if ( separation != 0 ) {
			VectorCopy( baseOrg, cg.refdef.vieworg );
		}
	}
	// Golliwog.

	// Golliwog: Render 'sniper wash' if required
	if( cg.sniperWashColour[3] )
		CG_FillRect( 0, 0, 640, 480, cg.sniperWashColour );
	// Golliwog.

	// Golliwog: Agent gas colour effect
	if( cg.gasCurrColour[3] )
		CG_FillRect( 0, 0, 640, 480, cg.gasCurrColour );
	// Golliwog.

	// djbob: pain flash
	if( cg_blood.integer && cg.bleedtime ) {
		if((cg.time - cg.bleedtime) > Q3F_BLOODFLASH_TIME )
			cg.bleedtime = 0;
		else {
			float alphascale;
			vec4_t flash = {1.0f, 0.0f, 0.0f, 1.0f};
			alphascale = (cg.bleeddmg / 100.0f) * 0.75f;
			if(alphascale > 0.75f)
				alphascale = 0.75f;

			flash[3] = (1 - ((cg.time - cg.bleedtime)/Q3F_BLOODFLASH_TIME)) * alphascale;

			CG_FillRect(0, 0, 640, 480, flash);
		}
	}
	// djbob
	
	// RR2DO2: Lensflare blinding
	if( cg.lensflare_blinded[3] ) {
		CG_FillRectAdditive( 0, 0, 640, 480, cg.lensflare_blinded );
	}
	// RR2DO2

	trap_R_SaveViewParms();
	// RR2DO2: scopes
	CG_Q3F_DrawScope();
	// RR2DO2

	// draw status bar and other floating elements
 	CG_Draw2D();
	trap_R_RestoreViewParms();
}

// RR2DO2: Sky portal
void CG_ETF_DrawSkyPortal( refdef_t *parentrefdef, vec4_t *parentflareblind, stereoFrame_t stereoView, vec3_t sky_origin ) {
	refdef_t	sky_refdef;
	float		separation;

	memcpy( &sky_refdef, parentrefdef, sizeof( *parentrefdef ) );
	cg.currentrefdef = &sky_refdef;

	VectorCopy( sky_origin, sky_refdef.vieworg );

	sky_refdef.time = cg.time;
	sky_refdef.rdflags |= RDF_SKYBOXPORTAL | RDF_DRAWINGSKY;

	switch ( stereoView ) {
	case STEREO_CENTER:
		separation = 0;
		break;
	case STEREO_LEFT:
		separation = -cg_stereoSeparation.value / 2;
		break;
	case STEREO_RIGHT:
		separation = cg_stereoSeparation.value / 2;
		break;
	default:
		separation = 0;
		CG_Error( "CG_ETF_DrawSkyPortal: Undefined stereoView" );
	}

	if ( separation != 0 ) {
		VectorMA( sky_refdef.vieworg, -separation, sky_refdef.viewaxis[1], sky_refdef.vieworg );
	}

	// set flare rendering to skyportal refdef
	CG_SetFlareRenderer( &sky_refdef, NULL );

	cg.rendering2ndRefDef = qtrue;
	cg.renderingSkyPortal = qtrue;
	//CG_AddPacketEntities();						// alter calcViewValues, so predicted player state is correct
	//CG_AddMarks();
	//CG_AddLocalEntities();
	//Spirit_RunFrame( cg.time, cg.oldTime, &sky_refdef, qfalse );		// RR2DO2: Add particles*/
	cg.rendering2ndRefDef = qfalse;
	cg.renderingSkyPortal = qfalse;

	// set flare rendering to main refdef
	CG_SetFlareRenderer( parentrefdef, parentflareblind );

	trap_R_RenderScene( &sky_refdef );

	cg.currentrefdef = &cg.refdef;
}

#define MAX_WORLDTEXT 512
#define MAX_TEXTLENGTH 256

typedef struct onsText_s 
{
	struct onsText_s *next;
	int			endtime;
	int			color;
	char		text[MAX_TEXTLENGTH];
	vec3_t		origin;
} onsText_t;

static onsText_t WorldText[MAX_WORLDTEXT];
static onsText_t * freeworldtext;			// List of world text
static onsText_t * activeworldtext;			// List of world text

void CG_InitWorldText( void ) {
	int i;

	memset( &WorldText, 0, sizeof(WorldText) );
	for( i = 0; i < MAX_WORLDTEXT - 1; i++ ) {
		WorldText[i].next = &WorldText[i+1];
	}

	freeworldtext = &WorldText[0];
	activeworldtext = NULL;
}
/*
================
CG_WorldToScreen
================
*/
qboolean CG_WorldToScreen(vec3_t point, float *x, float *y)
{
	vec3_t          trans;
	float           xc, yc;
	float           px, py;
	float           z;

	px = tan(cg.refdef.fov_x * M_PI / 360.0);
	py = tan(cg.refdef.fov_y * M_PI / 360.0);

	VectorSubtract(point, cg.refdef.vieworg, trans);

	xc = 640.0f / 2.0f;
	yc = 480.0f / 2.0f;

	z = DotProduct(trans, cg.refdef.viewaxis[0]);
	if(z <= 0.001f)
		return qfalse;

	if(x)
		*x = xc - DotProduct(trans, cg.refdef.viewaxis[1]) * xc / (z * px);

	if(y)
		*y = yc - DotProduct(trans, cg.refdef.viewaxis[2]) * yc / (z * py);

	return qtrue;
}

qboolean CG_AddOnScreenText( const char *text, vec3_t origin, int _color, float duration )
{
	onsText_t *worldtext = freeworldtext;
	if (!worldtext) return qfalse;

	freeworldtext = worldtext->next;
	worldtext->next=activeworldtext;
	activeworldtext=worldtext;

	/*With persistance, it doesn't make sense to cull it
	if(!CG_WorldToScreen(origin, &x, &y)) {
		activeworldtext=worldtext->next;
		worldtext->next=freeworldtext;
		freeworldtext=worldtext;
		return qfalse;
	}*/

	VectorCopy(origin, worldtext->origin);
	/*worldtext->x = x;
	worldtext->y = y;*/
	worldtext->endtime = cg.time + (int)((float)duration * 1000.f);
	worldtext->color = _color;
	Q_strncpyz(worldtext->text,text,MAX_TEXTLENGTH);
	return qtrue;
}

void CG_DrawOnScreenText(void) {
	onsText_t *worldtext;
	onsText_t * * whereworldtext;
	//trace_t	tr;
	const float fTxtScale = 0.27f;
	float x,y;
	union 
	{
		char		m_RGBA[4];
		int			m_RGBAi;
	} ColorUnion;
	ColorUnion.m_RGBAi = 0xFFFFFFFF;

	/* Render/Move the world text */
	worldtext = activeworldtext;
	whereworldtext=&activeworldtext;

	while( worldtext ) 
	{
		/* Check for expiration */
		if(worldtext->endtime < cg.time) 
		{
			/* Clear up this world text */
			*whereworldtext=worldtext->next;
			worldtext->next=freeworldtext;
			freeworldtext=worldtext;
			worldtext=*whereworldtext;
			continue;
		}
		
		if(CG_WorldToScreen(worldtext->origin, &x, &y) && trap_R_inPVS(cg.refdef.vieworg, worldtext->origin))
		{
			//CG_Trace(&tr, cg.refdef.vieworg, NULL, NULL, worldtext->origin, -1, CONTENTS_SOLID);

			///* Check for in a solid */
			//if(tr.fraction < 1.0f) 
			//{
			//	/* Clear up this world text */
			//	*whereworldtext=worldtext->next;
			//	worldtext->next=freeworldtext;
			//	freeworldtext=worldtext;
			//	worldtext=*whereworldtext;
			//	continue;
			//}

			ColorUnion.m_RGBAi = worldtext->color;

			//FIXME - use correct function for each game, and handle new lines as well.
			//FIXME - need to make the text follow around instead of creating a new paint each time...

			{
				const char *tokens = "\n";
				const char *tok = 0;
				char temp[1024];
				int heightOffset = 0;
				vec4_t v4Color = 
				{
					(float)ColorUnion.m_RGBA[0]/255.f,
					(float)ColorUnion.m_RGBA[1]/255.f,
					(float)ColorUnion.m_RGBA[2]/255.f,
					(float)ColorUnion.m_RGBA[3]/255.f,
				};

				Q_strncpyz(temp,worldtext->text,1024);
				tok = strtok(temp,tokens);
				while(tok)
				{
					const int width = CG_Text_Width(tok,fTxtScale,0,NULL);
					const int height = CG_Text_Height(tok,fTxtScale,0,NULL);

					CG_Text_Paint(
						x - width/2, 
						y + heightOffset,
						fTxtScale, 
						v4Color, 
						tok, 
						0, 0, 0, NULL,
						ITEM_TEXTSTYLE_NORMAL);

					heightOffset += height*1.5;
					tok = strtok(NULL,tokens);
				}
			}
		}		

		/*CG_Text_Paint(worldtext->x, worldtext->y, fTxtScale, colorWhite, worldtext->text, 
			0, 0, ITEM_TEXTSTYLE_NORMAL);*/
		trap_R_SetColor(NULL);

		whereworldtext=&worldtext->next;
		worldtext = worldtext->next;
	}
}
