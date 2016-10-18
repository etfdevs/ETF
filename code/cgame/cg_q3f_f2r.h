#ifndef __CG_Q3F_F2R_H
#define __CG_Q3F_F2R_H

#define MAX_F2R 196

#define TAGLENGTH	40
#ifdef CGAME
typedef struct F2RLink_s {
	SpiritScript_t		*SpiritScript;
	int					key;
	char				tag[TAGLENGTH];
	struct F2RLink_s	*next;
} F2RLink_t;
#endif

typedef struct F2RDef_s {
	char			F2RFile[256];
	qhandle_t		model;
#ifdef CGAME
	F2RLink_t		*links;
#endif
	animation_t		*animations[ANI_NUM];	// dynamically allocated pool
	int				numAnims;
	struct F2RDef_s	*next;
} F2RDef_t;

F2RDef_t *F2R_New(int handle);
int F2R_Count( );
F2RDef_t *F2R_Get( int f2rnum );
F2RDef_t *F2R_GetForModel( qhandle_t model );
int F2R_NumGet( F2RDef_t *F2RScript );
void F2R_Reset( );
void F2R_Reload( );

F2RDef_t *Parse_F2RFile( const char *F2RFile );
F2RDef_t *Load_F2RFile( const char *modelFile );
qhandle_t trap_R_RegisterModel( const char *name );			// RR2DO2: this is a wrapper function around trap_R_RealRegisterModel now

#endif
