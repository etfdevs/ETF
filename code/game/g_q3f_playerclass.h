/*
**	q3f_playerclass.h
**
**	Definitions for player classes
**
*/

#ifndef __G_Q3F_PLAYERCLASS_H
#define __G_Q3F_PLAYERCLASS_H

#include "q_shared.h"
#include "g_local.h"
#include "bg_q3f_playerclass.h"

#define G_Q3F_PARAMEDIC_HEAL_INTERVAL	3000
#define G_Q3F_PARAMEDIC_HEAL_AMOUNT		2


typedef struct g_q3f_playerclass_s {
	bg_q3f_playerclass_t *s;

	// Class init/term
	void	(*BeginClass)( struct gentity_s *ent );
	void	(*EndClass)( struct gentity_s *ent );
	void	(*DeathCleanup)( struct gentity_s *ent );

	// Client command
	qboolean (*ClientCommand)( struct gentity_s *ent, char *cmd );

} g_q3f_playerclass_t;

extern g_q3f_playerclass_t *g_q3f_classlist[];			// Array of pointers to class structures


	// Client and server functions
g_q3f_playerclass_t *G_Q3F_GetClass( const playerState_t *ps );	// Get a pointer to a class structure

	// Server only functions
qboolean G_Q3F_ChangeClassCommand( struct gentity_s *ent, char *cmd );
qboolean G_Q3F_GlobalCommand( struct gentity_s *ent, char *cmd );
void Q3F_SetupClass(struct gentity_s *ent);
qboolean Q3F_IsSpectator(struct gclient_s *client);
int G_Q3F_SelectRandomClass( int teamnum, gentity_t *ent );
void G_Q3F_SendClassMenu( gentity_t *player, int teamnum );
void G_Q3F_DropClient( gentity_t *ent, const char *reason );
void G_Q3F_MuteClient( gentity_t *ent, qboolean mute );
void G_Q3F_SetClassMaskString(void);

	// Class Specific Functions
qboolean G_Q3F_Grenadier_Command( struct gentity_s *ent, char *cmd );
qboolean G_Q3F_Recon_Command( struct gentity_s *ent, char *cmd );
void G_Q3F_ToggleScanner(struct gentity_s *ent);

// Cleanup functions
void G_Q3F_Sniper_Death_Cleanup( struct gentity_s *ent);
void G_Q3F_Minigunner_Death_Cleanup( struct gentity_s *ent);
void G_Q3F_Global_Death_Cleanup( struct gentity_s *self);
void G_Q3F_Grenadier_Term_Cleanup( gentity_t *ent );
void G_Q3F_Grenadier_Death_Cleanup( struct gentity_s *ent);
void G_Q3F_Recon_Death_Cleanup( struct gentity_s *ent);
void G_Q3F_Agent_Term_Cleanup( struct gentity_s *ent );
void G_Q3F_Agent_Death_Cleanup( struct gentity_s *ent );
void G_Q3F_Engineer_Term_Cleanup( struct gentity_s *ent );
void G_Q3F_Engineer_Death_Cleanup( struct gentity_s *ent );
void G_Q3F_GenericEndCleanup( struct gentity_s *self );
void G_Q3F_SendClassInfo( struct gentity_s *player );

#endif	// __G_Q3F_PLAYERCLASS_H

