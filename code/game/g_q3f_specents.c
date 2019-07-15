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
**	g_q3f_specents.c
**
**	Specific entities for use with the extended map system.
*/

#include "g_local.h"
#include "bg_public.h"
#include "g_q3f_mapents.h"
#include "g_q3f_playerclass.h"
#include "g_q3f_team.h"
#include "g_q3f_flag.h"
#include "g_q3f_mapselect.h"
#include "g_q3f_weapon.h"
#include "surfaceflags.h"

void ExitLevel( void );

/*
**	Command point.
*/


static int G_Q3F_CommandPointStateThink( gentity_t *ent, gentity_t *activator, int targetstate, int oldstate, int force, trace_t *trace ) {
	if(targetstate == Q3F_STATE_INVISIBLE) {
		ent->mapdata->ownerteam = 0;
		ent->mapdata->team = 0;
		return Q3F_STATE_INACTIVE;
	} else {
		return targetstate;
	}
}

void G_Q3F_CommandPointThink( gentity_t *ent )
{
	// Sit around 'pulsing' now and then - trigger entities on 'pulse',
	// as well as on 'expire'.

	q3f_keypairarray_t *array;
	q3f_keypair_t *kp, *targkp;
	q3f_array_t *targetarray;
	q3f_data_t *data, *targdata;
	int index, targindex;
	gentity_t *target, *activator;
	trace_t trace;

	if( !ent->mapdata->ownerteam )
		return;			// Bad pulse?
	if( ent->mapdata->state == Q3F_STATE_DISABLED || ent->mapdata->state == Q3F_STATE_INVISIBLE )
	{
		ent->nextthink = level.time + ent->wait + ent->random * Q_flrand(-1.0f, 1.0f);
		return;			// Can't pulse, we've been disabled
	}

	activator = ent->activator;
	if(	!(activator && activator->client && activator->client->sess.sessionTeam == ent->mapdata->ownerteam) )
	{
		// The activator is no longer valid, try and find an appropriate player

		for( activator = &level.gentities[0]; activator < &level.gentities[MAX_CLIENTS]; activator++ )
		{
			if( activator->inuse && activator->client && activator->client->sess.sessionTeam == ent->mapdata->ownerteam ) 
				break;		// Stop the loop, we have a match
		}
		if( activator >= &level.gentities[MAX_CLIENTS] )
			activator = NULL;		// Nobody left on the team
	}

	if( activator )
	{
		// Trigger 'pulsetarget'
		if(	ent->mapdata->other &&
			(kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "pulsetarget" ))) )
			G_Q3F_PropogateTrigger( kp->value.d.keypairarraydata, activator, &trace );

		// Perform our 'give' command
		array = ent->mapdata->give;

		if(	ent->mapdata->other &&
			(kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "pulsegive" ))) )
			ent->mapdata->give = kp->value.d.keypairarraydata;
		else ent->mapdata->give = NULL;
		G_Q3F_MapGive( activator, ent );	// Always processes mapdata->give
		ent->mapdata->give = array;
		// And our pulse teamscore (common enough to keep in this ent as well)
		// Check for team scoring
		if(g_matchState.integer <= MATCH_STATE_PLAYING && !level.intermissionQueued && !level.intermissiontime)
		{		
			if(	ent->mapdata->other && (kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "pulseteamscore" ))) )
			{
				if( kp->value.type == Q3F_TYPE_STRING )
				{
					index = atoi( kp->value.d.strdata );
					G_Q3F_RemString( &kp->value.d.strdata );
					kp->value.d.intdata = index;
					kp->value.type = Q3F_TYPE_INTEGER;
				}
				level.teamScores[ent->mapdata->ownerteam] += kp->value.d.intdata;
				CalculateRanks();	// Update the score
			}
		}
	}
	// Decrease our pulse counter
	if( !activator || (ent->count && !--ent->count) )
	{
		// We've expired.

			// Triger 'expiretarget'
		if(	ent->mapdata->other &&
			(kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "expiretarget" ))) )
			G_Q3F_PropogateTrigger( kp->value.d.keypairarraydata, activator, &trace );

			// Reset the 'allowteams' field of the named entities.
		if( kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "teamset" ) ) )
		{
			for( index = -1; data = G_Q3F_ArrayTraverse( kp->value.d.arraydata, &index ); )
			{
				if( !(targkp = G_Q3F_KeyPairArrayFind( level.targetnameArray, data->d.strdata )) )
					continue;
				targetarray = targkp->value.d.arraydata;
				for( targindex = -1; targdata = G_Q3F_ArrayTraverse( targetarray, &targindex ); )
				{
					target = targdata->d.entitydata;
					if( target->mapdata )
						target->mapdata->team = 0;
					if( target->s.eType == ET_Q3F_HUD )
						G_Q3F_UpdateHUD( target );
				}
			}
		}
		if( kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "nonteamset" ) ) )
		{
			for( index = -1; data = G_Q3F_ArrayTraverse( kp->value.d.arraydata, &index ); )
			{
				if( !(targkp = G_Q3F_KeyPairArrayFind( level.targetnameArray, data->d.strdata )) )
					continue;
				targetarray = targkp->value.d.arraydata;
				for( targindex = -1; targdata = G_Q3F_ArrayTraverse( targetarray, &targindex ); )
				{
					target = targdata->d.entitydata;
					if( target->mapdata )
						target->mapdata->team = 0;
					if( target->s.eType == ET_Q3F_HUD )
						G_Q3F_UpdateHUD( target );
				}
			}
		}

			// Restore the initial 'allowteams' variable if there was one
		kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "intialallowteams" ) );
		ent->mapdata->team = kp ? kp->value.d.intdata : 0;	// We can be triggered again, too.

		ent->mapdata->ownerteam = 0;		// Reset team
		ent->nextthink = 0;					// Prevent firings (irrelevant, with ownerteam=0)
		return;
	}

	ent->nextthink = level.time + ent->wait + ent->random * Q_flrand(-1.0f, 1.0f);
}

void G_Q3F_CommandPointTouch( gentity_t *ent, gentity_t *other, trace_t *trace )
{
	// User has touched this entity...

	int teamnum, teammask, index, targindex;
	q3f_keypair_t *kp, *targkp;
	q3f_array_t *targetarray;
	q3f_data_t *data, *targdata;
	gentity_t *target;

	if( !other->client )
		return;			// Not a client?
	teamnum = other->client->sess.sessionTeam;
	//if( teamnum == Q3F_TEAM_SPECTATOR || teamnum == Q3F_TEAM_FREE )
	if( Q3F_IsSpectator(other->client) || teamnum == Q3F_TEAM_FREE )	// RR2DO2
		return;			// Only owned by a team
	if( teamnum == ent->mapdata->ownerteam )
		return;			// Already owned by this team

	kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "overrideteams" ) );
	teammask = kp ? kp->value.d.intdata : (1 << teamnum);

		// Set teamset teams to only respond to the activator's team
	if( kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "teamset" ) ) )
	{
		for( index = -1; data = G_Q3F_ArrayTraverse( kp->value.d.arraydata, &index ); )
		{
			if( !(targkp = G_Q3F_KeyPairArrayFind( level.targetnameArray, data->d.strdata )) )
				continue;
			targetarray = targkp->value.d.arraydata;
			for( targindex = -1; targdata = G_Q3F_ArrayTraverse( targetarray, &targindex ); )
			{
				target = targdata->d.entitydata;
				if( !target->mapdata )
					target->mapdata = G_Alloc( sizeof(q3f_mapent_t) );
				target->mapdata->team = teammask;
				if( target->s.eType == ET_Q3F_HUD )
					G_Q3F_UpdateHUD( target );
			}
		}
	}
		// Set nonteamset teams to only respond to the activator's team
	if( kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "nonteamset" ) ) )
	{
		for( index = -1; data = G_Q3F_ArrayTraverse( kp->value.d.arraydata, &index ); )
		{
			if( !(targkp = G_Q3F_KeyPairArrayFind( level.targetnameArray, data->d.strdata )) )
				continue;
			targetarray = targkp->value.d.arraydata;
			for( targindex = -1; targdata = G_Q3F_ArrayTraverse( targetarray, &targindex ); )
			{
				target = targdata->d.entitydata;
				if( !target->mapdata )
					target->mapdata = G_Alloc( sizeof(q3f_mapent_t) );
				target->mapdata->team = ~teammask;
				if( target->s.eType == ET_Q3F_HUD )
					G_Q3F_UpdateHUD( target );
			}
		}
	}

		// Stop this ent responding to further touches by the same team.
	ent->mapdata->team = ~teammask;

		// Set up this ent for the current pulse cycle.
	ent->activator = other;
	ent->nextthink = level.time + ent->wait + ent->random * Q_flrand(-1.0f, 1.0f);
	ent->mapdata->waittime = level.time + FRAMETIME;		// Can be retriggered almost instantly
	ent->mapdata->state = Q3F_STATE_INACTIVE;				// Ready to be reactivated immediately
	ent->mapdata->ownerteam = teamnum;
	kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "pulsecount" ) );
	ent->count = kp ? kp->value.d.intdata : 0;
}

void SP_Q3F_func_commandpoint( gentity_t *ent )
{
	// When activated, it sets the allowteams field of all entities listed
	// in 'teamset' to the team of the activator, and executes the '. It then 'pulses' the
	// 'pulsetarget' every 'wait' seconds, for 'pulsecount' times, or
	// indefinitely.

	q3f_keypair_t *kp;
	char *str;

	if( !ent->mapdata ) {
		// No ext data, it's useless.

		G_FreeEntity( ent );
		return;
	}

		// Sort before we can search (won't have been done yet)
	G_Q3F_KeyPairArraySort( ent->mapdata->other );

	if( kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "teamset" ) ) )
	{
		// Split up teamset string into individual entities
		str = kp->value.d.strdata;
		kp->value.d.arraydata = G_Q3F_ProcessStrings( str );
		kp->value.type = Q3F_TYPE_ARRAY;
		G_Q3F_RemString( &str );
	}
	if( kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "nonteamset" ) ) )
	{
		// Split up nonteamset string into individual entities
		str = kp->value.d.strdata;
		kp->value.d.arraydata = G_Q3F_ProcessStrings( str );
		kp->value.type = Q3F_TYPE_ARRAY;
		G_Q3F_RemString( &str );
	}
	if( kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "pulsetarget" ) ) )
	{
		// Split up target string into individual entities
		str = kp->value.d.strdata;
		kp->value.d.keypairarraydata = G_Q3F_ProcessStateString( str );
		kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
		G_Q3F_RemString( &str );
	}
	if( kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "expiretarget" ) ) )
	{
		// Split up target string into individual entities
		str = kp->value.d.strdata;
		kp->value.d.keypairarraydata = G_Q3F_ProcessStateString( str );
		kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
		G_Q3F_RemString( &str );
	}
	if( kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "pulsegive" ) ) )
	{
		// Split up give string into individual entities
		str = kp->value.d.strdata;
		kp->value.d.keypairarraydata = G_Q3F_ProcessGiveString( str );
		kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
		G_Q3F_RemString( &str );
	}
	if( kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "pulsecount" ) ) )
	{
		// Convert to an integer
		str = kp->value.d.strdata;
		kp->value.d.intdata = atoi( str );
		kp->value.type = Q3F_TYPE_INTEGER;
		G_Q3F_RemString( &str );
	}
	if( kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "overrideteams" ) ) )
	{
		// Convert to an integer
		str = kp->value.d.strdata;
		//kp->value.d.intdata = atoi( str );
		kp->value.d.intdata = G_Q3F_ProcessTeamString( str );
		kp->value.type = Q3F_TYPE_INTEGER;
		G_Q3F_RemString( &str );
	}
	if( ent->mapdata->team )
	{
		// Store our 'initialallowteams' variable (so we can restore it later)

		G_Q3F_AddString( &str, "initialallowteams" );
		if( (kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, str )) )
			kp->value.d.intdata = ent->mapdata->team;
		else {
			if( !ent->mapdata->other )
				ent->mapdata->other = G_Q3F_KeyPairArrayCreate();
			G_Q3F_KeyPairArrayAdd( ent->mapdata->other, str, Q3F_TYPE_INTEGER, 0, ent->mapdata->team );
			G_Q3F_KeyPairArraySort( ent->mapdata->other );
		}
		G_Q3F_RemString( &str );
	}

	ent->mapdata->ownerteam	= 0;						// Owner team
	ent->think				= G_Q3F_CommandPointThink;
	ent->touch				= G_Q3F_CommandPointTouch;
	ent->mapdata->statethink = G_Q3F_CommandPointStateThink;

	//if( !ent->wait )
	//	ent->wait = 30;		// Default wait of 30 seconds
	if( ent->wait )
		ent->wait *= 1000;		// Convert into milliseconds
	ent->random	*= 1000;

	if( ent->model )
	{
		ent->r.contents		= CONTENTS_TRIGGER;			// We want to know if it's touched
		ent->s.eType		= ET_GENERAL;				// And it's just, well, general ;)
		ent->physicsObject	= qfalse;
		ent->s.modelindex	= G_ModelIndex( ent->model );
		ent->use			= 0;

		G_SpawnVector( "mins", "-15 -15 -15",	ent->r.mins );	// This happens to be ITEM_RADIUS
		G_SpawnVector( "maxs", "15 15 15",		ent->r.maxs );

		VectorCopy( ent->s.angles, ent->s.apos.trBase );

		trap_LinkEntity( ent );
	}
	ent->s.apos.trTime = TR_STATIONARY;
	ent->s.otherEntityNum = MAX_CLIENTS;			// 'not held'.
}

/*
**	HUD icon.
*/

#define	Q3F_NUM_HUDPLAYERS	32
#define	HUDCOPY(w,x,y,z) if(z<Q3F_NUM_HUDPLAYERS){*(y+(char *)x) = *w++; z++;}
static void PackHudData( gentity_t *hud, unsigned char *ptr )
{
	// Pack the HUD data into the entitystate (macro is to prevent overflows)
	int size, temp;

	size = 0;
	HUDCOPY( ptr, &hud->s.time2,			0, size )
	HUDCOPY( ptr, &hud->s.time2,			1, size )
	HUDCOPY( ptr, &hud->s.time2,			2, size )
	HUDCOPY( ptr, &hud->s.time2,			3, size )

	temp = 0;
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	hud->s.origin[0] = temp;
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	hud->s.origin[1] = temp;
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	hud->s.origin[2] = temp;

	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	hud->s.origin2[0] = temp;
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	hud->s.origin2[1] = temp;
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	hud->s.origin2[2] = temp;

	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	hud->s.angles[0] = temp;
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	hud->s.angles[1] = temp;
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	hud->s.angles[2] = temp;

	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	hud->s.angles2[0] = temp;
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	hud->s.angles2[1] = temp;
	HUDCOPY( ptr, &temp,						0, size );
	HUDCOPY( ptr, &temp,						1, size );
	hud->s.angles2[2] = temp;

	HUDCOPY( ptr, &hud->s.powerups,				0, size )
	HUDCOPY( ptr, &hud->s.powerups,				1, size )
	HUDCOPY( ptr, &hud->s.otherEntityNum,		0, size )
	HUDCOPY( ptr, &hud->s.otherEntityNum2,		0, size )
//	HUDCOPY( ptr, &hud->s.modelindex2,			0, size )
	HUDCOPY( ptr, &hud->s.legsAnim,				0, size )
	HUDCOPY( ptr, &hud->s.torsoAnim,			0, size )
}

void G_Q3F_UpdateHUD( gentity_t *ent )
{
	//	Update this HUD icon

	q3f_keypair_t *kp;
	int team, index, count;
	unsigned char *ptr, playerbuff[Q3F_NUM_HUDPLAYERS];
	gentity_t *player;
	q3f_mapent_t *mapdata;

	mapdata = ent->mapdata;

	// djbob: new timer hud icon
	if( ent->s.eFlags & EF_VOTED) {
		int secs, mins;

		secs = ent->count;
		mins = secs / 60;
		secs %= 60;

		ent->s.modelindex =		secs;
		ent->s.constantLight =	mins;
	}
	else {
		// Set model
		kp = G_Q3F_KeyPairArrayFind( mapdata->other, G_Q3F_GetString( va( "%s_model", q3f_statestrings[mapdata->state] ) ) );
		if( kp ) {
			ent->model = kp->value.d.strdata;
			ent->s.modelindex = G_ModelIndex( ent->model );
			ent->s.eFlags &= ~EF_TALK;
		}
		else {
			kp = G_Q3F_KeyPairArrayFind( mapdata->other, G_Q3F_GetString( va( "%s_shader", q3f_statestrings[mapdata->state] ) ) );
			if( kp ) {
				ent->model = kp->value.d.strdata;
				ent->s.modelindex = G_ShaderIndex( ent->model );
				ent->s.eFlags |= EF_TALK;
			}
		}
	}

		// Set colour
	kp = G_Q3F_KeyPairArrayFind( mapdata->other, G_Q3F_GetString( "color" ) );
	if( kp )
		sscanf( kp->value.d.strdata, "%f %f %f", &ent->s.apos.trDelta[0], &ent->s.apos.trDelta[1], &ent->s.apos.trDelta[2] );
	else {
		// Set colour from a team, if possible
		for( team = mapdata->team, count = index = -1; team; team >>= 1, index++ ) {
			if( team & 1 )
				count++;
		}
		if( count == 1 ) {
			team = index;			// Single team allowed, set colour to this
		} else {
			team =	((mapdata->state == Q3F_STATE_ACTIVE || mapdata->state == Q3F_STATE_CARRIED) && mapdata->ownerteam) ? mapdata->ownerteam : Q3F_TEAM_SPECTATOR;
		}
		VectorCopy( *g_q3f_teamlist[team].color, ent->s.origin2 );
	}

		// Set who is allowed to see it
	ent->s.modelindex2 = mapdata->team
							? mapdata->team
							: ~0;
//							: ((mapdata->ownerteam && (mapdata->state == Q3F_STATE_ACTIVE || mapdata->state == Q3F_STATE_CARRIED))
//								? (1 << mapdata->ownerteam)
//								: ~0 );
	ptr = playerbuff;
	memset( playerbuff, 0, sizeof(playerbuff) );
	if( mapdata->holding || mapdata->notholding ) {
		// Only show to 'holding' clients
		index = Q3F_NUM_HUDPLAYERS;
		memset( ptr, 0xFE, index );		// Clients only reach 0x7F
		for( player = g_entities; player < &g_entities[level.maxclients] && index; player++ ) {
			if(	player->inuse && !Q3F_IsSpectator( player->client ) &&
				(ent->s.modelindex2 & (1 << player->client->sess.sessionTeam)) &&
				(!mapdata->holding || G_Q3F_CheckHeld( player, mapdata->holding )) &&
				(!mapdata->notholding || G_Q3F_CheckNotHeld( player, mapdata->notholding )) ) {
				*ptr++ = (char) (player->s.number & 0x7F);
				index--;
			}
		}
	}
	else {
		*ptr = 0xFF;		// Special 'show to all' token
	}
	PackHudData( ent, playerbuff );
}

void G_Q3F_UpdateHUDIcons()
{
	// Update all HUD icons

	gentity_t *hud;

	for( hud = level.hudHead; hud; hud = hud->chain )
		G_Q3F_UpdateHUD( hud ); 
}

static void G_Q3F_HUDTouch( gentity_t *ent, gentity_t *other, trace_t *trace )
{
	// Set owner team prior to update

	if( other->client && !Q3F_IsSpectator( other->client ) )
		ent->mapdata->ownerteam = other->client->sess.sessionTeam;
	G_Q3F_UpdateHUD( ent );
}

void G_PrecacheHud( gentity_t *ent )
{
	q3f_keypair_t *kp;
	q3f_mapent_t *mapdata;
	int index;

	mapdata = ent->mapdata;

	for( index = 0; index < Q3F_NUM_STATES; index++ )
	{
		kp = G_Q3F_KeyPairArrayFind( mapdata->other, G_Q3F_GetString( va( "%s_model", q3f_statestrings[index] ) ) );
		if( kp ) {
			G_ModelIndex( kp->value.d.strdata );
		}
		kp = G_Q3F_KeyPairArrayFind( mapdata->other, G_Q3F_GetString( va( "%s_shader", q3f_statestrings[index] ) ) );
		if( kp ) {
			G_ShaderIndex( kp->value.d.strdata );
		}
	}
}

void SP_Q3F_func_hud( gentity_t *ent )
{
	// Spawn a new HUD icon.

	q3f_keypair_t *kp;
	char *str;

	if( !ent->mapdata )		// No mapdata means no ent, effectively
	{
		G_FreeEntity( ent );
		return;
	}

	ent->r.svFlags	|= SVF_BROADCAST;	// All clients (potentially) need to know
	ent->s.eType	= ET_Q3F_HUD;
	ent->touch		= G_Q3F_HUDTouch;

	kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "slot" ) );
	if( kp )
		ent->s.weapon = atoi( kp->value.d.strdata );
	if( ent->s.weapon < 1 || ent->s.weapon > Q3F_SLOT_MAX )
		G_Error( "func_hud with no/invalid slot specified." );
	ent->s.weapon--;		// The actual slots start at zero

	kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "scale" ) );
	if( kp )
		ent->s.apos.trBase[0] = atof( kp->value.d.strdata );
	else {
		ent->s.apos.trBase[0] = 1;
	}

		// Turn _color (from radiant colour picker) to color.
	kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, str = G_Q3F_GetString( "_color" ) );
	if( kp )
	{
		G_Q3F_AddString( &kp->key, "color" );
		G_Q3F_RemString( &str );
	}

/*	if( ent->mapdata->holding )
	{
		// We don't want 'holding' to affect the triggering of the hud ent,
		// so we remove it from the criteria and re-add it as a kp.

		G_Q3F_KeyPairArrayAdd( ent->mapdata->other, "holding", Q3F_TYPE_ARRAY, 0, (int) ent->mapdata->holding );
		G_Q3F_KeyPairArraySort( ent->mapdata->other );
		ent->mapdata->holding = NULL;
	}*/

	ent->chain = level.hudHead;
	level.hudHead = ent;

	ent->classname = "func_hud";

	G_PrecacheHud( ent );

	G_Q3F_UpdateHUD( ent );

	trap_LinkEntity( ent );
}

/*
**	Create CTF compatability entities
*/

char *G_AddSpawnVarToken( const char *string );
void G_SpawnGEntityFromSpawnVars( qboolean fromBSP, gentity_t *usethisent );
static void AddCTFSpawnVar( char *key, char *value )
{
	level.spawnVars[level.numSpawnVars][0] = G_AddSpawnVarToken( key );
	level.spawnVars[level.numSpawnVars][1] = G_AddSpawnVarToken( value );
	level.numSpawnVars++;
}
static void SpawnCTFCompatibilityFlag( gentity_t *ent )
{
	// Generate the required entities for CTF flags.

	qboolean isred;
	vec3_t origin;
	vec3_t angle;
	char *teamstr, *nonteamstr;

	isred = ent->s.otherEntityNum;
	VectorCopy( ent->s.origin, origin );
	VectorCopy( ent->s.angles, angle );
	teamstr = isred ? "red" : "blue";
	nonteamstr = isred ? "blue" : "red";

		// Add flag
	level.numSpawnVars = level.numSpawnVarChars = 0;
	AddCTFSpawnVar( "classname",			"func_flag" );
	AddCTFSpawnVar( "origin",				va( "%f %f %f", origin[0], origin[1], origin[2] ) );
	AddCTFSpawnVar( "angle",				va( "%f %f %f", angle[0], angle[1], angle[2] ) );
	AddCTFSpawnVar( "model",				va( "models/flags/%s_flag.md3", isred ? "r" : "b" ) );
	AddCTFSpawnVar( "groupname",			va( "CTFCompat_%s_flag", teamstr ) );
	AddCTFSpawnVar( "allowteams",			nonteamstr );
	AddCTFSpawnVar( "wait",					"30" );
	AddCTFSpawnVar( "carried_all_message",	va( "~%%N has TAKEN the %s flag!", isred ? "^1RED^*" : "^4BLUE^*" ) );
	AddCTFSpawnVar( "carried_message",		va( "~You have the %s flag!", isred ? "^1RED^*" : "^4BLUE^*" ) );
	AddCTFSpawnVar( "active_all_message",	va( "~%%N has DROPPED %s flag!", isred ? "^1RED^*" : "^4BLUE^*" ) );
	AddCTFSpawnVar( "inactive_all_message",	va( "The %s flag has returned.", isred ? "^1RED^*" : "^4BLUE^*" ) );
//	AddCTFSpawnVar( "failtarget",			va( "CTFCompat_%s_returner", teamstr ) );	//Commented out by BirdDawg
	AddCTFSpawnVar( "flags",				"revealagent,showcarry" );
	AddCTFSpawnVar( "carried_nonteam_flaginfo",	va( "%%N has the %s flag.", isred ? "^1RED^*" : "^4BLUE^*" ) );
	AddCTFSpawnVar( "carried_team_flaginfo",	va( "%%N has the %s flag at $l.", isred ? "^1RED^*" : "^4BLUE^*" ) );
	AddCTFSpawnVar( "active_flaginfo",		va( "The %s flag has been dropped at $l.", isred ? "^1RED^*" : "^4BLUE^*" ) );
	AddCTFSpawnVar( "inactive_flaginfo",	va( "The %s flag is at the %s base.", isred ? "^1RED^*" : "^4BLUE^*" , teamstr) );

	AddCTFSpawnVar( "activetarget",		va( "CTFCompat_%s_status_indicator=~active", teamstr ) );
	AddCTFSpawnVar( "inactivetarget",	va( "CTFCompat_%s_flaggrab=~inactive,CTFCompat_%s_status_indicator=~inactive", teamstr, teamstr ) );
	AddCTFSpawnVar( "carriedtarget",	va( "CTFCompat_%s_flaggrab,CTFCompat_%s_status_indicator=~carried", teamstr, teamstr ) );	
	
	AddCTFSpawnVar( "carried_nonteam_sound",	"~sound/teamplay/voc_enemy_flag.wav" );
	AddCTFSpawnVar( "carried_team_sound",		"~sound/teamplay/voc_team_flag.wav" );
	AddCTFSpawnVar( "carried_sound",			"~sound/teamplay/voc_you_flag.wav" );
	AddCTFSpawnVar( "inactive_nonteam_sound",	"~sound/teamplay/flagreturn_opponent.wav" );
	AddCTFSpawnVar( "inactive_team_sound",		"~sound/teamplay/flagreturn_yourteam.wav" );
	
	AddCTFSpawnVar( "color" ,					isred ? "1 0 0" : "0 0 1"				);
	AddCTFSpawnVar( "sparkle" ,					isred ? "1 0 0" : "0 0 1"				);
	AddCTFSpawnVar( "light",					"150"									);
	G_SpawnGEntityFromSpawnVars( qfalse, NULL );

		// Add flaggrab
	level.numSpawnVars = level.numSpawnVarChars = 0;
	AddCTFSpawnVar( "classname",			"info_notnull" );
	AddCTFSpawnVar( "origin",				va( "%f %f %f", origin[0], origin[1], origin[2] ) );
	AddCTFSpawnVar( "groupname",			va( "CTFCompat_%s_flaggrab", teamstr ) );
	AddCTFSpawnVar( "wait",					".25" );
	AddCTFSpawnVar( "give",					"score=+2" );
	AddCTFSpawnVar( "active_message",		"You scored 2 frags for taking the enemy flag!" );
	AddCTFSpawnVar( "activetarget",			va( "CTFCompat_%s_flaggrab=~disabled", teamstr ) );
	G_SpawnGEntityFromSpawnVars( qfalse, NULL );

/*		// Add returner
	level.numSpawnVars = level.numSpawnVarChars = 0;
	AddCTFSpawnVar( "classname",			"info_notnull" );
	AddCTFSpawnVar( "groupname",			va( "CTFCompat_%s_returner", teamstr ) );
	AddCTFSpawnVar( "allowteams",			teamstr );
	AddCTFSpawnVar( "checkstate",			va( "CTFCompat_%s_flag=active", teamstr ) );
	AddCTFSpawnVar( "activetarget",			va( "CTFCompat_%s_flag=inactive", teamstr ) );
	G_SpawnGEntityFromSpawnVars( qfalse, NULL );

	Commented out by BirdDawg
*/

		// Add capture point
	level.numSpawnVars = level.numSpawnVarChars = 0;
	AddCTFSpawnVar( "classname",			"func_goalinfo" );
	AddCTFSpawnVar( "origin",				va( "%f %f %f", origin[0], origin[1], origin[2] ) );
	AddCTFSpawnVar( "wait",					".5" );
	AddCTFSpawnVar( "holding",				va( "CTFCompat_%s_flag", nonteamstr ) );
//	AddCTFSpawnVar( "checkstate",			va( "CTFCompat_%s_flag=inactive", teamstr ) );	//Commented out by BirdDawg
	AddCTFSpawnVar( "active_all_message",	va( "~%%N has CAPTURED the %s flag!", isred ? "^1RED^*" : "^4BLUE^*" ) );
	AddCTFSpawnVar( "active__message",		"You have scored 2 frags for capturing the flag." );
	AddCTFSpawnVar( "activetarget",			va( "CTFCompat_%s_flag=~inactive", nonteamstr ) );
	AddCTFSpawnVar( "teamscore",			"10" );
	//birddawg's additions
	AddCTFSpawnVar( "give",					"score=+2,health=+100,armor=+300,ammo_shells=+200,ammo_nails=+200,ammo_cells=+200,ammo_rockets=+200,ammo_medikit=+100,ammo_charge=+1,gren1=+4,gren2=+2,stun=~0,gas=~0,flash=~0,tranq=~0,fire=~0" );
	AddCTFSpawnVar( "active_all_sound",		va( "~sound/teamplay/voc_%s_scores.wav", teamstr ) );
	AddCTFSpawnVar( "active_nonteam_sound",	"~sound/teamplay/flagcapture_opponent.wav" );
	AddCTFSpawnVar( "active_team_sound",	"~sound/teamplay/flagcapture_yourteam.wav" );
	AddCTFSpawnVar( "active_sound",			"~sound/teamplay/flagcapture_yourteam.wav" );

	G_SpawnGEntityFromSpawnVars( qfalse, NULL );

		// Add HUD entity
	level.numSpawnVars = level.numSpawnVarChars = 0;
	AddCTFSpawnVar( "classname",			"func_hud" );
	AddCTFSpawnVar( "holding",				va( "CTFCompat_%s_flag", teamstr ) );
	AddCTFSpawnVar( "slot",					"1" );
	AddCTFSpawnVar( "inactive_model",		va( "models/flags/%s_flag.md3", isred ? "r" : "b" ) );
	G_SpawnGEntityFromSpawnVars( qfalse, NULL );

	//BirdDawg - add flag indicators
	level.numSpawnVars = level.numSpawnVarChars = 0;
	AddCTFSpawnVar( "classname",			"func_hud" );
	AddCTFSpawnVar( "groupname",			va ( "CTFCompat_%s_status_indicator" , teamstr));
	AddCTFSpawnVar( "slot",					isred ? "3" : "4" );
	AddCTFSpawnVar( "carried_shader",		va ( "textures/etf_hud/%s_lost" , teamstr ));
	AddCTFSpawnVar( "active_shader",		va ( "textures/etf_hud/%s_down" , teamstr ));
	AddCTFSpawnVar( "inactive_shader",		va ( "textures/etf_hud/%s_safe" , teamstr ));
	AddCTFSpawnVar( "scale",				".5" );
	G_SpawnGEntityFromSpawnVars( qfalse, NULL );

	G_FreeEntity( ent );
}

void SP_Q3F_CTF_redflag( gentity_t *ent )
{
//	SpawnCTFCompatabilityFlag( qtrue, ent->s.origin );
//	G_FreeEntity( ent );
	ent->s.otherEntityNum = 1;
	ent->think = SpawnCTFCompatibilityFlag;
	ent->nextthink = level.time + FRAMETIME;
	level.ctfcompat = qtrue;
}

void SP_Q3F_CTF_blueflag( gentity_t *ent )
{
//	SpawnCTFCompatabilityFlag( qfalse, ent->s.origin );
//	G_FreeEntity( ent );
	ent->s.otherEntityNum = 0;
	ent->think = SpawnCTFCompatibilityFlag;
	ent->nextthink = level.time + FRAMETIME;
	level.ctfcompat = qtrue;
}


//BirdDawg
static void SpawnCTFAmmoConversion ( gentity_t * ent )
{
	vec3_t origin;
	char* classname = ent->classname;
	VectorCopy( ent->s.origin, origin );

	level.numSpawnVars = level.numSpawnVarChars = 0;

	AddCTFSpawnVar ( "classname" , "func_goalinfo" );
	AddCTFSpawnVar ( "origin" , va ( "%f %f %f" , origin[0] , origin[1] , origin [2] ));
	AddCTFSpawnVar ( "flags" , "rotating,hideactive" );
	AddCTFSpawnVar ( "wait" , "10" );
	AddCTFSpawnVar ( "inactive_all_sound" , "sound/items/respawn1.wav" );
	AddCTFSpawnVar ( "active_all_sound" , "sound/misc/am_pkup.wav" );

	//-------------------------------
	if ( !Q_stricmp ( classname , "ammo_bfg" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "gren1=+1" );
		AddCTFSpawnVar ( "model" , "models/objects/backpack/backpack_small.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/backpack" );
		AddCTFSpawnVar ( "clientstats" , "gren1<!" );
	}
	//-------------------------------
	else if ( !Q_stricmp ( classname , "ammo_bullets" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "ammo_nails=+50" );
		AddCTFSpawnVar ( "model" , "models/powerups/ammo/machinegunam.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/icona_machinegun" );
		AddCTFSpawnVar ( "clientstats" , "ammo_nails<!" );
	}
	//-------------------------------
	else if ( !Q_stricmp ( classname , "ammo_nails" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "ammo_nails=+50" );
		AddCTFSpawnVar ( "model" , "models/powerups/ammo/machinegunam.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/icona_machinegun" );
		AddCTFSpawnVar ( "clientstats" , "ammo_nails<!" );
	}
	//-------------------------------
	else if ( !Q_stricmp ( classname , "ammo_cells" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "ammo_cells=+50" );
		AddCTFSpawnVar ( "model" , "models/powerups/ammo/plasmaam.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/icona_plasma" );
		AddCTFSpawnVar ( "clientstats" , "ammo_cells<!" );
	}
	//-------------------------------
	else if ( !Q_stricmp ( classname , "ammo_grenades" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "gren1=+1" );
		AddCTFSpawnVar ( "model" , "models/objects/backpack/backpack_small.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/backpack" );
		AddCTFSpawnVar ( "clientstats" , "ammo_gren1<!" );
	}
	//-------------------------------
	else if ( !Q_stricmp ( classname , "ammo_lightning" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "ammo_medikit=+100" );
		AddCTFSpawnVar ( "model" , "models/objects/backpack/backpack.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/icona_lightning" );
		AddCTFSpawnVar ( "clientstats" , "ammo_medikit<!" );
	}
	//-------------------------------
	else if ( !Q_stricmp ( classname , "ammo_rockets" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "ammo_rockets=+10" );
		AddCTFSpawnVar ( "model" , "models/powerups/ammo/rocketam.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/icona_rocket" );
		AddCTFSpawnVar ( "clientstats" , "ammo_rockets<!" );
	}
	//-------------------------------
	else if ( !Q_stricmp ( classname , "ammo_shells" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "ammo_shells=+10" );
		AddCTFSpawnVar ( "model" , "models/powerups/ammo/shotgunam.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/icona_shotgun" );
		AddCTFSpawnVar ( "clientstats" , "ammo_shells<!" );
	}
	//-------------------------------
	else if ( !Q_stricmp ( classname , "ammo_slugs" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "ammo_nails=+10" );
		AddCTFSpawnVar ( "model" , "models/powerups/ammo/machinegunam.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/icona_machinegun" );
		AddCTFSpawnVar ( "clientstats" , "ammo_nails<!" );
	}
	//-------------------------------
	if ( !Q_stricmp ( classname , "weapon_bfg" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "gren2=+2" );
		AddCTFSpawnVar ( "model" , "models/objects/backpack/backpack_small.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/backpack" );
		AddCTFSpawnVar ( "clientstats" , "gren2<!" );
	}
	//-------------------------------
	if ( !Q_stricmp ( classname , "weapon_grenadelauncher" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "gren1=+2" );
		AddCTFSpawnVar ( "model" , "models/objects/backpack/backpack_small.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/backpack" );
		AddCTFSpawnVar ( "clientstats" , "gren1<!" );
	}
	//-------------------------------
	if ( !Q_stricmp ( classname , "weapon_lightning" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "ammo_medikit=+200" );
		AddCTFSpawnVar ( "model" , "models/objects/backpack/backpack.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/backpack" );
		AddCTFSpawnVar ( "clientstats" , "ammo_medikit<!" );
	}
	//-------------------------------
	if ( !Q_stricmp ( classname , "weapon_machinegun" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "ammo_nails=+150" );
		AddCTFSpawnVar ( "model" , "models/powerups/ammo/machinegunam.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/icona_machinegun" );
		AddCTFSpawnVar ( "clientstats" , "ammo_nails<!" );
	}
	//-------------------------------
	if ( !Q_stricmp ( classname , "weapon_railgun" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "ammo_nails=+50" );
		AddCTFSpawnVar ( "model" , "models/powerups/ammo/machinegunam.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/icona_machinegun" );
		AddCTFSpawnVar ( "clientstats" , "ammo_nails<!" );
	}
	//-------------------------------
	else if ( !Q_stricmp ( classname , "weapon_plasmagun" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "ammo_cells=+200" );
		AddCTFSpawnVar ( "model" , "models/powerups/ammo/plasmaam.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/icona_plasma" );
		AddCTFSpawnVar ( "clientstats" , "ammo_cells<!" );
	}
	//-------------------------------
	else if ( !Q_stricmp ( classname , "weapon_rocketlauncher" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "ammo_rockets=+50" );
		AddCTFSpawnVar ( "model" , "models/powerups/ammo/rocketam.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/icona_rocket" );
		AddCTFSpawnVar ( "clientstats" , "ammo_rockets<!" );
	}
	//-------------------------------
	else if ( !Q_stricmp ( classname , "weapon_shotgun" ) )
	//-------------------------------
	{
		AddCTFSpawnVar ( "give" , "ammo_shells=+50" );
		AddCTFSpawnVar ( "model" , "models/powerups/ammo/shotgunam.md3" );
		AddCTFSpawnVar ( "simpleshader" , "icons/icona_shotgun" );
		AddCTFSpawnVar ( "clientstats" , "ammo_shells<!" );
	}

	G_SpawnGEntityFromSpawnVars( qfalse, NULL );
	G_FreeEntity( ent );
}

void SP_Q3F_CTF_AmmoConversion ( gentity_t * ent )
{
	ent->think = SpawnCTFAmmoConversion;
	ent->nextthink = level.time + FRAMETIME;
	level.ctfcompat = qtrue;
}

static void SpawnCTFCompatibilityStart( gentity_t *ent )
{
	qboolean isred;
	vec3_t origin;
	vec3_t angle;

	isred = ent->s.otherEntityNum;
	VectorCopy( ent->s.origin, origin );
	VectorCopy( ent->s.angles, angle );

	level.numSpawnVars = level.numSpawnVarChars = 0;
	AddCTFSpawnVar( "classname",			"info_player_deathmatch" );
	AddCTFSpawnVar( "origin",				va( "%f %f %f", origin[0], origin[1], origin[2] ) );
	AddCTFSpawnVar( "angle",				va( "%f", angle[1] ));
	AddCTFSpawnVar( "allowteams",			isred ? "red" : "blue" );
	AddCTFSpawnVar( "give",					"health=+100,armor=+300,ammo_shells=+200,ammo_nails=+200,ammo_rockets=+200,ammo_cells=+200,ammo_medikit=+100,gren1=+4" );

	G_SpawnGEntityFromSpawnVars( qfalse, NULL );

	G_FreeEntity( ent );
}

void SP_Q3F_team_CTF_redplayer( gentity_t *ent )
{
		// Add player spawn
	ent->s.otherEntityNum = 1;
	ent->think = SpawnCTFCompatibilityStart;
	ent->nextthink = level.time + FRAMETIME;
	level.ctfcompat = qtrue;
}

void SP_Q3F_team_CTF_blueplayer( gentity_t *ent )
{
		// Add player spawn
	ent->s.otherEntityNum = 0;
	ent->think = SpawnCTFCompatibilityStart;
	ent->nextthink = level.time + FRAMETIME;
	level.ctfcompat = qtrue;
}

void SP_Q3F_item_flagreturn_team1( gentity_t *ent )
{
	// WFA compatability ent - acts as a flag capture point

	level.wfacompat = qtrue;

	level.numSpawnVars = level.numSpawnVarChars = 0;
	AddCTFSpawnVar( "classname",			"func_goalinfo" );
	AddCTFSpawnVar( "groupname",			"WFACompat_item_flagreturn_team1" );
	AddCTFSpawnVar( "origin",				va( "%f %f %f", ent->s.pos.trBase[0], ent->s.pos.trBase[1], ent->s.pos.trBase[2] ) );
	AddCTFSpawnVar( "holding",				"CTFCompat_blue_flag" );
	AddCTFSpawnVar( "active_all_message",	"%N captured the blue flag!" );
	AddCTFSpawnVar( "activetarget",			"CTFCompat_blue_flag=~inactive" );
	AddCTFSpawnVar( "teamscore",			"1" );
	G_SpawnGEntityFromSpawnVars( qfalse, NULL );

	G_FreeEntity( ent );
}

void SP_Q3F_item_flagreturn_team2( gentity_t *ent )
{
	// WFA compatability ent - acts as a flag capture point

	level.wfacompat = qtrue;

	level.numSpawnVars = level.numSpawnVarChars = 0;
	AddCTFSpawnVar( "classname",			"func_goalinfo" );
	AddCTFSpawnVar( "groupname",			"WFACompat_item_flagreturn_team2" );
	AddCTFSpawnVar( "origin",				va( "%f %f %f", ent->s.pos.trBase[0], ent->s.pos.trBase[1], ent->s.pos.trBase[2] ) );
	AddCTFSpawnVar( "holding",				"CTFCompat_red_flag" );
	AddCTFSpawnVar( "active_all_message",	"%N captured the red flag!" );
	AddCTFSpawnVar( "activetarget",			"CTFCompat_red_flag=~inactive" );
	AddCTFSpawnVar( "teamscore",			"1" );
	G_SpawnGEntityFromSpawnVars( qfalse, NULL );

	G_FreeEntity( ent );
}

void SP_Q3F_item_pack( gentity_t *ent )
{
	// WFA compatability ent - acts as a refill

	level.numSpawnVars = level.numSpawnVarChars = 0;
	AddCTFSpawnVar( "classname",			"func_goalinfo" );
	AddCTFSpawnVar( "origin",				va( "%f %f %f", ent->s.pos.trBase[0], ent->s.pos.trBase[1], ent->s.pos.trBase[2] ) );
	AddCTFSpawnVar( "wait",					"5" );
	AddCTFSpawnVar( "model",				"models/objects/backpack/backpack.md3" );
	AddCTFSpawnVar( "give",					"ammo_nails=+100,ammo_rockets=+10,ammo_cells=+100,ammo_shells=+50,armor=+300,health=+100,gren1=+1" );
	AddCTFSpawnVar( "flags",				"hideactive,rotating" );
	G_SpawnGEntityFromSpawnVars( qfalse, NULL );

	G_FreeEntity( ent );
}

void G_Q3F_CTFCompatAdjust()
{
	// Perform any adjustments required for CTF/WFA maps
	// This is no longer needed, since it doesn't import
#if 0
	char *ptr;
	q3f_array_t *array;
	q3f_data_t *data;
	q3f_keypair_t *kp;
	q3f_keypairarray_t *kpa;
	int index;

	if( level.wfacompat )
	{
		// Remove failtarget on red flag (so it doesn't return when touched)
		if(	(ptr = G_Q3F_GetString( "WFACompat_item_flagreturn_team1" )) &&
			(kp = G_Q3F_KeyPairArrayFind( level.targetnameArray, ptr )) &&
			kp->value.d.arraydata->used )
		{
			ptr = G_Q3F_GetString( "CTFCompat_red_flag" );
			if( ptr )
			{
				kp = G_Q3F_KeyPairArrayFind( level.targetnameArray, ptr );
				index = -1;
				array = kp->value.d.arraydata;
				data = G_Q3F_ArrayTraverse( array, &index );
				if( data )
				{
					kpa = data->d.entitydata->mapdata->failtarget;
					data->d.entitydata->mapdata->failtarget = NULL;
					G_Q3F_KeyPairArrayDestroy( kpa );
				}
			}
		}

		// Remove failtarget on blue flag (so it doesn't return when touched)
		if(	(ptr = G_Q3F_GetString( "WFACompat_item_flagreturn_team2" )) &&
			(kp = G_Q3F_KeyPairArrayFind( level.targetnameArray, ptr )) &&
			kp->value.d.arraydata->used )
		{
			ptr = G_Q3F_GetString( "CTFCompat_blue_flag" );
			if( ptr )
			{
				kp = G_Q3F_KeyPairArrayFind( level.targetnameArray, ptr );
				index = -1;
				array = kp->value.d.arraydata;
				data = G_Q3F_ArrayTraverse( array, &index );
				if( data )
				{
					kpa = data->d.entitydata->mapdata->failtarget;
					data->d.entitydata->mapdata->failtarget = NULL;
					G_Q3F_KeyPairArrayDestroy( kpa );
				}
			}
		}
	}
#endif
}

/* Ensiform - Fixes the misc_beam on blue base not returning back on after blueflag returns to base */
void G_Q3F_MuonFix( void )
{
	// Add info_notnull
	level.numSpawnVars = level.numSpawnVarChars = 0;
	AddCTFSpawnVar("classname", "info_notnull");
	AddCTFSpawnVar("origin", "-1316 2944 844");
	AddCTFSpawnVar("gameindex", "1,3");
	AddCTFSpawnVar("groupname", "blue_flag_arc_activate");
	AddCTFSpawnVar("inactivetarget", "blue_flag_arc=~inactive,blue_flag_checker");
	AddCTFSpawnVar("spawnflags", "1");
	AddCTFSpawnVar("wait", "1");

	G_SpawnGEntityFromSpawnVars(qfalse, NULL);
}

/* Ensiform - Fixes the spectator bug & adds line of sight */
void G_Q3F_OdiumFix( void )
{
	gentity_t *ent;
	ent = G_Find (NULL, FOFS(classname), "func_explosion");
	if (ent) {
		G_FreeEntity( ent );

		// Add func_explosion
		level.numSpawnVars = level.numSpawnVarChars = 0;
		AddCTFSpawnVar("classname", "func_explosion");
		AddCTFSpawnVar("origin", "3072 0 524");
		//AddCTFSpawnVar("origin", "3072 0 732");
		AddCTFSpawnVar("give", "health=-200");
		AddCTFSpawnVar("targetname", "explosion1");
		AddCTFSpawnVar("effectradius", "10000");
		AddCTFSpawnVar("radius", "200");
		AddCTFSpawnVar("script", "spirit/explosions/explosion_he.spirit");
		AddCTFSpawnVar("affectteams", "red,blue");
		AddCTFSpawnVar("flags", "lineofsight");
		//AddCTFSpawnVar("dmg", "200");

		G_SpawnGEntityFromSpawnVars(qfalse, NULL);
	}

	ent = G_Find(NULL, FOFS(classname), "target_speaker");
	if (ent) {
		G_FreeEntity( ent );

		// Add target_speaker
		level.numSpawnVars = level.numSpawnVarChars = 0;
		AddCTFSpawnVar("classname", "target_speaker");
		AddCTFSpawnVar("origin", "3072 0 544");
		AddCTFSpawnVar("noise", "sound/weapons/explosive/he_charge.wav");
		AddCTFSpawnVar("spawnflags", "4");
		AddCTFSpawnVar("targetname", "explosion1");

		G_SpawnGEntityFromSpawnVars(qfalse, NULL);
	}
}


/*
** 'Command' entity
*/

void G_Q3F_TargetCommandTouch( gentity_t *ent, gentity_t *other, trace_t *trace )
{
	// Parse the message string, pull out 'magic' commands and process,
	// and ignore any others

	char cmdbuff[MAX_STRING_CHARS];
	char databuff[MAX_STRING_CHARS];
	char *ptr, *endptr;
	int index, spaceindex;

	for( ptr = ent->message; *ptr; )
	{
		for( endptr = ptr, index = spaceindex = 0; *endptr && *endptr != ';'; endptr++)
		{
			if( *endptr == ' ' )
			{
				if( !index )
					continue;
				if( !spaceindex )
					spaceindex = index;
			}
			if( index < sizeof(cmdbuff) - 2 )
				cmdbuff[index++] = *endptr;
		}
		cmdbuff[index] = 0;

		if( *cmdbuff )
		{
			if( !spaceindex )
			{
				spaceindex = index;
				cmdbuff[index+2] = 0;
			}
			cmdbuff[spaceindex++] = 0;

			if( !Q_stricmp( cmdbuff, "nextmap" ) ) {
				// Magic map change
				trap_Cvar_VariableStringBuffer( "nextmap", databuff, sizeof(databuff) );
				trap_SendConsoleCommand( EXEC_APPEND, va(
					(*databuff ? "set nextmap \"map %s; set nextmap %s\"\n" : "set nextmap \"map %s\"\n")
					, &cmdbuff[spaceindex], databuff ) );
			}
			else if( !Q_stricmp( cmdbuff, "intermission" ) ) {
				// Magic intermission command
				LogExit( "Intermission called." );
				trap_SendServerCommand( -1, "print \"Intermission called.\n\"" );
			}
			else if( !Q_stricmp( cmdbuff, "ceasefire" ) ) {
				// Magic ceasefire command

				if(!Q_stricmp(&cmdbuff[spaceindex], "on") || !Q_stricmp(&cmdbuff[spaceindex], "1")) {
					trap_SetConfigstring( CS_FORTS_CEASEFIRE, "1");
					trap_SendServerCommand( -1, "print \"Ceasefire on\n\"" );
					level.ceaseFire = qtrue;
				} else if(!Q_stricmp(&cmdbuff[spaceindex], "off") || !Q_stricmp(&cmdbuff[spaceindex], "0")) {
					trap_SetConfigstring( CS_FORTS_CEASEFIRE, "0");
					trap_SendServerCommand( -1, "print \"Ceasefire off\n\"" );
					level.ceaseFire = qfalse;
				}
			}
			else if( !Q_stricmp( cmdbuff, "map" ) ) {
				// Magic ceasefire command
				trap_SendConsoleCommand( EXEC_APPEND, va(
					(*databuff ? "set nextmap \"map %s; set nextmap %s\"\n" : "set nextmap \"map %s\"\n")
					, &cmdbuff[spaceindex], databuff ) );

				level.mapSelectState = Q3F_MAPSELECT_CHANGEMAP;
				ExitLevel();
			}
			else {
				G_Printf( "target_command: forbidden command \"%s\"\n", cmdbuff );
			}
		}

		ptr = *endptr ? (endptr + 1) : endptr;
	}
}

void SP_Q3F_target_command( gentity_t *ent )
{
	// Allow the mapper to execute arbitrary commands on the server.
	// Well, almost arbitrary :)

	ent->touch = G_Q3F_TargetCommandTouch;
}


/*
**	No Building ent stuff.
**	We offload from ents to reduce the entity count and processing time a little
**	Someday, hopefully, we can have this as a content flag instead of a seperate ent
*/

static q3f_array_t *nobuildarray;
static char *typestrptr;

typedef struct g_q3f_nobuild_s {
	// This should fit neatly into 32 bytes: 3x4 + 3x4 + 4 + 4 = 32;
	vec3_t mins, maxs;
	int teams, flags;
} g_q3f_nobuild_t;

#define	Q3F_NOBUILD_AUTOSENTRY		0x01
#define	Q3F_NOBUILD_SUPPLYSTATION	0x02
#define	Q3F_NOBUILD_CHARGE			0x04

qboolean G_Q3F_NoBuildCheck( vec3_t mins, vec3_t maxs, int team, int mask )
{
	// Check to see if we're within a nobuid area.

	int i;
	g_q3f_nobuild_t *ptr;
	q3f_data_t *data;

	if( !nobuildarray )
		return( qfalse );

	i = -1;
	while( data = G_Q3F_ArrayTraverse( nobuildarray, &i ) )
	{
		ptr = (g_q3f_nobuild_t *) data->d.intdata;
		if(	ptr->mins[0] > maxs[0] ||
			ptr->mins[1] > maxs[1] ||
			ptr->mins[2] > maxs[2] ||
			ptr->maxs[0] < mins[0] ||
			ptr->maxs[1] < mins[1] ||
			ptr->maxs[2] < mins[2] ||
			!(ptr->flags & mask) ||
			!(ptr->teams & (1 << team)) )
			continue;
		return( qtrue );
	}

	return( qfalse );
}

void G_Q3F_NoBuildFinish()
{
	// Just do some cleanup

	G_Q3F_ArrayConsolidate( nobuildarray );
	G_Q3F_RemString( &typestrptr );
}

void SP_Q3F_func_nobuild( gentity_t *ent )
{
	// Add a nobuild entry to the array

	g_q3f_nobuild_t *nobuild;
	q3f_keypair_t *kp;
	char buff[MAX_STRING_CHARS];
	char *ptr, *endptr;

	if( !nobuildarray )
	{
		G_Q3F_AddString( &typestrptr, "type" );
		nobuildarray = G_Q3F_ArrayCreate();
	}

	nobuild = G_Alloc( sizeof(g_q3f_nobuild_t) );
	trap_SetBrushModel( ent, ent->model );
	VectorCopy( ent->r.absmin, nobuild->mins );
	VectorCopy( ent->r.absmax, nobuild->maxs );
	nobuild->teams = (ent->mapdata && ent->mapdata->team) ? ent->mapdata->team : -1;
	nobuild->flags = 0;

	if( ent->mapdata && (kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, typestrptr )) )
	{
		// Check the different types in the bitfield.

		ptr = kp->value.d.strdata;
		memset( buff, 0, sizeof(buff) );
		Q_strncpyz( buff, ptr, sizeof(buff) - 1 );
		for( ptr = buff; *ptr; ptr = endptr + 1 )
		{
			for( endptr = ptr; *endptr && *endptr != ','; endptr++ );
			*endptr = 0;
			if( !Q_stricmp( ptr, "autosentry" ) )
				nobuild->flags |= Q3F_NOBUILD_AUTOSENTRY;
			else if( !Q_stricmp( ptr, "supplystation" ) )
				nobuild->flags |= Q3F_NOBUILD_SUPPLYSTATION;
			else if( !Q_stricmp( ptr, "charge" ) )
				nobuild->flags |= Q3F_NOBUILD_CHARGE;
			else {
				G_Printf( "Unknown flag '%s' in func_nobuild.\n", ptr );
			}
		}
	}

	G_Q3F_ArrayAdd( nobuildarray, Q3F_TYPE_OTHER, 0, (int) nobuild );

	G_FreeEntity( ent );	// We don't really want the ent cluttering the place up anymore
}



/*
**	No Annoying ent stuff.
**	We offload from ents to reduce the entity count and processing time a little
**	Someday, hopefully, we can have this as a content flag instead of a seperate ent
*/

static q3f_array_t *noannoyarray;

qboolean G_Q3F_NoAnnoyCheck( vec3_t mins, vec3_t maxs, int team, int mask )
{
	// Check to see if we're within a noannoy area.

	int i;
	g_q3f_nobuild_t *ptr;
	q3f_data_t *data;

	if( !noannoyarray )
		return( qfalse );

	i = -1;
	while( data = G_Q3F_ArrayTraverse( noannoyarray, &i ) )
	{
		ptr = (g_q3f_nobuild_t *) data->d.intdata;
		if(	ptr->mins[0] > maxs[0] ||
			ptr->mins[1] > maxs[1] ||
			ptr->mins[2] > maxs[2] ||
			ptr->maxs[0] < mins[0] ||
			ptr->maxs[1] < mins[1] ||
			ptr->maxs[2] < mins[2] ||
			!(ptr->flags & mask) ||
			!(ptr->teams & (1 << team)) )
			continue;
		return( qtrue );
	}

	return( qfalse );
}

void G_Q3F_NoAnnoyFinish()
{
	// Just do some cleanup

	G_Q3F_ArrayConsolidate( noannoyarray );
	G_Q3F_RemString( &typestrptr );
}

qboolean levelhasnoannoys = qfalse;

qboolean G_Q3F_MapHasNoAnnoys()
{
	// Useful for making maps that dont use it, less expensive for calling
	if( !noannoyarray )
	{
		levelhasnoannoys = qfalse;
		return qfalse;
	}

	if( noannoyarray->used )
	{
		levelhasnoannoys = qtrue;
		return( qtrue );
	}

	levelhasnoannoys = qfalse;
	return( qfalse );
}

void SP_Q3F_func_noannoyances( gentity_t *ent )
{
	// Add a nobuild entry to the array

	g_q3f_nobuild_t *noannoy;
	q3f_keypair_t *kp;
	char buff[MAX_STRING_CHARS];
	char *ptr, *endptr;

	if( !noannoyarray )
	{
		G_Q3F_AddString( &typestrptr, "type" );
		noannoyarray = G_Q3F_ArrayCreate();
	}

	noannoy = G_Alloc( sizeof(g_q3f_nobuild_t) );
	trap_SetBrushModel( ent, ent->model );
	VectorCopy( ent->r.absmin, noannoy->mins );
	VectorCopy( ent->r.absmax, noannoy->maxs );
	noannoy->teams = (ent->mapdata && ent->mapdata->team) ? ent->mapdata->team : -1;
	noannoy->flags = 0;

	if( ent->mapdata && (kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, typestrptr )) )
	{
		// Check the different types in the bitfield.

		ptr = kp->value.d.strdata;
		memset( buff, 0, sizeof(buff) );
		Q_strncpyz( buff, ptr, sizeof(buff) - 1 );
		for( ptr = buff; *ptr; ptr = endptr + 1 )
		{
			for( endptr = ptr; *endptr && *endptr != ','; endptr++ );
			*endptr = 0;
			if( !Q_stricmp( ptr, "buildings" ) )
				noannoy->flags |= Q3F_NOANNOY_BUILDING;
			else if( !Q_stricmp( ptr, "gasgrens" ) )
				noannoy->flags |= Q3F_NOANNOY_GASGREN;
			else if( !Q_stricmp( ptr, "napalmgrens" ) )
				noannoy->flags |= Q3F_NOANNOY_NAPALMGREN;
			else if( !Q_stricmp( ptr, "nailgrens" ) )
				noannoy->flags |= Q3F_NOANNOY_NAILGREN;
			else if( !Q_stricmp( ptr, "normalgrens" ) && !(noannoy->flags & Q3F_NOANNOY_GRENS) )
				noannoy->flags |= Q3F_NOANNOY_GRENS;
			else if( !Q_stricmp( ptr, "grens" ) && !(noannoy->flags & Q3F_NOANNOY_GRENS) )
				noannoy->flags |= Q3F_NOANNOY_GRENS;
			else if( !Q_stricmp( ptr, "projectiles" ) )
				noannoy->flags |= Q3F_NOANNOY_PROJECTILES;
			else if( !Q_stricmp( ptr, "charges" ) )
				noannoy->flags |= Q3F_NOANNOY_CHARGES;
			else if( !Q_stricmp( ptr, "backpacks" ) )
				noannoy->flags |= Q3F_NOANNOY_BACKPACKS;
			else {
				G_Printf( "Unknown flag '%s' in func_noannoyances.\n", ptr );
			}
		}
	}

	G_Q3F_ArrayAdd( noannoyarray, Q3F_TYPE_OTHER, 0, (int) noannoy );

	G_FreeEntity( ent );	// We don't really want the ent cluttering the place up anymore
}

/*
**	target_cycle entity - When triggered, drops back to inactive immediately,
**	and attempts to trigger the cycletarget as the next user in sequence.
**
*/

static char *cycleallowteamsptr, *cycleallowclassesptr, *cycleholdingptr, *cyclenotholdingptr, *cycleclientstatsptr;
static char *cycletargetptr, *cyclefailtargetptr;
void G_Q3F_TargetCycleTouch( gentity_t *ent, gentity_t *other, trace_t *tr )
{
	// Find the next player who matches the criteria, trigger the target.

	q3f_keypair_t *allowteams, *allowclasses, *holding, *notholding, *clientstats, *cycletarget, *cyclefailtarget;//, *kp;
//	q3f_keypairarray_t *kpa;
	qboolean allowdead, fullcycle;
	int start, curr, last;
	gentity_t *player;//, *target;
	gclient_t *client;

	allowteams		= G_Q3F_KeyPairArrayFind( ent->mapdata->other, cycleallowteamsptr	);
	allowclasses	= G_Q3F_KeyPairArrayFind( ent->mapdata->other, cycleallowclassesptr	);
	holding			= G_Q3F_KeyPairArrayFind( ent->mapdata->other, cycleholdingptr		);
	notholding		= G_Q3F_KeyPairArrayFind( ent->mapdata->other, cyclenotholdingptr	);
	clientstats		= G_Q3F_KeyPairArrayFind( ent->mapdata->other, cycleclientstatsptr	);
	cycletarget		= G_Q3F_KeyPairArrayFind( ent->mapdata->other, cycletargetptr		);
	cyclefailtarget	= G_Q3F_KeyPairArrayFind( ent->mapdata->other, cyclefailtargetptr	);
	allowdead		= ent->mapdata->flags & Q3F_FLAG_ALLOWDEAD;

	start = other->s.number;
	last = (ent->mapdata->flags & Q3F_FLAG_ALLOWSAME) ? ((start + 1) % MAX_CLIENTS) : start;
	for(	fullcycle = qfalse, curr = (start + 1) % MAX_CLIENTS;
			curr != last || !fullcycle;
			curr = (curr + 1) % MAX_CLIENTS )
	{
		fullcycle = qtrue;		// We can now check curr != last without cutting out

		player = &g_entities[curr];
		client = player->client;

		// Connect check is there to prevent trigger by disconnecting player
		// causing an infinite loop, or an invalid clientnum on carried goalitems.
		if( !player->inuse || !client || client->pers.connected != CON_CONNECTED )
			continue;
		if( allowteams && !(allowteams->value.d.intdata & (1 << client->sess.sessionTeam)) )
			continue;
		if( allowclasses && !(allowclasses->value.d.intdata & (1 << client->ps.persistant[PERS_CURRCLASS])) )
			continue;
		if( holding && !G_Q3F_CheckHeld( other, holding->value.d.arraydata ) )
			continue;
		if( notholding && !G_Q3F_CheckNotHeld( other, notholding->value.d.arraydata ) )
			continue;
		if( clientstats && !G_Q3F_CheckClientStats( other, clientstats->value.d.keypairarraydata, qfalse ) )
			continue;
		if( !allowdead && g_entities[curr].health <= 0 )
			continue;

		// We've passed the gauntlet, lets fire off our targets 

		if( cycletarget )
			G_Q3F_PropogateTrigger( cycletarget->value.d.keypairarraydata, player, tr );
		curr = -1;	// Indicate a complete loop
		break;
	}

		// Trigger the failtarget if we failed
	if( cyclefailtarget && curr >= 0 )
		G_Q3F_PropogateTrigger( cyclefailtarget->value.d.keypairarraydata, other, tr );

		// Now trigger ourself back to inactive, ready for another shot.
	G_Q3F_TriggerEntity( ent, other, Q3F_STATE_INACTIVE, tr, qfalse );
}

void SP_Q3F_target_cycle( gentity_t *ent )
{
	q3f_keypair_t *kp;
	char *oldstr;

	if( ent->mapdata )
	{
		ent->touch = G_Q3F_TargetCycleTouch;

		kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, cycleallowteamsptr = G_Q3F_GetString( "cycleallowteams" ) );
		if( kp )
		{
			oldstr = kp->value.d.strdata;
			kp->value.d.intdata = G_Q3F_ProcessTeamString( oldstr );
			kp->value.type = Q3F_TYPE_INTEGER;
			G_Q3F_RemString( &oldstr );
		}
		kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, cycleallowclassesptr = G_Q3F_GetString( "cycleallowclasses" ) );
		if( kp )
		{
			oldstr = kp->value.d.strdata;
			kp->value.d.intdata = G_Q3F_ProcessClassString( oldstr );
			kp->value.type = Q3F_TYPE_INTEGER;
			G_Q3F_RemString( &oldstr );
		}

		kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, cycleholdingptr = G_Q3F_GetString( "cycleholding" ) );
		if( kp )
		{
			oldstr = kp->value.d.strdata;
			kp->value.d.arraydata = G_Q3F_ProcessStrings( oldstr );
			kp->value.type = Q3F_TYPE_ARRAY;
			G_Q3F_RemString( &oldstr );
		}

		kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, cyclenotholdingptr = G_Q3F_GetString( "cyclenotholding" ) );
		if( kp )
		{
			oldstr = kp->value.d.strdata;
			kp->value.d.arraydata = G_Q3F_ProcessStrings( oldstr );
			kp->value.type = Q3F_TYPE_ARRAY;
			G_Q3F_RemString( &oldstr );
		}

		kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, cycleclientstatsptr = G_Q3F_GetString( "cycleclientstats" ) );
		if( kp )
		{
			oldstr = kp->value.d.strdata;
			kp->value.d.keypairarraydata = G_Q3F_ProcessClientStatsString( oldstr );
			kp->value.type = Q3F_TYPE_ARRAY;
			G_Q3F_RemString( &oldstr );
		}

		kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, cycletargetptr = G_Q3F_GetString( "cycletarget" ) );
		if( kp )
		{
			oldstr = kp->value.d.strdata;
			kp->value.d.keypairarraydata = G_Q3F_ProcessStateString( oldstr );
			kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
			G_Q3F_RemString( &oldstr );
		}

		kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, cyclefailtargetptr = G_Q3F_GetString( "cyclefailtarget" ) );
		if( kp )
		{
			oldstr = kp->value.d.strdata;
			kp->value.d.keypairarraydata = G_Q3F_ProcessStateString( oldstr );
			kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
			G_Q3F_RemString( &oldstr );
		}
	}
	else G_FreeEntity( ent );
}

/*
**
**	target_respawn - causes all affected clients to be respawned instantly.
**
*/

static void G_Q3F_TargetRespawnTouch( gentity_t *respawn, gentity_t *player, trace_t *tr )
{
	// Force the client to respawn.

	int teams, pointcontents;
	float distance, effectfactor;
	q3f_keypair_t *data;
	q3f_array_t *holding, *notholding;
	q3f_keypairarray_t *clientstats;
	gentity_t *current, *minent, *maxent, *tent;
	vec3_t vec3;
	trace_t trace;
	char *affectteamsptr, *effectradiusptr/*, *holdingptr, *notholdingptr*/;
	g_q3f_playerclass_t	*cls;

	if( player && !player->client )
		player = NULL;		// Stop checking against ent for criteria

	// Make sure we have our pointers to search on. NULL pointers
	// don't matter, we know they don't exist in any ents anyway
	// or they'd have been in the string table.
	affectteamsptr	= G_Q3F_GetString( "affectteams" );
	effectradiusptr	= G_Q3F_GetString( "effectradius" );
	/*holdingptr		= G_Q3F_GetString( "holding" );
	notholdingptr	= G_Q3F_GetString( "notholding" );
	holding = notholding = NULL;*/

		// Work out what players are affected by this give
	teams = 0;
	if( respawn->mapdata )
	{
		if( player )
		{
			if( respawn->mapdata->flags & Q3F_FLAG_AFFECTTEAM )
				teams |= 1 << player->client->sess.sessionTeam;
			if( respawn->mapdata->flags & Q3F_FLAG_AFFECTNONTEAM )
				teams |= ~((1 << player->client->sess.sessionTeam) | Q3F_TEAM_SPECTATOR);
		}
		data = G_Q3F_KeyPairArrayFind( respawn->mapdata->other, affectteamsptr );
		teams |= data ? G_Q3F_ProcessTeamString( data->value.d.strdata ) : 0;
		data = G_Q3F_KeyPairArrayFind( respawn->mapdata->other, effectradiusptr );
		distance = data ? atof( data->value.d.strdata ) : 0;

		if( respawn->mapdata->flags & Q3F_FLAG_ENVIRONMENT )
			pointcontents = trap_PointContents( respawn->s.pos.trBase, respawn - level.gentities ) & MASK_WATER;

		/*if( respawn->mapdata )
		{
			if( data = G_Q3F_KeyPairArrayFind( respawn->mapdata->other, holdingptr ) )
				holding = data->value.d.arraydata;
			if( data = G_Q3F_KeyPairArrayFind( respawn->mapdata->other, notholdingptr ) )
				notholding = data->value.d.arraydata;
		}*/
		holding = respawn->mapdata->holding;
		notholding = respawn->mapdata->notholding;
		clientstats = respawn->mapdata->clientstats;
	}

		// Work out min and max of our loop
	if( teams || distance )
	{
		minent = level.gentities;
		maxent = &level.gentities[MAX_CLIENTS-1];
	}
	else minent = maxent = player;

	for( current = minent; current && current <= maxent; current++ )
	{
			// For each player, check the ent can affect them
		if( !current->client || !current->inuse )
			continue;		// Not a player, or dead
		if( teams && !(teams & (1 << current->client->sess.sessionTeam) ))
			continue;		// Bad team
		if( current->health <= 0 && !(respawn->mapdata->flags & Q3F_FLAG_ALLOWDEAD) )
			continue;		// Dead player
		if( distance )
		{
			VectorSubtract( current->s.pos.trBase, respawn->s.pos.trBase, vec3 );
			effectfactor = 1.0 - VectorLength( vec3 ) / distance;
			if( effectfactor <= 0 )
				continue;	// Out of range
		}
		else effectfactor = 1;
		if( respawn->mapdata && respawn->mapdata->flags & Q3F_FLAG_ENVIRONMENT )
		{
			if( (trap_PointContents( current->s.pos.trBase, current - level.gentities ) & MASK_WATER) !=
				pointcontents )
				continue;	// Different environment
		}
		if( respawn->mapdata && respawn->mapdata->flags & Q3F_FLAG_LINEOFSIGHT )
		{
			trap_Trace( &trace, respawn->s.pos.trBase, NULL, NULL, current->s.pos.trBase, respawn-level.gentities, MASK_SOLID );
			if( trace.entityNum != current-level.gentities )
				continue;	// Trace failed, not in line-of-sight
		}
		if( holding && !G_Q3F_CheckHeld( current, holding ) )
			continue;		// Not holding the required ents
		if( notholding && !G_Q3F_CheckNotHeld( current, notholding ) )
			continue;		// Not holding the required ents
		if( clientstats && !G_Q3F_CheckClientStats( current, clientstats, (respawn->mapdata->flags & Q3F_FLAG_ORCLIENTSTATS) ) )
			continue;		// Not having the right stats			

		// Looks like they're prime candidates for the job. Respawn them.
		tent = G_TempEntity( current->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = current->s.clientNum;
		cls = G_Q3F_GetClass(&(current->client->ps));
		if(cls->DeathCleanup)
			cls->DeathCleanup(current);
		ClientSpawn( current );
		tent = G_TempEntity( current->client->ps.origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = current->s.clientNum;
	}

		// Trigger ourself inactive again immediately
	G_Q3F_TriggerEntity( respawn, player, Q3F_STATE_INACTIVE, tr, qfalse );
}

void SP_Q3F_target_respawn( gentity_t *ent )
{
	ent->touch = G_Q3F_TargetRespawnTouch;
}

/*
**
**	target_respawn - causes all affected clients to be teleported instantly.
**
*/

static void G_Q3F_TargetMultiportTouch( gentity_t *respawn, gentity_t *player, trace_t *tr )
{
	// Teleport clients to the named teleports or respawns

	int teams, pointcontents, index, count;
	float distance, effectfactor;
	q3f_keypair_t *data;
	q3f_data_t *dest;
	q3f_array_t *holding, *notholding, *destinations;
	q3f_keypairarray_t *clientstats;
	gentity_t *current, *minent, *maxent, *tent, *destport;
	vec3_t vec3;
	trace_t trace;
	char *affectteamsptr, *effectradiusptr/*, *holdingptr, *notholdingptr*/;
	gentity_t *destports[256];

	// Start by locating the possible destinations
	if( !(data = G_Q3F_KeyPairArrayFind( level.targetnameArray, respawn->target )) )
		return;
	destinations = data->value.d.arraydata;

	if( player && !player->client )
		player = NULL;		// Stop checking against ent for criteria

	// Make sure we have our pointers to search on. NULL pointers
	// don't matter, we know they don't exist in any ents anyway
	// or they'd have been in the string table.
	affectteamsptr	= G_Q3F_GetString( "affectteams" );
	effectradiusptr	= G_Q3F_GetString( "effectradius" );
	/*holdingptr		= G_Q3F_GetString( "holding" );
	notholdingptr	= G_Q3F_GetString( "notholding" );
	holding = notholding = NULL;*/

		// Work out what players are affected by this give
	teams = 0;
	if( respawn->mapdata )
	{
		if( player )
		{
			if( respawn->mapdata->flags & Q3F_FLAG_AFFECTTEAM )
				teams |= 1 << player->client->sess.sessionTeam;
			if( respawn->mapdata->flags & Q3F_FLAG_AFFECTNONTEAM )
				teams |= ~((1 << player->client->sess.sessionTeam) | Q3F_TEAM_SPECTATOR);
		}
		data = G_Q3F_KeyPairArrayFind( respawn->mapdata->other, affectteamsptr );
		teams |= data ? G_Q3F_ProcessTeamString( data->value.d.strdata ) : 0;
		data = G_Q3F_KeyPairArrayFind( respawn->mapdata->other, effectradiusptr );
		distance = data ? atof( data->value.d.strdata ) : 0;

		if( respawn->mapdata->flags & Q3F_FLAG_ENVIRONMENT )
			pointcontents = trap_PointContents( respawn->s.pos.trBase, respawn - level.gentities ) & MASK_WATER;

		/*if( respawn->mapdata )
		{
			if( data = G_Q3F_KeyPairArrayFind( respawn->mapdata->other, holdingptr ) )
				holding = data->value.d.arraydata;
			if( data = G_Q3F_KeyPairArrayFind( respawn->mapdata->other, notholdingptr ) )
				notholding = data->value.d.arraydata;
		}*/
		holding = respawn->mapdata->holding;
		notholding = respawn->mapdata->notholding;
		clientstats = respawn->mapdata->clientstats;
	}

		// Work out min and max of our loop
	if( teams || distance )
	{
		minent = level.gentities;
		maxent = &level.gentities[MAX_CLIENTS-1];
	}
	else minent = maxent = player;

	for( current = minent; current && current <= maxent; current++ )
	{
			// For each player, check the ent can affect them
		if( !current->client || !current->inuse )
			continue;		// Not a player, or dead
		if( teams && !(teams & (1 << current->client->sess.sessionTeam) ))
			continue;		// Bad team
		if( current->health <= 0 && !(respawn->mapdata->flags & Q3F_FLAG_ALLOWDEAD) )
			continue;		// Dead player
		if( distance )
		{
			VectorSubtract( current->s.pos.trBase, respawn->s.pos.trBase, vec3 );
			effectfactor = 1.0 - VectorLength( vec3 ) / distance;
			if( effectfactor <= 0 )
				continue;	// Out of range
		}
		else effectfactor = 1;
		if( respawn->mapdata && respawn->mapdata->flags & Q3F_FLAG_ENVIRONMENT )
		{
			if( (trap_PointContents( current->s.pos.trBase, current - level.gentities ) & MASK_WATER) !=
				pointcontents )
				continue;	// Different environment
		}
		if( respawn->mapdata && respawn->mapdata->flags & Q3F_FLAG_LINEOFSIGHT )
		{
			trap_Trace( &trace, respawn->s.pos.trBase, NULL, NULL, current->s.pos.trBase, respawn-level.gentities, MASK_SOLID );
			if( trace.entityNum != current-level.gentities )
				continue;	// Trace failed, not in line-of-sight
		}
		if( holding && !G_Q3F_CheckHeld( current, holding ) )
			continue;		// Not holding the required ents
		if( notholding && !G_Q3F_CheckNotHeld( current, notholding ) )
			continue;		// Not holding the required ents
		if( clientstats && !G_Q3F_CheckClientStats( current, clientstats, (respawn->mapdata->flags & Q3F_FLAG_ORCLIENTSTATS) ) )
			continue;		// Not having the right stats	

		for( index = -1, count = 0; (dest = G_Q3F_ArrayTraverse( destinations, &index )) && count < 256; )
		{
			// Locate a likely target
			destport = dest->d.entitydata;
			if( (!Q_stricmp( destport->classname, "info_player_deathmatch" ) ||
				!Q_stricmp( destport->classname, "misc_teleporter_dest" ) ||
				!Q_stricmp( destport->classname, "target_position" ) ||
				!Q_stricmp( destport->classname, "info_notnull" )) &&
				(!destport->mapdata || destport->mapdata->state == Q3F_STATE_INACTIVE) &&
				!SpotWouldTelefrag( destport ) )
				destports[count++] = destport;
		}
		if( count )
		{
			// Pick one of them
			destport = destports[rand() % count];
		}
		else {
			// Didn't find one, now try active targets (may result in telefrags)
			destport = dest->d.entitydata;
			if( (!Q_stricmp( destport->classname, "info_player_deathmatch" ) ||
				!Q_stricmp( destport->classname, "misc_teleporter_dest" ) ||
				!Q_stricmp( destport->classname, "target_position" ) ||
				!Q_stricmp( destport->classname, "info_notnull" )) &&
				(!destport->mapdata || destport->mapdata->state == Q3F_STATE_ACTIVE || destport->mapdata->state == Q3F_STATE_INACTIVE) )
				destports[count++] = destport;
			if( count )
			{
				// Pick one of them
				destport = destports[rand() % count];
			}
			else {
				destport = NULL;
			}
		}

		if( destport )
		{
			// Looks like they're prime candidates for the job. Respawn them.
			tent = G_TempEntity( current->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
			tent->s.clientNum = current->s.clientNum;
			TeleportPlayer( current, destport->s.origin, destport->s.angles );
			tent = G_TempEntity( current->client->ps.origin, EV_PLAYER_TELEPORT_IN );
			tent->s.clientNum = current->s.clientNum;
			G_Q3F_TriggerEntity( destport, current, Q3F_STATE_ACTIVE, tr, qfalse );
		}
	}

		// Trigger ourself inactive again immediately
	G_Q3F_TriggerEntity( respawn, player, Q3F_STATE_INACTIVE, tr, qfalse );
}

void SP_Q3F_target_multiport( gentity_t *ent )
{
	if( !ent->target || !*ent->target )
	{
		G_FreeEntity( ent );
	}
	else {
		ent->touch = G_Q3F_TargetMultiportTouch;
	}
}


/*
**	func_damage entity - solid (or trigger?) brush that can be 'damaged',
**	as well as triggered for a set number of points. It will also regenerate
**	itself at a set rate.
*/

static void G_Q3F_KillDamageBox( gentity_t *damage )
{
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;

	if( !damage || !damage->r.linked )
		return;
	num = trap_EntitiesInBox( damage->r.mins, damage->r.maxs, touch, MAX_GENTITIES );

	for (i=0 ; i<num ; i++) {
		hit = &g_entities[touch[i]];
		if( hit == damage )
			continue;
		if(	!hit->client &&
			strcmp( hit->classname, "autosentry" ) &&
			strcmp( hit->classname, "supplystation" ) )
			continue;

		// nail it
		G_Damage( hit, damage, damage, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_TELEFRAG );
	}
}

//static qboolean gotfuncdamageptrs;
qboolean gotfuncdamageptrs;
static char *healthtargetptr, *damagetargetptr, *restoretargetptr;
static char *q3f_fdhealthmessagekeys[3][8];	// Keep the strings for a little extra speed
static char *q3f_fddamagemessagekeys[3][8];	// Keep the strings for a little extra speed
static char *q3f_fdrestoremessagekeys[3][8];	// Keep the strings for a little extra speed
//static char *q3f_fdhealthmessagekeys[3][4];	// Keep the strings for a little extra speed
//static char *q3f_fddamagemessagekeys[3][4];	// Keep the strings for a little extra speed
//static char *q3f_fdrestoremessagekeys[3][4];	// Keep the strings for a little extra speed
static void G_Q3F_FuncDamageCalc( gentity_t *self, gentity_t *other, qboolean checkcriteria )
{
	// Think about where we are, and when next to think.

	q3f_mapent_t *mapdata;
	q3f_keypair_t *kp;
	float regenerated;
	char *ptr;
	trace_t tr;

	if( !gotfuncdamageptrs )
	{
		healthtargetptr		= G_Q3F_GetString( "healthtarget" );
		damagetargetptr		= G_Q3F_GetString( "damagetarget" );
		restoretargetptr	= G_Q3F_GetString( "restoretarget" );
		gotfuncdamageptrs	= qtrue;
	}

	mapdata = self->mapdata;
	if( checkcriteria && other && other->client )
	{
		// Do a manual criteria check, since this isn't necessarily a trigger

		if( !G_Q3F_CheckCriteria( other, self ) )
		{
			if( g_mapentDebug.integer )
				G_Printf(	"func_damage: %d damage ignored from %s.\n",
							(self->splashDamage - self->health),
							other->client->pers.netname );
			self->health = self->splashDamage;		// Reset health
			return;
		}
		if( g_mapentDebug.integer )
			G_Printf(	"func_damage: %d damage from %s.\n",
						(self->splashDamage - self->health),
						other->client->pers.netname );
	}

	if(	self->last_move_time &&
		(self->last_move_time <= level.time ||
		self->mapdata->state == Q3F_STATE_INACTIVE || self->mapdata->state == Q3F_STATE_ACTIVE) )
	{
		// Time to resurrect ourself

		if( g_mapentDebug.integer )
			G_Printf(	"func_damage: Restored at %d.\n",
						level.time );

		memset( &tr, 0, sizeof(tr) );
		G_Q3F_TriggerEntity( self, self->mapdata->lastTriggerer, Q3F_STATE_INACTIVE, &tr, qtrue );
		self->last_move_time = 0;
		self->health = self->count;

		if( kp = G_Q3F_KeyPairArrayFind( self->mapdata->other, restoretargetptr ) )
		{
			if( kp->value.type == Q3F_TYPE_STRING )
			{
				ptr = kp->value.d.strdata;
				kp->value.d.keypairarraydata = G_Q3F_ProcessStateString( ptr );
				kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
				G_Q3F_RemString( &ptr );
			}
			memset( &tr, 0, sizeof(tr) );
			G_Q3F_PropogateTrigger( kp->value.d.keypairarraydata, NULL, &tr );
			G_Q3F_StateBroadcast( self, self, self, "_message", &q3f_fdrestoremessagekeys[0],	Q3F_BROADCAST_TEXT, "restore" );
			G_Q3F_StateBroadcast( self, self, self, "_sound", &q3f_fdrestoremessagekeys[1],		Q3F_BROADCAST_SOUND, "restore" );
			G_Q3F_StateBroadcast( self, self, self, "_dict", &q3f_fdrestoremessagekeys[2],		Q3F_BROADCAST_DICT, "restore" );
		}

		self->takedamage = qtrue;
		trap_LinkEntity( self );
		trap_AdjustAreaPortalState( self, qfalse );
		G_Q3F_KillDamageBox( self );
	}
	else if( !self->last_move_time ) {
		char*	key;
		char	buf[256];
		int		j;
		q3f_keypair_t* kp;
		q3f_keypairarray_t *kpa;
		int given;

		regenerated = (level.time - self->timestamp) * self->speed / 1000;	// Work out 'regenerated' health
		if( self->splashDamage + regenerated > self->count )
			regenerated = self->count - self->splashDamage;

		given = (self->health + regenerated) - self->splashDamage;
//		self->health
		if( given > 0)
			for(j = self->splashDamage; j < self->splashDamage + given; j++) {
				Com_sprintf(buf, 256, "health_over_%i_target", j);

			if( ( key = G_Q3F_GetString( buf )) &&
				(kp = G_Q3F_KeyPairArrayFind( self->mapdata->other, key)) ) {

				if( kp->value.type == Q3F_TYPE_STRING ) {
					key = kp->value.d.strdata;
					kp->value.d.keypairarraydata = G_Q3F_ProcessStateString( key );
					kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
					G_Q3F_RemString( &key );
				}
				kpa = kp->value.d.keypairarraydata;
				if( g_mapentDebug.integer ) {
					G_Printf( "    Triggering func_damage health check target\n" );
				}
				G_Q3F_PropogateTrigger( kpa, other, NULL );
			}
		} else if( given < 0)
			for(j = self->splashDamage; j > self->splashDamage + given; j--) {
				Com_sprintf(buf, 256, "health_under_%i_target", j);

			if( ( key = G_Q3F_GetString( buf )) &&
				(kp = G_Q3F_KeyPairArrayFind( self->mapdata->other, key)) ) {

				if( kp->value.type == Q3F_TYPE_STRING ) {
					key = kp->value.d.strdata;
					kp->value.d.keypairarraydata = G_Q3F_ProcessStateString( key );
					kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
					G_Q3F_RemString( &key );
				}
				kpa = kp->value.d.keypairarraydata;
				if( g_mapentDebug.integer ) {
					G_Printf( "    Triggering func_damage health check target\n" );
				}
				G_Q3F_PropogateTrigger( kpa, other, NULL );
			}
		}

		if( self->mapdata->state != Q3F_STATE_INVISIBLE && self->health < self->count && (self->health + regenerated) >= self->count )
		{
			// We've hit max health. Yippee!

			if( g_mapentDebug.integer ) {
				G_Printf(	"func_damage: Reached %d health at %d.\n",
							self->count, level.time );
				G_Printf(	"func_damage: Reached max health at %d.\n",
							level.time );

			}

			self->health = self->count;
			if( kp = G_Q3F_KeyPairArrayFind( self->mapdata->other, healthtargetptr ) )
			{
				if( kp->value.type == Q3F_TYPE_STRING )
				{
					ptr = kp->value.d.strdata;
					kp->value.d.keypairarraydata = G_Q3F_ProcessStateString( ptr );
					kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
					G_Q3F_RemString( &ptr );
				}
				memset( &tr, 0, sizeof(tr) );
				G_Q3F_PropogateTrigger( kp->value.d.keypairarraydata, NULL, &tr );
				G_Q3F_StateBroadcast( self, self, self, "_message", &q3f_fdhealthmessagekeys[0],	Q3F_BROADCAST_TEXT, "health" );
				G_Q3F_StateBroadcast( self, self, self, "_sound", &q3f_fdhealthmessagekeys[1],		Q3F_BROADCAST_SOUND, "health" );
				G_Q3F_StateBroadcast( self, self, self, "_dict", &q3f_fdhealthmessagekeys[2],		Q3F_BROADCAST_DICT, "health" );
			}
		}
		else {
			self->health += regenerated;
			if( self->health <= 0 && self->mapdata->state != Q3F_STATE_INVISIBLE )
			{
				// We've died (or been triggered into oblivion). Booooh :(

				if( g_mapentDebug.integer ) {
					G_Printf(	"func_damage: damaged at %d.\n",
								level.time );
					G_Printf(	"func_damage: died at %d.\n",
								level.time );
				}

				self->health = 0;

				self->last_move_time =	self->pain_debounce_time
										? level.time + self->pain_debounce_time
										: 0;
				self->takedamage = qfalse;
				trap_AdjustAreaPortalState( self, qtrue );
				trap_UnlinkEntity( self );		// Vanish from mortal ken.

				if( kp = G_Q3F_KeyPairArrayFind( self->mapdata->other, damagetargetptr ) )
				{
					if( kp->value.type == Q3F_TYPE_STRING )
					{
						ptr = kp->value.d.strdata;
						kp->value.d.keypairarraydata = G_Q3F_ProcessStateString( ptr );
						kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
						G_Q3F_RemString( &ptr );
					}
					memset( &tr, 0, sizeof(tr) );
					G_Q3F_PropogateTrigger( kp->value.d.keypairarraydata, other, &tr );
					G_Q3F_StateBroadcast( self, other, other, "_message", &q3f_fddamagemessagekeys[0],	Q3F_BROADCAST_TEXT, "damage" );
					G_Q3F_StateBroadcast( self, other, other, "_sound", &q3f_fddamagemessagekeys[1],	Q3F_BROADCAST_SOUND, "damage" );
					G_Q3F_StateBroadcast( self, other, other, "_dict", &q3f_fddamagemessagekeys[2],		Q3F_BROADCAST_DICT, "damage" );
				}

				memset( &tr, 0, sizeof(tr) );
				G_Q3F_TriggerEntity( self, NULL, Q3F_STATE_INVISIBLE, &tr, qtrue );
			}
			else {
				if( g_mapentDebug.integer )
					G_Printf(	"func_damage: health %d at %d.\n",
								self->health, level.time );
			}
		}
	}

	self->splashDamage	= self->health;
	self->timestamp		= level.time;

	if( self->last_move_time )
		self->nextthink = self->last_move_time;
	else //if( (regenerated = 1000 * ((float)(self->count - self->health)) / self->speed) )
//		self->nextthink = level.time + regenerated;
		self->nextthink = level.time + 1000; 
	// need to think often for the target_under_<value>s to work well

	if( g_mapentDebug.integer )
		G_Printf(	"func_damage: nextthink %d (now %d).\n",
					self->nextthink, level.time );

	if( self->mapdata->state == Q3F_STATE_ACTIVE )
		G_Q3F_TriggerEntity( self, other, Q3F_STATE_INACTIVE, NULL, qtrue );
}

static void G_Q3F_FuncDamageThink( gentity_t *ent )
	{ G_Q3F_FuncDamageCalc( ent, NULL, qfalse ); }
static void G_Q3F_FuncDamageDie( gentity_t *self, gentity_t *inflictor, gentity_t *attacker, int damage, int meansOfDeath )
	{ G_Q3F_FuncDamageCalc( self, attacker, qtrue ); }
static void G_Q3F_FuncDamagePain( gentity_t *self, gentity_t *attacker, int damage )
	{ G_Q3F_FuncDamageCalc( self, attacker, qtrue ); }
static void G_Q3F_FuncDamageUse( gentity_t *self, gentity_t *other, gentity_t *activator )
{
	// 'damage' the entity by a preset amount when triggered.
	self->health = self->splashDamage - self->damage;
	G_Q3F_FuncDamageCalc( self, activator, qfalse );
}

void SP_Q3F_func_damage( gentity_t *ent )
{
	float restoretime;

	if( !ent->mapdata )
	{
		// Not much use without extended data

		ent->mapdata = G_Alloc( sizeof(q3f_mapent_t) );
//		return;
	}

	if( ent->health <= 0 )
		ent->health = 100;

	ent->count = ent->health;						// count holds the 'max health'
	ent->splashDamage = ent->health;				// splashDamage holds the 'last health'
	ent->model2	= ent->model;						// model2 holds the 'original model'
	G_SpawnFloat( "regen", "1", &ent->speed );		// Speed holds the 'regen per second'
	G_SpawnInt( "damage", "30", &ent->damage );	// Damage done by a trigger.
	G_SpawnFloat( "restore", "30", &restoretime );	// Time before a destroyed door regenerates
	ent->pain_debounce_time = 1000 * restoretime;
	ent->r.contents &= ~CONTENTS_TRIGGER;			// Not touchable.

	trap_SetBrushModel( ent, ent->model );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );
	ent->r.svFlags	= SVF_USE_CURRENT_ORIGIN;
	ent->s.eType	= ET_GENERAL;

	if( ent->mapdata->state == Q3F_STATE_INVISIBLE )
	{
		// If it's invisible, 
		ent->nextthink = level.time + 10000;	// Irrelevant, as long as it causes think()s
		ent->last_move_time = 0;				// Long time in the future.
		trap_AdjustAreaPortalState( ent, qtrue );
	}
	else {
		ent->takedamage = qtrue;
		trap_LinkEntity( ent );						// Only link if it's visible
//		trap_AdjustAreaPortalState( ent, qfalse );
	}

	ent->think		= G_Q3F_FuncDamageThink;
	ent->use		= G_Q3F_FuncDamageUse;		// Not touch, since it's not actually touchable.
	ent->pain		= G_Q3F_FuncDamagePain;
	ent->die		= G_Q3F_FuncDamageDie;
}


/*
**	misc_onkill and misc_onprotect entities - when a player is killed/almost killed,
**	these get triggered if the criteria match.
*/

typedef struct g_q3f_onkill_s {
	// Duplicate criteria for the victim
	int team, classes, flags;//, teamscore;
	q3f_array_t *holding, *notholding;
	q3f_keypairarray_t *target, *clientstats;//, *give, *failtarget;
} g_q3f_onkill_t;

static g_q3f_onkill_t *G_Q3F_OnKillCriteria( gentity_t *ent, char *prefix )
{
	// Parse the 'victim' criteria.

	g_q3f_onkill_t *criteria;
	char *key, *value;
	q3f_keypair_t *kp;
	int index;
	char buffallowteams[32], buffallowclasses[32], buffholding[32], buffnotholding[32];
	char bufftarget[32], buffflags[32], buffclientstats[32];//, buffteamscore[32], buffgive[32], bufffailtarget[32];

	if( !ent->mapdata || !ent->mapdata->other )
		return( NULL );		// No victim criteria

	if( !(criteria = G_Alloc( sizeof(g_q3f_onkill_t) )) )
		G_Error( "G_Q3F_MiscOnKillCriteria(): Unable to allocate g_q3f_misconkill_t" );
	memset( criteria, 0, sizeof(g_q3f_onkill_t) );

	index = strlen( prefix );
	Q_strncpyz( buffallowteams, prefix, sizeof(buffallowteams) );
	Q_strcat( buffallowteams, sizeof(buffallowteams), "allowteams" );
	Q_strncpyz( buffallowclasses, prefix, sizeof(buffallowclasses) );
	Q_strcat( buffallowclasses, sizeof(buffallowteams), "allowclasses" );
	Q_strncpyz( buffholding, prefix, sizeof(buffholding) );
	Q_strcat( buffholding, sizeof(buffallowteams), "holding" );
	Q_strncpyz( buffnotholding, prefix, sizeof(buffnotholding) );
	Q_strcat( buffnotholding, sizeof(buffnotholding), "notholding" );
	Q_strncpyz( buffclientstats, prefix, sizeof(buffclientstats) );
	Q_strcat( buffclientstats, sizeof(buffclientstats), "clientstats" );
//	Q_strncpyz( buffteamscore, prefix, sizeof(buffteamscore) );
//	Q_strcat( buffteamscore, sizeof(buffteamscore), "teamscore" );
//	Q_strncpyz( buffgive, prefix, sizeof(buffgive) );
//	Q_strcat( buffgive, sizeof(buffgive), "give" );
	Q_strncpyz( bufftarget, prefix, sizeof(bufftarget) );
	Q_strcat( bufftarget, sizeof(bufftarget), "target" );
//	Q_strncpyz( bufffailtarget, prefix, sizeof(bufffailtarget) );
//	Q_strcat( bufffailtarget, sizeof(bufffailtarget), "failtarget" );
	Q_strncpyz( buffflags, prefix, sizeof(buffflags) );
	Q_strcat( buffflags, sizeof(buffflags), "flags" );

	for( index = -1; kp = G_Q3F_KeyPairArrayTraverse( ent->mapdata->other, &index ); )
	{
		key = kp->key;
		value = kp->value.d.strdata;
		if( !Q_stricmp( buffallowteams, key ) )
			criteria->team = G_Q3F_ProcessTeamString( value );
		else if( !Q_stricmp( buffallowclasses, key ) )
			criteria->classes = G_Q3F_ProcessClassString( value );
		else if( !Q_stricmp( buffholding, key ) )
			criteria->holding = G_Q3F_ProcessStrings( value );
		else if( !Q_stricmp( buffnotholding, key ) )
			criteria->notholding = G_Q3F_ProcessStrings( value );
		else if( !Q_stricmp( buffclientstats, key ) )
			criteria->clientstats = G_Q3F_ProcessClientStatsString( value );
//		else if( !Q_stricmp( buffteamscore, key ) )
//			criteria->teamscore = atoi( value );
//		else if( !Q_stricmp( buffgive, key ) )
//			criteria->give = G_Q3F_ProcessGiveString( value );
		else if( !Q_stricmp( bufftarget, key ) )
			criteria->target = G_Q3F_ProcessStateString( value );
//		else if( !Q_stricmp( bufffailtarget, key ) )
//			criteria->failtarget = G_Q3F_ProcessStateString( value );
		else if( !Q_stricmp( buffflags, key ) )
			criteria->flags = G_Q3F_ProcessFlagString( value );
	}

	return( criteria );
}

void G_Q3F_LinkOnKillChain()
{
	// Build a chain of onkill entities

	gentity_t *curr, *last;

	level.onKillHead = NULL;
	for(	last = NULL, curr = &g_entities[MAX_CLIENTS];
			curr < &g_entities[level.num_entities]; curr++ )
	{
		if( !curr->inuse || strcmp( curr->classname, "misc_onkill" ) )
			continue;
		if( last )
			last->chain = curr;
		else level.onKillHead = curr;
		curr->chain = NULL;
		last = curr;
	}
}

static qboolean G_Q3F_CheckOnKillCriteria( gentity_t *miscent, gentity_t *player, gentity_t *other, g_q3f_onkill_t *killdata ) 
{
	// Return true if all criteria are satisfied.

	int cls, team;
	qboolean passedCriteria;

	if( (miscent->mapdata->flags & Q3F_FLAG_SAMECLASS) &&
		player->client->ps.persistant[PERS_CURRCLASS] != other->client->ps.persistant[PERS_CURRCLASS] )
		return( qfalse );
	if( (miscent->mapdata->flags & Q3F_FLAG_SAMETEAM) &&
		player->client->sess.sessionTeam != other->client->sess.sessionTeam )
		return( qfalse );

		// Check criteria for the killed
	if( killdata && other )
	{
		team	= other->client->sess.sessionTeam;
		cls		= other->client->ps.persistant[PERS_CURRCLASS];
		if( cls == Q3F_CLASS_AGENT && other->flags & Q3F_FLAG_DISGUISECRITERIA )
		{
			if( other->client->agentclass )
				cls = other->client->agentclass;
			if( other->client->agentteam )
				team = other->client->agentteam;
		}
		
		{
			qboolean a1, b1, c1, d1, e1;
			a1 = !killdata->team || (killdata->team & (1 << team));
			b1 = !killdata->classes || (killdata->classes & (1 << cls));
			c1 = !killdata->holding || G_Q3F_CheckHeld( other, killdata->holding );
			d1 = !killdata->clientstats || G_Q3F_CheckClientStats( other, killdata->clientstats, (other->mapdata->flags & Q3F_FLAG_ORCLIENTSTATS) );
			e1 = !killdata->notholding || G_Q3F_CheckNotHeld( other, killdata->notholding );

			passedCriteria =	(a1) &&	// In the correct team?
								(b1) &&	// In required class?
								(c1) &&	// Holding the required items?
								(d1) &&	// Are our clientstats how we want them?
								(e1);	// Not holding the required items?
		}
		if( (killdata->flags & Q3F_FLAG_REVERSECRITERIA) ? passedCriteria : !passedCriteria )
			return( qfalse );
	}

	return( qtrue );
}

#define	Q3F_MAX_ONKILL_PLAYERS		32

static qboolean gotonkillptrs;
char *attackertargetptr, *victimtargetptr;
char *okattackermessagekeys[3][4];
char *okvictimmessagekeys[3][4];
void G_Q3F_CheckOnKill( gentity_t *attacker, gentity_t *victim )
{
	// Run through all the onkill entities in turn, check to see
	// if attackertarget or victimtarget can be executed.

	gentity_t *current, *scan, *killer, *killed, *other;
	qboolean issameplayer, needrankcalculation;
	trace_t tr;
	g_q3f_onkill_t *killdata;
	vec3_t pos;

	if( !attacker || !attacker->client )
		attacker = victim;
	issameplayer = attacker == victim;

	needrankcalculation = qfalse;
	for( current = level.onKillHead; current; current = current->chain )
	{
		if( issameplayer && (!current->mapdata || !(current->mapdata->flags & Q3F_FLAG_ALLOWSAME)) )
			continue;			// Not applicable to suicides
		if( !current->mapdata || current->mapdata->state != Q3F_STATE_INACTIVE )
			continue;			// Not triggerable

		killer = NULL;
		if( current->methodOfDeath )
		{
			// For misc_onprotect, we need to scan around the dead person's area to see
			// if there was a 'protected victim' to match criteria against.
			// Check criteria for the killer first, since we can do this before we start
			// scanning through other 

			if( !G_Q3F_CheckCriteria( attacker, current ) )
				continue;	// The main criteria failed
			if( !G_Q3F_CheckOnKillCriteria( current, attacker, victim, (g_q3f_onkill_t *) current->health ) )
				continue;	// The 'attacker' (i.e. person _trying_ to attack) criteria failed.
			for( scan = g_entities; scan < &g_entities[level.maxclients]; scan++ )
			{
				// We now scan through for (still-living) 'victims' that would match the onprotect ent.
				if( !scan->inuse || !scan->client || scan->health <= 0 || Q3F_IsSpectator( scan->client ) )
					continue;
				if( !G_Q3F_CheckOnKillCriteria( current, attacker, scan, (g_q3f_onkill_t *) current->count ) )
					continue;	// The 'victim' (i.e. person who was in danger) criteria failed.
				if( current->damage && Distance( attacker->client->ps.origin, scan->client->ps.origin ) > current->damage )
					continue;
				if( (current->mapdata->flags & Q3F_FLAG_LINEOFSIGHT) )
				{
					VectorCopy( attacker->client->ps.origin, pos );
					pos[2] += attacker->client->ps.viewheight;
					if( !CanDamage( scan, pos, attacker ) )
						continue;
				}

				// We have a match, now we do the stuff.
				killer	= attacker;
				killed	= victim;
				other	= scan;
				break;
			}
		}
		else {
			// For misc_onkill, we don't need to do any scanning.

			if( !G_Q3F_CheckCriteria( attacker, current ) )
				continue;	// The main criteria failed
			if( !G_Q3F_CheckOnKillCriteria( current, attacker, victim, (g_q3f_onkill_t *) current->count ) )
				continue;
			if( current->damage && Distance( attacker->client->ps.origin, victim->client->ps.origin ) > current->damage )
				continue;
			if( current->parent && current->health )
			{
				gentity_t *watchclient = current->spawnflags & 4 ? victim : attacker;

				if( current->health && Distance( watchclient->client->ps.origin, current->parent->r.currentOrigin ) > current->health )
					continue;
				if( current->spawnflags & 1 )
				{
					if( !CanDamage( victim, current->parent->r.currentOrigin, current->parent ) )
						continue;
				}
				if( current->spawnflags & 2 )
				{
					if( !CanDamage( attacker, current->parent->client->ps.origin, current->parent ) )
						continue;
				}
			}
			if( (current->mapdata->flags & Q3F_FLAG_LINEOFSIGHT) )
			{
				VectorCopy( attacker->client->ps.origin, pos );
				pos[2] += attacker->client->ps.viewheight;
				if( !CanDamage( victim, pos, attacker ) )
					continue;
			}

			killer	= attacker;
			killed	= victim;
			other	= NULL;
		}

		if( killer )
		{
			G_Q3F_TriggerEntity( current, killer, Q3F_STATE_ACTIVE, &tr, qfalse );
			if( killed && (killdata = (g_q3f_onkill_t *) current->count) )
				G_Q3F_PropogateTrigger( killdata->target, killed, &tr );
			if( other && (killdata = (g_q3f_onkill_t *) current->health) )
				G_Q3F_PropogateTrigger( killdata->target, other, &tr );
		}
	}
}

void Q3F_locateonkillwatch( gentity_t *ent )
{
	ent->parent = G_PickTarget( ent->message );
	if ( !ent->parent ) {
		G_Printf( "Couldn't find watch entity for misc_onkill\n" );
		G_FreeEntity( ent );
		return;
	}
}

void SP_Q3F_misc_onkill( gentity_t *self )
{
	// When a player is killed, this is triggered

	q3f_keypair_t *kp;
	char *str;

	if( !self->mapdata )
	{
		G_FreeEntity( self );
		return;
	}
	self->classname		= "misc_onkill";
	self->methodOfDeath	= 0;
	self->count			= (int) G_Q3F_OnKillCriteria( self, "victim" );

		// Specify the range the entity is effective to
	if( self->mapdata &&
		(str = G_Q3F_GetString( "range" )) &&
		(kp = G_Q3F_KeyPairArrayFind( self->mapdata->other, str )) )
	{
		self->damage = atoi( kp->value.d.strdata );
		G_Q3F_KeyPairArrayDel( self->mapdata->other, str );
		if( self->damage < 0 )
			self->damage = 0;
	}

		// Set an entity to watch and the range to it
	self->soundPos1 = 0;
	self->soundPos2 = 0;
	self->health = 0;
	self->parent = NULL;
	if( self->mapdata &&
		(str = G_Q3F_GetString( "watch" )) &&
		(kp = G_Q3F_KeyPairArrayFind( self->mapdata->other, str )) )
	{
		self->message = G_NewString( kp->value.d.strdata );
		G_Q3F_KeyPairArrayDel( self->mapdata->other, str );

		if( (str = G_Q3F_GetString( "watchrange" )) &&
			(kp = G_Q3F_KeyPairArrayFind( self->mapdata->other, str )) )
		{
			self->health = atoi( kp->value.d.strdata );
			G_Q3F_KeyPairArrayDel( self->mapdata->other, str );
			if( self->health < 0 )
				self->health = 0;
		}

		self->think = Q3F_locateonkillwatch;
		self->nextthink = level.time + FRAMETIME;
	}
}

void SP_Q3F_misc_onprotect( gentity_t *self )
{
	// When a player is killed, this is triggered.
	// There are three players involved in this: defender (who made the kill),
	// attacker (who was attacking the victim), and victim (who just stood there looking stupid)

	SP_Q3F_misc_onkill( self );
	if( !self->inuse )
		return;
	self->methodOfDeath	= 1;
	self->health		= (int) G_Q3F_OnKillCriteria( self, "attacker" );
}


/*
**	target_semitrigger - when activated, randomly triggers a number of
**	entities in semitarget
*/

#define MAX_SEMITRIGGER_TARGETS	128
static void G_Q3F_TargetSemitriggerTouch( gentity_t *ent, gentity_t *other, trace_t *tr )
{
	// Randomly trigger a set of entities

	gentity_t *targets[MAX_SEMITRIGGER_TARGETS];
	int states[MAX_SEMITRIGGER_TARGETS];
	qboolean forceflags[MAX_SEMITRIGGER_TARGETS];
	int index, targindex, arrayindex;
	q3f_keypair_t *curr, *targetkp;
	q3f_array_t *targetarray;
	q3f_data_t *target;

		// Build an array of all entities to target
	for(	index = -1, arrayindex = 0;
			(curr = G_Q3F_KeyPairArrayTraverse( (q3f_keypairarray_t *) ent->timestamp, &index )) && arrayindex < MAX_SEMITRIGGER_TARGETS; )
	{
		if( !(targetkp = G_Q3F_KeyPairArrayFind( level.targetnameArray, curr->key )) )
			continue;
		targindex = -1;
		targetarray = targetkp->value.d.arraydata;
		while( (target = G_Q3F_ArrayTraverse( targetarray, &targindex )) && arrayindex < MAX_SEMITRIGGER_TARGETS )
		{
			targets[arrayindex] = target->d.entitydata;
			states[arrayindex] = curr->value.d.intdata;
			forceflags[arrayindex] = curr->value.flags & Q3F_VFLAG_FORCE;
			arrayindex++;
		}
	}

		// Work out the number of entries we'll trigger
	targindex = ent->damage - ent->count + 1;
	index = ent->count + (targindex ? (rand() % targindex) : 0);

		// Now (attempt to) trigger our entities
	while( index && arrayindex > 0 )
	{
		targindex = rand() % arrayindex;
		if( G_Q3F_TriggerEntity( targets[targindex], other, states[targindex], tr, forceflags[targindex] ) )
			index--;		// Successful trigger, reduce count
		arrayindex--;
		targets[targindex]		= targets[arrayindex];		// Fill the 'gap' in the array left
		states[targindex]		= states[arrayindex];		// by the entity we just triggered,	
		forceflags[targindex]	= forceflags[arrayindex];	// to stop it being triggered twice.
	}

		// Now trigger ourself back to inactive, ready for another shot.
	G_Q3F_TriggerEntity( ent, other, Q3F_STATE_INACTIVE, tr, qfalse );
}

void SP_Q3F_target_semitrigger( gentity_t *self )
{
	q3f_keypair_t *kp;
	char *str;

	if( !self->mapdata || !(kp = G_Q3F_KeyPairArrayFind( self->mapdata->other, G_Q3F_GetString( "semitarget" ) )) )
	{
		G_Printf( "Warning: target_semitrigger has no semitarget specified.\n" );
		G_FreeEntity( self );
		return;
	}

	// Split up target string into individual entities
	str = kp->value.d.strdata;
	kp->value.d.keypairarraydata = G_Q3F_ProcessStateString( str );
	kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
	G_Q3F_RemString( &str );

	self->timestamp = (int) kp->value.d.keypairarraydata;
	G_SpawnInt( "count2", "-1", &self->damage );
	if( self->damage < 0 )
		self->damage = self->count;

	self->touch = G_Q3F_TargetSemitriggerTouch;
}


/*
**	func_explosion - cause a visual explosion effect.
*/

void G_Q3F_FuncExplosionTouch( gentity_t *self, gentity_t *other, trace_t *tr )
{
	// 'pulse' so the client sees us
	G_AddEvent( self, EV_ETF_EXPLOSION, 0 );
	trap_LinkEntity( self );
	self->unlinkAfterEvent = qtrue;
	self->mapdata->waittime = level.time + self->wait + Q_flrand(-1.0f, 1.0f) * self->random;
}

void SP_Q3F_func_explosion( gentity_t *self )
{
	// Spawn ent, create default values
	char *str;

	G_SetOrigin( self, self->s.origin );

	G_SpawnFloat( "radius", "24", &self->s.angles[0] );
	G_SpawnFloat( "impact", "50", &self->s.angles[1] );

	if( !self->wait )
		self->wait = 1;
	self->wait *= 1000;

	G_SpawnString( "script", "spirit/explosions/explosion_normal.spirit", &str );
	if( *str ) {
		self->s.legsAnim = G_SpiritScriptIndex( str );
	}

	self->touch = G_Q3F_FuncExplosionTouch;
	self->s.eType = ET_INVISIBLE;
}

/*QUAKED misc_beam (0 .5 .8) (-8 -8 -8) (8 8 8) ? BEAM_STRAIGHT BEAM_WAVE BEAM_WAVE_3D BEAM_TILESHADER
When on, displays a electric beam from target to target2.
"target"	start of beam
"target2"	end of beam
"shader"	the shader
"color"		colour of beam
"dmg"		damage on crossing
"segments"	number of segments (0 = shaft stylee)
"scale"		width of beam
"maxamp"	maximum amplitude
*/

#define Q3F_BEAM_INVULNERABILITYTIME 250;

void G_Q3F_MiscBeamThink (gentity_t *self) {
	trace_t	trace;
	vec3_t startpos, endpos;
	gentity_t		*traceEnt;

	if ( self->enemy ) {
		if ( self->enemy != self ) {
			//VectorCopy ( self->enemy->s.origin, self->s.origin2 );
			self->s.apos.trType = self->enemy->s.pos.trType;
			self->s.apos.trTime = self->enemy->s.pos.trTime;
			self->s.apos.trDuration = self->enemy->s.pos.trDuration;
			VectorCopy( self->enemy->s.pos.trBase, self->s.apos.trBase );
			VectorCopy( self->enemy->s.pos.trDelta, self->s.apos.trDelta );
		} else {
			self->s.apos.trType = TR_STATIONARY;
			VectorCopy( self->s.origin, self->s.apos.trBase );
		}
	} else if ( self->pos2[0] != 0 && self->pos2[1] != 0 && self->pos2[2] != 0 ) {
		self->s.apos.trType = TR_STATIONARY;
		VectorCopy( self->pos2, self->s.apos.trBase );
	}

	if ( self->target_ent ) {
		//VectorCopy ( self->target_ent->s.origin, self->s.origin );
		self->s.pos.trType = self->target_ent->s.pos.trType;
		self->s.pos.trTime = self->target_ent->s.pos.trTime;
		self->s.pos.trDuration = self->target_ent->s.pos.trDuration;
		VectorCopy( self->target_ent->s.pos.trBase, self->s.pos.trBase );
		VectorCopy( self->target_ent->s.pos.trDelta, self->s.pos.trDelta );
	} else if ( self->pos1[0] != 0 && self->pos1[1] != 0 && self->pos1[2] != 0 ) {
		self->s.pos.trType = TR_STATIONARY;
		VectorCopy( self->pos1, self->s.pos.trBase );
	} else { // trace to a random target
		vec3_t dest;
		int r;

		r = rand() % NUMVERTEXNORMALS;

		VectorMA( self->s.apos.trBase, 4096, bytedirs[r], dest );
		trap_Trace ( &trace, self->s.apos.trBase, NULL, NULL, dest, -1, CONTENTS_SOLID );

		if ( trace.fraction < 1.0 ) {
			self->s.pos.trType = TR_STATIONARY;
			VectorCopy( trace.endpos, self->s.pos.trBase );
		}
	}

	if ( self->mapdata->state != Q3F_STATE_DISABLED && self->mapdata->state != Q3F_STATE_INVISIBLE && self->damage ) {
		BG_EvaluateTrajectory( &self->s.pos, level.time, startpos );
		BG_EvaluateTrajectory( &self->s.apos, level.time, endpos );

		if ( Distance( startpos, endpos ) > 5 ) {
			trap_Trace( &trace, startpos, NULL, NULL, endpos, self->s.number, MASK_SHOT);
			traceEnt = &g_entities[ trace.entityNum ];

			// make sure everyone in the beam gets hit
			while ( traceEnt->takedamage && traceEnt->nextbeamhittime < level.time ) {
				if ( traceEnt->nextbeamhittime < level.time ) {
					if( traceEnt && traceEnt->client ) {
						if ( G_Q3F_CheckCriteria( traceEnt, self ) ) {
							G_Damage ( traceEnt, self, self->activator, NULL, 
								trace.endpos, self->damage, DAMAGE_NO_KNOCKBACK, MOD_BEAM);
							G_Q3F_TriggerEntity( self, traceEnt, Q3F_STATE_ACTIVE, &trace, qfalse );
							traceEnt->nextbeamhittime = level.time + Q3F_BEAM_INVULNERABILITYTIME;
						}
					}
					else if( (traceEnt->s.eType == ET_Q3F_SENTRY || traceEnt->s.eType == ET_Q3F_SUPPLYSTATION) && traceEnt->parent && traceEnt->parent->client ) {
							if ( G_Q3F_CheckCriteria( traceEnt->parent, self ) ) {
								G_Damage ( traceEnt, self, self->activator, NULL, 
									trace.endpos, self->damage, DAMAGE_NO_KNOCKBACK, MOD_BEAM);
								//G_Q3F_TriggerEntity( self, traceEnt, Q3F_STATE_ACTIVE, &trace, qfalse );
								traceEnt->nextbeamhittime = level.time + Q3F_BEAM_INVULNERABILITYTIME;
							}
					}
				}

				VectorCopy( trace.endpos, startpos );

				trap_Trace( &trace, startpos, NULL, NULL, endpos, self->s.number, MASK_SHOT);
				traceEnt = &g_entities[ trace.entityNum ];
			}
		}
	}

	if ( self->mapdata->state == Q3F_STATE_DISABLED || self->mapdata->state == Q3F_STATE_INVISIBLE ) {
		trap_UnlinkEntity( self );
	} else {
		trap_LinkEntity( self );
	}

	self->nextthink = level.time + FRAMETIME;
}

void G_Q3F_MiscBeamStart ( gentity_t *self ) {
	gentity_t *ent;

	self->s.eType = ET_Q3F_BEAM;

	if ( self->target ) {
		ent = G_Find (NULL, FOFS(targetname), self->target);
		if (!ent) {
			G_Printf ("%s at %s: %s is a bad target\n", self->classname, vtos(self->s.origin), self->target);
			G_FreeEntity( self );
			return;
		}
		self->target_ent = ent;
	}

	if( self->target_ent && self->pos1[0] != 0 && self->pos1[1] != 0 && self->pos1[2] != 0 ) {
		G_Printf ("%s at %s: defines a target and a startpos\n", self->classname, vtos(self->s.origin));
		G_FreeEntity( self );
		return;
	}
	
	if( self->message ) {
		ent = G_Find (NULL, FOFS(targetname), self->message);
		if( !ent ) {
			G_Printf ("%s at %s: %s is a bad target2\n", self->classname, vtos(self->s.origin), self->message);
			G_FreeEntity( self );
			return; // No targets by this name.
		}
		self->enemy = ent;
	} else { // the misc_beam is it's own ending point
		self->enemy = self;
	}

	if( self->enemy && self->enemy != self && self->pos2[0] != 0 && self->pos2[1] != 0 && self->pos2[2] != 0) {
		G_Printf ("%s at %s: defines a target2 and an endpos\n", self->classname, vtos(self->s.origin));
		G_FreeEntity( self );
		return;
	}

	self->think = G_Q3F_MiscBeamThink;
	self->nextthink = level.time + FRAMETIME;
}

void SP_Q3F_misc_beam( gentity_t *self ) {
	char *str;

/*	if( !self->mapdata )
	{
		// Not much use without extended data
		self->mapdata = G_Alloc( sizeof(q3f_mapent_t) );
	}
	*/

	G_SpawnString( "target2", "", &str );
	if( *str ) {
		self->message = G_NewString( str );
	}

	G_SpawnVector( "startpos", "0 0 0", self->pos1 );
	G_SpawnVector( "endpos", "0 0 0", self->pos2 );

	G_SpawnString( "shader", "lightningBolt", &str );
	if( *str )
		self->s.modelindex2 = G_ShaderIndex( str );

	G_SpawnInt( "segments", "0", &self->s.otherEntityNum );
	//G_SpawnInt( "style", "0", &self->s.otherEntityNum2 ); // 0 = straight, 1 = wave TODO: make this nice text parsing
	G_SpawnInt( "scale", "1", &self->s.torsoAnim );
	G_SpawnInt( "maxamp", "5", &self->s.legsAnim );
	G_SpawnColor( "1 1 1", self->s.angles2 );
	G_SpawnFloat( "wavescale", "1", &self->s.angles[0] );

	G_SpawnInt( "dmg", "0", &self->damage );

	//BEAM_STRAIGHT BEAM_WAVE BEAM_WAVE_3D BEAM_TILESHADER
	if ( self->spawnflags & 1 )
		self->s.otherEntityNum2  |= Q3F_BEAM_STRAIGHT;
	else if ( self->spawnflags & 2 )
		self->s.otherEntityNum2  |= Q3F_BEAM_WAVE_EFFECT;
	else if ( self->spawnflags & 4 )
		self->s.otherEntityNum2  |= Q3F_BEAM_WAVE_EFFECT_3D;
	else
		self->s.otherEntityNum2  |= Q3F_BEAM_NO_EFFECT;
	
	if ( self->spawnflags & 8 )
		self->s.otherEntityNum2  |= Q3F_BEAM_TILESHADER;

	// let everything else get spawned before we start firing
	self->think = G_Q3F_MiscBeamStart;
	self->nextthink = level.time + FRAMETIME;
}

/*
**	func_wall entity - When disabled or invisible, it's hidden, else it's,
**	just a wall with working areaportal.
**
*/
void G_Q3F_FuncWallThink (gentity_t *self) {
	if ( self->mapdata->state == Q3F_STATE_INACTIVE || self->mapdata->state == Q3F_STATE_ACTIVE ) {
		if ( self->mapdata->state != Q3F_STATE_INACTIVE )
			G_Q3F_TriggerEntity( self, self->mapdata->lastTriggerer, Q3F_STATE_INACTIVE, NULL, qtrue );
		if( !self->r.linked ) {
			trap_LinkEntity( self );
		}
		if ( self->count ) {
			self->count = 0;
			trap_AdjustAreaPortalState( self, qfalse );
		}
	} else {
		if ( self->mapdata->state != Q3F_STATE_INVISIBLE )
			G_Q3F_TriggerEntity( self, NULL, Q3F_STATE_INVISIBLE, NULL, qtrue );
		if ( !self->count ) {
			self->count = 1;
			trap_AdjustAreaPortalState( self, qtrue );
		}
		if( self->r.linked ) {
			trap_UnlinkEntity( self );
		}
	}
	self->nextthink = level.time + FRAMETIME;
}

void SP_Q3F_func_wall( gentity_t *ent )
{
	if( !ent->mapdata )
	{
		// Not much use without extended data
		ent->mapdata = G_Alloc( sizeof(q3f_mapent_t) );
	}

	ent->model2	= ent->model;						// model2 holds the 'original model'
	ent->r.contents = CONTENTS_DETAIL;

	trap_SetBrushModel( ent, ent->model );
	ent->r.svFlags	= SVF_USE_CURRENT_ORIGIN;
	ent->s.eType	= ET_GENERAL;
	ent->s.pos.trType = ent->s.apos.trType = TR_STATIONARY;

	if( ent->mapdata->state == Q3F_STATE_INVISIBLE || ent->mapdata->state == Q3F_STATE_DISABLED ) {
		ent->count = 1; // holds areaportal status
		trap_AdjustAreaPortalState( ent, qtrue );
	}
	else {
		ent->count = 0; // holds areaportal status
		trap_LinkEntity( ent );						// Only link if it's visible
	}

	ent->think		= G_Q3F_FuncWallThink;
	ent->nextthink = level.time + FRAMETIME;
}

/******************************************************************************
***** Forcefields
****/

qboolean G_Q3F_ForceFieldAllowDirection( vec3_t direction, const vec3_t start, const vec3_t end )
{
	// See if the specified entity is travelling in the correct direction.

	vec3_t diff;
	float dot;

	if( !(direction[0] || direction[1] || direction[2] ) )
		return( qtrue );
	VectorSubtract( end, start, diff );
	VectorNormalize( diff );
	dot = DotProduct( direction, diff );
	return( dot >= 0 );
}

void G_Q3F_ForceFieldExtTrace(	trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs,
								const vec3_t end, int passEntityNum, int realOwner, int contentmask )
{
	// Slightly different version of the above, owner field is set explicitely.

	gentity_t *ent, *activator;
	gclient_t *client;
	q3f_mapent_t *mapdata;
//	qboolean passedcriteria;

	trap_Trace( results, start, mins, maxs, end, passEntityNum, contentmask );
	ent	= &g_entities[results->entityNum];
	activator = &g_entities[realOwner];

	if( (contentmask & CONTENTS_FORCEFIELD) &&
		(results->startsolid || results->fraction < 1.0f) &&
		(!results->contents || (results->contents & CONTENTS_FORCEFIELD)) &&
		realOwner >= 0 && realOwner < MAX_CLIENTS &&
		activator && activator->inuse )
	{
		// Right, the magic starts here - Find the entity we hit, and see if it allows teams or values.

		client	= activator->client;
		mapdata	= ent->mapdata;

		// Check we're actually in a forcefield - the contentmask might be zero if we got a
		// startsolid result.
		if( ent->s.eType != ET_Q3F_FORCEFIELD )
			return;

			// A startsolid trace is just broken, period. We set this so that anyone checking the results
			// knows what hit them.
		results->contents |= CONTENTS_FORCEFIELD;

		if( mapdata )
		{
			if( !G_Q3F_CheckCriteria( activator, ent ) )
			{
				if(	!(ent->s.eFlags & EF_Q3F_FAILDIRECTION) ||
					!G_Q3F_ForceFieldAllowDirection( ent->s.angles, start, end ) )
					return;		// Boing!
			}
			else if(	!(ent->s.eFlags & EF_Q3F_FAILDIRECTION) &&
						!G_Q3F_ForceFieldAllowDirection( ent->s.angles, start, end ) )
				return;			// Sproing!

				// Try the trace _ignoring_ the forcefield.
			trap_UnlinkEntity( ent );
			G_Q3F_ForceFieldExtTrace( results, start, mins, maxs, end, passEntityNum, realOwner, contentmask );
			trap_LinkEntity( ent );
		}
	}
}

void G_Q3F_ForceFieldTouch( gentity_t *ent, gentity_t *other, trace_t *trace )
{
	if( !ent->mapdata || !(ent->mapdata->flags & Q3F_FLAG_DETPIPES) )
		return;			// Not an extended ent

	// Empty function, it simply exists to allow trigger propogation
	if( !other->client || Q3F_IsSpectator( other->client ) || other->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_GRENADIER )
		return;			// Not a client?

	G_Q3F_DetPipe(other, level.time);
}

static int G_Q3F_ForceFieldStateThink( gentity_t *ent, gentity_t *activator, int targetstate, int oldstate, int force, trace_t *trace ) {
	if( targetstate == Q3F_STATE_INVISIBLE ) {
		trap_UnlinkEntity( ent );
	} else {
		trap_LinkEntity( ent );
	}

	return targetstate;
}

void SP_Q3F_func_forcefield( gentity_t *ent )
{
	// Not a lot to this function.
	trap_SetBrushModel( ent, ent->model );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );
	ent->r.contents = CONTENTS_FORCEFIELD|CONTENTS_TRIGGER;
	ent->clipmask = CONTENTS_FORCEFIELD|CONTENTS_TRIGGER;
	ent->s.eType = ET_Q3F_FORCEFIELD;

	if( ent->mapdata )
	{
		// This will allow prediction only in certain cases (i.e. it can't predict properly
		// if the holding/notholding/clientstats/checkstate/reversecriteria fields are in use).
		// Spectators are always allowed through.

		ent->mapdata->team |= (1 << Q3F_TEAM_SPECTATOR);

		ent->s.frame			= ent->mapdata->classes; // Ensiform: This did overflow with civilians (changed from otherEntityNum to frame)
		ent->s.otherEntityNum2	= ent->mapdata->team;

			// Client-side flags.
		if( ent->mapdata->flags & Q3F_FLAG_FAILDIRECTION )
			ent->s.eFlags |= EF_Q3F_FAILDIRECTION;
		if( ent->mapdata->flags & Q3F_FLAG_REVERSECRITERIA )
			ent->s.eFlags |= EF_Q3F_REVERSECRITERIA;
		if( ent->mapdata->flags & Q3F_FLAG_DISGUISECRITERIA )
			ent->s.eFlags |= EF_Q3F_DISGUISECRITERIA;
	}

		// If no teams or classes were specified, anyone can go through.
		// Ensiform: This did overflow with civilians (changed from otherEntityNum to frame)
	if( !ent->s.frame )
		ent->s.frame = (1 << Q3F_CLASS_MAX) - 1;
	if( !ent->s.otherEntityNum2 || ent->s.otherEntityNum2 == (1 << Q3F_TEAM_SPECTATOR) ) // Ensiform: This always fails since spectator is forced
		ent->s.otherEntityNum2 = (1 << Q3F_TEAM_NUM_TEAMS) - 1; // Ensiform: This was overwriting class

		// Movement direction
	if( G_SpawnVector( "angles", "0 0 0", ent->s.angles ) )
	{
		AngleVectors( ent->s.angles, ent->s.angles, NULL, NULL );
		ent->s.angles[PITCH] = -ent->s.angles[PITCH];		// Positive angles -> up more intuitive?
	}
	else ent->s.angles[0] = ent->s.angles[1] = ent->s.angles[2] = 0;

		// Spark colour
	G_SpawnColor( "1.0 1.0 1.0", ent->s.angles2 );
		// Trigger
	ent->touch	= G_Q3F_ForceFieldTouch;
	ent->wait	= ent->wait ? (ent->wait * 1000) : 2000;		// Default wait of 2 seconds.

	if(	ent->mapdata &&
		ent->mapdata->state != Q3F_STATE_INVISIBLE )
		trap_LinkEntity( ent );

	ent->mapdata->statethink = G_Q3F_ForceFieldStateThink;
}

void G_Q3F_VisibilityTouch(gentity_t * ent, gentity_t * other, trace_t * trace)
{
	// Empty function, it simply exists to allow trigger propogation
}

static int G_Q3F_VisibilityStateThink(gentity_t * ent, gentity_t * activator, int targetstate, int oldstate, int force,
									  trace_t * trace)
{
	if(targetstate == Q3F_STATE_INVISIBLE)
	{
		trap_UnlinkEntity(ent);
	}
	else
	{
		trap_LinkEntity(ent);
	}

	return targetstate;
}

// Ensiform : Simple Criteria and/or range based rendered brush (should be non-solid)
void SP_Q3F_func_visibility(gentity_t * ent)
{
	// Not a lot to this function.

	qboolean        specvis, alpha;

	if(VectorCompare(ent->s.origin, vec3_origin))
		G_Printf("func_visibility possibly at world origin or missing origin brush!\n");

	trap_SetBrushModel(ent, ent->model);
	VectorCopy(ent->s.origin, ent->r.currentOrigin);
	ent->r.contents = CONTENTS_DETAIL;
	ent->r.svFlags = SVF_USE_CURRENT_ORIGIN;
	ent->s.pos.trType = ent->s.apos.trType = TR_STATIONARY;
	ent->s.eType = ET_Q3F_VISIBILITY;

	G_SpawnBoolean("spectatorvis", "true", &specvis);
	G_SpawnFloat("distance", "0", &ent->s.origin2[0]);
	G_SpawnBoolean("alpha", "true", &alpha);

	if(alpha)
		ent->s.groundEntityNum = 1;
	else
		ent->s.groundEntityNum = 0;

	if(ent->s.origin2[0] <= 0)
		ent->s.origin2[0] = 65535.0;

	if(ent->mapdata)
	{
		if(specvis)
			ent->mapdata->team |= 1 << Q3F_TEAM_SPECTATOR;

		ent->s.frame			= ent->mapdata->classes;
		ent->s.otherEntityNum2	= ent->mapdata->team;
	}

	// If no teams or classes were specified, anyone can see it.
	if(!ent->s.frame)
		ent->s.frame = (1 << Q3F_CLASS_MAX) - 1;
	if(!ent->s.otherEntityNum2 || ent->s.otherEntityNum2 == (1 << Q3F_TEAM_SPECTATOR))
		ent->s.otherEntityNum2 = (1 << Q3F_TEAM_NUM_TEAMS) - 1;

	// Trigger
	ent->touch = G_Q3F_VisibilityTouch;
	ent->wait = ent->wait ? (ent->wait * 1000) : 2000;	// Default wait of 2 seconds.

	if(ent->mapdata && ent->mapdata->state != Q3F_STATE_INVISIBLE)
		trap_LinkEntity(ent);

	ent->mapdata->statethink = G_Q3F_VisibilityStateThink;
}

/******************************************************************************
***** Accumulator.
****/

static int G_Q3F_TargetAccumulatorStateThink( gentity_t *ent, gentity_t *activator, int targetstate, int oldstate, int force, trace_t *trace )
{
	// Called every time the accumulator state changes.

	char *key;
	q3f_keypairarray_t *kpa;
	q3f_keypair_t *kp;
	char *messagekeys[3][8];
	int i,j;
//	char *messagekeys[3][4];
//	int index;

	if( targetstate == Q3F_STATE_ACTIVE ) {
		ent->count += ent->soundPos1;
	} else if( targetstate < 0 ) {
		ent->count += ent->soundPos2;
	} else if( targetstate == Q3F_STATE_INVISIBLE ) {
		ent->count = ent->soundLoop;
	} else {
		return( targetstate );
	}

		// Print off any appropriate messages.
	memset( messagekeys, 0, sizeof(messagekeys) );
	key = va( "%d", ent->count );
	G_Q3F_StateBroadcast( ent, activator, activator, "_message", &messagekeys[0],	Q3F_BROADCAST_TEXT, key );
	G_Q3F_StateBroadcast( ent, activator, activator, "_sound", &messagekeys[1],		Q3F_BROADCAST_SOUND, key );
	G_Q3F_StateBroadcast( ent, activator, activator, "_dict", &messagekeys[2],		Q3F_BROADCAST_DICT, key );

	//BirdDawg: clean up allocated string memory, since these are going to be worthless to cache
	for ( i = 0; i < 3 ; i++ )
	{
		for ( j = 0; j < 8 ; j++ )
		{
			G_Q3F_RemString( &messagekeys[i][j] );
		}
	}

	key = va( "target_%d", ent->count );
	if( (key = G_Q3F_GetString( key )) &&
		(kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, key)) )
	{
		if( kp->value.type == Q3F_TYPE_STRING )
		{
			key = kp->value.d.strdata;
			kp->value.d.keypairarraydata = G_Q3F_ProcessStateString( key );
			kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
			G_Q3F_RemString( &key );
		}
		kpa = kp->value.d.keypairarraydata;
		if( g_mapentDebug.integer )
			G_Printf( "    Triggering accumulator target %d in state think.\n", ent->count );
		G_Q3F_PropogateTrigger( kpa, activator, trace );
	}

	switch( targetstate )
	{
		// Find the correct string to use to trigger other entities
		case Q3F_STATE_ACTIVE:		kpa = ent->mapdata->activetarget;	break;
		case Q3F_STATE_INVISIBLE:	kpa = ent->mapdata->invisibletarget;	break;
		default:					kpa = NULL;
	}
	if( kpa && kpa->used )		// No targets, no problem :)
	{
		if( g_mapentDebug.integer )
			G_Printf( "    Triggering further targets in state think.\n" );
		G_Q3F_PropogateTrigger( kpa, activator, trace );
	}

	return( Q3F_STATE_INACTIVE );	// We always force back to inactive if we did something.
}
void SP_Q3F_target_accumulator( gentity_t *ent )
{
	// Allows triggering different entities based on the accumulator value.

	if( !ent->mapdata )
	{
		G_FreeEntity( ent );
		return;
	}

	G_SpawnInt( "increment",		"1", &ent->soundPos1 );
	G_SpawnInt( "failincrement",	"0", &ent->soundPos2 );
	G_SpawnInt( "initialvalue",		"0", &ent->soundLoop );

	ent->count = ent->soundLoop;
	ent->mapdata->statethink = G_Q3F_TargetAccumulatorStateThink;
}


/******************************************************************************
***** target_timer
****/


void SP_Q3F_target_timer( gentity_t *ent )
{
	// Create a 'timer' entity that will be replicated to all client.

//	ent->count = 
}


/******************************************************************************
***** info_targetspawn
****/

#define	DYNSPOTATTEMPTS	10
extern vec3_t playerMins, playerMaxs;
qboolean G_Q3F_PositionDynamicSpawn( gentity_t *ent, vec3_t origin, vec3_t angles )
{
	// Find an agreeable position to spawn a player on.

	q3f_keypair_t *targetkp;
	q3f_data_t *target;
	int numAttempts, targindex;
	q3f_array_t *targetarray;
	gentity_t *ents[32], *ignoreEnt, *curr;
//	char entAttempts[32], entAngles[32];
	vec3_t spotOrigin, top, dest, dir, mins, maxs;
	float angle;
	int numEnts, num, index, touch[8];
	trace_t trace;
	qboolean carried;

		// Find all the ents we want to try.
	if( ent->target &&
		(targetkp = G_Q3F_KeyPairArrayFind( level.targetnameArray, ent->target )) )
	{
		targetarray = targetkp->value.d.arraydata;
		targindex = -1;
		for(	numEnts = 0;
				target = G_Q3F_ArrayTraverse( targetarray, &targindex );
				numEnts++ )
		{
			ents[numEnts] = target->d.entitydata;
	//		entAttempts[numEnts] = DYNSPOTATTEMPTS;	// We try each ent this many times each at most.
		}

			// Randomize the order they're tried in.
		for( index = 0; index < numEnts; index++ )
		{
			num = rand() % numEnts;
			curr = ents[index];
			ents[index] = ents[num];
			ents[num] = ents[index];
		}
	}
	else {
		// Use the ent itself as the target point.

		ents[0] = ent;
		numEnts = 1;
	}

		// For each ent, attempt to 'spot' the player around it.
	for( index = 0; index < numEnts; index++ )
	{
		angle = Q_flrand(0.0f, 1.0f) * 2*M_PI;
		curr = ents[index];
		if( curr->mapdata && (curr->mapdata->flags & Q3F_FLAG_CARRYABLE) &&
			curr->mapdata->state == Q3F_STATE_CARRIED && curr->activator && curr->activator->client )
		{
			carried = qtrue;
			VectorCopy( (ignoreEnt = curr->activator)->client->ps.origin, spotOrigin );
		}
		else {
			carried = qfalse;
			VectorCopy( (ignoreEnt = curr)->r.currentOrigin, spotOrigin );
		}
		VectorCopy( spotOrigin, top );
		top[2] += 128;
		trap_Trace( &trace, spotOrigin, playerMins, playerMaxs, top, ignoreEnt->s.number, MASK_PLAYERSOLID );
		VectorCopy( trace.endpos, top );


		for( numAttempts = DYNSPOTATTEMPTS; numAttempts; numAttempts--, angle += (2*M_PI)/DYNSPOTATTEMPTS )
		{
			// Attempt to 'spot' it somewhere nearby. We rise up, then attempt to trace
			// down to a point on the user's original level.
			// Rise up

				// Trace to outer edge.
			dir[0] = cos( angle );
			dir[1] = sin( angle );
			dir[2] = 0;
			VectorMA( spotOrigin, 32 + Q_flrand(0.0f, 1.0f) * (ent->speed - 32), dir, dest );
			trap_Trace( &trace, top, playerMins, playerMaxs, dest, ignoreEnt->s.number, MASK_PLAYERSOLID );
			VectorCopy( trace.endpos, dest );

			if( dest[2] - spotOrigin[2] > 32 || (trap_PointContents( dest, ignoreEnt->s.number ) & MASK_WATER) )
				continue;	// It's too high.
			else if( trace.fraction == 1 && !(trap_PointContents( dest, ignoreEnt->s.number ) & MASK_WATER) )
			{
				// We're floating, we need to drop them to the floor.

				VectorCopy( dest, dir );
				dir[2] -= 64;
				trap_Trace( &trace, dest, playerMins, playerMaxs, dir, ignoreEnt->s.number, MASK_PLAYERSOLID );
				VectorCopy( trace.endpos, dest );
			}

				// Just check it's not going to kill anyone :)
			VectorAdd( dest, playerMins, mins );
			VectorAdd( dest, playerMaxs, maxs );
			num = trap_EntitiesInBox( mins, maxs, touch, 8 );
			for( num--; num >= 0; num-- )
			{
				if( (g_entities[touch[num]].clipmask & MASK_PLAYERSOLID))
					continue;		// We've got a collision.
			}
			if( num < 0 )
			{
				VectorCopy( dest, origin );
				if( carried ) {
					VectorCopy( ignoreEnt->r.currentAngles, angles );
				} else {
					// Point the player towards the spawn target.

					VectorSubtract( spotOrigin, dest, dir );
					vectoangles( dir, angles );
				}

				return( qtrue );
			}
		}
	}

	return( qfalse );
}

void SP_info_player_targetspawn( gentity_t *ent )
{
	// Create a new mobile spawn target

	VectorCopy( ent->s.origin, ent->r.currentOrigin );	// In case it isn't targeted.

	if( G_SpawnString( "target", NULL, &ent->target ) )	// Get a searchable target string.
		G_Q3F_AddString( &ent->target, ent->target );	// Prepare key searches.

	G_SpawnFloat( "range", "100", &ent->speed );

	if( ent->speed <= 0 )
		ent->speed = 100;
}


static int G_Q3F_MiscMatchtimerStateThink( gentity_t *ent, gentity_t *activator, int targetstate, int oldstate, int force, trace_t *trace ) {
	if( targetstate == Q3F_STATE_INACTIVE ) {
		ent->nextthink = level.time + ent->sound1to2;
	} else if( targetstate == Q3F_STATE_INVISIBLE ) {
		ent->count = ent->soundLoop;
		return Q3F_STATE_INACTIVE;
	}

	return( targetstate );
}

void G_Q3F_MiscMatchtimerThink( gentity_t *ent )
{
	// Called every time the accumulator state changes.

	char *key;
	q3f_keypairarray_t *kpa;
	q3f_keypair_t *kp;
	char *messagekeys[3][8];
	int i,j;
//	int index;

	memset( messagekeys, 0, sizeof(messagekeys) );
	if(ent->mapdata->state != Q3F_STATE_INACTIVE) {
		return;
	}

	ent->count += ent->soundPos1;

	//G_Printf( "misc_matchtimer thinking: state %d: count: %d\n", ent->mapdata->state, ent->count );

	key = va( "target_%d", ent->count );
	G_Q3F_StateBroadcast( ent, ent, ent, "_message",	&messagekeys[0], Q3F_BROADCAST_TEXT, key );
	G_Q3F_StateBroadcast( ent, ent, ent, "_sound",		&messagekeys[1], Q3F_BROADCAST_SOUND, key );
	G_Q3F_StateBroadcast( ent, ent, ent, "_dict",		&messagekeys[2], Q3F_BROADCAST_DICT, key );

	//BirdDawg: clean up allocated string memory, since these are going to be worthless to cache
	for ( i = 0; i < 3 ; i++ )
	{
		for ( j = 0; j < 8 ; j++ )
		{
		G_Q3F_RemString( &messagekeys[i][j] );
		}		
	}

/*	G_Q3F_StateBroadcast_TeamedNoActivator( ent, "_message",	Q3F_BROADCAST_TEXT,		key );
	G_Q3F_StateBroadcast_TeamedNoActivator( ent, "_sound",		Q3F_BROADCAST_SOUND,	key );
	G_Q3F_StateBroadcast_TeamedNoActivator( ent, "_dict",		Q3F_BROADCAST_DICT,		key );*/

	key = va( "target_%d", ent->count );
	if( (key = G_Q3F_GetString( key )) &&
		(kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, key)) ) {

		if( kp->value.type == Q3F_TYPE_STRING )
		{
			key = kp->value.d.strdata;
			kp->value.d.keypairarraydata = G_Q3F_ProcessStateString( key );
			kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
			G_Q3F_RemString( &key );
		}
		kpa = kp->value.d.keypairarraydata;
		if( g_mapentDebug.integer )
			G_Printf( "    Triggering misc_matchtimer target %d in think.\n", ent->count );
		G_Q3F_PropogateTrigger( kpa, NULL, NULL );
	}

	if(ent->count == ent->soundPos2 || ent->sound2to1 == 1) {
		key = "active";
		G_Q3F_StateBroadcast( ent, ent, ent, "_message",	&messagekeys[0], Q3F_BROADCAST_TEXT, key );
		G_Q3F_StateBroadcast( ent, ent, ent, "_sound",		&messagekeys[1], Q3F_BROADCAST_SOUND, key );
		G_Q3F_StateBroadcast( ent, ent, ent, "_dict",		&messagekeys[2], Q3F_BROADCAST_DICT, key );

		/*		G_Q3F_StateBroadcast_TeamedNoActivator( ent, "_message",	Q3F_BROADCAST_TEXT,		key );
		G_Q3F_StateBroadcast_TeamedNoActivator( ent, "_sound",		Q3F_BROADCAST_SOUND,	key );
		G_Q3F_StateBroadcast_TeamedNoActivator( ent, "_dict",		Q3F_BROADCAST_DICT,		key );*/

		if( (key = G_Q3F_GetString( key )) &&
			(kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, key)) ) {

			if( kp->value.type == Q3F_TYPE_STRING )
			{
				key = kp->value.d.strdata;
				kp->value.d.keypairarraydata = G_Q3F_ProcessStateString( key );
				kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
				G_Q3F_RemString( &key );
			}
			kpa = kp->value.d.keypairarraydata;
			if( g_mapentDebug.integer )
				G_Printf( "    Triggering misc_matchtimer target %d in think.\n", ent->count );
			G_Q3F_PropogateTrigger( kpa, ent, NULL );
		}

		ent->mapdata->state = Q3F_STATE_ACTIVE;
	}

	ent->nextthink = level.time + ent->sound1to2;

	G_Q3F_UpdateHUD( ent );

	return;
}

void SP_Q3F_misc_matchtimer( gentity_t *ent ) {
	q3f_keypair_t* kp;

	if( !ent->mapdata )		// No mapdata means no ent, effectively
	{
		G_FreeEntity( ent );
		return;
	}

	G_SpawnInt( "initialvalue",		"30",	&ent->soundLoop );
	G_SpawnInt( "stepsize",			"-1",	&ent->soundPos1 );
	G_SpawnInt( "endvalue",			"0",	&ent->soundPos2 );
	G_SpawnInt( "steptime",			"1000",	&ent->sound1to2 );
	G_SpawnInt( "pulse",			"0",	&ent->sound2to1	);

	// Sort before we can search (won't have been done yet)
	G_Q3F_KeyPairArraySort( ent->mapdata->other );

	ent->count = ent->soundLoop;
	
	ent->think =		G_Q3F_MiscMatchtimerThink;
	ent->nextthink =	level.time + ent->sound1to2;
	ent->mapdata->statethink = G_Q3F_MiscMatchtimerStateThink;

	kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "slot" ));
	if( kp ) {
		ent->s.weapon = atoi( kp->value.d.strdata );
	}

//	G_SpawnColor( "1 1 1", ent->s.angles2);
	
	if( ent->s.weapon < 1 || ent->s.weapon > Q3F_SLOT_MAX ) {
	} else {
		ent->r.svFlags	|= SVF_BROADCAST;	// All clients (potentially) need to know
		ent->s.eType	= ET_Q3F_HUD;

		ent->s.weapon--;		// The actual slots start at zero

		ent->chain = level.hudHead;
		level.hudHead = ent;

//		ent->classname = "func_hud";

		ent->s.eFlags = EF_VOTED;

		G_Q3F_UpdateHUD( ent );

		trap_LinkEntity( ent );
	}
}

void G_Q3F_BlackHoleUse( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	int			i, num;
	int			touch[MAX_GENTITIES];
	gentity_t	*hit;
	q3f_keypair_t* kp;
	int mask;
	int team;
	int cls;
	int clsmask;

	num = trap_EntitiesInBox( self->r.mins, self->r.maxs, touch, MAX_GENTITIES );

	kp = G_Q3F_KeyPairArrayFind( self->mapdata->other, G_Q3F_GetString( "affectteams" ));
	if( kp ) {
		mask = G_Q3F_ProcessTeamString( kp->value.d.strdata );
		// only affects specified teams
	} else {
		mask = 0;
		// all teams
	}

	kp = G_Q3F_KeyPairArrayFind( self->mapdata->other, G_Q3F_GetString( "affectclasses" ));
	if( kp ) {
		clsmask = G_Q3F_ProcessClassString( kp->value.d.strdata );
		// only affects specified teams
	} else {
		clsmask = 0;
		// all classes
	}

	if( !self->count ) {
		return; // SHOULDNT happen!!!
	}
	
	//kp = G_Q3F_KeyPairArrayFind( self->mapdata->other, G_Q3F_GetString( "type" ));
	//if( !kp ) {
	//	return; // SHOULDNT happen!!!
	//}

	//kpa = kp->value.d.keypairarraydata;

	for (i=0 ; i<num ; i++) {
		hit = &g_entities[touch[i]];
		if( hit == self ) {
			continue;
		}

		if( mask ) {
			if(hit->client) {
				team = hit->client->sess.sessionTeam;
			} else if( hit->parent && hit->parent->client) {
				team = hit->parent->client->sess.sessionTeam;
			} else {
				team = 0;
			}

			if(!(mask & (1 << team))) {
				continue;
			}
		}

		if( clsmask ) {
			if(hit->client) {
				cls = hit->client->ps.persistant[PERS_CURRCLASS];
			} else if( hit->parent && hit->parent->client) {
				cls = hit->parent->client->ps.persistant[PERS_CURRCLASS];
			} else {
				cls = 0;
			}

			if(!(clsmask & (1 << cls))) {
				continue;
			}
		}

//		G_Printf("blackhole touch: Classname: %s\n", hit->classname);

		if(hit->s.eType == ET_Q3F_SENTRY) {
			if( (self->count & Q3F_BHT_AUTOSENTRY) ) {
				G_Damage( hit, self, self, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_UNKNOWN );
			}
		} else if(hit->s.eType == ET_Q3F_SUPPLYSTATION) {
			if( (self->count & Q3F_BHT_SUPPLYSTATION) ) {
				G_Damage( hit, self, self, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_UNKNOWN );
			}
		} else if(	!strcmp(hit->classname, "pipe")) {
			if( (self->count & Q3F_BHT_PROJECTILES) ) {
				if( hit->parent ) {
					hit->parent->client->pipecount --;

					// player pipe count
					hit->parent->client->ps.ammoclip[0] = hit->parent->client->pipecount;
				}
				G_FreeEntity( hit );
			}
		} else if(	!strcmp(hit->classname, "grenade") ||
					!strcmp(hit->classname, "rocket") ||
					!strcmp(hit->classname, "napalm") ||
					!strcmp(hit->classname, "tranq_dart") ||
					!strcmp(hit->classname, "rail")) {
			if( (self->count & Q3F_BHT_PROJECTILES) ) {
				G_FreeEntity( hit );
			}
		} else if(	!strcmp(hit->classname, "charge")) {
			if( (self->count & Q3F_BHT_CHARGE) ) {
				G_FreeEntity( hit );
			}
		} else if(	!strcmp(hit->classname, "backpack")) {
			if( (self->count & Q3F_BHT_BACKPACKS) ) {
				G_FreeEntity( hit );
			}
		} else if(	!strcmp(hit->classname, "handgrenade") ||
					!strcmp(hit->classname, "flame") ) {
			if( (self->count & Q3F_BHT_GRENADES) ) {
				G_FreeEntity( hit );
			}
		}

		// FIXME: move to some sort of hash table style thingymibober?
#if 0

		if(hit->s.eType == ET_Q3F_SENTRY) {
			if( G_Q3F_KeyPairArrayFind( kpa, G_Q3F_GetString( "autosentry" ))) {
				G_Damage( hit, self, self, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_UNKNOWN );
			}
		} else if(hit->s.eType == ET_Q3F_SUPPLYSTATION) {
			if( G_Q3F_KeyPairArrayFind( kpa, G_Q3F_GetString( "supplystation" ))) {
				G_Damage( hit, self, self, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_UNKNOWN );
			}
		} else if(	!strcmp(hit->classname, "pipe")) {
			if( G_Q3F_KeyPairArrayFind( kpa, G_Q3F_GetString( "projectiles" ))) {
				if( hit->parent ) {
					hit->parent->client->pipecount --;

					// player pipe count
					hit->parent->client->ps.ammoclip[0] = hit->parent->client->pipecount;
				}
				G_FreeEntity( hit );
			}
		} else if(	!strcmp(hit->classname, "grenade") ||
					!strcmp(hit->classname, "rocket") ||
					!strcmp(hit->classname, "napalm") ||
					!strcmp(hit->classname, "tranq_dart") ||
					!strcmp(hit->classname, "rail")) {
			if( G_Q3F_KeyPairArrayFind( kpa, G_Q3F_GetString( "projectiles" ))) {
				G_FreeEntity( hit );
			}
		} else if(	!strcmp(hit->classname, "charge")) {
			if( G_Q3F_KeyPairArrayFind( kpa, G_Q3F_GetString( "charge" ))) {
				G_FreeEntity( hit );
			}
		} else if(	!strcmp(hit->classname, "backpack")) {
			if( G_Q3F_KeyPairArrayFind( kpa, G_Q3F_GetString( "backpacks" ))) {
				G_FreeEntity( hit );
			}
		} else if(	!strcmp(hit->classname, "handgrenade") ||
					!strcmp(hit->classname, "flame") ) {
			if( G_Q3F_KeyPairArrayFind( kpa, G_Q3F_GetString( "grenades" ))) {
				G_FreeEntity( hit );
			}
		}
#endif
	}

	self->mapdata->state = Q3F_STATE_INACTIVE; // trigger and reset entity
}

// fixme: move this over to g_trigger.c?
void SP_Q3F_misc_blackhole( gentity_t *ent ) {

	char *value;
	q3f_keypair_t* kp;

	if( !ent->mapdata ) {	// No mapdata means no ent, effectively
		G_FreeEntity( ent );
		return;
	}

	// Sort before we can search (won't have been done yet)
	G_Q3F_KeyPairArraySort( ent->mapdata->other );

	kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "type" ));
	if( kp ) {
		value = kp->value.d.strdata;
		ent->count = G_Q3F_ProcessBlackHoleTypeString( value );

		if( !ent->count ) {
			G_FreeEntity( ent );
			return;	// no type => nothing to remove => no point in having entity
		}
#if 0

		key = kp->value.d.strdata;
		kp->value.d.keypairarraydata = G_Q3F_ProcessStateString( key );
		kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
		G_Q3F_RemString( &key );
#endif

		// set up removal list
	} else {
		G_FreeEntity( ent );
		return;	// no type => nothing to remove => no point in having entity
	}

	ent->use = G_Q3F_BlackHoleUse;

	trap_SetBrushModel( ent, ent->model );
	ent->r.svFlags = SVF_NOCLIENT;
	ent->r.contents = CONTENTS_TRIGGER;

	trap_LinkEntity( ent );

	//G_Printf("^4initializing blackhole");
}

/*QUAKED misc_stopwatch (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) START_ON
*/

static void misc_stopwatch_use( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	ent->s.time = level.time;
}

static int misc_stopwatch_statethink( gentity_t *ent, gentity_t *activator, int targetstate, int oldstate, int force, trace_t *trace ) {
	if (targetstate == Q3F_STATE_INACTIVE) {
		int passed = (level.time - ent->s.time) / 1000;
		G_Q3F_EntityMessage( "%02d:%02d:%03d",
			passed / 60, passed % 60, (level.time - ent->s.time) % 1000);
	}
	return targetstate;
}


void SP_Q3F_misc_stopwatch( gentity_t *ent ) {
	if( !ent->mapdata )		// No mapdata means no ent, effectively
	{
		G_FreeEntity( ent );
		return;
	}
	ent->classname = "misc_stopwatch";
	ent->s.time = level.time;
	ent->use = misc_stopwatch_use;
	ent->mapdata->statethink = misc_stopwatch_statethink;
	ent->r.svFlags = SVF_NOCLIENT;
}

/*QUAKED misc_changeclass (0.3 0.1 0.6) (-8 -8 -8) (8 8 8) START_ON
*/

static void misc_changeclass_use( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	// broadcast a changeclass menu command to all clients
	gentity_t *player;

	for( player = g_entities; player < &g_entities[MAX_CLIENTS]; player++ ) {
		if ( player->client->sess.sessionTeam == Q3F_TEAM_SPECTATOR )			// don't bother with specs
			return;

		if( !player->client )
			return;

		G_Q3F_SendClassInfo(player);

		trap_SendServerCommand( player->s.number, "menu class");
	}
}

void SP_Q3F_misc_changeclass( gentity_t *ent ) {
	if( !ent->mapdata )		// No mapdata means no ent, effectively
	{
		G_FreeEntity( ent );
		return;
	}
	ent->classname = "misc_changeclass";
	ent->s.time = level.time;
	ent->use = misc_changeclass_use;
	ent->r.svFlags = SVF_NOCLIENT;
}

/*QUAKED misc_setweapons (0.3 0.1 0.6) (-8 -8 -8) (8 8 8)
*/

static void misc_setweapons_use( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	// broadcast a changeclass menu command to all clients
	gentity_t *player;

	for( player = g_entities; player < &g_entities[MAX_CLIENTS]; player++ ) {
		if ( player->client->sess.sessionTeam == Q3F_TEAM_SPECTATOR )			// don't bother with specs
			return;

		if( !player->client )
			return;

		G_Q3F_SendClassInfo(player);

		trap_SendServerCommand( player->s.number, "menu class");
	}
}

void SP_Q3F_misc_setweapons( gentity_t *ent ) {
	if( !ent->mapdata )		// No mapdata means no ent, effectively
	{
		G_FreeEntity( ent );
		return;
	}
	ent->classname = "misc_setweapons";
	ent->s.time = level.time;
	ent->use = misc_changeclass_use;
	ent->r.svFlags = SVF_NOCLIENT;
}

/*
**
**	misc_disableweapons - causes all affected clients to be respawned instantly.
**
*/

static void G_Q3F_MiscDisableWeaponsTouch( gentity_t *disabler, gentity_t *player, trace_t *tr )
{
	// Force the client to disable weapon list.

	int teams, pointcontents, classes, weapons;
	float distance, effectfactor;
	q3f_keypair_t *data;
	q3f_array_t *holding, *notholding;
	q3f_keypairarray_t *clientstats;
	gentity_t *current, *minent, *maxent;//, *tent;
	vec3_t vec3;
	trace_t trace;
	char *affectteamsptr, *effectradiusptr, *weaponsptr, *affectclassesptr/*, *holdingptr, *notholdingptr*/;
//	g_q3f_playerclass_t	*cls;

	if( player && !player->client )
		player = NULL;		// Stop checking against ent for criteria

	// Make sure we have our pointers to search on. NULL pointers
	// don't matter, we know they don't exist in any ents anyway
	// or they'd have been in the string table.
	affectteamsptr		= G_Q3F_GetString( "affectteams" );
	affectclassesptr	= G_Q3F_GetString( "affectclasses" );
	effectradiusptr		= G_Q3F_GetString( "effectradius" );
	weaponsptr			= G_Q3F_GetString( "weapons" );
	/*holdingptr		= G_Q3F_GetString( "holding" );
	notholdingptr	= G_Q3F_GetString( "notholding" );
	holding = notholding = NULL;*/

		// Work out what players are affected by this give
	teams = 0;
	weapons = 0;
	classes = 0;
	pointcontents = 0;
	if( disabler->mapdata )
	{
		if( player )
		{
			if( disabler->mapdata->flags & Q3F_FLAG_AFFECTTEAM )
				teams |= 1 << player->client->sess.sessionTeam;
			if( disabler->mapdata->flags & Q3F_FLAG_AFFECTNONTEAM )
				teams |= ~((1 << player->client->sess.sessionTeam) | Q3F_TEAM_SPECTATOR);
		}
		data = G_Q3F_KeyPairArrayFind( disabler->mapdata->other, affectteamsptr );
		teams |= data ? G_Q3F_ProcessTeamString( data->value.d.strdata ) : 0;

		data = G_Q3F_KeyPairArrayFind( disabler->mapdata->other, effectradiusptr );
		distance = data ? atof( data->value.d.strdata ) : 0;

		data = G_Q3F_KeyPairArrayFind( disabler->mapdata->other, affectclassesptr );
		classes |= data ? G_Q3F_ProcessClassString( data->value.d.strdata ) : 0;

		data = G_Q3F_KeyPairArrayFind( disabler->mapdata->other, weaponsptr );
		weapons |= data ? G_Q3F_ProcessWeaponString( data->value.d.strdata ) : 0;

		if ( weapons & (1 << WP_AXE) )
			weapons &= ~(1 << WP_AXE);

		if( disabler->mapdata->flags & Q3F_FLAG_ENVIRONMENT )
			pointcontents = trap_PointContents( disabler->s.pos.trBase, disabler - level.gentities ) & MASK_WATER;

		holding = disabler->mapdata->holding;
		notholding = disabler->mapdata->notholding;
		clientstats = disabler->mapdata->clientstats;
	}

		// Work out min and max of our loop
	if( teams || distance )
	{
		minent = level.gentities;
		maxent = &level.gentities[MAX_CLIENTS-1];
	}
	else minent = maxent = player;

	for( current = minent; current && current <= maxent; current++ )
	{
			// For each player, check the ent can affect them
		if( !current->client || !current->inuse )
			continue;		// Not a player, or dead
		if( teams && !(teams & (1 << current->client->sess.sessionTeam) ))
			continue;		// Bad team
		if( classes && !(classes & (1 << current->client->ps.persistant[PERS_CURRCLASS]) ))
			continue;		// Bad class
		if( distance )
		{
			VectorSubtract( current->s.pos.trBase, disabler->s.pos.trBase, vec3 );
			effectfactor = 1.0 - VectorLength( vec3 ) / distance;
			if( effectfactor <= 0 )
				continue;	// Out of range
		}
		else effectfactor = 1;
		if( disabler->mapdata && (disabler->mapdata->flags & Q3F_FLAG_ENVIRONMENT) )
		{
			if( (trap_PointContents( current->s.pos.trBase, current - level.gentities ) & MASK_WATER) !=
				pointcontents )
				continue;	// Different environment
		}
		if( disabler->mapdata && (disabler->mapdata->flags & Q3F_FLAG_LINEOFSIGHT) )
		{
			trap_Trace( &trace, disabler->s.pos.trBase, NULL, NULL, current->s.pos.trBase, disabler-level.gentities, MASK_SOLID );
			if( trace.entityNum != current-level.gentities )
				continue;	// Trace failed, not in line-of-sight
		}
		if( holding && !G_Q3F_CheckHeld( current, holding ) )
			continue;		// Not holding the required ents
		if( notholding && !G_Q3F_CheckNotHeld( current, notholding ) )
			continue;		// Not holding the required ents
		if( clientstats && !G_Q3F_CheckClientStats( current, clientstats, (disabler->mapdata->flags & Q3F_FLAG_ORCLIENTSTATS) ) )
			continue;		// Not having the right stats

#if 0
		// Looks like they're prime candidates for the job. Respawn them.
		tent = G_TempEntity( current->client->ps.origin, EV_PLAYER_TELEPORT_OUT );
		tent->s.clientNum = current->s.clientNum;
		cls = G_Q3F_GetClass(&(current->client->ps));
		if(cls->DeathCleanup)
			cls->DeathCleanup(current);
		ClientSpawn( current );
		tent = G_TempEntity( current->client->ps.origin, EV_PLAYER_TELEPORT_IN );
		tent->s.clientNum = current->s.clientNum;
#endif
	}

		// Trigger ourself inactive again immediately
	G_Q3F_TriggerEntity( disabler, player, Q3F_STATE_INACTIVE, tr, qfalse );
}

void SP_Q3F_misc_disableweapons( gentity_t *ent )
{
	ent->touch = G_Q3F_MiscDisableWeaponsTouch;
}

#if 0

void G_Q3F_MiscFlamerThink (gentity_t *self) {
	trace_t	trace;
	vec3_t startpos, endpos;
	gentity_t		*traceEnt;

	if( self->mapdata->state == Q3F_STATE_DISABLED || self->mapdata->state == Q3F_STATE_INVISIBLE )
	{
		ent->nextthink = level.time + ent->wait + ent->random * Q_flrand(-1.0f, 1.0f);
		return;			// Can't pulse, we've been disabled
	}

	if ( self->enemy ) {
		if ( self->enemy != self ) {
			//VectorCopy ( self->enemy->s.origin, self->s.origin2 );
			self->s.apos.trType = self->enemy->s.pos.trType;
			self->s.apos.trTime = self->enemy->s.pos.trTime;
			self->s.apos.trDuration = self->enemy->s.pos.trDuration;
			VectorCopy( self->enemy->s.pos.trBase, self->s.apos.trBase );
			VectorCopy( self->enemy->s.pos.trDelta, self->s.apos.trDelta );
		} else {
			self->s.apos.trType = TR_STATIONARY;
			VectorCopy( self->s.origin, self->s.apos.trBase );
		}
	} else if ( self->pos2[0] != 0 && self->pos2[1] != 0 && self->pos2[2] != 0 ) {
		self->s.apos.trType = TR_STATIONARY;
		VectorCopy( self->pos2, self->s.apos.trBase );
	}

	if ( self->target_ent ) {
		self->s.pos.trType = self->target_ent->s.pos.trType;
		self->s.pos.trTime = self->target_ent->s.pos.trTime;
		self->s.pos.trDuration = self->target_ent->s.pos.trDuration;
		VectorCopy( self->target_ent->s.pos.trBase, self->s.pos.trBase );
		VectorCopy( self->target_ent->s.pos.trDelta, self->s.pos.trDelta );
	} else if ( self->pos1[0] != 0 && self->pos1[1] != 0 && self->pos1[2] != 0 ) {
		self->s.pos.trType = TR_STATIONARY;
		VectorCopy( self->pos1, self->s.pos.trBase );
	} else { // trace to a random target
		vec3_t dest;
		int r;

		r = rand() % NUMVERTEXNORMALS;

		VectorMA( self->s.apos.trBase, 4096, bytedirs[r], dest );
		trap_Trace ( &trace, self->s.apos.trBase, NULL, NULL, dest, -1, CONTENTS_SOLID );

		if ( trace.fraction < 1.0 ) {
			self->s.pos.trType = TR_STATIONARY;
			VectorCopy( trace.endpos, self->s.pos.trBase );
		}
	}

	if ( self->mapdata->state != Q3F_STATE_DISABLED && self->mapdata->state != Q3F_STATE_INVISIBLE ) {
		fire_mapflame( self, startpos, endpos );

		/* Ensiform - FIXME NOTE Criteria based (specifically allowteam has been tested), */
		/* causes server to crash upon touch */
		BG_EvaluateTrajectory( &self->s.pos, level.time, startpos );
		BG_EvaluateTrajectory( &self->s.apos, level.time, endpos );

		if ( Distance( startpos, endpos ) > 5 ) {
			//VectorSubtract( startpos, endpos, dir );
			//VectorNormalizeFast( dir );

			trap_Trace( &trace, startpos, NULL, NULL, endpos, self->s.number, MASK_SHOT);
			traceEnt = &g_entities[ trace.entityNum ];

			// make sure everyone in the beam gets hit
			while ( traceEnt->takedamage && traceEnt->nextbeamhittime < level.time ) {
					if( traceEnt && traceEnt->client ) {
						if ( G_Q3F_CheckCriteria( traceEnt, self ) ) {
							G_Damage ( traceEnt, self, self->activator, NULL, 
								trace.endpos, self->damage, DAMAGE_NO_KNOCKBACK, MOD_BEAM);
							G_Q3F_TriggerEntity( self, traceEnt, Q3F_STATE_ACTIVE, &trace, qfalse );
						}
						else {
							// Failed criteria, run the failure trigger.
							//if( g_mapentDebug.integer )
							//	G_Printf( "<<< Failed Criteria.\n" );
							G_Q3F_PropogateTrigger( self->mapdata->failtarget, traceEnt, &trace );
						}
					}
					else if( (traceEnt->s.eType == ET_Q3F_SENTRY || ET_Q3F_SUPPLYSTATION) && traceEnt->parent && traceEnt->parent->client ) {
							if ( G_Q3F_CheckCriteria( traceEnt->parent, self ) ) {
								G_Damage ( traceEnt, self, self->activator, NULL, 
									trace.endpos, self->damage, DAMAGE_NO_KNOCKBACK, MOD_BEAM);
								//G_Q3F_TriggerEntity( self, traceEnt, Q3F_STATE_ACTIVE, &trace, qfalse );
							}
					}

					traceEnt->nextbeamhittime = level.time + Q3F_FLAMER_PULSETIME;

					//G_Q3F_TriggerEntity( self, traceEnt, Q3F_STATE_ACTIVE, &trace, qfalse );
				//}

				VectorCopy( trace.endpos, startpos );
				//VectorMA( startpos, 64, dir, startpos);

				trap_Trace( &trace, startpos, NULL, NULL, endpos, self->s.number, MASK_SHOT);
				traceEnt = &g_entities[ trace.entityNum ];
			}
		}
	}

	if ( self->mapdata->state == Q3F_STATE_DISABLED || self->mapdata->state == Q3F_STATE_INVISIBLE ) {
		trap_UnlinkEntity( self );
	} else {
		trap_LinkEntity( self );
	}

	self->nextthink = level.time + FRAMETIME;
}

void G_Q3F_MiscFlamerStart ( gentity_t *self ) {
	gentity_t *ent;

	self->s.eType = ET_Q3F_FLAMER;

	if ( self->target ) {
		ent = G_Find (NULL, FOFS(targetname), self->target);
		if (!ent) {
			G_Printf ("%s at %s: %s is a bad target\n", self->classname, vtos(self->s.origin), self->target);
			G_FreeEntity( self );
			return;
		}
		self->target_ent = ent;
	}

	if( self->target_ent && self->pos1[0] != 0 && self->pos1[1] != 0 && self->pos1[2] != 0 ) {
		G_Printf ("%s at %s: defines a target and a startpos\n", self->classname, vtos(self->s.origin));
		G_FreeEntity( self );
		return;
	}
	
	if( self->message ) {
		ent = G_Find (NULL, FOFS(targetname), self->message);
		if( !ent ) {
			G_Printf ("%s at %s: %s is a bad target2\n", self->classname, vtos(self->s.origin), self->message);
			G_FreeEntity( self );
			return; // No targets by this name.
		}
		self->enemy = ent;
	} else { // the misc_beam is it's own ending point
		self->enemy = self;
	}

	if( self->enemy && self->enemy != self && self->pos2[0] != 0 && self->pos2[1] != 0 && self->pos2[2] != 0) {
		G_Printf ("%s at %s: defines a target2 and an aimpos\n", self->classname, vtos(self->s.origin));
		G_FreeEntity( self );
		return;
	}

	self->think = G_Q3F_MiscFlamerThink;
	self->nextthink = level.time + FRAMETIME;
}

void SP_Q3F_misc_flamethrower( gentity_t *self ) {
	char *str;

/*	if( !self->mapdata )
	{
		// Not much use without extended data
		self->mapdata = G_Alloc( sizeof(q3f_mapent_t) );
	}
	*/

	G_SpawnString( "target2", "", &str );
	if( *str ) {
		self->message = G_NewString( str );
	}

	G_SpawnVector( "startpos", "0 0 0", self->pos1 );
	G_SpawnVector( "aimpos", "0 0 0", self->pos2 );

	G_SpawnFloat( "duration", "2", &self->speed );		// Speed holds the 'duration'

	if ( self->wait <= 0.3 ) {
		self->wait = 2000;
	} else if ( self->wait > 15 ) {
		self->wait = 15000;
	} else {
		self->wait *= 1000;
	}

	if ( self->mapdata && ( self->mapdata->state == Q3F_STATE_ACTIVE || self->mapdata->state == Q3F_STATE_INACTIVE ) )
	{
		self->mapdata->state = Q3F_STATE_INVISIBLE;
		// Should properly be propogated as ET_INVISIBLE in G_SpawnGEntityFromSpawnVars
	}

	// let everything else get spawned before we start firing
	self->think = G_Q3F_MiscFlamerStart;
	self->nextthink = level.time + FRAMETIME;
}
#endif

#if 1

/*QUAKED props_flamethrower (.6 .7 .3) (-8 -8 -8) (8 8 8) TRACKING NOSOUND
the effect occurs when this entity is used
needs to aim at a info_notnull
"duration" how long the effect is going to last for example 1.2 sec 2.7 sec
"random" how long of a random variance so the effect isnt exactly the same each time for example 1.1 sec or 0.2 sec

NOSOUND - silent (duh)
*/
void misc_flamethrower_think( gentity_t *ent ) {
	vec3_t vec, angles;
	gentity_t   *target = NULL;
	// TAT - actually create flamechunks that do damage in this direction
	vec3_t flameDir;

	if ( ent->spawnflags & 1 ) { // tracking
		if ( ent->target ) {
			target = G_Find (NULL, FOFS(targetname), ent->target);
		}

		if ( !target ) {
//			VectorSet (ent->r.currentAngles, 0, 0, 1);	// (SA) wasn't working
			VectorSet( ent->s.apos.trBase, 0, 0, 1 );
			// TAT - try that for the flame too
			VectorSet( flameDir, 0, 0, 1 );
		} else {
			VectorSubtract( target->s.origin, ent->s.origin, vec );
			VectorNormalize( vec );
			vectoangles( vec, angles );
//			VectorCopy (angles, ent->r.currentAngles);	// (SA) wasn't working
			VectorCopy( angles, ent->s.apos.trBase );

			// TAT - we want the vector going the other way for the flame
			VectorSubtract( ent->s.origin, target->s.origin, flameDir );
		}
	} else {
		if ( ent->target ) {
			target = G_Find (NULL, FOFS(targetname), ent->target);
		}

		if ( !target ) {
			// TAT - try that for the flame too
			VectorSet( flameDir, 0, 0, 1 );
		} else {
			// TAT - we want the vector going the other way for the flame
			VectorSubtract( ent->s.origin, target->s.origin, flameDir );
		}
	}

	if ( ( ent->timestamp + ent->speed ) > level.time ) {
		G_AddEvent( ent, EV_ETF_FLAMETHROWER_EFFECT, 0 );

		ent->nextthink = level.time + 50;

		// TAT 11/12/2002
		//		The flamethrower effect above is purely visual
		//		we actual need to create an entity that is the fire and will do damage
		fire_flame( ent, ent->r.currentOrigin, flameDir ); // fire_mapflame

		{
			int rval;
			int rnd;

			if ( ent->random ) {
				rval = ent->random * 1000;
				rnd = rand() % rval;
			} else {
				rnd = 0;
			}

			ent->timestamp = level.time + rnd;
			ent->nextthink = ent->timestamp + 50;
	//		ent->soundLoop = 0;
		}
	}
}

void misc_flamethrower_use( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	int rval;
	int rnd;

	if ( ent->spawnflags & 2 ) {
		ent->spawnflags &= ~2;
		ent->think = NULL;      // (SA) wasn't working
		ent->nextthink = 0;
		return;
	} else {
		ent->spawnflags |= 2;
	}

	if ( ent->random ) {
		rval = ent->random * 1000;
		rnd = rand() % rval;
	} else {
		rnd = 0;
	}

	ent->timestamp = level.time + rnd;

	ent->think = misc_flamethrower_think;
	ent->nextthink = level.time + 50;
}

void misc_flamethrower_init( gentity_t *ent ) {
	gentity_t *target = NULL;
	vec3_t vec;
	vec3_t angles;

	if ( ent->target ) {
		target = G_Find (NULL, FOFS(targetname), ent->target);
	}

	if ( !target ) {
//		VectorSet (ent->r.currentAngles, 0, 0, 1);	//----(SA)
		VectorSet( ent->s.apos.trBase, 0, 0, 1 );
	} else {
		VectorSubtract( target->s.origin, ent->s.origin, vec );
		VectorNormalize( vec );
		vectoangles( vec, angles );
//		VectorCopy (angles, ent->r.currentAngles);	//----(SA)
		VectorCopy( angles, ent->s.apos.trBase );
		VectorCopy( angles, ent->s.angles ); // RF, added to fix weird release build issues
	}

	trap_LinkEntity( ent );
}

void SP_Q3F_misc_flamethrower( gentity_t *ent ) {
//	char *size;
//	float dsize;

	ent->think = misc_flamethrower_init;
	ent->nextthink = level.time + 50;
	ent->use = misc_flamethrower_use;

	G_SetOrigin( ent, ent->s.origin );

	G_SpawnFloat( "duration", "1", &ent->speed );		// Speed holds the 'duration'

	if ( !( ent->speed ) ) {
		ent->speed = 1000;
	} else {
		ent->speed *= 1000;
	}

	if ( !( ent->wait ) ) {
		ent->wait = 2000;
	} else {
		ent->wait *= 1000;
	}

/*
	G_SpawnString( "size", "0", &size );
	dsize = atof( size );
	if ( !dsize ) {
		dsize = 1;
	}
	ent->accuracy = dsize;
	*/
}

#endif

#if 0
void G_Q3F_ClassLimitsUse( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	int			i, num;
	gentity_t	*hit;
	q3f_keypair_t* kp;
	q3f_keypairarray_t *kpa;
	int mask;
	int team;

	kp = G_Q3F_KeyPairArrayFind( self->mapdata->other, G_Q3F_GetString( "affectteams" ));
	if( kp ) {
		mask = G_Q3F_ProcessTeamString( kp->value.d.strdata );
		// only affects specified teams
	} else {
		mask = 0;
		// all teams
	}

	kpa = kp->value.d.keypairarraydata;

	for (i=0 ; i<num ; i++) {
		hit = &g_entities[touch[i]];
		if( hit == self ) {
			continue;
		}

		if( mask ) {
			if(hit->client) {
				team = hit->client->sess.sessionTeam;
			} else if( hit->parent && hit->parent->client) {
				team = hit->parent->client->sess.sessionTeam;
			} else {
				team = 0;
			}

			if(!(mask & (1 << team))) {
				continue;
			}
		}

//		G_Printf("blackhole touch: Classname: %s\n", hit->classname);

		// FIXME: move to some sort of hash table style thingymibober?

		if(hit->s.eType == ET_Q3F_SENTRY) {
			if( G_Q3F_KeyPairArrayFind( kpa, G_Q3F_GetString( "autosentry" ))) {
				G_Damage( hit, self, self, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_UNKNOWN );
			}
		} else if(hit->s.eType == ET_Q3F_SUPPLYSTATION) {
			if( G_Q3F_KeyPairArrayFind( kpa, G_Q3F_GetString( "supplystation" ))) {
				G_Damage( hit, self, self, NULL, NULL, 100000, DAMAGE_NO_PROTECTION, MOD_UNKNOWN );
			}
		} else if(	!strcmp(hit->classname, "pipe") ||
					!strcmp(hit->classname, "grenade") ||
					!strcmp(hit->classname, "rocket") ||
					!strcmp(hit->classname, "napalm") ||
					!strcmp(hit->classname, "nail")) {
			if( G_Q3F_KeyPairArrayFind( kpa, G_Q3F_GetString( "projectiles" ))) {
				G_FreeEntity( hit );
			}
		} else if(	!strcmp(hit->classname, "charge")) {
			if( G_Q3F_KeyPairArrayFind( kpa, G_Q3F_GetString( "charge" ))) {
				G_FreeEntity( hit );
			}
		} else if(	!strcmp(hit->classname, "backpack")) {
			if( G_Q3F_KeyPairArrayFind( kpa, G_Q3F_GetString( "backpacks" ))) {
				G_FreeEntity( hit );
			}
		} else if(	!strcmp(hit->classname, "handgrenade") ||
					!strcmp(hit->classname, "flame") ) {
			if( G_Q3F_KeyPairArrayFind( kpa, G_Q3F_GetString( "grenades" ))) {
				G_FreeEntity( hit );
			}
		}
	}

	self->mapdata->state = Q3F_STATE_INACTIVE; // trigger and reset entity
}

void SP_Q3F_misc_classlimits( gentity_t *ent ) {
	if( !ent->mapdata ) {	// No mapdata means no ent, effectively
		G_FreeEntity( ent );
		return;
	}
}

#endif
