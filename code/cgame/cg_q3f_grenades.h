/*
**	cg_grenades.h
**
**	Client side functions for grenade handling.
*/

#ifndef	__CG_GRENADES_H
#define	__CG_GRENADES_H

#include "../game/bg_q3f_grenades.h"

typedef struct cg_q3f_grenade_s {
	bg_q3f_grenade_t *g;

	qboolean (*RenderGren)( struct centity_s *cent, refEntity_t *ent );	// Custom render

	qhandle_t hModel;			// Not set statically, a dynamic fudge thing :)
	qhandle_t hSkin;

} cg_q3f_grenade_t;

extern cg_q3f_grenade_t *cg_q3f_grenades[Q3F_NUM_GRENADES];

cg_q3f_grenade_t *CG_Q3F_GetGrenade( int index );

void CG_Q3F_Grenade( centity_t *cent );		// Run function for grenades

void CG_Q3F_GrenOnePrime( void );		// Prime and throw grenade
void CG_Q3F_GrenTwoPrime( void );
void CG_Q3F_GrenThrow( void );
void CG_Q3F_GrenOnePlusPrime( void );
void CG_Q3F_GrenOneThrow( void );
void CG_Q3F_GrenTwoPlusPrime( void );
void CG_Q3F_GrenTwoThrow( void );
void CG_Q3F_GrenOneToggle( void );
void CG_Q3F_GrenTwoToggle( void );

#endif//__CG_GRENADES_H
