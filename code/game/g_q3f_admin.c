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
**	g_q3f_admin.c
**
**	Administrative functions for non-RCON users, as well as automated
**	banning and banlist maintenance functions.
*/

#include "g_q3f_admin.h"
#include "g_q3f_team.h"
#include "g_q3f_playerclass.h"
#include "g_q3f_mapselect.h"
#include "g_bot_interface.h"

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

//static ipFilter_t	ipFilters[MAX_IPFILTERS];
//static int			numIPFilters;

q3f_array_t *ipFilters;
int g_q3f_banCheckTime;

#if 0
q3f_array_t *ipMutes;
int g_q3f_muteCheckTime;
#endif

/*static*/ void G_Q3F_AdminPrint( gentity_t *admin, char *fmt, ... )
{
	// Print to the admin, or the console (if done as an RCON command)

	va_list		argptr;
	char		text[1024];

	strcpy( text, "print \"" );
	va_start (argptr, fmt);
	Q_vsnprintf( text + (admin ? 7 : 0), sizeof(text) - (admin ? 7 : 0), fmt, argptr );
	va_end (argptr);

	if( admin )
	{
		// Ensiform : FIXME should this be Q_strcat to protect from overflow?
		strcat( text, "\"" );
		trap_SendServerCommand( admin->s.number, text );
	}
	else {
		trap_Printf( text );
	}
}

qboolean StringToFilter( gentity_t *admin, char *s, g_q3f_extIpFilter_t *f )
{
	// Extended version of the original, accepts single numbers
	// as well as IP addresses. Assumes that if you give it a
	// Class A network, you're nuts :)

	char	num[128];
	int		i, j;
	byte	b[4];
	byte	m[4];
	gentity_t *ent;
	
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

	if( i == 0 )
	{
		// They're nuts, or want to ban a user number

		ent = &g_entities[b[0]];
		if( !ent->client || ent->client->pers.connected != CON_CONNECTED )
		{
			G_Q3F_AdminPrint( admin, "No client with number %i\n", b[0] );
			return( qfalse );
		}
		if( /*(ent->r.svFlags & SVF_BOT) ||*/ ent->client->pers.localClient )
		{
			G_Q3F_AdminPrint( admin, "Cannot ban local clients\n" );
			return( qfalse );
		}
		if( !ent->client->sess.ipStr || !*ent->client->sess.ipStr )
		{
			G_Q3F_AdminPrint( admin, "Unknown client IP?\n" );
			return( qfalse );
		}
		return( StringToFilter( admin, ent->client->sess.ipStr, f ) );
	}
	else {
		f->mask = *(unsigned *)m;
		f->compare = *(unsigned *)b;
	}

	return qtrue;
}

/*
=================
UpdateIPBans
=================
*/
void UpdateIPBans (void)
{
	q3f_data_t *data;
	int index;
	g_q3f_extIpFilter_t *filter;
	byte	b[4];
	char	iplist[MAX_INFO_STRING];
	fileHandle_t fh;
	char *str;

	// Write to the seperate banlist first
	if( trap_FS_FOpenFile( Q3F_ADMIN_BANFILE, &fh, FS_WRITE ) >= 0 )
	{
		str = va( "%d\n", level.time );
		trap_FS_Write( str, strlen(str), fh );

		if( ipFilters )
		{
			index = -1;
			while( ( data = G_Q3F_ArrayTraverse( ipFilters, &index ) ) != NULL )
			{
				filter = data->d.ptrdata;
				*(unsigned *)b = filter->compare;
				str = va( "%i.%i.%i.%i %i %s\n", b[0], b[1], b[2], b[3], filter->endtime, filter->reason );
				trap_FS_Write( str, strlen(str), fh );
			}
		}
		trap_FS_FCloseFile( fh );
	}

	// Set up the config string
	*iplist = 0;
	if( ipFilters )
	{
		index = -1;
		while( ( data = G_Q3F_ArrayTraverse( ipFilters, &index ) ) != NULL )
		{
			filter = data->d.ptrdata;
			if( filter->compare == 0xFFFFFFFF )
				continue;			// Don't store single-IP bans
			if( filter->endtime )
				continue;			// Don't store tempbans (should we?)
			*(unsigned *)b = filter->compare;
			Com_sprintf( iplist + strlen(iplist), sizeof(iplist) - strlen(iplist), 
				"%i.%i.%i.%i ", b[0], b[1], b[2], b[3]);
		}
	}

	trap_Cvar_Set( "g_banIPs", iplist );
}

/*
=================
G_FilterPacket
=================
*/
qboolean G_FilterPacket( char *from, char **reason )
{
	int		i;
	unsigned	in;
	byte m[4];
	char *p;
	q3f_data_t *data;
	g_q3f_extIpFilter_t *filter;

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

	i = -1;
	while( ( data = G_Q3F_ArrayTraverse( ipFilters, &i ) ) != NULL )
	{
		filter = data->d.ptrdata;
		if( (in & filter->mask) == filter->compare )
		{
			if( reason )
				*reason = filter->reason;
			return( g_filterBan.integer != 0 );
		}
	}
	return g_filterBan.integer == 0;
}

/*
=================
AddIP
=================
*/
void AddIP( gentity_t *admin, char *str, int time, char *reason )
{
	g_q3f_extIpFilter_t *filter, *scan;
	q3f_data_t *data;
	int index;

	filter = G_Alloc( sizeof(g_q3f_extIpFilter_t) );
	if( StringToFilter( admin, str, filter ) )
	{
		if( ipFilters )
		{
			// Remove any existing entry matching this first
			for( index = -1; (data = G_Q3F_ArrayTraverse( ipFilters, &index ) ) != NULL; )
			{
				scan = data->d.ptrdata;
				if( scan->mask == filter->mask &&
					scan->compare == filter->compare )
				{
					G_Q3F_RemString( &scan->reason );
					G_Q3F_ArrayDel( ipFilters, index );
					break;
				}
			}
		}
		else ipFilters = G_Q3F_ArrayCreate();

		if( !reason || !*reason )
			reason = "No reason";
		if( time > 0 )
			filter->endtime = level.time + 1000 * time;

		G_Q3F_AddString( &filter->reason, reason );
		G_Q3F_ArrayAdd( ipFilters, Q3F_TYPE_OTHER, 0, (int) filter );
		UpdateIPBans();

//		G_Q3F_AdminPrint( admin, "Added.\n" );
	}
}

/*
=================
G_ProcessIPBans
=================
*/
void G_ProcessIPBans(void) 
{
	char *s, *t, *u, *time, *reason;
	char		str[MAX_TOKEN_CHARS];
	fileHandle_t	fh;
	int				filelen, realtime;
	char *buff;
	qboolean firstline, keeptempbans;

	Q_strncpyz( str, g_banIPs.string, sizeof(str) );

	filelen = trap_FS_FOpenFile( Q3F_ADMIN_BANFILE, &fh, FS_READ );
	if( filelen >= 0 )
	{
		// Read entries from the file.
		buff = G_Alloc( filelen + 1 );
		trap_FS_Read( buff, filelen, fh );
		trap_FS_FCloseFile( fh );
	}
	else buff = NULL;

	for (t = s = g_banIPs.string; *t; /* */ ) {
		s = strchr(s, ' ');
		if (!s)
			break;
		while (*s == ' ')
			*s++ = 0;
		if (*t)
			AddIP( NULL, t, 0, NULL );
		t = s;
	}

	if( buff )
	{
		// Read entries from the file.

		for( s = buff, firstline = keeptempbans = qtrue; *s; s = t )
		{
			for( t = u = s; *t && *t != '\n'; t++ );
			if( *t == '\n' )
				*t++ = 0;
			if( firstline )
			{
				// Look at the timestamp - if it's higher than current, we
				// lose temp bans since they'll be invalid now.
				keeptempbans = (atoi( u ) <= level.time);
				firstline = qfalse;
			}
			else {
				for( time = u; *time && *time != ' '; time++ );
				while( *time == ' ' )
					*time++ = 0;
				for( reason = time; *reason && *reason != ' '; reason++ );
				while( *reason == ' ' )
					*reason++ = 0;
				realtime = time ? atoi(time) : 0;
				if( realtime && !keeptempbans )
					continue;
				if( realtime )
					realtime = realtime - level.time;
				AddIP( NULL, u, realtime, reason ? reason : "No reason" );
			}
		}
		G_Free( buff );
	}
	g_q3f_banCheckTime =  G_Q3F_AdminNextExpireBans();
}


// Mute By IP

#if 0
/*
=================
UpdateIPMutes
=================
*/
void UpdateIPMutes (void)
{
	q3f_data_t *data;
	int index;
	g_q3f_extIpFilter_t *filter;
	byte	b[4];
	fileHandle_t fh;
	char *str;

	// Write to the seperate banlist first
	if( trap_FS_FOpenFile( Q3F_ADMIN_MUTEFILE, &fh, FS_WRITE ) >= 0 )
	{
		str = va( "%d\n", level.time );
		trap_FS_Write( str, strlen(str), fh );

		if( ipMutes )
		{
			index = -1;
			while( data = G_Q3F_ArrayTraverse( ipMutes, &index ) )
			{
				filter = data->d.ptrdata;
				*(unsigned *)b = filter->compare;
				str = va( "%i.%i.%i.%i %i\n", b[0], b[1], b[2], b[3], filter->endtime );
				trap_FS_Write( str, strlen(str), fh );
			}
		}
		trap_FS_FCloseFile( fh );
	}
}

qboolean G_PlayerIPIsMuted( char *from )
{
	int		i;
	unsigned	in;
	byte m[4];
	char *p;
	q3f_data_t *data;
	g_q3f_extIpFilter_t *filter;

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

	i = -1;
	while( data = G_Q3F_ArrayTraverse( ipMutes, &i ) )
	{
		filter = data->d.ptrdata;
		if( (in & filter->mask) == filter->compare )
		{
			return qtrue;
		}
	}
	return qfalse;
}

/*
=================
G_PlayerIsMuted
=================
*/
qboolean G_PlayerIsMuted( gentity_t *ent )
{
	if(!ent || !ent->inuse || !ent->client || ent->client->pers.connected != CON_CONNECTED || level.ceaseFire )
		return qfalse;

	return G_PlayerIPIsMuted( ent->client->sess.ipStr );
}

/*
=================
AddIPMute
=================
*/
void AddIPMute( gentity_t *admin, char *str, int time )
{
	g_q3f_extIpFilter_t *filter, *scan;
	q3f_data_t *data;
	int index;

	filter = G_Alloc( sizeof(g_q3f_extIpFilter_t) );
	if( StringToFilter( admin, str, filter ) )
	{
		if( ipMutes )
		{
			// Remove any existing entry matching this first
			for( index = -1; data = G_Q3F_ArrayTraverse( ipMutes, &index ); )
			{
				scan = data->d.ptrdata;
				if( scan->mask == filter->mask &&
					scan->compare == filter->compare )
				{
					G_Q3F_ArrayDel( ipMutes, index );
					break;
				}
			}
		}
		else ipMutes = G_Q3F_ArrayCreate();

		if( time > 0 )
			filter->endtime = level.time + 1000 * time;

		G_Q3F_ArrayAdd( ipMutes, Q3F_TYPE_OTHER, 0, (int) filter );
		UpdateIPMutes();

//		G_Q3F_AdminPrint( admin, "Added.\n" );
	}
}

/*
=================
G_ProcessIPMutes
=================
*/
void G_ProcessIPMutes(void) 
{
	char *s, *t, *u, *time, *reason;
	char		str[MAX_TOKEN_CHARS];
	fileHandle_t	fh;
	int				filelen, realtime;
	char *buff;
	qboolean firstline, keeptempbans;

	Q_strncpyz( str, g_banIPs.string, sizeof(str) );

	filelen = trap_FS_FOpenFile( Q3F_ADMIN_BANFILE, &fh, FS_READ );
	if( filelen >= 0 )
	{
		// Read entries from the file.
		buff = G_Alloc( filelen + 1 );
		trap_FS_Read( buff, filelen, fh );
		trap_FS_FCloseFile( fh );
	}
	else buff = NULL;

	for (t = s = g_banIPs.string; *t; /* */ ) {
		s = strchr(s, ' ');
		if (!s)
			break;
		while (*s == ' ')
			*s++ = 0;
		if (*t)
			AddIP( NULL, t, 0, NULL );
		t = s;
	}

	if( buff )
	{
		// Read entries from the file.

		for( s = buff, firstline = keeptempbans = qtrue; *s; s = t )
		{
			for( t = u = s; *t && *t != '\n'; t++ );
			if( *t == '\n' )
				*t++ = 0;
			if( firstline )
			{
				// Look at the timestamp - if it's higher than current, we
				// lose temp bans since they'll be invalid now.
				keeptempbans = (atoi( u ) <= level.time);
				firstline = qfalse;
			}
			else {
				for( time = u; *time && *time != ' '; time++ );
				while( *time == ' ' )
					*time++ = 0;
				for( reason = time; *reason && *reason != ' '; reason++ );
				while( *reason == ' ' )
					*reason++ = 0;
				realtime = time ? atoi(time) : 0;
				if( realtime && !keeptempbans )
					continue;
				if( realtime )
					realtime = realtime - level.time;
				AddIP( NULL, u, realtime, reason ? reason : "No reason" );
			}
		}
		G_Free( buff );
	}
	g_q3f_muteCheckTime =  G_Q3F_AdminNextExpireMutes();
}

/*
** Automated tempbanning for in-game activities	
*/

void G_Q3F_AdminCheckMutedPlayers()
{
	// Check all players, mute them if they're banned

	gentity_t *player;
	char *reason;
	char banbuff[MAX_STRING_CHARS];

	for( player = g_entities; player < &g_entities[MAX_CLIENTS]; player++ )
	{
		if( player->inuse && player->client )
		{
			if(G_PlayerIsMuted(player))
				player->client->sess.muted = qtrue;
			else
				player->client->sess.muted = qfalse;
		}
	}
}


/*
**	Maintenance routines
*/

int G_Q3F_AdminNextExpireMutes()
{
	// Expire any 'past' mutes, return the next expiry time.

	g_q3f_extIpFilter_t *filter;
	q3f_data_t *data;
	int			index, nexttime;

	if( !ipMutes )
		return( 0 );

	for( index = -1, nexttime = 0; data = G_Q3F_ArrayTraverse( ipMutes, &index ); )
	{
		filter = data->d.ptrdata;
		if( filter->endtime )
		{
			if( filter->endtime <= level.time )
				G_Q3F_ArrayDel( ipMutes, index );
			else if( !nexttime || filter->endtime < nexttime )
				nexttime = filter->endtime;
		}
	}

	if( !ipMutes->used )
	{
		G_Q3F_ArrayDestroy( ipMutes );
		ipMutes = NULL;
	}
	return( nexttime );
}
#endif


/*
**	Admin functions
*/

static void G_Q3F_AdminStatus( gentity_t *admin )
{
	// Print off a list of clients, so they can easily be kicked.

	gentity_t *player;

	G_Q3F_AdminPrint( admin, "^3 ID Address               Ping Colour Score Name\n" );
	for( player = g_entities; player < &g_entities[level.maxclients]; player++ )
	{
		if( player->inuse && player->client )
		{
			G_Q3F_AdminPrint(	admin, "%3d %21s %4d %6s %5d %s\n",
								player->s.number, player->client->sess.ipStr,
								player->client->ps.ping,
								(player->client->sess.sessionTeam ? g_q3f_teamlist[player->client->sess.sessionTeam].name : "spec"),
								player->client->ps.persistant[PERS_SCORE],
								player->client->pers.netname );
		}
		else if( player->client && player->client->pers.connected == CON_CONNECTING )
		{
			G_Q3F_AdminPrint(	admin, "%3d %21s %4d %6s %5d %s\n",
								(int)(player-g_entities), player->client->sess.ipStr,
								player->client->ps.ping,
								"conn",
								player->client->ps.persistant[PERS_SCORE],
								player->client->pers.netname );
		}
	}
	G_Q3F_AdminPrint( admin, "Complete.\n" );
}

static void G_Q3F_AdminClientIPs( gentity_t *admin )
{
	gentity_t *player;
	char buffer[1024];

	*buffer = '\0';

	if(!admin) {
		return;
	}

	for( player = g_entities; player < &g_entities[level.maxclients]; player++ ) {
		if( player->inuse && player->client ) {
			Q_strcat(buffer, 1024, va("%21s ", player->client->sess.ipStr));
		}
		else if( player->client && player->client->pers.connected == CON_CONNECTING ) {
			Q_strcat(buffer, 1024, va("%21s ", player->client->sess.ipStr));
		}
	}

	trap_SendServerCommand( admin->s.number, va("hud_iplist %s", buffer)); 
}

static void G_Q3F_AdminBannedIPs( gentity_t *admin )
{
	// Display all current IPs.

	g_q3f_extIpFilter_t *filter;
	q3f_data_t *data;
	int			index;
	byte ban[4];
	char buffer[1024];

	*buffer = '\0';

	if( ipFilters ) {

		for( index = -1; ( data = G_Q3F_ArrayTraverse( ipFilters, &index ) ) != NULL; )
		{
			filter = data->d.ptrdata;

			memcpy( ban, &filter->compare, sizeof(ban) );
			Q_strcat(buffer, 1024, va( "%d.%d.%d.%d", ban[0], ban[1], ban[2], ban[3] ));
		}
	}

	trap_SendServerCommand( admin->s.number, va("hud_banlist %s", buffer)); 
}

static void G_Q3F_AdminAddIP( gentity_t *admin )
{
	char		str[MAX_TOKEN_CHARS];
	char		reasonword[MAX_STRING_CHARS];
	char		reason[MAX_STRING_CHARS];
	int			time, curr, maxarg;

	if ( trap_Argc() < 3 ) {
		G_Q3F_AdminPrint( admin, "Usage: admin addip <ip-mask> [seconds] [reason]\n");
		return;
	}

	trap_Argv( 2, str, sizeof( str ) );
	trap_Argv( 3, reason, sizeof(reason) );
	time = atoi( reason );

	if( trap_Argc() >= 4 && !time )
	{
		// Stop them putting in a reason but no time
		G_Q3F_AdminPrint( admin, "Use a seconds of -1 to indicate a permanent ban.\n" );
		return;
	}

	for( curr = 4, maxarg = trap_Argc(), *reason = 0; curr < maxarg; curr++ )
	{
		// Odd bug, \admin addips ipaddr time "string with spaces" doesn't work (as of 1.16n)

		trap_Argv( curr, reasonword, sizeof(reasonword) );
		Q_strcat( reason, sizeof(reason), reasonword );
		if( maxarg - curr > 1 )
			Q_strcat( reason, sizeof(reason), " " );
	}

	AddIP( admin, str, time, reason );
	G_Q3F_AdminCheckBannedPlayers();
	g_q3f_banCheckTime = G_Q3F_AdminNextExpireBans();
}

static void G_Q3F_AdminRemoveIP( gentity_t *admin )
{
	//ipFilter_t	f;
	g_q3f_extIpFilter_t f, *filter;
	q3f_data_t *data;
	int			i;
	char		str[MAX_TOKEN_CHARS];

	if ( trap_Argc() < 3 ) {
		G_Q3F_AdminPrint( admin, "Usage: admin removeip <ip-mask>\n" );
		return;
	}

	trap_Argv( 2, str, sizeof( str ) );

	// Golliwog: Changed for admin stuff
	if( !ipFilters )
	{
		G_Q3F_AdminPrint( admin, "No bans present.\n" );
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
			G_Q3F_AdminPrint( admin, "Removed.\n" );
			UpdateIPBans();
			g_q3f_banCheckTime = G_Q3F_AdminNextExpireBans();
			return;
		}
	}
	// Golliwog.

	G_Q3F_AdminPrint( admin, "Didn't find %s.\n", str );
}

static void G_Q3F_AdminListIPs( gentity_t *admin )
{
	// Display all current IPs.

	g_q3f_extIpFilter_t *filter;
	q3f_data_t *data;
	int			index, time;
	char		*ipstr, *timestr;
	byte ban[4];

	if( !ipFilters )
	{
		G_Q3F_AdminPrint( admin, "There are no bans set.\n" );
		return;
	}
	G_Q3F_AdminPrint( admin, "Ban             Time left  Reason\n" );

	for( index = -1; (data = G_Q3F_ArrayTraverse( ipFilters, &index )) != NULL; )
	{
		filter = data->d.ptrdata;
		timestr = "";
		if( filter->endtime && filter->endtime > level.time )
		{
			time = (filter->endtime - level.time) / 1000;
			if( time > 100 )
				timestr = va( "%0.1f mins", ((float)time) / 60 );
			else timestr = va( "%d secs", time );
		}

		memcpy( ban, &filter->compare, sizeof(ban) );
		ipstr = va( "%d.%d.%d.%d", ban[0], ban[1], ban[2], ban[3] ); 
		G_Q3F_AdminPrint(	admin, "%15s %10s %s\n",
							ipstr, timestr, filter->reason );
	}
	G_Q3F_AdminPrint( admin, "Complete.\n" );
}

void G_Q3F_AdminMatchPassword( gentity_t *admin )
{
	// Setup a match admin, with no control over other
	// admins, but capable of temporarily doing most
	// admin duties.
	int i;
	char matchpass[64];
	qboolean setok;

	if( admin && admin->client->sess.adminLevel < ADMIN_FULL )
	{
		G_Q3F_AdminPrint( admin, "You do not have authority to set match admin passwords.\n" );
		return;
	}

	setok = qfalse;
	if( trap_Argc() >= 3 )
	{
		trap_Argv( 2, matchpass, sizeof(matchpass) );

		if( !Q_stricmp( matchpass, g_adminPassword.string ) )
		{
			G_Q3F_AdminPrint( admin, "Match password cannot be the same as full admin password.\n" );
			return;
		}
		if( *matchpass )
			setok = qtrue;
	}
	if( !setok )
	{
		G_Q3F_AdminPrint( admin, "Usage: admin matchpassword <matchpassword>\n" );
		return;
	}

	/* Check if this is a changed password, if so remove current match admins */
	if (Q_stricmp( g_matchPassword.string, matchpass )) {
		for (i=0; i<MAX_CLIENTS; i++ ) {
			if ( level.clients[i].sess.adminLevel == ADMIN_MATCH )
				level.clients[i].sess.adminLevel = ADMIN_NONE;
		}
	}

	trap_Cvar_Set( "g_matchPassword", matchpass );
	G_Q3F_AdminPrint( admin, "New match password set.\n" );
}

void G_Q3F_AdminMatch( gentity_t *admin )
{
	// Match admin functions - start and stop

	char cmdbuff[16];int i, mode;

	if( trap_Argc() < 3 )
		cmdbuff[0] = 0;
	else trap_Argv( 2, cmdbuff, sizeof(cmdbuff) );

	if( !Q_stricmp( cmdbuff, "warmup" ) ) {
		if( trap_Argc() > 3 ) {
			trap_Argv( 3, cmdbuff, sizeof(cmdbuff) );
			trap_Cvar_Set( "g_warmup", va ( "%i", atoi(cmdbuff)));
		}
		G_Q3F_AdminPrint( admin, "Warmup set at %i seconds.\n", g_warmup.integer );
		return;
	} else if( !Q_stricmp( cmdbuff, "forceready" ) ) {
		if ( g_matchState.integer != MATCH_STATE_READYUP ) {
			G_Q3F_AdminPrint( admin, "Not waiting for players to ready up.\n" );
			return;
		} else {
			for (i=0; i<MAX_CLIENTS; i++ ) {
				if ( level.clients[i].pers.connected < CON_CONNECTED )
					continue;
				if ( Q3F_IsSpectator( &level.clients[i] ) )
					continue;
				level.clients[i].pers.isReady = qtrue;
			}
		}
	} else if( !Q_stricmp( cmdbuff, "start" ) ) {
		if( g_matchState.integer )
		{
			G_Q3F_AdminPrint( admin, "A match is already in progress.\n" );
			return;
		}
		// Start a match in warmup mode.
		G_SetMatchState( MATCH_STATE_PREPARE );
		mode = 0;
		for (i=3; i<=trap_Argc(); i++) {
			trap_Argv( i, cmdbuff, sizeof(cmdbuff) );
			if (!Q_stricmp( cmdbuff, "matchmode"))
				mode |= MATCH_MODE_ACTIVE;
			else if (!Q_stricmp( cmdbuff, "noreadyup"))
				mode |= MATCH_MODE_NOREADYUP;
		}
		trap_Cvar_Set( "g_matchMode", va("%i", mode ));
		G_Q3F_RestartMap();
		return;
	} else if( !Q_stricmp( cmdbuff, "end" ) ) {
		if( !g_matchState.integer )	{
			G_Q3F_AdminPrint( admin, "There is no match running.\n" );
			return;
		}
		trap_Cvar_Set( "g_matchMode", "0" );
		// Stop the match
		level.ceaseFire = qfalse;
		trap_SetConfigstring( CS_FORTS_CEASEFIRE, "0" );
		LogExit( "Admin match end." );
		trap_SendServerCommand( -1, "print \"Match Ended.\n\"" );
	} else {
		G_Q3F_AdminPrint( admin, "Usage: admin match <start|end|warmup|forceready>.\n" );
	}
}

void G_Q3F_AdminCeaseFire( gentity_t *admin )
{
	// Start/stop the ceasefire

	char cmdbuff[8];

	if( trap_Argc() < 3 )
		cmdbuff[0] = 0;
	else trap_Argv( 2, cmdbuff, sizeof(cmdbuff) );

	if( !Q_stricmp( cmdbuff, "on" ) || (!*cmdbuff && !level.ceaseFire) )
	{
		// Enable ceasefire

		if( !level.ceaseFire )
		{
			trap_SetConfigstring( CS_FORTS_CEASEFIRE, "1" );
			level.ceaseFire = qtrue;
			G_Q3F_AdminPrint( admin, "Ceasefire activated.\n" );
			trap_SendServerCommand( -1, "print \"Ceasefire on\n\"" );
		}
		else G_Q3F_AdminPrint( admin, "Ceasefire is already active.\n" );
	}
	else if( !Q_stricmp( cmdbuff, "off" ) || (!*cmdbuff && level.ceaseFire) )
	{
		if( level.ceaseFire )
		{
			level.ceaseFire = qfalse;
			trap_SetConfigstring( CS_FORTS_CEASEFIRE, "0" );
			G_Q3F_AdminPrint( admin, "Ceasefire deactivated.\n" );
			trap_SendServerCommand( -1, "print \"Ceasefire off\n\"" );
		}
		else G_Q3F_AdminPrint( admin, "Ceasefire is not active.\n" );
	}
	else {
		G_Q3F_AdminPrint( admin, "Usage: admin ceasefire <on|off>.\n" );
	}
}

void G_Q3F_AdminMap( gentity_t *admin )
{
	// Restart or change the map

	char mapname[128], buff[128];
	char *ptr;
	fileHandle_t fh;

	trap_Argv( 2, mapname, sizeof(mapname) );

	if( !*mapname )
	{
		// Just display the map name, then exit

		// Ensiform sv_mapname is bogus cvar
		//trap_Cvar_VariableStringBuffer( "sv_mapname", buff, sizeof(buff) );
		trap_Cvar_VariableStringBuffer( "mapname", buff, sizeof(buff) );
		ptr = COM_SkipPath( buff );
		COM_StripExtension( ptr, mapname, sizeof(mapname) );

		G_Q3F_AdminPrint( admin, "Current map is '%s'.\n", mapname );
		G_Q3F_AdminPrint( admin, "Current GameIndex is %d\n", g_gameindex.integer);
	}
	else if( !Q_stricmp( mapname, "restart" ) )
	{
		//Restart the current map (cue horrible crashes)
		G_Q3F_RestartMap();
	}
	else if( !Q_stricmp( mapname, "vote" ) )
	{
		// Call for a map vote (ends current game immediately)
		if( g_mapVote.integer )
		{
			LogExit( "Mapvote called." );
			G_Q3F_MapSelectInit();
		}
		else
			G_Q3F_AdminPrint( admin, "Map voting is disabled.\n" );
	}
	else {
		// Set to this map

		ptr = COM_SkipPath( mapname );
		COM_StripExtension( ptr, buff, sizeof(buff) );
		ptr = va( "maps/%s.bsp", buff );
		if( trap_FS_FOpenFile( ptr, &fh, FS_READ ) >= 0 )
		{
			trap_FS_FCloseFile( fh );
			trap_SendConsoleCommand( EXEC_APPEND, va( "map %s\n", buff ) );
		}
		else G_Q3F_AdminPrint( admin, "Map '%s' is unavailable.\n", buff );
	}
}

void G_Q3F_AdminTimelimit( gentity_t *admin )
{
	// View timelimit or change the timelimit

	char num[8];
	int index;

	trap_Argv( 2, num, 8 );

	if(!*num) {
		G_Q3F_AdminPrint( admin, "Current Timelimit is %d\n", g_timelimit.integer);
		return;
	}
	
	index = atoi(num);
	if(index < 0)
		index = 0;
	if(index > 999)
		index = 999;
	if(num[0] == '+') {
		int newtimelimit = ((level.time - level.startTime) / 60000) + index;		// elapsed minutes + 
		trap_Cvar_Set( "timelimit", va ( "%i", newtimelimit));
	}
	else {
		trap_Cvar_Set( "timelimit", va ( "%i", index));
	}
	trap_Cvar_Update(&g_timelimit);
	G_Q3F_AdminPrint( admin, "Time limit set at %i minutes.\n", g_timelimit.integer );
}

void G_Q3F_AdminCapturelimit( gentity_t *admin )
{
	// View capturelimit or change the capturelimit

	char num[8];
	int index;

	trap_Argv( 2, num, 8 );

	if(!*num) {
		G_Q3F_AdminPrint( admin, "Current Capturelimit is %d\n", g_capturelimit.integer);
		return;
	}
	
	index = atoi(num);
	if(index < 0)
		index = 0;
	if(index > INT_MAX-1)
		index = INT_MAX-1;
	if(num[0] == '+') {
		int newcapturelimit = g_capturelimit.integer + index;		// elapsed minutes + 
		trap_Cvar_Set( "capturelimit", va ( "%i", newcapturelimit));
	}
	else {
		trap_Cvar_Set( "capturelimit", va ( "%i", index));
	}
	trap_Cvar_Update(&g_capturelimit);
	G_Q3F_AdminPrint( admin, "Capture limit set at %i.\n", g_capturelimit.integer );
}

void G_Q3F_AdminGameIndex( gentity_t *admin )
{
	// Restart or change the map

	char num[8];
	int index;

	trap_Argv( 2, num, 8 );

	if(!*num) {
		G_Q3F_AdminPrint( admin, "Current GameIndex is %d\n", g_gameindex.integer);
		return;
	}

	index = atoi(num);

	if(index <= 0) {
		G_Q3F_AdminPrint( admin, "Usage: admin gameindex <index>\n");
		return;
	}

	trap_Cvar_Set("g_gameindex", va("%i", index));
}



void G_ETF_AdminVote( gentity_t *admin )
{
	// admin overrides vote
	char param[8];

	trap_Argv( 2, param, 8 );

	if(!*param) {
		G_Q3F_AdminPrint( admin, "Usage: admin vote yes/no\n");
		return;
	}

	if( !Q_stricmp("yes", param) ) {
		trap_SendServerCommand( -1, "print \"Admin passed the vote.\n\"" );
		level.voteExecuteTime = level.time + 3000;
	} else if( !Q_stricmp("no", param) ) {
		trap_SendServerCommand( -1, "print \"Admin canceled the vote.\n\"" );
	} else {
		G_Q3F_AdminPrint( admin, "Usage: admin vote yes/no\n");
		return;
	}

	level.voteTime = 0;
	trap_SetConfigstring( CS_VOTE_TIME, "" );
}

void G_ETF_AdminPunish( gentity_t *admin )
{
	// kick player to spectator for x minutes
	char minutes[8];
	char player[8];
	int mins = 1;
	int clientNum;
	gentity_t *ent;

	trap_Argv( 2, player, 8 );
	trap_Argv( 3, minutes, 8 );

	if(!*player) {
		G_Q3F_AdminPrint( admin, "Usage: admin punish playerid [minutes (defaults to 1)]\n");
		return;
	}

	clientNum = atoi(player);

	if(*minutes) {
		mins = atoi(minutes);
		if(mins == 0)
			mins = 1;
	}

	level.clients[clientNum].punishTime = level.time + (60 * 1000 * mins);
	trap_SendServerCommand( clientNum, va("punished %d", mins));

	ent = g_entities + clientNum;
	// RR2DO2: if in flyby, reset the flyby
	if( ent->client && ent->client->inFlyBy )
		trap_SendServerCommand( ent-g_entities, "flyby" );

	if( SetTeam(ent , "spectator" ) ) {
		ent->client->switchTeamTime = level.time + 5000;
	}
	ent->client->ps.persistant[PERS_FLAGS] |= PF_JOINEDTEAM;
}

void G_ETF_AdminMute( gentity_t *admin )
{
	// mute player
	char player[8];
	int clientNum;
	gentity_t *ent;

	trap_Argv( 2, player, 8 );

	if(!*player) {
		G_Q3F_AdminPrint( admin, "Usage: admin mute playerid\n");
		return;
	}

	clientNum = atoi(player);

	// sanity check if it's a number
	if(player[0] >= '0' && player[0] <= '9')
	{
		for(ent = g_entities; ent < &g_entities[MAX_CLIENTS]; ent++) {
			if(!ent->inuse || !ent->client || ent->s.number != clientNum)
				continue;

			if(ent->client->sess.adminLevel != ADMIN_NONE)
			{
				G_Q3F_AdminPrint( admin, "Cannot mute an admin\n");
				return;
			}

			if(ent->client->sess.muted)
			{
				G_Q3F_AdminPrint( admin, "Player is already muted\n");
				return;
			}

			G_Q3F_MuteClient(ent, qtrue);
			trap_SendServerCommand( ent-g_entities, "cp \"You've been muted!\n\"" );
			trap_SendServerCommand( -1, va("print \"%s^7 has been muted!\n\"", ent->client->pers.netname) );
			return;
		}
	}

	G_Q3F_AdminPrint( admin, "Invalid player index\n");
}

void G_ETF_AdminUnMute( gentity_t *admin )
{
	// mute player
	char player[8];
	int clientNum;
	gentity_t *ent;

	trap_Argv( 2, player, 8 );

	if(!*player) {
		G_Q3F_AdminPrint( admin, "Usage: admin unmute playerid\n");
		return;
	}

	clientNum = atoi(player);

	// sanity check if it's a number
	if(player[0] >= '0' && player[0] <= '9')
	{
		for(ent = g_entities; ent < &g_entities[MAX_CLIENTS]; ent++) {
			if(!ent->inuse || !ent->client || ent->s.number != clientNum)
				continue;

			if(!ent->client->sess.muted)
			{
				G_Q3F_AdminPrint( admin, "Player is not muted\n");
				return;
			}

			G_Q3F_MuteClient(ent, qfalse);
			trap_SendServerCommand( ent-g_entities, "cp \"You've been unmuted!\n\"" );
			trap_SendServerCommand( -1, va("print \"%s^7 has been unmuted!\n\"", ent->client->pers.netname) );
			return;
		}
	}

	G_Q3F_AdminPrint( admin, "Invalid player index\n");
}

void G_ETF_AdminWarn( gentity_t *admin )
{
	char text[128];
	char player[8];

	trap_Argv( 2, player, 8 );
	trap_Argv( 3, text, 128 );

	if(!*player || !*text) {
		G_Q3F_AdminPrint( admin, "Usage: admin warn playerid \"text\"\n");
		return;
	}

	trap_SendServerCommand( atoi(player), va("cp \"^1Warning from admin!\n^3%s\"", text ));
}


/*
**	The functions called from g_cmds.c
*/

void G_Q3F_RCONPasswordCommand( gentity_t *ent ) {
	char password[MAX_STRING_CHARS];
	char buff[MAX_STRING_CHARS];

	trap_Cvar_VariableStringBuffer("rconpassword", buff, MAX_STRING_CHARS);
	trap_Argv(1, password, MAX_STRING_CHARS);

	if( *buff && !Q_stricmp( password, buff )) {
		ent->client->sess.adminLevel = ADMIN_FULL;
		trap_SendServerCommand( ent->s.number, "hud_auth_rcon 1" );
		trap_SendServerCommand( ent->s.number, "hud_auth_admin 1" );
	} else {
		trap_SendServerCommand( ent->s.number, "hud_auth_rcon 0" );
	}
}

void G_Q3F_AdminPasswordCommand( gentity_t *admin )
{
	// User wants to authenticate as an admin

	char password[MAX_STRING_CHARS];

	if( trap_Argc() < 2 )
	{
		G_Q3F_AdminPrint( admin, "Usage: adminpassword <password>\n" );
		return;
	}
	trap_Argv( 1, password, sizeof(password) );

	if(	g_adminPassword.string && *g_adminPassword.string && !Q_stricmp( password, g_adminPassword.string ) ) {
		admin->client->sess.adminLevel = ADMIN_FULL;
		G_Q3F_AdminPrint( admin, "Admin password accepted.\n" );
		trap_SendServerCommand( admin->s.number, "hud_auth_admin 1" );
	}
	else if( g_matchPassword.string && *g_matchPassword.string && !Q_stricmp( password, g_matchPassword.string ) ) {
		admin->client->sess.adminLevel = ADMIN_MATCH;
		G_Q3F_AdminPrint( admin, "Match admin password accepted.\n" );
		trap_SendServerCommand( admin->s.number, "hud_auth_admin 1" );
	}
	else {
		G_Q3F_AdminPrint( admin, "Wrong password / password not set.\n" );
		trap_SendServerCommand( admin->s.number, "hud_auth_admin 0" );
		if( ++admin->client->sess.adminAttempts > Q3F_ADMIN_MAXAUTH && g_banRules.value > 0 )
			G_Q3F_AdminTempBan( admin, "Repeated authentication attempts", Q3F_ADMIN_TEMPBAN_TIME );
		return;
	}
}

void G_Q3F_AdminCommand( gentity_t *admin )
{
	// User wants to do an admin command

	char command[64];

	if( admin && !(admin->client->sess.adminLevel >= ADMIN_MATCH) )
	{
		G_Q3F_AdminPrint( admin, "Please identify yourself with adminpassword first.\n" );
		return;
	}

	trap_Argv( 1, command, sizeof(command) );
	if( !Q_stricmp( "status", command ) )
		G_Q3F_AdminStatus( admin );
	else if( !Q_stricmp( "addip", command ) )
		G_Q3F_AdminAddIP( admin );
	else if( !Q_stricmp( "removeip", command ) )
		G_Q3F_AdminRemoveIP( admin );
	else if( !Q_stricmp( "listips", command ) )
		G_Q3F_AdminListIPs( admin );
	else if( !Q_stricmp( "match", command ) )
		G_Q3F_AdminMatch( admin );
	else if( !Q_stricmp( "matchpassword", command ) )
		G_Q3F_AdminMatchPassword( admin );
	else if( !Q_stricmp( "ceasefire", command ) )
		G_Q3F_AdminCeaseFire( admin );
	else if( !Q_stricmp( "map", command ) )
		G_Q3F_AdminMap( admin );
	else if( !Q_stricmp( "gameindex", command ) )
		G_Q3F_AdminGameIndex( admin );
	else if( !Q_stricmp( "timelimit", command ) )
		G_Q3F_AdminTimelimit( admin );
	else if( !Q_stricmp( "capturelimit", command ) )
		G_Q3F_AdminCapturelimit( admin );
	else if ( !Q_stricmp( "vote", command ) )
		G_ETF_AdminVote( admin );
	else if ( !Q_stricmp( "warn", command ) )
		G_ETF_AdminWarn( admin );
	else if ( !Q_stricmp( "punish", command ) )
		G_ETF_AdminPunish( admin );
	else if ( !Q_stricmp( "mute", command ) )
		G_ETF_AdminMute( admin );
	else if ( !Q_stricmp( "unmute", command ) )
		G_ETF_AdminUnMute( admin );
#ifdef DREVIL_BOT_SUPPORT
	else if( !Q_stricmp( "bot", command ) )
		Bot_Interface_ConsoleCommand();
#endif
	else {
#ifdef DREVIL_BOT_SUPPORT
		G_Q3F_AdminPrint( admin, "Usage: admin status|addip|removeip|listips|map|gameindex|timelimit|capturelimit|vote|mute|unmute|bot\n" );	// match|matchpassword
#else
		G_Q3F_AdminPrint( admin, "Usage: admin status|addip|removeip|listips|map|gameindex|timelimit|capturelimit|vote|mute|unmute\n" );	// match|matchpassword
#endif
	}
}

void G_Q3F_AdminCommand_ClientOnly( gentity_t* admin ) {
	char command[64];

	if(!admin) {
		return;
	}

	if( !(admin->client->sess.adminLevel >= ADMIN_MATCH) )
	{
		G_Q3F_AdminPrint( admin, "Please identify yourself with adminpassword first.\n" );
		return;
	}

	trap_Argv( 1, command, sizeof(command) );

	if( !Q_stricmp( "iplist", command ) ) {
		G_Q3F_AdminClientIPs( admin );
	} else if( !Q_stricmp( "banlist", command ) ) {
		G_Q3F_AdminBannedIPs( admin );
	} 
}

/*
** Automated tempbanning for in-game activities	
*/

void G_Q3F_AdminCheckBannedPlayers()
{
	// Check all players, kick them if they're banned

	gentity_t *player;
	char *reason;
	char banbuff[MAX_STRING_CHARS];

	for( player = g_entities; player < &g_entities[MAX_CLIENTS]; player++ )
	{
		if( player->inuse && player->client &&
			/*!(player->r.svFlags & SVF_BOT) &&*/
			!player->client->pers.localClient &&
			G_FilterPacket( player->client->sess.ipStr, &reason ) )
		{
			Q_strncpyz( banbuff, "Banned: ", sizeof(banbuff) );
			Q_strcat( banbuff, sizeof(banbuff), reason );
			G_Q3F_DropClient( player, banbuff );
//			trap_DropClient( player->s.number, banbuff );
		}
		else if( player->client && player->client->pers.connected == CON_CONNECTING &&
			/*!(player->r.svFlags & SVF_BOT) &&*/
			!player->client->pers.localClient &&
			G_FilterPacket( player->client->sess.ipStr, &reason ) )
		{
			Q_strncpyz( banbuff, "Banned: ", sizeof(banbuff) );
			Q_strcat( banbuff, sizeof(banbuff), reason );
			G_Q3F_DropClient( player, banbuff );
//			trap_DropClient( player->s.number, banbuff );
		}
	}
}

void G_Q3F_AdminTempBan( gentity_t *player, char *reason, int time )
{
	// Kick a player.

	if( !g_filterBan.integer || !player || !player->client /*|| (player->r.svFlags & SVF_BOT)*/ || player->client->pers.localClient || 	player->client->pers.connected == CON_DISCONNECTED )
		return;

	trap_SendServerCommand( player->s.number, va("print \"Temp ban: %s\"", reason));

	AddIP( NULL, player->client->sess.ipStr, time, reason );
	G_Q3F_AdminCheckBannedPlayers();
	g_q3f_banCheckTime = G_Q3F_AdminNextExpireBans();
}


/*
**	Maintenance routines
*/

int G_Q3F_AdminNextExpireBans()
{
	// Expire any 'past' bans, return the next expiry time.

	g_q3f_extIpFilter_t *filter;
	q3f_data_t *data;
	int			index, nexttime;

	if( !ipFilters )
		return( 0 );

	for( index = -1, nexttime = 0; (data = G_Q3F_ArrayTraverse( ipFilters, &index )) != NULL; )
	{
		filter = data->d.ptrdata;
		if( filter->endtime )
		{
			if( filter->endtime <= level.time )
				G_Q3F_ArrayDel( ipFilters, index );
			else if( !nexttime || filter->endtime < nexttime )
				nexttime = filter->endtime;
		}
	}

	if( !ipFilters->used )
	{
		G_Q3F_ArrayDestroy( ipFilters );
		ipFilters = NULL;
	}
	return( nexttime );
}

void G_Q3F_ShoutcastLoginCommand( gentity_t *player )
{
	// User wants to authenticate as an admin

	char password[MAX_STRING_CHARS];

	if( trap_Argc() < 2 )
	{
		G_Q3F_AdminPrint( player, "Usage: sclogin <password>\n" );
		return;
	}
	trap_Argv( 1, password, sizeof(password) );

	if(	g_shoutcastPassword.string && *g_shoutcastPassword.string && !Q_stricmp( password, g_shoutcastPassword.string ) ) {
		player->client->sess.shoutcaster = qtrue;
		G_Q3F_AdminPrint( player, "Shoutcast password accepted.\n" );
		trap_SendServerCommand( player->s.number, "hud_auth_shoutcast 1" );
	}
	else {
		G_Q3F_AdminPrint( player, "Wrong password / password not set.\n" );
		trap_SendServerCommand( player->s.number, "hud_auth_shoutcast 0" );
		if( ++player->client->sess.adminAttempts > Q3F_ADMIN_MAXAUTH && g_banRules.value > 0 )
			G_Q3F_AdminTempBan( player, "Repeated authentication attempts", Q3F_ADMIN_TEMPBAN_TIME );
		return;
	}
}


