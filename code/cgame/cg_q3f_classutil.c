/*
**	cg_q3f_classutil.c
**
**	Playerclass util functions.
**
*/

#include "cg_local.h"
#include "../game/bg_q3f_playerclass.h"

void CG_Q3F_PlayClassnameSound( int clsnum, qboolean xian )
{
	// Play a classname sample if available

	if( clsnum < 0 || clsnum >= Q3F_CLASS_MAX )
		clsnum = 0;
	// Golliwog: Remove temporarily
	if( xian )
	{
		if( cgs.media.classnamesounds[clsnum][1] )
			trap_S_StartLocalSound( cgs.media.classnamesounds[clsnum][1], CHAN_LOCAL_SOUND );
		else if( cgs.media.classnamesounds[clsnum][0] )
			trap_S_StartLocalSound( cgs.media.classnamesounds[clsnum][0], CHAN_LOCAL_SOUND );
	}
	else
	{
		if( cgs.media.classnamesounds[clsnum][0] )
			trap_S_StartLocalSound( cgs.media.classnamesounds[clsnum][0], CHAN_LOCAL_SOUND );
	}
}


