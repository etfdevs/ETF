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

// bg_events.h -- definitions shared by both the server game and client game modules

#ifdef EVENT_ENUMS
#define DECLARE_EVENT( ev ) ev
#endif

#ifdef EVENT_STRINGS
#define DECLARE_EVENT( ev ) #ev
#endif

#ifdef EVENT_ENUMS
DECLARE_EVENT( EV_NONE ) = 0,
#elif defined(EVENT_STRINGS)
DECLARE_EVENT( EV_NONE ),
#endif

DECLARE_EVENT( EV_FOOTSTEP ),
DECLARE_EVENT( EV_FOOTSTEP_METAL ),
DECLARE_EVENT( EV_FOOTSPLASH ),
DECLARE_EVENT( EV_FOOTWADE ),
DECLARE_EVENT( EV_SWIM ),

DECLARE_EVENT( EV_STEP_4 ),
DECLARE_EVENT( EV_STEP_8 ),
DECLARE_EVENT( EV_STEP_12 ),
DECLARE_EVENT( EV_STEP_16 ),

DECLARE_EVENT( EV_FALL_SHORT ),

DECLARE_EVENT( EV_FALL_D11 ),
DECLARE_EVENT( EV_FALL_D13 ),
DECLARE_EVENT( EV_FALL_D15 ),
DECLARE_EVENT( EV_FALL_D17 ),
DECLARE_EVENT( EV_FALL_D19 ),
DECLARE_EVENT( EV_FALL_D21 ),
DECLARE_EVENT( EV_FALL_D23 ),
DECLARE_EVENT( EV_FALL_D25 ),
DECLARE_EVENT( EV_FALL_D27 ),
DECLARE_EVENT( EV_FALL_D29 ),
DECLARE_EVENT( EV_FALL_D31 ),

DECLARE_EVENT( EV_JUMP_PAD ),			// boing sound at origin ), jump sound on player

DECLARE_EVENT( EV_JUMP ),
DECLARE_EVENT( EV_WATER_TOUCH ),	// foot touches
DECLARE_EVENT( EV_WATER_LEAVE ),	// foot leaves
DECLARE_EVENT( EV_WATER_UNDER ),	// head touches
DECLARE_EVENT( EV_WATER_CLEAR ),	// head leaves

DECLARE_EVENT( EV_ITEM_PICKUP ),			// normal item pickups are predictable
DECLARE_EVENT( EV_GLOBAL_ITEM_PICKUP ),	// powerup / team sounds are broadcast to everyone

DECLARE_EVENT( EV_NOAMMO ),
DECLARE_EVENT( EV_CHANGE_WEAPON ),
DECLARE_EVENT( EV_COCK_WEAPON ),
DECLARE_EVENT( EV_FIRE_WEAPON ),

DECLARE_EVENT( EV_USE_ITEM0 ),
DECLARE_EVENT( EV_USE_ITEM1 ),
DECLARE_EVENT( EV_USE_ITEM2 ),
DECLARE_EVENT( EV_USE_ITEM3 ),
DECLARE_EVENT( EV_USE_ITEM4 ),
DECLARE_EVENT( EV_USE_ITEM5 ),
DECLARE_EVENT( EV_USE_ITEM6 ),
DECLARE_EVENT( EV_USE_ITEM7 ),
DECLARE_EVENT( EV_USE_ITEM8 ),
DECLARE_EVENT( EV_USE_ITEM9 ),
DECLARE_EVENT( EV_USE_ITEM10 ),
DECLARE_EVENT( EV_USE_ITEM11 ),
DECLARE_EVENT( EV_USE_ITEM12 ),
DECLARE_EVENT( EV_USE_ITEM13 ),
DECLARE_EVENT( EV_USE_ITEM14 ),
DECLARE_EVENT( EV_USE_ITEM15 ),

DECLARE_EVENT( EV_ITEM_RESPAWN ),
DECLARE_EVENT( EV_ITEM_POP ),
DECLARE_EVENT( EV_PLAYER_TELEPORT_IN ),
DECLARE_EVENT( EV_PLAYER_TELEPORT_OUT ),

DECLARE_EVENT( EV_GRENADE_BOUNCE ),		// eventParm will be the soundindex

DECLARE_EVENT( EV_GENERAL_SOUND ),
DECLARE_EVENT( EV_GLOBAL_SOUND ),		// no attenuation
DECLARE_EVENT( EV_GLOBAL_TEAM_SOUND ),

DECLARE_EVENT( EV_BULLET_HIT_FLESH ),
DECLARE_EVENT( EV_BULLET_HIT_WALL ),

DECLARE_EVENT( EV_SNIPER_HIT_FLESH ),
DECLARE_EVENT( EV_SNIPER_HIT_WALL ),

DECLARE_EVENT( EV_MISSILE_HIT ),
DECLARE_EVENT( EV_MISSILE_MISS ),
DECLARE_EVENT( EV_MISSILE_MISS_METAL ),
DECLARE_EVENT( EV_RAILTRAIL ),
DECLARE_EVENT( EV_SHOTGUN ),
DECLARE_EVENT( EV_SINGLESHOTGUN ),
DECLARE_EVENT( EV_MINIGUN ),
DECLARE_EVENT( EV_NAIL ),				// otherEntity is the shooter

DECLARE_EVENT( EV_PAIN ),
DECLARE_EVENT( EV_DEATH1 ),
DECLARE_EVENT( EV_DEATH2 ),
DECLARE_EVENT( EV_DEATH3 ),
DECLARE_EVENT( EV_OBITUARY ),

DECLARE_EVENT( EV_POWERUP_QUAD ),
DECLARE_EVENT( EV_POWERUP_BATTLESUIT ),
DECLARE_EVENT( EV_POWERUP_REGEN ),

DECLARE_EVENT( EV_GIB_PLAYER ),			// gib a previously living player
DECLARE_EVENT( EV_SCOREPLUM ),			// score plum

DECLARE_EVENT( EV_DEBUG_LINE ),

DECLARE_EVENT( EV_STOPLOOPINGSOUND ),

DECLARE_EVENT( EV_TAUNT ),

DECLARE_EVENT( EV_WEAPON_AIM ),
DECLARE_EVENT( EV_WEAPON_START ),
DECLARE_EVENT( EV_WEAPON_END ),

DECLARE_EVENT( EV_DISEASE ),

DECLARE_EVENT( EV_SENTRY_IDLESTART ),	// Golliwog: Idle start and stop sounds
DECLARE_EVENT( EV_SENTRY_IDLESTOP ),

DECLARE_EVENT( EV_MUZZLEFLASH ),				// Golliwog: The old classic

DECLARE_EVENT( EV_ALLYOBITUARY ),				// Just killed a teammate.

DECLARE_EVENT( EV_ETF_EXPLOSION ),				// Ent-driven explosion effect
DECLARE_EVENT( EV_ETF_GRENADE_EXPLOSION ),		// Ent-driven explosion effect for grenades

DECLARE_EVENT( EV_ETF_DISCARD_AMMO ),
DECLARE_EVENT( EV_ETF_USE_ITEM_FAILED ),

DECLARE_EVENT( EV_ETF_GASEXPLOSION ),

DECLARE_EVENT( EV_ETF_WATERSPLASH ),

DECLARE_EVENT( EV_VISUAL_TRIGGER ),
DECLARE_EVENT( EV_SENTRY_BUILD ),

DECLARE_EVENT( EV_ETF_MINIGUN_START ),

DECLARE_EVENT( EV_VISUAL_NAILFIRE ),

DECLARE_EVENT( EV_DEBUG_DATA ),

DECLARE_EVENT( EV_SENTRY_EXPLOSION ),
DECLARE_EVENT( EV_ETF_SUPPLYSTATION_EXPLOSION ),

DECLARE_EVENT( EV_GURP ),
DECLARE_EVENT( EV_DROWN ),
DECLARE_EVENT( EV_BURN ),
DECLARE_EVENT( EV_BURNTODEATH ),

DECLARE_EVENT( EV_HE_BEEP ),
DECLARE_EVENT( EV_HE_BEEP2 ),
DECLARE_EVENT( EV_HE_EXPLODE ),

DECLARE_EVENT( EV_RELOAD_WEAPON ),

DECLARE_EVENT( EV_DOOR ),			// slothy
DECLARE_EVENT( EV_LIFT ),			// slothy

DECLARE_EVENT( EV_BOT_DEBUG_LINE ),		// drevil
DECLARE_EVENT( EV_BOT_DEBUG_RADIUS ),	// drevil

DECLARE_EVENT( EV_HEAL_PERSON ),

DECLARE_EVENT( EV_SENTRY_SPINUP ), 

DECLARE_EVENT( EV_DISCONNECT ),

DECLARE_EVENT( EV_ETF_FLAMETHROWER_EFFECT ),

DECLARE_EVENT( EV_SUPPLY_BUILD ),
DECLARE_EVENT( EV_PLACE_BUILDING ),

DECLARE_EVENT( EV_POWERUP_PENTAGRAM ),

DECLARE_EVENT( EV_ARMOR_PERSON )

#ifdef EVENT_ENUMS
	, DECLARE_EVENT( EV_MAX )
#endif

#undef DECLARE_EVENT
