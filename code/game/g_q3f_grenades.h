/*
**	g_grenades.h
**
**	Server side functions for grenade handling.
*/

#ifndef	__G_GRENADES_H
#define	__G_GRENADES_H

#include "bg_q3f_grenades.h"

typedef struct g_q3f_grenade_s {
	bg_q3f_grenade_t *g;
	void		(*ThrowGren)( struct gentity_s *ent );		// Custom throw function
	qboolean	(*ExplodeGren)( struct gentity_s *ent );	// Custom explode function
} g_q3f_grenade_t;

extern g_q3f_grenade_t *g_q3f_grenades[Q3F_NUM_GRENADES];

g_q3f_grenade_t *G_Q3F_GetGrenade( int index );

void G_Q3F_RunGrenade( gentity_t *ent );		// Run function for grenades
qboolean G_Q3F_GrenadeCommand( gentity_t *ent );

//Canabis, common function to check for burning gas
extern void HallucinoGenicCheckFire(vec3_t flamepos,float flameradius,gentity_t * owner);

#endif //__G_GRENADES_H

