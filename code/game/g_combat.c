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

// g_combat.c

#include "g_local.h"
#include "bg_local.h"
#include "g_q3f_mapents.h"
#include "g_q3f_flag.h"
#include "g_q3f_playerclass.h"
#include "g_q3f_weapon.h"
#include "g_q3f_team.h"
#include "g_q3f_admin.h"

#include "g_bot_interface.h"
#ifdef BUILD_LUA
#include "g_lua.h"
#endif
/*
============
ScorePlum
============
*/
void ScorePlum( gentity_t *ent, vec3_t origin, int score ) {
	gentity_t *plum;

	plum = G_TempEntity( origin, EV_SCOREPLUM );
	// only send this temp entity to a single client
	plum->r.svFlags |= SVF_SINGLECLIENT;
	plum->r.singleClient = ent->s.number;
	//
	plum->s.otherEntityNum = ent->s.number;
	plum->s.time = score;
}

/*
============
AddScore

Adds score to both the client and his team
============
*/
void AddScore( gentity_t *ent, vec3_t origin, int score ) {
	if ( !ent->client ) {
		return;
	}
	// no scoring during pre-match warmup
	if ( g_matchState.integer > MATCH_STATE_PLAYING )
		return;

	if (level.intermissionQueued || level.intermissiontime)
		return;

	// show score plum
	ScorePlum(ent, origin, score);

	ent->client->ps.persistant[PERS_SCORE] += score;
	level.teamFrags[ ent->client->ps.persistant[PERS_TEAM] ] += score;
	CalculateRanks();
}

/*
=================
TossClientItems

Toss the weapon and powerups for the killed player
=================
*/
void TossClientItems( gentity_t *self ) {
	gitem_t		*item;
	gentity_t	*drop;

	if ( self->client->ps.ammo[AMMO_SHELLS] ||
		 self->client->ps.ammo[AMMO_NAILS] ||
		 self->client->ps.ammo[AMMO_ROCKETS] ||
		 self->client->ps.ammo[AMMO_CELLS])
	{
		item = BG_FindItem( "Backpack" );
		drop = Drop_Item( self, item, 0 );
		drop->activator		= self;
		drop->s.time2		= self->client->ps.ammo[AMMO_SHELLS];
		drop->s.legsAnim	= self->client->ps.ammo[AMMO_NAILS];
		drop->s.torsoAnim	= self->client->ps.ammo[AMMO_ROCKETS];
		drop->s.weapon		= self->client->ps.ammo[AMMO_CELLS];
		drop->flags			|= FL_DROPPED_ITEM;
	}
}


/*
==================
LookAtKiller
==================
*/
void LookAtKiller( gentity_t *self, gentity_t *inflictor, gentity_t *attacker ) {
	vec3_t		dir;

	if ( attacker && attacker != self ) {
		VectorSubtract (attacker->s.pos.trBase, self->s.pos.trBase, dir);
	} else if ( inflictor && inflictor != self ) {
		VectorSubtract (inflictor->s.pos.trBase, self->s.pos.trBase, dir);
	} else {
		self->client->ps.stats[STAT_DEAD_YAW] = self->s.angles[YAW];
		return;
	}

	self->client->ps.stats[STAT_DEAD_YAW] = vectoyaw ( dir );
}

/*
==================
GibEntity
==================
*/
void GibEntity( gentity_t *self, int killer ) {
	G_AddEvent( self, EV_GIB_PLAYER, killer );
	self->takedamage = qfalse;
	self->s.eType = ET_INVISIBLE;
	self->r.contents = 0;
}

/*
==================
body_die
==================
*/
void body_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	if ( self->health > GIB_HEALTH ) {
		return;
	}
	if ( !g_blood.integer ) {
		self->health = GIB_HEALTH+1;
		return;
	}

	GibEntity( self, 0 );
}

void G_Q3F_RegisterTeamKill( gentity_t *attacker, gentity_t *obituary ) {
	int			i;

	i = attacker->client->sess.allyKill - attacker->client->sess.enemyKill;
	i = (i > 0) ? i * Q3F_ADMIN_TEAMKILL_BIAS : 0;
	attacker->client->sess.teamKillHeat -= (level.time - attacker->client->sess.lastTeamKillTime) / 1000;
	if( attacker->client->sess.teamKillHeat < i )
		attacker->client->sess.teamKillHeat = i;
	attacker->client->sess.teamKillHeat += Q3F_ADMIN_TEAMKILL_HEAT;
	attacker->client->sess.lastTeamKillTime = level.time;
	if( ( g_teamKillRules.integer == 1 && attacker->client->sess.teamKillHeat >= Q3F_ADMIN_TEAMKILL_KICK_1 ) ||
		( g_teamKillRules.integer == 2 && attacker->client->sess.teamKillHeat >= Q3F_ADMIN_TEAMKILL_KICK_2 ) )
	{
		// Good riddance
		if ( g_banRules.value > 0 && !attacker->client->pers.localClient ) {
			G_Damage( attacker, NULL, attacker, NULL, NULL, 10000, DAMAGE_NO_PROTECTION|DAMAGE_NO_GIB, MOD_DISCONNECT );
		
			G_Q3F_AdminTempBan( attacker, "Team killing.", Q3F_ADMIN_TEMPBAN_TIME );

			if( obituary ) {
				obituary->s.otherEntityNum = attacker->s.number;
				obituary->s.eventParm = MOD_UNKNOWN;			// Stop a bad obituary appearing.
			}
		}
	}
	if( attacker->client->sess.teamKillHeat >= Q3F_ADMIN_TEAMKILL_WARN )
	{
		if( !attacker->client->sess.teamKillWarn )
		{
			trap_SendServerCommand( attacker->s.number, "tkwarn" );
			attacker->client->sess.teamKillWarn = qtrue;
		}
	}
	else {
		attacker->client->sess.teamKillWarn = qfalse;
	}
}

/*
==================
player_die
==================
*/
void player_die( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath ) {
	gentity_t	*ent;
	int			anim;
	int			contents;
	int			killer;
	int			i;
	char		*killerName, *obit;
	g_q3f_playerclass_t *cls;
	//qboolean	tkban;

	if ( self->client->ps.pm_type == PM_DEAD ) {
		return;
	}

	if ( level.intermissiontime ) {
		return;
	}

	//unlagged - make sure the body shows up in the client's current position
	G_UnTimeShiftClient( self );

	// JT: Do deathcleanup functions, if appropriate.
	self->client->ps.pm_type = PM_DEAD;

	self->client->deathLoc = Team_GetLocation( self );	// Golliwog: store death location for later reference.
	G_Q3F_Global_Death_Cleanup(self);			// JT: Remove diseases, flames, etc
	self->nextbeamhittime = 0; // RR2DO2: reset beamhittime

	cls = G_Q3F_GetClass(&(self->client->ps));
	if(cls->DeathCleanup)
		cls->DeathCleanup(self);

	if ( attacker ) {
		killer = attacker->s.number;
		if ( attacker->client ) {
			killerName = attacker->client->pers.netname;
		} else {
			killerName = "<non-client>";
		}
	} else {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

#ifdef DREVIL_BOT_SUPPORT
	Bot_Interface_SendEvent(MESSAGE_DEATH, self->s.clientNum, 0,0,0);
#endif
	if ( killer < 0 || killer >= MAX_CLIENTS ) {
		killer = ENTITYNUM_WORLD;
		killerName = "<world>";
	}

	// Golliwog: Allow custom death messages on ents
	if( !level.ceaseFire && !(attacker->client && attacker->client->ps.powerups[PW_Q3F_CEASEFIRE]) &&
		inflictor && inflictor->mapdata && (inflictor->mapdata->flags & Q3F_FLAG_KILLMSG) )
	{
		meansOfDeath = MOD_CUSTOM;
		G_Q3F_KillMessage( self, inflictor, attacker );
	}

	if ( meansOfDeath < 0 || meansOfDeath >= MOD_LASTONE || !modNames[ meansOfDeath ]) {
		obit = "<bad obituary>";
	} else {
		obit = modNames[ meansOfDeath ];
	}

#ifdef BUILD_BOTS
	Bot_Event_Death(self, attacker, obit);
	Bot_Event_KilledSomeone(attacker, self, obit);
#endif

	if( !level.ceaseFire && !(attacker->client && attacker->client->ps.powerups[PW_Q3F_CEASEFIRE]) )
	{
		G_LogPrintf("Kill: %i %i %i: %s killed %s by %s\n", 
			killer, self->s.number, meansOfDeath, killerName, 
			self->client->pers.netname, obit );

		// broadcast the death event to everyone
		ent = G_TempEntity( self->r.currentOrigin, (G_Q3F_IsAllied( attacker, self ) ? EV_ALLYOBITUARY : EV_OBITUARY) );
		ent->s.eventParm = meansOfDeath;
		ent->s.otherEntityNum = self->s.number;
		ent->s.otherEntityNum2 = killer;
		ent->r.svFlags = SVF_BROADCAST;	// send to everyone

		self->enemy = attacker;

		self->client->ps.persistant[PERS_KILLED]++;

		//tkban = qfalse;
		if (attacker && attacker->client) {
			attacker->client->lastkilled_client = self->s.number;

			if ( attacker == self || G_Q3F_IsAllied( attacker, self ) )
			{
				AddScore( attacker, self->r.currentOrigin, -1 );

				// Golliwog: Cheak for teamkills and ban accordingly.
				if( attacker != self &&
					meansOfDeath != MOD_CRUSHEDBYSENTRY && meansOfDeath != MOD_CRUSHEDBYSUPPLYSTATION &&
					meansOfDeath != MOD_FAILED_OPERATION ) // RR2DO2: these aren't teamkills on purpose
				{
					attacker->client->sess.allyKill++;
					attacker->client->pers.stats.teamkills++;
					if ((attacker->client->sess.lastTeamKillTime + Q3F_ADMIN_TEAMKILL_GRACE) < level.time &&	g_teamKillRules.integer > 0) 
						G_Q3F_RegisterTeamKill( attacker, ent );
				}
				// Golliwog.
			}
			else {
				G_DeathStats( self->client, attacker->client, meansOfDeath );
				attacker->client->sess.enemyKill++;
				AddScore( attacker, self->r.currentOrigin, 1 );

				if( meansOfDeath == MOD_AXE ) {
					// play humiliation on player
					attacker->client->ps.persistant[PERS_GAUNTLET_FRAG_COUNT]++;


					// add the sprite over the player's head
					attacker->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT /*| EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP*/ );
					/* attacker->client->ps.eFlags |= EF_AWARD_GAUNTLET; 
					attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME; */

					// also play humiliation on target
					self->client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_GAUNTLETREWARD;
				}

				// check for two kills in a short amount of time
				// if this is close enough to the last kill, give a reward sound
				if ( level.time - attacker->client->lastKillTime < CARNAGE_REWARD_TIME ) {
					// play excellent on player
					attacker->client->ps.persistant[PERS_EXCELLENT_COUNT]++;

					// add the sprite over the player's head
					attacker->client->ps.eFlags &= ~(EF_AWARD_IMPRESSIVE | EF_AWARD_EXCELLENT /*| EF_AWARD_GAUNTLET | EF_AWARD_ASSIST | EF_AWARD_DEFEND | EF_AWARD_CAP*/ );
					attacker->client->ps.eFlags |= EF_AWARD_EXCELLENT;
					attacker->client->rewardTime = level.time + REWARD_SPRITE_TIME;
				}
				attacker->client->lastKillTime = level.time;

			}
		} else {
			AddScore( self, self->r.currentOrigin, -1 );
		}

		// Add team bonuses
		//Team_FragBonuses(self, inflictor, attacker);
	}

#ifdef BUILD_LUA
	if (meansOfDeath != MOD_DISCONNECT)
	{
		// Ensiform: FIXME implement MOD_CUSTOM parsing
		if (G_LuaHook_Obituary(self->s.number, killer, meansOfDeath))
		{
			if (self->s.number < 0 || self->s.number >= MAX_CLIENTS)
			{
				G_Error("G_LuaHook_Obituary: target out of range");
			}
		}
	}
#endif

	// Ensiform: Disconnects and Team Switch shouldn't cause onkills
	if( meansOfDeath != MOD_SWITCHTEAM && meansOfDeath != MOD_DISCONNECT )
		G_Q3F_CheckOnKill( attacker, self );	// Golliwog: Run through any special death processing
	if( self->health > 0 )
		self->health = 0;					// In case some bright spark tried to give the victim health

	G_Q3F_DropAllFlags( self, qtrue, qfalse );	// Golliwog: Drop any flags carried (including those just given)

	// if client is in a nodrop area, don't drop anything (but return CTF flags!)
	contents = trap_PointContents( self->r.currentOrigin, -1 );
	if ( !( contents & CONTENTS_NODROP ) && !level.intermissiontime ) {
		TossClientItems( self );
	}

	Cmd_Score_f( self );		// show scores
	// send updated scores to any clients that are following this one,
	// or they would get stale scoreboards
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		gclient_t	*client;

		client = &level.clients[i];
		if ( client->pers.connected != CON_CONNECTED ) {
			continue;
		}
		if ( client->sess.sessionTeam != Q3F_TEAM_SPECTATOR ) {
			continue;
		}
		if ( client->sess.spectatorClient == self->s.number ) {
			Cmd_Score_f( g_entities + i );
		}
	}

	self->takedamage = qtrue;	// can still be gibbed

	self->s.weapon = WP_NONE;
	self->s.powerups = 0;
	self->r.contents = CONTENTS_CORPSE;

	self->s.angles[0] = 0;
	self->s.angles[2] = 0;
	LookAtKiller (self, inflictor, attacker);

	VectorCopy( self->s.angles, self->client->ps.viewangles );

	self->s.loopSound = 0;

	self->r.maxs[2] = -8;

	// don't allow respawn until the death anim is done
	// g_forcerespawn may force spawning at some later time
	self->client->respawnTime = level.time + 1700;

	// remove powerups
	memset( self->client->ps.powerups, 0, sizeof(self->client->ps.powerups) );

	// Golliwog: Update lives counter
	if( self->client->sess.lives > 0 )
		self->client->sess.lives--;
	// Golliwog.

	// never gib in a nodrop
	if ( (self->health <= GIB_HEALTH && !(contents & CONTENTS_NODROP) && g_blood.integer) || meansOfDeath == MOD_SUICIDE) {
		// gib death
		GibEntity( self, killer );
	} else {
		// normal death
		const int deathAnim = rand() % 2;
		const int deathSound = rand() % 3;

		switch ( deathAnim ) {
		case 0:
			anim = ANI_DEATH_1;
			break;
		case 1:
		default:
			anim = ANI_DEATH_2;
			break;
		}

		// for the no-blood option, we need to prevent the health
		// from going to gib level
		if ( self->health <= GIB_HEALTH ) {
			self->health = GIB_HEALTH+1;
		}

		self->client->ps.legsAnim = 
			( ( self->client->ps.legsAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;
		self->client->ps.torsoAnim = 
			( ( self->client->ps.torsoAnim & ANIM_TOGGLEBIT ) ^ ANIM_TOGGLEBIT ) | anim;

		if( meansOfDeath == MOD_WATER ) {
			G_AddEvent( self, EV_DROWN, killer );
		} else if( meansOfDeath == MOD_NAPALMGREN ||
				   meansOfDeath == MOD_FLAME ||
				   meansOfDeath == MOD_FLAME_SPLASH ) {
			G_AddEvent( self, EV_BURNTODEATH, killer );
		} else {
			G_AddEvent( self, EV_DEATH1 + deathSound, killer );
		}

		// the body can still be gibbed
		self->die = body_die;
	}

	trap_LinkEntity (self);
}


/*
================
CheckArmor
================
*/
float CheckArmor (gentity_t *ent, float damage, int dflags, float adamagescale )
{
	gclient_t	*client;
	float		save, count, temp;

	if (!damage)
		return 0;

	client = ent->client;

	if (!client)
		return 0;

	if (dflags & DAMAGE_NO_ARMOR)
		return 0;

	count = client->ps.stats[STAT_ARMOR];
	if( count <= 0 )
	{
		count = 0;
		client->ps.stats[STAT_ARMOR] = 0;
		client->ps.stats[STAT_Q3F_ARMOURCLASS] &= ~DAMAGE_Q3F_MASK;	// Remove armour types
	}

	// armor
	temp = ((float)client->ps.stats[STAT_ARMORTYPE] / (float)100);
	save = /*floor(*/ damage * temp;// );		// Golliwog: Not ceil, or you can end up with no damage taken
	if (save >= count)
		save = count;

	client->ps.stats[STAT_ARMOR] -= ceil( save * adamagescale );

//	if( client->ps.stats[STAT_Q3F_ARMOURCLASS] & DAMAGE_Q3F_MASK & dflags )
//		save *= 1.15;		// Increase effect by 15% (it's an integer, so a little inaccurate)

	if( save > damage )
		save = damage;

	return save;
}

/*
================
RaySphereIntersections
================
*/
int RaySphereIntersections( vec3_t origin, float radius, vec3_t point, vec3_t dir, vec3_t intersections[2] ) {
	float b, c, d, t;

	//	| origin - (point + t * dir) | = radius
	//	a = dir[0]^2 + dir[1]^2 + dir[2]^2;
	//	b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
	//	c = (point[0] - origin[0])^2 + (point[1] - origin[1])^2 + (point[2] - origin[2])^2 - radius^2;

	// normalize dir so a = 1
	VectorNormalize(dir);
	b = 2 * (dir[0] * (point[0] - origin[0]) + dir[1] * (point[1] - origin[1]) + dir[2] * (point[2] - origin[2]));
	c = (point[0] - origin[0]) * (point[0] - origin[0]) +
		(point[1] - origin[1]) * (point[1] - origin[1]) +
		(point[2] - origin[2]) * (point[2] - origin[2]) -
		radius * radius;

	d = b * b - 4 * c;
	if (d > 0) {
		t = (- b + sqrt(d)) / 2;
		VectorMA(point, t, dir, intersections[0]);
		t = (- b - sqrt(d)) / 2;
		VectorMA(point, t, dir, intersections[1]);
		return 2;
	}
	else if (d == 0) {
		t = (- b ) / 2;
		VectorMA(point, t, dir, intersections[0]);
		return 1;
	}
	return 0;
}

/*
============
T_Damage

targ		entity that is being damaged
inflictor	entity that is causing the damage
attacker	entity that caused the inflictor to damage targ
	example: targ=monster, inflictor=rocket, attacker=player

dir			direction of the attack for knockback
point		point at which the damage is being inflicted, used for headshots
damage		amount of damage being inflicted
knockback	force to be applied against targ as a result of the damage

inflictor, attacker, dir, and point can be NULL for environmental effects

dflags		these flags are used to control how T_Damage works
	DAMAGE_RADIUS			damage was indirect (from a nearby explosion)
	DAMAGE_NO_ARMOR			armor does not protect from this damage
	DAMAGE_NO_KNOCKBACK		do not affect velocity, just view angles
	DAMAGE_NO_PROTECTION	kills godmode, armor, everything
============
*/

void G_Damage( gentity_t *targ, gentity_t *inflictor, gentity_t *attacker,
			   vec3_t dir, vec3_t point, int damage, int dflags, int mod ) {
	float		knockback;
	float		damagescale, adamagescale, take, /*save,*/ asave, fdamage;
	bg_q3f_playerclass_t *cls;
	qboolean	isallied;
	qboolean	pentagram = qfalse;
#ifdef DEBUGLOG
	int			origdamage, origtake, origarmour;
#endif

	if (!targ->takedamage) {
		return;
	}

	if ( targ->s.eType == ET_INVISIBLE ) {
//		Com_Printf("Invisible ent hit, not applying damage\n");
		return;
	}

	if ( inflictor ) {
		if (inflictor->s.powerups & (1 << PW_QUAD))
			damage *= g_quadfactor.integer;
	} else if ( attacker ) {
		if (attacker->s.powerups & (1 << PW_QUAD))
			damage *= g_quadfactor.integer;
	}

	// the intermission has allready been qualified for, so don't
	// allow any extra scoring
	if ( level.intermissionQueued ) {
		return;
	}

	if ( !inflictor ) {
		inflictor = &g_entities[ENTITYNUM_WORLD];
	}
	if ( !attacker ) {
		attacker = &g_entities[ENTITYNUM_WORLD];
	}

	// Golliwog: Some entities can be damaged only with the gauntlet weapon
	if( targ->mapdata && (targ->mapdata->flags & Q3F_FLAG_USEGAUNTLET) )
	{
		// RR2DO2: check if the current weapon is a gauntlet
		if ( attacker->client && ( attacker->s.weapon == WP_AXE ) ) {
			G_Q3F_UseEntity( targ, inflictor, attacker );
			return;
		} else {
			return;
		}
	}
	// Golliwog.

	// shootable doors / buttons don't actually have any health
	if ( targ->s.eType == ET_MOVER ) {
		if ( targ->use && targ->moverState == MOVER_POS1 ) {
			G_Q3F_UseEntity( targ, inflictor, attacker );	// Golliwog: Check states & criteria first
		}
		return;
	}

#ifdef DEBUGLOG
	origdamage = damage;
#endif

	fdamage = damage;
	knockback = damage;

	if( attacker->s.number < MAX_CLIENTS )
		fdamage *= 0.9f;		// Damage from players is reduced slightly (why?)

	// Arnout: armour class stuff, also influences knockback
	if( targ->client &&
		(targ->client->ps.stats[STAT_Q3F_ARMOURCLASS] & dflags & DAMAGE_Q3F_MASK) &&
		!(dflags & (DAMAGE_NO_PROTECTION|DAMAGE_NO_ARMOR)) )
	{
		// Players wearing a matching 'special armour' type.
		fdamage *= 0.5f;
		knockback *= 0.5f;
	}

	if ( targ->client ) {
		if ( targ->client->noclip ) {
			return;
		}
		isallied = attacker != targ && G_Q3F_IsAllied( attacker, targ );
	}
	else if( targ->s.eType == ET_Q3F_SENTRY || targ->s.eType == ET_Q3F_SUPPLYSTATION )
		isallied = attacker != targ && G_Q3F_IsAllied( attacker, targ->parent );
	else isallied = qfalse;

	// Golliwog: Handle mirror damage here
	if( isallied && level.friendlyFire & FF_Q3F_MIRRORMASK && !(targ->r.contents & CONTENTS_CORPSE) )
		G_Damage( attacker, NULL, attacker, dir, point, damage, dflags|DAMAGE_MIRROR, MOD_MIRROR );
	// Golliwog.

	if ( !dir ) {
		dflags |= DAMAGE_NO_KNOCKBACK;
	} else {
		VectorNormalize(dir);
	}

   	//knockback = damage;
	if ( dflags & DAMAGE_RADIUS )
		knockback *= 1.25;
	if ( knockback > 200 ) {
		knockback = 200;
	}
	if ( targ->flags & FL_NO_KNOCKBACK ) {
		knockback = 0;
	} else if ( dflags & (DAMAGE_NO_KNOCKBACK|DAMAGE_MIRROR) ) {
		knockback = 0;
	} else if(	targ->client && targ->client->ps.weapon == WP_MINIGUN && (targ->s.eFlags & EF_Q3F_AIMING) ) {
		//No knockback for client firing a minigun
		knockback = 0;
	} 
	else switch ( mod ) {
	case MOD_SINGLESHOTGUN:
	case MOD_SHOTGUN:
		knockback *= 1.6;
		break;
	case MOD_MINIGUN:
		knockback *= 1.5;
		break;
	case MOD_NAPALMGREN:
	case MOD_HANDGREN:
		knockback *= 1.05;
		break;
	case MOD_AUTOSENTRY_BULLET:
	case MOD_MAPSENTRY_BULLET:
		knockback *= 1.15;		// changed for 1.5, was 1.44;
		break;
	case MOD_ROCKET_SPLASH:
		if ( attacker == targ ) knockback *= 0.90 * 1.152;
		else knockback *= 1.152;
		break;
	case MOD_ROCKET:
	case MOD_GRENADE:
//		knockback *= g_rocketImpactKnockback.value;
		knockback *= 1.44f;
		if (targ->client) {
			/* Special check to see if your hitting the bottom of the hitbox */
			if ((point[2]-0.5) < targ->r.absmin[2]) {
				targ->client->ps.velocity[2] = 0;
			}
		}
	case MOD_SNIPER_RIFLE:
	case MOD_SNIPER_RIFLE_HEAD:
	case MOD_SNIPER_RIFLE_FEET:
		knockback *= 0.75f;
		break;
	}
	
	// figure momentum add, even if the damage won't be taken
	if ( knockback && targ->client ) {
		vec3_t	kvel,oldvel;
		float	mass;

		cls = BG_Q3F_GetClass( &targ->client->ps );
		mass = cls->mass;

		VectorScale (dir, g_knockback.value * knockback / mass, kvel);
		VectorCopy( targ->client->ps.velocity, oldvel ); 
		VectorAdd (targ->client->ps.velocity, kvel, targ->client->ps.velocity);
#if 0
		trap_SendServerCommand( -1, va
			("print \"old %.1f %.1f %.1f new  %.1f %.1f %.1f\n\"",
			oldvel[0],oldvel[1],oldvel[2],
			targ->client->ps.velocity[0],targ->client->ps.velocity[1],targ->client->ps.velocity[2]
		));
#endif		

		// set the timer so that the other client can't cancel
		// out the movement immediately
		if ( !targ->client->ps.pm_time ) {
			int		t;

			t = knockback * 2;
			if ( t < 50 ) {
				t = 50;
			}
			if ( t > 200 ) {
				t = 200;
			}
			targ->client->ps.pm_time = t;
			targ->client->ps.pm_flags |= PMF_TIME_KNOCKBACK;
		}
	}

	// check for completely getting out of the damage
	if ( !(dflags & DAMAGE_NO_PROTECTION) ) {

		// if TF_NO_FRIENDLY_FIRE is set, don't do damage to the target
		// if the attacker was on the same team
/*		if ( targ != attacker && isallied  ) {
			if ( level.friendlyFire == FF_Q3F_NONE ) {
				return;
			}
		}*/
		if( (targ->s.eType == ET_Q3F_SENTRY || targ->s.eType == ET_Q3F_SUPPLYSTATION) &&
			!level.friendlyFire &&
			(targ->parent == attacker || isallied) )
			return;

		// check for godmode
		if ( targ->flags & FL_GODMODE ) {
			return;
		}

		// Golliwog: See if they're invulnerable
		if( targ->client && targ->client->ps.powerups[PW_Q3F_INVULN] )
			return;
		// Golliwog.

		// Ensiform: see if noselfdmg is turned on and not mirror FF damage
		if ( !(dflags & DAMAGE_MIRROR) ) {
			if( targ == attacker && level.noselfdmg && mod != MOD_CRUSHEDBYSENTRY && mod != MOD_CRUSHEDBYSUPPLYSTATION )
				return;
		}

		// Golliwog: Different damage types cause different effects
		if( isallied && !(targ->r.contents & CONTENTS_CORPSE))
		{
			switch( level.friendlyFire & FF_Q3F_MASK )
			{
				case FF_Q3F_FULL:	damagescale		= 1;
									adamagescale	= 1;
									break;
				case FF_Q3F_HALF:	damagescale		= 0.5;
									adamagescale	= 0.5;
									break;
				case FF_Q3F_ARMOUR:	damagescale		= 0;
									adamagescale	= 1;
									break;
				default:			damagescale		= 0;
									adamagescale	= 0;
									break;
			}
		}
		else if( dflags & DAMAGE_MIRROR )
		{
			switch( level.friendlyFire & FF_Q3F_MIRRORMASK )
			{
				case FF_Q3F_MIRRORFULL:		damagescale		= 1;
											adamagescale	= 1;
											break;
				case FF_Q3F_MIRRORHALF:		damagescale		= 0.5;
											adamagescale	= 0.5;
											break;
				case FF_Q3F_MIRRORARMOUR:	damagescale		= 0;
											adamagescale	= 1;
											break;
				default:					//damagescale		= 0;
											//adamagescale	= 0;
											//break;
											return;
			}
		}
		else {
			damagescale = 1;
			adamagescale = 1;
		}
		// Golliwog.
	}
	else {
		damagescale = 1;
		adamagescale = 1;
	}

	// battlesuit protects from all radius damage (but takes knockback)
	// and protects 50% against all damage
	if ( targ->client && targ->client->ps.powerups[PW_BATTLESUIT] ) {
		G_AddEvent( targ, EV_POWERUP_BATTLESUIT, 0 );
		if ( (dflags & DAMAGE_RADIUS) || (mod == MOD_FALLING) ) {
			return;
		}
		fdamage *= 0.5f;
	}

	// pentagram of protection protects from all radius damage (but takes knockback)
	// and protects against all damage (but still takes armor)
	if ( targ->client && targ->client->ps.powerups[PW_PENTAGRAM] ) {
		G_AddEvent( targ, EV_POWERUP_PENTAGRAM, 0 );
		//if ( ( dflags & DAMAGE_RADIUS ) || ( mod == MOD_FALLING ) ) {
		//	return;
		//}
		pentagram = qtrue;
	}

	if(	targ->s.eType == ET_Q3F_SENTRY && (dflags & (DAMAGE_Q3F_SHELL|DAMAGE_Q3F_SHOCK|DAMAGE_Q3F_NAIL|DAMAGE_Q3F_EXPLOSION|DAMAGE_Q3F_FIRE)) ) {
		// Sentry 40% resistant to non-shock direct damage below midline, or 10% otherwise.
		// Vulnerable to shock damage, resistant to nails and fire.
		//G_Printf(va("Sentry might take %i damage\n", damage));
		if( dflags & DAMAGE_Q3F_SHOCK ) {
			fdamage *= 1.44f;
		}
		if( dflags & DAMAGE_Q3F_FIRE ) {
			fdamage *= 0.35f;
		}
		if( dflags & DAMAGE_Q3F_NAIL ) {
			fdamage *= 0.5f;
		}
		
		if( !(dflags & DAMAGE_RADIUS) && (point && (point[2] < (targ->r.currentOrigin[2] + (targ->r.mins[2] + targ->r.maxs[2]) * 0.5))) ) {
			fdamage *= 0.6f;
		} else {
			fdamage *= 0.9f;
		}
	}
	// Golliwog.

	// add to the attacker's hit counter (if the target isn't a general entity like a prox mine)
	// Golliwog: Alter so you only hear about it in same PVS...
	// Stops it from being useful for stats
	if ( attacker->client && targ != attacker && targ->health > 0
			&& targ->s.eType != ET_MISSILE
			&& !(targ->s.eType == ET_Q3F_GRENADE && targ->s.weapon == Q3F_GREN_CHARGE)
			&& targ->s.eType != ET_GENERAL
			&& trap_InPVS( attacker->r.currentOrigin, targ->r.currentOrigin ) ) {
		// RR2DO2: cvar implementation
		if( isallied || ( !g_agentHitBeep.integer && targ->client && targ->client->agentteam &&
			(g_q3f_teamlist[attacker->client->sess.sessionTeam].allyteams & (1 << targ->client->agentteam))) ||
			!Q_stricmp(targ->classname, "charge") ||
			!Q_stricmp(targ->classname, "pipe") ||
			!Q_stricmp(targ->classname, "grenade") )
		{
			attacker->client->ps.persistant[PERS_HITS] -= (int) fdamage;
		} else {
			attacker->client->ps.persistant[PERS_HITS] += (int) fdamage;
		}
	}

	// Golliwog: Two kinds of self-damage - normal and 'extra' (for hand grenades, mostly)
	if( targ == attacker && (dflags & DAMAGE_RADIUS) && !(dflags & DAMAGE_MIRROR) )
		fdamage *= (dflags & DAMAGE_EXTRA_SELF) ? 0.75 : 0.75;
	// Golliwog.

	if ( fdamage < 1 ) {
		fdamage = 1;
	}
	take = fdamage;
	//save = 0;

#ifdef DEBUGLOG
	origarmour = client ? client->ps.stats[STAT_ARMOR] : 0;
#endif

	// save some from armor
	if( !(dflags & (DAMAGE_NO_PROTECTION|DAMAGE_NO_ARMOR)) )	// Golliwog: This wasn't in?
	{
		asave = CheckArmor( targ, take, dflags, adamagescale );
		take -= asave;
	}
	else asave = 0;

	if ( pentagram && !( dflags & ( DAMAGE_NO_PROTECTION ) ) )
	{
		take = damagescale = 0;
	}

#ifdef DEBUGLOG
	origtake = take;
#endif
	take *= damagescale;		// Golliwog: Adjust for friendlyfire variables
	
	if( damagescale > 0 && take < 1 )
		take = 1;				// Round up to ensure some damage is done.

	if ( g_debugDamage.integer ) {
//		G_Printf( "client:%i health:%i damage:%i armor:%i\n", targ->s.number,
		G_Printf( "%i: client:%i health:%i damage:%f armor:%f\n", level.time, targ->s.number,
			targ->health, take, asave );
	}

#ifdef DEBUGLOG
	if( attacker->client )
	{
		if( client )
		{
			G_DebugLog(	"*** G_Damage: %s -> %s\n", attacker->client->pers.netname, client->pers.netname );
			G_DebugLog( "Damage: %d (%d), Health: %d, Armor: %d (%d%%, %d)\n",
						origdamage, dflags, targ->health, origarmour,
						client->ps.stats[STAT_ARMORTYPE], client->ps.stats[STAT_Q3F_ARMOURCLASS] );
			G_DebugLog(	"Post-modifier damage: %f, Post-armor damage: %f, Post FF damage: %f\n",
						fdamage, origtake, take );
			G_DebugLog(	"Final Health: %f, Final Armor: %d\n",
						((float) targ->health) - take, client->ps.stats[STAT_ARMOR] );
		}
		else if( targ->parent && targ->parent->client ) {
			G_DebugLog(	"*** G_Damage: %s -> %s\n", attacker->client->pers.netname, targ->parent->client->pers.netname );
			G_DebugLog( "Damage: %d (%d), Health: %d\n",
						origdamage, dflags, targ->health );
			G_DebugLog(	"Post-modifier damage: %f, Post FF damage: %f\n", damage, take );
			G_DebugLog(	"Final Health: %f\n", (float) (targ->health) - take );
		}
	}
#endif

#ifdef BUILD_LUA
	if (G_LuaHook_Damage(targ->s.number, attacker->s.number, take, dflags, mod))
	{
		return;
	}
#endif

	// add to the damage inflicted on a player this frame
	// the total will be turned into screen blends and view angle kicks
	// at the end of the frame
	if ( targ->client ) {
		if ( attacker ) {
			targ->client->ps.persistant[PERS_ATTACKER] = attacker->s.number;
		} else {
			targ->client->ps.persistant[PERS_ATTACKER] = ENTITYNUM_WORLD;
		}
		if ( !isallied && attacker->client && attacker != targ ) {
			G_DamageStats( targ->client, attacker->client, Q_ftol(take + asave), mod );
		}
		targ->client->damage_armor += (int) asave;
		targ->client->damage_blood += (int) take;
		targ->client->damage_knockback += knockback;
		if ( dir ) {
			VectorCopy ( dir, targ->client->damage_from );
			targ->client->damage_fromWorld = qfalse;
		} else {
			VectorCopy ( targ->r.currentOrigin, targ->client->damage_from );
			targ->client->damage_fromWorld = qtrue;
		}
		if( dflags & DAMAGE_Q3F_FIRE ) {
			targ->client->damage_fromFire = qtrue;
		}
	}

	if ( targ->client ) {
		// set the last client who damaged the target
		targ->client->lasthurt_client = attacker->s.number;
		targ->client->lasthurt_mod = mod;
	}

	// do the damage
	if (take) {
		targ->health = (int) targ->health - take;

		if ( targ->client ) {
			targ->client->ps.stats[STAT_HEALTH] = targ->health;
		}

		if ( targ->health <= 0 ) {
			if ( targ->client )
				targ->flags |= FL_NO_KNOCKBACK;

			if( dflags & DAMAGE_NO_GIB )		// Golliwog: Prevent gibs appearing
				targ->health = -1;
			else if( targ->health < -999 )
				targ->health = -999;

			targ->enemy = attacker;
			if(targ->die)
				targ->die (targ, inflictor, attacker, take, mod);
		} else if ( targ->pain ) {
			targ->pain (targ, attacker, take );
		}

#ifdef BUILD_BOTS
		if ( targ->s.number < level.maxclients ) {
			// notify omni-bot framework
			Bot_Event_TakeDamage(targ, attacker);
		}
#endif
	}
}


/*
============
CanDamage

Returns qtrue if the inflictor can directly damage the target.  Used for
explosions and melee attacks.
============
*/
qboolean CanDamage (gentity_t *targ, vec3_t origin, gentity_t *attacker ) {
	vec3_t	dest;
	trace_t	tr;
	vec3_t	midpoint;

	// use the midpoint of the bounds instead of the origin, because
	// bmodels may have their origin is 0,0,0
	VectorAdd (targ->r.absmin, targ->r.absmax, midpoint);
	VectorScale (midpoint, 0.5, midpoint);

	VectorCopy (midpoint, dest);
//	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	G_Q3F_ForceFieldExtTrace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, attacker->s.number, MASK_SOLID|CONTENTS_FORCEFIELD );
	if (tr.fraction == 1.0 || tr.entityNum == targ->s.number)
		return qtrue;

	// this should probably check in the plane of projection, 
	// rather than in world coordinate, and also include Z
	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] += 15.0;
//	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	G_Q3F_ForceFieldExtTrace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, attacker->s.number, MASK_SOLID|CONTENTS_FORCEFIELD );
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] += 15.0;
	dest[1] -= 15.0;
//	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	G_Q3F_ForceFieldExtTrace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, attacker->s.number, MASK_SOLID|CONTENTS_FORCEFIELD );
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] += 15.0;
//	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	G_Q3F_ForceFieldExtTrace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, attacker->s.number, MASK_SOLID|CONTENTS_FORCEFIELD );
	if (tr.fraction == 1.0)
		return qtrue;

	VectorCopy (midpoint, dest);
	dest[0] -= 15.0;
	dest[1] -= 15.0;
//	trap_Trace ( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, MASK_SOLID);
	G_Q3F_ForceFieldExtTrace( &tr, origin, vec3_origin, vec3_origin, dest, ENTITYNUM_NONE, attacker->s.number, MASK_SOLID|CONTENTS_FORCEFIELD );
	if (tr.fraction == 1.0)
		return qtrue;


	return qfalse;
}


/*
============
G_RadiusDamage
============
*/

void G_RadiusDamage ( vec3_t origin, gentity_t *inflictor, gentity_t *attacker, float damage, 
					 gentity_t *ignore, int mod, int dflags) { 

	float		points, dist, radius, radiussquared, step;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		dir;
	int			i, e;

	switch ( mod ) {
	case MOD_HANDGREN:
	case MOD_CLUSTERGREN:
        radius = damage + 10;
		step = 0.5 * (damage + 40) / radius;
		break;
	default:
		step = 0.5;
        radius = damage + 40;
	}
	radiussquared = radius * radius;

#ifdef DEBUGLOG
	G_DebugLog( "*** G_RadiusDamage: Damage: %f, Radius: %f\n", damage, radius );
#endif

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		dir[0] = -origin[0] + ent->r.currentOrigin[0] + 0.5*(ent->r.mins[0] + ent->r.maxs[0]) ;
		dir[1] = -origin[1] + ent->r.currentOrigin[1] + 0.5*(ent->r.mins[1] + ent->r.maxs[1]) ;
		dir[2] = -origin[2] + ent->r.currentOrigin[2] + 0.5*(ent->r.mins[2] + ent->r.maxs[2]) ;

		dist = VectorLengthSquared( dir );
		if ( dist >= radiussquared ) {
			continue;
		}
		dist = sqrt( dist );
		points = damage - (step * dist);						// JT
		if(points < 0 )
			continue;

		if( CanDamage (ent, origin, attacker ) ) {
			// push the center of mass higher than the origin so players
			// get knocked into the air more
#ifdef DEBUGLOG
			G_DebugLog( "Distance: %f Damage: %f\n", dist, points );
#endif
//			trap_SendServerCommand( -1,"print \"splash\n\"");
			G_Damage (ent, inflictor, attacker, dir, origin, (int)points, (dflags|DAMAGE_Q3F_EXPLOSION|DAMAGE_RADIUS), mod);

		}
	}
}

/*
============
G_NapalmRadiusDamage
============
*/
void G_NapalmRadiusDamage ( vec3_t origin, gentity_t *attacker, float damage, gentity_t *ignore, int mod) {
	float		points, dist, radius;
	gentity_t	*ent;
	int			entityList[MAX_GENTITIES];
	int			numListedEntities;
	vec3_t		mins, maxs;
	vec3_t		dir;
	int			i, e;

	radius = 120;

	for ( i = 0 ; i < 3 ; i++ ) {
		mins[i] = origin[i] - radius;
		maxs[i] = origin[i] + radius;
	}

	numListedEntities = trap_EntitiesInBox( mins, maxs, entityList, MAX_GENTITIES );

	for ( e = 0 ; e < numListedEntities ; e++ ) {
		ent = &g_entities[entityList[ e ]];

		if (ent == ignore)
			continue;
		if (!ent->takedamage)
			continue;

		VectorSubtract (ent->r.currentOrigin, origin, dir);
		dir[2] += 4;
		dist = VectorLength( dir );

		if ( dist >= radius ) {
			continue;
		}

		points = damage * ( 1.0 - dist / radius );

		if( CanDamage (ent, origin, attacker) ) {
			if ( attacker == ent ) {
				bg_q3f_playerclass_t *cls = BG_Q3F_GetClass( &ent->client->ps );
//				float knockback = (100 - dist) * g_napalmKnockBack.value * g_knockback.value / cls->mass;
				float knockback = (110 - dist) * 0.55 * g_knockback.value / cls->mass;
				if (knockback > 0 ) {
					VectorNormalize( dir );
					VectorMA( ent->client->ps.velocity, knockback, dir, ent->client->ps.velocity );
				}
			}
			G_Damage (ent, NULL, attacker, dir, origin, (int)points, DAMAGE_RADIUS|DAMAGE_Q3F_FIRE, mod);
			G_Q3F_Burn_Person(ent, attacker );
			G_Q3F_Burn_Person(ent, attacker );
		}
	}
}

