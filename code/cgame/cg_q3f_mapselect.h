/*
**	cg_q3f_mapselect.h
**
**	Headers for the client side functions for selecting a map after the end of the game proper
*/

#ifndef	__CG_Q3F_MAPSELECT_H
#define	__CG_Q3F_MAPSELECT_H

#include "cg_local.h"

#define	ETF_MAPSELECT_SELECTCOUNT	10		// Number of maps to offer (no more than 10, or we'll run out of numbers)
#define	Q3F_MAPSELECT_SLIDEINTIME	3000	// Time for slide-in effect
#define	Q3F_MAPSELECT_FADEINTIME	500		// Time for fade-in effect
#define	Q3F_MAPSELECT_FADEOUTTIME	500		// Time for fade-out effect

enum cg_q3f_mapselectmode_t {
	Q3F_MAPSELECT_NONE,
	Q3F_MAPSELECT_SLIDEIN,
	Q3F_MAPSELECT_FADEIN,
	Q3F_MAPSELECT_FADEOUT,
	Q3F_MAPSELECT_READY,
};

void CG_Q3F_MapSelectTally( const char *cs );
void CG_Q3F_MapSelectInit( const char *cs );
qboolean CG_Q3F_MapSelectChoice( int choice );
qboolean CG_Q3F_MapSelectVote();
void CG_Q3F_MapSelectRespond();
qboolean CG_Q3F_MapSelectDraw();

#endif //__CG_Q3F_MAPSELECT_H
