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
**	g_q3f_mapents.c
**
**	Not to be confused with mapdata, this actually DOES stuff with the
**	structures defined in mapdata.
**
*/

#include "g_local.h"
#include "g_q3f_mapdata.h"
#include "g_q3f_mapents.h"
//#include "bg_q3f_playerclass.h"
#include "g_q3f_playerclass.h"
#include "g_q3f_flag.h"
#include "g_q3f_team.h"
#include "g_q3f_weapon.h" /* Ensiform - Included for disease code */
#include "g_bot_interface.h"

/*
**	Static entity parsing functions
*/

const char* team_suffixes[5] = {
	"all",
	"red",
	"blue",
	"yellow",
	"green"
};

const int team_for_suffix[5] = {
	(1 << Q3F_TEAM_RED) + (1 << Q3F_TEAM_BLUE) + (1 << Q3F_TEAM_YELLOW) + (1 << Q3F_TEAM_GREEN),
	(1 << Q3F_TEAM_RED),
	(1 << Q3F_TEAM_BLUE),
	(1 << Q3F_TEAM_YELLOW),
	(1 << Q3F_TEAM_GREEN)
};

char *q3f_statestrings[Q3F_NUM_STATES] = {
	// A good idea to keep these consistent with the definitions.

	"inactive", "active", "disabled", "invisible", "carried"
};

q3f_keypairarray_t *G_Q3F_ProcessStateString( const char *value )
{
	// Process a state string, of the format:
	// target[=state],target[=state],target[=state] etc.

	char *startptr, *stateptr, *endptr;
	char *target;
	char statebuff[12];		// Big enough for any state string
	int strsize, statecode, index;
	char flags;
	q3f_keypairarray_t *targetarray;

	targetarray = G_Q3F_KeyPairArrayCreate();

	for( startptr = (char *) value; *startptr; )
	{
		flags = 0;
		for( endptr = startptr; *endptr && *endptr != '=' && *endptr != ','; endptr++ );
		strsize = endptr - startptr + 1;
		target = G_Alloc( strsize );
		Q_strncpyz( target, startptr, strsize );
		statecode = Q3F_STATE_ACTIVE;
		if( *endptr == '=' )
		{
			// We have an explicit state, work out what it is.

			stateptr = ++endptr;
			if( *stateptr == '~' )
			{
				stateptr = ++endptr;		// Skip the 'force' character
				flags |= Q3F_VFLAG_FORCE;
			}
			for( ; *endptr && *endptr != ','; endptr++ );
			strsize = endptr - stateptr + 1;
			Q_strncpyz( statebuff, stateptr, (strsize > 12) ? 12 : strsize);
			statecode = -1;
			for( index = 0; index < Q3F_NUM_STATES; index++ )
			{
				if( Q_stricmp( q3f_statestrings[index], statebuff ) )
					continue;
				statecode = index;
				break;
			}
			if( statecode == -1 )
				G_Error( "Unknown state '%s' on entity.", statebuff );
		}

			// Add our target to the list.
		G_Q3F_KeyPairArrayAdd( targetarray, target, Q3F_TYPE_INTEGER, flags, statecode );
		G_Free( target );

		if( *(startptr = endptr) )
			startptr++;		// Skip comma.
	}

	if( targetarray->used )
	{
		G_Q3F_KeyPairArrayConsolidate( targetarray );
		G_Q3F_KeyPairArraySort( targetarray );
		return( targetarray );
	}
	G_Q3F_KeyPairArrayDestroy( targetarray );
	return( NULL );
}

int G_Q3F_ProcessInitialStateString( const char *value )
{
	// Process a single state string, without allowing active or carried

	int index;
	for( index = 0; index < Q3F_NUM_STATES; index++ )
	{
		if( index == Q3F_STATE_ACTIVE || index == Q3F_STATE_CARRIED )
			continue;
		if( !Q_stricmp( value, q3f_statestrings[index] ) )
			return( index );
	}
	G_Error( "Unknown initial state '%s' on entity.", value );
	return( Q3F_STATE_INACTIVE );	// Will never be reached
}

q3f_array_t *G_Q3F_ProcessStrings( const char *value )
{
	// Generic string split, return an array of strings split where the commas were.
	// Anything starting with ~ has the force flag set, as usual.

	q3f_array_t *array;
	char *startptr, *ptr, *buff;
	char flags;

	array = G_Q3F_ArrayCreate();

	for( startptr = (char *) value; *startptr; )
	{
		for( ptr = startptr; *ptr && *ptr != ','; ptr++ );
		if( *startptr == '~' )
		{
			flags = Q3F_VFLAG_FORCE;
			startptr++;
		}
		else flags = 0;
		buff = G_Alloc( ptr - startptr + 1 );
		Q_strncpyz( buff, startptr, ptr + 1 - startptr );	// Need the extra one because of the null terminator
		G_Q3F_ArrayAdd( array, Q3F_TYPE_STRING, flags, (int) buff );
		G_Free( buff );

		if( *(startptr = ptr) )
			startptr++;
	}

	if( !array->used )
	{
		G_Q3F_ArrayDestroy( array );
		return( NULL );
	}
	G_Q3F_ArrayConsolidate( array );
	G_Q3F_ArraySort( array );
	return( array );
}

typedef struct g_q3f_pfsmap_s {
	int flags;
	char *str;
} g_q3f_pfsmap_t;
static g_q3f_pfsmap_t pfsmap[] = {
	{ Q3F_FLAG_HIDEACTIVE,		"hideactive"		},
	{ Q3F_FLAG_AFFECTTEAM,		"affectteam"		},
	{ Q3F_FLAG_AFFECTNONTEAM,	"affectnonteam"		},
	{ Q3F_FLAG_EFFECTDROPOFF,	"dropoff"			},
	{ Q3F_FLAG_LINEOFSIGHT,		"lineofsight"		},
	{ Q3F_FLAG_ENVIRONMENT,		"environment"		},
	{ Q3F_FLAG_SHOOTABLE,		"shootable"			},
	{ Q3F_FLAG_REVERSECRITERIA,	"reversecriteria"	},
	{ Q3F_FLAG_REVEALAGENT,		"revealagent"		},
	{ Q3F_FLAG_SHOWCARRY,		"showcarry"			},
	{ Q3F_FLAG_CHARGEABLE,		"chargeable"		},
	{ Q3F_FLAG_ROTATING,		"rotating"			},
	{ Q3F_FLAG_NOSHRINK,		"noshrink"			},
	{ Q3F_FLAG_NODROP,			"nodrop"			},
	{ Q3F_FLAG_ALLOWDEAD,		"allowdead"			},
	{ Q3F_FLAG_ALLOWSAME,		"allowsame"			},
	{ Q3F_FLAG_KEEPONDEATH,		"keepondeath"		},
	{ Q3F_FLAG_USEGAUNTLET,		"usegauntlet"		},
	{ Q3F_FLAG_FAILDIRECTION,	"faildirection"		},
	{ Q3F_FLAG_ALLOWSENTRYLOCK,	"allowsentrylock"	},
	{ Q3F_FLAG_DISGUISECRITERIA,"disguisecriteria"	},
//	{ Q3F_FLAG_FLASHPROTECT,	"flashprotect"		},
	{ Q3F_FLAG_ORCLIENTSTATS,	"orclientstats"		},
	{ Q3F_FLAG_RESETABLE,		"resetable"			},
	{ Q3F_FLAG_DETPIPES,		"detpipes",			},
	{ 0,						""					}
};
int G_Q3F_ProcessFlagString( const char *value )
{
	// Process assorted generic flag fields that alter an entities behaviour

	q3f_array_t *array;
	q3f_data_t *data;
	int index, flags;
	g_q3f_pfsmap_t *pfsptr;

	array = G_Q3F_ProcessStrings( value );
	flags = 0;
	for( index = -1; data = G_Q3F_ArrayTraverse( array, &index ); )
	{
		for( pfsptr = pfsmap; pfsptr->flags; pfsptr++ )
		{
			if( !Q_stricmp( pfsptr->str, data->d.strdata ) )
			{
				flags |= pfsptr->flags;
				break;
			}
		}
		if( !pfsptr->flags )
			G_Printf( "Unknown flag '%s' on entity.\n", data->d.strdata );
	}
	G_Q3F_ArrayDestroy( array );

	return( flags );
}

int G_Q3F_ProcessTeamString( const char *value )
{
	// Process the team string into a bitfield.
	// Assumes strings of the format red,blue,yellow,green etc.

	int bitfield;
	q3f_array_t *array;
	q3f_data_t *data;
	int index;
	int teamnum;

	if( !(array = G_Q3F_ProcessStrings( value )) )
		return( 0 );
	bitfield = 0;
	for( index = -1; data = G_Q3F_ArrayTraverse( array, &index ); )
	{
		teamnum = G_Q3F_GetTeamNum( data->d.strdata );
		if( !teamnum )
			G_Error( "Unknown team '%s' on entity.", data->d.strdata );
		bitfield |= 1 << teamnum;
	}
	G_Q3F_ArrayDestroy( array );

	return( bitfield );
}

/*	WP_SHOTGUN,
	WP_SUPERSHOTGUN,
	WP_NAILGUN,
	WP_SUPERNAILGUN,
	WP_GRENADE_LAUNCHER,
	WP_ROCKET_LAUNCHER,
	WP_SNIPER_RIFLE,
	WP_RAILGUN,
	WP_FLAMETHROWER,
	WP_MINIGUN,
	WP_ASSAULTRIFLE,
	WP_DARTGUN,
	WP_PIPELAUNCHER,
	WP_NAPALMCANNON,*/


static const char *gunNames[] = {
	"",
	"axe",
	"shotgun",
	"supershotgun",
	"nailgun",
	"supernailgun",
	"grenadelauncher",
	"rocketlauncher",
	"sniperrifle",
	"railgun",
	"flamethrower",
	"minigun",
	"assaultrifle",
	"dartgun",
	"pipelauncher",
	"napalmcannon",
	""
};
static const int numGunNames = sizeof(gunNames) / sizeof(const char*);

int G_Q3F_ProcessWeaponString( const char *value )
{
	// Process the team string into a bitfield.
	// Assumes strings of the format axe,rocketlauncher,nailgun,dartgun etc.

	int bitfield;
	q3f_array_t *array;
	q3f_data_t *data;
	int index, wpnindex;

	if( !(array = G_Q3F_ProcessStrings( value )) )
		return( 0 );
	bitfield = 0;
	for( index = -1; data = G_Q3F_ArrayTraverse( array, &index ); )
	{
		for( wpnindex = WP_AXE; wpnindex < WP_NUM_WEAPONS; wpnindex++ )
		{
			if( *gunNames[wpnindex] && !Q_stricmp( gunNames[wpnindex], data->d.strdata ) )
			{
				bitfield |= (1 << wpnindex);
				break;
			}
		}
		if( wpnindex >= WP_NUM_WEAPONS )
			G_Printf( "Unknown weapon '%s' in entity.\n", data->d.strdata );
	}
	G_Q3F_ArrayDestroy( array );

	return( bitfield );
}

int G_Q3F_ProcessClassString( const char *value )
{
	// Process the class string into a bitfield
	// Assumes strings of the format recon,soldier etc.

	int bitfield;
	q3f_array_t *array;
	q3f_data_t *data;
	int index, clsindex;

	if( !(array = G_Q3F_ProcessStrings( value )) )
		return( 0 );
	bitfield = 0;
	for( index = -1; data = G_Q3F_ArrayTraverse( array, &index ); )
	{
		for( clsindex = 0; clsindex < Q3F_CLASS_MAX; clsindex++ )
		{
			if( *bg_q3f_classlist[clsindex]->commandstring &&
				!Q_stricmp( bg_q3f_classlist[clsindex]->commandstring, data->d.strdata ) ) 
			{
				bitfield |= (1 << clsindex);
				break;
			}
		}
		if( clsindex >= Q3F_CLASS_MAX )
			G_Printf( "Unknown class '%s' in entity.\n", data->d.strdata );
	}
	G_Q3F_ArrayDestroy( array );

	return( bitfield );
}

int G_Q3F_ProcessGameIndexString( const char *value )
{
	// Process the gameindex string into a bitfield.
	// Assumes strings of the format 1,2,3 etc.

	int bitfield;
	q3f_array_t *array;
	q3f_data_t *data;
	int index;
	int gameindex;

	if( !(array = G_Q3F_ProcessStrings( value )) )
		return( 0 );
	bitfield = 0;
	for( index = -1; data = G_Q3F_ArrayTraverse( array, &index ); )
	{
		gameindex = atoi( data->d.strdata );
		if( !gameindex ) // integer, DUH
			G_Error( "Invalid gameindex '%s'.", data->d.strdata );
		bitfield |= 1 << gameindex;
	}
	G_Q3F_ArrayDestroy( array );

	return( bitfield );
}

q3f_keypairarray_t *G_Q3F_ProcessGiveString( const char *value )
{
	// Set of commands defining what happens to affected players (ents?)
	// when activated.

	char *startptr, *endptr;
	char *target;
	int strsize, num;
	char flags, inv;
	q3f_keypairarray_t *targetarray;

	targetarray = G_Q3F_KeyPairArrayCreate();

	for( startptr = (char *) value; *startptr; )
	{
		for( endptr = startptr; *endptr && *endptr != '='; endptr++ );
		strsize = endptr - startptr + 1;
		target = G_Alloc( strsize );
		Q_strncpyz( target, startptr, strsize );

		inv = num = flags = 0;
		while( *endptr && *endptr != ',' )
		{
			if( *endptr >= '0' && *endptr <= '9' )
				num = 10*num + *endptr - '0';
			else if( *endptr == '+' )
				{ flags |= Q3F_VFLAG_GIVE_ADD; inv = 0; }
			else if( *endptr == '-' )
				{ flags |= Q3F_VFLAG_GIVE_ADD; inv = 1; }
			else if( *endptr == '~' )
				flags |= Q3F_VFLAG_FORCE;
			endptr++;
		}
		if( inv )
			num = -num;

			// Add our target to the list.
		G_Q3F_KeyPairArrayAdd( targetarray, target, Q3F_TYPE_INTEGER, flags, num );
		G_Free( target );

		if( *(startptr = endptr) )
			startptr++;		// Skip comma.
	}

	if( targetarray->used )
	{
		G_Q3F_KeyPairArrayConsolidate( targetarray );
		G_Q3F_KeyPairArraySort( targetarray );
		return( targetarray );
	}
	
	G_Q3F_KeyPairArrayDestroy( targetarray );
	return( NULL );
}

q3f_keypairarray_t *G_Q3F_ProcessClientStatsString( const char *value )
{
	// Set of evaluations used to check for certain client stats when activated

	char *startptr, *endptr;
	char *target;
	int strsize, num;
	char flags, inv;
	q3f_keypairarray_t *targetarray;

	targetarray = G_Q3F_KeyPairArrayCreate();

	for( startptr = (char *) value; *startptr; )
	{
		for( endptr = startptr; *endptr && ( *endptr != '=' && *endptr != '<' && *endptr != '>' ); endptr++ );
		strsize = endptr - startptr + 1;
		target = G_Alloc( strsize );
		Q_strncpyz( target, startptr, strsize );

		inv = num = flags = 0;

		if( *endptr == '=' )
			flags |= Q3F_VFLAG_CLSTAT_EQ;
		else if( *endptr == '<' ) {
			flags |= Q3F_VFLAG_CLSTAT_LT;
			if( *(endptr + 1) && *(endptr + 1) == '=' ) {
				flags |= Q3F_VFLAG_CLSTAT_EQ;
				endptr++;
			}
		}
		else if( *endptr == '>' ) {
			flags |= Q3F_VFLAG_CLSTAT_GT;
			if( *(endptr + 1) && *(endptr + 1) == '=' ) {
				flags |= Q3F_VFLAG_CLSTAT_EQ;
				endptr++;
			}
		}

		while( *endptr && *endptr != ',' )
		{
			if( *endptr >= '0' && *endptr <= '9' )
				num = 10*num + *endptr - '0';
			else if( *endptr == '-' )
				inv = 1;
			else if( *endptr == '!' )
				flags |= Q3F_VFLAG_CLSTAT_CM;
			endptr++;
		}
		if( inv )
			num = -num;

			// Add our target to the list.
		G_Q3F_KeyPairArrayAdd( targetarray, target, Q3F_TYPE_INTEGER, flags, num );
		G_Free( target );

		if( *(startptr = endptr) )
			startptr++;		// Skip comma.
	}

	if( targetarray->used )
	{
		G_Q3F_KeyPairArrayConsolidate( targetarray );
		G_Q3F_KeyPairArraySort( targetarray );
		return( targetarray );
	}
	
	G_Q3F_KeyPairArrayDestroy( targetarray );
	return( NULL );
}

typedef struct g_q3f_bhtmap_s {
	int type;
	char *str;
} g_q3f_bhtmap_t;
static g_q3f_bhtmap_t bhtmap[] = {
	{ Q3F_BHT_AUTOSENTRY,		"autosentry"	},
	{ Q3F_BHT_SUPPLYSTATION,	"supplystation"	},
	{ Q3F_BHT_PROJECTILES,		"projectiles"	},
	{ Q3F_BHT_CHARGE,			"charge"		},
	{ Q3F_BHT_BACKPACKS,		"backpacks"		},
	{ Q3F_BHT_GRENADES,			"grenades"		},
	{ 0,						""				}
};
int G_Q3F_ProcessBlackHoleTypeString( const char *value )
{
	// Process assorted black hole type fields

	q3f_array_t *array;
	q3f_data_t *data;
	int index, types;
	g_q3f_bhtmap_t *bhtptr;

	array = G_Q3F_ProcessStrings( value );
	types = 0;
	for( index = -1; data = G_Q3F_ArrayTraverse( array, &index ); )
	{
		for( bhtptr = bhtmap; bhtptr->type; bhtptr++ )
		{
			if( !Q_stricmp( bhtptr->str, data->d.strdata ) )
			{
				types |= bhtptr->type;
				break;
			}
		}
		if( !bhtptr->type )
			G_Printf( "Unknown type '%s' on target_blackhole entity.\n", data->d.strdata );
	}
	G_Q3F_ArrayDestroy( array );

	return( types );
}


/*
**	General entity startup/parsing/cleanup functinos
*/

void G_Q3F_ProcessMapField( const char *key, const char *value, gentity_t *ent )
{
	// Process the supplied field and store in in mapdata accordingly.
	// It begins by attempting the fields above, then the 'special cases',
	// and finally just stores them as text.

	char flag;
	char buff[MAX_STRING_CHARS], *srcptr, *dstptr;

	if( !ent->mapdata )
	{
		if( !(ent->mapdata = G_Alloc( sizeof(q3f_mapent_t) )) )
			return;
		ent->mapdata->state = Q3F_STATE_INACTIVE;
		ent->mapdata->statechangetime = 0;
		ent->mapdata->origetype = -1;
	}

	if( !Q_stricmp( "activetarget", key ) || !Q_stricmp( "activatetarget", key ) )
		ent->mapdata->activetarget = G_Q3F_ProcessStateString( value );
	else if( !Q_stricmp( "inactivetarget", key ) || !Q_stricmp( "inactivatetarget", key ) )
		ent->mapdata->inactivetarget = G_Q3F_ProcessStateString( value );
	else if( !Q_stricmp( "disabletarget", key ) || !Q_stricmp( "disabledtarget", key ) )
		ent->mapdata->disabletarget = G_Q3F_ProcessStateString( value );
	else if( !Q_stricmp( "invisibletarget", key ) )
		ent->mapdata->invisibletarget = G_Q3F_ProcessStateString( value );
	else if( !Q_stricmp( "carrytarget", key ) || !Q_stricmp( "carriedtarget", key ) )
		ent->mapdata->carrytarget = G_Q3F_ProcessStateString( value );
	else if( !Q_stricmp( "failtarget", key ) )
		ent->mapdata->failtarget = G_Q3F_ProcessStateString( value );
	else if( !Q_stricmp( "give", key ) )
		ent->mapdata->give = G_Q3F_ProcessGiveString( value );
	else if( !Q_stricmp( "initialstate", key ) )
		ent->mapdata->state = G_Q3F_ProcessInitialStateString( value );
	else if( !Q_stricmp( "groupname", key ) )
		ent->mapdata->groupname = G_Q3F_ProcessStrings( value );
	else if( !Q_stricmp( "flags", key ) )
		ent->mapdata->flags |= G_Q3F_ProcessFlagString( value );
	else if( !Q_stricmp( "allowteams", key ) || !Q_stricmp( "wf_team", key ) )	// wf_team is a compatability thing for WF ents
		ent->mapdata->team = G_Q3F_ProcessTeamString( value );
	else if( !Q_stricmp( "allowclasses", key ) )
		ent->mapdata->classes = G_Q3F_ProcessClassString( value );
	else if( !Q_stricmp( "gameindex", key ) )
		ent->mapdata->gameindex = G_Q3F_ProcessGameIndexString( value );
	else if( !Q_stricmp( "checkstate", key ) )
		ent->mapdata->checkstate = G_Q3F_ProcessStateString( value );
	else if( !Q_stricmp( "holding", key ) )
		ent->mapdata->holding = G_Q3F_ProcessStrings( value );
	else if( !Q_stricmp( "notholding", key ) )
		ent->mapdata->notholding = G_Q3F_ProcessStrings( value );
	else if( !Q_stricmp( "clientstats", key ) )
		ent->mapdata->clientstats = G_Q3F_ProcessClientStatsString( value );

	else {
		// It's not one of these, just store it raw.

		if( !ent->mapdata->other )
			if( !(ent->mapdata->other = G_Q3F_KeyPairArrayCreate()) )
				return;

		if( *value == '~' )
		{
			flag = Q3F_VFLAG_FORCE;
			value++;
		}
		else flag = 0;

		for( srcptr = (char *) value, dstptr = buff; dstptr < (buff + sizeof(buff) - 1) && *srcptr; )
		{
			// Process newlines in the string
			if( *srcptr == '\\' && *(srcptr+1) == 'n' )
			{
				*dstptr++ = '\n';
				srcptr += 2;
			}
			else *dstptr++ = *srcptr++;
		}
		*dstptr = 0;	// Null terminator

			// (Probably) has flaginfo data (we check for faster lookups on \flaginfo command)
		if( !strcmp( key + strlen(key) - 9, "_flaginfo" ) )
			ent->mapdata->flags |= Q3F_FLAG_FLAGINFO;

		// Ensiform: Why was this an else if? what if i want to have both...
		/*else */if( !Q_strncmp( key, "kill_", 5 ) )
			ent->mapdata->flags |= Q3F_FLAG_KILLMSG;
			// (Probably) a sound key, let's precache it to avoid 'first play' misses (?).
			// This could result in a LOT of pre-cached config strings - overflow problems?
		if( !strcmp( key + strlen(key) - 6, "_sound" ) )
			G_SoundIndex( (char *) value );		// This is the value WITHOUT a ~ prefix.

		// Ensiform: Hack fix shitty maps with "q3f_hud" status icons
		if( !strcmp( key + strlen(key) - 7, "_shader" ) && !Q_stricmpn( buff, "textures/q3f_hud", 16 ) ) {
			buff[9] = 'e';
			buff[10] = 't';
		}

		G_Q3F_KeyPairArrayAdd( ent->mapdata->other, (char *) key, Q3F_TYPE_STRING, flag, (int) buff );
	}
}

void G_Q3F_FreeMapData( gentity_t *ent )
{
	// Free any dependent mapdata before deleting an entity.

	if( !ent->mapdata )
		return;
	G_Q3F_KeyPairArrayDestroy( ent->mapdata->activetarget );
	G_Q3F_KeyPairArrayDestroy( ent->mapdata->carrytarget );
	G_Q3F_KeyPairArrayDestroy( ent->mapdata->inactivetarget );
	G_Q3F_KeyPairArrayDestroy( ent->mapdata->disabletarget );
	G_Q3F_KeyPairArrayDestroy( ent->mapdata->invisibletarget );
	G_Q3F_KeyPairArrayDestroy( ent->mapdata->failtarget );
	G_Q3F_KeyPairArrayDestroy( ent->mapdata->give );
	G_Q3F_KeyPairArrayDestroy( ent->mapdata->other );
	G_Q3F_KeyPairArrayDestroy( ent->mapdata->clientstats );
	G_Q3F_KeyPairArrayDestroy( ent->mapdata->spawVars );

	G_Q3F_ArrayDestroy( ent->mapdata->groupname );

	G_Free( ent->mapdata );
	ent->mapdata = NULL;
}


void G_Q3F_CopyMapData( gentity_t *src, gentity_t *dest )
{
	// Copy all extended mapdata from src to dest. This is
	// mostly useful for the likes of doors and platforms
	// that can be triggered, but create a trigger field for
	// the purpose. All strings and subarrays are also
	// duplicated, to prevent nasty accidents if an entity is
	// freed.

	if( !src->mapdata )
	{
		G_Q3F_FreeMapData( dest );
		return;
	}
	if( !dest->mapdata )
		dest->mapdata = G_Alloc( sizeof(q3f_mapent_t) );

	dest->mapdata->team		= src->mapdata->team;
	dest->mapdata->flags	= src->mapdata->flags;
	dest->mapdata->classes	= src->mapdata->classes;

	dest->mapdata->activetarget		= G_Q3F_KeyPairArrayCopy( src->mapdata->activetarget );
	dest->mapdata->carrytarget		= G_Q3F_KeyPairArrayCopy( src->mapdata->carrytarget );
	dest->mapdata->inactivetarget	= G_Q3F_KeyPairArrayCopy( src->mapdata->inactivetarget );
	dest->mapdata->disabletarget	= G_Q3F_KeyPairArrayCopy( src->mapdata->disabletarget );
	dest->mapdata->invisibletarget	= G_Q3F_KeyPairArrayCopy( src->mapdata->invisibletarget );
	dest->mapdata->failtarget		= G_Q3F_KeyPairArrayCopy( src->mapdata->failtarget );
	dest->mapdata->give				= G_Q3F_KeyPairArrayCopy( src->mapdata->give );
	dest->mapdata->other			= G_Q3F_KeyPairArrayCopy( src->mapdata->other );

	dest->mapdata->groupname		= G_Q3F_ArrayCopy( src->mapdata->groupname );
}


static void null_use( gentity_t *ent, gentity_t *other, gentity_t *activator ) {

}

/*
**	Touch/Use/Trigger an entity
*/

qboolean G_Q3F_UseEntity( gentity_t *ent, gentity_t *other, gentity_t *attacker )
{
	// Attempt to use ent, while preventing touching. This is a rather nasty
	// hack, but I don't know a good way of solving it right now.

	void (*touch)(gentity_t *self, gentity_t *other, trace_t *trace);
	void (*use)(gentity_t *self, gentity_t *other, gentity_t *activator);
	trace_t trace;
	qboolean allowuse;

	if( !attacker->client || Q3F_IsSpectator( attacker->client ) || attacker->health <= 0 )
		return( qfalse );		// Dead players can't use triggers.

	//Entities without mapdata just check for ceasefire and else will run their use function
	if( !ent->mapdata )
	{
		if( level.ceaseFire ) 
			return( qfalse );
		if( !ent->use && g_mapentDebug.integer )
		{
			G_Printf( "Attempted G_Q3F_UseEntity(%s) without a use function defined\n", ent->classname ? ent->classname : "" );
			return( qfalse );
		}
		ent->use( ent, other, attacker );
		return( qtrue );
	}

	touch = ent->touch;
	//Replace the use handler with a bogus empty one if there was a use originally
	use = ent->use;
	if ( use )
		ent->use = null_use;
	ent->touch = 0;
	allowuse = G_Q3F_TriggerEntity( ent, attacker, Q3F_STATE_ACTIVE, &trace, 0 );
	ent->touch = touch;
	ent->use = use;
	//Call the original use now if it's allowed
	if( allowuse ) {
		ent->use( ent, other, attacker );
			// This is very nasty, and will quite likely break lots of things
			/* Ensiform - FIXME This appears to break buttons with health applied, */
			/* I'm assuming that it is due to the code in g_mover.c though */
		if( ent->mapdata && ent->mapdata->waittime >= 0 && ent->mapdata->waittime <= level.time )
			ent->mapdata->waittime = level.time + ent->wait + ent->random * Q_flrand(-1.0f, 1.0f);
	}
	return( allowuse );
}

qboolean G_Q3F_TouchEntity( gentity_t *ent, gentity_t *other, trace_t *trace )
{
	// See if ent can touch other, and perform the touch function.
	// Holdable entities behave slightly differently, in that they
	// have an extra state, and can be carried while already active.

	int targetstate;
	q3f_mapent_t *mapdata;
	qboolean isholdable;

	if( !ent->client || Q3F_IsSpectator( ent->client ) || ent->health <= 0 ) {
		// RR2DO2: fix, spectators may go through doors! (spectator teleport)
		if ( !Q3F_IsSpectator( ent->client ) && strcmp( other->classname, "func_door" ) ) {
			return( qfalse );		// Dead players can't use triggers.
		} else {
			other->touch( other, ent, trace );
			return( qtrue );
		}
	}

	if( mapdata = other->mapdata )
	{
		// If there's no mapdata, there's no criteria to check :)

		if(	mapdata->state == Q3F_STATE_DISABLED ||
			mapdata->state == Q3F_STATE_INVISIBLE ||
			(mapdata->flags & Q3F_FLAG_USEGAUNTLET) )
			return( qfalse );						// Can't touch it like this

		targetstate = mapdata->state;		// Fallback in case nethier case matches

		if( targetstate == Q3F_STATE_ACTIVE && (mapdata->flags & Q3F_FLAG_RETOUCH) )
		{
			// It's already active, and 'retouchable' (i.e. doors/plats)
			// Ensiform : We don't want to touch these if we are in ceasefire though!!!

// XreaL BEGIN
#ifdef _ETXREAL
			if( level.ceaseFire || (other->targetname && !other->targetnameAutogenerated) || !G_Q3F_CheckCriteria( ent, other ) )
#else
			if( level.ceaseFire || other->targetname || !G_Q3F_CheckCriteria( ent, other ) )
#endif
// XreaL END
				return( qfalse );
			other->touch( other, ent, trace );
			return( qtrue );
		}

		if(	mapdata->flags & Q3F_FLAG_CARRYABLE )
		{
			if( other->parent == ent && (other->timestamp + 2000) >= level.time )
				return( qfalse );		// 'Last holder' can't touch it again.
			isholdable = qtrue;
			if( (mapdata->state == Q3F_STATE_INACTIVE ||
				mapdata->state == Q3F_STATE_ACTIVE) )
				targetstate = Q3F_STATE_CARRIED;	// Pick up holdable
		}
		else {
			isholdable = qfalse;
			if( mapdata->waittime > level.time )
				return( qfalse );					// We're still waiting
			if( mapdata->state == Q3F_STATE_INACTIVE )
				targetstate = Q3F_STATE_ACTIVE;		// Activate item
		}

		if( mapdata->state == targetstate )
			return( qfalse );						// We're already in our desired state

			// waittime _must_ be less than time or it wouldn't have been touched
			// This should call the touch function
		if( G_Q3F_TriggerEntity( other, ent, targetstate, trace, 0 ) )
		{
			if( targetstate == Q3F_STATE_ACTIVE && mapdata->waittime >= 0 && mapdata->waittime <= level.time )
				mapdata->waittime = level.time + other->wait + other->random * Q_flrand(-1.0f, 1.0f);
			return( qtrue );
		}
		return( qfalse );
	}
	else {
		other->touch( other, ent, trace );		// Call the touch function
	}
	return( qtrue );
}

qboolean G_Q3F_CheckCriteria( gentity_t *activator, gentity_t *ent )
{
	// Return true if the activator passes the criteria on the specified entity.

	qboolean passedCriteria;
	int team, cls;
	q3f_mapent_t *mapdata;

	if( !activator || !activator->client || !ent || !ent->mapdata )
		return( qtrue );

	mapdata	= ent->mapdata;
	team	= activator->client->sess.sessionTeam;
	cls		= activator->client->ps.persistant[PERS_CURRCLASS];
	if( cls == Q3F_CLASS_AGENT && mapdata->flags & Q3F_FLAG_DISGUISECRITERIA )
	{
		if( activator->client->agentclass )
			cls = activator->client->agentclass;
		if( activator->client->agentteam )
			team = activator->client->agentteam;
	}

	{
		qboolean a1, b1, c1, d1, e1, f1;
		a1 = !mapdata->team || (mapdata->team & (1 << team));
		b1 = !mapdata->classes || (mapdata->classes & (1 << cls));
		c1 = !mapdata->holding || G_Q3F_CheckHeld( activator, mapdata->holding );
		d1 = !mapdata->notholding || G_Q3F_CheckNotHeld( activator, mapdata->notholding );
		e1 = !mapdata->clientstats || G_Q3F_CheckClientStats( activator, mapdata->clientstats, (mapdata->flags & Q3F_FLAG_ORCLIENTSTATS));
		f1 = !mapdata->checkstate || G_Q3F_CheckStates( mapdata->checkstate );

	passedCriteria =	(a1) &&									// In the correct team?
						(b1) &&								// In required class?
						(c1) &&				// Holding the required items?
						(d1) &&		// Not holding the required items?
						(e1) &&	// Are our clientstats how we want them?
						(f1);					// Ents in the required state?
	}

	return( (mapdata->flags & Q3F_FLAG_REVERSECRITERIA) ? !passedCriteria : passedCriteria );
}

qboolean G_Q3F_TriggerEntity( gentity_t *ent, gentity_t *activator, int state, trace_t *trace, int force )
{
	// Set the entity to the specified state, message all relevant players,
	// And propogate the reaction. If the state is 'active', call the touch
	// function. Normally, only certain state changes are allowed, but the
	// force flag allows others.

	q3f_mapent_t		*mapdata;
	q3f_keypairarray_t	*propogator;
	int					newstate, oldstate;
	char *actname = 0, *entname = 0;

	if( level.ceaseFire )
		return( qfalse );			// Nothing is triggered during ceasefires

	if( g_matchState.integer > MATCH_STATE_PLAYING && state == Q3F_STATE_CARRIED)
		return( qfalse );			// Can't carry stuff during pre match

	if( !activator )
	{
		activator = g_entities + ENTITYNUM_NONE;
		actname = "ENTITYNUM_NONE";
	}
	
	//if( g_mapentDebug.integer )
	//{		DrEvil : I need this info as triggers for the bots.
		if( activator == g_entities + ENTITYNUM_WORLD )
			actname = "ENTITYNUM_WORLD";
		else if( activator->client )
			actname = activator->client->pers.netname;
		else if( activator->mapdata && activator->mapdata->groupname && activator->mapdata->groupname->data->d.strdata )
			actname = activator->mapdata->groupname->data->d.strdata;
		else actname = activator->classname;

		if( ent->client )
			entname = ent->client->pers.netname;
		else if( ent->mapdata && ent->mapdata->groupname && ent->mapdata->groupname->data->d.strdata )
			entname = ent->mapdata->groupname->data->d.strdata;
		else entname = ent->classname;

		if( g_mapentDebug.integer )
		{
			G_Printf(	">>> Trigger: %s (%d) => %s (%d), %s %s. (%d)\n",
					actname, activator->s.number,
					entname, ent->s.number,
					(force ? "forcing" : "attempting"),
					q3f_statestrings[state],
					level.time);
		}
	//}

	if( !(mapdata = ent->mapdata) )
	{
		if( g_mapentDebug.integer )
			G_Printf( "<<< No mapdata.\n" );
		return( qfalse );			// No extended mapdata?
	}
	if( mapdata->state == state )
	{
		if( g_mapentDebug.integer )
			G_Printf( "<<< Already in desired state.\n" );
		return( qfalse );			// We're already in this state
	}
	if( mapdata->statechangetime == level.time && mapdata->statechangecount > 100 )
	{
			// We're caught in a state-change loop
		G_Printf( "Trigger loop trapped, entity %d (%s).\n", ent->s.number, entname );
		return( qfalse );
	}

	if( !force )
	{
		// Check for disallowed changes.
		if(	(mapdata->state == Q3F_STATE_DISABLED || mapdata->state == Q3F_STATE_INVISIBLE) &&
			(state == Q3F_STATE_ACTIVE || state == Q3F_STATE_CARRIED) )
		{
			if( g_mapentDebug.integer )
				G_Printf( "<<< Cannot trigger to %s from %s.\n", q3f_statestrings[state], q3f_statestrings[mapdata->state] );
			return( qfalse );
		}
		else if( (mapdata->state == Q3F_STATE_CARRIED) &&
			(state == Q3F_STATE_INACTIVE || state == Q3F_STATE_DISABLED || state == Q3F_STATE_INVISIBLE) )
		{
			if( g_mapentDebug.integer )
				G_Printf( "<<< Cannot trigger to %s from %s.\n", q3f_statestrings[state], q3f_statestrings[mapdata->state] );
			return( qfalse );
		}
		else if( (mapdata->state == Q3F_STATE_ACTIVE) &&
			(state == Q3F_STATE_DISABLED || state == Q3F_STATE_INVISIBLE) )
		{
			if( g_mapentDebug.integer )
				G_Printf( "<<< Cannot trigger to %s from %s.\n", q3f_statestrings[state], q3f_statestrings[mapdata->state] );
			return( qfalse );
		}
	}

		// Checking criteria (only if there is an activator to check against)
	if( !force )
	{
/*		passedcriteria =
			(!activator || !activator->client) ||
			((!mapdata->team || (mapdata->team & (1 << activator->client->sess.sessionTeam))) && // In the correct team?
			(!mapdata->classes || (mapdata->classes & (1 << activator->client->ps.persistant[PERS_CURRCLASS]))) &&	// In required class?
			(!mapdata->holding || G_Q3F_CheckHeld( activator, mapdata->holding )) && // Holding the required items?
			(!mapdata->notholding || G_Q3F_CheckNotHeld( activator, mapdata->notholding )) && // Not holding the required items?
			(!mapdata->checkstate || G_Q3F_CheckStates( mapdata->checkstate )));	// Ents in the required state?*/
		if( G_Q3F_CheckCriteria( activator, ent ) )// passedcriteria || ((mapdata->flags & Q3F_FLAG_REVERSECRITERIA) && !passedcriteria) )
		{
			if( g_mapentDebug.integer )
				G_Printf( "    Passed criteria.\n" );

			if(	((mapdata->flags & Q3F_FLAG_CARRYABLE) && state == Q3F_STATE_CARRIED) ||
				(!(mapdata->flags & Q3F_FLAG_CARRYABLE) && state == Q3F_STATE_ACTIVE) )
			{
				if( g_mapentDebug.integer )
					G_Printf( "    Processing 'give'.\n" );
				G_Q3F_MapGive( activator, ent );	// Give 'em what they're due (only in highest state).
			}
		}
		else {
			// Failed criteria, run the failure trigger.
			if( g_mapentDebug.integer )
				G_Printf( "<<< Failed Criteria.\n" );
			if( mapdata->statethink )
				mapdata->statethink( ent, activator, -1, mapdata->state, force, trace );
			propogator = mapdata->failtarget;
			G_Q3F_PropogateTrigger( propogator, activator, trace );
			return( qfalse );
		}
	}

	oldstate = mapdata->state;

	if( mapdata->statethink )
	{
		// We run custom "state" processing here.

		newstate = mapdata->statethink( ent, activator, state, mapdata->state, force, trace );
		if( oldstate != mapdata->state )
		{
			// This is nasty, but must be handled... it means that something has actually triggered
			// the same entity _again_ indside the statethink call, and we shouldn't undo all it's
			// hard work.
			if( g_mapentDebug.integer )
				G_Printf( "<<< State changed inside code filter, aborting trigger process!\n" );
			return( qfalse );
		}
		if( newstate < 0 )
		{
			if( g_mapentDebug.integer )
				G_Printf( "<<< Failed code filter.\n" );
			propogator = mapdata->failtarget;
			G_Q3F_PropogateTrigger( propogator, activator, trace );
			return( qfalse );
		}
		if( newstate == mapdata->state )
		{
			if( g_mapentDebug.integer )
				G_Printf( "<<< State overridden by code filter to existing state.\n" );
			return( qfalse );			// We're already in this state
		}
		if( newstate != state )
		{
			if( g_mapentDebug.integer )
				G_Printf( "    State overridden by code filter to %s.\n", q3f_statestrings[newstate] );
			state = newstate;
		}
	}


	if( mapdata->statechangetime == level.time )
		mapdata->statechangecount++;
	else {
		mapdata->statechangetime	= level.time;	// Prevent circular loops.
		mapdata->statechangecount	= 1;
	}
	mapdata->state				= state;		// Set our state

	if( g_mapentDebug.integer )
		G_Printf( "    State set, state count %d.\n", mapdata->statechangecount );

	// We've survived this far, we'll try affecting players.

#ifdef BUILD_BOTS
	// Send a trigger to the bots.
	if(ent->client)
		Bot_Util_SendTrigger(ent, activator, entname, q3f_statestrings[state]);
	else if(ent->mapdata && ent->mapdata->groupname && ent->mapdata->groupname->data->d.strdata)
		Bot_Util_SendTrigger(ent, activator, entname, q3f_statestrings[state]);
	else
	{
		static char     newentname[256];
		newentname[0] = '\0';
		Com_sprintf(newentname, 256, "%s_%i", ent->classname, (int)(ent - g_entities));
		Bot_Util_SendTrigger(ent, activator, newentname, q3f_statestrings[state]);
	}
#endif
#ifdef DREVIL_BOT_SUPPORT
	{
		static TriggerInfo ti;
		ti.m_TagName = entname;
		ti.m_Action = q3f_statestrings[state];
		ti.m_Activator = activator;
		ti.m_Entity = ent;
		Bot_SendTrigger(&ti);
	}			
#endif

	G_Q3F_StateMessage( ent, activator );			// Print off any relevant messages.

	mapdata->lastTriggerer = activator;				// Keep a copy of this for use by automatic inactive switches

	if( (state == Q3F_STATE_INVISIBLE || (state == Q3F_STATE_ACTIVE && mapdata->flags & Q3F_FLAG_HIDEACTIVE)) &&
		mapdata->origetype == -1 )
	{
		// We're invisible, vanish ourselves.
		mapdata->origetype = ent->s.eType;
		ent->s.eType = ET_INVISIBLE;
		if( g_mapentDebug.integer )
			G_Printf( "    Rendered invisible.\n" );
	}
	else if(	!(state == Q3F_STATE_INVISIBLE && (state == Q3F_STATE_ACTIVE && mapdata->flags & Q3F_FLAG_HIDEACTIVE)) &&
				mapdata->origetype != -1 )
	{
		// We're no longer invisible. If we're a goal, also have a fade-in
		ent->s.eType = mapdata->origetype;
		mapdata->origetype = -1;

		if( ent->s.eType == ET_Q3F_GOAL && !(mapdata->flags & Q3F_FLAG_NOSHRINK) )
		{
			ent->s.time = 0;
			ent->s.time2 = level.time;		// 'Shrink in' time.
		}

		if( g_mapentDebug.integer )
			G_Printf( "    Rendered visible.\n" );
	}

		// Do we want to strip off disguises?
	if( activator &&
		(mapdata->flags & Q3F_FLAG_REVEALAGENT) &&
		activator->client &&
		activator->client->agentdata &&
		( (activator->client->agentdata->s.modelindex2 & Q3F_AGENT_DISGUISEMASK) == 1 || (activator->client->agentdata->s.modelindex2 & Q3F_AGENT_INVISMASK) == 4 ) )
	{
		trap_SendServerCommand( activator->s.number, "print \"You've lost your disguise!\n" );
		G_Q3F_StopAgentDisguise( activator );
	}

		// 'Touch' an entity.
	if( state == Q3F_STATE_CARRIED )
	{
		if( g_mapentDebug.integer )
			G_Printf( "    Executing touch() function.\n" );
		if( mapdata->flags & Q3F_FLAG_CARRYABLE )
		{
			if( ent->touch )
				ent->touch( ent, activator, trace );
		}
		else if( strcmp( ent->classname, "func_hud" ) /*&& strcmp( ent->classname, "func_objectiveicon" )*/ )
			G_Printf( "Attempting to carry non-carryable ent '%s'.\n", entname );
	}
	else if( state == Q3F_STATE_ACTIVE )
	{
		if( ent->use || ent->touch )
		{
			if( mapdata->flags & Q3F_FLAG_CARRYABLE )
			{
				if( g_mapentDebug.integer )
					G_Printf( "    Dropping entity.\n" );
				G_Q3F_DropFlag( ent );
			}
			else {
				if( ent->use )
				{
					if( g_mapentDebug.integer )
						G_Printf( "    Executing use() function.\n" );
					ent->use( ent, ent, activator );
				}
				else {
					if( g_mapentDebug.integer )
						G_Printf( "    Executing touch() function.\n" );
					ent->touch( ent, activator, trace );
				}
			}
			if( ent->inuse && ent->mapdata->waittime >= 0 && ent->mapdata->waittime < ent->nextthink )
			{
				if( g_mapentDebug.integer )
					G_Printf( "    Waittime set to %d.\n", ent->nextthink );
				ent->mapdata->waittime = ent->nextthink;
			}
		}
		else {
			// Some ents have no use or touch, but expect to fall inactive again
			ent->mapdata->waittime = level.time + ent->wait + Q_flrand(-1.0f, 1.0f) * ent->random;
		}
	}
	else if( state == Q3F_STATE_INACTIVE || state == Q3F_STATE_DISABLED || state == Q3F_STATE_INVISIBLE )
	{
		if( ent->think )
		{
			if( ent->nextthink >= level.time || (ent->wait < 0) || oldstate == Q3F_STATE_CARRIED )	// Accelerate think time, or trigger 'holding' entity
			{
				if( g_mapentDebug.integer )
					G_Printf( "    Executing think() function.\n" );
				ent->nextthink = 0;
				ent->think( ent );
			}
			if( ent->inuse && ent->mapdata->waittime >= 0 && ent->mapdata->waittime < ent->nextthink )
			{
				if( g_mapentDebug.integer )
					G_Printf( "    Waittime set to %d.\n", ent->nextthink );
				ent->mapdata->waittime = ent->nextthink;
			}
		}
		else {
			ent->mapdata->waittime = 0;
		}
	}

	if( !ent->inuse )		// Make sure it hasn't vanished beneath us
	{
		if( g_mapentDebug.integer )
			G_Printf( "<<< Entity deleted self.\n" );
		return( qtrue );
	}

		// If it's a HUD icon, we have to update it
	if( ent->s.eType == ET_Q3F_HUD )
	{
		if( g_mapentDebug.integer )
			G_Printf( "    HUD updated.\n" );
		G_Q3F_UpdateHUD( ent );
	}

	switch( state )
	{
		// Find the correct string to use to trigger other entities
		case Q3F_STATE_CARRIED:		propogator = mapdata->carrytarget;		break;
		case Q3F_STATE_ACTIVE:		propogator = mapdata->activetarget;		break;
		case Q3F_STATE_INACTIVE:	propogator = mapdata->inactivetarget;	break;
		case Q3F_STATE_DISABLED:	propogator = mapdata->disabletarget;	break;
		case Q3F_STATE_INVISIBLE:	propogator = mapdata->invisibletarget;	break;
		default:					propogator = NULL;
	}

	if( !propogator || !propogator->used )		// No targets, no problem :)
	{
		if( g_mapentDebug.integer )
			G_Printf( "<<< No further targets.\n" );
		return( qtrue );
	}
	if( g_mapentDebug.integer )
		G_Printf( "<<< Triggering further targets.\n" );
	G_Q3F_PropogateTrigger( propogator, activator, trace );

	return( qtrue );
}


/*
**	Messages/sounds sent to users. Broken down into states and groups.
*/

static char *q3f_killmessagekeys[3][8];	// Keep the strings for a little extra speed
//static char *q3f_killmessagekeys[3][4];	// Keep the strings for a little extra speed
void G_Q3F_KillMessage( gentity_t *victim, gentity_t *inflictor, gentity_t *attacker )
{
	// Prints/plays/says death message strings.

	G_Q3F_StateBroadcast( inflictor, attacker, victim, "_message", &q3f_killmessagekeys[0], Q3F_BROADCAST_TEXT, "kill" );
	G_Q3F_StateBroadcast( inflictor, attacker, victim, "_sound", &q3f_killmessagekeys[1], Q3F_BROADCAST_SOUND, "kill" );
	G_Q3F_StateBroadcast( inflictor, attacker, victim, "_dict", &q3f_killmessagekeys[2], Q3F_BROADCAST_DICT, "kill" );
}
static char *q3f_messagekeys[Q3F_NUM_STATES][8];	// Keep the strings for a little extra speed
static char *q3f_soundkeys[Q3F_NUM_STATES][8];	// Keep the strings for a little extra speed
static char *q3f_playstringkeys[Q3F_NUM_STATES][8];	// Keep the strings for a little extra speed
//static char *q3f_messagekeys[Q3F_NUM_STATES][4];	// Keep the strings for a little extra speed
//static char *q3f_soundkeys[Q3F_NUM_STATES][4];	// Keep the strings for a little extra speed
//static char *q3f_playstringkeys[Q3F_NUM_STATES][4];	// Keep the strings for a little extra speed
void G_Q3F_StateMessage( gentity_t *ent, gentity_t *activator )
{
	// Prints off messages to player/team/nonteam/everyone, assumes
	// the current state of ent is accurate.

	G_Q3F_StateBroadcast( ent, activator, activator, "_message", q3f_messagekeys, Q3F_BROADCAST_TEXT, NULL );
	G_Q3F_StateBroadcast( ent, activator, activator, "_sound", q3f_soundkeys, Q3F_BROADCAST_SOUND, NULL );
	G_Q3F_StateBroadcast( ent, activator, activator, "_dict", q3f_playstringkeys, Q3F_BROADCAST_DICT, NULL );
}

void G_Q3F_StateBroadcast( gentity_t *ent, gentity_t *activator, gentity_t *queryent, char *messagesuffix, char *strarray[5][8], int type, char *prefix )
{
	// Prints a general message, based on the messagesuffix (e.g. "message", "flaginfo" etc.)
	// Expects both suffix and a static array to store keys to speed up repeated broadcasts.
	// If playsound is set, it plays a sound rather than a message. The force flag in this
	// case indicates a global sound (and is only meaningful for '_all_' sounds)
	// If 'prefix' is specified, fires off a 'special' broadcast instead.

	char *message = NULL;
	char keybuff[64];			// Must be big enough for "invisible_nonteam_<messagesuffix>"
	char prefixinfobuff[64];
	q3f_keypairarray_t *otherdata;
	q3f_keypair_t *entry;
	int state, index;
	char *actstring;
	int i;
	int max;

	if( !(messagesuffix && strarray && ent->mapdata) )
		return;
	otherdata = ent->mapdata->other;
	if( !otherdata )
		return;

	state = ent->mapdata->state;
	if( state < 0 || state > Q3F_NUM_STATES )
		G_Error( "G_ETF_Message: Invalid state %d.", state );

	if( prefix && *prefix )
	{
		// Generate 'special' keys

		if( !strarray[0][0] )
		{
			strcpy( keybuff, prefix );
			Q_strcat( keybuff, sizeof(keybuff), messagesuffix );
			G_Q3F_AddString( &strarray[0][0], keybuff );	// Message to querier
			strcpy( keybuff, prefix );
			Q_strcat( keybuff, sizeof(keybuff), "_team" );
			Q_strcat( keybuff, sizeof(keybuff), messagesuffix );
			G_Q3F_AddString( &strarray[0][1], keybuff );	// Message to querier team
			strcpy( keybuff, prefix );
			Q_strcat( keybuff, sizeof(keybuff), "_nonteam" );
			Q_strcat( keybuff, sizeof(keybuff), messagesuffix );
			G_Q3F_AddString( &strarray[0][2], keybuff );	// Message to querier non-team
			strcpy( keybuff, prefix );

			for(i = 3; i < 8; i++) {
				Q_strcat( keybuff, sizeof(keybuff), "_" );
				Q_strcat( keybuff, sizeof(keybuff), team_suffixes[i-3]);
				Q_strcat( keybuff, sizeof(keybuff), messagesuffix );
				G_Q3F_AddString( &strarray[0][i], keybuff );	// Message to teams
				strcpy( keybuff, prefix );
			}
		}

		state = 0;		// Always goes into first slot, state is irrelevant.
		prefixinfobuff[0] = '_';
		Q_strncpyz( prefixinfobuff + 1, prefix, sizeof(prefixinfobuff)-1 );

		max = 8;
	}
	else if( !strarray[ent->mapdata->state][0] )
	{
		// We need to make the keys first of all.
		// They're cached to save a little time on later calls (since there's
		// a LOT of calls to these)

		strcpy( keybuff, q3f_statestrings[state] );
		Q_strcat( keybuff, sizeof(keybuff), messagesuffix );
		G_Q3F_AddString( &strarray[state][0], keybuff );	// Message to querier
		strcpy( keybuff, q3f_statestrings[state] );
		Q_strcat( keybuff, sizeof(keybuff), "_team" );
		Q_strcat( keybuff, sizeof(keybuff), messagesuffix );
		G_Q3F_AddString( &strarray[state][1], keybuff );	// Message to querier team
		strcpy( keybuff, q3f_statestrings[state] );
		Q_strcat( keybuff, sizeof(keybuff), "_nonteam" );
		Q_strcat( keybuff, sizeof(keybuff), messagesuffix );
		G_Q3F_AddString( &strarray[state][2], keybuff );	// Message to querier non-team
		strcpy( keybuff, q3f_statestrings[state] );
		Q_strcat( keybuff, sizeof(keybuff), "_all" );
		Q_strcat( keybuff, sizeof(keybuff), messagesuffix );
		G_Q3F_AddString( &strarray[state][3], keybuff );	// Message to everybody

		max = 4;
	} else {
		max = 4;
	}

	if( !activator || !activator->client || !(actstring = activator->client->pers.netname) )
		actstring = "";		// Activator name
	if( *actstring || queryent != activator ) {
			// Message to player
		if( entry = G_Q3F_KeyPairArrayFind( otherdata, strarray[state][0] ) ) {
			message = G_Q3F_MessageString( entry->value.d.strdata, activator, queryent, 7 );
			switch( type ) {
				case Q3F_BROADCAST_TEXT:		trap_SendServerCommand(	queryent - g_entities, va( "%s \"%s\n\"", ((entry->value.flags & Q3F_VFLAG_FORCE) ? "cp" : "print"), message ) ); break;
				case Q3F_BROADCAST_SOUND:		trap_SendServerCommand( queryent - g_entities, va( "play %d\n", G_SoundIndex( entry->value.d.strdata ) ) ); break;
				case Q3F_BROADCAST_DICT:		trap_SendServerCommand( queryent - g_entities, va( "playstring %s\n", message ) ); break;
			}
			// Also send these to followers
			for( index = 0 ; index < level.maxclients ; index++ ) {
				if( level.clients[index].pers.connected == CON_CONNECTED && level.clients[index].sess.sessionTeam == Q3F_TEAM_SPECTATOR &&
				  ( level.clients[index].sess.spectatorState == SPECTATOR_FOLLOW || level.clients[index].sess.spectatorState == SPECTATOR_CHASE ) &&
					level.clients[index].sess.spectatorClient == (queryent - g_entities) ) {
					switch( type ) {
						case Q3F_BROADCAST_TEXT:		trap_SendServerCommand(	index, va( "%s \"%s\n\"", ((entry->value.flags & Q3F_VFLAG_FORCE) ? "cp" : "print"), message ) ); break;
						case Q3F_BROADCAST_SOUND:		trap_SendServerCommand( index, va( "play %d\n", G_SoundIndex( entry->value.d.strdata ) ) ); break;
						case Q3F_BROADCAST_DICT:		trap_SendServerCommand( index, va( "playstring %s\n", message ) ); break;
					}
				}
			}
			if( type == Q3F_BROADCAST_TEXT && queryent->client )
				G_LogPrintf( "etfbroadcast: single%s \"%s\" %s\n", (prefix ? prefixinfobuff : messagesuffix), queryent->client->pers.netname, message );
		}
		if( entry = G_Q3F_KeyPairArrayFind( otherdata, strarray[state][1] ) ) {
			// Message to team players (all except player?)

			message = G_Q3F_MessageString( entry->value.d.strdata, activator, queryent, 7 );
			for( index = 0 ; index < level.maxclients ; index++ ) {
				if( &level.clients[index] != queryent->client && level.clients[index].sess.sessionTeam == queryent->client->sess.sessionTeam ) {
					switch( type ) {
						case Q3F_BROADCAST_TEXT:		trap_SendServerCommand(	index, va( "%s \"%s\n\"", ((entry->value.flags & Q3F_VFLAG_FORCE) ? "cp" : "print"), message ) ); break;
						case Q3F_BROADCAST_SOUND:		trap_SendServerCommand( index, va( "play %d\n", G_SoundIndex( entry->value.d.strdata ) ) ); break;
						case Q3F_BROADCAST_DICT:		trap_SendServerCommand( index, va( "playstring %s\n", message ) ); break;
					}
				}
				// Also send these to followers
				if( level.clients[index].pers.connected == CON_CONNECTED && level.clients[index].sess.sessionTeam == Q3F_TEAM_SPECTATOR &&
				  ( level.clients[index].sess.spectatorState == SPECTATOR_FOLLOW || level.clients[index].sess.spectatorState == SPECTATOR_CHASE ) &&
					level.clients[index].spectatorTeam == queryent->client->sess.sessionTeam && level.clients[index].sess.spectatorClient != (queryent - g_entities)) {
					switch( type ) {
						case Q3F_BROADCAST_TEXT:		trap_SendServerCommand(	index, va( "%s \"%s\n\"", ((entry->value.flags & Q3F_VFLAG_FORCE) ? "cp" : "print"), message ) ); break;
						case Q3F_BROADCAST_SOUND:		trap_SendServerCommand( index, va( "play %d\n", G_SoundIndex( entry->value.d.strdata ) ) ); break;
						case Q3F_BROADCAST_DICT:		trap_SendServerCommand( index, va( "playstring %s\n", message ) ); break;
					}
				}
			}
//			trap_SendServerCommand( ent-g_entities, va( entry->value.d.strdata, actstring ) );
			if( type == Q3F_BROADCAST_TEXT && queryent->client )
				G_LogPrintf( "etfbroadcast: team%s \"%s\" %s\n", (prefix ? prefixinfobuff : messagesuffix), queryent->client->pers.netname, message );
		}

		if( entry = G_Q3F_KeyPairArrayFind( otherdata, strarray[state][2] ) ) {
			// Message to non-team players
			message = G_Q3F_MessageString( entry->value.d.strdata, activator, queryent, 7 );
			for( index = 0 ; index < level.maxclients ; index++ ) {
				if( (level.clients[index].pers.connected == CON_CONNECTED) && 
					((level.clients[index].sess.sessionTeam == Q3F_TEAM_SPECTATOR && level.clients[index].sess.spectatorState == SPECTATOR_FREE ) ||
					 (level.clients[index].sess.sessionTeam == Q3F_TEAM_SPECTATOR && (level.clients[index].sess.spectatorState == SPECTATOR_FOLLOW || level.clients[index].sess.spectatorState == SPECTATOR_CHASE) && level.clients[index].spectatorTeam != queryent->client->sess.sessionTeam) ||
					 (level.clients[index].sess.sessionTeam != Q3F_TEAM_SPECTATOR && level.clients[index].sess.sessionTeam != queryent->client->sess.sessionTeam)) ) {
					switch( type ) {
						case Q3F_BROADCAST_TEXT:		trap_SendServerCommand(	index, va( "%s \"%s\n\"", ((entry->value.flags & Q3F_VFLAG_FORCE) ? "cp" : "print"), message ) ); break;
						case Q3F_BROADCAST_SOUND:		trap_SendServerCommand( index, va( "play %d\n", G_SoundIndex( entry->value.d.strdata ) ) ); break;
						case Q3F_BROADCAST_DICT:		trap_SendServerCommand( index, va( "playstring %s\n", message ) ); break;
					}
				}
			}
//			trap_SendServerCommand( ent-g_entities, va( entry->value.d.strdata, actstring ) );
			if( type == Q3F_BROADCAST_TEXT && queryent->client )
				G_LogPrintf( "etfbroadcast: nonteam%s \"%s\" %s\n", (prefix ? prefixinfobuff : messagesuffix), queryent->client->pers.netname, message );
		}
	}
	
	for(i = 3; i < max; i++) {
		if( entry = G_Q3F_KeyPairArrayFind( otherdata, strarray[state][i] ) ) {
			if ( i == 3 && type == Q3F_BROADCAST_SOUND && !(entry->value.flags & Q3F_VFLAG_FORCE)) {
				G_AddEvent( ent, EV_GENERAL_SOUND, G_SoundIndex( entry->value.d.strdata ));
				continue;
			}
			message = G_Q3F_MessageString( entry->value.d.strdata, activator, queryent, 7 );
			for( index = 0 ; index < level.maxclients ; index++ ) {
				if( (1 << level.clients[index].sess.sessionTeam) & team_for_suffix[i-3] ) {
					switch( type )
					{
						case Q3F_BROADCAST_TEXT:		trap_SendServerCommand(	index, va( "%s \"%s\n\"", ((entry->value.flags & Q3F_VFLAG_FORCE) ? "cp" : "print"), message ) ); break;
						case Q3F_BROADCAST_SOUND:		trap_SendServerCommand( index, va( "play %d\n", G_SoundIndex( entry->value.d.strdata ) ) ); break;
						case Q3F_BROADCAST_DICT:		trap_SendServerCommand( index, va( "playstring %s\n", message ) ); break;
					}
				}
				if( level.clients[index].pers.connected == CON_CONNECTED && level.clients[index].sess.sessionTeam == Q3F_TEAM_SPECTATOR &&
				  ( level.clients[index].sess.spectatorState == SPECTATOR_FOLLOW || level.clients[index].sess.spectatorState == SPECTATOR_CHASE ) &&
					(1 << level.clients[index].spectatorTeam) & team_for_suffix[i-3]) {
					switch( type )
					{
						case Q3F_BROADCAST_TEXT:		trap_SendServerCommand(	index, va( "%s \"%s\n\"", ((entry->value.flags & Q3F_VFLAG_FORCE) ? "cp" : "print"), message ) ); break;
						case Q3F_BROADCAST_SOUND:		trap_SendServerCommand( index, va( "play %d\n", G_SoundIndex( entry->value.d.strdata ) ) ); break;
						case Q3F_BROADCAST_DICT:		trap_SendServerCommand( index, va( "playstring %s\n", message ) ); break;
					}
				}
			}
			if( type == Q3F_BROADCAST_TEXT && queryent->client )
				G_LogPrintf( "etfbroadcast: %s%s \"%s\" %s\n", (prefix ? prefixinfobuff : messagesuffix), team_suffixes[i-3], queryent->client->pers.netname, message );
		}
	}
}

/*
** Utility functions used by the others
*/

void G_Q3F_PropogateTrigger( q3f_keypairarray_t *propogator, gentity_t *activator, trace_t *trace )
{
	// Trigger all the entities in the specified kparray

	int index, targindex;
	q3f_keypair_t *curr, *targetkp;
	q3f_array_t *targetarray;
	q3f_data_t *target;

	if( !propogator || !level.targetnameArray )
		return;

	for( index = -1; curr = G_Q3F_KeyPairArrayTraverse( propogator, &index ); )
	{
		if( !(targetkp = G_Q3F_KeyPairArrayFind( level.targetnameArray, curr->key )) ) {
			continue;
		}
		targindex = -1;
		targetarray = targetkp->value.d.arraydata;
		while( target = G_Q3F_ArrayTraverse( targetarray, &targindex ) )
			G_Q3F_TriggerEntity( target->d.entitydata, activator, curr->value.d.intdata, trace, (curr->value.flags & Q3F_VFLAG_FORCE) );
	}
}

qboolean G_Q3F_CheckStates( q3f_keypairarray_t *array )
{
	// Check each entry and ensure that at least one named
	// entity matching is in the desired state

	int index, targindex;
	gentity_t *ent = NULL;
	q3f_keypair_t *kp, *targkp;//*data, *targdata;
//	q3f_keypairarray_t *targetkpa;
	q3f_array_t *targetarray;
	q3f_data_t *targdata;

	for( index = -1; kp = G_Q3F_KeyPairArrayTraverse( array, &index ); )
	{
		if( !(targkp = G_Q3F_KeyPairArrayFind( level.targetnameArray, kp->key )) )
			return( qfalse );		// No targets by this name.
		targetarray = targkp->value.d.arraydata;
		for( targindex = -1; targdata = G_Q3F_ArrayTraverse( targetarray, &targindex ); )
		{
			ent = targdata->d.entitydata;
			if( ent->mapdata && ent->mapdata->state == kp->value.d.intdata )
				break;		// Continue to the next array entry
			ent = NULL;
		}
		if( !ent )
			return( qfalse );	// No matching entities found.
	}
	return( qtrue );		// Everything matched.
}

#define NUM_CLIENTSTATSSTRINGS 28
//renamed haste to speed
static char *clientstatsstrings[NUM_CLIENTSTATSSTRINGS] = {
	"health", "armor", "armour", "ammo_shells", "ammo_nails", "ammo_rockets", "ammo_cells",
	"score", "gren1", "gren2", "quad", "regen", "flight", "battlesuit", "invis", "speed",
	"ammo_medikit", "ammo_charge", "invuln", "aqualung", "gas", "stun", "flash", "tranq",
	"fire", "ceasefire", "disease", "pentagram"
};
static char *clientstatsstringptrs[NUM_CLIENTSTATSSTRINGS];
static qboolean hasclientstatsstrings;

typedef enum {
	EVAL_EQ,
	EVAL_LT,
	EVAL_GT,
	EVAL_LTEQ,
	EVAL_GTEQ
} evaltype_t;

static qboolean G_Q3F_EvalStat( const int value, const evaltype_t eval, const int checkvalue ) {
	if( eval == EVAL_EQ ) {
		return( value == checkvalue );
	} else if( eval == EVAL_LT ) {
		return( value < checkvalue );
	} else if( eval == EVAL_GT ) {
		return( value > checkvalue );
	} else if( eval == EVAL_LTEQ ) {
		return( value <= checkvalue );
	} else if( eval == EVAL_GTEQ ) {
		return( value >= checkvalue );
	}

	// should never get here
	return( qfalse );
}

#define EVALCLIENTSTAT(v,e,c,o) if( G_Q3F_EvalStat( v, e, c ) ) { if( o ) return( qtrue ); else continue; } else { if( o ) continue; else return( qfalse ); }

qboolean G_Q3F_CheckClientStats( gentity_t *activator, q3f_keypairarray_t *array, qboolean or )
{
	// Check each entry and make sure all of them evaluate to true

	int index;
	q3f_keypair_t *data;
	evaltype_t eval = -1;
	bg_q3f_playerclass_t *cls;

	if( !activator || !activator->client )
		return( qtrue );	// Not a client, so can pass without problems

	cls = bg_q3f_classlist[activator->client->ps.persistant[PERS_CURRCLASS]];

	if( !hasclientstatsstrings )
	{
		// Make sure we have our pointers to search on. NULL pointers
		// don't matter, we know they don't exist in any ents anyway
		// or they'd have been in the string table.
		for( index = 0; index < NUM_CLIENTSTATSSTRINGS; index++ )
		{
			if( !clientstatsstringptrs[index] )
				clientstatsstringptrs[index] = G_Q3F_GetString( clientstatsstrings[index] );
		}
		hasclientstatsstrings = qtrue;
	}

	for( index = -1; data = G_Q3F_KeyPairArrayTraverse( array, &index ); )
	{
		if( data->value.flags & Q3F_VFLAG_CLSTAT_EQ && data->value.flags & Q3F_VFLAG_CLSTAT_LT )
			eval = EVAL_LTEQ;
		else if( data->value.flags & Q3F_VFLAG_CLSTAT_EQ && data->value.flags & Q3F_VFLAG_CLSTAT_GT )
			eval = EVAL_GTEQ;
		else if( data->value.flags & Q3F_VFLAG_CLSTAT_EQ )
			eval = EVAL_EQ;
		else if( data->value.flags & Q3F_VFLAG_CLSTAT_LT )
			eval = EVAL_LT;
		else if( data->value.flags & Q3F_VFLAG_CLSTAT_GT )
			eval = EVAL_GT;

		if( data->key == clientstatsstringptrs[0] ) {			// health
			EVALCLIENTSTAT( activator->client->ps.stats[STAT_HEALTH], eval, (data->value.flags & Q3F_VFLAG_CLSTAT_CM ? cls->maxhealth : data->value.d.intdata), or );
		} else if( data->key == clientstatsstringptrs[1] ||
				   data->key == clientstatsstringptrs[2] ) {	// armour
			EVALCLIENTSTAT( activator->client->ps.stats[STAT_ARMOR], eval, (data->value.flags & Q3F_VFLAG_CLSTAT_CM ? cls->maxarmour : data->value.d.intdata), or );
		} else if( data->key == clientstatsstringptrs[3] ) {	// shells
			EVALCLIENTSTAT( activator->client->ps.ammo[AMMO_SHELLS], eval, (data->value.flags & Q3F_VFLAG_CLSTAT_CM ? cls->maxammo_shells : data->value.d.intdata), or );
		} else if( data->key == clientstatsstringptrs[4] ) {	// nails
			EVALCLIENTSTAT( activator->client->ps.ammo[AMMO_NAILS], eval, (data->value.flags & Q3F_VFLAG_CLSTAT_CM ? cls->maxammo_nails : data->value.d.intdata), or );
		} else if( data->key == clientstatsstringptrs[5] ) {	// rockets
			EVALCLIENTSTAT( activator->client->ps.ammo[AMMO_ROCKETS], eval, (data->value.flags & Q3F_VFLAG_CLSTAT_CM ? cls->maxammo_rockets : data->value.d.intdata), or );
		} else if( data->key == clientstatsstringptrs[6] ) {	// cells
			EVALCLIENTSTAT( activator->client->ps.ammo[AMMO_CELLS], eval, (data->value.flags & Q3F_VFLAG_CLSTAT_CM ? cls->maxammo_cells : data->value.d.intdata), or );
		} else if( data->key == clientstatsstringptrs[7] ) {	// score
			EVALCLIENTSTAT( activator->client->ps.persistant[PERS_SCORE], eval, data->value.d.intdata, or );
		} else if( data->key == clientstatsstringptrs[8] ) {	// gren1
			EVALCLIENTSTAT( activator->client->ps.ammo[AMMO_GRENADES] & 0xFF, eval, (data->value.flags & Q3F_VFLAG_CLSTAT_CM ? cls->gren1max : data->value.d.intdata), or );
		} else if( data->key == clientstatsstringptrs[9] ) {	// gren2
			EVALCLIENTSTAT( activator->client->ps.ammo[AMMO_GRENADES] & 0xFF00, eval, (data->value.flags & Q3F_VFLAG_CLSTAT_CM ? cls->gren2max : data->value.d.intdata), or );
		} else if( data->key == clientstatsstringptrs[10] ) {	// quad
			EVALCLIENTSTAT( activator->client->ps.powerups[PW_QUAD], eval, data->value.d.intdata, or );
		} else if( data->key == clientstatsstringptrs[11] ) {	// regen
			EVALCLIENTSTAT( activator->client->ps.powerups[PW_REGEN], eval, data->value.d.intdata, or );
		} else if( data->key == clientstatsstringptrs[12] ) {	// flight
			EVALCLIENTSTAT( activator->client->ps.powerups[PW_FLIGHT], eval, data->value.d.intdata, or );
		} else if( data->key == clientstatsstringptrs[13] ) {	// battlesuit
			EVALCLIENTSTAT( activator->client->ps.powerups[PW_BATTLESUIT], eval, data->value.d.intdata, or );
		} else if( data->key == clientstatsstringptrs[14] ) {	// invis
			EVALCLIENTSTAT( activator->client->ps.powerups[PW_INVIS], eval, data->value.d.intdata, or );
		} else if( data->key == clientstatsstringptrs[15] ) {	// haste
			EVALCLIENTSTAT( activator->client->ps.powerups[PW_HASTE], eval, data->value.d.intdata, or );
		} else if( data->key == clientstatsstringptrs[16] ) {	// medikit
			EVALCLIENTSTAT( activator->client->ps.ammo[AMMO_MEDIKIT], eval, (data->value.flags & Q3F_VFLAG_CLSTAT_CM ? cls->maxammo_medikit : data->value.d.intdata), or );
		} else if( data->key == clientstatsstringptrs[17] ) {	// charge
			EVALCLIENTSTAT( activator->client->ps.ammo[AMMO_CHARGE], eval, (data->value.flags & Q3F_VFLAG_CLSTAT_CM ? cls->maxammo_charge : data->value.d.intdata), or );
		} else if( data->key == clientstatsstringptrs[18] ) {	// invuln
			EVALCLIENTSTAT( activator->client->ps.powerups[PW_Q3F_INVULN], eval, data->value.d.intdata, or );
		} else if( data->key == clientstatsstringptrs[19] ) {	// aqualung
			EVALCLIENTSTAT( activator->client->ps.powerups[PW_Q3F_AQUALUNG], eval, data->value.d.intdata, or );
		} else if( data->key == clientstatsstringptrs[20] ) {	// gas
			EVALCLIENTSTAT( activator->client->ps.powerups[PW_Q3F_GAS], eval, data->value.d.intdata, or );
		} else if( data->key == clientstatsstringptrs[21] ) {	// stun
			EVALCLIENTSTAT( activator->client->ps.powerups[PW_Q3F_CONCUSS], eval, data->value.d.intdata, or );
		} else if( data->key == clientstatsstringptrs[22] ) {	// flash
			EVALCLIENTSTAT( activator->client->ps.powerups[PW_Q3F_FLASH], eval, data->value.d.intdata, or );
		} else if( data->key == clientstatsstringptrs[23] ) {	// tranq
			EVALCLIENTSTAT( activator->client->tranqTime, eval, data->value.d.intdata, or );
		} else if( data->key == clientstatsstringptrs[24] ) {	// fire
			EVALCLIENTSTAT( activator->client->flames, eval, (data->value.flags & Q3F_VFLAG_CLSTAT_CM ? Q3F_MAX_FLAMES_PER_PERSON : data->value.d.intdata), or );
		} else if( data->key == clientstatsstringptrs[25] ) {	// ceasefire
			EVALCLIENTSTAT( activator->client->ps.powerups[PW_Q3F_CEASEFIRE], eval, data->value.d.intdata, or );
		} else if( data->key == clientstatsstringptrs[26] ) {	// disease
			EVALCLIENTSTAT( activator->client->diseaseTime, eval, (data->value.flags & Q3F_VFLAG_CLSTAT_CM ? Q3F_DISEASE_DAMAGE_EVERY : data->value.d.intdata), or );
		} else if ( data->key == clientstatsstringptrs[27] ) {	// pentagram of protection
			EVALCLIENTSTAT( activator->client->ps.powerups[PW_PENTAGRAM], eval, data->value.d.intdata, or );
		}
	}

	if( or )
		// bleh, we made it all the way through
		return( qfalse );
	else
		// woop, we made it all the way through
		return( qtrue );
}


/*
** The 'give' command, gives the specified players specified items.
*/

static int _CalcGiveValue( int srcvalue, qboolean add, int maxvalue, int minvalue, float factor, int newvalue )
{
	// Calculate the new value from the supplied parameters

	int value;

	value =	(add ? srcvalue : 0) +
			(factor	? (newvalue * factor)
					: newvalue);
	if( value > maxvalue )
		value = maxvalue;
	if( value < minvalue )
		value = minvalue;
	if( value < srcvalue && add && newvalue >= 0 )
		value = srcvalue;		// If adding health and we're already above max, stop 'losing' health as a result.
	return( value );
}

static int _CalcPowerupValue( int srcvalue, qboolean add, qboolean force, float factor, int newvalue )
{
	int value;

	value = (srcvalue >= level.time) ? (srcvalue - level.time) : 0;
	value = _CalcGiveValue(	value, add,
						(force ? 999000 : 60000), -999000,
						factor, newvalue * 1000 );
	return( level.time + value );
}

#define NUMGIVESTRINGS 37
//keeg renamed haste to speed
static char *givestrings[NUMGIVESTRINGS] = {
	"health", "armor", "ammo_shells", "ammo_nails", "ammo_rockets", "ammo_cells",
	"score", "gren1", "gren2", "quad", "regen", "flight", "battlesuit", "invis", "speed",
	"armortype", "aclass_shell", "aclass_nail", "aclass_explosive", "aclass_shock",
	"aclass_fire", "aclass_all", "ammo_medikit", "ammo_charge", "invuln", "aqualung", "ceasefire",
	"aclass_bullet", "ammo_bullets", "damage", "gas", "stun", "flash", "tranq", "fire", "disease",
	"pentagram"
};

static char *givestringptrs[NUMGIVESTRINGS];
static char *effectradiusptr, *affectteamsptr, *teamscoreptr/*, *holdingptr, *notholdingptr*/;
static qboolean hasgivestrings;
void G_Q3F_MapGive( gentity_t *ent, gentity_t *other )
{
	// Attempt to give all affected entities the specified bonuses.
	// Currently, only affects the activating player.

	int index, force, add, teams, pointcontents, newvalue;
	float distance, effectfactor;
	q3f_keypair_t *data;
	q3f_keypairarray_t *give, *clientstats;
	q3f_array_t *holding, *notholding;
	bg_q3f_playerclass_t *cls;
	gentity_t *current, *minent, *maxent;
	vec3_t vec3;
	trace_t trace;
	qboolean magicarmourtype, setarmourtype;

	if( !other->mapdata )
		return;
	if( ent && !ent->client )
		ent = NULL;		// Stop checking against ent for criteria

	if( !hasgivestrings )
	{
		// Make sure we have our pointers to search on. NULL pointers
		// don't matter, we know they don't exist in any ents anyway
		// or they'd have been in the string table.
		for( index = 0; index < NUMGIVESTRINGS; index++ )
		{
			if( !givestringptrs[index] )
				givestringptrs[index] = G_Q3F_GetString( givestrings[index] );
		}
		G_Q3F_AddString( &affectteamsptr,	"affectteams" );
		G_Q3F_AddString( &effectradiusptr,	"effectradius" );
		G_Q3F_AddString( &teamscoreptr,		"teamscore" );
//		G_Q3F_AddString( &holdingptr,		"holding" );
//		G_Q3F_AddString( &notholdingptr,	"notholding" );
		hasgivestrings = qtrue;
	}

		// Work out what players are affected by this give
	teams = 0;
	if( ent )
	{
		if( other->mapdata->flags & Q3F_FLAG_AFFECTTEAM )
			teams |= 1 << ent->client->sess.sessionTeam;
		if( other->mapdata->flags & Q3F_FLAG_AFFECTNONTEAM )
			teams |= ~((1 << ent->client->sess.sessionTeam) | Q3F_TEAM_SPECTATOR);
	}
	data = G_Q3F_KeyPairArrayFind( other->mapdata->other, affectteamsptr );
	teams |= data ? G_Q3F_ProcessTeamString( data->value.d.strdata ) : 0;
	data = G_Q3F_KeyPairArrayFind( other->mapdata->other, effectradiusptr );
	distance = data ? atof( data->value.d.strdata ) : 0;

	/*data = G_Q3F_KeyPairArrayFind( level.targetnameArray, holdingptr );
	holding = data ? data->value.d.arraydata : NULL;
	data = G_Q3F_KeyPairArrayFind( level.targetnameArray, notholdingptr );
	notholding = data ? data->value.d.arraydata : NULL;*/
	holding = other->mapdata->holding;
	notholding = other->mapdata->notholding;
	clientstats = other->mapdata->clientstats;

	// Check for team scoring
	if(g_matchState.integer <= MATCH_STATE_PLAYING && !level.intermissionQueued && !level.intermissiontime)
	{
		data = G_Q3F_KeyPairArrayFind( other->mapdata->other, teamscoreptr );
		if( data && data->value.d.intdata ) 
		{
			if( data->value.type == Q3F_TYPE_STRING )
			{
				index = atoi( data->value.d.strdata );
				G_Q3F_RemString( &data->value.d.strdata );
				data->value.d.intdata = index;
				data->value.type = Q3F_TYPE_INTEGER;
			}
			if( teams )
			{
				for( index = 0; index < Q3F_TEAM_NUM_TEAMS; index++ )
				{
					if( teams & (1 << index) )
					{
						level.teamScores[index] += data->value.d.intdata;
					}
				}
			}
			else if( ent ) {
				level.teamScores[ent->client->sess.sessionTeam] += data->value.d.intdata;
			}
			CalculateRanks();	// Update the score
		}
	}

	if( !(give = other->mapdata->give) )
		return;

	// Work out min and max of our loop
	if( teams || distance )
	{
		minent = level.gentities;
		maxent = &level.gentities[MAX_CLIENTS-1];
	}
	else minent = maxent = ent;

	if( other->mapdata->flags & Q3F_FLAG_ENVIRONMENT )
		pointcontents = trap_PointContents( other->s.pos.trBase, other - level.gentities ) & MASK_WATER;

	for( current = minent; current && current <= maxent; current++ )
	{
			// For each player, check the ent can affect them
		if( !current->client || !current->inuse || current->health <= 0 )
			continue;		// Not a player, or dead
		if ( Q3F_IsSpectator( current->client ) )
			continue;		// Ensiform: Not a spectator either (as spectators are pointless to give to)
		if( teams && !(teams & (1 << current->client->sess.sessionTeam) ))
			continue;		// Bad team
		if( distance )
		{
			VectorSubtract( current->s.pos.trBase, other->s.pos.trBase, vec3 );
			effectfactor = 1.0 - VectorLength( vec3 ) / distance;
			if( effectfactor <= 0 )
				continue;	// Out of range
		}
		else effectfactor = 1;
		if( other->mapdata->flags & Q3F_FLAG_ENVIRONMENT )
		{
			if( (trap_PointContents( current->s.pos.trBase, current - level.gentities ) & MASK_WATER) !=
				pointcontents )
				continue;	// Different environment
		}
		if( other->mapdata->flags & Q3F_FLAG_LINEOFSIGHT )
		{
			trap_Trace( &trace, other->s.pos.trBase, NULL, NULL, current->s.pos.trBase, other-level.gentities, MASK_SOLID );
			if( trace.entityNum != current-level.gentities )
				continue;	// Trace failed, not in line-of-sight
		}
		if( holding && !G_Q3F_CheckHeld( current, holding ) )
			continue;		// Not holding the required ents
		if( notholding && !G_Q3F_CheckNotHeld( current, notholding ) )
			continue;		// Holding the forbidden ents
		if( clientstats && !G_Q3F_CheckClientStats( ent, clientstats, (other->mapdata->flags & Q3F_FLAG_ORCLIENTSTATS) ) )
			continue;		// Not feeling right

		if( !(other->mapdata->flags & Q3F_FLAG_EFFECTDROPOFF) )
			effectfactor = 0;		// No dropoff

			// This client is OK, now we traverse the array and give them everything
			// requested.
		cls = BG_Q3F_GetClass( &current->client->ps );
		magicarmourtype = setarmourtype = qfalse;
		for( index = -1; data = G_Q3F_KeyPairArrayTraverse( give, &index ); )
		{
			force = data->value.flags	& Q3F_VFLAG_FORCE;
			add = data->value.flags		& Q3F_VFLAG_GIVE_ADD;
			if( data->key == givestringptrs[0] )		// Alter player health
			{
				newvalue = _CalcGiveValue(	current->health, add,
											(force ? 999 : cls->maxhealth), -10000,
											effectfactor, data->value.d.intdata );
				if( newvalue >= current->health )
					current->client->ps.stats[STAT_HEALTH] = current->health = newvalue;
				else G_Damage( current, other, NULL, NULL, NULL, current->health - newvalue, DAMAGE_NO_ARMOR, MOD_UNKNOWN );
			}
			else if( data->key == givestringptrs[1] )	// Alter player armour
			{
				newvalue = _CalcGiveValue(	current->client->ps.stats[STAT_ARMOR], add,
											(force ? 999 : cls->maxarmour), 0,
											effectfactor, data->value.d.intdata );
				if( newvalue > current->client->ps.stats[STAT_ARMOR] )
					magicarmourtype = qtrue;
				current->client->ps.stats[STAT_ARMOR] = newvalue;
			}
			else if( data->key == givestringptrs[29] )	// Deal damage
			{
				G_Damage( current, other, NULL, NULL, NULL, data->value.d.intdata, (force ? DAMAGE_NO_ARMOR : 0), MOD_UNKNOWN );
			}

			else if( data->key == givestringptrs[2] )	// Alter player ammo_shells
				current->client->ps.ammo[AMMO_SHELLS] =
					_CalcGiveValue(	current->client->ps.ammo[AMMO_SHELLS], add,
									(force ? 999 : cls->maxammo_shells), 0,
									effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[3] || data->key == givestringptrs[28] )	// Alter player ammo_nails
				current->client->ps.ammo[AMMO_NAILS] =
					_CalcGiveValue(	current->client->ps.ammo[AMMO_NAILS], add,
									(force ? 999 : cls->maxammo_nails), 0,
									effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[4] )	// Alter player ammo_rockets
				current->client->ps.ammo[AMMO_ROCKETS] =
					_CalcGiveValue(	current->client->ps.ammo[AMMO_ROCKETS], add,
									(force ? 999 : cls->maxammo_rockets), 0,
									effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[5] )	// Alter player ammo_cells
				current->client->ps.ammo[AMMO_CELLS] =
					_CalcGiveValue(	current->client->ps.ammo[AMMO_CELLS], add,
									(force ? 999 : cls->maxammo_cells), 0,
									effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[6] )	// Alter player score (always add)
				AddScore( current, current->r.currentOrigin, _CalcGiveValue(	((force && !add) ? -current->client->ps.persistant[PERS_SCORE] : 0),
																				1, 99999, -99999,
																				effectfactor, data->value.d.intdata ) );
			else if( data->key == givestringptrs[7] )	// Alter grenade type 1
				current->client->ps.ammo[AMMO_GRENADES] = (current->client->ps.ammo[AMMO_GRENADES] & 0xFF00)
					+ _CalcGiveValue(	(current->client->ps.ammo[AMMO_GRENADES] & 0xFF), add,
										(force ? 100 : cls->gren1max), 0,
										effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[8] )	// Alter grenade type 2
				current->client->ps.ammo[AMMO_GRENADES] = (current->client->ps.ammo[AMMO_GRENADES] & 0xFF)
					+ (_CalcGiveValue(	(current->client->ps.ammo[AMMO_GRENADES] >> 8), add,
										(force ? 100 : cls->gren2max), 0,
										effectfactor, data->value.d.intdata ) << 8);
			else if( data->key == givestringptrs[22] )	// Alter player medikit
				current->client->ps.ammo[AMMO_MEDIKIT] =
					_CalcGiveValue(	current->client->ps.ammo[AMMO_MEDIKIT], add,
									(force ? 999 : cls->maxammo_medikit), 0,
									effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[23] )	// Alter player charges
				current->client->ps.ammo[AMMO_CHARGE] =
					_CalcGiveValue(	current->client->ps.ammo[AMMO_CHARGE], add,
									(force ? 999 : cls->maxammo_charge), 0,
									effectfactor, data->value.d.intdata );

			else if( data->key == givestringptrs[9] )	// Alter player quad time
				current->client->ps.powerups[PW_QUAD] =
					_CalcPowerupValue( current->client->ps.powerups[PW_QUAD], add, force, effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[10] )	// Alter player regen time
				current->client->ps.powerups[PW_REGEN] =
					_CalcPowerupValue( current->client->ps.powerups[PW_REGEN], add, force, effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[11] )	// Alter player flight time
				current->client->ps.powerups[PW_FLIGHT] =
					_CalcPowerupValue( current->client->ps.powerups[PW_FLIGHT], add, force, effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[12] )	// Alter player battlesuit time
				current->client->ps.powerups[PW_BATTLESUIT] =
					_CalcPowerupValue( current->client->ps.powerups[PW_BATTLESUIT], add, force, effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[13] )	// Alter player invis time
				current->client->ps.powerups[PW_INVIS] =
					_CalcPowerupValue( current->client->ps.powerups[PW_INVIS], add, force, effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[14] )	// Alter player haste time
				current->client->ps.powerups[PW_HASTE] =
					_CalcPowerupValue( current->client->ps.powerups[PW_HASTE], add, force, effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[24] )	// Alter player invuln time
				current->client->ps.powerups[PW_Q3F_INVULN] =
					_CalcPowerupValue( current->client->ps.powerups[PW_Q3F_INVULN], add, force, effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[25] )	// Alter player aqualung time
				current->client->ps.powerups[PW_Q3F_AQUALUNG] =
					_CalcPowerupValue( current->client->ps.powerups[PW_Q3F_AQUALUNG], add, force, effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[26] )	// Alter player ceasefire time
				current->client->ps.powerups[PW_Q3F_CEASEFIRE] =
					_CalcPowerupValue( current->client->ps.powerups[PW_Q3F_CEASEFIRE], add, force, effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[30] )	// Alter player gas time
				current->client->ps.powerups[PW_Q3F_GAS] =
					_CalcPowerupValue( current->client->ps.powerups[PW_Q3F_GAS], add, force, effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[31] )	// Alter player stun time
				current->client->ps.powerups[PW_Q3F_CONCUSS] =
					_CalcPowerupValue( current->client->ps.powerups[PW_Q3F_CONCUSS], add, force, effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[32] )	// Alter player flash time
				current->client->ps.powerups[PW_Q3F_FLASH] =
					_CalcPowerupValue( current->client->ps.powerups[PW_Q3F_FLASH], add, force, effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[33] )	// Alter player tranq time
				current->client->tranqTime =
					_CalcPowerupValue( current->client->tranqTime, add, force, effectfactor, data->value.d.intdata );
			else if( data->key == givestringptrs[34] )	// Alter player fire time
			{
				int i;
				newvalue = _CalcGiveValue( current->client->flames, add, Q3F_MAX_FLAMES_PER_PERSON, 0, effectfactor, data->value.d.intdata );

				if( newvalue == 0 ) {
					// Remove Flames
					if( current->client->flames )
					{
						gentity_t *flame = NULL;

						// They're alight
						while ((flame = G_Find (flame, FOFS(classname), "flame")) != NULL)
						{
							if(flame->target_ent == current)
							{
								flame->nextthink = level.time;
								flame->think = G_FreeEntity;
								//flame->nextthink = 0;
								//flame->think = NULL;
								//G_FreeEntity(flame);
							}
						}
						current->client->flames = 0;
					}
				} else if( current->client->flames < newvalue ) {
					for( i = current->client->flames; i < newvalue; i++ )
						G_Q3F_Burn_Person( current, other );
				} else if( current->client->flames > newvalue ) {
					gentity_t *flame = NULL;

					// They're alight
					while ((flame = G_Find (flame, FOFS(classname), "flame")) != NULL)
					{
						if(flame->target_ent == current)
						{
							flame->nextthink = level.time;
							flame->think = G_FreeEntity;
							//G_FreeEntity(flame);
							current->client->flames--;

							if( current->client->flames == newvalue )
								break;
						}
					}
				}
			}

			else if( data->key == givestringptrs[35] ) // Alter player disease state
			{
				newvalue = _CalcGiveValue( (current->client->diseaseTime > level.time) ? 1 : 0, add, 1, 0, effectfactor, data->value.d.intdata );

				if( newvalue == 0 ) {
					// Remove Disease
					current->client->diseaseTime = 0;		// Remove the disease.
					current->client->diseaseEnt = 0;
				} else {
					// Give Non-Client Attacker Disease
					G_Q3F_Disease2_Person(current, other, qfalse);
				}
			}

			else if( data->key == givestringptrs[36] )	// Alter player pentagram of protection time
			{
				current->client->ps.powerups[PW_PENTAGRAM] =
					_CalcPowerupValue( current->client->ps.powerups[PW_PENTAGRAM], add, force, effectfactor, data->value.d.intdata );
			}

			else if( data->key == givestringptrs[15] )	// Alter player armourtype
			{
				current->client->ps.stats[STAT_ARMORTYPE] =
					_CalcGiveValue(	current->client->ps.stats[STAT_ARMORTYPE], add,
									cls->maxarmourtype, 0,
									effectfactor, data->value.d.intdata );
				setarmourtype = qtrue;
			}

			else if( data->key == givestringptrs[16] )	// Armour class 'shell'
				current->client->ps.stats[STAT_Q3F_ARMOURCLASS] =
					data->value.d.intdata	? (current->client->ps.stats[STAT_Q3F_ARMOURCLASS] | (cls->maxarmourclass & DAMAGE_Q3F_SHELL))
											: (current->client->ps.stats[STAT_Q3F_ARMOURCLASS] & ~DAMAGE_Q3F_SHELL);
			else if( data->key == givestringptrs[17] || data->key == givestringptrs[27] )	// Armour class 'nail'
				current->client->ps.stats[STAT_Q3F_ARMOURCLASS] =
					data->value.d.intdata	? (current->client->ps.stats[STAT_Q3F_ARMOURCLASS] | (cls->maxarmourclass & DAMAGE_Q3F_NAIL))
											: (current->client->ps.stats[STAT_Q3F_ARMOURCLASS] & ~DAMAGE_Q3F_NAIL);
			else if( data->key == givestringptrs[18] )	// Armour class 'explosion'
				current->client->ps.stats[STAT_Q3F_ARMOURCLASS] =
					data->value.d.intdata	? (current->client->ps.stats[STAT_Q3F_ARMOURCLASS] | (cls->maxarmourclass & DAMAGE_Q3F_EXPLOSION))
											: (current->client->ps.stats[STAT_Q3F_ARMOURCLASS] & ~DAMAGE_Q3F_EXPLOSION);
			else if( data->key == givestringptrs[19] )	// Armour class 'shock'
				current->client->ps.stats[STAT_Q3F_ARMOURCLASS] =
					data->value.d.intdata	? (current->client->ps.stats[STAT_Q3F_ARMOURCLASS] | (cls->maxarmourclass & DAMAGE_Q3F_SHOCK))
											: (current->client->ps.stats[STAT_Q3F_ARMOURCLASS] & ~DAMAGE_Q3F_SHOCK);
			else if( data->key == givestringptrs[20] )	// Armour class 'fire'
				current->client->ps.stats[STAT_Q3F_ARMOURCLASS] =
					data->value.d.intdata	? (current->client->ps.stats[STAT_Q3F_ARMOURCLASS] | (cls->maxarmourclass & DAMAGE_Q3F_FIRE))
											: (current->client->ps.stats[STAT_Q3F_ARMOURCLASS] & ~DAMAGE_Q3F_FIRE);
			else if( data->key == givestringptrs[21] )	// Armour class 'all' (i.e. the five above)
				current->client->ps.stats[STAT_Q3F_ARMOURCLASS] =
					data->value.d.intdata	? (current->client->ps.stats[STAT_Q3F_ARMOURCLASS] | (cls->maxarmourclass & DAMAGE_Q3F_MASK))
											: (current->client->ps.stats[STAT_Q3F_ARMOURCLASS] & ~DAMAGE_Q3F_MASK);
		}
		if( magicarmourtype && !setarmourtype )		// Magically give them armourtype if not specified
			current->client->ps.stats[STAT_ARMORTYPE] = cls->maxarmourtype;

		BG_PlayerStateToEntityState( &current->client->ps, &current->s, qtrue );
	}
}


/*
**	The movement function for the goal. Basically, it can be stationary
**	or moving. Doesn't get much simpler :)
*/

static void G_Q3F_GoalItemExpand( gentity_t *ent, float scale )
{
	// Attempt to expand the goalitem to it's full size if possible.

	vec3_t mins, maxs, origin;
	trace_t tr;

	VectorScale( ent->r.mins, scale, mins );
	VectorScale( ent->r.maxs, scale, maxs );
	if( mins[0] < ent->movedir[0] )	mins[0] = ent->movedir[0];
	if( mins[1] < ent->movedir[1] )	mins[1] = ent->movedir[1];
	if( mins[2] < ent->movedir[2] )	mins[2] = ent->movedir[2];
	if( maxs[0] > ent->pos2[0] )	maxs[0] = ent->pos2[0];
	if( maxs[1] > ent->pos2[1] )	maxs[1] = ent->pos2[1];
	if( maxs[2] > ent->pos2[2] )	maxs[2] = ent->pos2[2];

	VectorCopy( ent->r.currentOrigin, origin );
	trap_Trace( &tr, origin, mins, maxs, origin, ent->s.number, MASK_DEADSOLID );
	if( tr.startsolid && mins[0] != ent->r.mins[0] )
	{
		// Overlapped, lets try moving it up a little in case it's sitting on the ground already.

		origin[2] += ent->r.mins[2] - mins[2];
		trap_Trace( &tr, origin, mins, maxs, origin, ent->s.number, MASK_DEADSOLID );
		if( tr.startsolid )
			return;			// We can't expand this frame.
	}

		// Stop expansion after this if we're at full size.
	if(	mins[0] == ent->movedir[0] &&
		mins[1] == ent->movedir[1] &&
		mins[2] == ent->movedir[2] &&
		maxs[0] == ent->pos2[0] &&
		maxs[1] == ent->pos2[1] &&
		maxs[2] == ent->pos2[2] )
		ent->soundLoop = 0;

	VectorCopy( mins, ent->r.mins );
	VectorCopy( maxs, ent->r.maxs );
	if( origin[2] != ent->r.currentOrigin[2] )
	{
		// We need to reset the position, but set it to fall again so that it seats itself properly afterwards.
		if( ent->s.pos.trType == TR_STATIONARY )
			G_SetOrigin( ent, origin );
		else {
			ent->s.pos.trBase[2] += ent->r.currentOrigin[2] - origin[2];
			ent->r.currentOrigin[2] = origin[2];
		}
	}
//	trap_LinkEntity( ent );
}

void G_Q3F_RunGoal( gentity_t *ent )
{
	vec3_t origin;
	trace_t tr;
//	gentity_t *other;
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	if( ent->s.otherEntityNum < MAX_CLIENTS )
	{
		// We're being carried, check we're synced. Nasty bandwidth issues?

		if( !VectorCompare( level.gentities[ent->s.otherEntityNum].r.currentOrigin, origin ) ) {
			VectorCopy( level.gentities[ent->s.otherEntityNum].r.currentOrigin, origin );
			SnapVector( origin );
			G_SetOrigin( ent, origin );

			trap_LinkEntity( ent );
		}
	}
	else if( ent->s.pos.trType != TR_STATIONARY )
	{
		// get current position
		BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

		// trace a line from the previous position to the current position,
		// ignoring interactions with the missile owner
		trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, 
			ent->r.ownerNum, MASK_DEADSOLID );

		VectorCopy( tr.endpos, ent->r.currentOrigin );
		trap_LinkEntity( ent );

		if ( tr.startsolid ) {
			//G_Printf("^3G_Q3F_RunGoal: Starting in solid!^7\n");
			tr.fraction = 0;
		}

		if ( tr.fraction != 1 )
		{
			//other = &g_entities[tr.entityNum];
				// reflect the velocity on the trace plane
			hitTime = level.previousTime + ( level.time - level.previousTime ) * tr.fraction;
			BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
			VectorScale( velocity, ent->soundLoop ? 1.0f : ent->physicsBounce, velocity );
			dot = DotProduct( velocity, tr.plane.normal );
			VectorMA( velocity, -2*dot, tr.plane.normal, ent->s.pos.trDelta );
			VectorScale( ent->s.pos.trDelta, ent->soundLoop ? 1.0f : ent->physicsBounce, ent->s.pos.trDelta );
				// check for stop
			if( tr.startsolid ||
				(!ent->soundLoop && tr.plane.normal[2] > 0.2 && VectorLength( ent->s.pos.trDelta ) < 40) )
			{
				G_SetOrigin( ent, tr.endpos );
				ent->s.pos.trType = TR_STATIONARY;
				ent->s.groundEntityNum = tr.entityNum;

				if(	ent->mapdata->state == Q3F_STATE_ACTIVE &&
					(ent->mapdata->flags & Q3F_FLAG_CARRYABLE) &&
					(trap_PointContents( tr.endpos, ent->s.number ) & CONTENTS_NODROP) )
				{
					// Nodrop area, return to base

					G_Q3F_TriggerEntity( ent, ent->mapdata->lastTriggerer, Q3F_STATE_INACTIVE, &tr, 1 );
				}
			}
			else {
				VectorAdd( ent->r.currentOrigin, tr.plane.normal, ent->r.currentOrigin);
				VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
				ent->s.pos.trTime = level.time;
				ent->s.groundEntityNum = ENTITYNUM_NONE;
			}
		}

		trap_LinkEntity( ent );
	}
	else {
		// Check to see if we're still stationary (i.e. trapped inside another entity)
		if( ent->mapdata->state == Q3F_STATE_ACTIVE &&
			(ent->mapdata->flags & Q3F_FLAG_CARRYABLE) )
		{
			VectorCopy( ent->r.currentOrigin, origin );
			origin[2] -= 0.25;
			trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, 
						ent->s.number, MASK_DEADSOLID );
			if( !tr.startsolid && tr.fraction == 1 )
			{
				ent->s.pos.trType = TR_GRAVITY;
				ent->s.pos.trTime = level.time;
				ent->s.groundEntityNum = ENTITYNUM_NONE;

				trap_LinkEntity( ent );
			}
		}
	}

	if( ent->soundLoop )
		G_Q3F_GoalItemExpand( ent, 1.1f );

	// check think function after bouncing
	G_RunThink( ent );
}

/*
**	The message parsing function. When given a string, activator, and queryer,
**	it will fill in all the expected fields approprately, e.g. %H %L.
*/

static void ms_itoa( int value, char *buffptr )
{
	// Convert integer to ascii (assumes buff is big enough for an int)

	char *startptr;
	int index;
	char temp;

	if( value < 0 )
	{
		*buffptr++ = '-';
		value = -value;
	}
	startptr = buffptr;
	if( !value )
		*buffptr++ = '0';		// Force a zero

	while( value )
	{
		*buffptr++ = '0' + (value % 10);
		value /= 10;
	}
	*buffptr = 0;
	for( index = 0; index < ((buffptr - startptr)>>1); index++ )
	{
		// Swap all the digits round
		temp = *(startptr + index);
		*(startptr  + index) = *(buffptr - index - 1);
		*(buffptr - index - 1) = temp;
	}
}

static char messagestringbuff[32000];		// Same size as va() uses
void _MS_TraceLocation( gentity_t *ent, vec3_t dest )
{
	// Find where the specified player is pointing.

	trace_t tr;
	vec3_t org, forward;

	AngleVectors( ent->client->ps.viewangles, forward, NULL, NULL );
	VectorCopy( ent->client->ps.origin, org );
	org[2] += ent->client->ps.viewheight;
	VectorMA( org, 10240, forward, dest );
	trap_Trace( &tr, org, NULL, NULL, dest, ent->s.number, MASK_OPAQUE );
	VectorCopy( tr.endpos, dest );
}

static char EntityMessage[1024];
char *G_Q3F_MessageString( char *srcptr, gentity_t *activator, gentity_t *queryent, int colour )
{
	char *buffptr, *buffendptr;
	char curr;
	gentity_t *current, *other;
	g_q3f_location_t *loc;
	char minibuff[64];
	bg_q3f_playerclass_t *cls;
	q3f_keypair_t *kp;
	int colourstack[32], colourstacksize;
	vec3_t pos;

	if( colour < 0 || colour > 31 )
	{
		colour = ColorIndex( colour );
		if( colour < 0 || colour > 31 )
			colour = 7;
	}

	colourstack[0] = colour;
	colourstacksize = 0;

	for( buffptr = messagestringbuff, buffendptr = messagestringbuff + 31999; *srcptr && buffptr < buffendptr; )
	{
		if( *srcptr == '%' || *srcptr == '$' )
		{
			curr = *(srcptr+1) | 32;
			current = (*(srcptr+1) & 32) ? queryent : activator;	// If it's upper or lower case
			other = (*(srcptr+1) & 32) ? activator : queryent;	// If it's upper or lower case
			srcptr += 2;
			if( current ) {
				switch( curr )	// Get lowercase letter;
				{
					case '1':	// slothy - fix for ET upcase problem
								current = other;
					case 'h':	// Health
								if( current->client )
								{
									ms_itoa( current->client->ps.stats[STAT_HEALTH], minibuff );
									Q_strncpyz( buffptr, minibuff, buffendptr - buffptr );
									buffptr = _MS_FixColour( buffptr, colour );
								}
								break;
					case '2':	// slothy - fix for ET upcase problem
								current = other;
					case 'a':	// Armour
								if( current->client )
								{
									ms_itoa( current->client->ps.stats[STAT_ARMOR], minibuff );
									Q_strncpyz( buffptr, minibuff, buffendptr - buffptr );
									buffptr = _MS_FixColour( buffptr, colour );
								}
								break;
					case '3':	// slothy - fix for ET upcase problem
						current = other;
					case 'l':	// Location
								loc = Team_GetLocation( current );
								Q_strncpyz( buffptr, loc ? loc->str : "unknown location", buffendptr - buffptr );
								if( current->client )
									current->client->reportLoc = loc;
								buffptr = _MS_FixColour( buffptr, colour );
								break;
					case '4':	// slothy - fix for ET upcase problem
						current = other;
					case 'd':	// Location of death
								if( current->client )
								{
									loc = current->client->deathLoc;
									Q_strncpyz( buffptr, loc ? loc->str : "unknown location", buffendptr - buffptr );
									buffptr = _MS_FixColour( buffptr, colour );
								}
								break;
					case 'r':	// Last reported location
								if( current->client )
								{
									loc = current->client->reportLoc;
									Q_strncpyz( buffptr, loc ? loc->str : "unknown location", buffendptr - buffptr );
									buffptr = _MS_FixColour( buffptr, colour );
								}
								break;
					case 'p':	// Location client is facing
								if( current->client && current->health > 0 )
								{
									_MS_TraceLocation( current, pos );
									loc = Team_GetLocationFromPos( pos );
									Q_strncpyz( buffptr, loc ? loc->str : "unknown location", buffendptr - buffptr );
									buffptr = _MS_FixColour( buffptr, colour );
								}
								break;
					case '5':	// slothy - fix for ET upcase problem
						current = other;
					case 't':	// Team
								if( current->client )
								{
									Q_strncpyz( buffptr, g_q3f_teamlist[current->client->sess.sessionTeam].description, buffendptr - buffptr );
									buffptr = _MS_FixColour( buffptr, colour );
								}
								else if( current->mapdata && current->mapdata->ownerteam )
								{
									Q_strncpyz( buffptr, g_q3f_teamlist[current->mapdata->ownerteam].description, buffendptr - buffptr );
									buffptr = _MS_FixColour( buffptr, colour );
								}
								break;
					case '6':	// slothy - fix for ET upcase problem
						current = other;
					case 'c':	// Team colour
								if( current->client )
								{
									Q_strncpyz( buffptr, g_q3f_teamlist[current->client->sess.sessionTeam].name, buffendptr - buffptr );
									buffptr = _MS_FixColour( buffptr, colour );
								}
								else if( current->mapdata && current->mapdata->ownerteam )
								{
									Q_strncpyz( buffptr, g_q3f_teamlist[current->mapdata->ownerteam].name, buffendptr - buffptr );
									buffptr = _MS_FixColour( buffptr, colour );
								}
								break;
					case '7':	// slothy - fix for ET upcase problem
						current = other;
					case 'g':	// Disguise
								if( current->client )
								{
									if( G_Q3F_IsDisguised( current ) )
									{
										if( current->client->agentdata && (current->client->agentdata->s.modelindex2 & Q3F_AGENT_INVISMASK) == Q3F_AGENT_INVIS )
										{
											Q_strncpyz( buffptr, "invisible ", buffendptr - buffptr );
											buffptr = _MS_FixColour( buffptr, colour );
										}
										Q_strncpyz( buffptr, g_q3f_teamlist[current->client->agentteam ? current->client->agentteam : current->client->sess.sessionTeam].description, buffendptr - buffptr );
										while( *buffptr ) buffptr++;	// Find end of string
										if( buffptr < buffendptr )
										{
											*buffptr++ = ' ';
											cls = bg_q3f_classlist[current->client->agentclass ? current->client->agentclass : current->client->ps.persistant[PERS_CURRCLASS]];
											if( !cls || cls == bg_q3f_classlist[Q3F_CLASS_NULL] )
												cls = BG_Q3F_GetClass( &current->client->ps );
											Q_strncpyz( buffptr, cls->title, buffendptr - buffptr );
											buffptr = _MS_FixColour( buffptr, colour );
										}
									}
									else {
										if( current->client->agentdata && (current->client->agentdata->s.modelindex2 & Q3F_AGENT_INVISMASK) == Q3F_AGENT_INVIS )
											Q_strncpyz( buffptr, "invisible", buffendptr - buffptr );
										else Q_strncpyz( buffptr, "undisguised", buffendptr - buffptr );
										buffptr = _MS_FixColour( buffptr, colour );
									}
								}
								break;
					case '8':	// slothy - fix for ET upcase problem
						current = other;
					case 's':	// Current class
								if( current->client )
								{
									cls = bg_q3f_classlist[current->client->ps.persistant[PERS_CURRCLASS]];
									if( !cls || cls == bg_q3f_classlist[Q3F_CLASS_NULL] )
										cls = BG_Q3F_GetClass( &current->client->ps );
									Q_strncpyz( buffptr, cls->title, buffendptr - buffptr );
									buffptr = _MS_FixColour( buffptr, colour );
								}
								break;
					case '9':	// slothy - fix for ET upcase problem
						current = other;
					case 'n':	// Name
								if( current->client )
								{
									Q_strncpyz( buffptr, current->client->pers.netname, buffendptr - buffptr );
									buffptr = _MS_FixColour( buffptr, colour );
								}
								else {
									if( current->mapdata && current->mapdata->other )
										kp = G_Q3F_KeyPairArrayFind( current->mapdata->other, G_Q3F_GetString( "netname" ) );
									else kp = NULL;
									if( kp )
									{
										Q_strncpyz( buffptr, kp->value.d.strdata, buffendptr - buffptr );
										buffptr = _MS_FixColour( buffptr, colour );
									}
									else if( current->classname )
									{
										Q_strncpyz( buffptr, current->classname, buffendptr - buffptr );
										buffptr = _MS_FixColour( buffptr, colour );
									}
								}
								Q_strncpyz( buffptr, "^*", buffendptr - buffptr );
								buffptr = _MS_FixColour( buffptr, colour );
								break;
					case 'e':	// State
								if( current->mapdata )
								{
									Q_strncpyz( buffptr, q3f_statestrings[current->mapdata->state], buffendptr - buffptr );
									buffptr = _MS_FixColour( buffptr, colour );
								}
								break;
					case 'x':	//class specific
								Q_strncpyz( buffptr, EntityMessage, buffendptr - buffptr );
								buffptr = _MS_FixColour( buffptr, colour );
								break;
					default:	// Another letter - leave it as-is, so it can be processed with va()
								*buffptr++ = *(srcptr - 2);
								*buffptr++ = *(srcptr - 1);
				}
			} else {
				G_Printf( "^3WARNING: G_Q3F_Messagestring tried to create a message with a NULL entity as examination target\n");
				*buffptr++ = *(srcptr - 2);
				*buffptr++ = *(srcptr - 1);
			}
		}
		else {
			*buffptr++ = *srcptr++;
			if( Q_IsColorStringPtr( srcptr - 1 ) )
			{
				if( *srcptr < '0' || toupper(*srcptr) > 'O' )
				{
					// A cancel, take the last one from the stack
					if( colourstacksize > 0 )
						colourstacksize--;
					colour = colourstack[colourstacksize];
					*buffptr++ = '0' + colour;
					srcptr++;

				}
				else {
					// A new colour, add it to the stack
					colour = ColorIndex( toupper(*srcptr) );
					*buffptr++ = *srcptr++;
					if( colourstacksize < 32 )
						colourstack[++colourstacksize] = colour;
				}
			}
		}
	}
	*buffptr = 0;	// Terminate buffer

	return( messagestringbuff );
}

void G_Q3F_EntityMessage(const char * format, ... ) {
	va_list		argptr;
	int ret;

	va_start (argptr,format);
	ret = Q_vsnprintf (EntityMessage, sizeof(EntityMessage), format, argptr);
	va_end (argptr);
	if (ret == -1)
		EntityMessage[0] = 0;
}

/*
**	The func_goalinfo - a smallish entity that simply acts as a trigger
**	without a brush. Anything entering it's bounding box makes it do it's
**	stuff. Also, it can take a model, unlike a trigger_multiple.
*/

static void G_Q3F_func_goalinfo_touch( gentity_t *self, gentity_t *other, trace_t *trace )
{
	// We need a touch function to be able to trigger.

	self->nextthink = level.time + ( self->wait + Q_flrand(-1.0f, 1.0f) * self->random );
}
static void G_Q3F_func_goalinfo_think( gentity_t *self )
{
	// A placeholder function, that does nothing except prevent a crash.
}

void SP_Q3F_func_goalinfo( gentity_t *ent )
{
	char *shaderStr;

	if( !ent->mapdata )
	{
		G_FreeEntity( ent );		// A flag with no ext. data is pretty useless
		return;
	}

	//if( !ent->wait )
	//	ent->wait = 30;		// Default wait of 30 seconds
	if( ent->wait )
		ent->wait *= 1000;		// Convert into milliseconds

		// origin position
	VectorCopy( ent->s.origin, ent->pos1 );

	ent->r.contents		= CONTENTS_TRIGGER;			// We want to know if it's touched
	ent->s.eType		= ET_Q3F_GOAL;				// And it's just, well, general ;)
	ent->physicsObject	= qfalse;
	ent->s.modelindex	= G_ModelIndex( ent->model );
	ent->s.otherEntityNum = MAX_CLIENTS;			// No holder
	ent->use			= 0;
	ent->touch			= G_Q3F_func_goalinfo_touch;
	ent->think			= G_Q3F_func_goalinfo_think;
	ent->nextthink		= 0;

	G_SpawnVector( "mins", "-15 -30 -30", ent->r.mins );
	G_SpawnVector( "maxs", "15 15 15", ent->r.maxs );

		// Find the simple shader to use.
	G_SpawnString( "simpleshader", "", &shaderStr );
	if( !shaderStr || !*shaderStr ||
		!(ent->s.torsoAnim = G_ShaderIndex( shaderStr )) )
	{
		// Use a default shader if appropriate.

		if( !Q_stricmp( ent->model,			"models/flags/r_flag.md3" ) )
			shaderStr = "textures/etf_hud/red_safe";
		else if( !Q_stricmp( ent->model,	"models/flags/b_flag.md3" ) )
			shaderStr = "textures/etf_hud/blue_safe";
		else if( !Q_stricmp( ent->model,	"models/flags/g_flag.md3" ) )
			shaderStr = "textures/etf_hud/green_safe";
		else if( !Q_stricmp( ent->model,	"models/flags/y_flag.md3" ) )
			shaderStr = "textures/etf_hud/yellow_safe";
		else if( !Q_stricmp( ent->model,	"models/flags/jap_flag_red.md3" ) )
			shaderStr = "textures/etf_hud/red_safe";
		else if( !Q_stricmp( ent->model,	"models/flags/jap_flag_blue.md3" ) )
			shaderStr = "textures/etf_hud/blue_safe";
		else if( !Q_stricmp( ent->model,	"models/flags/jap_flag_green.md3" ) )
			shaderStr = "textures/etf_hud/green_safe";
		else if( !Q_stricmp( ent->model,	"models/flags/jap_flag_yellow.md3" ) )
			shaderStr = "textures/etf_hud/yellow_safe";
		else if( !Q_stricmp( ent->model,	"models/mapobjects/keycard/keycard_red.md3" ) )
			shaderStr = "textures/etf_hud/key_safe_red";
		else if( !Q_stricmp( ent->model,	"models/mapobjects/keycard/keycard_blue.md3" ) )
			shaderStr = "textures/etf_hud/key_safe_blue";
		else if( !Q_stricmp( ent->model,	"models/objects/backpack/backpack.md3" ) )
			shaderStr = "icons/backpack";
		else if( !Q_stricmp( ent->model,	"models/objects/backpack/backpack_small.md3" ) )
			shaderStr = "icons/backpack";
		else if( !Q_stricmp( ent->model,	"models/powerups/instant/quad.md3" ) )
			shaderStr = "icons/quad";
		else if( !Q_stricmp( ent->model,	"models/powerups/instant/regen.md3" ) )
			shaderStr = "icons/regen";
		else if( !Q_stricmp( ent->model,	"models/powerups/instant/enviro.md3" ) )
			shaderStr = "icons/envirosuit";
		else if( !Q_stricmp( ent->model,	"models/powerups/instant/flight.md3" ) )
			shaderStr = "icons/flight";
		else if( !Q_stricmp( ent->model,	"models/powerups/instant/invis.md3" ) )
			shaderStr = "icons/invis";
		else if( !Q_stricmp( ent->model,	"models/powerups/instant/speed.md3" ) )
			shaderStr = "icons/haste";
		else if( !Q_stricmp( ent->model,	"models/powerups/armor/armor_red.md3" ) )
			shaderStr = "icons/iconr_red";
		else if( !Q_stricmp( ent->model,	"models/powerups/armor/armor_blue.md3" ) )
			shaderStr = "icons/iconr_blue";
		else if( !Q_stricmp( ent->model,	"models/powerups/armor/armor_green.md3" ) )
			shaderStr = "icons/iconr_green";
		else if( !Q_stricmp( ent->model,	"models/powerups/armor/armor_yellow.md3" ) )
			shaderStr = "icons/iconr_yellow";
		else if( !Q_stricmp( ent->model,	"models/powerups/armor/shard.md3" ) )
			shaderStr = "icons/iconr_shard";
		else shaderStr = NULL;

		if( shaderStr )
			ent->s.torsoAnim = G_ShaderIndex( shaderStr );
	}

	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	ent->s.apos.trType = TR_STATIONARY;

	if( ent->mapdata->flags & Q3F_FLAG_ROTATING )
		ent->s.eFlags |= EF_Q3F_ROTATING;

	trap_LinkEntity( ent );
}


/*
**	Target array maintenance
*/

static qboolean G_Q3F_AddNameToTargetArray( char *str, gentity_t *ent )
{
	// Add a string to array, assumes it's AddString created
	// True return value means the array needs resorted

	q3f_keypair_t *key;
	q3f_array_t *array;
	qboolean newkey;

	if( !str || !*str )
		return( qfalse );

	key = G_Q3F_KeyPairArrayFind( level.targetnameArray, str );
	if( !key )
	{
		if( !(array = G_Q3F_ArrayCreate()) )
			G_Error( "G_Q3F_AddNameToTargetArray(): Array creation failed." );
		if( G_Q3F_KeyPairArrayAdd( level.targetnameArray, str, Q3F_TYPE_ARRAY, 0, (int) array ) < 0 )
			G_Error( "G_Q3F_AddNameToTargetArray(): Unable to expand keypair array." );
		newkey = qtrue;
	}
	else {
		newkey = qfalse;
		array = key->value.d.arraydata;
	}

		// Add new entry, sort
	if( G_Q3F_ArrayAdd( array, Q3F_TYPE_ENTITY, 0, (int) ent ) < 0 )
		G_Error( "G_Q3F_AddNameToTargetArray(): Unable to expand array." );
	G_Q3F_ArraySort( array );

	return( newkey );
}

static qboolean G_Q3F_RemoveNameFromTargetArray( char *str, gentity_t *ent )
{
	// Remove string from array, assumes it's AddString created
	// True return value means the array needs resorted

	q3f_keypair_t *key;
	q3f_array_t *array;
	q3f_data_t *data;
	int index;

	if(	!level.targetnameArray ||
		!str || !*str ||
		!(key = G_Q3F_KeyPairArrayFind( level.targetnameArray, str )) )
		return( qfalse );

	array = key->value.d.arraydata;
	//if( !(data = G_Q3F_ArrayFind( array, (int) str )) )
	if( !(data = G_Q3F_ArrayFind( array, (int) ent )) )
		return( qfalse );			// Didn't match ent in list
	index = data - array->data;		// Find the index into the array.
	G_Q3F_ArrayDel( array, index );
	G_Q3F_ArraySort( array );

	if( !array->used )
	{
		// It's empty, remove it.

		G_Q3F_KeyPairArrayDel( level.targetnameArray, str );
//		G_Q3F_ArrayDestroy( array );		// djbob: already gets freed in the above function.
		if( !level.targetnameArray->used )
		{
			G_Q3F_KeyPairArrayDestroy( level.targetnameArray );
			level.targetnameArray = NULL;
			return( qfalse );
		}
		return( qtrue );
	}
	else if( array->max > 4 && array->used <= (array->max >> 1) )
		// Shrink down a bit
		G_Q3F_ArrayConsolidate( array );

	return( qfalse );
}

void G_Q3F_AddEntityToTargetArray( gentity_t *ent )
{
	// Add the specific targetname/groupnames to the lookup array.
	// Assumes that the name is an AddString pointer.

	q3f_data_t *data;
	int index;

	if( !level.targetnameArray )
	{
		level.targetnameArray = G_Q3F_KeyPairArrayCreate();
		if( !level.targetnameArray )
			G_Error( "G_Q3F_AddEntityToTargetArray(): Array creation failed." );
	}

	if( G_Q3F_AddNameToTargetArray( ent->targetname, ent ) )
		G_Q3F_KeyPairArraySort( level.targetnameArray );

	if( ent->mapdata && ent->mapdata->groupname )
	{
		index = -1;
		while( data = G_Q3F_ArrayTraverse( ent->mapdata->groupname, &index ) )
		{
			if( G_Q3F_AddNameToTargetArray( data->d.strdata, ent ) )
				G_Q3F_KeyPairArraySort( level.targetnameArray );
		}
	}
}

void G_Q3F_RemoveEntityFromTargetArray( gentity_t *ent )
{
	// Reverse of G_Q3F_AddEntityToTargetArray(). Assumes the
	// target/groupnames are still identical.

	q3f_data_t *data;
	int index;

	if( !level.targetnameArray )
		return;

	if( G_Q3F_RemoveNameFromTargetArray( ent->targetname, ent ) )
		G_Q3F_KeyPairArraySort( level.targetnameArray );

	if( ent->mapdata && ent->mapdata->groupname )
	{
		index = -1;
		while( data = G_Q3F_ArrayTraverse( ent->mapdata->groupname, &index ) )
		{
			if( G_Q3F_RemoveNameFromTargetArray( data->d.strdata, ent ) )
				G_Q3F_KeyPairArraySort( level.targetnameArray );
		}
	}
}

/*
==========================
target_reset entity

	"resettarget" "group1,group2,targetX"

	kill target entities and reparse them from the bsp
==========================
*/
extern char *G_AddSpawnVarToken( const char *string );
extern qboolean gotfuncdamageptrs;

void G_Q3F_KillAndRecreateEntity( gentity_t *ent ) {
	int i;
	q3f_keypair_t *data;
	// backup ent spawnindex
	int entSpawnIndex = ent->spawnIndex;
	// backup level spawnindex
	int levelSpawnIndex = level.spawnIndex;


	if( ent->spawnIndex <= 0 || !ent->mapdata || !ent->mapdata->spawVars ) {
		G_Printf( "^3WARNING: trying to kill an entity that is not resetable\n" );
		return;
	}

	// fill level.spawnVars from ent->mapdata->spawnVars
	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;

	for( i = -1; data = G_Q3F_KeyPairArrayTraverse( ent->mapdata->spawVars, &i ); ) {
		level.spawnVars[ level.numSpawnVars ][0] = G_AddSpawnVarToken( data->key );
		level.spawnVars[ level.numSpawnVars ][1] = G_AddSpawnVarToken( data->value.d.strdata );
		level.numSpawnVars++;
	}

	// kill our entity - goodbye cruel world!
	G_FreeEntity( ent );

	// but luckily there is life after dead!
	G_InitGentity( ent );

	// level.spawnvars is now filled with the spawncode for our entity
	level.spawnIndex = entSpawnIndex;
	G_SpawnGEntityFromSpawnVars( qtrue, ent );
	level.spawnIndex = levelSpawnIndex;

	// func_damage hack
	if( !Q_stricmp( ent->classname, "func_damage" ) ) {
		gotfuncdamageptrs = qfalse;
	}
	else if( !Q_stricmp( ent->classname, "misc_stopwatch" ) ) {
		EntityMessage[0] = 0;
	}
}

#define MAX_RESET_TARGETS 64
void G_Q3F_TargetResetUse( gentity_t *self, gentity_t *other, gentity_t *activator ) {
	q3f_keypair_t	*kp;
	q3f_array_t		*targets;
	q3f_data_t		*data;
	int				index, arrayindex;
	gentity_t		*resetents[MAX_RESET_TARGETS];

	if ( !self->target ) {
		return;
	}

	arrayindex = 0;
	if( kp = G_Q3F_KeyPairArrayFind( level.targetnameArray, self->target ) )
	{
		targets = kp->value.d.arraydata;
		for( index = -1; ( arrayindex < MAX_RESET_TARGETS ) && ( data = G_Q3F_ArrayTraverse( targets, &index ) ) ; )
		{
			resetents[arrayindex] = data->d.entitydata;
			if ( resetents[arrayindex] == self ) {
				G_Printf ("^3WARNING: target_reset trying to kill and recreate itself.\n");
			} else {
				arrayindex++;
			}
		}
	}

	if( !arrayindex )
		return;	// nothing to reset

	for( index = 0; index < arrayindex; index++ )
		G_Q3F_KillAndRecreateEntity( resetents[index] );

	G_Printf( "Killed and recreated %i entities\n", arrayindex );

	// go back to inactive
	G_Q3F_TriggerEntity( self, other, Q3F_STATE_INACTIVE, NULL, qtrue );
}

void SP_Q3F_target_reset( gentity_t *ent ) {
	ent->use = G_Q3F_TargetResetUse;
}
