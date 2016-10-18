/*
**	g_q3f_flag.h
**
**	Server-side stuff for Q3F flags entities (which will be a large
**	part of the whole lot)
*/

#ifndef __G_Q3F_FLAG_H
#define	__G_Q3F_FLAG_H


#include "g_local.h"
#include "g_q3f_mapdata.h"


void G_Q3F_FlagInfo( gentity_t *queryent );		// Query flag status.

void SP_Q3F_func_flag( gentity_t *ent );		// Spawn a func_flag

void Q3F_func_flag_touch( gentity_t *self, gentity_t *other, trace_t *trace );
void Q3F_func_flag_think( gentity_t *self );
void Q3F_func_flag_use( gentity_t *self, gentity_t *other, gentity_t *activator );

void G_Q3F_FlagUseHeld( gentity_t *player );	// 'use' specified flag.

float G_Q3F_CalculateGoalItemSpeedScale( gentity_t *player );	// Calculate speed scale.
void G_Q3F_DropFlag( gentity_t *ent );			// Drop the flag
void G_Q3F_DropAllFlags( gentity_t *player, qboolean ignorenodrop, qboolean ignorekeepondeath );	// Drop all a player's flags
void G_Q3F_ReturnFlag( gentity_t *ent );		// Return the flag

	// See if holder has all the named entities
qboolean G_Q3F_CheckHeld( gentity_t *holder, q3f_array_t *array );
qboolean G_Q3F_CheckNotHeld( gentity_t *holder, q3f_array_t *array );


#endif//__G_Q3F_FLAG_H
