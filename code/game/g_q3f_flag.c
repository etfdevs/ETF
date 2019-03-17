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
**	g_q3f_flag.c
**
**	Q3F Flag handling functions
**
*/

#include "g_q3f_flag.h"
#include "g_q3f_mapents.h"
#include "g_q3f_team.h"

#include "g_bot_interface.h"

static char *q3f_flaginfokeys[Q3F_NUM_STATES][Q3F_TEAM_SPECTATOR];	// Keep the strings for a little extra speed
static char *q3f_teamflaginfokeys[Q3F_NUM_STATES];
static char *q3f_nonteamflaginfokeys[Q3F_NUM_STATES];

void G_Q3F_FlagInfo( gentity_t *queryent )
{
	// Player wants to know about status of assorted flags. So, we tell him.
	// This is actually scanning all ents with mapdata, which is possibly rather inefficient.

	gentity_t *ent;
	q3f_keypair_t *kp;
	char **keyptr;
	char fihud[MAX_STRING_CHARS];
	char *tempbuf;

	const size_t lengthOfCenter = 6;// strlen( "cp \"\"\n" );
	const size_t lengthOfPrint = 9;// strlen( "print \"\"\n" );

	fihud[0] = 0;

	/* Ensiform - This loop is the source of flaginfo bug me thinks */
	/* valid pointer was not checked per-se and <= is wrong!!! */
	/* Changed to add 'ent' check and <= now is < */
	/* APPARENTLY NOT FIXED! FIXME! */
	for( ent = level.gentities; ent < &level.gentities[MAX_GENTITIES]; ent++ )
	{
		if( ent && ent->inuse && ent->mapdata && (ent->mapdata->flags & Q3F_FLAG_FLAGINFO) )
		{
//#ifdef _DEBUG
			if( g_mapentDebug.integer )
				G_Printf(	"Flaginfo for %d: %s (%d %d %d).\n",
							ent->s.number, q3f_statestrings[ent->mapdata->state],
							(int) ent->r.currentOrigin[0], (int) ent->r.currentOrigin[1], (int) ent->r.currentOrigin[2] );
//#endif

				// Look for team-specific key
			if( queryent->client->sess.sessionTeam )
			{
				keyptr = &q3f_flaginfokeys[ent->mapdata->state][queryent->client->sess.sessionTeam];
				if( *keyptr != (char *) -1 )
				{
					if( !*keyptr )		// Need to 'get' the key first
						*keyptr = G_Q3F_GetString( va( "%s_%s_flaginfo", q3f_statestrings[ent->mapdata->state], g_q3f_teamlist[queryent->client->sess.sessionTeam].name ) );
					if( keyptr )
					{
						kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, *keyptr );
						if( kp )
						{
							tempbuf = G_Q3F_MessageString( kp->value.d.strdata, ent->activator, ent, 7 );
							trap_SendServerCommand(	queryent->s.number, va( "%s \"%s\n\"",
													(kp->value.flags & Q3F_VFLAG_FORCE) ? "cp" : "print",
													tempbuf) );
							Q_strcat(fihud, MAX_STRING_CHARS, va("%s\n", tempbuf));
							continue;
						}
					}
					else *keyptr = (char *) -1;
				}
			}

			if( ent->activator && ent->activator->inuse && ent->activator->client )
			{
				// Look for team and nonteam keys

				if( ent->activator->client->sess.sessionTeam == queryent->client->sess.sessionTeam )
				{
					keyptr = &q3f_teamflaginfokeys[ent->mapdata->state];
					if( !*keyptr )
						*keyptr = G_Q3F_GetString( va( "%s_team_flaginfo", q3f_statestrings[ent->mapdata->state] ) );
				}
				else {
					keyptr = &q3f_nonteamflaginfokeys[ent->mapdata->state];
					if( !*keyptr )
						*keyptr = G_Q3F_GetString( va( "%s_nonteam_flaginfo", q3f_statestrings[ent->mapdata->state] ) );
				}

				if( *keyptr != (char *) -1 )
				{
					if( keyptr )
					{
						kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, *keyptr );
						if( kp )
						{
							tempbuf = G_Q3F_MessageString( kp->value.d.strdata, ent->activator, ent, 7 );
							trap_SendServerCommand(	queryent->s.number, va( "%s \"%s\n\"",
													((kp->value.flags & Q3F_VFLAG_FORCE) ? "cp" : "print"),
													tempbuf) );
							Q_strcat(fihud, MAX_STRING_CHARS, va("%s\n", tempbuf));
							continue;
						}
					}
					else *keyptr = (char *) -1;
				}
			}

				// Look for general key
			keyptr = &q3f_flaginfokeys[ent->mapdata->state][0];
			if( *keyptr != (char *) -1 )
			{
				if( !*keyptr )		// Need to 'get' the key first
					*keyptr = G_Q3F_GetString( va( "%s_flaginfo", q3f_statestrings[ent->mapdata->state] ) );

				if( g_mapentDebug.integer )
					G_Printf(	"Flaginfo for %d: %s (%d %d %d) (%s).\n",
								ent->s.number, q3f_statestrings[ent->mapdata->state],
								(int) ent->r.currentOrigin[0], (int) ent->r.currentOrigin[1], (int) ent->r.currentOrigin[2], (*keyptr) ? *keyptr : "" );
				if( keyptr )
				{
					kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, *keyptr );
					if( kp )
					{
						tempbuf = G_Q3F_MessageString( kp->value.d.strdata, ent->activator, ent, 7 );
						trap_SendServerCommand(	queryent->s.number, va( "%s \"%s\n\"",
												((kp->value.flags & Q3F_VFLAG_FORCE) ? "cp" : "print"),
												tempbuf) );
						Q_strcat(fihud, MAX_STRING_CHARS, va("%s\n", tempbuf));

						continue;
					}
				}
				else *keyptr = (char *) -1;
			}
		}
	}

	trap_SendServerCommand(	queryent->s.number, va( "finfo \"%s\"", fihud) );
}

void SP_Q3F_func_flag( gentity_t *ent )
{
	// Spawn a func_flag

	vec3_t color;
	q3f_keypair_t *kp;
	qboolean lightSet, colorSet;
	float light;
	char *shaderStr;

	if( !ent->mapdata )
	{
		G_FreeEntity( ent );		// A flag with no ext. data is pretty useless
		return;
	}

	//if( !ent->wait )
	//	ent->wait = 30;		// Default wait of 30 seconds
	if( ent->wait )
		ent->wait *= 1000;		// Convert into milliseconds
	if( ent->wait < FRAMETIME )		// Kludge to allow 'instant return' goalitems.
		ent->wait = 0;

		// origin position
	VectorCopy( ent->s.origin, ent->pos1 );

		// default angles

	VectorCopy( ent->s.angles, ent->pos3 );

	ent->r.contents		= CONTENTS_TRIGGER;			// We want to know if it's touched
	ent->s.eType		= ET_Q3F_GOAL;				// And it's just, well, general ;)
	ent->physicsObject	= qfalse;
	ent->s.modelindex	= G_ModelIndex( ent->model );
	ent->s.otherEntityNum = MAX_CLIENTS;			// 'not held'.
	ent->use			= 0;
	ent->touch			= Q3F_func_flag_touch;
	ent->think			= Q3F_func_flag_think;
	ent->nextthink		= 0;

		// Find the simple shader to use.
	G_SpawnString( "simpleshader", "", &shaderStr );
	if( !shaderStr || !*shaderStr ||
		!(ent->s.torsoAnim = G_ShaderIndex( shaderStr )) )		// damn you Cana, switching legsAnim -> torsoAnim
	{
		// Use a default shader if appropriate.

		if( !Q_stricmp( ent->model,			"models/flags/r_flag.md3" ) )
			shaderStr = "textures/etf_hud/red_safe";
		else if( !Q_stricmp( ent->model,	"models/flags/b_flag.md3" ) )
			shaderStr = "textures/etf_hud/blue_safe";
		else if( !Q_stricmp( ent->model,	"models/flags/g_flag.md3" ) )
			shaderStr = "textures/etf_hud/green_safe";
		else if( !Q_stricmp( ent->model,	"models/flags/y_flag.md3" ) )
			shaderStr = "textures/etf_hud/yellow_safe";
		else if( !Q_stricmp( ent->model,	"models/flags/jap_flag_red.md3" ) )
			shaderStr = "textures/etf_hud/red_safe";
		else if( !Q_stricmp( ent->model,	"models/flags/jap_flag_blue.md3" ) )
			shaderStr = "textures/etf_hud/blue_safe";
		else if( !Q_stricmp( ent->model,	"models/flags/jap_flag_green.md3" ) )
			shaderStr = "textures/etf_hud/green_safe";
		else if( !Q_stricmp( ent->model,	"models/flags/jap_flag_yellow.md3" ) )
			shaderStr = "textures/etf_hud/yellow_safe";
		else if( !Q_stricmp( ent->model,	"models/mapobjects/keycard/keycard_red.md3" ) )
			shaderStr = "textures/etf_hud/key_safe_red";
		else if( !Q_stricmp( ent->model,	"models/mapobjects/keycard/keycard_blue.md3" ) )
			shaderStr = "textures/etf_hud/key_safe_blue";
		else if( !Q_stricmp( ent->model,	"models/objects/backpack/backpack.md3" ) )
			shaderStr = "icons/backpack";
		else if( !Q_stricmp( ent->model,	"models/objects/backpack/backpack_small.md3" ) )
			shaderStr = "icons/backpack";
		else if( !Q_stricmp( ent->model,	"models/powerups/instant/quad.md3" ) )
			shaderStr = "icons/quad";
		else if( !Q_stricmp( ent->model,	"models/powerups/instant/regen.md3" ) )
			shaderStr = "icons/regen";
		else if( !Q_stricmp( ent->model,	"models/powerups/instant/enviro.md3" ) )
			shaderStr = "icons/envirosuit";
		else if( !Q_stricmp( ent->model,	"models/powerups/instant/flight.md3" ) )
			shaderStr = "icons/flight";
		else if( !Q_stricmp( ent->model,	"models/powerups/instant/invis.md3" ) )
			shaderStr = "icons/invis";
		else if( !Q_stricmp( ent->model,	"models/powerups/instant/speed.md3" ) )
			shaderStr = "icons/haste";
		else if( !Q_stricmp( ent->model,	"models/powerups/armor/armor_red.md3" ) )
			shaderStr = "icons/iconr_red";
		else if( !Q_stricmp( ent->model,	"models/powerups/armor/armor_blue.md3" ) )
			shaderStr = "icons/iconr_blue";
		else if( !Q_stricmp( ent->model,	"models/powerups/armor/armor_green.md3" ) )
			shaderStr = "icons/iconr_green";
		else if( !Q_stricmp( ent->model,	"models/powerups/armor/armor_yellow.md3" ) )
			shaderStr = "icons/iconr_yellow";
		else if( !Q_stricmp( ent->model,	"models/powerups/armor/shard.md3" ) )
			shaderStr = "icons/iconr_shard";
		else shaderStr = NULL;

		if( shaderStr && *shaderStr)
			ent->s.torsoAnim = G_ShaderIndex( shaderStr );
	}

	if( ent->mapdata->flags & Q3F_FLAG_ROTATING )
		ent->s.eFlags |= EF_Q3F_ROTATING;

	ent->mapdata->flags |= Q3F_FLAG_CARRYABLE;
	G_SpawnVector( "mins", "-30 -30 -30", ent->r.mins );	// Fairly big items, these.
	G_SpawnVector( "maxs", "30 30 30", ent->r.maxs );
	VectorCopy( ent->r.mins, ent->movedir );
	VectorCopy( ent->r.maxs, ent->pos2 );

	// Light effect
	lightSet = G_SpawnFloat( "light", "100", &light );
	colorSet = G_SpawnColor( "1 1 1", color );
	if ( lightSet || colorSet ) {
		int		r, g, b, i;

		r = color[0] * 255;
		if ( r > 255 ) {
			r = 255;
		}
		g = color[1] * 255;
		if ( g > 255 ) {
			g = 255;
		}
		b = color[2] * 255;
		if ( b > 255 ) {
			b = 255;
		}
		i = light / 4;
		if ( i > 255 ) {
			i = 255;
		}
		ent->s.constantLight = r | ( g << 8 ) | ( b << 16 ) | ( i << 24 );
	}
		// Sparkle effect
	G_Q3F_KeyPairArraySort( ent->mapdata->other );		// Not sorted yet
	kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "sparkle" ) );
	if( kp )
	{
		sscanf( kp->value.d.strdata, "%f %f %f", &color[0], &color[1], &color[2] );
		VectorCopy( color, ent->s.origin2 );
	}

	kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "sparklescript" ) );
	if( kp )
		ent->s.legsAnim = G_SpiritScriptIndex( kp->value.d.strdata );
	else
		ent->s.legsAnim = G_SpiritScriptIndex( "spirit/goaltrail.spirit" );


	kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "sparkleshader" ) );
	if( kp )
		ent->s.modelindex2 = G_ShaderIndex( kp->value.d.strdata );
	else
		ent->s.modelindex2 = G_ShaderIndex( "hasteSmokePuff" );
	
	kp = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "speedscale" ) );

	if( kp )
		ent->speed = atof( kp->value.d.strdata );
	else ent->speed = -1;
	
	VectorCopy( ent->s.angles, ent->s.apos.trBase );
	ent->s.apos.trType = TR_STATIONARY;

	trap_LinkEntity( ent );
}

void Q3F_func_flag_touch( gentity_t *self, gentity_t *other, trace_t *trace )
{
	// We've been touched, go to carry state
	
	if( !other->inuse || !other->client )
	{
		G_Printf( "Flag touched by non-player.\n" );
		return;
	}
	if( self->activator && self->activator != other && self->mapdata->state == Q3F_STATE_CARRIED )
	{
		G_Printf(	"*** Flag touched by %s, but already carried by %s.\n",
					other->client->pers.netname, self->activator->client->pers.netname );
		return;
	}
#ifdef _DEBUG
	G_Printf( "Flag %d picked up by %s^7.\n", self->s.number, other->client->pers.netname );
#endif

	self->r.contents		= 0;			// Turn off the trigger
	self->r.ownerNum		= other->s.number;
	self->physicsObject		= qfalse;
	VectorClear( self->r.mins );
	VectorClear( self->r.maxs );

	if( self->activator != other && self->speed >= 0 )
	{
		if( !other->client->speedscale )
			other->client->speedscale = self->speed;
		else other->client->speedscale *= self->speed;		// Alter player speed
	}
	self->activator			= other;		// 'Holder'
	self->parent			= other;		// 'Last Holder'
	self->s.otherEntityNum	= other->s.number;
	self->think				= Q3F_func_flag_think;
	self->nextthink			= 0;			// Stop it happening unexpectedly.

	if( self->mapdata && (self->mapdata->flags & Q3F_FLAG_SHOWCARRY) )
		self->s.eFlags |= EF_Q3F_SHOWCARRY;

	if( self->mapdata )		// Set the owner team
	{
		self->mapdata->ownerteam = other->client->sess.sessionTeam;
		self->mapdata->waittime  = self->nextthink;
	}

//#ifdef DREVIL_BOT_SUPPORT
//	g_BotUserData.entity = (GameEntity)self;
//	Bot_Interface_SendGlobalEvent(MESSAGE_FLAG_PICKUP, other->s.number, 0, &g_BotUserData);
//#endif

	G_Q3F_UpdateHUDIcons();
}

void Q3F_func_flag_think( gentity_t *self )
{
	// Let's decide what to do next.

#ifdef _DEBUG
	G_Printf( "Flag %d thinks, state %s.\n", self->s.number, q3f_statestrings[self->mapdata->state] );
	G_Printf( "Current Position: %d %d %d\n", (int) self->r.currentOrigin[0], (int) self->r.currentOrigin[1] , (int) self->r.currentOrigin[2] );
	G_Printf( "Base Position: %d %d %d\n", (int) self->pos1[0], (int) self->pos1[1], (int) self->pos1[2] );
	G_Printf( "Base Angles: %d %d %d\n", (int) self->pos3[0], (int) self->pos3[1], (int) self->pos3[2] );
	G_Printf( "Now: %d Waittime: %d Nextthink %d\n", level.time, self->mapdata->waittime, self->nextthink );
#endif

	if( self->mapdata->state == Q3F_STATE_DISABLED || self->mapdata->state == Q3F_STATE_INVISIBLE )
	{
		// This never happens normally, it occurs when set to by another ent.
		if( self->activator )
			G_Q3F_DropFlag( self );
		self->nextthink = self->mapdata->waittime = 0;
	}
	else if( self->mapdata->state == Q3F_STATE_ACTIVE )
	{
		// We've been dropped - this may well be an error.
		if( !self->activator )
		{
			// If some pudding has set to active from disabled/invisible, we'll pretend it's "just been dropped"
			if( !self->mapdata->waittime )
				self->mapdata->waittime = level.time + self->wait + Q_flrand(-1.0f, 1.0f) * self->random;
			self->nextthink = self->mapdata->waittime;	// This is a kludge to fix a bug, possibly caused by frame hitches
		}
		else G_Q3F_DropFlag( self );
	}
	else if( self->mapdata->state == Q3F_STATE_INACTIVE )
	{
		// We want to return to base.
		gentity_t *holder = self->activator;
		if (holder) {
			VectorCopy( self->movedir, self->r.mins);
			VectorCopy( self->pos2, self->r.maxs);
		}
		G_SetOrigin( self, self->pos1 );
		self->r.contents		= CONTENTS_TRIGGER;
		if( self->s.time && !(self->mapdata->flags & Q3F_FLAG_NOSHRINK) )
		{
			self->s.time		= 0;			// Time to 'shrink out'
			self->s.time2		= level.time;	// Time to 'shrink in'
		}
		self->s.eFlags			&= ~EF_Q3F_SHOWCARRY;
		self->physicsObject		= qfalse;
		//VectorCopy( vec3_origin, self->s.angles ); // djbob: muon flag gets knackered without this, mebe make it a spawnflag?
		// Ensiform: We store the default angles now, rather than setting to 0
		VectorCopy( self->pos3, self->s.angles );
		VectorCopy( self->pos3, self->s.apos.trBase );
		self->s.apos.trType		= TR_STATIONARY;
		self->activator			= NULL;			// 'Holder'
		self->parent			= NULL;			// 'Last holder'
		self->s.otherEntityNum	= MAX_CLIENTS;	// 'Not held'
		self->s.groundEntityNum	= ENTITYNUM_NONE;
		trap_LinkEntity( self );

		if( self->mapdata )		// Reset the owner team
			self->mapdata->ownerteam = 0;

		if( holder != NULL && holder->client != NULL && self->speed >= 0 )
			holder->client->speedscale = G_Q3F_CalculateGoalItemSpeedScale( holder );		// Can't just /=, speedscale may be zero :)

		G_Q3F_UpdateHUDIcons();

//#ifdef DREVIL_BOT_SUPPORT
//		g_BotUserData.entity = (GameEntity)self;
//		Bot_Interface_SendGlobalEvent(MESSAGE_FLAG_RETURNED, 0, 0, &g_BotUserData);
//#endif
	}
}

float G_Q3F_CalculateGoalItemSpeedScale( gentity_t *player )
{
	// Check all the goalitems, work out the current speedscale for activator

	float scale;
	gentity_t *ent;

	for( scale = 1, ent = &g_entities[MAX_CLIENTS]; ent < &g_entities[level.num_entities]; ent++ )
	{
		if( ent->mapdata && ent->speed >= 0 && ent->mapdata->state == Q3F_STATE_CARRIED &&
			(ent->mapdata->flags & Q3F_FLAG_CARRYABLE) && ent->activator == player )
			scale *= ent->speed;
	}
	return( scale );
}

void G_Q3F_DropFlag( gentity_t *ent )
{
	// Drop the flag
	trace_t trace;
	vec3_t	velocity;
	vec3_t	angles;
	gentity_t *activator;
	q3f_keypair_t *data;
	float isClipped;

	/* Ensiform - Flag doesn't really bounce when you are up against walls facing south */
	/* It sticks inside the wall which appears to be due to the scaling up of bounding box code? */

	if( !ent->activator || !ent->mapdata )
		return;

	if( !ent->activator->client )
	{
		// Return immediately to base if no activator

		G_Q3F_TriggerEntity( ent, ent->activator, Q3F_STATE_INACTIVE, &trace, 1 );
		Q3F_func_flag_think( ent );
		G_Q3F_UpdateHUDIcons();
		return;
	}

	activator = ent->activator;
	G_SetOrigin( ent, activator->r.currentOrigin );
	isClipped = 0;
	VectorCopy( ent->movedir, ent->r.mins);
	VectorCopy( ent->pos2, ent->r.maxs);

	if(activator->health > 0 && activator->client->noclip)
	{
		bg_q3f_playerclass_t *cls;
		cls = BG_Q3F_GetClass(&(activator->client->ps));

		if( ent->r.mins[0] < cls->mins[0] )
			isClipped = ent->r.mins[0] = cls->mins[0];
		if( ent->r.mins[1] < cls->mins[1] )
			isClipped = ent->r.mins[1] = cls->mins[1];
		if( ent->r.mins[2] < cls->mins[2] )
			isClipped = ent->r.mins[2] = cls->mins[2];
		if( ent->r.maxs[0] > cls->maxs[0] )
			isClipped = ent->r.maxs[0] = cls->maxs[0];
		if( ent->r.maxs[1] > cls->maxs[1] )
			isClipped = ent->r.maxs[1] = cls->maxs[1];
		if( ent->r.maxs[2] > cls->maxs[2] )
			isClipped = ent->r.maxs[2] = cls->maxs[2];
	}
	else
	{
		if( ent->r.mins[0] < activator->r.mins[0] )
			isClipped = ent->r.mins[0] = activator->r.mins[0];
		if( ent->r.mins[1] < activator->r.mins[1] )
			isClipped = ent->r.mins[1] = activator->r.mins[1];
		if( ent->r.mins[2] < activator->r.mins[2] )
			isClipped = ent->r.mins[2] = activator->r.mins[2];
		if( ent->r.maxs[0] > activator->r.maxs[0] )
			isClipped = ent->r.maxs[0] = activator->r.maxs[0];
		if( ent->r.maxs[1] > activator->r.maxs[1] )
			isClipped = ent->r.maxs[1] = activator->r.maxs[1];
		if( ent->r.maxs[2] > activator->r.maxs[2] )
			isClipped = ent->r.maxs[2] = activator->r.maxs[2];
	}

	ent->soundLoop = isClipped ? 1 : 0;

	ent->nextthink			= level.time + ent->wait;
	ent->think				= Q3F_func_flag_think;
	ent->mapdata->waittime	= ent->nextthink;
	ent->timestamp			= level.time;			// Time dropped (to avoid 'retouch' issues)

	if( !(ent->mapdata->flags & Q3F_FLAG_NOSHRINK) )
	{
		ent->s.time				= ent->nextthink;		// Time to 'shrink out'
		ent->s.time2			= 0;					// Time to 'shrink in'
	}

	ent->r.contents		= CONTENTS_TRIGGER;
	ent->s.eFlags		&= ~EF_Q3F_SHOWCARRY;
	ent->physicsObject	= qtrue;		// Allow it to be stopped by brushes
	data = G_Q3F_KeyPairArrayFind( ent->mapdata->other, G_Q3F_GetString( "bouncefactor" ) );
	ent->physicsBounce	= data ? atof( data->value.d.strdata ) : 0.3;		// Bit of a bounce

	VectorCopy( ent->activator->s.apos.trBase, angles );
	angles[YAW] += Q_flrand(-1.0f, 1.0f) * 30;
	angles[PITCH] = 0;	// always forward
	if ( ent->activator && ent->activator->client && ent->activator->client->ps.stats[STAT_HEALTH]> 0 ) {
		AngleVectors( ent->activator->client->ps.viewangles, velocity, NULL, NULL );
		VectorScale( velocity, 280, velocity );
		VectorMA( velocity, 0.25, ent->activator->client->ps.velocity, velocity );
		velocity[2] += 150 + Q_flrand(-1.0f, 1.0f) * 50;
	} else {
		AngleVectors( angles, velocity, NULL, NULL );
		VectorScale( velocity, 200, velocity );
		velocity[2] += 200 + Q_flrand(-1.0f, 1.0f) * 50;
	}
	ent->s.pos.trType		= TR_GRAVITY;
	ent->s.pos.trTime		= level.time;
	ent->s.pos.trDuration	= 0;
	ent->s.pos.trBase[2]++;			// Stop it from embedding itself in the floor at the beginning.
	VectorCopy( velocity, ent->s.pos.trDelta );
	VectorCopy( angles, ent->s.angles );

	activator = ent->activator;

	// In case it's not already taking place
	if( ent->mapdata->state == Q3F_STATE_CARRIED )
		G_Q3F_TriggerEntity( ent, activator, Q3F_STATE_ACTIVE, &trace, 0 );

	trap_LinkEntity( ent );
#ifdef _DEBUG
	G_Printf(	"Flag %d dropped by %s, returns in %d seconds.\n",
				ent->s.number, activator->client->pers.netname,
				(ent->nextthink - level.time) / 1000 );
#endif

	ent->activator = NULL;
	ent->mapdata->lastTriggerer = NULL;
	ent->s.otherEntityNum = MAX_CLIENTS;
	G_Q3F_UpdateHUDIcons();

	if( ent->speed >= 0 )
		activator->client->speedscale = G_Q3F_CalculateGoalItemSpeedScale( activator );		// Can't just /=, speedscale may be zero :)

	// throw animation
	if( !(activator->client->ps.extFlags & EXTF_ANI_THROWING) ) {
		activator->client->ps.extFlags |= EXTF_ANI_THROWING;
		activator->client->torsoanimEndTime = level.time + Q3F_THROW_ANIM_DURATION;
	}
}

void G_Q3F_DropAllFlags( gentity_t *player, qboolean ignorenodrop, qboolean ignorekeepondeath )
{
	// Drop all flags a player is carrying.
	
	qboolean ceasefirestate = level.ceaseFire;
	gentity_t *ent;
	trace_t trace;

	for( ent = g_entities; ent < &g_entities[level.num_entities]; ent++ )
	{
		if( !ent->mapdata || ent->mapdata->state != Q3F_STATE_CARRIED || ent->activator != player )
			continue;
		if( !ignorenodrop && (ent->mapdata->flags & Q3F_FLAG_NODROP) )
			continue;
		if( !ignorekeepondeath && (ent->mapdata->flags & Q3F_FLAG_KEEPONDEATH) )
			continue;

		level.ceaseFire = qfalse;
		G_Q3F_TriggerEntity( ent, player, Q3F_STATE_ACTIVE, &trace, qtrue );
		level.ceaseFire = ceasefirestate;
	}
}

qboolean G_Q3F_CheckHeld( gentity_t *holder, q3f_array_t *array )
{
	// Check that all entities are held by holder.

	int index, index2;
	q3f_keypair_t *targkp;
	q3f_array_t *targarray;
	q3f_data_t *data, *data2;
	gentity_t *ent;

	for( index = -1; (data = G_Q3F_ArrayTraverse( array, &index )) != NULL; )
	{
		// Locate each target map in the main kparray
		if( !(targkp = G_Q3F_KeyPairArrayFind( level.targetnameArray, data->d.strdata ) ) )
			return( qfalse );
		targarray = targkp->value.d.arraydata;
		// Check each ent in that hash
		for( index2 = -1, ent = NULL; data2 = G_Q3F_ArrayTraverse( targarray, &index2 ); )
		{
			ent = data2->d.entitydata;
			if( ent->mapdata &&
				ent->mapdata->state == Q3F_STATE_CARRIED &&
				ent->activator == holder )
				break;		// Found this one, go for the next entry
			ent = NULL;
		}
		if( !ent )
			return( qfalse );	// Didn't find this entity
	}
	return( qtrue );
}

qboolean G_Q3F_CheckNotHeld( gentity_t *holder, q3f_array_t *array )
{
	// Check that none of the specified entities are held by holder.

	int index, index2;
	q3f_keypair_t *targkp;
	q3f_array_t *targarray;
	q3f_data_t *data, *data2;
	gentity_t *ent;

	for( index = -1; (data = G_Q3F_ArrayTraverse( array, &index )) != NULL; )
	{
		// Locate each target map in the main kparray
		if( !(targkp = G_Q3F_KeyPairArrayFind( level.targetnameArray, data->d.strdata ) ) )
			return( qfalse );
		targarray = targkp->value.d.arraydata;
		// Check each ent in that hash
		for( index2 = -1, ent = NULL; data2 = G_Q3F_ArrayTraverse( targarray, &index2 ); )
		{
			ent = data2->d.entitydata;
			if( ent->mapdata &&
				ent->mapdata->state == Q3F_STATE_CARRIED &&
				ent->activator == holder )
				break;		// Found this one, go for the next entry
			ent = NULL;
		}
		if( !ent )
			return( qtrue );	// Didn't find any of this one
	}
	return( qfalse );		// We found all of these.
}


/*
**	For each goalitem held, 'use' it if it's useable.
*/

static qboolean flagusegotstrings;
static char *flagusetargetptr, *flagusegiveptr, *flaguseteamscoreptr;
static char *flagusemessagestrings[3][8];
//static char *flagusemessagestrings[3][4];
void G_Q3F_FlagUseHeld( gentity_t *player )
{
	gentity_t *scan;
	q3f_keypairarray_t *oldgive;
	char *ptr;
	q3f_keypair_t *kp;
	q3f_mapent_t *mapdata;
	qboolean EntsUsed;

	EntsUsed = qfalse;

	if( !flagusegotstrings )
	{
		flagusetargetptr	= G_Q3F_GetString( "usetarget" );
		flagusegiveptr		= G_Q3F_GetString( "usegive" );
		flaguseteamscoreptr	= G_Q3F_GetString( "useteamscore" );
		flagusegotstrings	= qtrue;
	}

	for( scan = &g_entities[MAX_CLIENTS]; scan < &g_entities[level.num_entities]; scan++ )
	{
		mapdata = scan->mapdata;

		if( !(mapdata && mapdata->state == Q3F_STATE_CARRIED &&
			scan->activator == player) )
			continue;

		G_Q3F_StateBroadcast( scan, player, player, "_message", &flagusemessagestrings[0], Q3F_BROADCAST_TEXT, "use" );
		G_Q3F_StateBroadcast( scan, player, player, "_sound", &flagusemessagestrings[1], Q3F_BROADCAST_SOUND, "use" );
		G_Q3F_StateBroadcast( scan, player, player, "_dict", &flagusemessagestrings[2], Q3F_BROADCAST_DICT, "use" );

		kp = G_Q3F_KeyPairArrayFind( scan->mapdata->other, flagusegiveptr );
		if( kp )
		{
			if( kp->value.type == Q3F_TYPE_STRING )
			{
				ptr = kp->value.d.strdata;
				kp->value.d.keypairarraydata = G_Q3F_ProcessGiveString( ptr );
				G_Q3F_RemString( &ptr );
				kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
			}
			oldgive = mapdata->give;
			mapdata->give = kp->value.d.keypairarraydata;
			G_Q3F_MapGive( player, scan );
			mapdata->give = oldgive;
			EntsUsed = qtrue;
		}

		kp = G_Q3F_KeyPairArrayFind( scan->mapdata->other, flagusetargetptr );
		if( kp )
		{
			if( kp->value.type == Q3F_TYPE_STRING )
			{
				ptr = kp->value.d.strdata;
				kp->value.d.keypairarraydata = G_Q3F_ProcessStateString( ptr );
				G_Q3F_RemString( &ptr );
				kp->value.type = Q3F_TYPE_KEYPAIRARRAY;
			}
			G_Q3F_PropogateTrigger( kp->value.d.keypairarraydata, player, NULL );
			EntsUsed = qtrue;
		}
	}

	if ( !EntsUsed )
		G_AddEvent( player, EV_ETF_USE_ITEM_FAILED, 0 );
}
