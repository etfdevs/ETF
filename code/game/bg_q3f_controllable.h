/*
**	bg_q3f_controllable.h
**
**	Shared headers for handling 'controllable' objects, such as vehicles
**	or gun turrets.
*/

#ifndef	_BG_Q3F_CONTROLLABLE_H
#define	_BG_Q3F_CONTROLLABLE_H

typedef enum {
	CONTROLLABLE_CAR,

	NUM_CONTROLLABLES
} controllableid_t;

typedef enum {
	CONTROLLABLETYPE_WHEELED,
	CONTROLLABLETYPE_SUBMERSIBLE,
	CONTROLLABLETYPE_FLOATING,
	CONTROLLABLETYPE_FLYING,

	NUM_CONTROLLABLTYPES
} controllabletype_t;

#define MAX_CONTROL_POINTS	6

typedef struct controllabledata_s {
	int id;
	char *name;
	int type, numControlPoints;
	vec4_t controlPoints[MAX_CONTROL_POINTS];		// Each wheel/turbine/rotor, whatever
	float controlPointGive[MAX_CONTROL_POINTS];		// The amount of 'give' against solid objects.

} controllabledata_t;


#endif//_BG_Q3F_CONTROLLABLE_H
