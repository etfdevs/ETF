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
**	g_q3f_sfk.c
**
**	Skills For Kills functions
**
*/

#include "g_local.h"
#include "bg_q3f_playerclass.h"
#include "bg_q3f_util.h"
#include "bg_q3f_weapon.h"
#include "g_q3f_team.h"
#include "g_q3f_weapon.h"

	// Definitions used in regen
#define SFK_REGEN_SHELLS		1.0
#define SFK_REGEN_NAILS			1.5
#define SFK_REGEN_ROCKETS		0.5
#define SFK_REGEN_CELLS			1.5
#define SFK_REGEN_GREN1			0.2
#define SFK_REGEN_GREN2			0.1
#define SFK_REGEN_HEALTH		1.0
#define SFK_REGEN_MEDIKIT		0.5
#define SFK_REGEN_CHARGE		0.1

#define SFK_MORTARLOADTIME		1000		// 1 every 4 seconds, no matter what skill

#define	SFK_JETPACK_KICKIN		350			// Time in msec before jetpack kicks in.
#define	SFK_JETPACK_IGNITION	20			// This many cells are consumed in ignition.

#define	SFK_AUTOINFECT_RADIUS	64			// This many cells between players for medic infection.

	// Handy variables used by SFK
static float sfkSkillFactor, sfkRegenFactor;

void G_SFK_Initialize()
{
	// Ensure settings are valid, and set up some handy variables.

	sfkSkillFactor = 1.0f / (float) (SFK_SKILLLEVELMAX - SFK_SKILLLEVEL1);
	sfkRegenFactor = 1.0f / (float) (SFK_SKILLLEVELMAX - SFK_SKILLLEVEL2);
}

void G_SFK_BumpSkill( gentity_t *player, int amount, qboolean forceSkill )
{
	// Add 'amount' to the player skill, capping at level3.
	// If the player has no existing skills, they will be bumped
	// immediately to level 1 (unless they're being penalized, of course).
	// amount 0 should be used to alter skill levels temporarily (e.g.
	// when granted/removed by carrying a flag)

	gclient_t *client;
	qboolean hasSkill;

	if( !player || !player->client || !amount )
		return;

	client = player->client;

	client->ps.stats[STAT_Q3F_SKILL] += amount;
	if( amount > 0 )
	{
		// Increment skill

		if( client->ps.stats[STAT_Q3F_SKILL] > SFK_SKILLLEVELMAX )
			client->ps.stats[STAT_Q3F_SKILL] = SFK_SKILLLEVELMAX;		// Cap maximum
		if( forceSkill && client->ps.stats[STAT_Q3F_SKILL] < SFK_SKILLLEVEL1 )
			client->ps.stats[STAT_Q3F_SKILL] = SFK_SKILLLEVEL1;			// Raise minumum
	}
	else {
		if( client->ps.stats[STAT_Q3F_SKILL] < 0 )
			client->ps.stats[STAT_Q3F_SKILL] = 0;								// Cap minimum
	}

	hasSkill = client->sfkSkill;

		// Calculate skill and regen ratios - if any skill/regen is available, it is
		// > 0 (hence the + 1 stuff). Think of it as being fully inclusive between low and high levels.
	client->sfkSkill =	client->ps.stats[STAT_Q3F_SKILL] < SFK_SKILLLEVEL1 ? 0 :
						sfkSkillFactor * (float) (client->ps.stats[STAT_Q3F_SKILL] - SFK_SKILLLEVEL1 + 1);
	client->sfkRegen =	client->ps.stats[STAT_Q3F_SKILL] < SFK_SKILLLEVEL2 ? 0 : 
						sfkRegenFactor * (float) (client->ps.stats[STAT_Q3F_SKILL] - SFK_SKILLLEVEL2 + 1);

	if( client->sfkSkill && !hasSkill )
		G_SFK_ActivateSkill( player );
	else if( !client->sfkSkill && hasSkill )
		G_SFK_DeactivateSkill( player );
}

void G_SFK_ActivateSkill( gentity_t *player )
{
	// Activate the player's skill - this should only be called
	// when the skill is first activated.

	player->client->ps.extFlags	|= EXTF_SKILLED;
	BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );
}

void G_SFK_DeactivateSkill( gentity_t *player )
{
	// Deactivate the player's skill - this should only be called
	// when the skill is removed (e.g. on death, or when ).

	bg_q3f_weapon_t *wp;

	player->client->ps.extFlags	&= ~EXTF_SKILLED;
	BG_PlayerStateToEntityState( &player->client->ps, &player->s, qtrue );

	switch( player->client->ps.persistant[PERS_CURRCLASS] )
	{
		case Q3F_CLASS_SOLDIER:	// Cut their clip back down to size if necessary.
								wp = BG_Q3F_GetWeapon( WP_ROCKET_LAUNCHER );
								if( Q3F_GetClipValue( WP_ROCKET_LAUNCHER, &player->client->ps ) > wp->clipsize )
									Q3F_SetClipValue( WP_ROCKET_LAUNCHER, wp->clipsize, &player->client->ps );
								break;
	}
}

void G_SFK_BumpSkillCommand_f( gentity_t *player )
{
	// Process a client-generated "bumpskills" command.

	if( g_cheats.integer == 0 )
		trap_SendServerCommand( player->s.number, "print \"Cheats must be enabled to allow this command.\n\"" );
	else if( player->client->ps.stats[STAT_Q3F_SKILL] >= SFK_SKILLLEVELMAX )
		trap_SendServerCommand( player->s.number, "print \"You already have maximum skill.\n\"" );
	else {
		G_SFK_BumpSkill( player, SFK_SKILLLEVEL1, qfalse );
		trap_SendServerCommand( player->s.number, "print \"Skill level bumped.\n\"" );
	}
}

static void G_SFK_BumpValue( float *accumulator, int *data, int maximum )
{
	// Update the actual value from the float accumulator.

	int val;

	if( *data > maximum )
	{
		*accumulator = 0;
		return;
	}
	if( *accumulator < 1.0f )
		return;
	val = (int) *accumulator;
	*accumulator -= val;
	if( (*data += val) > maximum )
	{
		*data = maximum;
		*accumulator = 0;
	}
}
void G_SFK_Regenerate( gentity_t *player )
{
	// Regenerate ammunition and health.

	int tmp, skill, max;
	playerState_t *ps;
	bg_q3f_playerclass_t *cls;
	float rate;
	gclient_t *client;

	client = player->client;
	ps = &client->ps; 
	skill = ps->stats[STAT_Q3F_SKILL];

		// Start with the mortar reload counter.
	if( skill > SFK_SKILLLEVEL1 &&
		ps->persistant[PERS_CURRCLASS] == Q3F_CLASS_MINIGUNNER )
	{
		if( !client->mortarLoadTime )
		{
			client->mortarLoadTime = level.time + SFK_MORTARLOADTIME;
			Q3F_SetClipValue( WP_MORTAR, 0, ps );
		}
		else if( client->mortarLoadTime <= level.time )
		{
			client->mortarLoadTime += SFK_MORTARLOADTIME;
			tmp = Q3F_GetClipValue( WP_MORTAR, ps ) + 1;
			max = 3 + 7 * sfkSkillFactor * (float) (skill - SFK_SKILLLEVEL1);
			if( tmp > max )
				tmp = max;
			if( tmp > ps->ammo[AMMO_ROCKETS] )
				tmp = ps->ammo[AMMO_ROCKETS];
			Q3F_SetClipValue( WP_MORTAR, tmp, ps );
		}
	}

		// Do we have any other 'regen' going on yet?
	if( skill < SFK_SKILLLEVEL2 )
		return;

	cls = BG_Q3F_GetClass( ps );
	rate = player->client->sfkRegen * 0.001 * (float) (level.time - client->sfkRegenPreviousTime);	// Measure in 10ths of a second here rather than seconds.
	client->sfkRegenPreviousTime = level.time;

		// Ensure there's no sudden 'jump' if the player has just started getting regen.
	if( rate > 1 )
		return;

		// Store the dribbles of regen in the accumulators
	client->sfkShellAcc		+= rate * SFK_REGEN_SHELLS;
	client->sfkNailAcc		+= rate * SFK_REGEN_NAILS;
	client->sfkRocketAcc	+= rate * SFK_REGEN_ROCKETS;
	client->sfkCellAcc		+= rate * SFK_REGEN_CELLS;
	client->sfkGren1Acc		+= rate * SFK_REGEN_GREN1;
	client->sfkGren2Acc		+= rate * SFK_REGEN_GREN2;
	client->sfkMedikitAcc	+= rate * SFK_REGEN_MEDIKIT;
	client->sfkChargeAcc	+= rate * SFK_REGEN_CHARGE;
	client->sfkHealthAcc	+= rate * SFK_REGEN_HEALTH;

	G_SFK_BumpValue( &client->sfkShellAcc,		&ps->ammo[AMMO_SHELLS],		cls->maxammo_shells		);
	G_SFK_BumpValue( &client->sfkNailAcc,		&ps->ammo[AMMO_NAILS],		cls->maxammo_nails		);
	G_SFK_BumpValue( &client->sfkRocketAcc,		&ps->ammo[AMMO_ROCKETS],	cls->maxammo_rockets	);
	G_SFK_BumpValue( &client->sfkCellAcc,		&ps->ammo[AMMO_CELLS],		cls->maxammo_cells		);
	G_SFK_BumpValue( &client->sfkMedikitAcc,	&ps->ammo[AMMO_MEDIKIT],	cls->maxammo_medikit	);
	G_SFK_BumpValue( &client->sfkChargeAcc,		&ps->ammo[AMMO_CHARGE],		cls->maxammo_charge		);

	tmp = ps->ammo[AMMO_GRENADES] & 0xFF;
	G_SFK_BumpValue( &client->sfkGren1Acc,		&tmp,						cls->gren1max			);
	ps->ammo[AMMO_GRENADES] = (ps->ammo[AMMO_GRENADES] & 0xFF00) + (tmp & 0xFF);
	tmp = (ps->ammo[AMMO_GRENADES] & 0xFF00) >> 8;
	G_SFK_BumpValue( &client->sfkGren2Acc,		&tmp,						cls->gren2max			);
	ps->ammo[AMMO_GRENADES] = (ps->ammo[AMMO_GRENADES] & 0xFF) + ((tmp & 0xFF) << 8);

#if		SFK_HEALTHREGEN
	G_SFK_BumpValue( &client->sfkHealthAcc,	&player->health,			cls->maxhealth			);
#endif//SFK_HEALTHREGEN
}

/*void G_SFK_BumpTeamSkill( int teamNum )
{
	// Give skills to the entire team (if enabled)

	int index;
	gentity_t *ent;

	if( g_sfk_captureBonus.integer )
	{
		for( index = 0; index < MAX_CLIENTS; index++ )
		{
			ent = &g_entities[index];
			if( ent->inuse && ent->client && ent->health > 0 && ent->client->sess.sessionTeam == teamNum )
				G_SFK_BumpSkill( ent, g_sfk_bonusSize.integer, qtrue );
		}
	}
}*/

void G_SFK_Jetpack( gentity_t *ent, gclient_t *client, pmove_t *pm )
{
	// Process jetpack handling. Assumes death/unskilled status is
	// already checked.

	int msec;

	if( client->ps.extFlags & EXTF_JETPACK )
	{
		if( pm->cmd.upmove > 10 && ent->health > 0 )
		{
			// Jetpack in progress

			msec = ((level.time - client->sfkJetpackTime) * SFK_JETSTRENGTH) / 1000;
			if( msec > 0 )
			{
				if( (client->ps.ammo[AMMO_CELLS] -= msec) < 0 )
				{
					// Ran out of cells.
					client->ps.ammo[AMMO_CELLS] = 0;

					client->ps.extFlags		&= ~EXTF_JETPACK;
					ent->s.extFlags			&= ~EXTF_JETPACK;
					client->sfkJetpackTime	= 0;
				}
				else {
					// Use the time to keep accurate cell debits, rather than just setting
					// to the current time and waiting for the next opportunity to remove
					// an integer cell count.
					client->sfkJetpackTime += (msec * 1000) / SFK_JETSTRENGTH;
				}
			}
		}
		else {
			// Stopped jetpack

			client->ps.extFlags		&= ~EXTF_JETPACK;
			ent->s.extFlags			&= ~EXTF_JETPACK;
			client->sfkJetpackTime	= 0;
		}
	}
	else {
		if( pm->cmd.upmove > 10 && client->ps.ammo[AMMO_CELLS] >= SFK_JETPACK_IGNITION )
		{
			if( !client->sfkJetpackTime )
				client->sfkJetpackTime = level.time;	// Prime for jetpack in KICKIN msec
			else if( (level.time - client->sfkJetpackTime) >= SFK_JETPACK_KICKIN )
			{
				// Jetpack kicking in.

				client->ps.ammo[AMMO_CELLS] -= SFK_JETPACK_IGNITION;
				client->sfkJetpackTime = level.time;
				client->ps.extFlags		|= EXTF_JETPACK;
				ent->s.extFlags			|= EXTF_JETPACK;
			}
		}
		else {
			// No jetpack

			client->sfkJetpackTime = 0;
		}
	}
}

void G_SFK_AutoInfect( gentity_t *player )
{
	// Have the skilled medic infect any enemies within a set radius around him.

	int i, numEnts;
	int others[32];
	gentity_t *other;
	vec3_t mins, maxs;
	trace_t trace;

	if( !player->client ||
		player->health <= 0 ||
		!(player->s.extFlags & EXTF_SKILLED) ||
		player->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_PARAMEDIC )
		return;

	VectorSet(		maxs, SFK_AUTOINFECT_RADIUS, SFK_AUTOINFECT_RADIUS, SFK_AUTOINFECT_RADIUS );
	VectorAdd(		player->s.origin, maxs, mins );
	VectorSubtract(	player->s.origin, mins, mins );
	numEnts = trap_EntitiesInBox( mins, maxs, others, 32 );

	for( i = 0; i < numEnts; i++ )
	{
		other = &g_entities[others[i]];
		if( !other->client ||
			other->health < 0 ||
			other->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_PARAMEDIC ||
			other->client->diseaseTime ||
			Distance( other->s.origin, player->s.origin ) > SFK_AUTOINFECT_RADIUS ||
			G_Q3F_IsAllied( player, other ) )
			continue;
		trap_Trace( &trace, player->s.origin, NULL, NULL, other->s.origin, player->s.number, MASK_SHOT );
		if( trace.fraction >= 1 )
			G_Q3F_Disease_Person( other, player, qfalse );
	}
}

void G_SFK_MineTouch( gentity_t *mine, gentity_t *other, trace_t *tr )
{
	// Mine is touched by someone, decide whether or not to go off.

#if		SFK_PROXMINE
	int distance;
#endif//SFK_PROXMINE

	if( level.time < mine->s.time ||
		(level.time < mine->s.time + 5000 && mine->parent == other) ||
		!other->client )
//		(g_sfk_safeTeamMine.integer && G_Q3F_IsAllied( mine->parent, other )) )
		return;

#if		SFK_SAFETEAMMINE
	if( G_Q3F_IsAllied( mine->parent, other ) )
		return;
#endif//SFK_SAFETEAMMINE

#if		SFK_PROXMINE
		// We try to be clever about this.

		distance = Distance( mine->r.currentOrigin, other->r.currentOrigin );
			// Don't switch if we're already tracking an enemy.
		if( mine->enemy && mine->enemy != other && distance > mine->speed )
			return;
		if( mine->enemy == other && distance > mine->speed )
		{
			// They're moving away from the center - fire.
			G_SFK_MineThink( mine );
			return;
		}

		mine->enemy = other;
		mine->speed = distance;
		return;
#else
	G_SFK_MineThink( mine );
#endif//SFK_PROXMINE
}

void G_SFK_MineThink( gentity_t *mine )
{
	// Mine explodes, bang.

	gclient_t *player;

	if( !mine->think )
		return;

	if( player = mine->parent->client )
	{
		player->pers.mineCount--;
		if( player->mineEntity == mine )
			player->mineEntity = NULL;
		Q3F_SetClipValue( WP_MINELAYER, SFK_MINECOUNT - player->pers.mineCount, &player->ps );
	}

	mine->takedamage = qfalse;
	G_RadiusDamage( mine->r.currentOrigin, mine, mine->parent, SFK_MINESIZE * 40, 0, NULL, MOD_MINE, DAMAGE_HIGH_KNOCKBACK|DAMAGE_HALF_RADIUS, player ? player->ps.powerups[PW_QUAD] : qfalse );
	mine->s.weapon = WP_GRENADE_LAUNCHER;	// Used to get the correct explosion effect.
	G_AddEvent( mine, EV_MISSILE_MISS, DirToByte( mine->pos1 ) );
	mine->freeAfterEvent = qtrue;
	mine->s.time = 0;
	mine->think = NULL;
	mine->touch = NULL;
	mine->die	= NULL;
}

void G_SFK_MineDie( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker, int damage, int mod )
{
	// Mine has been damaged (see MineThink :)

	G_SFK_MineThink( targ );
}

void G_SFK_DestroyMines( gentity_t *player )
{
	gentity_t *ent;

	for( ent = g_entities; ent < &g_entities[ENTITYNUM_WORLD]; ent++ )
	{
		if( ent->s.eType == ET_SFK_MINE && ent->parent == player && ent->think )
			G_SFK_MineThink( ent );
	}
}
