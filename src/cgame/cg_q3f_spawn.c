/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

Enemy Territory Fortress
Copyright (C) 2000-2006 Quake III Fortress (Q3F) Development Team / Splash Damage Ltd.
Copyright (C) 2005-2018 Enemy Territory Fortress Development Team

This file is part of Enemy Territory Fortress (ETF).

ETF is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

ETF is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with ETF. If not, see <http://www.gnu.org/licenses/>.

In addition, the Wolfenstein: Enemy Territory GPL Source Code is also subject to certain additional terms. You should have received a copy of these additional terms immediately following the terms and conditions of the GNU General Public License which accompanied the ETF Source Code.  If not, please request a copy in writing from id Software at the address below.

If you have questions concerning this license or the applicable additional terms, you may contact in writing id Software LLC, c/o ZeniMax Media Inc., Suite 120, Rockville, Maryland 20850 USA.

===========================================================================
*/

/*
**	cg_q3f_spawn.c
**
**	Client-side functions for parsing entity data.
*/

#include "cg_local.h"

#define	MAX_SPAWN_VARS			64
#define	MAX_SPAWN_VARS_CHARS	2048

static int	numSpawnVars, numSpawnVarChars, spawnIndex;
static char	*spawnVars[MAX_SPAWN_VARS][2];	// key / value pairs
static char	spawnVarChars[MAX_SPAWN_VARS_CHARS];


/******************************************************************************
***** Entity support functions
****/

static char *CG_Q3F_GetEntValue( const char *key )
{
	// Return the value matching the specified key.

	int index;
	for( index = 0; index < numSpawnVars; index++ )
	{
		if( !Q_stricmp( spawnVars[index][0], key ) )
			return( spawnVars[index][1] );
	}
	return( NULL );
}

static qboolean CG_Q3F_SpawnString( const char *key, const char *defaultString, char **out ) {
	const char		*s;
	qboolean	present;

	s = CG_Q3F_GetEntValue( key );
	if( !(present = s != NULL) )
		s = defaultString;
	*out = (char *)s;
	return present;
}

static qboolean CG_Q3F_SpawnFloat( const char *key, const char *defaultString, float *out ) {
	const char		*s;
	qboolean	present;

	s = CG_Q3F_GetEntValue( key );
	if( !(present = s != NULL) )
		s = defaultString;
	*out = atof( s );
	return present;
}

static qboolean CG_Q3F_SpawnInt( const char *key, const char *defaultString, int *out ) {
	const char		*s;
	qboolean	present;

	s = CG_Q3F_GetEntValue( key );
	if( !(present = s != NULL) )
		s = defaultString;
	*out = atoi( s );
	return present;
}

/*static qboolean CG_Q3F_SpawnBoolean( const char *key, const char *defaultString, qboolean *out ) {
	const char		*s;
	qboolean	present;

	s = CG_Q3F_GetEntValue( key );

	if( !(present = s != NULL) )
		s = defaultString;
	if ( !Q_stricmp( s, "qfalse" ) || !Q_stricmp( s, "false" ) || !Q_stricmp( s, "no" ) || !Q_stricmp( s, "0" ) ) {
		*out = qfalse;
	} else if ( !Q_stricmp( s, "qtrue" ) || !Q_stricmp( s, "true" ) || !Q_stricmp( s, "yes" ) || !Q_stricmp( s, "1" ) ) {
		*out = qtrue;
	} else {
		*out = qfalse;
	}
	return present;
}*/

static qboolean CG_Q3F_SpawnVector( const char *key, const char *defaultString, float *out ) {
	const char		*s;
	qboolean	present;

	s = CG_Q3F_GetEntValue( key );
	if( !(present = s != NULL) )
		s = defaultString;
	sscanf( s, "%f %f %f", &out[0], &out[1], &out[2] );
	return present;
}

//Keeger for tracemap
qboolean CG_SpawnVector2D( const char *key, const char *defaultString, float *out ) {
	const char		*s;
	qboolean	present;

	//present = CG_SpawnString( key, defaultString, &s );
	s = CG_Q3F_GetEntValue(key);
	if( !(present = s != NULL) )
		s = defaultString;
	sscanf( s, "%f %f", &out[0], &out[1] );
	return present;
}

static qboolean CG_Q3F_SpawnColor( const char *defaultString, float *out ) {
	// Golliwog: Convenience function for colour keys.

	return( CG_Q3F_SpawnVector( "_color", defaultString, out ) ||
			CG_Q3F_SpawnVector( "color", defaultString, out ) ||
			CG_Q3F_SpawnVector( "_colour", defaultString, out ) ||
			CG_Q3F_SpawnVector( "colour", defaultString, out ) );
}


static void CG_Q3F_AddEntityData( void *data, int datasize, int alignment )
{
	// Add arbitrary data to the current entity.

	void *allocated;

	if( cgs.numEntityData >= ARRAY_LEN(cgs.entityIndex) )
		CG_Error( "Out of entity data space" );

	allocated = CG_Q3F_AddBlock( datasize, alignment );
	memcpy( allocated, data, datasize );

	cgs.entityData[cgs.numEntityData]	= allocated;
	cgs.entityIndex[cgs.numEntityData]	= spawnIndex;
	cgs.numEntityData++;
}


/******************************************************************************
***** Entity processing
****/

static void SP_WorldSpawn()
{
	char *s;
	cg.spawning = qtrue;

	s = CG_Q3F_GetEntValue( "celshader" );

	if( s )
		cgs.media.celshader = trap_R_RegisterShader( s );
	else
		cgs.media.celshader = 0;

   //Keeger  need mapMins and mapMaxs for tracemap code to be happy.
	if( CG_SpawnVector2D( "mapcoordsmins", "-128 128", cg.mapcoordsMins ) &&	// top left
		CG_SpawnVector2D( "mapcoordsmaxs", "128 -128", cg.mapcoordsMaxs ) ) {	// bottom right
		cg.mapcoordsValid = qtrue;
	} else {
		cg.mapcoordsValid = qfalse;
	}
	cg.spawning = qfalse;
}

static void SP_Target_Location()
{
	vec3_t loc;
	char *s;

	if( cgs.numLocations >= (int)(sizeof(cgs.locations) / sizeof(cg_q3f_location_t)) )
		return;

	if(	!CG_Q3F_SpawnVector( "origin", "0 0 0", loc ) ||
		!(s = CG_Q3F_GetEntValue( "message" )) )
		return;

	VectorCopy( loc, cgs.locations[cgs.numLocations].pos );
	Q_CleanStr( s );
	cgs.locations[cgs.numLocations].str = CG_Q3F_AddString( s );
	cgs.numLocations++;
}

static void SP_Panel_Message()
{
	// Store the message strings for client-side rendering.

	char *statestrings[5];
	qboolean hasmessage;

	hasmessage = qfalse;
	if( (statestrings[Q3F_STATE_CARRIED] = CG_Q3F_GetEntValue( "carriedmessage" )) != NULL )
	{
		statestrings[Q3F_STATE_CARRIED] = CG_Q3F_AddString( statestrings[Q3F_STATE_CARRIED] );
		hasmessage = qtrue;
	}
	if( (statestrings[Q3F_STATE_ACTIVE] = CG_Q3F_GetEntValue( "activemessage" )) != NULL )
	{
		statestrings[Q3F_STATE_ACTIVE] = CG_Q3F_AddString( statestrings[Q3F_STATE_ACTIVE] );
		hasmessage = qtrue;
	}
	if( (statestrings[Q3F_STATE_INACTIVE] = CG_Q3F_GetEntValue( "inactivemessage" )) != NULL )
	{
		statestrings[Q3F_STATE_INACTIVE] = CG_Q3F_AddString( statestrings[Q3F_STATE_INACTIVE] );
		hasmessage = qtrue;
	}
	if( (statestrings[Q3F_STATE_DISABLED] = CG_Q3F_GetEntValue( "disabledmessage" )) != NULL )
	{
		statestrings[Q3F_STATE_DISABLED] = CG_Q3F_AddString( statestrings[Q3F_STATE_DISABLED] );
		hasmessage = qtrue;
	}
	if( (statestrings[Q3F_STATE_INVISIBLE] = CG_Q3F_GetEntValue( "invisiblemessage" )) != NULL )
	{
		statestrings[Q3F_STATE_INVISIBLE] = CG_Q3F_AddString( statestrings[Q3F_STATE_INVISIBLE] );
		hasmessage = qtrue;
	}

	if( hasmessage )
		CG_Q3F_AddEntityData( statestrings, sizeof(statestrings), sizeof(char *) );
}

/*QUAKED misc_flare (1.0 0.9 0) (-4 -4 -4) (4 4 4) LENSFLARE LENSBLIND
spawns the flare entity.

spawnflags:
LENSFLARE:		makes it a lensflare
LENSBLIND:		makes it blinds the screen
LENSFLAREBLIND:	makes it a lensflare that blinds the screen

keys:
light:		flare intensity, equiv to light (default: 300)
_color:		flare color, equiv to light (default 1.0 1.0 1.0)
shader:		path to shader (default: "flareshader")
radius:		flare radius (default: 1.0, about 64x64 pixels @ 640x480)
rotation:	initial rotation of light flare
*/
static void SP_Misc_Flare() {
	cg_q3f_flare_t flare;
	char *s;
	int spawnflags;

	if( cgs.numFlares >= (int)(sizeof(cgs.flares) / sizeof(cg_q3f_flare_t)) )
		return;

	memset( &flare, 0, sizeof(cg_q3f_flare_t) );

	if( !CG_Q3F_SpawnVector( "origin", "0 0 0", flare.pos ) )
		return;

	CG_Q3F_SpawnFloat( "radius", "1.0", &flare.radius );
	CG_Q3F_SpawnFloat( "light", "300", &flare.intensity );
	CG_Q3F_SpawnInt( "rotation", "0", &flare.rotation );
	CG_Q3F_SpawnColor( "1 1 1", flare.color );
	s = CG_Q3F_GetEntValue( "shader" );
	CG_Q3F_SpawnInt( "spawnflags", "0", &spawnflags );

	flare.rotation %= 360;

	if( s ) {
		if(!Q_stricmp(s, "flareshader"))
			flare.shader = trap_R_RegisterShader( "etf_flareshader" );
		else
			flare.shader = trap_R_RegisterShader( s );
	}
	else
		flare.shader = trap_R_RegisterShader( "etf_flareshader" );

	if( spawnflags & 3 )
		flare.type = FL_LENSFLAREBLIND;
	else if( spawnflags & 2 )
		flare.type = FL_LENSBLIND;
	else if( spawnflags & 1 )
		flare.type = FL_LENSFLARE;
	else 
		flare.type = FL_FLARE;

	memcpy( &cgs.flares[cgs.numFlares++], &flare, sizeof(cg_q3f_flare_t) );
}

/*QUAKED misc_sunflare (0.5 0.9 0) (-4 -4 -4) (4 4 4) LENSFLARE LENSBLIND
special flare entity that can be put in a skybox and still renders properly

spawnflags:
LENSFLARE:	makes it a lensflare
LENSBLIND:		makes it blinds the screen
LENSFLAREBLIND:	makes it a lensflare that blinds the screen

keys:
light:		flare intensity, equiv to light (default: 300)
_color:		flare color, equiv to light (default 1.0 1.0 1.0)
shader:		path to shader (default: "flareshader")
radius:		flare radius (default: 1.0, about 64x64 pixels @ 640x480)
rotation:	initial rotation of light flare
*/
static void SP_Misc_SunFlare() {
	cg_q3f_flare_t flare;
	char *s;
	int spawnflags;

	if( cgs.numSunFlares >= (int)(sizeof(cgs.sunFlares) / sizeof(cg_q3f_flare_t)) )
		return;

	memset( &flare, 0, sizeof(cg_q3f_flare_t) );

	if( !CG_Q3F_SpawnVector( "origin", "0 0 0", flare.pos ) )
		return;

	CG_Q3F_SpawnFloat( "radius", "1.0", &flare.radius );
	CG_Q3F_SpawnFloat( "light", "300", &flare.intensity );
	CG_Q3F_SpawnInt( "rotation", "0", &flare.rotation );
	CG_Q3F_SpawnColor( "1 1 1", flare.color );
	s = CG_Q3F_GetEntValue( "shader" );
	CG_Q3F_SpawnInt( "spawnflags", "0", &spawnflags );

	flare.rotation %= 360;

	if( s ) {
		if(!Q_stricmp(s, "flareshader"))
			flare.shader = trap_R_RegisterShader( "etf_flareshader" );
		else
			flare.shader = trap_R_RegisterShader( s );
	}
	else
		flare.shader = trap_R_RegisterShader( "etf_flareshader" );

	if( spawnflags & 3 )
		flare.type = FL_LENSFLAREBLIND;
	else if( spawnflags & 2 )
		flare.type = FL_LENSBLIND;
	else if( spawnflags & 1 )
		flare.type = FL_LENSFLARE;
	else
		flare.type = FL_FLARE;

	memcpy( &cgs.sunFlares[cgs.numSunFlares++], &flare, sizeof(cg_q3f_flare_t) );
}

/*QUAKED misc_particlesystem (0.5 0.9 0) (-4 -4 -4) (4 4 4)
spawns a Spirit system, used to create a particle system without using models

keys:
script:		points to a .f2r script
dir:		directional vector at which to emit particles (equivalent to
			the normal vector of a tag when spawning a Spirit system
			through F2R)
rotation:	rotation of the axis
*/
static void SP_Misc_ParticleSystem() {
	vec3_t origin, dir;
	float rotation;
	char *s;
	
	if( !CG_Q3F_SpawnVector( "origin", "0 0 0", origin ) )
		return;

	if( !CG_Q3F_SpawnVector( "dir", "0 0 0", dir ) )
		return;

	CG_Q3F_SpawnFloat( "rotation", "0", &rotation );

	if( !(s = CG_Q3F_GetEntValue( "script" ) ) )
		return;

	Spirit_AddStandAlone( s, origin, dir, rotation );
}

void CG_Q3F_ParallaxShader( char *spawnVars[], int numSpawnVars, vec3_t entOrigin, float xscale, float yscale, float xoff, float yoff, float angle );
static void SP_Misc_Parallax()
{
	vec3_t org;
	float xscale, yscale, xoff, yoff, angle;
	
	CG_Q3F_SpawnFloat( "xscale", "100", &xscale );
	CG_Q3F_SpawnFloat( "yscale", "100", &yscale );
	CG_Q3F_SpawnFloat( "xoff", "0", &xoff );
	CG_Q3F_SpawnFloat( "yoff", "0", &yoff );
	CG_Q3F_SpawnFloat( "angle", "0", &angle );
	if( CG_Q3F_SpawnVector( "origin", "0 0 0", org ) )
		CG_Q3F_ParallaxShader( (char **) spawnVars, numSpawnVars, org, xscale, yscale, xoff, yoff, angle );
}

#ifdef Q3F_WATER
void CG_Q3F_WaterShader( vec3_t entOrigin, int xwidth, int ywidth, int xsub, int ysub, float sscale, float tscale, const char *shader );
#endif // Q3F_WATER
static void SP_Misc_Water() {
#ifdef Q3F_WATER
	vec3_t org;

	int xwidth, ywidth, xsub, ysub;
	float sscale, tscale;
	const char *s;

	CG_Q3F_SpawnInt( "xwidth", "256", &xwidth );
	CG_Q3F_SpawnInt( "ywidth", "256", &ywidth );
	CG_Q3F_SpawnInt( "xsub", "32", &xsub );
	CG_Q3F_SpawnInt( "ysub", "32", &ysub );
	CG_Q3F_SpawnFloat( "sscale", "1", &sscale );
	CG_Q3F_SpawnFloat( "tscale", "1", &tscale );
	s = CG_Q3F_GetEntValue( "shader" );
	if( CG_Q3F_SpawnVector( "origin", "0 0 0", org ) )
		CG_Q3F_WaterShader( org, xwidth, ywidth, xsub, ysub, sscale, tscale, s );
#endif // Q3F_WATER
}


static void SP_Misc_SkyPortal(void) {
	char *line;
	if( !CG_Q3F_SpawnVector( "origin", "0 0 0", cgs.skyportal.origin ) )
		return;
	line = CG_Q3F_GetEntValue( "portalshader" );
	if (!line) 
		return;
	Q_strncpyz( cgs.skyportal.portalShader, line, sizeof( cgs.skyportal.portalShader ));
	line = CG_Q3F_GetEntValue( "disableshader" );
	if (!line) 
		return;
	Q_strncpyz( cgs.skyportal.disableShader, line, sizeof( cgs.skyportal.disableShader ));
	cgs.skyportal.hasportal = qtrue;
}

static void SP_Misc_SunPortal(void) {
	if( cgs.sunportal.exists )
		return;
	if( !CG_Q3F_SpawnVector( "origin", "0 0 0", cgs.sunportal.origin ) )
		return;
	cgs.sunportal.exists = qtrue;
}

static void SP_Misc_MapSentry(void) {
	char *str;
	// Find the correct type to use.
	CG_Q3F_SpawnString( "type", "minigun", &str );
	if ( !Q_stricmp( str, "minigun" ) ) {
		CG_Q3F_RegisterMapSentry( qfalse );
	} else if ( !Q_stricmp( str, "rocketlauncher" ) ) {
		CG_Q3F_RegisterMapSentry( qtrue );
	}
}

typedef struct {
	char *classname;
	void (* processor)();
} CG_Q3F_EntityProcessor_t;

static CG_Q3F_EntityProcessor_t processors[] = {
	{	"worldspawn",			SP_WorldSpawn			},
	{	"target_location",		SP_Target_Location		},
	{	"panel_message",		SP_Panel_Message		},
	{	"misc_flare",			SP_Misc_Flare			},
	{	"misc_sunflare",		SP_Misc_SunFlare,		},
	{	"misc_particlesystem",	SP_Misc_ParticleSystem	},
	{	"misc_parallax",		SP_Misc_Parallax		},
	{	"misc_water",			SP_Misc_Water			},
	{	"misc_skyportal",		SP_Misc_SkyPortal		},
	{	"misc_sunportal",		SP_Misc_SunPortal		},
	{	"misc_mapsentry",		SP_Misc_MapSentry },		// RR2DO2: Mapsentry
};
#define NUMPROCESSORS	(sizeof(processors)/sizeof(CG_Q3F_EntityProcessor_t))


/******************************************************************************
***** Post-processing cleanup.
****/

static int QDECL LS_SortFunc( const void *a, const void *b )
{
	// Comparison function

	cg_q3f_location_t *la, *lb;

	la = (cg_q3f_location_t *) a;
	lb = (cg_q3f_location_t *) b;
	if( la->pos[0] != lb->pos[0] )
		return( la->pos[0] - lb->pos[0] );
	if( la->pos[1] != lb->pos[1] )
		return( la->pos[1] - lb->pos[1] );
	return( la->pos[2] - lb->pos[2] );
}
void CG_Q3F_LocationSort()
{
	// Sort the locations so we can do (slightly) faster lookups.

	qsort( cgs.locations, cgs.numLocations, sizeof(cg_q3f_location_t), &LS_SortFunc );
}

/******************************************************************************
***** Entity data parsing.	
****/

static char *CG_Q3F_AddSpawnVarToken( const char *string ) {
	int		l;
	char	*dest;

	l = strlen( string );
	if ( numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS ) {
		CG_Error( "CG_Q3F_AddSpawnVarToken: MAX_SPAWN_VARS_CHARS" );
	}

	dest = spawnVarChars + numSpawnVarChars;
	memcpy( dest, string, l+1 );

	numSpawnVarChars += l + 1;

	return dest;
}

/*
====================
CG_Q3F_ParseSpawnVars

Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVars[]

This does not actually spawn an entity.

The observant may notice it's basically lifted from g_spawn.c
====================
*/
static qboolean CG_Q3F_ParseSpawnVars()
{
	char		keyname[MAX_TOKEN_CHARS];
	char		token[MAX_TOKEN_CHARS];

	numSpawnVars = 0;
	numSpawnVarChars = 0;

	// parse the opening brace
	if( !trap_GetEntityToken( token, sizeof(token) ) || !token[0] )
	// end of spawn string
		return qfalse;

	if( token[0] != '{' )
		CG_Error( "CG_Q3F_ParseSpawnVars: found %s when expecting {", token );

	// go through all the key / value pairs
	while ( 1 ) {
		// parse key
		if( !trap_GetEntityToken( token, sizeof(token) ) )
			CG_Error( "CG_Q3F_ParseSpawnVars: EOF without closing brace" );
		if( token[0] == '}' )
			break;

		Q_strncpyz( keyname, token, sizeof(keyname) );
		
		// parse value	
		if( !trap_GetEntityToken( token, sizeof(token) ) ) {
			CG_Error( "CG_Q3F_ParseSpawnVars: EOF without closing brace" );
		}

		if( token[0] == '}' )
			CG_Error( "CG_Q3F_ParseSpawnVars: closing brace without data" );
		if ( numSpawnVars == MAX_SPAWN_VARS )
			CG_Error( "CG_Q3F_ParseSpawnVars: MAX_SPAWN_VARS" );

		spawnVars[ numSpawnVars ][0] = CG_Q3F_AddSpawnVarToken( keyname );
		spawnVars[ numSpawnVars ][1] = CG_Q3F_AddSpawnVarToken( token );
		numSpawnVars++;
	}

	return( qtrue );
}

static qboolean CG_Q3F_CheckSpawnGameIndex()
{
	// Check that the entity's gameindex string matches the
	// current gameindex in use.
	// Assumes strings of the format 1,2,3 etc.

	char *ptr;
	int index;

	if( !(ptr = CG_Q3F_GetEntValue( "gameindex" )) )
		return( qtrue );		// No gameindex means valid in all modes.

	while( *ptr )
	{
		//Skip any non digit numbers to prepare for a new atoi call
		if( *ptr < '0' || *ptr > '9') {
			ptr++;
		}

		index = atoi( ptr );
		if( !index ) // integer, DUH
			CG_Error( "CG_Q3F_CheckSpawnGameIndex: Invalid gameindex '%i'.", index );
		if( index == cgs.gameindex )
			return( qtrue );

		//Skip the digits used for the atoi call
		while ( *ptr >= '0' && *ptr <= '9') {
			ptr++;
		}
	}
	return( qfalse );
}

qboolean CG_LoadFlareScript( const char *filename );

void CG_Q3F_ParseEntities()
{
	// The main entry point, performs all the parse functions.
	// We will soon (hopefully) get access to a trap that gives direct
	// access to the data loaded by the engine, which will save a little time.

	char *classname;
	int index;

	cgs.numLocations = 0;
	cgs.numEntityData = 0;
	cgs.numFlares = 0;
	cgs.numSunFlares = 0;
	cgs.skyportal.hasportal = qfalse;
	cgs.sunportal.exists = qfalse;

	spawnIndex = 0;
	while( CG_Q3F_ParseSpawnVars() )
	{
		if( ( classname = CG_Q3F_GetEntValue( "classname" ) ) != NULL )
		{
			for( index = 0; index < (int)NUMPROCESSORS; index++ )
			{
				if( !Q_stricmp( processors[index].classname, classname ) )
				{
					if( CG_Q3F_CheckSpawnGameIndex() )
					{
						processors[index].processor();
						break;
					}
				}
			}
		}
		spawnIndex++;
	}

	CG_LoadFlareScript( va( "maps/flares/%s.flr", cgs.rawmapname ) );

	if(!cgs.sunportal.exists)
	{
		if(!Q_stricmp(cgs.rawmapname, "etf_stag"))
		{
			VectorSet(cgs.sunportal.origin, 3680, -416, 1392);
			cgs.sunportal.exists = qtrue;
		}

		if(!Q_stricmp(cgs.rawmapname, "etf_2stoned"))
		{
			VectorSet(cgs.sunportal.origin, -1493, 3231, -96);
			cgs.sunportal.exists = qtrue;
		}
	}

		// Perform final cleanup.
	CG_Q3F_LocationSort();

	CG_Printf( BOX_PRINT_MODE_CHAT, "...loaded %i locations, %i flares, %i sunflares\n", cgs.numLocations, cgs.numFlares, cgs.numSunFlares );

	if( cgs.skyportal.hasportal )
		CG_Printf( BOX_PRINT_MODE_CHAT, "...loaded a skyportal\n");

	if( cgs.sunportal.exists )
		CG_Printf( BOX_PRINT_MODE_CHAT, "...loaded a sunportal\n");
}
