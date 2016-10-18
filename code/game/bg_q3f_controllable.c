/*
**	bg_q3f_controllable.c
**
**	Code shared between both games for handling 'controllable' objects, such
**	as vehicles or gun turrets.
*/

#include "q_shared.h"
#include "bg_public.h"
#include "bg_local.h"
#include "bg_q3f_controllable.h"


/******************************************************************************
***** Data for controllable objects.
****/

/*
	char *name;
	int numControlPoints;
	vec4_t controlPoints[MAX_CONTROL_POINTS];		// Each wheel/turbine/rotor, whatever
	float controlPointGive[MAX_CONTROL_POINTS];		// The amount of 'give' against solid objects.
*/

controllabledata_t controllables[] =
{
	{	CONTROLLABLE_CAR,
		"Car", CONTROLLABLETYPE_WHEELED, 4,
		{
			{ 64, -32, 0, 16 },		{ 64, 32, 0, 16 },
			{ -64, -32, 0, 16 },	{ -64, 32, 0, 16 },
			{ 0, 0, 0, 0 },			{ 0, 0, 0, 0 },
		},
		{ 0.1f, 0.1f, 0.1f, 0.1f,		0, 0 },
	}
};
#define	CONTROLLABLETABLESIZE	(sizeof(controllables) / sizeof(controllabledata_t))


/******************************************************************************
***** Support functions
****/

#define FORWARD	0
#define RIGHT	1
#define UP		2

static void BG_Q3F_ControlWheelPosition( vec3_t center, vec3_t relPos, vec3_t axis[], vec3_t output )
{
}

/******************************************************************************
***** Movement control functions.
****/

qboolean BG_Q3F_ControlWheeled( pmove_t *pm )
{
	int index;
	vec3_t wheels[MAX_CONTROL_POINTS];
//	vec3_t wheelDirs[MAX_CONTROL_POINTS];
	vec3_t pos, vel, angles, angleVel;
	vec3_t axis[3];

		// Work out our current positions and movements.
	BG_EvaluateTrajectory(		&pm->cs->pos,	pm->cmd.serverTime,	pos			);
	BG_EvaluateTrajectory(		&pm->cs->apos,	pm->cmd.serverTime,	angles		);
	BG_EvaluateTrajectoryDelta(	&pm->cs->pos,	pm->cmd.serverTime,	vel			);
	BG_EvaluateTrajectoryDelta(	&pm->cs->apos,	pm->cmd.serverTime,	angleVel	);
	AngleVectors( angles, axis[FORWARD], axis[RIGHT], axis[UP] );


		// Now calculate the position and direction of each wheel.
	for( index = 0; index < MAX_CONTROL_POINTS; index++ )
	{
		BG_Q3F_ControlWheelPosition( pos, pm->cdata->controlPoints[index], axis, wheels[index] );
	}

		// Calculate player movements into wheel movements. Need to find which wheels are
		// powered and how much steering goes into each wheel?

		// Check each wheel is in contact with the ground and move it by the appropriate amount
		// if so, shifting up to seat the wheel on the ground (noting 'give', e.g. wheel pressure)

		// Map the wheel positions back into a single position and angle, which should also fix
		// Cases where the wheels get away from each other.

		// Trace to see the vehicle can actually _move_ the distance calculated.

	return( qtrue );
}

qboolean BG_Q3F_ControlSubmersible( pmove_t *pm )
{
	return( qfalse );
}

qboolean BG_Q3F_ControlFloating( pmove_t *pm )
{
	return( qfalse );
}

qboolean BG_Q3F_ControlFlying( pmove_t *pm )
{
	return( qfalse );
}


/******************************************************************************
***** The main entry point.
****/

static qboolean BG_Q3F_ControlMoveSingle( pmove_t *pm )
{
	// Run the object for a frame.

	switch( pm->cdata->type )
	{
		case CONTROLLABLETYPE_WHEELED:		return( BG_Q3F_ControlWheeled( pm ) );
		case CONTROLLABLETYPE_SUBMERSIBLE:	return( BG_Q3F_ControlSubmersible( pm ) );
		case CONTROLLABLETYPE_FLOATING:		return( BG_Q3F_ControlFloating( pm ) );
		case CONTROLLABLETYPE_FLYING:		return( BG_Q3F_ControlFlying( pm ) );
		default:							return( qfalse );
	}
}

qboolean BG_Q3F_ControlMove( pmove_t *pm )
{
	// Perform the appropriate movement code based on the type of controlled object.

	int index, finalTime, msec;

	if( !pm->cdata )
	{
		// We need to obtain the correct controldata.

		for( index = 0; index < CONTROLLABLETABLESIZE; index++ )
		{
			pm->cdata = &controllables[index];
			if( pm->cs->otherEntityNum2 == pm->cdata->id )
				break;
		}
		if( index >= CONTROLLABLETABLESIZE )
			return( qfalse );	// This deserves an error.
	}

	finalTime = pm->cmd.serverTime;
	if( finalTime < pm->ps->commandTime )
		return( qfalse );	// should not happen
	if( finalTime > pm->ps->commandTime + 1000 )
		pm->ps->commandTime = finalTime - 1000;
	pm->ps->pmove_framecount = (pm->ps->pmove_framecount+1) & ((1<<PS_PMOVEFRAMECOUNTBITS)-1);

		// chop the move up if it is too long, to prevent framerate dependent behavior
	while ( pm->ps->commandTime != finalTime )
	{
		msec = finalTime - pm->ps->commandTime;
		index = pm->pmove_fixed ? pm->pmove_msec : 66;
		if( msec > index )
			msec = index;
		pm->cmd.serverTime = pm->ps->commandTime + msec;
		
		if( !BG_Q3F_ControlMoveSingle( pm ) )
			return( qfalse );	// Something nasty happened this frame, kick them out
		// (Do we want to ensure that if they lost control the commandTime is updated for the normal movement code)
	}


	return( qtrue );
}
