/*
**	$Id: g_q3f_waypoint.c,v 1.2 2001/10/02 07:58:50 rr2do2 Exp $
**
**	Parses and processes map information.
**
*/

#include "g_local.h"

/******************************************************************************
***** Create the waypoint list.
****/

static int QDECL WBA_SortFunc( const void *a, const void *b )
{
	g_q3f_waypoint_t *wa, *wb;

	if( !((q3f_data_t *) a)->type )
		return( 1 );
	if( !((q3f_data_t *) b)->type )
		return( -1 );
	wa = (g_q3f_waypoint_t *)((q3f_data_t *) a)->d.intdata;
	wb = (g_q3f_waypoint_t *)((q3f_data_t *) b)->d.intdata;

	return( Q_stricmp( wa->str, wb->str ) );
}

void G_Q3F_WaypointBuildArray()
{
	int iterator, checkIterator;
	q3f_data_t *locData, *checkLocData;
	vec3_t mins, maxs;
	g_q3f_location_t *loc;
	g_q3f_waypoint_t *wp;

	if( !level.locationarray )
		return;

		// Build a set of waypoints.
	level.waypointarray = G_Q3F_ArrayCreate();
	for( iterator = -1; locData = G_Q3F_ArrayTraverse( level.locationarray, &iterator ); )
	{
		for( checkIterator = -1; (checkLocData = G_Q3F_ArrayTraverse( level.locationarray, &checkIterator )) && checkIterator < iterator; )
		{
				// The entry is already listed.
			if( !Q_stricmp(	((g_q3f_location_t *)locData->d.ptrdata)->str, 
							((g_q3f_location_t *)checkLocData->d.ptrdata)->str ) )
			{
				checkLocData = NULL;
				break;
			}
		}
		if( checkLocData )
		{
			// It's a new entry, add it to the waypoint list.

			loc = (g_q3f_location_t *) locData->d.ptrdata;
			VectorCopy( loc->pos, mins );
			VectorCopy( loc->pos, maxs );
			for( ; checkLocData = G_Q3F_ArrayTraverse( level.locationarray, &checkIterator ); )
			{
				if( !Q_stricmp(	((g_q3f_location_t *)locData->d.ptrdata)->str, 
								((g_q3f_location_t *)checkLocData->d.ptrdata)->str ) )
					AddPointToBounds( ((g_q3f_location_t *)checkLocData->d.ptrdata)->pos, mins, maxs );
			}
			wp = G_Alloc( sizeof(g_q3f_waypoint_t) );
			wp->pos[0] = mins[0] + 0.5 * (maxs[0] - mins[0]);
			wp->pos[1] = mins[1] + 0.5 * (maxs[1] - mins[1]);
			wp->pos[2] = mins[2] + 0.5 * (maxs[2] - mins[2]);
			wp->str = loc->str;
			G_Q3F_ArrayAdd( level.waypointarray, Q3F_TYPE_OTHER, 0, (int) wp );
		}
	}
	G_Q3F_ArrayConsolidate( level.waypointarray );		// Free up some space
	qsort( level.waypointarray->data, level.waypointarray->max, sizeof(q3f_data_t), &WBA_SortFunc );
}


/******************************************************************************
***** Send waypoints to all appropriate users.
****/

void G_Q3F_WaypointSend( gentity_t *ent, gentity_t *other, g_q3f_waypoint_t *wp, char *message )
{
	// Send this waypoint to the client.
	// Format is: sendernum x y z "message"

	char *cmd;

	cmd = va(	"waypoint %d %d %d %d \"%s\"\n",
				ent->s.number,
				(int) wp->pos[0], (int) wp->pos[1], (int) wp->pos[2],
				message );
	trap_SendServerCommand(	other->s.number, cmd );
}

extern int ClientNumberFromString( gentity_t *to, char *s );
void G_Q3F_WaypointCommand( gentity_t *ent )
{
	// Command format is #channel waypointid message

	int index, count;
	gentity_t *other;
	char location[64], message[1024];
	char *chanptr;
	g_q3f_waypoint_t *wp;

	if( trap_Argc() != 4 )
	{
		trap_SendServerCommand( ent->s.number, "print \"Usage: waypoint #channel waypointid message\"\n" );
		return;
	}

		// Find who to send the waypoint to.
	trap_Argv( 1, location, sizeof(location) );
	if( location[0] != '#' )
	{
		if( !Q_stricmp( "_self", location ) )
			other = ent;
		else {
			index = ClientNumberFromString( ent, location );
			if( index < 0 )
				return;
			other = &g_entities[index];
		}
	}
	else {
		for( index = 1; index < sizeof(location); index++ )
		{
			if( location[index] >= 'A' && location[index] >= 'Z' )
				location[index] |= 32;
		}
		other = NULL;
		chanptr = G_Q3F_GetString( &location[1] );	// Find a stored copy
		if( !chanptr ) 
		{
			chanptr = va( "print \"Channel '%s' is empty.\"\n", location );
			trap_SendServerCommand( ent->s.number, chanptr );
			return;
		}
	}

	trap_Argv( 2, location, sizeof(location) );
	index = atoi( location );
	if( index == -1 )
	{
		// Debug 'dump all waypoints' command.

		for( index = 0; index < level.waypointarray->used; index++ )
		{
			//chanptr = va( "print \"%s\"\n", ((g_q3f_waypoint_t *) level.waypointarray->data[index].d.ptrdata)->str );
			//trap_SendServerCommand( ent->s.number, chanptr );
			G_Printf( "%d: %s^7\n", index, ((g_q3f_waypoint_t *) level.waypointarray->data[index].d.ptrdata)->str );
		}
		return;
	}
	if( !level.waypointarray ||
		index < 0 || index >= level.waypointarray->used ||
		(index == 0 && location[0] != '0') )
	{
		chanptr = va( "print \"Invalid waypoint no. '%s'.\"\n", location );
		trap_SendServerCommand( ent->s.number, chanptr );
		return;
	}
	wp = (g_q3f_waypoint_t *) level.waypointarray->data[index].d.ptrdata;
	trap_Argv( 3, message, sizeof(message) );

	if( other )
		G_Q3F_WaypointSend( ent, other, wp, message );
	else
	{
		for( index = count = 0; index < level.maxclients; index++ )
		{
			other = &g_entities[index];
			if(	other->client->chatchannels &&
				G_Q3F_ArrayFind( other->client->chatchannels, (int) chanptr ) )
			{
				G_Q3F_WaypointSend( ent, other, wp, message );
				count++;
			}
		}
	}
}
