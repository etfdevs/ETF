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
#include "bg_q3f_playerclass.h"
#include "g_q3f_mapents.h"
#include "g_q3f_flag.h"

/*

  Items are any object that a player can touch to gain some effect.

  Pickup will return the number of seconds until they should respawn.

  all items should pop when dropped in lava or slime

  Respawnable items don't actually go away when picked up, they are
  just made invisible and untouchable.  This allows them to ride
  movers and respawn apropriately.
*/

#define	RESPAWN_ARMOR		10//25
#define	RESPAWN_HEALTH		10//35
#define	RESPAWN_AMMO		10//40
#define	RESPAWN_HOLDABLE	10//60
#define	RESPAWN_MEGAHEALTH	10//35//120
#if 0
#define	RESPAWN_ARMOR		25
#define	RESPAWN_HEALTH		35
#define	RESPAWN_AMMO		40
#define	RESPAWN_HOLDABLE	60
#define	RESPAWN_MEGAHEALTH	35//120
#endif
#define	RESPAWN_POWERUP		120


//======================================================================

int Pickup_Powerup( gentity_t *ent, gentity_t *other ) {
	int			quantity;
	int			i;
	gclient_t	*client;

	if ( !other->client->ps.powerups[ent->item->giTag] ) {
		// round timing to seconds to make multiple powerup timers
		// count in sync
		other->client->ps.powerups[ent->item->giTag] = 
			level.time - ( level.time % 1000 );
	}

	if ( ent->count ) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

	other->client->ps.powerups[ent->item->giTag] += quantity * 1000;

	// give any nearby players a "denied" anti-reward
	for ( i = 0 ; i < level.maxclients ; i++ ) {
		vec3_t		delta;
		float		len;
		vec3_t		forward;
		trace_t		tr;

		client = &level.clients[i];
		if ( client == other->client ) {
			continue;
		}
		if ( client->pers.connected == CON_DISCONNECTED ) {
			continue;
		}
		if ( client->ps.stats[STAT_HEALTH] <= 0 ) {
			continue;
		}

    // if same team in team game, no sound
    // cannot use OnSameTeam as it expects to g_entities, not clients
  	if ( /*g_gametype.integer >= GT_TEAM && */other->client->sess.sessionTeam == client->sess.sessionTeam  ) {
      continue;
    }

		// if too far away, no sound
		VectorSubtract( ent->s.pos.trBase, client->ps.origin, delta );
		len = VectorNormalize( delta );
		if ( len > 192 ) {
			continue;
		}

		// if not facing, no sound
		AngleVectors( client->ps.viewangles, forward, NULL, NULL );
		if ( DotProduct( delta, forward ) < 0.4 ) {
			continue;
		}

		// if not line of sight, no sound
		trap_Trace( &tr, client->ps.origin, NULL, NULL, ent->s.pos.trBase, ENTITYNUM_NONE, CONTENTS_SOLID );
		if ( tr.fraction != 1.0 ) {
			continue;
		}

		// anti-reward
		client->ps.persistant[PERS_PLAYEREVENTS] ^= PLAYEREVENT_DENIEDREWARD;
	}

	return ent->item->giTag == PW_PENTAGRAM ? 500 : RESPAWN_POWERUP;
}

//======================================================================

int Pickup_Holdable( gentity_t *ent, gentity_t *other ) {

	other->client->ps.stats[STAT_HOLDABLE_ITEM] = ent->item - bg_itemlist;

	/*if( ent->item->giTag == HI_KAMIKAZE ) {
		other->client->ps.eFlags |= EF_KAMIKAZE;
	}*/

	return RESPAWN_HOLDABLE;
}

//======================================================================

int G_Q3F_Pickup_Backpack( gentity_t *ent, gentity_t *other )
{
	// Pick up the item and it's contents, then print a message for
	// the picker.

	char ammobuff[4][128];
	char messagebuff[1024];
	bg_q3f_playerclass_t *cls;
	int buffcount, oldammo, addammo, maxammo, ammoindex, index;
	char *ammoname;

	memset( ammobuff, 0, sizeof(ammobuff) );
	memset( messagebuff, 0, sizeof(messagebuff) );
	cls = BG_Q3F_GetClass( &other->client->ps );
	buffcount = -1;

		// Give the player all the ammo in this backpack
	for( index = 0; index < 4; index++ )
	{
		switch( index )
		{
			case 0:	addammo		= ent->s.time2;
					ammoindex	= AMMO_SHELLS;
					ammoname	= "shell";
					maxammo		= cls->maxammo_shells;
					break;
			case 1:	addammo		= ent->s.legsAnim;
					ammoindex	= AMMO_NAILS;
					ammoname	= "nail";
					maxammo		= cls->maxammo_nails;
					break;
			case 2:	addammo		= ent->s.torsoAnim;
					ammoindex	= AMMO_ROCKETS;
					ammoname	= "rocket";
					maxammo		= cls->maxammo_rockets;
					break;
			case 3:	addammo		= ent->s.weapon;
					ammoindex	= AMMO_CELLS;
					ammoname	= "cell";
					maxammo		= cls->maxammo_cells;
					break;
		}

		if( addammo && other->client->ps.ammo[ammoindex] < maxammo )
		{
			oldammo = other->client->ps.ammo[ammoindex];
			other->client->ps.ammo[ammoindex] += addammo;
			if( other->client->ps.ammo[ammoindex] > maxammo )
				other->client->ps.ammo[ammoindex] = maxammo;
			buffcount++;
			oldammo = other->client->ps.ammo[ammoindex] - oldammo;
			Q_strncpyz(	ammobuff[index], va( "%d %s%s", oldammo, ammoname, ((oldammo == 1) ? "" : "s")), 128 );
		}
	}

		// And print the message
	Q_strncpyz( messagebuff, "print \"^7You picked up ", sizeof(messagebuff) );
	if( buffcount < 0 )
		Q_strcat( messagebuff, sizeof(messagebuff), "nothing" );
	else {
		for( index = 0; index < 4; index++ )
		{
			if( !*ammobuff[index] )
				continue;
			Q_strcat( messagebuff, sizeof(messagebuff), ammobuff[index] );
			if( buffcount > 1 )
				Q_strcat( messagebuff, sizeof(messagebuff), ", " );
			else if( buffcount == 1 )
				Q_strcat( messagebuff, sizeof(messagebuff), " and " );
			buffcount--;
		}
	}
	switch( (int) ent->s.angles2[0] )
	{
		case 0:		ammoname = "pack";				break;
		case 1:		ammoname = "discarded pack";	break;
		case 2:		ammoname = "ammo box";			break;
		default:	ammoname = "supply";			break;
	}
	Q_strcat(	messagebuff, sizeof(messagebuff), va( " from %s^7's %s.\n\"",
				((ent->activator && ent->activator->client) ? ent->activator->client->pers.netname : "someone"),
				ammoname ) );
	trap_SendServerCommand( other->s.number, messagebuff );

	// djbob: disabled for b24 as per ghetto's request
/*	if( other->client->diseaseEnt && ent->activator == other->client->diseaseEnt && ent->s.angles2[0] < 2 && ent->activator->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_PARAMEDIC )
	{
		other->client->diseaseTime = 0;		// Remove the disease.
		other->client->diseaseEnt = 0;
		trap_SendServerCommand( other->s.number, "print \"You found the cure for your disease!\n\"" );
	}*/


	// RR2DO2: make sure ammobox count decreased again
	if ( ent->item->giType == IT_Q3F_AMMOBOX ) {
		if( g_entities[ent->r.ownerNum].client ) {
			for ( index = 0; index < Q3F_MAX_AMMOBOXES; index++ ) {
				if ( ent == g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[index] )
					break;
			}
			for ( buffcount = index; buffcount < Q3F_MAX_AMMOBOXES - 1; buffcount++ ) {
				g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[buffcount] = g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[buffcount+1];
			}
			buffcount = Q3F_MAX_AMMOBOXES - 1;
			g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[buffcount] = NULL;
		}
	}
	// RR2DO2

	return( RESPAWN_AMMO );
}


//======================================================================

void Add_Ammo (gentity_t *ent, int weapon, int count)
{
	ent->client->ps.ammo[weapon] += count;
	if ( ent->client->ps.ammo[weapon] > 200 ) {
		ent->client->ps.ammo[weapon] = 200;
	}
}

int Pickup_Ammo (gentity_t *ent, gentity_t *other)
{
	int		quantity;

	if ( ent->count ) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

	Add_Ammo (other, ent->item->giTag, quantity);

	return RESPAWN_AMMO;
}

//======================================================================


int Pickup_Weapon (gentity_t *ent, gentity_t *other) {
	//int		quantity;

	return 0;			// JT - None of that.
	//RR2DO2: LLC friendly comments ;)
	/*if ( ent->count < 0 ) {
		quantity = 0; // None for you, sir!
	} else {
		if ( ent->count ) {
			quantity = ent->count;
		} else {
			quantity = ent->item->quantity;
		}

		// dropped items and teamplay weapons always have full ammo
		if ( ! (ent->flags & FL_DROPPED_ITEM) && g_gametype.integer != GT_TEAM ) {
			// respawning rules
			// drop the quantity if the already have over the minimum
			if ( other->client->ps.ammo[ ent->item->giTag ] < quantity ) {
				quantity = quantity - other->client->ps.ammo[ ent->item->giTag ];
			} else {
				quantity = 1;		// only add a single shot
			}
		}
	}

	// add the weapon
	other->client->ps.stats[STAT_WEAPONS] |= ( 1 << ent->item->giTag );

	Add_Ammo( other, ent->item->giTag, quantity );*/

/*	if (ent->item->giTag == WP_GRAPPLING_HOOK)
		other->client->ps.ammo[ent->item->giTag] = -1; // unlimited ammo
*/
	// team deathmatch has slow weapon respawns
	// RR2DO2: still LCC friendly comments
	/*if ( g_gametype.integer == GT_TEAM ) {
		return g_weaponTeamRespawn.integer;
	}

	return g_weaponRespawn.integer;*/
}


//======================================================================

int Pickup_Health (gentity_t *ent, gentity_t *other) {
	int			max;
	int			quantity;
	bg_q3f_playerclass_t *cls;

	cls = BG_Q3F_GetClass( &other->client->ps );
	// small and mega healths will go over the max
	// Golliwog: Use class maxhealth
	if ( ent->item->quantity != 5 && ent->item->quantity != 100 ) {
		max = cls->maxhealth;
	} else {
		max = cls->maxhealth * 2;
	}
	// Golliwog.

	if ( ent->count ) {
		quantity = ent->count;
	} else {
		quantity = ent->item->quantity;
	}

	other->health += quantity;

	if (other->health > max ) {
		other->health = max;
	}
	other->client->ps.stats[STAT_HEALTH] = other->health;

	if ( ent->item->quantity == 100 ) {		// mega health respawns slow
		return RESPAWN_MEGAHEALTH;
	}

	return RESPAWN_HEALTH;
}

//======================================================================

int Pickup_Armor( gentity_t *ent, gentity_t *other ) {
	bg_q3f_playerclass_t *cls;

	cls = BG_Q3F_GetClass( &other->client->ps );
	other->client->ps.stats[STAT_ARMOR] += ent->item->quantity;
	// Golliwog: Use class max armour
//	if ( other->client->ps.stats[STAT_ARMOR] > other->client->ps.stats[STAT_MAX_HEALTH] * 2 ) {
//		other->client->ps.stats[STAT_ARMOR] = other->client->ps.stats[STAT_MAX_HEALTH] * 2;
//	}
	if ( other->client->ps.stats[STAT_ARMOR] > cls->maxarmour )
		other->client->ps.stats[STAT_ARMOR] = cls->maxarmour;
	// Golliwog.

	return RESPAWN_ARMOR;
}

//======================================================================

/*
===============
RespawnItem
===============
*/
void RespawnItem( gentity_t *ent ) {
	// randomly select from teamed entities
	if (ent->team) {
		gentity_t	*master;
		int	count;
		int choice;

		if ( !ent->teammaster ) {
			G_Error( "RespawnItem: bad teammaster");
		}
		master = ent->teammaster;

		for (count = 0, ent = master; ent; ent = ent->teamchain, count++)
			;

		choice = rand() % count;

		for (count = 0, ent = master; count < choice; ent = ent->teamchain, count++)
			;
	}

	ent->r.contents = CONTENTS_TRIGGER;
	ent->s.eFlags &= ~EF_NODRAW;
	ent->r.svFlags &= ~SVF_NOCLIENT;
	trap_LinkEntity (ent);

	if ( ent->item->giType == IT_POWERUP ) {
		// play powerup spawn sound to all clients
		gentity_t	*te;

		// if the powerup respawn sound should Not be global
		if (ent->speed) {
			te = G_TempEntity( ent->s.pos.trBase, EV_GENERAL_SOUND );
		}
		else {
			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_SOUND );
		}
		te->s.eventParm = G_SoundIndex( "sound/items/poweruprespawn.wav" );
		te->r.svFlags |= SVF_BROADCAST;
	}

	/*if ( ent->item->giType == IT_HOLDABLE && ent->item->giTag == HI_KAMIKAZE ) {
		// play powerup spawn sound to all clients
		gentity_t	*te;

		// if the powerup respawn sound should Not be global
		if (ent->speed) {
			te = G_TempEntity( ent->s.pos.trBase, EV_GENERAL_SOUND );
		}
		else {
			te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_SOUND );
		}
		te->s.eventParm = G_SoundIndex( "sound/items/kamikazerespawn.wav" );
		te->r.svFlags |= SVF_BROADCAST;
	}*/

	// play the normal respawn sound only to nearby clients
	G_AddEvent( ent, EV_ITEM_RESPAWN, 0 );

	ent->nextthink = 0;
}


/*
===============
Touch_Item
===============
*/
void Touch_Item (gentity_t *ent, gentity_t *other, trace_t *trace) {
	int			respawn;
	qboolean	predict;

	if( other )		// Golliwog: NULL other is used for explosions
	{
		if (!other->client)
			return;
		if (other->health < 1)
			return;		// dead people can't pickup
		if( level.ceaseFire )
			return;		// Golliwog: Can't pick stuff up during ceasefires.

		// the same pickup rules are used for client side and server side
		if ( !BG_CanItemBeGrabbed( g_gametype.integer, &ent->s, &other->client->ps, level.time ) ) {
			return;
		}

		G_LogPrintf( "Item: %i %s\n", other->s.number, ent->item->classname );

		predict = other->client->pers.predictItemPickup;

		// call the item-specific pickup function
		switch( ent->item->giType ) {
		case IT_WEAPON:
			respawn = Pickup_Weapon(ent, other);
			predict = qfalse;
			break;
		case IT_AMMO:
			respawn = Pickup_Ammo(ent, other);
			predict = qfalse;
			break;
		case IT_ARMOR:
			respawn = Pickup_Armor(ent, other);
			break;
		case IT_HEALTH:
			respawn = Pickup_Health(ent, other);
			break;
		case IT_POWERUP:
			respawn = Pickup_Powerup(ent, other);
			predict = qfalse;
			break;
/*		case IT_TEAM:
			respawn = Pickup_Team(ent, other);
			break;*/
		case IT_HOLDABLE:
			respawn = Pickup_Holdable(ent, other);
			break;
		case IT_Q3F_BACKPACK:
			respawn = G_Q3F_Pickup_Backpack( ent, other );
			break;
		case IT_Q3F_AMMOBOX:
			respawn = G_Q3F_Pickup_Backpack( ent, other );
			break;
		default:
			return;
		}

		if ( !respawn ) {
			return;
		}

		// play the normal pickup sound
		if (predict) {
			G_AddPredictableEvent( other, EV_ITEM_PICKUP, ent->s.modelindex );
		} else {
			G_AddEvent( other, EV_ITEM_PICKUP, ent->s.modelindex );
		}

		// powerup pickups are global broadcasts
		if ( ent->item->giType == IT_POWERUP /*|| ent->item->giType == IT_TEAM*/) {
			// if we want the global sound to play
			if (!ent->speed) {
				gentity_t	*te;

				te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_ITEM_PICKUP );
				te->s.eventParm = ent->s.modelindex;
				te->r.svFlags |= SVF_BROADCAST;
			} else {
				gentity_t	*te;

				te = G_TempEntity( ent->s.pos.trBase, EV_GLOBAL_ITEM_PICKUP );
				te->s.eventParm = ent->s.modelindex;
				// only send this temp entity to a single client
				te->r.svFlags |= SVF_SINGLECLIENT;
				te->r.singleClient = other->s.number;
			}
		}

		// fire item targets
		G_UseTargets (ent, other);
	} else {
		if ( ent->item->giType == IT_Q3F_AMMOBOX ) {
			int index, buffcount;

			if( g_entities[ent->r.ownerNum].client ) {
				for ( index = 0; index < Q3F_MAX_AMMOBOXES; index++ ) {
					if ( ent == g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[index] )
						break;
				}
				for ( buffcount = index; buffcount < Q3F_MAX_AMMOBOXES - 1; buffcount++ ) {
					g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[buffcount] = g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[buffcount+1];
				}
				buffcount = Q3F_MAX_AMMOBOXES - 1;
				g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[buffcount] = NULL;
			}
		}
	}

	// wait of -1 will not respawn
	if ( ent->wait == -1 ) {
		ent->r.svFlags |= SVF_NOCLIENT;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		ent->unlinkAfterEvent = qtrue;
		return;
	}

	// This forces wait of 10 on all items, rather than defaults :? why :CCC
#if 0
	// non zero wait overrides respawn time
	respawn = ent->wait ? ent->wait : 10;
#endif
	// non zero wait overrides respawn time
	if ( ent->wait ) {
		respawn = ent->wait;
	}

	// random can be used to vary the respawn time
	if ( ent->random ) {
		respawn += Q_flrand(-1.0f, 1.0f) * ent->random;
		if ( respawn < 1 ) {
			respawn = 1;
		}
	}

	// dropped items will not respawn
	if ( ent->flags & FL_DROPPED_ITEM ) {
		ent->freeAfterEvent = qtrue;
	}

	// picked up items still stay around, they just don't
	// draw anything.  This allows respawnable items
	// to be placed on movers.
	ent->r.svFlags |= SVF_NOCLIENT;
	ent->s.eFlags |= EF_NODRAW;
	ent->r.contents = 0;

	// ZOID
	// A negative respawn times means to never respawn this item (but don't 
	// delete it).  This is used by items that are respawned by third party 
	// events such as ctf flags
	if ( respawn <= 0 ) {
		ent->nextthink = 0;
		ent->think = 0;
	} else {
		ent->nextthink = level.time + respawn * 1000;
		ent->think = RespawnItem;
	}
	trap_LinkEntity( ent );
}


//======================================================================

// RR2DO2
void G_Q3F_FreeAmmoBox( gentity_t *ent ) {
	int index, buffcount;

	if( g_entities[ent->r.ownerNum].client ) {
		for ( index = 0; index < Q3F_MAX_AMMOBOXES; index++ ) {
			if ( ent == g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[index] )
				break;
		}

		if(index == Q3F_MAX_AMMOBOXES)			// note: if ammobox is picked up, it's nulled in G_Q3F_Pickup_Backpack and freed in G_FreeEntity
			return;

		if(g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[index]) {
			g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[index]->free = NULL;
			G_FreeEntity(g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[index]);
			g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[index] = NULL;	
		}

		for ( buffcount = index; buffcount < Q3F_MAX_AMMOBOXES - 1; buffcount++ ) {
			if(g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[buffcount + 1] == NULL)
				break;
			g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[buffcount] = g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[buffcount+1];
		}
//		--buffcount;	// = Q3F_MAX_AMMOBOXES - 1;
		g_entities[ent->r.ownerNum].client->pers.AmmoBoxes[buffcount] = NULL;	

	}
}
// RR2DO2

/*
================
LaunchItem

Spawns an item and tosses it forward
================
*/
gentity_t *LaunchItem( gitem_t *item, vec3_t origin, vec3_t velocity ) {
	gentity_t	*dropped;
	int			contents;

	dropped = G_Spawn();

	dropped->s.eType = ET_ITEM;
	dropped->s.modelindex = item - bg_itemlist;	// store item number in modelindex
	dropped->s.modelindex2 = 1; // This is non-zero is it's a dropped item

	dropped->classname = item->classname;
	dropped->item = item;
	VectorSet (dropped->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS);
	VectorSet (dropped->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS);
	dropped->r.contents = CONTENTS_TRIGGER;

	dropped->touch = Touch_Item;

	G_SetOrigin( dropped, origin );
	dropped->s.pos.trType = TR_GRAVITY;
	dropped->s.pos.trTime = level.time;
	VectorCopy( velocity, dropped->s.pos.trDelta );

	dropped->s.eFlags |= EF_BOUNCE_HALF;

	/*if (g_gametype.integer == GT_CTF && item->giType == IT_TEAM) { // Special case for CTF flags
		dropped->think = Team_DroppedFlagThink;
		dropped->nextthink = level.time + 30000;
		Team_CheckDroppedItem( dropped );
	// RR2DO2: make sure ammobox count decreased again
	} else*/ if (item->giType == IT_Q3F_AMMOBOX ) {
		dropped->think = G_Q3F_FreeAmmoBox;
		dropped->nextthink = level.time + 30000;
	// RR2DO2
	} else { // auto-remove after 30 seconds
		dropped->think = G_FreeEntity;
		dropped->nextthink = level.time + 30000;
	}

	dropped->flags = FL_DROPPED_ITEM;

	contents = trap_PointContents( dropped->r.currentOrigin, -1 );

	if ( contents & CONTENTS_WATER ) {
		dropped->hasbeeninwater = qtrue;
	}

	trap_LinkEntity (dropped);

	return dropped;
}

/*
================
Drop_Item

Spawns an item and tosses it forward
================
*/
gentity_t *Drop_Item( gentity_t *ent, gitem_t *item, float angle ) {
	vec3_t	velocity;
	vec3_t	angles;
	gentity_t *itement;

	VectorCopy( ent->s.apos.trBase, angles );
	angles[YAW] += angle;
	angles[PITCH] = 0;	// always forward

	AngleVectors( angles, velocity, NULL, NULL );
	VectorScale( velocity, 150, velocity );
	velocity[2] += 200 + Q_flrand(-1.0f, 1.0f) * 50;
	
	itement = LaunchItem( item, ent->s.pos.trBase, velocity );
	return( itement );
}


/*
================
Use_Item

Respawn the item
================
*/
void Use_Item( gentity_t *ent, gentity_t *other, gentity_t *activator ) {
	RespawnItem( ent );
}

//======================================================================

/*
================
FinishSpawningItem

Traces down to find where an item should rest, instead of letting them
free fall from their spawn points
================
*/
void FinishSpawningItem( gentity_t *ent ) {
	trace_t		tr;
	vec3_t		dest;
	int			contents;

	VectorSet( ent->r.mins, -ITEM_RADIUS, -ITEM_RADIUS, -ITEM_RADIUS );
	VectorSet( ent->r.maxs, ITEM_RADIUS, ITEM_RADIUS, ITEM_RADIUS );

	ent->s.eType = ET_ITEM;
	ent->s.modelindex = ent->item - bg_itemlist;		// store item number in modelindex
	ent->s.modelindex2 = 0; // zero indicates this isn't a dropped item

	ent->r.contents = CONTENTS_TRIGGER;
	ent->touch = Touch_Item;
	// useing an item causes it to respawn
	// Golliwog: Disabled this for now, it's not used in any CTF/Q3F maps anyway, as far as I know.
	// It has a nasty side effect in that if the item is given extended entity data, it goes into
	// the ext processing system, and "use" (i.e. respawn) will be used in preference to "touch",
	// causing the ent to be impossible to pick up.
//	ent->use = Use_Item;
	// Golliwog.

	if ( ent->spawnflags & 1 ) {
		// suspended
		G_SetOrigin( ent, ent->s.origin );
	} else {
		// drop to floor
		VectorSet( dest, ent->s.origin[0], ent->s.origin[1], ent->s.origin[2] - 4096 );
		trap_Trace( &tr, ent->s.origin, ent->r.mins, ent->r.maxs, dest, ent->s.number, MASK_SOLID );
		if ( tr.startsolid ) {
			G_Printf ("FinishSpawningItem: %s startsolid at %s\n", ent->classname, vtos(ent->s.origin));
			G_FreeEntity( ent );
			return;
		}

		// allow to ride movers
		ent->s.groundEntityNum = tr.entityNum;

		G_SetOrigin( ent, tr.endpos );
	}

	// team slaves and targeted items aren't present at start
// XreaL BEGIN
#ifdef _ETXREAL
	// Doom 3 mapping convention: every entity has a name	
	if ( ( ent->flags & FL_TEAMSLAVE ) || ( ent->targetname && !ent->targetnameAutogenerated ) ) {
#else
	if ( ( ent->flags & FL_TEAMSLAVE ) || ent->targetname ) {
#endif
// XreaL END
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		return;
	}

	// powerups don't spawn in for a while
	if ( ent->item->giType == IT_POWERUP ) {
		float	respawn;

		respawn = 45 + Q_flrand(-1.0f, 1.0f) * 15;
		ent->s.eFlags |= EF_NODRAW;
		ent->r.contents = 0;
		ent->nextthink = level.time + respawn * 1000;
		ent->think = RespawnItem;
		return;
	}

	contents = trap_PointContents( ent->r.currentOrigin, -1 );

	if ( contents & CONTENTS_WATER ) {
		ent->hasbeeninwater = qtrue;
	}

	trap_LinkEntity (ent);
}


qboolean	itemRegistered[MAX_ITEMS];

/*
==================
G_CheckTeamItems
==================
*/
void G_CheckTeamItems( void ) {

	// Set up team stuff
	Team_InitGame();

}

/*
==============
ClearRegisteredItems
==============
*/
void ClearRegisteredItems( void ) {
	gitem_t		*item;

	memset( itemRegistered, 0, sizeof( itemRegistered ) );

	// players always start with the base weapon
	if ( ( item = BG_FindItemForWeapon( WP_NAILGUN ) ) != NULL )
		RegisterItem(item);
	if ( ( item = BG_FindItemForWeapon( WP_AXE ) ) != NULL )
		RegisterItem(item);

	// Golliwog: Ensure the backpack & ammo box is precached
	if ( ( item = BG_FindItem( "Backpack" ) ) != NULL )
		RegisterItem(item);
	if ( ( item = BG_FindItem( "Ammo (Shells)" ) ) != NULL )
		RegisterItem(item);
	if ( ( item = BG_FindItem( "Ammo (Nails)" ) ) != NULL )
		RegisterItem(item);
	if ( ( item = BG_FindItem( "Ammo (Rockets)" ) ) != NULL )
		RegisterItem(item);
	if ( ( item = BG_FindItem( "Ammo (Cells)" ) ) != NULL )
		RegisterItem(item);
	// Golliwog.
}

/*
===============
RegisterItem

The item will be added to the precache list
===============
*/
void RegisterItem( gitem_t *item ) {
	if ( !item ) {
		G_Error( "RegisterItem: NULL" );
	}
	itemRegistered[ item - bg_itemlist ] = qtrue;
}


/*
===============
SaveRegisteredItems

Write the needed items to a config string
so the client will know which ones to precache
===============
*/
void SaveRegisteredItems( void ) {
	char	string[MAX_ITEMS+1];
	int		i;
	int		count;

	count = 0;
	for ( i = 0 ; i < bg_numItems ; i++ ) {
		if ( itemRegistered[i] ) {
			count++;
			string[i] = '1';
		} else {
			string[i] = '0';
		}
	}
	string[ bg_numItems ] = 0;

	G_Printf( "%i items registered\n", count );
	trap_SetConfigstring(CS_ITEMS, string);
}

/*
============
G_SpawnItem

Sets the clipping size and plants the object on the floor.

Items can't be immediately dropped to floor, because they might
be on an entity that hasn't spawned yet.
============
*/
void G_SpawnItem (gentity_t *ent, gitem_t *item) {
	G_SpawnFloat( "random", "0", &ent->random );
	G_SpawnFloat( "wait", "0", &ent->wait );

	RegisterItem( item );

	ent->item = item;
	// some movers spawn on the second frame, so delay item
	// spawns until the third frame so they can ride trains
	ent->nextthink = level.time + FRAMETIME * 2;
	ent->think = FinishSpawningItem;

	ent->physicsBounce = 0.50;		// items are bouncy

	if ( item->giType == IT_POWERUP ) {
		G_SoundIndex( "sound/items/poweruprespawn.wav" );
		G_SpawnFloat( "noglobalsound", "0", &ent->speed);
	}

	ent->hasbeeninwater = qfalse;
}


/*
================
G_BounceItem

================
*/
void G_BounceItem( gentity_t *ent, trace_t *trace ) {
	vec3_t	velocity;
	float	dot;
	int		hitTime;

	// reflect the velocity on the trace plane
	hitTime = level.previousTime + ( level.time - level.previousTime ) * trace->fraction;
	BG_EvaluateTrajectoryDelta( &ent->s.pos, hitTime, velocity );
	dot = DotProduct( velocity, trace->plane.normal );
	VectorMA( velocity, -2*dot, trace->plane.normal, ent->s.pos.trDelta );

	// cut the velocity to keep from bouncing forever
	VectorScale( ent->s.pos.trDelta, ent->physicsBounce, ent->s.pos.trDelta );

	// check for stop
	if ( trace->plane.normal[2] > 0 && ent->s.pos.trDelta[2] < 40 ) {
		trace->endpos[2] += 1.0;	// make sure it is off ground
		SnapVector( trace->endpos );
		G_SetOrigin( ent, trace->endpos );
		ent->s.groundEntityNum = trace->entityNum;
		return;
	}

	VectorAdd( ent->r.currentOrigin, trace->plane.normal, ent->r.currentOrigin);
	VectorCopy( ent->r.currentOrigin, ent->s.pos.trBase );
	ent->s.pos.trTime = level.time;
}


/*
================
G_RunItem

================
*/
void G_RunItem( gentity_t *ent ) {
	vec3_t		origin;
	trace_t		tr;
	int			contents;
	int			mask;

	// if groundentity has been set to -1, it may have been pushed off an edge
	if ( ent->s.groundEntityNum == -1 || ent->s.groundEntityNum == ENTITYNUM_NONE ) {
		if ( ent->s.pos.trType != TR_GRAVITY ) {
			ent->s.pos.trType = TR_GRAVITY;
			ent->s.pos.trTime = level.time;
		}
	}

	if ( ent->s.pos.trType == TR_STATIONARY ) {
		// check think function
		G_RunThink( ent );
		return;
	}

	// get current position
	BG_EvaluateTrajectory( &ent->s.pos, level.time, origin );

	// trace a line from the previous position to the current position
	if ( ent->clipmask ) {
		mask = ent->clipmask;
	} else {
		mask = MASK_PLAYERSOLID & ~CONTENTS_BODY;//MASK_SOLID;
	}
//	trap_Trace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, 
//		ent->r.ownerNum, mask );
	if( ent->s.eType == ET_GENERAL && ent->parent && ent->parent->inuse && ent->parent->client )
		G_Q3F_ForceFieldExtTrace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->r.ownerNum, ent->parent->s.number, mask );
	else G_Q3F_ForceFieldTrace( &tr, ent->r.currentOrigin, ent->r.mins, ent->r.maxs, origin, ent->r.ownerNum, mask );

	VectorCopy( tr.endpos, ent->r.currentOrigin );

	if ( tr.startsolid ) {
		tr.fraction = 1;
		// Golliwog: Have it just 'stick' here.
		G_SetOrigin( ent, tr.endpos );
		// Golliwog.
	}

	/* This is going to be expensive */
	if(levelhasnoannoys)
	{
		if(ent->activator && ent->activator->client && ent->flags & FL_DROPPED_ITEM)
		{
			vec3_t tracestart, traceend;

			VectorAdd( tr.endpos, ent->r.mins, tracestart );
			VectorAdd( tr.endpos, ent->r.maxs, traceend );

			if( G_Q3F_NoAnnoyCheck( tracestart, traceend, ent->activator->client->sess.sessionTeam, Q3F_NOANNOY_BACKPACKS ) )
			{
				G_FreeEntity( ent );
				return;
			}
		}
	}

	trap_LinkEntity( ent );	// FIXME: avoid this for stationary?

	// check think function
	G_RunThink( ent );

	// RR2DO2: check for watersplash
	contents = trap_PointContents( ent->r.currentOrigin, -1 );
	if( !ent->hasbeeninwater && ent->s.eType != ET_INVISIBLE ) {
		if ( contents & CONTENTS_WATER ) {
			G_AddEvent( ent, EV_ETF_WATERSPLASH, 0 );
			ent->hasbeeninwater = qtrue;
		}
	} else {
		if ( !(contents & CONTENTS_WATER) ) {
			ent->hasbeeninwater = qfalse;
		}
	}

	if ( tr.fraction == 1 ) {
		return;
	}

	// if it is in a nodrop volume, remove it
	contents = trap_PointContents( ent->r.currentOrigin, -1 );
	if ( contents & CONTENTS_NODROP ) {
		if( ent->takedamage )
			G_Damage( ent, NULL, NULL, NULL, NULL, 10000, DAMAGE_NO_PROTECTION, MOD_NODROP );
		else
			G_FreeEntity( ent );
		return;
	}

	G_BounceItem( ent, &tr );
}

