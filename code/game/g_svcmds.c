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

// this file holds commands that can be executed by the server console, but not remote clients

#include "g_local.h"
#include "g_q3f_admin.h"
#include "g_q3f_mapselect.h"

#include "g_bot_interface.h"

#ifdef BUILD_LUA
#include "g_lua.h"  
#endif // BUILD_LUA


/*
==============================================================================

PACKET FILTERING
 

You can add or remove addresses from the filter list with:

addip <ip>
removeip <ip>

The ip address is specified in dot format, and any unspecified digits will match any value, so you can specify an entire class C network with "addip 192.246.40".

Removeip will only remove an address specified exactly the same way.  You cannot addip a subnet, then removeip a single host.

listip
Prints the current list of filters.

g_filterban <0 or 1>

If 1 (the default), then ip addresses matching the current list will be prohibited from entering the game.  This is the default setting.

If 0, then only addresses matching the list will be allowed.  This lets you easily set up a private game, or a game that only allows players from your local network.


==============================================================================
*/

// extern	vmCvar_t	g_banIPs;
// extern	vmCvar_t	g_filterBan;

#if 0
typedef struct ipFilter_s
{
	unsigned	mask;
	unsigned	compare;
} ipFilter_t;

#define	MAX_IPFILTERS	1024

static ipFilter_t	ipFilters[MAX_IPFILTERS];
static int			numIPFilters;

/*
=================
StringToFilter
=================
*/
static qboolean StringToFilter (char *s, ipFilter_t *f)
{
	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	
	for (i=0 ; i<4 ; i++)
	{
		b[i] = 0;
		m[i] = 0;
	}
	
	for (i=0 ; i<4 ; i++)
	{
		if (*s < '0' || *s > '9')
		{
			G_Printf( "Bad filter address: %s\n", s );
			return qfalse;
		}
		
		j = 0;
		while (*s >= '0' && *s <= '9')
		{
			num[j++] = *s++;
		}
		num[j] = 0;
		b[i] = atoi(num);
		if (b[i] != 0)
			m[i] = 255;

		if (!*s)
			break;
		s++;
	}
	
	f->mask = *(unsigned *)m;
	f->compare = *(unsigned *)b;
	
	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
static void UpdateIPBans (void)
{
	byte	b[4];
	int		i;
	char	iplist[MAX_INFO_STRING];

	*iplist = 0;
	for (i = 0 ; i < numIPFilters ; i++)
	{
		if (ipFilters[i].compare == 0xffffffff)
			continue;

		*(unsigned *)b = ipFilters[i].compare;
		Com_sprintf( iplist + strlen(iplist), sizeof(iplist) - strlen(iplist), 
			"%i.%i.%i.%i ", b[0], b[1], b[2], b[3]);
	}

	trap_Cvar_Set( "g_banIPs", iplist );
}

/*
=================
G_FilterPacket
=================
*/
qboolean G_FilterPacket (char *from)
{
	int		i;
	unsigned	in;
	byte m[4];
	char *p;

	i = 0;
	p = from;
	while (*p && i < 4) {
		m[i] = 0;
		while (*p >= '0' && *p <= '9') {
			m[i] = m[i]*10 + (*p - '0');
			p++;
		}
		if (!*p || *p == ':')
			break;
		i++, p++;
	}
	
	in = *(unsigned *)m;

	for (i=0 ; i<numIPFilters ; i++)
		if ( (in & ipFilters[i].mask) == ipFilters[i].compare)
			return g_filterBan.integer != 0;

	return g_filterBan.integer == 0;
}

/*
=================
AddIP
=================
*/
static void AddIP( char *str )
{
	int		i;

	for (i = 0 ; i < numIPFilters ; i++)
		if (ipFilters[i].compare == 0xffffffff)
			break;		// free spot
	if (i == numIPFilters)
	{
		if (numIPFilters == MAX_IPFILTERS)
		{
			G_Printf ("IP filter list is full\n");
			return;
		}
		numIPFilters++;
	}
	
	if (!StringToFilter (str, &ipFilters[i]))
		ipFilters[i].compare = 0xffffffffu;

	UpdateIPBans();
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans(void) 
{
	char *s, *t;
	char		str[MAX_TOKEN_CHARS];

	Q_strncpyz( str, g_banIPs.string, sizeof(str) );

	for (t = s = g_banIPs.string; *t; /* */ ) {
		s = strchr(s, ' ');
		if (!s)
			break;
		while (*s == ' ')
			*s++ = 0;
		if (*t)
			AddIP( t );
		t = s;
	}
}
#endif

/*
=================
Svcmd_AddIP_f
=================
*/
void Svcmd_AddIP_f (void)
{
	char		str[MAX_TOKEN_CHARS];
	char		reason[MAX_STRING_CHARS];
	int			time;

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage: addip <ip-mask> [seconds] [reason]\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );
	trap_Argv( 2, reason, sizeof(reason) );
	time = atoi( reason );
	trap_Argv( 3, reason, sizeof(reason) );

	if( trap_Argc() >= 3 && !time )
	{
		// Stop them putting in a reason but no time
		G_Printf( "Use a seconds of -1 to indicate a permanent ban.\n" );
		return;
	}

	AddIP( NULL, str, time, reason );
	G_Q3F_AdminCheckBannedPlayers();
	g_q3f_banCheckTime = G_Q3F_AdminNextExpireBans();
}

/*
=================
Svcmd_RemoveIP_f
=================
*/
void Svcmd_RemoveIP_f (void)
{
	//ipFilter_t	f;
	g_q3f_extIpFilter_t f, *filter;
	q3f_data_t *data;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 2 ) {
		G_Printf("Usage: removeip <ip-mask>\n");
		return;
	}

	trap_Argv( 1, str, sizeof( str ) );

	// Golliwog: Changed for admin stuff
	if( !ipFilters )
	{
		G_Printf( "No bans present.\n" );
		return;
	}
	if( !StringToFilter( NULL, str, &f ) )
		return;
	for( i = -1; (data = G_Q3F_ArrayTraverse( ipFilters, &i )) != NULL; )
	{
		filter = data->d.ptrdata;
		if( filter->mask == f.mask &&
			filter->compare == f.compare )
		{
			G_Q3F_RemString( &filter->reason );
			G_Q3F_ArrayDel( ipFilters, i );
			if( !ipFilters->used )
			{
				G_Q3F_ArrayDestroy( ipFilters );
				ipFilters = NULL;
			}
			G_Printf( "Removed.\n" );
			UpdateIPBans();
			g_q3f_banCheckTime = G_Q3F_AdminNextExpireBans();
			return;
		}
	}
	// Golliwog.

/*	if (!StringToFilter( str, &f))
		return;

	for (i=0 ; i<numIPFilters ; i++) {
		if (ipFilters[i].mask == f.mask	&&
			ipFilters[i].compare == f.compare) {
			ipFilters[i].compare = 0xffffffffu;
			G_Printf ("Removed.\n");

			UpdateIPBans();
			return;
		}
	}*/

	G_Printf ( "Didn't find %s.\n", str );
}

/*
===================
Svcmd_EntityList_f
===================
*/
void	Svcmd_EntityList_f (void) {
	int			e;
	gentity_t		*check;

	check = g_entities;
	for (e = 0; e < level.num_entities ; e++, check++) {
		if ( !check->inuse ) {
			continue;
		}
		G_Printf("%3i:", e);
		switch ( check->s.eType ) {
		case ET_GENERAL:
			G_Printf("ET_GENERAL          ");
			break;
		case ET_PLAYER:
			G_Printf("ET_PLAYER           ");
			break;
		case ET_ITEM:
			G_Printf("ET_ITEM             ");
			break;
		case ET_MISSILE:
			G_Printf("ET_MISSILE          ");
			break;
		case ET_MOVER:
			G_Printf("ET_MOVER            ");
			break;
		case ET_BEAM:
			G_Printf("ET_BEAM             ");
			break;
		case ET_PORTAL:
			G_Printf("ET_PORTAL           ");
			break;
		case ET_SPEAKER:
			G_Printf("ET_SPEAKER          ");
			break;
		case ET_PUSH_TRIGGER:
			G_Printf("ET_PUSH_TRIGGER     ");
			break;
		case ET_TELEPORT_TRIGGER:
			G_Printf("ET_TELEPORT_TRIGGER ");
			break;
		case ET_INVISIBLE:
			G_Printf("ET_INVISIBLE        ");
			break;
/*		case ET_GRAPPLE:
			G_Printf("ET_GRAPPLE          ");
			break;*/
		case ET_Q3F_GRENADE:
			G_Printf("ET_Q3F_GRENADE         ");
			break;
		case ET_Q3F_GOAL:
			G_Printf("ET_Q3F_GOAL         ");
			break;
		case ET_Q3F_HUD:
			G_Printf("ET_Q3F_HUD         ");
			break;
		case ET_Q3F_AGENTDATA:
			G_Printf("ET_Q3F_AGENTDATA         ");
			break;
		case ET_Q3F_SCANNERDATA:
			G_Printf("ET_Q3F_SCANNERDATA         ");
			break;
		case ET_SNIPER_DOT:
			G_Printf("ET_SNIPER_DOT         ");
			break;
		case ET_FLAME:
			G_Printf("ET_FLAME         ");
			break;
		case ET_Q3F_SENTRY:
			G_Printf("ET_Q3F_SENTRY         ");
			break;
		case ET_Q3F_SUPPLYSTATION:
			G_Printf("ET_Q3F_SUPPLYSTATION         ");
			break;
		case ET_Q3F_BEAM:
			G_Printf("ET_Q3F_BEAM         ");
			break;
		case ET_Q3F_MAPSENTRY:
			G_Printf("ET_Q3F_MAPSENTRY         ");
			break;
		case ET_Q3F_PANEL:
			G_Printf("ET_Q3F_PANEL         ");
			break;
		case ET_Q3F_FORCEFIELD:
			G_Printf("ET_Q3F_FORCEFIELD         ");
			break;
		case ET_Q3F_SENTRYCAM:
			G_Printf("ET_Q3F_SENTRYCAM         ");
			break;
		case ET_Q3F_TELEPORTTRANSITION:
			G_Printf("ET_Q3F_TELEPORTTRANSITION         ");
			break;
		case ET_Q3F_SKYPORTAL:
			G_Printf("ET_Q3F_SKYPORTAL         ");
			break;

		default:
			G_Printf("%3i                 ", check->s.eType);
			break;
		}

		if ( check->classname ) {
			G_Printf("%s", check->classname);
		}
		G_Printf("\n");
	}
}

gclient_t	*ClientForString( const char *s ) {
	gclient_t	*cl;
	int			i;
	int			idnum;

	// check for a name match
	for ( i=0 ; i < level.maxclients ; i++ ) {
		cl = &level.clients[i];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( !Q_stricmp( cl->pers.netname, s ) ) {
			return cl;
		}
	}

	// numeric values are just slot numbers
	if ( s[0] >= '0' && s[0] <= '9' ) {
		idnum = atoi( s );
		if ( idnum < 0 || idnum >= level.maxclients ) {
			Com_Printf( "Bad client slot: %i\n", idnum );
			return NULL;
		}

		cl = &level.clients[idnum];
		if ( cl->pers.connected == CON_DISCONNECTED ) {
			G_Printf( "Client %i is not connected\n", idnum );
			return NULL;
		}
		return cl;
	}

	G_Printf( "User %s is not on the server\n", s );

	return NULL;
}

/*
===================
Svcmd_ForceTeam_f

forceteam <player> <team>
===================
*/
void	Svcmd_ForceTeam_f( void ) {
	gclient_t	*cl;
	char		str[MAX_TOKEN_CHARS];

	// find the player
	trap_Argv( 1, str, sizeof( str ) );
	cl = ClientForString( str );
	if ( !cl ) {
		return;
	}

	// set the team
	trap_Argv( 2, str, sizeof( str ) );
	SetTeam( &g_entities[cl - level.clients], str );
}

void	G_ETF_Map( void ) {
	char strmap[128], index[128];
	trap_Argv(1, strmap, 128);
	if(!*strmap) {
		G_Printf("Usage: etfmap <map name> [game index]\n");
		return;
	}

	trap_Argv(2, index, 128); 
	if(!*index) {
		G_Printf("No gameindex specified, defaulting to 1\n");
		Q_strncpyz(index, "1", 128);
	}

	trap_Cvar_Set("g_gameindex", index);
	trap_SendConsoleCommand(EXEC_APPEND, va("map \"%s\"\n", strmap));
}

void	G_ETF_DevMap( void ) {
	char strmap[128], index[128];
	trap_Argv(1, strmap, 128);
	if(!*strmap) {
		G_Printf("Usage: etfdevmap <map name> [game index]\n");
		return;
	}

	trap_Argv(2, index, 128); 
	if(!*index) {
		G_Printf("No gameindex specified, defaulting to 1\n");
		Q_strncpyz(index,  "1", 128);
	}

	trap_Cvar_Set("g_gameindex", index);
	trap_SendConsoleCommand(EXEC_APPEND, va("devmap \"%s\"\n", strmap));
}

void	G_ETF_KickAll( void ) {
	int count, i;
	gentity_t *player;

	count = 0;

	for(i = 0, player = g_entities; i < MAX_CLIENTS && player < &g_entities[MAX_CLIENTS]; i++, player++) {
		if(player->inuse && player->client) {
			if(player->client->pers.localClient && !(player->r.svFlags & SVF_BOT)) {
				continue;
			}

			G_Q3F_DropClient(player, "Kicked");
			count++;
		}
		else if(player->client && player->client->pers.connected >= CON_CONNECTING) {
			if(player->client->pers.localClient && !(player->r.svFlags & SVF_BOT)) {
				continue;
			}

			G_Q3F_DropClient(&g_entities[i], "Kicked");
			count++;
		}
	}

	if(!count) {
		G_Printf("No clients to kick\n");
	}
}

void	G_ETF_KickBots( void ) {
	int count, i;
	gentity_t *player;

	count = 0;

	for(i = 0, player = g_entities; i < MAX_CLIENTS && player < &g_entities[MAX_CLIENTS]; i++, player++) {
		if(player->inuse && player->client) {
			if(!(player->r.svFlags & SVF_BOT)) {
				continue;
			}

			G_Q3F_DropClient(player, "Kicked");
			count++;
		}
		else if(player->client && player->client->pers.connected >= CON_CONNECTING) {
			if(!(player->r.svFlags & SVF_BOT)) {
				continue;
			}

			G_Q3F_DropClient(&g_entities[i], "Kicked");
			count++;
		}
	}

	if(!count) {
		G_Printf("No bots to kick\n");
	}
}

void	G_ETF_ClientKick( void ) {
	char index[128];
	int num, i;
	gentity_t *player;

	trap_Argv(1, index, 128);
	if(!*index) {
		G_Printf("Usage: clientkick <player index>\n");
		return;
	}

	num = atoi(index);

	// sanity check if it's a number
	if(index[0] >= '0' && index[0] <= '9')
	{
		for(i = 0, player = g_entities; i < MAX_CLIENTS && player < &g_entities[MAX_CLIENTS]; i++, player++) {
			if(player->inuse && player->client && player->s.number == num) {
				if(player->client->pers.localClient && !(player->r.svFlags & SVF_BOT))
				{
					G_Printf("Cannot kick host player\n");
					return;
				}

				G_Q3F_DropClient(player, "Kicked");
				return;
			}
			else if(player->client && player->client->pers.connected >= CON_CONNECTING && i == num) {
				if(player->client->pers.localClient && !(player->r.svFlags & SVF_BOT))
				{
					G_Printf("Cannot kick host player\n");
					return;
				}

				G_Q3F_DropClient(&g_entities[i], "Kicked");
				return;
			}
		}
	}

	G_Printf("Invalid player index\n");
}

/* Ensiform - Client Number Only */
/* Ensiform - Reusing clientkick code */
void	G_ETF_Mute( void ) {
	char index[8];
	int num;
	gentity_t *player;

	trap_Argv(1, index, 8);
	if(!*index) {
		G_Printf("Usage: mute <player index>\n");
		return;
	}

	num = atoi(index);

	// sanity check if it's a number
	if(index[0] >= '0' && index[0] <= '9')
	{
		for(player = g_entities; player < &g_entities[MAX_CLIENTS]; player++) {
			if(!player->inuse || !player->client || player->s.number != num)
				continue;

			if(player->client->sess.adminLevel != ADMIN_NONE)
			{
				G_Printf("Cannot mute an admin\n");
				return;
			}

			if(player->client->sess.muted)
			{
				G_Printf("Player is already muted\n");
				return;
			}

			G_Q3F_MuteClient(player, qtrue);
			trap_SendServerCommand( player-g_entities, "cp \"You've been muted!\n\"" );
			trap_SendServerCommand( -1, va("print \"%s^7 has been muted!\n\"", player->client->pers.netname) );
			return;
		}
	}

	G_Printf("Invalid player index\n");
}

/* Ensiform - Client Number Only */
/* Ensiform - Reusing clientkick code */
void	G_ETF_UnMute( void ) {
	char index[8];
	int num;
	gentity_t *player;

	trap_Argv(1, index, 8);
	if(!*index) {
		G_Printf("Usage: unmute <player index>\n");
		return;
	}

	num = atoi(index);

	// sanity check if it's a number
	if(index[0] >= '0' && index[0] <= '9')
	{
		for(player = g_entities; player < &g_entities[MAX_CLIENTS]; player++) {
			if(!player->inuse || !player->client || player->s.number != num)
				continue;

			if(!player->client->sess.muted)
			{
				G_Printf("Player is not muted\n");
				return;
			}

			G_Q3F_MuteClient(player, qfalse);
			trap_SendServerCommand( player-g_entities, "cp \"You've been unmuted!\n\"" );
			trap_SendServerCommand( -1, va("print \"%s^7 has been unmuted!\n\"", player->client->pers.netname) );
			return;
		}
	}

	G_Printf("Invalid player index\n");
}

void	G_ETF_Kick( void ) {
	char name[128];
	gentity_t *player;

	trap_Argv(1, name, 128);
	if(!*name) {
		G_Printf("Usage: kick <player name>\n");
		return;
	}

	for( player = g_entities; player < &g_entities[MAX_CLIENTS]; player++ )
	{
		if( player->inuse && (Q_stricmp(player->client->pers.netname, name) == 0) )
		{
			if(player->client->pers.localClient && !(player->r.svFlags & SVF_BOT))
			{
				G_Printf("Cannot kick host player\n");
				return;
			}

			G_Q3F_DropClient(player, "Kicked");
			return;
		}
		else if( player->client && player->client->pers.connected == CON_CONNECTING && (Q_stricmp(player->client->pers.netname, name) == 0) )
		{
			if(player->client->pers.localClient && !(player->r.svFlags & SVF_BOT))
			{
				G_Printf("Cannot kick host player\n");
				return;
			}

			G_Q3F_DropClient(player, "Kicked");
			return;
		}
	}
	G_Printf("Player not found\n");
}

void	G_ETF_BanNum( void ) {
	char index[128];
	char reason[MAX_STRING_CHARS];
	int num, time, i;
	gentity_t *player;

	
	if(trap_Argc() < 2) {
		G_Printf("Usage: banClient <player index> [seconds] [reason]\n");
		return;
	}

	trap_Argv(1, index, sizeof(index));

	if(!*index) {
		G_Printf("Usage: banClient <player index> [seconds] [reason]\n");
		return;
	}

	num = atoi(index);

	trap_Argv(2, reason, sizeof(reason));
	time = atoi(reason);
	trap_Argv(3, reason, sizeof(reason));

	if(trap_Argc() >= 3 && !time)
	{
		// Stop them putting in a reason but no time
		G_Printf( "Use a seconds of -1 to indicate a permanent ban.\n" );
		return;
	}

	// sanity check if it's a number
	if(index[0] >= '0' && index[0] <= '9')
	{
		for( i = 0, player = g_entities; i < MAX_CLIENTS && player < &g_entities[MAX_CLIENTS]; i++, player++ )
		{
			if( player->inuse && i == num )
			{
				if(player->client->pers.localClient && !(player->r.svFlags & SVF_BOT))
				{
					G_Printf("Cannot ban host player\n");
					return;
				}

				if((player->r.svFlags & SVF_BOT))
				{
					G_Printf("Cannot ban bots\n");
					return;
				}

				AddIP( NULL, player->client->sess.ipStr, time, reason );
				G_Q3F_AdminCheckBannedPlayers();
				g_q3f_banCheckTime = G_Q3F_AdminNextExpireBans();
				return;
			}
			else if( player->client && player->client->pers.connected == CON_CONNECTING && i == num )
			{
				if(player->client->pers.localClient && !(player->r.svFlags & SVF_BOT))
				{
					G_Printf("Cannot ban host player\n");
					return;
				}

				if((player->r.svFlags & SVF_BOT))
				{
					G_Printf("Cannot ban bots\n");
					return;
				}

				AddIP( NULL, player->client->sess.ipStr, time, reason );
				G_Q3F_AdminCheckBannedPlayers();
				g_q3f_banCheckTime = G_Q3F_AdminNextExpireBans();
				return;
			}
		}
	}
	G_Printf("Invalid player index\n");
}

void	G_ETF_BanName( void ) {
	char name[128];
	char reason[MAX_STRING_CHARS];
	int time;
	gentity_t *player;

	
	if(trap_Argc() < 2) {
		G_Printf("Usage: banUser <player name> [seconds] [reason]\n");
		return;
	}

	trap_Argv(1, name, sizeof(name));

	if(!*name) {
		G_Printf("Usage: banUser <player name> [seconds] [reason]\n");
		return;
	}

	trap_Argv(2, reason, sizeof(reason));
	time = atoi(reason);
	trap_Argv(3, reason, sizeof(reason));

	if(trap_Argc() >= 3 && !time)
	{
		// Stop them putting in a reason but no time
		G_Printf( "Use a seconds of -1 to indicate a permanent ban.\n" );
		return;
	}

	for( player = g_entities; player < &g_entities[MAX_CLIENTS]; player++ )
	{
		if( player->inuse && (Q_stricmp(player->client->pers.netname, name) == 0) )
		{
			if(player->client->pers.localClient && !(player->r.svFlags & SVF_BOT))
			{
				G_Printf("Cannot ban host player\n");
				return;
			}

			if((player->r.svFlags & SVF_BOT))
			{
				G_Printf("Cannot ban bots\n");
				return;
			}

			AddIP( NULL, player->client->sess.ipStr, time, reason );
			G_Q3F_AdminCheckBannedPlayers();
			g_q3f_banCheckTime = G_Q3F_AdminNextExpireBans();
			return;
		}
		else if( player->client && player->client->pers.connected == CON_CONNECTING && (Q_stricmp(player->client->pers.netname, name) == 0) )
		{
			if(player->client->pers.localClient && !(player->r.svFlags & SVF_BOT))
			{
				G_Printf("Cannot ban host player\n");
				return;
			}

			if((player->r.svFlags & SVF_BOT))
			{
				G_Printf("Cannot ban bots\n");
				return;
			}

			AddIP( NULL, player->client->sess.ipStr, time, reason );
			G_Q3F_AdminCheckBannedPlayers();
			g_q3f_banCheckTime = G_Q3F_AdminNextExpireBans();
			return;
		}
	}
	G_Printf("Player not found\n");
}

char	*ConcatArgs( int start );

void G_ETF_Echo( void ) {
	G_Printf("%s\n", ConcatArgs(1));
}

/*
=================
ConsoleCommand

=================
*/
qboolean	ConsoleCommand( void ) {
	char	cmd[MAX_TOKEN_CHARS];

	trap_Argv( 0, cmd, sizeof( cmd ) );

#ifdef BUILD_LUA
	if (Q_stricmp (cmd, "lua_status") == 0 ) {
		G_LuaStatus(NULL);
		return qtrue;
	}

	if (Q_stricmp (cmd, "lua_restart") == 0 ) {
		G_LuaShutdown();
		G_LuaInit();
		return qtrue;
	}

	if (Q_stricmp (cmd, "lua_api") == 0 ) {
		G_LuaStackDump();
		return qtrue;
	}

	// *LUA* API callbacks
	if (G_LuaHook_ConsoleCommand (cmd) ) {
		return qtrue;
	}
#endif

	if ( Q_stricmp (cmd, "entitylist") == 0 ) {
		Svcmd_EntityList_f();
		return qtrue;
	}

	if ( Q_stricmp (cmd, "forceteam") == 0 ) {
		Svcmd_ForceTeam_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "game_memory") == 0) {
		Svcmd_GameMem_f();
		return qtrue;
	}

	// Golliwog: Get a breakdown of strings
	if (Q_stricmp (cmd, "game_strings") == 0) {
		Q3F_Svcmd_GameStrings_f();
		return qtrue;
	}
	// Golliwog.

	/*if (Q_stricmp (cmd, "addbot") == 0) {
		Svcmd_AddBot_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removebot") == 0) {
		Svcmd_RemoveBot_f();
		return qtrue;
	}*/

	/*if (Q_stricmp (cmd, "abort_podium") == 0) {
		Svcmd_AbortPodium_f();
		return qtrue;
	}*/

	if (Q_stricmp (cmd, "addip") == 0) {
		Svcmd_AddIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "removeip") == 0) {
		Svcmd_RemoveIP_f();
		return qtrue;
	}

	if (Q_stricmp (cmd, "listip") == 0) {
		trap_SendConsoleCommand( EXEC_NOW, "g_banIPs\n" );
		return qtrue;
	}

	if( Q_stricmp (cmd, "mapvote") == 0) {
		LogExit( "Mapvote called." );
		G_Q3F_MapSelectInit();
		return qtrue;
	}

	if( Q_stricmp( cmd, "admin" ) == 0 ) {
		G_Q3F_AdminCommand( NULL );
		return( qtrue );
	}

	if( Q_stricmp( cmd, "configtest" ) == 0 ) {
		G_Q3F_TestServerConfiguration();
		return( qtrue );
	}

	if( Q_stricmp( cmd, "etfmap" ) == 0 ) {
		G_ETF_Map();
		return( qtrue );
	}
	if( Q_stricmp( cmd, "etfdevmap" ) == 0 ) {
		G_ETF_DevMap();
		return( qtrue );
	}

	if( Q_stricmp( cmd, "clientkick" ) == 0 ) {
		G_ETF_ClientKick();		// do our own kick by client number
		return( qtrue );
	}

	if( Q_stricmp( cmd, "mute" ) == 0 ) {
		G_ETF_Mute();		// mute by client number
		return( qtrue );
	}

	if( Q_stricmp( cmd, "unmute" ) == 0 ) {
		G_ETF_UnMute();		// unmute by client number
		return( qtrue );
	}

	if( Q_stricmp( cmd, "kick" ) == 0 ) {
		G_ETF_Kick();			// do our own kick by client name
		return( qtrue );
	}

	if( Q_stricmp( cmd, "kickall" ) == 0 ) {
		G_ETF_KickAll();			// do our own kick all
		return( qtrue );
	}

	if( Q_stricmp( cmd, "kickbots" ) == 0 ) {
		G_ETF_KickBots();			// do our own kick all bots
		return( qtrue );
	}

	if( Q_stricmp( cmd, "banClient" ) == 0 ) {
		G_ETF_BanNum();			// do our own ban by client number
		return( qtrue );
	}

	if( Q_stricmp( cmd, "banUser" ) == 0 ) {
		G_ETF_BanName();			// do our own ban by client name
		return( qtrue );
	}

#ifdef BUILD_BOTS
	if (Q_stricmp (cmd, "bot") == 0) 
	{
		Bot_Interface_ConsoleCommand(0);
		return qtrue;
	}
#endif

#ifdef DREVIL_BOT_SUPPORT
	if (Q_stricmp (cmd, "bot") == 0) 
	{
		Bot_Interface_ConsoleCommand();
		return qtrue;
	}
#endif
	if( Q_stricmp( cmd, "echo_etf" ) == 0 ) {
		G_ETF_Echo();		// echo to server console
		return( qtrue );
	}

	if (g_dedicated.integer) {
		if (Q_stricmp (cmd, "say") == 0) {
			trap_SendServerCommand( -1, va("print \"server: %s\n\"", ConcatArgs(1) ) );
			return qtrue;
		}
		// everything else will also be printed as a say command
		trap_SendServerCommand( -1, va("print \"server: %s\n\"", ConcatArgs(0) ) );
		return qtrue;
	}

	return qfalse;
}
