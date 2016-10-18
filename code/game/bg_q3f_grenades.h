/*
**	bg_q3f_grenades.h
**
**	Common functions for grenade handling.
*/

#ifndef	__BG_GRENADES_H
#define	__BG_GRENADES_H

#include "q_shared.h"

enum {
	// Types of grenades

	Q3F_GREN_NONE,

	Q3F_GREN_NORMAL,
	Q3F_GREN_CONCUSS,
	Q3F_GREN_FLASH,
	Q3F_GREN_FLARE,
	Q3F_GREN_NAIL,
	Q3F_GREN_CLUSTER,
	Q3F_GREN_CLUSTERSECTION,
	Q3F_GREN_NAPALM,
	Q3F_GREN_GAS,
	Q3F_GREN_EMP,
	Q3F_GREN_CHARGE,

	Q3F_NUM_GRENADES
};

#define	Q3F_GFLAG_QUICKTHROW		0x01
#define	Q3F_GFLAG_STICKY			0x02
#define	Q3F_GFLAG_NOEXPLODE			0x04
#define	Q3F_GFLAG_NOTHROW			0x08
#define	Q3F_GFLAG_NOSPIN			0x10
#define	Q3F_GFLAG_NOSOUND			0x20
#define	Q3F_GFLAG_EXTENDEDEFFECT	0x40
#define Q3F_GFLAG_LIESFLAT			0x80

typedef struct bg_q3f_grenade_s {
	int flags;
	int mod, damage;			// Initial detonation values
	char *name, *model, *skin;
//	float light;
//	vec3_t lightColor;
} bg_q3f_grenade_t;

extern bg_q3f_grenade_t *bg_q3f_grenades[Q3F_NUM_GRENADES];

bg_q3f_grenade_t *BG_Q3F_GetGrenade( int index );
void BG_Q3F_ConcussionEffect( int seed, int left, vec3_t out );

#endif //__BG_GRENADES_H
