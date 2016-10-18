/*
**	cg_q3f_mapsentry.c
**
**	Handles all mapsentry rendering and animation
*/

/*
	g_q3f_mapsentry.c has authoritative versions of these -

	s.legsAnim		- Type.
	s.frame			- Signal new firing			
	s.angles		- The normal vector of the sentry.
	s.angles2		- The normal angle of the sentry.
	s.pos			- Trajectory/position.
	s.apos			- Trajectory/angle of barrel, before base angles are taken into account.
*/

#include "cg_local.h"

typedef struct cg_q3f_mapsentry_s {
	qboolean hadInit;
	const char *turretModelName, *gunModelName, *barrelModelName;
	qhandle_t turretModel, gunModel, barrelModel;
	const char *fireSoundName;
	qhandle_t  fireSound;
} cg_q3f_mapsentry_t;

static cg_q3f_mapsentry_t cg_q3f_mapSentryMini = {
	qfalse,
	"models/objects/mapsentry/turret.md3",
	"models/objects/mapsentry/gun.md3",
	"models/objects/mapsentry/barrel_minigun.md3",
	0, 0, 0,
	"sound/weapons/deploy/sentry_fire.wav", 
	0
};

static cg_q3f_mapsentry_t cg_q3f_mapSentryRocket = {
	qfalse,
	"models/objects/mapsentry/turret.md3",
	"models/objects/mapsentry/gun.md3",
	"models/objects/mapsentry/barrel_rocket.md3",
	0, 0, 0,
	"sound/weapons/rocket/rocket_fire.wav",
	0
};


void CG_Q3F_MapSentry( centity_t *cent )
{
	// Render the map sentry.

	entityState_t *state;
	cg_q3f_mapsentry_t *sentry;
	refEntity_t turret, gun, barrel;
	vec3_t matrix[3], axis[3];
	vec3_t angles;

	state = &cent->currentState;
	if (state->legsAnim) 
		sentry = &cg_q3f_mapSentryRocket;
	else 
		sentry = &cg_q3f_mapSentryMini;

	/* Load map sentry data if needed */
	if (!sentry->hadInit ) {
		sentry->hadInit = qtrue;
		sentry->turretModel = trap_R_RegisterModel( sentry->turretModelName );
		sentry->gunModel = trap_R_RegisterModel( sentry->gunModelName );
		sentry->barrelModel = trap_R_RegisterModel( sentry->barrelModelName );
		sentry->fireSound = trap_S_RegisterSound( sentry->fireSoundName, qfalse );
	}	

	// Position the turret.
	memset( &turret, 0, sizeof(turret) );
	VectorCopy( cent->lerpOrigin, turret.origin );
	VectorCopy( state->angles, angles );

	angles[PITCH] += 90;	// The turret model needs to be tipped 90 degrees... 
	AnglesToAxis( angles, axis );

	VectorClear(angles);
	angles[YAW] = cent->lerpAngles[YAW];
	AnglesToAxis( angles, matrix );
	MatrixMultiply( matrix, axis, turret.axis );
	turret.hModel = sentry->turretModel;
	trap_R_AddRefEntityToScene( &turret, cent );

	// Stick the gun onto the turret
	memset( &gun, 0, sizeof(gun) );
	angles[ROLL] = 0;
	angles[YAW] = 0;	// It's attached to the already yaw'ed base
	angles[PITCH] = cent->lerpAngles[PITCH] - 90;
	AnglesToAxis( angles, gun.axis );
	CG_PositionRotatedEntityOnTag( &gun, &turret, "tag_cannon" );
	gun.hModel = sentry->gunModel;
	trap_R_AddRefEntityToScene( &gun, cent );

	// Stick the barrel onto the gun.
	memset( &barrel, 0, sizeof(barrel) );
	AxisClear( barrel.axis );
	CG_PositionRotatedEntityOnTag( &barrel, &gun, "tag_barrel" );
	barrel.hModel = sentry->barrelModel;
	trap_R_AddRefEntityToScene( &barrel, cent );

	/* Check for new sounds/flashes */
	if ( state->frame != cent->miscTime ) {
		cent->miscTime = state->frame;
		trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, sentry->fireSound );
	}
}
