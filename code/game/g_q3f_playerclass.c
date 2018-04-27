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
**	q3f_playerclass.c
**
**	Utility functions and definition table for player classes.
**
*/

#include "g_local.h"
#include "bg_public.h"
#include "g_q3f_playerclass.h"
#include "g_q3f_weapon.h"
#include "g_weapon.h"
#include "g_q3f_scanner.h"
#include "bg_q3f_util.h"
#include "g_q3f_weapon.h"
#include "g_q3f_mapents.h"
#include "g_q3f_team.h"
#include "g_q3f_charge.h"
#include "bg_q3f_grenades.h"

#include "g_bot_interface.h"

	// We know these exist in bg_q3f_playerclass
extern bg_q3f_playerclass_t bg_q3f_playerclass_null;
extern bg_q3f_playerclass_t bg_q3f_playerclass_recon;
extern bg_q3f_playerclass_t bg_q3f_playerclass_sniper;
extern bg_q3f_playerclass_t bg_q3f_playerclass_soldier;
extern bg_q3f_playerclass_t bg_q3f_playerclass_grenadier;
extern bg_q3f_playerclass_t bg_q3f_playerclass_paramedic;
extern bg_q3f_playerclass_t bg_q3f_playerclass_minigunner;
extern bg_q3f_playerclass_t bg_q3f_playerclass_flametrooper;
extern bg_q3f_playerclass_t bg_q3f_playerclass_agent;
extern bg_q3f_playerclass_t bg_q3f_playerclass_engineer;
extern bg_q3f_playerclass_t bg_q3f_playerclass_civilian;

g_q3f_playerclass_t g_q3f_playerclass_null = {
	// A null class, should never be apparent in real life.

	&bg_q3f_playerclass_null,

	0,		// Class init function
	0,		// Class term function
	0,		// Class death function

	0,		// Client commands
};

g_q3f_playerclass_t g_q3f_playerclass_recon = {
	// Recon class

	&bg_q3f_playerclass_recon,

	0,		// Class init function
	0,		// Class term function
	&G_Q3F_Recon_Death_Cleanup,		// Class death function

	&G_Q3F_Recon_Command,	// Client commands
};

g_q3f_playerclass_t g_q3f_playerclass_sniper = {
	// Sniper class

	&bg_q3f_playerclass_sniper,

	0,								// Class init function
	0,								// Class term function
	&G_Q3F_Sniper_Death_Cleanup,	// Class death function

	0,		// Client commands
};

g_q3f_playerclass_t g_q3f_playerclass_soldier = {
	// Soldier class

	&bg_q3f_playerclass_soldier,

	0,		// Class init function
	0,		// Class term function
	0,		// Class death function

	0,		// Client commands
};

g_q3f_playerclass_t g_q3f_playerclass_grenadier = {
	// Grenadier class

	&bg_q3f_playerclass_grenadier,

	0,		// Class init function
	&G_Q3F_Grenadier_Term_Cleanup,		// Class term function
	&G_Q3F_Grenadier_Death_Cleanup,		// Class death function

	&G_Q3F_Grenadier_Command,		// Client commands
};

g_q3f_playerclass_t g_q3f_playerclass_paramedic = {
	// Paramedic class

	&bg_q3f_playerclass_paramedic,

	0,		// Class init function
	0,		// Class term function
	0,		// Class death function

	0,		// Client commands
};

g_q3f_playerclass_t g_q3f_playerclass_minigunner = {
	// Minigunner class

	&bg_q3f_playerclass_minigunner,

	0,		// Class init function
	0,		// Class term function
	&G_Q3F_Minigunner_Death_Cleanup,		// Class death function

	0,		// Client commands
};

g_q3f_playerclass_t g_q3f_playerclass_flametrooper = {
	// Flametrooper class

	&bg_q3f_playerclass_flametrooper,

	0,		// Class init function
	0,		// Class term function
	0,		// Class death function

	0,		// Client commands
};

g_q3f_playerclass_t g_q3f_playerclass_agent = {
	// Secret Agent class

	&bg_q3f_playerclass_agent,

	0,		// Class init function
	G_Q3F_Agent_Term_Cleanup,		// Class term function
	G_Q3F_Agent_Death_Cleanup,	// Class death function

	0,		// Client commands
};

g_q3f_playerclass_t g_q3f_playerclass_engineer = {
	// Engineer class

	&bg_q3f_playerclass_engineer,

	0,		// Class init function
	G_Q3F_Engineer_Term_Cleanup,		// Class term function
	G_Q3F_Engineer_Death_Cleanup,		// Class death function

	0,		// Client commands
};

g_q3f_playerclass_t g_q3f_playerclass_civilian = {
	// Engineer class

	&bg_q3f_playerclass_civilian,

	0,		// Class init function
	0,		// Class term function
	0,		// Class death function

	0,		// Client commands
};

// Array of pointers to class structures
g_q3f_playerclass_t *g_q3f_classlist[Q3F_CLASS_MAX] = {
	0,
	&g_q3f_playerclass_recon,
	&g_q3f_playerclass_sniper,
	&g_q3f_playerclass_soldier,
	&g_q3f_playerclass_grenadier,
	&g_q3f_playerclass_paramedic,
	&g_q3f_playerclass_minigunner,
	&g_q3f_playerclass_flametrooper,
	&g_q3f_playerclass_agent,
	&g_q3f_playerclass_engineer,
	&g_q3f_playerclass_civilian
};


	// Utility functions

g_q3f_playerclass_t *G_Q3F_GetClass( const playerState_t *ps )
{
	// Get the class pointer from the playerstate

	g_q3f_playerclass_t *cls;

	if(	ps->persistant[PERS_CURRCLASS] < 1 ||
		ps->persistant[PERS_CURRCLASS] >= Q3F_CLASS_MAX )
		return( &g_q3f_playerclass_null );
	cls = g_q3f_classlist[ps->persistant[PERS_CURRCLASS]];
	return( cls ? cls : &g_q3f_playerclass_null );
}

int G_Q3F_IsValidClass( int classnum, int teamnum, gentity_t *ignoreent )
{
	// Return 0 if the class is selectable, 1 if not, and -1 if yes, but
	// maximum is reached.

	int count;
	gentity_t *ent;

	if( teamnum == Q3F_TEAM_FREE || teamnum == Q3F_TEAM_SPECTATOR )
		return( 0 );

	if( !g_q3f_teamlist[teamnum].classmaximums[classnum] )
		return( 1 );		// Not allowed
	if( g_q3f_teamlist[teamnum].classmaximums[classnum] > level.maxclients )
		return( 0 );		// No (effective) limit.

	for( count = 0, ent = g_entities; ent < &g_entities[MAX_CLIENTS]; ent++ )
	{
		if(	ent->inuse && ent != ignoreent &&
			ent->client->sess.sessionTeam == teamnum && ent->client->ps.persistant[PERS_CURRCLASS] )
			count++;
	}
	return( (count < g_q3f_teamlist[teamnum].classmaximums[classnum]) ? 0 : -1 );
}

int G_Q3F_SelectRandomClass( int teamnum, gentity_t *ent )
{
	// Select a random playerclass, obeying max class limits

	int index, validclasses, classcount;

	for( index = validclasses = classcount = 0; index < Q3F_CLASS_MAX; index++ )
	{
		if( !G_Q3F_IsValidClass( index, teamnum, ent ) )
		{
			validclasses |= (1 << index);
			classcount++;
		}
	}
	classcount = (rand() % classcount) + 1;	// This will be between 1 and classcount
	for( index = 0; index < Q3F_CLASS_MAX; index++ )
	{
		if( validclasses & (1 << index) )
		{
			if( !--classcount )
				return( index );
		}
	}
	return( 0 );		// Will be a NULL class - a bad thing
}

void G_Q3F_SetClassMaskString(void)
{
	// Generate a string of valid classes for each team, for
	// precaching purposes. Not precaching won't prevent the
	// client loading, but slows it down.
	// Also precache 'essential' models and sounds.

	int clsnum, teamnum, mask, wantclass;

	mask = 0;
	for( clsnum = 0; clsnum < Q3F_CLASS_MAX; clsnum++ )
	{
		for( teamnum = Q3F_TEAM_RED; teamnum < Q3F_TEAM_SPECTATOR; teamnum++ )
		{	
			wantclass = g_q3f_teamlist[teamnum].classmaximums[clsnum];
			if( wantclass && (g_q3f_allowedteams & (1 << teamnum)) )				// g_q3f_teamlist[teamnum].classmaximums[clsnum] 
				mask |= 1 << clsnum;
		}
	}
	trap_SetConfigstring( CS_CLASSMASK, va("%i", mask) );
}

void G_Q3F_DropClient( gentity_t *ent, const char *reason )
{
	// Remove a client, cleaning up first.

	g_q3f_playerclass_t *cls;

	if( !ent->client )
		return;
	cls = G_Q3F_GetClass( &ent->client->ps );
	if( ent->health > 0 )		// Kill them first :)
		G_Damage( ent, NULL, ent, NULL, NULL, 10000, DAMAGE_NO_PROTECTION, MOD_DISCONNECT );
	if( cls->EndClass )
		cls->EndClass( ent );
	trap_SendServerCommand(-1, va("print \"Disconnected %s (%s)\n\"", ent->client->pers.netname, reason));
	G_Q3F_GenericEndCleanup( ent );
	trap_DropClient( ent - g_entities, reason, 0 );
}

void G_Q3F_MuteClient( gentity_t *ent, qboolean mute )
{
	// Mute a client

	if( !ent->client )
		return;

	ent->client->sess.muted = mute;
}

/*
**	Functions called on class death
*/

void G_Q3F_Sniper_Death_Cleanup( struct gentity_s *ent)
{
	// Remove the sniper dot on death.
	if(ent->client->sniperdot)
	{
		G_Q3F_SniperDot(ent,qfalse);
		ent->client->ps.eFlags &= ~EF_Q3F_AIMING;
		trap_LinkEntity(ent);
	}
}

void G_Q3F_Minigunner_Death_Cleanup( struct gentity_s *ent)
{
	// Remove the shudder/slowdown dot on death.
	ent->client->ps.eFlags &= ~EF_Q3F_AIMING;
	trap_LinkEntity(ent);
}

void G_Q3F_Recon_Death_Cleanup( struct gentity_s *ent)
{
	if(ent->client->ps.stats[STAT_Q3F_FLAGS] & 1<< FL_Q3F_SCANNER)
		G_Q3F_ToggleScanner(ent);
}

void G_Q3F_Agent_Death_Cleanup( struct gentity_s *self )
{
	// Cancel any invisible/disguise effects

	G_Q3F_StopAgentDisguise( self );
	G_Q3F_StopAgentInvisible( self );
}

void G_Q3F_GenericEndCleanup( struct gentity_s *self )
{
	// Clean up everything that could cause problems

	gentity_t *ent;
	int ownernum;
	bg_q3f_grenade_t *gren;

	ownernum = self - g_entities;
	for( ent = &g_entities[MAX_CLIENTS]; ent < &g_entities[level.num_entities]; ent++ )
	{
		if( !ent->inuse )
			continue;
		if( ent->s.eType == ET_MISSILE )
		{
			if( ent->r.ownerNum == ownernum )
			{
				if( ent->splashDamage )
					G_ExplodeMissile( ent );
				else G_FreeEntity( ent );
			}
		}
		else if( ent->s.eType == ET_Q3F_GRENADE )
		{
			if( ent->activator == self )
			{
				gren = BG_Q3F_GetGrenade( ent->s.weapon );
				if( gren->damage )
				{
					G_RadiusDamage(	ent->r.currentOrigin, ent, ent->activator,
									gren->damage, ent, gren->mod, 0 );
				}
				if( !(gren->flags & Q3F_GFLAG_EXTENDEDEFFECT) && ent->think )
					ent->think( ent );
				G_FreeEntity( ent );
			}
		}
	}
}

void G_Q3F_Global_Death_Cleanup( struct gentity_s *self)
{
	// Remove Flames
	
	struct gentity_s *ent;

	ent = NULL;

	while ((ent = G_Find (ent, FOFS(classname), "flame")) != NULL)
	{
		if(ent->target_ent == self)
		{
			//ent->nextthink = 0;
			ent->nextthink = level.time;
			ent->think = G_FreeEntity;
			//ent->think = NULL;
			//G_FreeEntity(ent);
		}
	}
	self->client->flames = 0;

	// In case they were in no-knockback-mode. Don't know if this is needed,really.

	self->flags &= ~FL_NO_KNOCKBACK;

	// Remove the shudder/slowdown dot on death. (Do this _ALL_ the time... not just if you're a minigunner
	self->client->ps.eFlags &= ~EF_Q3F_AIMING;
	trap_LinkEntity(self);

	// Remove conc/flash/gas
	self->client->ps.powerups[PW_Q3F_CONCUSS]	= 0;
	self->client->ps.powerups[PW_Q3F_FLASH]		= 0;
	self->client->ps.powerups[PW_Q3F_GAS]		= 0;

	// Remove Tranqs

/*	ent = NULL;

	while ((ent = G_Find (ent, FOFS(classname), "tranq")) != NULL)
	{
		if(ent->target_ent == self)
			G_FreeEntity(ent);
	}
	self->client->tranq = 0;
*/

	// Remove Diseases

//	self->client->next_disease = 0;		// Automatically cleaned up on death anyway

	// Remove leg wounds

//	self->client->legwounds = 0;		// Automatically cleaned up on death anyway

}

void G_Q3F_Engineer_Death_Cleanup( gentity_t *ent )
{
	if( ent->client->sentry )
		G_Q3F_SentryCancel( ent->client->sentry );
	if( ent->client->supplystation )
		G_Q3F_SupplyStationCancel( ent->client->supplystation );
}

/*
**	Class termination cleanup
*/

void G_Q3F_Agent_Term_Cleanup( gentity_t *ent )
{
	if( ent->client->agentdata )
	{
		if( ent->client->agentdata->inuse && ent->client->agentdata->s.eType == ET_Q3F_AGENTDATA )
			G_FreeEntity( ent->client->agentdata );
#ifdef _DEBUG
		else G_Printf( "Attempted to free '%s' as agentdata.\n", (ent->client->agentdata ? ent->client->agentdata->classname : "NULL") );
#endif
	}

	ent->client->agentdata = NULL;
	ent->client->agentclass = 0;
	ent->client->agentteam = 0;
}

void G_Q3F_Engineer_Term_Cleanup( gentity_t *ent )
{
	if( ent->client->sentry )
		G_Q3F_SentryDie( ent->client->sentry, NULL, ent, 0, 0  );
	if( ent->client->supplystation )
		G_Q3F_SupplyStationDie( ent->client->supplystation, NULL, ent, 0, 0  );
}

/*
**	Commands
*/

qboolean G_Q3F_Grenadier_Command( struct gentity_s *ent, char *cmd )
{
	if(Q_stricmp("det",cmd) == 0 || Q_stricmp("detpipe", cmd) == 0 || Q_stricmp( "special", cmd) == 0)	// JT - Allow various aliases
	{
/*		if(ent->client->ps.weaponstate == WEAPON_FIRING && ent->client->ps.weaponTime > 100)
		{
			// JT: Weapon is still recycling... so don't allow det.
			return qtrue;
		}
*/
		if(level.ceaseFire || ent->client->ps.powerups[PW_Q3F_CEASEFIRE])
			return(qtrue);

		if(Q3F_IsSpectator(ent->client))
			return(qtrue);

		G_Q3F_DetPipe(ent, level.time);
		return(qtrue);
	}
	return(qfalse);

}

qboolean G_Q3F_Recon_Command( struct gentity_s *ent, char *cmd )
{
	if(Q_stricmp("scanner",cmd) == 0 || Q_stricmp("special", cmd) == 0)	// JT - Allow various aliases
	{
		if(level.ceaseFire || ent->client->ps.powerups[PW_Q3F_CEASEFIRE])
			return(qtrue);
		if(Q3F_IsSpectator(ent->client))
			return(qtrue);
		G_Q3F_ToggleScanner(ent);
		return(qtrue);
	}
	return(qfalse);

}


void G_Q3F_Grenadier_Term_Cleanup( gentity_t *ent )
{
	// Fizzle any detpacks

	if(	ent->client && ent->client->chargeEntity &&
		ent->client->chargeEntity->inuse &&
		!Q_stricmp( ent->client->chargeEntity->classname, "charge" ) )
	{
		trap_SendServerCommand( -1, va( "print \"%s^7's HE charge failed to go off.\n\"", ent->client->pers.netname ) );
		G_FreeEntity( ent->client->chargeEntity );
		ent->client->chargeEntity = NULL;
		ent->client->chargeTime = 0;
	}
}

void G_Q3F_Grenadier_Death_Cleanup( struct gentity_s *ent)
{
	// Detonate the blokes pipes on death, remove charges
	G_Q3F_DetPipe(ent, level.time);
	if( ent->client->chargeTime )
	{
		if( ent->client->chargeEntity )
		{
			if( ent->client->chargeEntity->inuse && ent->client->chargeEntity->s.eType == ET_Q3F_GRENADE && ent->client->chargeEntity->s.weapon == Q3F_GREN_CHARGE )
				G_FreeEntity( ent->client->chargeEntity );
	#ifdef _DEBUG
			else G_Printf( "Attempted to free '%s' as charge.\n", ent->client->chargeEntity->classname );
	#endif
			ent->client->chargeTime = 0;
			ent->client->chargeEntity = NULL;
		}
	}
}


void G_Q3F_ToggleScanner(gentity_t *ent)
{
	struct gentity_s *agentent;

	if(!ent->client)
		G_Error("PANIC: G_Q3F_ToggleScanner called with null client");

	if(	ent->client->ps.stats[STAT_HEALTH] > 0 && !(ent->client->ps.stats[STAT_Q3F_FLAGS] & (1 << FL_Q3F_SCANNER )))
	{
		if (level.ceaseFire)
			return;

		if( ent->client->ps.ammo[AMMO_CELLS] <= 0 )	{
			trap_SendServerCommand( ent->s.number, va("print \"You need cells to operate your scanner.\n\"") );
			return;
		}

		ent->client->ps.stats[STAT_Q3F_FLAGS] |= 1<< FL_Q3F_SCANNER;
		agentent = G_Spawn();	// No current ent, make a new one

		agentent->s.eType				= ET_Q3F_SCANNERDATA;
		agentent->classname			= "scannerdata";
		agentent->s.otherEntityNum	= ent - level.gentities;
		ent->s.otherEntityNum		= agentent->s.number;
		agentent->activator			= ent;
		//agentent->s.time				= level.time;
		//agentent->s.time2				= level.time + AGENTCHANGETIME;
		//agentent->s.modelindex2		= 0;
		//agentent->s.torsoAnim			= agentclass	? agentclass	: ent->client->ps.persistant[PERS_CURRCLASS];
		//agentent->s.weapon			= agentteam	? agentteam	: ent->client->sess.sessionTeam;
		agentent->think				= G_Q3F_Check_Scanner;
		agentent->nextthink			= level.time;// + Q3F_SCANNER_UPDATE_INTERVAL;
		agentent->wait				= level.time;
		VectorCopy( ent->r.currentOrigin, agentent->s.pos.trBase );	

		// Ensiform : FAIL FAIL WHY WERE WE MODIFYING THE CLIENT ORIGIN ?!
		SnapVector( agentent->s.pos.trBase );
		G_SetOrigin( agentent, agentent->s.pos.trBase );
		//SnapVector( ent->s.pos.trBase );
		//G_SetOrigin( ent, ent->s.pos.trBase );

		ent->client->scanner_ent = agentent;
		trap_LinkEntity( agentent );

		trap_SendServerCommand( ent->s.number, va("print \"Scanner Enabled.\n\"") );

	}
	else
	{
		ent->client->ps.stats[STAT_Q3F_FLAGS] &= ~(1<< FL_Q3F_SCANNER);
		if( ent->client->scanner_ent )
		{
			if( ent->client->scanner_ent->inuse && ent->client->scanner_ent->s.eType == ET_Q3F_SCANNERDATA )
				G_FreeEntity(ent->client->scanner_ent);
#ifdef _DEBUG
			else G_Printf( "Attempted to free '%s' as scanner data.\n", ent->client->scanner_ent->classname );
#endif

			ent->client->scanner_ent = NULL;
			trap_SendServerCommand( ent->s.number, va("print \"Scanner Disabled.\n\"") );
		}
	}
}

qboolean G_Q3F_ChangeClassCommand( struct gentity_s *ent, char *cmd )
{
	// Check the array of classes, see if this is a command to change
	// into one. Also (at some point), check to see class limits etc.

	int index;
	char *synptr;
	qboolean matched;
	bg_q3f_playerclass_t *cls;

	if( !cmd || !*cmd )
		return( qfalse );

	// RR2DO2: spectators can't call this!
	if (ent->client->sess.sessionTeam == Q3F_TEAM_SPECTATOR)
		return( qfalse );

	for( index = 1; index < Q3F_CLASS_MAX; index++ )
	{
		cls = bg_q3f_classlist[index];
		matched = qfalse;
		if(	cls && bg_q3f_classlist[index]->commandstring &&
			*bg_q3f_classlist[index]->commandstring &&
			!Q_stricmp( bg_q3f_classlist[index]->commandstring, cmd ) )
			matched = qtrue;
		else {
			synptr = cls->commandsynonyms;
			while( *synptr && !matched )
			{
				// Check each NULL-terminated synonym (double-NULL list terminator)
				if( !Q_stricmp( synptr, cmd ) )
					matched = qtrue;
				else {
					while( *++synptr );
					synptr++;	// Skip the trailing NULL
				}
			}
		}
		if( matched )
		{
			// We have a match.
			// RR2DO2: is it a valid class right now?
			if( g_q3f_teamlist[ent->client->sess.sessionTeam].classmaximums[index] == -1 ||
				g_q3f_teamlist[ent->client->sess.sessionTeam].classmaximums[index] > G_Q3F_ClassCount(ent->client->sess.sessionTeam, index) )
			{
				if( Q3F_IsSpectator( ent->client ) )
					trap_SendServerCommand( ent->s.number, va("print \"You have chosen to become a %s.\n\"", bg_q3f_classlist[index]->title ));
				else if( ent->health <= 0 )
					trap_SendServerCommand( ent->s.number, va("print \"When you respawn you will become a %s.\n\"", bg_q3f_classlist[index]->title ));
				else trap_SendServerCommand( ent->s.number, va("print \"After dying you will become a %s.\n\"", bg_q3f_classlist[index]->title ));
				ent->client->sess.sessionClass = index;

#ifdef DREVIL_BOT_SUPPORT
				{
				BotUserData data;
				data.m_DataType = dtInt;
				data.udata.m_Int = index;
				Bot_Interface_SendEvent(MESSAGE_CHANGECLASS, ent->s.number, 0,0, &data);
				}
#endif
				// RR2DO2: if we had a null class, we can and should (re)spawn now
				if (ent->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL) {
					respawn( ent );
				}
			}
			else {
				trap_SendServerCommand( ent-g_entities, "print \"This class is not enabled.\n\"");
				return( qtrue );
			}

			// Cancel any class selection menus
			trap_SendServerCommand( ent-g_entities, "menu cancel class\n" );
			return( qtrue );
		}
	}

	if( !Q_stricmp( "randompc", cmd ) )
	{
		trap_SendServerCommand( ent-g_entities, "print \"After dying you will become a random class.\n\"" );
		ent->client->sess.sessionClass = Q3F_CLASS_MAX;

		// RR2DO2: if we had a null class, we can and should (re)spawn now
		if (ent->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL)
			respawn( ent );

		// Cancel any class selection menus
		trap_SendServerCommand( ent-g_entities, "menu cancel class\n" );
		return( qtrue );
	}

	return( qfalse );
}

/*	 Q3F_SetupClass - JT
--   Sets up initial load-out and maximums, et al --
*/
void Q3F_SetupClass(struct gentity_s *ent)
{
	bg_q3f_playerclass_t *cls;
	bg_q3f_weapon_t *weapon;
	int i;
	int weaponnum;
	playerState_t *ps;

	if(!ent->client)			// JT - Safety
		return;

#ifdef BUILD_BOTS
	Bot_Event_ResetWeapons( ent );
#endif

	ps = &(ent->client->ps);

	cls = BG_Q3F_GetClass(&(ent->client->ps));

	// Sort out initial health
	ent->health = ent->client->ps.stats[STAT_HEALTH] = cls->maxhealth;

	// Sort out initial armour
	if(g_spawnFullStats.integer)
		ent->client->ps.stats[STAT_ARMOR] = cls->maxarmour;
	else
		ent->client->ps.stats[STAT_ARMOR] = cls->initarmour;

	// Sort out initial armour type and class
	ent->client->ps.stats[STAT_ARMORTYPE] = cls->initarmourtype;
	ent->client->ps.stats[STAT_Q3F_ARMOURCLASS] = cls->initarmourclass;

	// Sort out initial weapons and initial clips.

	ent->client->ps.stats[STAT_WEAPONS] = (1 << WP_AXE);
	
	if(g_spawnFullStats.integer)
	{
		ent->client->ps.ammo[AMMO_SHELLS]	= cls->maxammo_shells;
		ent->client->ps.ammo[AMMO_NAILS]	= cls->maxammo_nails;
		ent->client->ps.ammo[AMMO_ROCKETS]  = cls->maxammo_rockets;
		ent->client->ps.ammo[AMMO_CELLS]	= cls->maxammo_cells;
		ent->client->ps.ammo[AMMO_MEDIKIT]	= cls->maxammo_medikit;
		ent->client->ps.ammo[AMMO_CHARGE]	= cls->maxammo_charge;
		ent->client->ps.ammo[AMMO_NONE]		= -1;				// JT - Don't ask.

		ent->client->ps.ammo[AMMO_GRENADES]	= cls->gren1max + (cls->gren2init << 8);
	}
	else
	{
		ent->client->ps.ammo[AMMO_SHELLS]	= cls->initammo_shells;
		ent->client->ps.ammo[AMMO_NAILS]	= cls->initammo_nails;
		ent->client->ps.ammo[AMMO_ROCKETS]  = cls->initammo_rockets;
		ent->client->ps.ammo[AMMO_CELLS]	= cls->initammo_cells;
		ent->client->ps.ammo[AMMO_MEDIKIT]	= cls->initammo_medikit;
		ent->client->ps.ammo[AMMO_CHARGE]	= cls->initammo_charge;
		ent->client->ps.ammo[AMMO_NONE]		= -1;				// JT - Don't ask.

		ent->client->ps.ammo[AMMO_GRENADES]	= cls->gren1init + (cls->gren2init << 8);
	}

	for(i =0; i < Q3F_NUM_WEAPONSLOTS; i ++)
	{
		if(cls->weaponslot[i])
		{
			weaponnum = cls->weaponslot[i];
			weapon = BG_Q3F_GetWeapon(weaponnum);
			ent->client->ps.stats[STAT_WEAPONS] |= 1 << weaponnum;
			#ifdef BUILD_BOTS
			Bot_Event_AddWeapon( ent, Bot_WeaponGameToBot( weaponnum, ent->client->ps.persistant[PERS_CURRCLASS] ) );
			#endif
			Q3F_SetClipValue(weaponnum,weapon->clipsize,ps);
		}
	}

	VectorCopy(cls->mins, ent->r.mins);
	VectorCopy(cls->maxs, ent->r.maxs);
}

/* Q3F_IsSpectator - RR2DO2
-- Returns true if the examined client is a spectator
*/
qboolean Q3F_IsSpectator(struct gclient_s *client)
{
	if (client->sess.sessionTeam == Q3F_TEAM_SPECTATOR)
		return (qtrue);
	if( client->ps.eFlags & EF_Q3F_NOSPAWN )
		return( qtrue );
	else 
	{
		if( client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_NULL || client->sess.sessionClass == Q3F_CLASS_NULL )
			return (qtrue);
	}

	return (qfalse);
}

/*
**	Agent disguise functions
*/

qboolean G_Q3F_IsDisguised( gentity_t *ent )
{
	// Work out if a player is disguised

	if( !ent->client || ent->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_AGENT )
		return( qfalse );
	if( !ent->client->agentdata || (ent->client->agentdata->s.modelindex2 & Q3F_AGENT_DISGUISEMASK) != Q3F_AGENT_DISGUISE )
		return( qfalse );
	return( qtrue );
}

static void G_Q3F_AgentThink( gentity_t *ent )
{
	// Work out if we still require this ent, and whether a
	// ClientUserinfoChanged() call is required.

	int mode, disguisethink, invisthink, disguiseendtime, invisendtime;
	gclient_t *client;
	qboolean infochange, flagchange;

	mode = ent->s.modelindex2;
	client = ent->activator->client;

	infochange = flagchange = qfalse;
	disguisethink = invisthink = -1;
	disguiseendtime = invisendtime = 0;

	if( mode & Q3F_AGENT_DISGUISE )
	{
		disguiseendtime = -1;

		if( level.time < ent->s.time2 )
			disguisethink = ent->s.time2;
		else {
			// We've passed the transition, now what?
			if( mode & Q3F_AGENT_DISGUISEEND )
			{
				if( client->agentclass || client->agentteam )
				{
					client->agentclass = client->agentteam = 0;
					infochange = qtrue;
					ent->s.modelindex2 &= ~Q3F_AGENT_DISGUISEMASK;
				}
				if( !client->agentclass )
				{
					client->ps.eFlags &= ~EF_Q3F_STEPMASK;
					BG_PlayerStateToEntityState( &client->ps, &ent->activator->s, qfalse );
				}

				disguisethink = disguiseendtime = ent->s.time2 + 2000;	// Time we can end disguise
			}
			else {
				if(	(client->agentclass != ent->s.torsoAnim ||
					client->agentteam != ent->s.weapon) &&
					level.time >= ent->s.time2 )
				{
					ent->s.otherEntityNum2 = DISGUISING_NOT;
					client->agentclass = ent->s.torsoAnim;
					client->agentteam = ent->s.weapon;
					client->ps.eFlags &= ~EF_Q3F_STEPMASK;
					if( client->agentclass && client->agentclass != Q3F_CLASS_AGENT )
						client->ps.eFlags |= (client->agentclass == Q3F_CLASS_MINIGUNNER) ? EF_Q3F_METALSTEPS : EF_Q3F_FOOTSTEPS;
					BG_PlayerStateToEntityState( &client->ps, &ent->activator->s, qfalse );
					infochange = qtrue;
					trap_SendServerCommand( ent->activator->s.number, "print \"Disguise complete.\n\"" );
				}
			}
		}
	}
	if( mode & Q3F_AGENT_INVIS )
	{
		// Only act if we're A: ending, and B: past the fadeout

		invisendtime = -1;
		if( mode & Q3F_AGENT_INVISEND )
		{
			invisthink = invisendtime = ent->s.origin2[0] + 1000;
			if( invisthink != -1 && level.time >= invisthink )
			{
				invisthink = -1;
				flagchange = qtrue;
				ent->s.modelindex2 &= ~Q3F_AGENT_INVISMASK;
			}
		}
	}

	if(invisthink == -1) {
		ent->nextthink = disguisethink;
	} else if(disguisethink == -1) {
		ent->nextthink = invisthink;
	} else {
		ent->nextthink = (disguisethink < invisthink) ? disguisethink : invisthink;
	}

	if( flagchange )
	{
		client->ps.eFlags &= ~(EF_Q3F_DISGUISE|EF_Q3F_INVISIBLE);
		if( disguisethink && (level.time < disguiseendtime || disguiseendtime == -1) )
			client->ps.eFlags |= EF_Q3F_DISGUISE;
		if( invisthink && (level.time < invisendtime || invisendtime == -1) )
			client->ps.eFlags |= EF_Q3F_INVISIBLE;
		BG_PlayerStateToEntityState( &client->ps, &ent->activator->s, qtrue );
	}
	if( infochange )
		ClientUserinfoChanged( ent->activator->s.number, "agentthink" );

	if(	(!disguiseendtime || (level.time >= disguiseendtime && disguiseendtime != -1)) &&
		(!invisendtime || (level.time >= invisendtime && invisendtime != -1)) )
	{
		client->agentdata = NULL;
		G_FreeEntity( ent );
		return;
	}
}

void G_Q3F_StartAgentDisguise( gentity_t *ent, int agentclass, int agentteam, disguising_t dt )
{
	// Start disguising (only if we're not _already_ disguising or vice versa)
	// This is rather verbose (in that it sends messages to the player).

	gentity_t *goalitem;

	if( !ent->client )
		return;
	if(	ent->client->agentdata &&
		(ent->client->agentdata->s.modelindex2 & Q3F_AGENT_DISGUISEMASK) == Q3F_AGENT_DISGUISE &&
		ent->client->agentdata->s.time2 > level.time )
	{
		trap_SendServerCommand( ent->s.number, "print \"You are already disguising.\n\"" );
		return;		// We're already disguising
	}

	for( goalitem = g_entities; goalitem < &g_entities[level.num_entities] ; goalitem++ )
	{
		if( !goalitem->inuse )
			continue;
		if( (goalitem->s.eType == ET_Q3F_GOAL) && (goalitem->activator == ent) &&
			goalitem->mapdata && (goalitem->mapdata->state == Q3F_STATE_CARRIED) &&
			(goalitem->mapdata->flags & Q3F_FLAG_REVEALAGENT) )
		{
			trap_SendServerCommand( ent->s.number, "print \"You cannot disguise now.\n\"" );
			return;
		}
	}

		// Not in the middle of anything, start now.
	if( ent->client->agentdata ) {
		// already had an agentdata, store current disguise info
		ent->client->agentdata->s.legsAnim		= ent->client->agentdata->s.torsoAnim;	// class
		ent->client->agentdata->s.frame			= ent->client->agentdata->s.weapon;		// team
	}
	//if( !ent->client->agentdata )
	else
	{
		ent->client->agentdata = G_Spawn();	// No current ent, make a new one
#ifdef _DEBUG
		G_Printf( "Agentdata for %s created.\n", ent->client->pers.netname );
#endif
		ent->client->agentdata->s.legsAnim		= 0;
		ent->client->agentdata->s.frame			= 0;
	}

	ent->client->agentdata->s.eType				= ET_Q3F_AGENTDATA;
	ent->client->agentdata->classname			= "agentdata";
	ent->client->agentdata->s.otherEntityNum	= ent - level.gentities;
	ent->client->agentdata->activator			= ent;
	ent->client->agentdata->s.time				= level.time;
	ent->client->agentdata->s.time2				= level.time + Q3F_AGENT_MORPH_TIME;
	ent->client->agentdata->s.modelindex2		= (ent->client->agentdata->s.modelindex2 & ~Q3F_AGENT_DISGUISEMASK) | Q3F_AGENT_DISGUISE;	// Disguise on
	ent->client->agentdata->s.torsoAnim			= agentclass ? agentclass : ent->client->ps.persistant[PERS_CURRCLASS];
	ent->client->agentdata->s.weapon			= agentteam ? agentteam : ent->client->sess.sessionTeam;
	ent->client->agentdata->s.otherEntityNum2	= dt;
	ent->client->agentdata->think				= G_Q3F_AgentThink;
	G_SetOrigin( ent->client->agentdata, ent->r.currentOrigin );
	trap_LinkEntity( ent->client->agentdata );
	G_Q3F_AgentThink( ent->client->agentdata );

	ent->client->ps.eFlags |= EF_Q3F_DISGUISE;
	BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );

#ifdef _DEBUG
	G_Printf( "Agentdata for %s set to disguise.\n", ent->client->pers.netname );
#endif
	trap_SendServerCommand( ent->s.number, "print \"Beginning Disguise.\n\"" );
}

void G_Q3F_StopAgentDisguise( gentity_t *ent )
{
	// Stop disguising (if we're disguising)

	if( !ent->client || ent->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_AGENT )
		return;
	if(	ent->client->agentdata && (ent->client->agentdata->s.modelindex2 & Q3F_AGENT_DISGUISEMASK) == Q3F_AGENT_DISGUISEMASK )
		return;		// We're already undisguising
	if( !ent->client->agentdata || !(ent->client->agentdata->s.modelindex2 & Q3F_AGENT_DISGUISEMASK) )
		return;			// We're not disguising at all?
	if( !ent->client->agentdata )
	{
#ifdef _DEBUG
		G_Printf( "Agentdata for %s created.\n", ent->activator->client->pers.netname );
#endif
		ent->client->agentdata = G_Spawn();
	}

	ent->client->agentdata->s.eType				= ET_Q3F_AGENTDATA;
	ent->client->agentdata->classname			= "agentdata";
	ent->client->agentdata->s.otherEntityNum	= ent - level.gentities;
	ent->client->agentdata->activator			= ent;
	ent->client->agentdata->s.time				= level.time;
	ent->client->agentdata->s.time2				= level.time + Q3F_AGENT_UNMORPH_TIME;
	ent->client->agentdata->s.modelindex2		|= Q3F_AGENT_DISGUISEMASK;		// Disguise off.
	ent->client->agentdata->s.torsoAnim			= ent->client->ps.persistant[PERS_CURRCLASS];
	ent->client->agentdata->s.weapon			= ent->client->sess.sessionTeam;
	ent->client->agentdata->think				= G_Q3F_AgentThink;
	G_Q3F_AgentThink( ent->client->agentdata );

	ent->client->ps.eFlags |= EF_Q3F_DISGUISE;
	BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );

#ifdef _DEBUG
	G_Printf( "Agentdata for %s set to strip disguise.\n", ent->client->pers.netname );
#endif

	G_SetOrigin( ent->client->agentdata, ent->r.currentOrigin );
	trap_LinkEntity( ent->client->agentdata );
}

void G_Q3F_StartAgentInvisible( gentity_t *ent )
{
	// Start becoming invisible

	gentity_t *goalitem;

	if( !ent->client || ent->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_AGENT )
		return;
	if(	ent->client->agentdata &&
		(ent->client->agentdata->s.modelindex2 & Q3F_AGENT_INVISMASK) == Q3F_AGENT_INVIS )
	{
		trap_SendServerCommand( ent->s.number, "print \"Invisible is already active.\n\"" );
		return;			// We're already going invisible
	}

	for( goalitem = g_entities; goalitem < &g_entities[level.num_entities] ; goalitem++ )
	{
		if( !goalitem->inuse )
			continue;
		if( (goalitem->s.eType == ET_Q3F_GOAL) && (goalitem->activator == ent) &&
			goalitem->mapdata && (goalitem->mapdata->state == Q3F_STATE_CARRIED) &&
			(goalitem->mapdata->flags & Q3F_FLAG_REVEALAGENT) )
		{
			trap_SendServerCommand( ent->s.number, "print \"You cannot become invisible now.\n\"" );
			return;
		}
	}

	if( !ent->client->agentdata )
	{
#ifdef _DEBUG
		G_Printf( "Agentdata for %s created.\n", ent->client->pers.netname );
#endif
		ent->client->agentdata = G_Spawn();
	}

	ent->client->agentdata->s.eType				= ET_Q3F_AGENTDATA;
	ent->client->agentdata->classname			= "agentdata";
	ent->client->agentdata->s.otherEntityNum	= ent - level.gentities;
	ent->client->agentdata->activator			= ent;
	ent->client->agentdata->s.origin2[0]		= level.time;			// Start time for invisible
	ent->client->agentdata->s.modelindex2		= (ent->client->agentdata->s.modelindex2 & ~Q3F_AGENT_INVISMASK) | Q3F_AGENT_INVIS;		// Invisible on
	ent->client->agentdata->think				= G_Q3F_AgentThink;
	G_Q3F_AgentThink( ent->client->agentdata );
	G_SetOrigin( ent->client->agentdata, ent->r.currentOrigin );
	trap_LinkEntity( ent->client->agentdata );

	ent->client->ps.eFlags |= EF_Q3F_INVISIBLE;		// Lock us in place
	BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );

#ifdef _DEBUG
	G_Printf( "Agentdata for %s set to invisible.\n", ent->client->pers.netname );
#endif
	trap_SendServerCommand( ent->s.number, "print \"Invisibility activated.\n\"" );
}

void G_Q3F_StopAgentInvisible( gentity_t *ent )
{
	// Stop being invisible (if we're invisible)

	if( !(ent->s.eFlags & EF_Q3F_INVISIBLE ) )
		return;
	if( !ent->client || ent->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_AGENT )
		return;
	if( !ent->client->agentdata || (ent->client->agentdata->s.modelindex2 & Q3F_AGENT_INVISMASK) == Q3F_AGENT_INVISMASK )
		return;		// We're not disguised, or already undisguising

	ent->client->agentdata->s.eType				= ET_Q3F_AGENTDATA;
	ent->client->agentdata->classname			= "agentdata";
	ent->client->agentdata->s.otherEntityNum	= ent - level.gentities;
	ent->client->agentdata->activator			= ent;
	ent->client->agentdata->s.origin2[0]		= level.time;
	ent->client->agentdata->s.modelindex2		|= 12;		// Invisible off.
	ent->client->agentdata->think				= G_Q3F_AgentThink;
	G_Q3F_AgentThink( ent->client->agentdata );
	VectorCopy( ent->r.currentOrigin, ent->client->agentdata->s.pos.trBase );
	SnapVector( ent->client->agentdata->s.pos.trBase );
	G_SetOrigin( ent->client->agentdata, ent->r.currentOrigin );
	trap_LinkEntity( ent->client->agentdata );

	ent->client->ps.eFlags |= EF_Q3F_DISGUISE;		// Tell it we've a disguise
	ent->client->ps.eFlags &= ~EF_Q3F_INVISIBLE;	// ... but don't lock us in place
	BG_PlayerStateToEntityState( &ent->client->ps, &ent->s, qtrue );

#ifdef _DEBUG
	G_Printf( "Agentdata for %s set to not invisible.\n", ent->client->pers.netname );
#endif
}

void G_Q3F_DisguiseCommand( gentity_t *ent )
{
	//	We've recieved a 'disguise' command.

	char commandbuff[64];
	int index, numargs, i;

	if( ent->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_AGENT ) {
		trap_SendServerCommand( ent->s.number, "print \"Only agents can disguise themselves.\n\"" );
		return;
	}
	if( ent->client->ps.powerups[PW_QUAD] > level.time ) {
		trap_SendServerCommand( ent->s.number, "print \"Can't disguise when Quad powered.\n\"" );
		return;
	}
	if( ent->client->ps.stats[STAT_HEALTH] <= 0 || level.ceaseFire )
		return;

	numargs = trap_Argc();
	trap_Argv( 1, commandbuff, 64 );

	if( !Q_stricmp( commandbuff, "class" ) && numargs == 3 )
	{
		// Change class

		trap_Argv( 2, commandbuff, 64 );
		for( index = Q3F_CLASS_RECON; index < Q3F_CLASS_MAX; index++ )
		{
			if( !Q_stricmp( bg_q3f_classlist[index]->commandstring, commandbuff ) )
				break;
		}
		if( index == Q3F_CLASS_MAX )
		{
			trap_SendServerCommand( ent->s.number, va( "print \"Unknown class '%s'.\n\"", commandbuff ) );
			return;
		}
		if( index == (ent->client->agentclass ? ent->client->agentclass : ent->client->ps.persistant[PERS_CURRCLASS]) )
			return;
		if( (ent->client->agentteam ? ent->client->agentteam : ent->client->sess.sessionTeam) != Q3F_CLASS_AGENT &&
			!g_q3f_teamlist[ent->client->agentteam ? ent->client->agentteam : ent->client->sess.sessionTeam].classmaximums[index] )
		{
			trap_SendServerCommand( ent->s.number, va( "print \"Class '%s' is unavailable.\n\"", commandbuff ) );
			return;
		}
		G_Q3F_StartAgentDisguise( ent, index, ent->client->agentteam, DISGUISING_CLASS );
	}
	else if( !Q_stricmp( commandbuff, "team" ) && numargs == 3 )
	{
		trap_Argv( 2, commandbuff, 64 );
		// RR2DO2
		for( index = -1, i=Q3F_TEAM_FREE + 1; i < Q3F_TEAM_SPECTATOR; i++)
		{
			if( !Q_stricmp( g_q3f_teamlist[i].name, commandbuff ) &&
				(g_q3f_allowedteams & (1 << i)) )
			{
				// is this team enabled?
				index = G_Q3F_GetTeamNum( commandbuff );
				break;
			}
		}
		// RR2DO2

		if( index > -1 )
		{
			if( index != (ent->client->agentteam ? ent->client->agentteam : ent->client->sess.sessionTeam) )
				G_Q3F_StartAgentDisguise( ent, ent->client->agentclass, index, DISGUISING_TEAM );
		}
		else trap_SendServerCommand( ent->s.number, va( "print \"Unknown/invalid team '%s'.\n\"", commandbuff ) );
	}
	else if( !Q_stricmp( commandbuff, "reset" ) && numargs == 2 )
	{
		// Stop disguising
		if( !G_Q3F_IsDisguised( ent ) )
			trap_SendServerCommand( ent->s.number, "print \"You are not disguised.\n\"" );
		G_Q3F_StopAgentDisguise( ent );
	}
	else if( !Q_stricmp( commandbuff, "menu" ) )
	{
		// Throw a menu

		if( numargs > 2 )
		{
			trap_Argv( 2, commandbuff, 64 );
			if( !Q_stricmp( commandbuff, "class" ) )
				G_Q3F_SendClassMenu( ent, ent->client->agentteam ? ent->client->agentteam : ent->client->sess.sessionTeam );
			else if( !Q_stricmp( commandbuff, "team" ) )
				G_Q3F_SendTeamMenu( ent, qtrue );
			else trap_SendServerCommand( ent->s.number, "print \"Unknown disguise menu option.\n\"" );
		}
		else trap_SendServerCommand( ent->s.number, "menu disguise" );
	}
	else {
		trap_SendServerCommand( ent->s.number, "print \"Usage: disguise class|team|reset|menu [classname|teamname]\n\"" );
	}
}

void G_Q3F_RunAgentData( gentity_t *ent )
{
	// Simply keep the agent data where the owner player is

	if( ent->activator && ent->activator->inuse )
		VectorCopy( ent->activator->r.currentOrigin, ent->s.pos.trBase );
	SnapVector( ent->s.pos.trBase );
	G_SetOrigin( ent, ent->s.pos.trBase );
	trap_LinkEntity( ent );

	G_RunThink( ent );
}

void G_Q3F_InvisibleCommand( gentity_t *ent )
{
	// Start/stop going invisible.

	qboolean start;
	char commandbuff[16];

	if( !ent->client || ent->client->ps.stats[STAT_HEALTH] <= 0 )
		return;

	if( trap_Argc() >= 2 )
	{
		trap_Argv( 1, commandbuff, 16 );
		if( !Q_stricmp( commandbuff, "start" ) )
			start = qtrue;
		else if( !Q_stricmp( commandbuff, "stop" ) )
			start = qfalse;
		else {
			trap_SendServerCommand( ent->s.number, va( "print \"Sorry, '%s' is not a valid invisible command.\n\"", commandbuff ) );
			return;
		}
	}
	else 
		start = !(ent->client->ps.eFlags & EF_Q3F_INVISIBLE);

	if( start )
	{
		// Start invisibility.

		if( ent->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_AGENT )
		{
			trap_SendServerCommand( ent->s.number, "print \"Only agents can use the \\invisible command.\n\"" );
			return;
		}
		if( ent->client->ps.powerups[PW_QUAD] > level.time ) {
			trap_SendServerCommand( ent->s.number, "print \"Can't use invisible when Quad powered.\n\"" );
			return;
		}
		if( ent->client->ps.ammo[AMMO_CELLS] <= 0 )
		{
			trap_SendServerCommand( ent->s.number, "print \"You require cells to become invisible.\n\"" );
			return;
		}
		if( ent->client->ps.groundEntityNum == ENTITYNUM_NONE || VectorLength( ent->client->ps.velocity ) > 20 )
		{
			trap_SendServerCommand( ent->s.number, "print \"You must be standing still to become invisible.\n\"" );
			return;
		}
		G_Q3F_StartAgentInvisible( ent );
	}
	else {
		// Stop being invisible
		if( !ent->client->agentdata || (ent->client->agentdata->s.modelindex2 & Q3F_AGENT_INVISMASK) != Q3F_AGENT_INVIS )
		{
			trap_SendServerCommand( ent->s.number, "print \"Invisibility is not active.\n\"" );
			return;
		}
		G_Q3F_StopAgentInvisible( ent );
	}
}

/*
** Generate classname strings
*/

void G_Q3F_SendAgentClassMenu( gentity_t *player, int teamnum ) {
	int index;//, classsize, size;
	char buff[MAX_INFO_STRING];
	//qboolean unlimited;

	buff[0] = 0;

	for( index = 1; index < Q3F_CLASS_MAX; index++ ) {
		if( !g_q3f_teamlist[teamnum].classmaximums[index] ) {
			Q_strcat( buff, sizeof(buff), "\"\" " );
			continue;
		}
		//size = G_Q3F_ClassCount( teamnum, index );
		//classsize = g_q3f_teamlist[teamnum].classmaximums[index];
		//unlimited = classsize < 0 || classsize > MAX_CLIENTS;
		Q_strcat( buff, sizeof(buff),	va( "\"%s%s\"", 
										((index == player->client->sess.sessionClass) ? "^5" : (index == player->client->ps.persistant[PERS_CURRCLASS] ? "^2" : "")),
										g_q3f_teamlist[teamnum].classnames[index] ) );
	}

	trap_SendServerCommand( player->s.number, va("menu disguiseclass %s", buff) );
}

void G_Q3F_SendClassInfo( gentity_t *player ) {
	int index, classsize, size, teamnum;
	char buff[MAX_INFO_STRING];
	qboolean unlimited;

	if ( player->client->sess.sessionTeam == Q3F_TEAM_SPECTATOR )
		return;

	if( !player->client )
		return;

	teamnum = player->client->sess.sessionTeam;

	buff[0] = 0;

	for( index = 1; index < Q3F_CLASS_MAX; index++ ) {
		if( !g_q3f_teamlist[teamnum].classmaximums[index] ) {
			Q_strcat( buff, sizeof(buff), "\"\" " );
			continue;
		}
		size = G_Q3F_ClassCount( teamnum, index );
		classsize = g_q3f_teamlist[teamnum].classmaximums[index];
		unlimited = classsize < 0 || classsize > MAX_CLIENTS;
		Q_strcat( buff, sizeof(buff),	va( "\"%s%s", 
										((index == player->client->sess.sessionClass) ? "^5" : (index == player->client->ps.persistant[PERS_CURRCLASS] ? "^2" : "")),
										g_q3f_teamlist[teamnum].classnames[index] ) );
		if( size || !unlimited ) {
			if(size > 99) {
				size = 99;
			}
			Q_strcat( buff, sizeof(buff),	va( size >= 10 ? "*%d" : "*0%d", size ) );
		}
		if(classsize > 99) {
			classsize = 99;
		}
		Q_strcat( buff, sizeof(buff), va( !unlimited ? ( classsize >= 10 ? "/%d\" " : "/0%d\" " ) : "\" ", classsize ) );

//		G_Printf(va("classsize: %i\n", classsize));
//		G_Printf(va("size:		%i\n", size));
//		G_Printf(va("unlimited: %i\n", unlimited));
	}

//	G_Printf(va("classinfo %s\n", buff));
	trap_SendServerCommand( player->s.number, va("classinfo %s", buff));
}

void G_Q3F_SendClassMenu( gentity_t *player, int teamnum ) {
	// Generate (and store statically) a menu-string for classnames.
	// If teamnum is non-NULL, it's a disguise menu rather than a class menu

	// spectators never should be here
	if ( player->client->sess.sessionTeam == Q3F_TEAM_SPECTATOR )
		return;

	if( !player->client )
		return;

	if( teamnum ) {
		G_Q3F_SendAgentClassMenu(player, teamnum);
		return;
	}

	G_Q3F_SendClassInfo(player);

	trap_SendServerCommand( player->s.number, "menu class");
}

void G_Q3F_ValidateEntities()
{
	// Attempt to validate all entities that are 'attached' to players - 
	// scanner, disguise, buildings, etc.
	// Also checks for disconnected players that haven't been reported as disconnected,
	// although this isn't really the appropriate place for it.
	// 10 entities are tested every frame, resulting in 1024 entities being
	// tested every 10 seconds or so (assuming 10 frames/sec).

	int index;
	gentity_t *ent, *other;
	qboolean purge;

	for( index = 10; index > 0; --index )
	{
		ent = &g_entities[level.validatedEntityIndex = (level.validatedEntityIndex + 1) & 1023];

		if( !ent->inuse )
			continue;

		purge = qfalse;
		switch( ent->s.eType )
		{
			case ET_Q3F_AGENTDATA:
					other = ent->activator;
					if( !other )
					{
						if( !strcmp( other->classname, "bodyque" ) )
							break;
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Agentdata %d attached to null entity.\n",
									level.validatedEntityIndex );
					}
					else if( !other->inuse || !other->client )
					{
						if( !strcmp( other->classname, "bodyque" ) )
							break;
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Agentdata %d attached to null client %d.\n",
									level.validatedEntityIndex, other->s.number );
					}
					else if(	other->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_AGENT ||
								other->client->agentdata != ent ||
								!(other->s.eFlags & (EF_Q3F_DISGUISE|EF_Q3F_INVISIBLE)) ||
								!(other->client->ps.eFlags & (EF_Q3F_DISGUISE|EF_Q3F_INVISIBLE)) )
								//!(other->client->agentdata->s.modelindex2 & (Q3F_AGENT_DISGUISEMASK|Q3F_AGENT_INVISMASK)) )
					{
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Agentdata %d wrongly attached to '%s'.\n",
									level.validatedEntityIndex, other->client->pers.netname );
					}
					else if(	(ent->s.modelindex2 & Q3F_AGENT_DISGUISEMASK && ent->s.time2 < other->client->respawnTime) ||
								(ent->s.modelindex2 & Q3F_AGENT_INVISMASK && ent->s.origin2[0] < other->client->respawnTime) )
					{
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Agentdata %d trapped over respawn '%s'.\n",
									level.validatedEntityIndex, other->client->pers.netname );
					}
					else if(	(ent->s.modelindex2 & Q3F_AGENT_DISGUISEMASK) == Q3F_AGENT_DISGUISEEND ||
								(ent->s.modelindex2 & Q3F_AGENT_INVISMASK) == Q3F_AGENT_INVISEND )
					{
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Agentdata %d has bad states with '%s'.\n",
									level.validatedEntityIndex, other->client->pers.netname );
					}

					if( purge )
					{
						if( other != NULL && other->client )
						{
							other->client->agentdata	= NULL;
							other->client->agentclass	= 0;
							other->client->agentteam	= 0;
							other->client->ps.eFlags	&= ~(EF_Q3F_DISGUISE|EF_Q3F_INVISIBLE);
							other->s.eFlags				&= ~(EF_Q3F_DISGUISE|EF_Q3F_INVISIBLE);
						}
						G_FreeEntity( ent );
					}
					break;

			case ET_Q3F_SCANNERDATA:
					other = ent->activator;
					if( !other )
					{
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Scannerdata %d attached to null entity.\n",
									level.validatedEntityIndex );
					}
					else if( !other->inuse || !other->client )
					{
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Scannerdata %d attached to null client %d.\n",
									level.validatedEntityIndex, other->s.number );
					}
					else if(	other->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_RECON ||
								other->client->scanner_ent != ent )
					{
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Scannerdata %d wrongly attached to '%s'.\n",
									level.validatedEntityIndex, other->client->pers.netname );
					}

					if( purge )
					{
						if( other != NULL && other->client )
							other->client->scanner_ent = NULL;
						G_FreeEntity( ent );
					}
					break;

			case ET_SNIPER_DOT:
					other = ent->activator;
					if( !other->inuse || !other->client )
					{
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Sniperdot %d attached to null client %d.\n",
									level.validatedEntityIndex, other->s.number );
					}
					else if(	other->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_SNIPER ||
								other->client->sniperdot != ent ||
								other->s.weapon != WP_SNIPER_RIFLE ||
								ent->timestamp < other->client->respawnTime )
					{
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Sniperdot %d wrongly attached to '%s'.\n",
									level.validatedEntityIndex, other->client->pers.netname );
					}

					if( purge )
					{
						if( other != NULL && other->client )
							other->client->sniperdot = NULL;
						G_FreeEntity( ent );
					}
					break;
					
			case ET_Q3F_SENTRY:
					other = ent->parent;
					if( !other ) {
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Autosentry %d attached to null entity.\n",
									level.validatedEntityIndex );
					}
					else if( !other->inuse || !other->client )
					{
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Autosentry %d attached to null client %d.\n",
									level.validatedEntityIndex, other->s.number );
					}
					else if(	other->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_ENGINEER ||
								other->client->sentry != ent )
					{
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Autosentry %d wrongly attached to '%s'.\n",
									level.validatedEntityIndex, other->client->pers.netname );
					}

					if( purge )
					{
						if( other != NULL && other->client )
							other->client->sentry = NULL;
						G_FreeEntity( ent );
					}
					break;

			case ET_Q3F_SUPPLYSTATION:
					other = ent->parent;
					if( !other ) {
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Supplystation %d attached to null entity.\n",
									level.validatedEntityIndex );
					}
					else if( !other->inuse || !other->client )
					{
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Supplystation %d attached to null client %d.\n",
									level.validatedEntityIndex, other->s.number );
					}
					else if(	other->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_ENGINEER ||
								other->client->supplystation != ent )
					{
						purge = qtrue;
						G_Printf(	"G_ValidateEntities(): Supplystation %d wrongly attached to '%s'.\n",
									level.validatedEntityIndex, other->client->pers.netname );
					}

					if( purge )
					{
						if( other != NULL && other->client )
							other->client->supplystation = NULL;
						G_FreeEntity( ent );
					}
					break;
		}
	}
}

