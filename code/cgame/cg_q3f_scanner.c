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
**	cg_q3f_scanner.c
**
*/

#include "cg_q3f_scanner.h"
#include "cg_local.h"

static scannerdata_t scannerdata[MAX_CLIENTS];

/* VALID FIELDS TO PACK ARE:

  s.time2
  s.origin2[0]
  s.origin2[1]
  s.origin2[2]
  s.angles2[0]
  s.angles2[1]
  s.angles2[2]
  s.otherEntityNum
  s.modelindex2
  s.frame
  s.legsAnim
  s.torsoAnim
*/

void CG_Q3F_CheckForScannerData(struct centity_s *cent)
{
	struct centity_s *parent_ent;
	struct entityState_s *s;

	int clientnum;

	// First off... check if it's my scanner data, or somebody elses


	if(cent->currentState.otherEntityNum != cg.predictedPlayerEntity.currentState.number)
		return;

	parent_ent = &(cg.predictedPlayerEntity);

	if(parent_ent->currentState.otherEntityNum2 != Q3F_CLASS_RECON)
		return;				// It's not a scanner extended ent.

	s = &(cent->currentState);

	clientnum = s->time2 & 0xff;
	if(clientnum != 0xff)
	{
		scannerdata[clientnum].x = (parent_ent->currentState.pos.trBase[0] + ((((int)s->origin2[0] & 0xff))-128)*Q3F_SCANNER_RANGE_DIVISOR);
		scannerdata[clientnum].y = (parent_ent->currentState.pos.trBase[1] + ((((int)s->origin2[0] >> 8))-128)*Q3F_SCANNER_RANGE_DIVISOR);
		scannerdata[clientnum].z = parent_ent->currentState.pos.trBase[2] + s->origin2[2] * Q3F_SCANNER_RANGE_DIVISOR;
		scannerdata[clientnum].team = s->origin2[1];
		scannerdata[clientnum].last_updated = cg.time;
	}
/*	JT: Currently not filled
	clientnum = s->time2 >>8;
	if(clientnum != 0xff)
	{
		scannerdata[clientnum].x = -128 + (cent->currentState.origin[0] + ((int)s->origin2[2] & 0xff))*Q3F_SCANNER_RANGE_DIVISOR;
		scannerdata[clientnum].y = -128 + (cent->currentState.origin[1] + ((int)s->origin2[2] >> 8))*Q3F_SCANNER_RANGE_DIVISOR;
		scannerdata[clientnum].team = ((int)s->origin2[1] >>2) & 3;
		scannerdata[clientnum].last_updated = cg.time;
	}

	clientnum = (int)s->angles2 & 0xff;
	if(clientnum != 0xff)
	{
		scannerdata[clientnum].x = -128 + (cent->currentState.origin[0] + ((int)s->angles2[1] & 0xff))*Q3F_SCANNER_RANGE_DIVISOR;
		scannerdata[clientnum].y = -128 + (cent->currentState.origin[1] + ((int)s->angles2[1] >> 8))*Q3F_SCANNER_RANGE_DIVISOR;
		scannerdata[clientnum].team =  ((int)s->origin2[1] >>4) & 3;
		scannerdata[clientnum].last_updated = cg.time;
	}

	clientnum = (int)s->angles2 >> 8;
	if(clientnum != 0xff)
	{
		scannerdata[clientnum].x = -128 + (cent->currentState.origin[0] + ((int)s->angles2[2] & 0xff))*Q3F_SCANNER_RANGE_DIVISOR;
		scannerdata[clientnum].y = -128 + (cent->currentState.origin[1] + ((int)s->angles2[2] >> 8))*Q3F_SCANNER_RANGE_DIVISOR;
		scannerdata[clientnum].team = ((int)s->origin2[1] >>6) & 3;
		scannerdata[clientnum].last_updated = cg.time;
	}
 END JT*/
}

void CG_Q3F_ScannerDraw()
{
	int i;
	float		w, h;
//	float		cosratio, sinratio;
	vec3_t	temp_vec, temp_vec2, dest, normal;
	vec_t	len;
//	float theta;
	//int xoffs, yoffs;
	struct centity_s *cent;

	float		x, y; //, dx, dy;
	vec4_t		hcolor;
	qhandle_t   hModel;

	cent = &cg_entities[cg.snap->ps.clientNum];

	if ( cg.snap->ps.persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR)
	{
		return;
	}

	if(!(cg.snap->ps.stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_SCANNER)))
	{
		return;
	}

	w = h = 128;


	x = 188;			// JT: Used to be -128
	y = 172;			// JT: Used to be 128
	CG_AdjustFrom640( &x, &y, &w, &h );



	trap_R_DrawStretchPic( x + cg.refdef.x + 0.5 * (cg.refdef.width - w), 
		y + cg.refdef.y + 0.5 * (cg.refdef.height - h), 
		w, h, 0, 0, 1, 1, cgs.media.scannerShader );


	for(i=0; i < MAX_CLIENTS; i++)
	{
		if(cg.time - scannerdata[i].last_updated > Q3F_SCANNER_BLIP_FADEOUT)
		{
			continue;
		}
		w=h=8;



		CG_Q3F_ColorForTeam( hcolor, scannerdata[i].team );
		// Now... fade it, depending on how old it is.
		hcolor[3] = 1-((cg.time - scannerdata[i].last_updated)/(float)Q3F_SCANNER_BLIP_FADEOUT);
		trap_R_SetColor( hcolor );

		dest[0] = scannerdata[i].x;
		dest[1] = scannerdata[i].y;
		dest[2] = 0;
	
		VectorCopy(cent->currentState.pos.trBase,temp_vec2);
		temp_vec2[2] = 0;

		VectorSubtract (dest, temp_vec2, temp_vec);
		len = VectorLength(temp_vec);

		if(len > 64* Q3F_SCANNER_RANGE_DIVISOR)
			continue;				// If it's too far...don't bother.


		VectorNormalize(temp_vec);

		normal[0] = 0;
		normal[1] = 0;
		normal[2] = -1;

		RotatePointAroundVector(dest, normal, temp_vec, cg.snap->ps.viewangles[1]);
		VectorScale(dest, len/(float)Q3F_SCANNER_RANGE_DIVISOR, dest);

		y=-dest[0];		// JT - Flip Axes
		x=-dest[1];


		x +=188;		// JT - disabled to move HUD to center
		y +=172;			// JT - used to be 128

		if(abs(scannerdata[i].z - cent->currentState.pos.trBase[2]) < 5)
			hModel = cgs.media.scannerblipShader;
		else if(cent->currentState.pos.trBase[2] < scannerdata[i].z)
			hModel = cgs.media.scannerupShader;
		else
			hModel = cgs.media.scannerdownShader;

			

		CG_AdjustFrom640( &x, &y, &w, &h );
		trap_R_DrawStretchPic( x + cg.refdef.x + 0.5 * (cg.refdef.width - w), 
			y + cg.refdef.y + 0.5 * (cg.refdef.height - h), 
			w, h, 0, 0, 1, 1, hModel );
		trap_R_SetColor( NULL );
	}
}

void CG_Q3F_ColorForTeam(vec4_t hcolor, int teamnum)
{
	hcolor[0] = hcolor[1] = hcolor[2] = 0;
	hcolor[3] = 1.0;
	switch(teamnum)
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

/*
void CG_UpdateScanner(centity_t *cent)
{
	int x,y,team;
	int clientnum;

	
	y = cent->currentState.eventParm & 63;
	x = (cent->currentState.eventParm >> 5) & 63;
	team = (cent->currentState.eventParm >> 10) &3;
	clientnum = (cent->currentState.eventParm >> 16);
	scannerdata[clientnum].y = y;
	scannerdata[clientnum].x = x;
	scannerdata[clientnum].team = team;
	scannerdata[clientnum].last_updated = cg.time;
	CG_Printf("UpdateScanner called at %d (x=%d, y=%d,team=%d - Eventparm: %d\n",cg.time,x,y,team, cent->currentState.eventParm);
}
*/

// RR2DO2: new version
void CG_DrawScanner( rectDef_t *rect ) {
	int					i;
	vec3_t				temp_vec, temp_vec2, dest, normal;
	vec_t				len;
	struct centity_s	*cent;
	float				x, y;
	vec4_t				hcolor;
	qhandle_t			hModel;
	float				scannerScaleX, scannerScaleY;

	cent = &cg_entities[cg.snap->ps.clientNum];

	if ( cg.snap->ps.persistant[PERS_TEAM] == Q3F_TEAM_SPECTATOR) {
		return;
	}

	if( !( cg.snap->ps.stats[STAT_Q3F_FLAGS] & ( 1 << FL_Q3F_SCANNER ) ) ) {
		return;
	}

	scannerScaleX = rect->w/ 128.f;
	scannerScaleY = rect->h/ 128.f;

	for( i = 0; i < MAX_CLIENTS; i++) {
		float	w, h;

		if( cg.time - scannerdata[i].last_updated > Q3F_SCANNER_BLIP_FADEOUT ) {
			continue;
		}

		w = h = 8.f;

		CG_Q3F_ColorForTeam( hcolor, scannerdata[i].team );
		// Now... fade it, depending on how old it is.
		hcolor[3] = 1-((cg.time - scannerdata[i].last_updated)/(float)Q3F_SCANNER_BLIP_FADEOUT);
		trap_R_SetColor( hcolor );

		dest[0] = scannerdata[i].x;
		dest[1] = scannerdata[i].y;
		dest[2] = 0;
	
		VectorCopy(cent->currentState.pos.trBase,temp_vec2);
		temp_vec2[2] = 0;

		VectorSubtract (dest, temp_vec2, temp_vec);
		len = VectorLength(temp_vec);

		if( len > 64 * Q3F_SCANNER_RANGE_DIVISOR )
			continue;				// If it's too far...don't bother.


		VectorNormalize(temp_vec);

		normal[0] = 0;
		normal[1] = 0;
		normal[2] = -1;

		RotatePointAroundVector( dest, normal, temp_vec, cg.snap->ps.viewangles[1] );
		VectorScale( dest, len / (float)Q3F_SCANNER_RANGE_DIVISOR, dest );

		x = rect->x + ( rect->w / 2.f ) - ( dest[1] * scannerScaleX );
		y = rect->y + ( rect->h / 2.f ) - ( dest[0] * scannerScaleY );		// JT - Flip Axes

		if( abs( scannerdata[i].z - cent->currentState.pos.trBase[2] ) < 5 )
			hModel = cgs.media.scannerblipShader;
		else if( cent->currentState.pos.trBase[2] < scannerdata[i].z )
			hModel = cgs.media.scannerupShader;
		else
			hModel = cgs.media.scannerdownShader;			

		CG_AdjustFrom640( &x, &y, &w, &h );

		trap_R_DrawStretchPic( x, y, w, h, 0, 0, 1, 1, hModel );
		trap_R_SetColor( NULL );
	}
}
