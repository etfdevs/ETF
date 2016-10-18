/*
**	g_q3f_mapselect.h
**
**	Headers for the Q3F map query/vote/select functions
*/

#ifndef	__G_Q3F_MAPSELECT_H
#define	__G_Q3F_MAPSELECT_H

#include "g_local.h"

#define	Q3F_MAPSELECT_MAXQUERY		10		// Number of maps to query at a time
#define	Q3F_MAPSELECT_QUERYDELAY	300		// Delay between queries
#define	Q3F_MAPSELECT_REPONSETIME	3000	// Time to wait for laggard responses
#define	Q3F_MAPSELECT_THRESHHOLD	0.75	// What proportion of players require a map to select it
#define	ETF_MAPSELECT_SELECTCOUNT	10		// Number of maps to offer.

#define	Q3F_MAPSELECT_VOTEALLTIME	15000	// 15 seconds if everyone has voted
#define	Q3F_MAPSELECT_VOTETIME		45000	// 45 seconds to vote for a map

enum g_q3f_mapselectmode_t {
	Q3F_MAPSELECT_NONE,
	Q3F_MAPSELECT_QUERY,
	Q3F_MAPSELECT_RESPONSE,
	Q3F_MAPSELECT_READY,
	Q3F_MAPSELECT_CHANGEMAP,
};

void G_Q3F_MapSelectQuery();
void G_Q3F_MapSelectInit();

void G_Q3F_MapSelectResponse( gentity_t *ent );
void G_Q3F_MapSelectVote( gentity_t *ent );

qboolean G_Q3F_MapSelectGetArenaField( char *buf, char *queryfield, char *outbuff, int outsize );

#endif //__G_Q3F_MAPSELECT_H
