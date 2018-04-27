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

#include "g_local.h"
#include "g_q3f_mapents.h"
#include "g_q3f_playerclass.h"
#include "g_q3f_flag.h"
#include "g_q3f_team.h"

#ifdef BUILD_LUA
#include "g_lua.h"  
#endif // BUILD_LUA

/*
qboolean	G_SpawnString( const char *key, const char *defaultString, char **out ) {
	int		i;

	if ( !level.spawning ) {
		*out = (char *)defaultString;
//		G_Error( "G_SpawnString() called while not spawning" );
	}

	for ( i = 0 ; i < level.numSpawnVars ; i++ ) {
		if ( !Q_stricmp( key, level.spawnVars[i][0] ) ) {
			*out = level.spawnVars[i][1];
			return qtrue;
		}
	}

	*out = (char *)defaultString;
	return qfalse;
}
*/
qboolean	G_SpawnFloat( const char *key, const char *defaultString, float *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atof( s );
	return present;
}

qboolean	G_SpawnInt( const char *key, const char *defaultString, int *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	*out = atoi( s );
	return present;
}

qboolean	G_SpawnBoolean( const char *key, const char *defaultString, qboolean *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString(key, defaultString, &s);

	if ( !Q_stricmp( s, "qfalse" ) || !Q_stricmp( s, "false" ) || !Q_stricmp( s, "no" ) || !Q_stricmp( s, "0" ) ) {
		*out = qfalse;
	} else if ( !Q_stricmp( s, "qtrue" ) || !Q_stricmp( s, "true" ) || !Q_stricmp( s, "yes" ) || !Q_stricmp( s, "1" ) ) {
		*out = qtrue;
	} else {
		*out = qfalse;
	}

	return present;
}

qboolean	G_SpawnVector( const char *key, const char *defaultString, float *out ) {
	char		*s;
	qboolean	present;

	present = G_SpawnString( key, defaultString, &s );
	sscanf( s, "%f %f %f", &out[0], &out[1], &out[2] );
	return present;
}

//Keeger for tracemap support
qboolean G_SpawnStringExt( const char *key, const char *defaultString, char **out, const char* file, int line ) {
	int		i;

	if ( !level.spawning ) {
		*out = (char *)defaultString;
		// Gordon: 26/11/02: re-enabling
		// see InitMover   
      //keeg:  commented out of version of q3f code we received, and is causing bugs with give commands...
	//	G_Error( "G_SpawnString() called while not spawning, file %s, line %i", file, line );
	}

	for ( i = 0 ; i < level.numSpawnVars ; i++ ) {
		if ( !strcmp( key, level.spawnVars[i][0] ) ) {
			*out = level.spawnVars[i][1];
			return qtrue;
		}
	}

	*out = (char *)defaultString;
	return qfalse;
}

qboolean	G_SpawnVector2DExt( const char *key, const char *defaultString, float *out, const char* file, int line ) {
	char		*s;
	qboolean	present;

	present = G_SpawnStringExt( key, defaultString, &s, file, line );
	sscanf( s, "%f %f", &out[0], &out[1] );
	return present;
}
//end keeg

qboolean	G_SpawnColor( const char *defaultString, float *out )
{
	// Golliwog: Convenience function for colour keys.

	return( G_SpawnVector( "_color", defaultString, out ) ||
			G_SpawnVector( "color", defaultString, out ) ||
			G_SpawnVector( "_colour", defaultString, out ) ||
			G_SpawnVector( "colour", defaultString, out ) );
}

// Golliwog: Allow mapinfo overrides
qboolean G_Q3F_SpawnStringOverride( const char *key, const char *defaultString, char **out )
{
	char keybuff[32];

	if( level.mapInfo )
	{
		Q_strncpyz( keybuff, key, sizeof(keybuff) );
		Q_strlwr( keybuff );
		if( (*out = G_Q3F_GetMapInfoEntry(	level.mapInfo, keybuff, g_gameindex.integer, NULL )) != NULL )
			return( qtrue );
	}
	return( G_SpawnString( key, defaultString, out ) );
}

qboolean	G_Q3F_SpawnFloatOverride( const char *key, const char *defaultString, float *out ) {
	char		*s;
	qboolean	present;

	present = G_Q3F_SpawnStringOverride( key, defaultString, &s );
	*out = atof( s );
	return present;
}

qboolean	G_Q3F_SpawnIntOverride( const char *key, const char *defaultString, int *out ) {
	char		*s;
	qboolean	present;

	present = G_Q3F_SpawnStringOverride( key, defaultString, &s );
	*out = atoi( s );
	return present;
}

qboolean	G_Q3F_SpawnBooleanOverride( const char *key, const char *defaultString, qboolean *out ) {
	char		*s;
	qboolean	present;

	present = G_Q3F_SpawnStringOverride( key, defaultString, &s );
	if ( !Q_stricmp( s, "qfalse" ) || !Q_stricmp( s, "false" ) || !Q_stricmp( s, "no" ) || !Q_stricmp( s, "0" ) ) {
		*out = qfalse;
	} else if ( !Q_stricmp( s, "qtrue" ) || !Q_stricmp( s, "true" ) || !Q_stricmp( s, "yes" ) || !Q_stricmp( s, "1" ) ) {
		*out = qtrue;
	} else {
		*out = qfalse;
	}
	return present;
}

qboolean	G_Q3F_SpawnVectorOverride( const char *key, const char *defaultString, float *out ) {
	char		*s;
	qboolean	present;

	present = G_Q3F_SpawnStringOverride( key, defaultString, &s );
	sscanf( s, "%f %f %f", &out[0], &out[1], &out[2] );
	return present;
}
// Golliwog.

field_t fields[] = {
	{"classname", FOFS(classname), F_LSTRING},
	{"origin", FOFS(s.origin), F_VECTOR},
	{"model", FOFS(model), F_LSTRING},
	{"model2", FOFS(model2), F_LSTRING},
	{"spawnflags", FOFS(spawnflags), F_INT},
	{"speed", FOFS(speed), F_FLOAT},
	{"target", FOFS(target), F_LSTRING},
	{"targetname", FOFS(targetname), F_LSTRING},
	{"message", FOFS(message), F_LSTRING},
	{"team", FOFS(team), F_LSTRING},
	{"wait", FOFS(wait), F_FLOAT},
	{"random", FOFS(random), F_FLOAT},
	{"count", FOFS(count), F_INT},
	{"health", FOFS(health), F_INT},
	{"light", 0, F_IGNORE},
	{"dmg", FOFS(damage), F_INT},
	{"angles", FOFS(s.angles), F_VECTOR},
	{"angle", FOFS(s.angles), F_ANGLEHACK},
	{"targetShaderName", FOFS(targetShaderName), F_LSTRING},
	{"targetShaderNewName", FOFS(targetShaderNewName), F_LSTRING},

// XreaL BEGIN
#ifdef _ETXREAL
	{"name", FOFS(targetname), F_LSTRING},
	{"_etxmap_autogeneratedName", FOFS(targetnameAutogenerated), F_INT},
	{"_etxmap_fixedDuplicatedName", FOFS(targetnameFixedDuplicated), F_INT},
#endif
// XreaL END

	{NULL}
};

const size_t numFields = ARRAY_LEN(fields);

typedef struct {
	const char	*name;
	void		(*spawn)(gentity_t *ent);
} spawn_t;

void SP_info_player_start (gentity_t *ent);
void SP_info_player_deathmatch (gentity_t *ent);
void SP_info_player_intermission (gentity_t *ent);
void SP_info_firstplace(gentity_t *ent);
void SP_info_secondplace(gentity_t *ent);
void SP_info_thirdplace(gentity_t *ent);
void SP_info_podium(gentity_t *ent);

void SP_func_plat (gentity_t *ent);
void SP_func_static (gentity_t *ent);
void SP_func_rotating (gentity_t *ent);
//void SP_func_child_rotating (gentity_t *ent); // RR2DO2
void SP_func_bobbing (gentity_t *ent);
void SP_func_pendulum( gentity_t *ent );
void SP_func_button (gentity_t *ent);
void SP_func_door (gentity_t *ent);
void SP_func_door_rotating (gentity_t *ent);
void SP_func_train (gentity_t *ent);
void SP_func_timer (gentity_t *self);

void SP_trigger_always (gentity_t *ent);
void SP_trigger_multiple (gentity_t *ent);
void SP_trigger_push (gentity_t *ent);
void SP_trigger_teleport (gentity_t *ent);
void SP_trigger_hurt (gentity_t *ent);

void SP_target_remove_powerups( gentity_t *ent );
void SP_target_give (gentity_t *ent);
void SP_target_delay (gentity_t *ent);
void SP_target_speaker (gentity_t *ent);
void SP_target_print (gentity_t *ent);
void SP_target_laser (gentity_t *self);
void SP_target_character (gentity_t *ent);
void SP_target_score( gentity_t *ent );
void SP_target_teleporter( gentity_t *ent );
void SP_target_relay (gentity_t *ent);
void SP_target_kill (gentity_t *ent);
void SP_target_position (gentity_t *ent);
void SP_target_location (gentity_t *ent);
void SP_target_push (gentity_t *ent);

void SP_light (gentity_t *self);
void SP_info_null (gentity_t *self);
void SP_info_notnull (gentity_t *self);
void SP_info_camp (gentity_t *self);
void SP_path_corner (gentity_t *self);
void SP_path_spline (gentity_t *self);

void SP_misc_teleporter_dest (gentity_t *self);
void SP_misc_model(gentity_t *ent);
void SP_misc_portal_camera(gentity_t *ent);
void SP_misc_portal_surface(gentity_t *ent);

void SP_Q3F_misc_flamethrower( gentity_t *ent );

void SP_shooter_rocket( gentity_t *ent );
void SP_shooter_plasma( gentity_t *ent );
void SP_shooter_grenade( gentity_t *ent );

void SP_team_CTF_redplayer( gentity_t *ent );
void SP_team_CTF_blueplayer( gentity_t *ent );

void SP_team_CTF_redspawn( gentity_t *ent );
void SP_team_CTF_bluespawn( gentity_t *ent );

//void SP_item_botroam( gentity_t *ent ) {};

void SP_Q3F_CTF_redflag( gentity_t *ent );
void SP_Q3F_CTF_blueflag( gentity_t *ent );
void SP_Q3F_team_CTF_redplayer( gentity_t *ent );
void SP_Q3F_team_CTF_blueplayer( gentity_t *ent );


spawn_t	spawns[] = {
	// info entities don't do anything at all, but provide positional
	// information for things controlled by other processes
	{"info_player_start", SP_info_player_start},
	{"info_player_deathmatch", SP_info_player_deathmatch},
	{"info_player_intermission", SP_info_player_intermission},
	{"info_player_targetspawn", SP_info_player_targetspawn},		// Golliwog: Mobile 'targeting' spawnpoint.
	{"info_null", SP_info_null},
	{"info_notnull", SP_info_notnull},		// use target_position instead
	{"info_camp", SP_info_camp},

	{"func_plat", SP_func_plat},
	{"func_button", SP_func_button},
	{"func_door", SP_func_door},
	{"func_static", SP_func_static},
	{"func_rotating", SP_func_rotating},
//	{"func_child_rotating", SP_func_child_rotating}, // RR2DO2: Func_rotating attached to another mover
	{"func_bobbing", SP_func_bobbing},
	{"func_pendulum", SP_func_pendulum},
	{"func_train", SP_func_train},
	{"func_group", SP_info_null},
	{"func_timer", SP_func_timer},			// rename trigger_timer?

	{"func_goalitem",	SP_Q3F_func_flag},	// Golliwog: Generic Q3F flag entity
	{"func_flag",		SP_Q3F_func_flag},	// Golliwog: Generic Q3F flag entity

	{"func_door_rotating", SP_func_door_rotating},	// Golliwog: Rotating doors.

	{"func_damage",			SP_Q3F_func_damage},		// Golliwog: Can be damaged to trigger things.
	{"func_goalinfo",		SP_Q3F_func_goalinfo},		// Golliwog: Generic Q3F trigger/nobrush
	{"func_commandpoint",	SP_Q3F_func_commandpoint},	// Golliwog: Command point entity
	{"func_hud",			SP_Q3F_func_hud},			// Golliwog: Generic Q3F HUD entity
	{"func_nobuild",		SP_Q3F_func_nobuild},		// Golliwog: Stop buildings being placed in the brush
	{"func_noannoyances",	SP_Q3F_func_noannoyances},	// Ensiform: Stop annoyances being placed in or entering the brush
	{"func_explosion",		SP_Q3F_func_explosion},		// Golliwog: Generate an explosion effect
	{"misc_beam",			SP_Q3F_misc_beam},			// RR2DO2: Generate an electrical beam
	{"func_wall",			SP_Q3F_func_wall},			// RR2DO2: Generates a visible or not visible wall
	{"misc_mapsentry",		SP_Q3F_misc_mapsentry},		// RR2DO2: Mapsentry
	{"func_forcefield",		SP_Q3F_func_forcefield},	// Golliwog: A criteria-matching forcefield.
	{"func_visibility",		SP_Q3F_func_visibility},	// Ensiform: A criteria-matching visibility nonsolid brush.
	{"misc_matchtimer",		SP_Q3F_misc_matchtimer},	// djbob: simple countdown, that can display on HUD, and activate triggers
	{"target_blackhole",	SP_Q3F_misc_blackhole},		// djbob: trigger to remove entities fromt he volume
	{"misc_stopwatch",		SP_Q3F_misc_stopwatch},		// canabis: Start counting and give the count in a message
	{"misc_changeclass",	SP_Q3F_misc_changeclass},	// slothy: brings up the change class screen
	
	// Triggers are brush objects that cause an effect when contacted
	// by a living player, usually involving firing targets.
	// While almost everything could be done with
	// a single trigger class and different targets, triggered effects
	// could not be client side predicted (push and teleport).
	{"trigger_always", SP_trigger_always},
	{"trigger_multiple", SP_trigger_multiple},
	{"trigger_push", SP_trigger_push},
	{"trigger_teleport", SP_trigger_teleport},
	{"trigger_hurt", SP_trigger_hurt},

	// targets perform no action by themselves, but must be triggered
	// by another entity
	{"target_give", SP_target_give},
	{"target_remove_powerups", SP_target_remove_powerups},
	{"target_delay", SP_target_delay},
	{"target_speaker", SP_target_speaker},
	{"target_print", SP_target_print},
	{"target_laser", SP_target_laser},
	{"target_score", SP_target_score},
	{"target_teleporter", SP_target_teleporter},
	{"target_relay", SP_target_relay},
	{"target_kill", SP_target_kill},
	{"target_position", SP_target_position},
	{"target_location", SP_target_location},
	{"target_push", SP_target_push},
	{"target_command",	SP_Q3F_target_command},		// Golliwog: Execute commands on server
	{"target_cycle",	SP_Q3F_target_cycle},		// Golliwog: Trigger entities 'by' the next client
	{"target_multiport",SP_Q3F_target_multiport},	// Golliwog: Teleport affected clients
	{"target_respawn",	SP_Q3F_target_respawn},		// Golliwog: Respawn affected clients.
	{"target_semitrigger",	SP_Q3F_target_semitrigger},		// Golliwog: Randomly trigger some entities.
	{"target_accumulator", SP_Q3F_target_accumulator},	// Golliwog: acts as a counter triggering other ents.
	{"target_reset",	SP_Q3F_target_reset},		// RR2DO2: Resetting entities by killing them and respawning them from the map

	{"light", SP_light},
	{"path_corner", SP_path_corner},
	{"path_spline", SP_path_spline},

	{"misc_teleporter_dest", SP_misc_teleporter_dest},
	{"misc_model", SP_misc_model},
	{"misc_portal_surface", SP_misc_portal_surface},
	{"misc_portal_camera", SP_misc_portal_camera},

	{"misc_onkill", SP_Q3F_misc_onkill},
	{"misc_onprotect", SP_Q3F_misc_onprotect},

	{"shooter_rocket", SP_shooter_rocket},
	{"shooter_grenade", SP_shooter_grenade},
	{"shooter_plasma", SP_shooter_plasma},

	{"team_CTF_redplayer", SP_Q3F_team_CTF_redplayer},//SP_team_CTF_redplayer},
	{"team_CTF_blueplayer", SP_Q3F_team_CTF_blueplayer},//SP_team_CTF_blueplayer},
	{"team_CTF_redspawn", SP_Q3F_team_CTF_redplayer},//SP_team_CTF_redspawn},
	{"team_CTF_bluespawn", SP_Q3F_team_CTF_blueplayer},//SP_team_CTF_bluespawn},
	{"team_CTF_redflag", SP_Q3F_CTF_redflag},
	{"team_CTF_blueflag", SP_Q3F_CTF_blueflag},

	//BirdDawg: CTF conversions
	{"ammo_bfg" , SP_Q3F_CTF_AmmoConversion },
	{"ammo_bullets" , SP_Q3F_CTF_AmmoConversion },
	{"ammo_cells" , SP_Q3F_CTF_AmmoConversion },
	{"ammo_grenades" , SP_Q3F_CTF_AmmoConversion },
	{"ammo_lightning" , SP_Q3F_CTF_AmmoConversion },
	{"ammo_nails" , SP_Q3F_CTF_AmmoConversion },
	{"ammo_rockets" , SP_Q3F_CTF_AmmoConversion },
	{"ammo_shells" , SP_Q3F_CTF_AmmoConversion },
	{"ammo_slugs" , SP_Q3F_CTF_AmmoConversion },

	{"weapon_bfg" , SP_Q3F_CTF_AmmoConversion },
	{"weapon_gauntlet" , SP_info_null },				//throw these out
	{"weapon_grapplinghook" , SP_info_null },
	{"weapon_grenadelauncher" , SP_Q3F_CTF_AmmoConversion },
	{"weapon_lightning" , SP_Q3F_CTF_AmmoConversion },
	{"weapon_machinegun" , SP_Q3F_CTF_AmmoConversion },
	{"weapon_plasmagun" , SP_Q3F_CTF_AmmoConversion },
	{"weapon_railgun" , SP_Q3F_CTF_AmmoConversion },
	{"weapon_rocketlauncher" , SP_Q3F_CTF_AmmoConversion },
	{"weapon_shotgun" , SP_Q3F_CTF_AmmoConversion },

	//{"item_botroam", SP_item_botroam},

	// WFA compat ents
	{"item_pack",				SP_Q3F_item_pack},
	{"item_flagreturn_team1",	SP_Q3F_item_flagreturn_team1},
	{"item_flagreturn_team2",	SP_Q3F_item_flagreturn_team2},

	// Panels
	{"panel_name",				SP_Q3F_Panel_Name},
	{"panel_scoresummary",		SP_Q3F_Panel_ScoreSummary},
	{"panel_location",			SP_Q3F_Panel_Location},
	{"panel_timer",				SP_Q3F_Panel_Timer},
	{"panel_radar",				SP_Q3F_Panel_Radar},
	{"panel_message",			SP_Q3F_Panel_Message},

	// Client sided ents
	{"misc_flare",				SP_info_null},
	{"misc_sunflare",			SP_info_null},
	{"misc_particlesystem",		SP_info_null},
	{"misc_parallax",			SP_info_null},
	{"misc_water",				SP_info_null},
	/* Ensiform - Eat both of these as well */
	{"misc_skyportal",			SP_info_null},
	{"misc_skyportal_surface",	SP_info_null},

	{"misc_sunportal",			SP_info_null},

	{"misc_flamethrower",		SP_Q3F_misc_flamethrower},

	{0, 0}
};

/*
============
G_ItemDisabled
============
*/
int G_ItemDisabled( gitem_t *item ) {

	char name[128];

	Com_sprintf(name, sizeof(name), "disable_%s", item->classname);
	return trap_Cvar_VariableIntegerValue( name );
}

/*
===============
G_CallSpawn

Finds the spawn function for the entity and calls it,
returning qfalse if not found
===============
*/
qboolean G_CallSpawn( gentity_t *ent ) {
	spawn_t	*s;
	gitem_t	*item;

	if ( !ent->classname ) {
		G_Printf ("G_CallSpawn: NULL classname\n");
		return qfalse;
	}

	// check normal spawn functions
	for ( s=spawns ; s->name ; s++ ) {
		if ( !strcmp(s->name, ent->classname) ) {
			// found it
			s->spawn(ent);
			return qtrue;
		}
	}

	// check item spawn functions

	// Golliwog: Don't spawn weapons
	if( !Q_strncmp( "weapon_", ent->classname, 7 ) )
		return( qfalse );

	for ( item=bg_itemlist+1 ; item->classname ; item++ ) {
		if ( !strcmp(item->classname, ent->classname) ) {
			if( G_ItemDisabled( item ) )
				return qfalse;
			G_SpawnItem( ent, item );
			return qtrue;
		}
	}

	G_Printf ("%s doesn't have a spawn function\n", ent->classname);
	return qfalse;
}

/*
=============
G_NewString

Builds a copy of the string, translating \n to real linefeeds
so message texts can be multi-line
=============
*/
char *G_NewString( const char *string ) {
	char	*newb, *new_p;
	int		i,l;
	
	l = strlen(string) + 1;

	newb = G_Alloc( l );

	new_p = newb;

	// turn \n into a real linefeed
	for ( i=0 ; i< l ; i++ ) {
		if (string[i] == '\\' && i < l-1) {
			i++;
			if (string[i] == 'n') {
				*new_p++ = '\n';
			} else {
				*new_p++ = '\\';
			}
		} else {
			*new_p++ = string[i];
		}
	}

	// Golliwog: This is a cruel and unusual thing to do to a defenceless pre-entity, but...
	G_Q3F_AddString( &new_p, newb );
	G_Free( newb );
	return( new_p );
	// Golliwog.
}




/*
===============
G_ParseField

Takes a key/value pair and sets the binary values
in a gentity
===============
*/
void G_ParseField( const char *key, const char *value, gentity_t *ent ) {
	field_t	*f;
	byte	*b;
	float	v;
	vec3_t	vec;

	for ( f=fields ; f->name ; f++ ) {
		if ( !Q_stricmp(f->name, key) ) {
			// found it
			b = (byte *)ent;

			switch( f->type ) {
			case F_LSTRING:
				*(char **)(b+f->ofs) = G_NewString (value);
				break;
			case F_VECTOR:
				sscanf (value, "%f %f %f", &vec[0], &vec[1], &vec[2]);
				((float *)(b+f->ofs))[0] = vec[0];
				((float *)(b+f->ofs))[1] = vec[1];
				((float *)(b+f->ofs))[2] = vec[2];
				break;
			case F_INT:
				*(int *)(b+f->ofs) = atoi(value);
				break;
			case F_FLOAT:
				*(float *)(b+f->ofs) = atof(value);
				break;
			case F_ANGLEHACK:
				v = atof(value);
				((float *)(b+f->ofs))[0] = 0;
				((float *)(b+f->ofs))[1] = v;
				((float *)(b+f->ofs))[2] = 0;
				break;
			default:
			case F_IGNORE:
				break;
			}
			return;
		}
	}
	// Golliwog: Custom map ent stuff - process the field properly
	G_Q3F_ProcessMapField( key, value, ent );
	// Golliwog.
}




/*
===================
G_SpawnGEntityFromSpawnVars

Spawn an entity and fill in all of the level fields from
level.spawnVars[], then call the class specfic spawn function
===================
*/
void G_SpawnGEntityFromSpawnVars( qboolean fromBSP, gentity_t *usethisent ) {
	int			i;
	qboolean	b;
	gentity_t	*ent;

	if( usethisent )
		ent = usethisent;
	else
		// get the next free entity
		ent = G_Spawn();

	if( fromBSP )
		ent->spawnIndex = level.spawnIndex;
	else
		ent->spawnIndex = -1;

	for ( i = 0 ; i < level.numSpawnVars ; i++ ) {
		G_ParseField( level.spawnVars[i][0], level.spawnVars[i][1], ent );
	}

	if ( g_gametype.integer == GT_FORTS ) {
		G_SpawnBoolean( "notq3f", "0", &b );
		if ( b ) {
			if ( !strcmp(ent->classname, "func_wall") || !strcmp(ent->classname, "func_damage") || (ent->s.eType == ET_MOVER) ) {
				trap_LinkEntity( ent );
				trap_AdjustAreaPortalState( ent, qtrue );
			}
			G_FreeEntity( ent );
			return;
		}
		G_SpawnBoolean( "notetf", "0", &b );
		if ( b ) {
			if ( !strcmp(ent->classname, "func_wall") || !strcmp(ent->classname, "func_damage") || (ent->s.eType == ET_MOVER) ) {
				trap_LinkEntity( ent );
				trap_AdjustAreaPortalState( ent, qtrue );
			}
			G_FreeEntity( ent );
			return;
		}
	}

	// RR2DO2 : check for gameindex
	if ( ent->mapdata && ent->mapdata->gameindex && !(ent->mapdata->gameindex & (1 << g_gameindex.integer)) ) {
		// Check for areaportals that we might need to disable to stop hom effects
		if ( !strcmp(ent->classname, "func_wall") || !strcmp(ent->classname, "func_damage") || (ent->s.eType == ET_MOVER) ) {
			trap_LinkEntity( ent );
			trap_AdjustAreaPortalState( ent, qtrue );
		}

		G_FreeEntity( ent );
		return;
	}
	// RR2DO2

	// move editor origin to pos
	VectorCopy( ent->s.origin, ent->s.pos.trBase );
	VectorCopy( ent->s.origin, ent->r.currentOrigin );

	// Golliwog: Sort our extended keys for further parsing
	if( ent->mapdata && ent->mapdata->other )
		G_Q3F_KeyPairArraySort( ent->mapdata->other);
	// Golliwog.

	// Golliwog: targetnamed entities must be triggerable, which means a mapdata struct
// XreaL BEGIN
#ifdef _ETXREAL
	if( ent->targetname && !ent->targetnameAutogenerated && !ent->mapdata )
#else
	if( ent->targetname && !ent->mapdata )
#endif
// XreaL END
	{
		ent->mapdata = G_Alloc( sizeof(q3f_mapent_t) );
		ent->mapdata->state = Q3F_STATE_INACTIVE;
		ent->mapdata->statechangetime = 0;
		ent->mapdata->origetype = -1;
	}

	// if we didn't get a classname, don't bother spawning anything
	if ( !G_CallSpawn( ent ) ) {
		G_FreeEntity( ent );
	}
	if( !ent->inuse )
		return;

	// Golliwog: Now it's created, lets clean up a little
	if( ent->mapdata )
	{
		G_Q3F_KeyPairArrayConsolidate( ent->mapdata->other );
		G_Q3F_KeyPairArraySort( ent->mapdata->other);
		ent->s.eFlags |= EF_Q3F_UNPREDICTABLE;		// Prevent client-side prediction
		switch( ent->mapdata->state )
		{
			case Q3F_STATE_INVISIBLE:	// Make this invisible already.
										ent->mapdata->origetype = ent->s.eType;
										ent->s.eType = ET_INVISIBLE;
										break;
			case Q3F_STATE_CARRIED:		// Can't have it carried at startup!
										ent->mapdata->state = Q3F_STATE_INACTIVE;
										break;
			case Q3F_STATE_ACTIVE:		// Put in a wait time
										ent->mapdata->waittime = level.time + ent->wait + Q_flrand(-1.0f, 1.0f) * ent->random;
										break;
		}
	}
	// Golliwog.

	G_Q3F_AddEntityToTargetArray( ent );	// Map all the target/groupnames

	// RR2DO2, if resettable we need to backup level.spawnVars
	if( ent->mapdata && ent->mapdata->flags & Q3F_FLAG_RESETABLE ) {
		ent->mapdata->spawVars = G_Q3F_KeyPairArrayCreate();

		for ( i = 0 ; i < level.numSpawnVars ; i++ ) {
			G_Q3F_KeyPairArrayAdd( ent->mapdata->spawVars, level.spawnVars[i][0], Q3F_TYPE_STRING, 0, (int)level.spawnVars[i][1] );
		}
	}
}

/*
====================
G_AddSpawnVarToken
====================
*/
char *G_AddSpawnVarToken( const char *string ) {
	int		l;
	char	*dest;

	l = strlen( string );
	if ( level.numSpawnVarChars + l + 1 > MAX_SPAWN_VARS_CHARS ) {
		G_Error( "G_AddSpawnVarToken: MAX_SPAWN_CHARS" );
	}

	dest = level.spawnVarChars + level.numSpawnVarChars;
	memcpy( dest, string, l+1 );

	level.numSpawnVarChars += l + 1;

	return dest;
}

/*
====================
G_ParseSpawnVars

Parses a brace bounded set of key / value pairs out of the
level's entity strings into level.spawnVars[]

This does not actually spawn an entity.
====================
*/
qboolean G_ParseSpawnVars( void ) {
	char		keyname[MAX_TOKEN_CHARS];
	char		com_token[MAX_TOKEN_CHARS];

	level.numSpawnVars = 0;
	level.numSpawnVarChars = 0;

	// parse the opening brace
	if ( !trap_GetEntityToken( com_token, sizeof( com_token ) ) ) {
		// end of spawn string
		return qfalse;
	}
	if ( com_token[0] != '{' ) {
		G_Error( "G_ParseSpawnVars: found %s when expecting {",com_token );
	}

	// go through all the key / value pairs
	while ( 1 ) {	
		// parse key
		if ( !trap_GetEntityToken( keyname, sizeof( keyname ) ) ) {
			G_Error( "G_ParseSpawnVars: EOF without closing brace" );
		}

		if ( keyname[0] == '}' ) {
			break;
		}
		
		// parse value	
		if ( !trap_GetEntityToken( com_token, sizeof( com_token ) ) ) {
			G_Error( "G_ParseSpawnVars: EOF without closing brace" );
		}

		if ( com_token[0] == '}' ) {
			G_Error( "G_ParseSpawnVars: closing brace without data" );
		}
		if ( level.numSpawnVars == MAX_SPAWN_VARS ) {
			G_Error( "G_ParseSpawnVars: MAX_SPAWN_VARS" );
		}
		level.spawnVars[ level.numSpawnVars ][0] = G_AddSpawnVarToken( keyname );
		level.spawnVars[ level.numSpawnVars ][1] = G_AddSpawnVarToken( com_token );
		level.numSpawnVars++;
	}

	return qtrue;
}

/*QUAKED worldspawn (0 0 0) ?

Every map should have exactly one worldspawn.
"music"		music wav file
"gravity"	800 is default gravity
"message"	Text to print during connection process
*/
void SP_worldspawn( void ) {
	char	*s;
	char	teamentvar[128], classentvar[128];
	char	tmpentvar[128];
	int		i,j,tmpint;
//	q3f_keypair_t *kp;

	G_SpawnString( "classname", "", &s );
	if ( Q_stricmp( s, "worldspawn" ) ) {
		G_Error( "SP_worldspawn: The first entity isn't 'worldspawn'" );
	}

	// make some data visible to connecting client
	trap_SetConfigstring( CS_GAME_VERSION, GAME_VERSION );
	trap_SetConfigstring( CS_FORTS_VERSION, FORTS_VERSION );	// Golliwog: Prevent version mismatches

	trap_SetConfigstring( CS_LEVEL_START_TIME, va("%i", level.startTime ) );

	G_Q3F_SpawnStringOverride( "music", "", &s );
	trap_SetConfigstring( CS_MUSIC, s );

	G_Q3F_SpawnStringOverride( "message", "", &s );
	trap_SetConfigstring( CS_MESSAGE, s );				// map specific message

	trap_SetConfigstring( CS_MOTD, g_motd.string );		// message of the day

	G_Q3F_SpawnStringOverride( "gravity", "800", &s );
	trap_Cvar_Set( "g_gravity", s );

	G_Q3F_SpawnStringOverride( "atmosphere", "", &s );
	trap_SetConfigstring( CS_FORTS_ATMOSEFFECT, s );		// Atmospheric effect

	G_Q3F_SpawnBooleanOverride( "nofallingdmg", "0", &level.nofallingdmg );
	G_Q3F_SpawnBooleanOverride( "noselfdmg", "0", &level.noselfdmg );

	for ( i = 0 ; i < level.numSpawnVars ; i++ ) {
		G_ParseField( level.spawnVars[i][0], level.spawnVars[i][1], &g_entities[ENTITYNUM_WORLD] );
	}

	// RR2DO2
	if( g_entities[ENTITYNUM_WORLD].mapdata )
		G_Q3F_KeyPairArraySort( g_entities[ENTITYNUM_WORLD].mapdata->other );
	// RR2DO2

	g_entities[ENTITYNUM_WORLD].s.number = ENTITYNUM_WORLD;
	g_entities[ENTITYNUM_WORLD].r.ownerNum = ENTITYNUM_NONE; // Ensiform: World is not owned by anything
	g_entities[ENTITYNUM_WORLD].classname = "worldspawn";

	// Ensiform: Initialize the ENTITYNUM_NONE index for conformity
	g_entities[ENTITYNUM_NONE].s.number = ENTITYNUM_NONE;
	g_entities[ENTITYNUM_NONE].r.ownerNum = ENTITYNUM_NONE;
	g_entities[ENTITYNUM_NONE].classname = "noclass";

	// see if we want a warmup time
	if ( g_restarted.integer ) {
		trap_Cvar_Set( "g_restarted", "0" );
	}

	switch ( g_matchState.integer ) {
	case MATCH_STATE_WAITING:
	case MATCH_STATE_WARMUP:
	case MATCH_STATE_READYUP:
		G_SetMatchState( MATCH_STATE_PLAYING );
		break;
	case MATCH_STATE_NORMAL:
		if (!(g_matchMode.integer & MATCH_MODE_ACTIVE)) 
			break;
	case MATCH_STATE_PREPARE:
		G_SetMatchState( MATCH_STATE_WAITING );
		break;
	}
	level.warmupTime = 0;
	trap_SetConfigstring( CS_WARMUP, va("%i", level.warmupTime) );

	// RR2DO2: parse q3f parameters
	G_Q3F_InitTeams();

	G_Printf("--- Reading ETF WorldSpawn Data ---\n");

	// first, loop through all classes
	for ( j = Q3F_CLASS_NULL+1; j < Q3F_CLASS_MAX; j++ )
	{
		qboolean tempchk;

		Q_strncpyz( classentvar, bg_q3f_classlist[j]->commandstring, sizeof(classentvar));

		// FIXME: removed "player", remove this bit in beta2!!!
		// <class>_playerlimit				Maximum of players of this class (overridden by team specific value)
		Q_strncpyz( tmpentvar, classentvar, sizeof(tmpentvar) );
		Q_strcat(tmpentvar,sizeof(tmpentvar),"_playerlimit");
		switch( j )
		{
			case Q3F_CLASS_RECON:			s = g_classReconLimit.string;			break;
			case Q3F_CLASS_SNIPER:			s = g_classSniperLimit.string;			break;
			case Q3F_CLASS_SOLDIER:			s = g_classSoldierLimit.string;			break;
			case Q3F_CLASS_GRENADIER:		s = g_classGrenadierLimit.string;		break;
			case Q3F_CLASS_PARAMEDIC:		s = g_classParamedicLimit.string;		break;
			case Q3F_CLASS_MINIGUNNER:		s = g_classMinigunnerLimit.string;		break;
			case Q3F_CLASS_FLAMETROOPER:	s = g_classFlametrooperLimit.string;	break;
			case Q3F_CLASS_AGENT:			s = g_classAgentLimit.string;			break;
			case Q3F_CLASS_ENGINEER:		s = g_classEngineerLimit.string;		break;
			case Q3F_CLASS_CIVILIAN:		s = "0";								break;
			default:						s = "-1";
		}
//		G_Printf("%s\n", s);
		tempchk = G_Q3F_SpawnIntOverride( tmpentvar, s, &tmpint );
		//G_Printf("%s : %i\n",tmpentvar,tmpint);
		for ( i = Q3F_TEAM_FREE+1; i < Q3F_TEAM_NUM_TEAMS-1; i++ )
		{
			if( tmpint > -1 )
				g_q3f_teamlist[i].classmaximums[j] = tmpint;
			else if( tmpint == -1 )
				g_q3f_teamlist[i].classmaximums[j] = 256;
		}

		if ( !tempchk ) {
		// <class>_limit				Maximum of players of this class (overridden by team specific value)
		Q_strncpyz( tmpentvar, classentvar, sizeof(tmpentvar) );
		Q_strcat(tmpentvar,sizeof(tmpentvar),"_limit");
		switch( j )
		{
			case Q3F_CLASS_RECON:			s = g_classReconLimit.string;			break;
			case Q3F_CLASS_SNIPER:			s = g_classSniperLimit.string;			break;
			case Q3F_CLASS_SOLDIER:			s = g_classSoldierLimit.string;			break;
			case Q3F_CLASS_GRENADIER:		s = g_classGrenadierLimit.string;		break;
			case Q3F_CLASS_PARAMEDIC:		s = g_classParamedicLimit.string;		break;
			case Q3F_CLASS_MINIGUNNER:		s = g_classMinigunnerLimit.string;		break;
			case Q3F_CLASS_FLAMETROOPER:	s = g_classFlametrooperLimit.string;	break;
			case Q3F_CLASS_AGENT:			s = g_classAgentLimit.string;			break;
			case Q3F_CLASS_ENGINEER:		s = g_classEngineerLimit.string;		break;
			case Q3F_CLASS_CIVILIAN:		s = "0";								break;
			default:						s = "-1";
		}
		G_Q3F_SpawnIntOverride( tmpentvar, s, &tmpint );
		//G_Printf("%s : %i\n",tmpentvar,tmpint);
		for ( i = Q3F_TEAM_FREE+1; i < Q3F_TEAM_NUM_TEAMS-1; i++ )
		{
			if( tmpint > -1 )
				g_q3f_teamlist[i].classmaximums[j]=tmpint;
			else if( tmpint == -1 )
				g_q3f_teamlist[i].classmaximums[j] = 256;
		}
		} // tmpchk

		// <class>_name					Name of this class (overridden by team specific value)
		Q_strncpyz( tmpentvar, classentvar, sizeof(tmpentvar) );
		Q_strcat(tmpentvar,sizeof(tmpentvar),"_name");
		G_Q3F_SpawnStringOverride( tmpentvar, bg_q3f_classlist[j]->title, &s );
		//G_Printf("%s : %s\n",tmpentvar,s);
		for ( i = Q3F_TEAM_FREE+1; i < Q3F_TEAM_NUM_TEAMS-1; i++ )
			G_Q3F_AddString( &g_q3f_teamlist[i].classnames[j], s );
	}

	// next, loop through all teams
	for ( i = Q3F_TEAM_FREE+1; i < Q3F_TEAM_NUM_TEAMS-1; i++ )
	{
//		Q_strncpyz( teamentvar, "team_", sizeof(teamentvar));
//		Q_strcat( teamentvar, sizeof(teamentvar), g_q3f_teamlist[i].name);
		Q_strncpyz( teamentvar, g_q3f_teamlist[i].name, sizeof(teamentvar) );

		// team_<colour>_name					Name of the team (default: "<colour> Team")
		Q_strncpyz( tmpentvar, teamentvar, sizeof(tmpentvar) );
		Q_strcat(tmpentvar,sizeof(tmpentvar),"_name");
		G_Q3F_SpawnStringOverride( tmpentvar, g_q3f_teamlist[i].description, &s );
		//G_Printf("%s : %s\n",tmpentvar,s);
		G_Q3F_AddString( &g_q3f_teamlist[i].description, s );

		// FIXME: removed "player", remove this bit in beta2!!!
		// team_<colour>_playerlimit			Maximum of players on this team (default: -1 (unlimited))
		Q_strncpyz( tmpentvar, teamentvar, sizeof(tmpentvar) );
		Q_strcat(tmpentvar,sizeof(tmpentvar),"_playerlimit");
		G_Q3F_SpawnIntOverride( tmpentvar, "-2", &g_q3f_teamlist[i].playerlimit );
		//G_Printf("%s : %i\n",tmpentvar,g_q3f_teamlist[i].playerlimit);

		// team_<colour>_limit			Maximum of players on this team (default: -1 (unlimited))
		Q_strncpyz( tmpentvar, teamentvar, sizeof(tmpentvar) );
		Q_strcat(tmpentvar,sizeof(tmpentvar),"_limit");
		G_Q3F_SpawnIntOverride( tmpentvar, "-2", &g_q3f_teamlist[i].playerlimit );
		//G_Printf("%s : %i\n",tmpentvar,g_q3f_teamlist[i].playerlimit);

		// team_<colour>_maxlives				Maximum lives per player on this team (default: -1 (unlimited))
		Q_strncpyz( tmpentvar, teamentvar, sizeof(tmpentvar) );
		Q_strcat(tmpentvar,sizeof(tmpentvar),"_maxlives");
		G_Q3F_SpawnIntOverride( tmpentvar, "-1", &g_q3f_teamlist[i].maxlives );
		//G_Printf("%s : %i\n",tmpentvar,g_q3f_teamlist[i].maxlives);

		// team_<colour>_allies					Allied teams (e.g. team_red_allies = "blue yellow")
		Q_strncpyz( tmpentvar, teamentvar, sizeof(tmpentvar) );
		Q_strcat(tmpentvar,sizeof(tmpentvar),"_allies");
		G_Q3F_SpawnStringOverride( tmpentvar, "", &s );

		if( s[0] ) {
			G_Q3F_SetTeamAllies( i, s );
		}

		// loop through all classes
		for ( j = Q3F_CLASS_NULL+1; j < Q3F_CLASS_MAX; j++ )
		{
			Q_strncpyz( classentvar, teamentvar, sizeof(classentvar) );
			Q_strcat( classentvar, sizeof(classentvar), "_");
			Q_strcat( classentvar, sizeof(classentvar), bg_q3f_classlist[j]->commandstring);

			// FIXME: removed "player", remove this bit in beta2!!!
			// team_<colour>_<class>_playerlimit	Maximum of players of this class on this team (default: 0 (unlimited))
			Q_strncpyz( tmpentvar, classentvar, sizeof(tmpentvar) );
			Q_strcat(tmpentvar,sizeof(tmpentvar),"_playerlimit");
			if ( j != Q3F_CLASS_CIVILIAN )
				G_Q3F_SpawnIntOverride( tmpentvar, va("%i",g_q3f_teamlist[i].classmaximums[j]), &tmpint );
			else
				G_Q3F_SpawnIntOverride( tmpentvar, va("%i",g_q3f_teamlist[i].classmaximums[j]), &tmpint );
			if( tmpint > -1 )
				g_q3f_teamlist[i].classmaximums[j] = tmpint;
			else if( tmpint == -1 )
				g_q3f_teamlist[i].classmaximums[j] = 256;
			//G_Printf("%s : %i\n",tmpentvar,g_q3f_teamlist[i].classmaximums[j]);

			// team_<colour>_<class>_limit	Maximum of players of this class on this team (default: 0 (unlimited))
			Q_strncpyz( tmpentvar, classentvar, sizeof(tmpentvar) );
			Q_strcat(tmpentvar,sizeof(tmpentvar),"_limit");
			if ( j != Q3F_CLASS_CIVILIAN )
				G_Q3F_SpawnIntOverride( tmpentvar, va("%i",g_q3f_teamlist[i].classmaximums[j]), &tmpint );
			else
				G_Q3F_SpawnIntOverride( tmpentvar, va("%i",g_q3f_teamlist[i].classmaximums[j]), &tmpint );
			if( tmpint > -1 )
				g_q3f_teamlist[i].classmaximums[j] = tmpint;
			else if( tmpint == -1 )
				g_q3f_teamlist[i].classmaximums[j] = 256;
			//G_Printf("%s : %i\n",tmpentvar,g_q3f_teamlist[i].classmaximums[j]);

			// team_<colour>_<class>_name			Name of this class on this team (default: <class>)
			Q_strncpyz( tmpentvar, classentvar, sizeof(tmpentvar) );
			Q_strcat(tmpentvar,sizeof(tmpentvar),"_name");
			G_Q3F_SpawnStringOverride( tmpentvar, bg_q3f_classlist[j]->title, &s );
			//G_Printf("%s : %s\n",tmpentvar,s);
			G_Q3F_RemString( &g_q3f_teamlist[i].classnames[j] );		// Remove old string first
			G_Q3F_AddString( &g_q3f_teamlist[i].classnames[j], s );
		}
	}

	trap_Cvar_Set( "g_etf_redteam",		g_q3f_teamlist[1].description );
	trap_Cvar_Set( "g_etf_blueteam",	g_q3f_teamlist[2].description );
	trap_Cvar_Set( "g_etf_yellowteam",	g_q3f_teamlist[3].description );
	trap_Cvar_Set( "g_etf_greenteam",	g_q3f_teamlist[4].description );

	// Let's free some space
	if (g_entities[ENTITYNUM_WORLD].mapdata && g_entities[ENTITYNUM_WORLD].mapdata->other)
	{
		G_Q3F_KeyPairArrayDestroy( g_entities[ENTITYNUM_WORLD].mapdata->other );
		g_entities[ENTITYNUM_WORLD].mapdata->other = NULL;
	}

	// RR2DO2
}

/*
==============
G_SpawnEntitiesFromString

Parses textual entity definitions out of an entstring and spawns gentities.
==============
*/
void G_SpawnEntitiesFromString( void ) {
	// allow calls to G_Spawn*()
	level.spawning = qtrue;
	level.numSpawnVars = 0;
	level.spawnIndex = 0;

	// the worldspawn is not an actual entity, but it still
	// has a "spawn" function to perform any global setup
	// needed by a level (setting configstrings or cvars, etc)
	if ( !G_ParseSpawnVars() ) {
		G_Error( "SpawnEntities: no entities" );
	}
	SP_worldspawn();

	// parse ents
	level.spawnIndex++;
	while( G_ParseSpawnVars() ) {
		G_SpawnGEntityFromSpawnVars( qtrue, NULL );
		level.spawnIndex++;
	}

#ifdef BUILD_LUA
	G_LuaHook_SpawnEntitiesFromString();
#endif // BUILD_LUA


	level.spawning = qfalse;			// any future calls to G_Spawn*() will be errors
}
