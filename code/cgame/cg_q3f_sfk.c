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
**	cg_q3f_sfk.c
**
**	Skills For Kills functions
**
*/

#include "cg_local.h"

#define	SFK_SNIPER_STANDING_OFFSET	(DEFAULT_VIEWHEIGHT * 0.75)
#define	SFK_SNIPER_CROUCHING_OFFSET	(CROUCH_VIEWHEIGHT * 0.75)

/*void CG_SFK_ParseData( char *data )
{
	// Decode the data sent across from the server.

	int bitfield;

	sscanf( data, "%d/%d/%d/%d/%d/%d/%d/%d/%d",
			&bitfield,				&cgs.sfkBonusSize,		&cgs.sfkSkillLevel1,
			&cgs.sfkSkillLevel2,	&cgs.sfkSkillLevel3,	&cgs.sfkSurvivalBonus,
			&cgs.sfkSkillMsgTime,	&cgs.sfkMineSize,		&cgs.sfkJetStrength );

		// These must match the order in the qagame code.
	cgs.sfkEnabled		= (bitfield & 0x01) ? qtrue : qfalse;
	cgs.sfkCaptureBonus	= (bitfield & 0x02) ? qtrue : qfalse;
	cgs.sfkCaptureCarry	= (bitfield & 0x04) ? qtrue : qfalse;
	cgs.sfkSpySkill		= (bitfield & 0x08) ? qtrue : qfalse;
	cgs.sfkProxMine		= (bitfield & 0x10) ? qtrue : qfalse;
	cgs.sfkMortar		= (bitfield & 0x20) ? qtrue : qfalse;
	cgs.sfkSafeTeamMine	= (bitfield & 0x40) ? qtrue : qfalse;
	cgs.sfkRegen		= (bitfield & 0x80) ? qtrue : qfalse;
}*/

void CG_SFK_DrawSkillometer( int left, int top, int right, int bottom )
{
	// Render the 'skills' bar.

	float width, skillx, level1x, level2x, clipx, intensity;
	vec4_t hcolor;

	width = right - left;
	skillx	= left + width * ((float) cg.snap->ps.stats[STAT_Q3F_SKILL] / (float) SFK_SKILLLEVELMAX);
	level1x	= left + width * ((float) SFK_SKILLLEVEL1 / (float) SFK_SKILLLEVELMAX);
	level2x	= left + width * ((float) SFK_SKILLLEVEL2 / (float) SFK_SKILLLEVELMAX);
	hcolor[3] = 0.6f;
	intensity = 0.85 + 0.15 * cos( 0.003 * cg.time );

	if( skillx > left )
	{
		// Selected red
		hcolor[0] = intensity;
		hcolor[1] = hcolor[2] = 0.0;
		CG_FillRect( left, top, (skillx > level1x ? level1x : skillx) - left, bottom - top, hcolor );
	}
	if( skillx > level1x )
	{
		// Selected yellow
		hcolor[0] = hcolor[1] = intensity;
		hcolor[2] = 0.0;
		CG_FillRect( level1x, top, (skillx > level2x ? level2x : skillx) - level1x, bottom - top, hcolor );
	}
	if( skillx > level2x )
	{
		// Selected green
		hcolor[1] = intensity;
		hcolor[0] = hcolor[2] = 0.0;
		CG_FillRect( level2x, top, skillx - level2x, bottom - top, hcolor );
	}

	if( skillx < level1x )
	{
		// Unselected red
		hcolor[0] = 0.3f;
		hcolor[1] = hcolor[2] = 0.0;
		CG_FillRect( skillx, top, level1x - skillx, bottom - top, hcolor );
	}
	if( skillx < level2x )
	{
		// Unselected yellow
		hcolor[0] = hcolor[1] = 0.3f;
		hcolor[2] = 0.0;
		clipx = skillx < level1x ? level1x : skillx;
		CG_FillRect( clipx, top, level2x - clipx, bottom - top, hcolor );
	}
	if( skillx < right )
	{
		// Unselected green
		hcolor[1] = 0.3f;
		hcolor[0] = hcolor[2] = 0.0;
		clipx = skillx < level2x ? level2x : skillx;
		CG_FillRect( clipx, top, right - clipx, bottom - top, hcolor );
	}
}

/*void CG_SFK_SkillInfo( void )
{
	// Display skill information to clients.

	if( !cgs.sfkEnabled )
	{
		CG_Printf( "Skills are disabled.\n" );
		return;
	}

	CG_Printf(	"Skills are enabled, and granted on: " );
	if( cgs.sfkCaptureBonus )
		CG_Printf( "team points, " );
	if( cgs.sfkCaptureCarry )
		CG_Printf( "flag carrying, " );
	CG_Printf( "%s%d consecutive frags.\n",
				((cgs.sfkCaptureBonus || cgs.sfkCaptureCarry) ? " or " : ""),
				cgs.sfkSkillLevel1 );
	CG_Printf( "Maximum skill is %d.\n", cgs.sfkSkillLevel3 );
	CG_Printf(	"Regeneration starts at level %d. Health will %sregenerate.\n",
				cgs.sfkSkillLevel2,
				(cgs.sfkRegen ? "" : "NOT ") ); 
	CG_Printf( "Recon jetpack consumes %d cell%s per second.\n",
				cgs.sfkJetStrength, (cgs.sfkJetStrength == 1 ? "" : "s") );
	CG_Printf( "Grenadier has %d-rocket %s mines. Teammates can %strigger them.\n",
				cgs.sfkMineSize, cgs.sfkProxMine ? "proximity" : "trip",
				cgs.sfkSafeTeamMine ? "NOT " : "" );
	CG_Printf(	"Minigunner has a %s.\n",
				cgs.sfkMortar ? "mortar" : "blaster" );
	CG_Printf(	"Spies %sgain in skill.\n",
				cgs.sfkSpySkill ? "" : "do NOT " );
	CG_Printf(	"Civilians becomes skilled after %d seconds.\n", cgs.sfkSurvivalBonus );
}*/

float CG_SFK_AngleToCoord( float angle, int coordRange, float fov, qboolean reverseCoord )
{
	// Take an angle and return the coord it maps to (may be out of visible range)
	// The conversion is: correct fov to degrees: (90.0 / fov), convert to radians: M_PI / 180,
	// obtain tangent (results in -1 - 1, at least in visible coords). The first two steps can
	// be combined, hence the M_PI * 0.5f / fov.

	angle = tan( angle * M_PI * 0.5f / fov );
	angle = coordRange * (reverseCoord ? (1.0f - angle) : (1.0f + angle));
	return( angle );
}
void CG_SFK_SniperHighlight()
{
	// Highlight all visible players if we have the sniper rifle active.
	// Assumes we've rendered the world and are now rendering 2d overlays.

	int i, offset, team, anim;
	centity_t *cent;
	vec3_t position, angles, viewdir;
	float *colour;
	vec4_t black;
	float playerX, playerY, adjustedX, adjustedY, size;
	qboolean timeFlip;

	if( (cg.snap->ps.extFlags & EXTF_SKILLED) &&
		cg.snap->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_SNIPER &&//*/
		cg.sniperDotEnt )
	{
		AngleVectors( cg.refdefViewAngles, viewdir, NULL, NULL );

		for( i = 0; i < cg.snap->numEntities; i++ )
		{
			cent = &cg_entities[offset = cg.snap->entities[i].number];
			if( offset > MAX_CLIENTS || offset == cg.clientNum )
//				continue;
			{
				colour = CG_TeamColor( team = Q3F_TEAM_SPECTATOR );

					// Calculate the angles relative to the view.
				position[0] = cent->lerpOrigin[0] - cg.refdef.vieworg[0];
				position[1] = cent->lerpOrigin[1] - cg.refdef.vieworg[1];
				position[2] = cent->lerpOrigin[2] - cg.refdef.vieworg[2] + SFK_SNIPER_STANDING_OFFSET;
				vectoangles( position, angles );
				VectorNormalize( position );
				if( (size = DotProduct( viewdir, position )) < 0 )
					continue;
				angles[PITCH]	= AngleNormalize180( angles[PITCH] - cg.refdefViewAngles[PITCH] );
				angles[YAW]		= AngleNormalize180( angles[YAW] - cg.refdefViewAngles[YAW] );
				playerX = CG_SFK_AngleToCoord( angles[YAW],		320,	cg.refdef.fov_x,	qtrue );
				playerY = CG_SFK_AngleToCoord( angles[PITCH],	240,	cg.refdef.fov_y,	qfalse );

					// Calculate a frame in advance to counter for ping.
				position[0] = cent->lerpOrigin[0] - cg.refdef.vieworg[0];
				position[1] = cent->lerpOrigin[1] - cg.refdef.vieworg[1];
				position[2] = cent->lerpOrigin[2] - cg.refdef.vieworg[2] + SFK_SNIPER_STANDING_OFFSET;
				vectoangles( position, angles );
				VectorNormalize( position );
				if( (size = DotProduct( viewdir, position )) < 0 )
					continue;
				angles[PITCH]	= AngleNormalize180( angles[PITCH] - cg.refdefViewAngles[PITCH] );
				angles[YAW]		= AngleNormalize180( angles[YAW] - cg.refdefViewAngles[YAW] );
				adjustedX = CG_SFK_AngleToCoord( angles[YAW],	320,	cg.refdef.fov_x,	qtrue );
				adjustedY = CG_SFK_AngleToCoord( angles[PITCH],	240,	cg.refdef.fov_y,	qfalse );

					// Draw rectangle
				black[0] = black[1] = black[2]	= 0;
				black[3] = colour[3]			= 0.6f;
				timeFlip = (cg.time & 512);
				size = cent->pe.visibleClass == Q3F_CLASS_SNIPER ? 4 : 2;

				CG_DrawRect( adjustedX - 16, adjustedY - 16, 32, 32, size, timeFlip ? (float *) black : colour );
				CG_DrawRect( adjustedX - 12, adjustedY - 12, 24, 24, size, timeFlip ? colour : (float *) black );
				CG_FillRect( playerX - 1, playerY - 1, 2, 2, colour );
			}
			else {
				colour = CG_TeamColor( team = cgs.clientinfo[offset].team );
				anim = cent->currentState.legsAnim & ~ANIM_TOGGLEBIT;
				size =	(anim == ANI_MOVE_WALKCROUCH || anim == ANI_MOVE_IDLECROUCH)
						? SFK_SNIPER_CROUCHING_OFFSET
						: SFK_SNIPER_STANDING_OFFSET;

					// Calculate the angles relative to the view.
				position[0] = cent->lerpOrigin[0] - cg.refdef.vieworg[0];
				position[1] = cent->lerpOrigin[1] - cg.refdef.vieworg[1];
				position[2] = cent->lerpOrigin[2] - cg.refdef.vieworg[2] + size;
				vectoangles( position, angles );
				VectorNormalize( position );
				if( (size = DotProduct( viewdir, position )) < 0 )
					continue;
				angles[PITCH]	= AngleNormalize180( angles[PITCH] - cg.refdefViewAngles[PITCH] );
				angles[YAW]		= AngleNormalize180( angles[YAW] - cg.refdefViewAngles[YAW] );
				playerX = CG_SFK_AngleToCoord( angles[YAW],		320,	cg.refdef.fov_x,	qtrue );
				playerY = CG_SFK_AngleToCoord( angles[PITCH],	240,	cg.refdef.fov_y,	qfalse );


					// Calculate a frame in advance to counter for ping.
				BG_EvaluateTrajectory( &cent->currentState.pos, cg.time + cg.snap->ping, position );
				position[2] += size;

				VectorSubtract( position, cg.refdef.vieworg, position );
				vectoangles( position, angles );
				VectorNormalize( position );
				if( (size = DotProduct( viewdir, position )) < 0 )
					continue;
				angles[PITCH]	= AngleNormalize180( angles[PITCH] - cg.refdefViewAngles[PITCH] );
				angles[YAW]		= AngleNormalize180( angles[YAW] - cg.refdefViewAngles[YAW] );
				adjustedX = CG_SFK_AngleToCoord( angles[YAW],	320,	cg.refdef.fov_x,	qtrue );
				adjustedY = CG_SFK_AngleToCoord( angles[PITCH],	240,	cg.refdef.fov_y,	qfalse );

					// Draw rectangle
				black[0] = black[1] = black[2]	= 0;
				black[3] = colour[3]			= 0.6f;
				timeFlip = (cg.time & 1024);
				size = cent->pe.visibleClass == Q3F_CLASS_SNIPER ? 4 : 2;

				CG_DrawRect( adjustedX - 16, adjustedY - 16, 32, 32, size, timeFlip ? (float *) black : colour );
				CG_DrawRect( adjustedX - 12, adjustedY - 12, 24, 24, size, timeFlip ? colour : (float *) black );
				CG_FillRect( playerX - 1, playerY - 1, 2, 2, colour );
			}
		}
	}
}

void CG_SFK_Mine( centity_t *cent )
{
	// Display the SFK mine.

	int timediff, i, j;
	float size;
	refEntity_t ref;
	vec3_t temp;

	if( !cent->currentState.time )
		return;

	if( !cgs.media.sfkMineModel )
		cgs.media.sfkMineModel = trap_R_RegisterModel( "models/ammo/napalm/napalm.md3" );

	timediff = cent->currentState.time - cg.time;
	if( timediff > 0 )
	{
		// Draw in 'spin-up' mode.

		if( timediff > 2000 )
			return;
		size = 0.5f - 0.5f * sin( M_PI * ((float) (timediff - 1000)) * 0.0005 );

		memset( &ref, 0, sizeof(ref) );
		AnglesToAxis( cent->currentState.angles, ref.axis );

			// This is RotateAroundDirection() on a different axis. No idea how it works... :(
		PerpendicularVector( ref.axis[0], ref.axis[2] );
		VectorCopy( ref.axis[0], temp );
		RotatePointAroundVector( ref.axis[0], ref.axis[2], temp, (1.0f - size) * 360.0f );
		CrossProduct( ref.axis[2], ref.axis[0], ref.axis[1] );

		for( i = 0; i < 3; i++ )
			for( j = 0; j < 3; j++ )
				ref.axis[i][j] *= size;
		ref.nonNormalizedAxes = qtrue;
	}
	else {
		// Draw static.

		memset( &ref, 0, sizeof(ref) );
		AnglesToAxis( cent->currentState.angles, ref.axis );
			// Rotation on non-horizontal surface is odd, this allows 'final' rotation to match
			// intermediate rotations while coming into being.
		PerpendicularVector( ref.axis[0], ref.axis[2] );
		CrossProduct( ref.axis[2], ref.axis[0], ref.axis[1] );
	}

	ref.hModel = cgs.media.sfkMineModel;
	ref.reType = RT_MODEL;
	VectorCopy( cent->currentState.origin, ref.origin );
	trap_R_AddRefEntityToScene( &ref, cent );
}
