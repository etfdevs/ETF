//#ifndef __CG_Q3F_SPIRIT_H
//#define __CG_Q3F_SPIRIT_H

#include "tr_types.h"

#define SPIRIT_SCRIPT_LOOPEDSOUND	0x1

typedef struct SpiritScript_s {
	char filename[256];
	struct SpiritSystem_s *SpiritSystem;
	int flags;
	struct SpiritScript_s *link;
	qhandle_t sound;
} SpiritScript_t;

void InitParticles( void );
int Particle_Count(void);
void Spirit_AddParticles(void);
void Spirit_PrepareFrame(void);

void Spirit_SetCustomColor( const vec3_t color );
void Spirit_SetCustomShader( const qhandle_t shader );
void Spirit_RunModel( const SpiritScript_t *SpiritScript, const refEntity_t *re, const char * tagname, int key );
void Spirit_RunScript( const SpiritScript_t *SpiritScript, const vec3_t origin, const vec3_t oldorigin, const vec3_t axis[3], int key );
qboolean Spirit_UpdateScript( const SpiritScript_t *SpiritScript, const vec3_t origin, const vec3_t axis[3], int key );
qboolean Spirit_UpdateModel( const SpiritScript_t *SpiritScript, const refEntity_t *re, const char * tagname, int key );

SpiritScript_t *Spirit_LoadScript( const char *filename );
void Spirit_AddStandAlone( char * script,vec3_t origin, vec3_t dir, float rotation );

void Spirit_SetupKeywordHash( void );
int Spirit_ScriptCount( );
int Spirit_SystemCount( );
void Spirit_Reset( );
void Spirit_Reload( );

//#endif
