/*
**	g_q3f_controllable.c
**
**	Server-side code for handling 'controllable' objects, such as vehicles
**	or gun turrets.
*/

#include "g_local.h"
#include "g_q3f_mapents.h"
#include "bg_q3f_controllable.h"
#include "g_q3f_playerclass.h"

void G_Q3F_ControlEnd( gentity_t *player )
{
	// Stop a player controlling something.

	gclient_t *client;
	gentity_t *controllable;

	if( !player ||
		!(client = player->client) ||
		!(controllable = client->controllable) )
		return;
	if( controllable->s.otherEntityNum == player->s.number )
		controllable->s.otherEntityNum = -1;
	client->controllable = NULL;
	player->s.extFlags	&= ~EXTF_CONTROL;
	client->ps.extFlags	&= ~EXTF_CONTROL;
}

void G_Q3F_ControlStart( gentity_t *player, gentity_t *controllable )
{
	// Set the controller accordingly.

	gclient_t *client;
	gentity_t *otherplayer;

	if( !player ||
		!(client = player->client) ||
		!controllable )
		return;

	if( client->controllable )
		G_Q3F_ControlEnd( player );		// Remove any existing control first.

	if( controllable->s.otherEntityNum != ENTITYNUM_NONE )
	{
		// Bug? The controllable is still being controlled by someone else. Not for long...

		otherplayer = &g_entities[controllable->s.otherEntityNum];
		if( otherplayer->inuse && otherplayer->client )
			G_Q3F_ControlEnd( otherplayer );
	}

	player->s.extFlags |= EXTF_CONTROL;
	client->ps.extFlags |= EXTF_CONTROL;
	client->controllable = controllable;
	controllable->s.otherEntityNum = player->s.number;
}

extern qboolean BG_Q3F_ControlMove( pmove_t *pmove );

qboolean G_Q3F_Control( gentity_t *ent )
{
	// Perform the BG control mechanism for the specified player.
	// Returns qfalse if the entity cannot be controlled for some reason.

	gclient_t *client;
	gentity_t *controllable;
	pmove_t pm;

	client = ent->client;
	controllable = client->controllable;
	if(	client->pers.connected != CON_CONNECTED ||
		Q3F_IsSpectator( client ) ||
		client->noclip ||
		client->ps.stats[STAT_HEALTH] <= 0 || 
		controllable->s.otherEntityNum != ent->s.number ) 
	{
		G_Q3F_ControlEnd( ent );
		return( qfalse );
	}

		// We're ready to control.
	memset( &pm, 0, sizeof(pm) );
	pm.ps				= &client->ps;
	pm.cmd				= ent->client->pers.cmd;
	pm.tracemask		= MASK_PLAYERSOLID;
	pm.trace			= G_Q3F_ForceFieldTrace;
	pm.pointcontents	= trap_PointContents;
	pm.debugLevel		= g_debugMove.integer;
	pm.noFootsteps		= (g_dmflags.integer & DF_NO_FOOTSTEPS) > 0;
	pm.pmove_fixed		= pmove_fixed.integer | client->pers.pmoveFixed;
	pm.pmove_msec		= pmove_msec.integer;
	pm.cs				= &controllable->s;

	return( BG_Q3F_ControlMove( &pm ) );
}

