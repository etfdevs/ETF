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

// bg_mod.h -- definitions shared by both the server game and client game modules

#ifndef _BG_MOD_H
#define _BG_MOD_H

#ifdef MOD_ENUMS
#define DECLARE_MOD( ev ) ev
#endif

#ifdef MOD_STRINGS
#define DECLARE_MOD( ev ) #ev
#endif

#ifdef MOD_ENUMS
DECLARE_MOD( MOD_UNKNOWN ) = 0,
#elif defined(MOD_STRINGS)
DECLARE_MOD( MOD_UNKNOWN ),
#endif
DECLARE_MOD( MOD_SHOTGUN ),
DECLARE_MOD( MOD_AXE ),
DECLARE_MOD( MOD_NAILGUN ),
DECLARE_MOD( MOD_GRENADE ),
DECLARE_MOD( MOD_GRENADE_SPLASH ),
DECLARE_MOD( MOD_PIPE ),
DECLARE_MOD( MOD_ROCKET ),
DECLARE_MOD( MOD_ROCKET_SPLASH ),
DECLARE_MOD( MOD_FLAME ),
DECLARE_MOD( MOD_FLAME_SPLASH ),
DECLARE_MOD( MOD_RAILGUN ),
DECLARE_MOD( MOD_WATER ),
DECLARE_MOD( MOD_SLIME ),
DECLARE_MOD( MOD_LAVA ),
DECLARE_MOD( MOD_CRUSH ),
DECLARE_MOD( MOD_TELEFRAG ),
DECLARE_MOD( MOD_FALLING ),
DECLARE_MOD( MOD_SUICIDE ),
DECLARE_MOD( MOD_TARGET_LASER ),
DECLARE_MOD( MOD_TRIGGER_HURT ),
DECLARE_MOD( MOD_SNIPER_RIFLE ),
DECLARE_MOD( MOD_SNIPER_RIFLE_HEAD ),
DECLARE_MOD( MOD_SNIPER_RIFLE_FEET ),
DECLARE_MOD( MOD_RIFLE_ASSAULT ),
DECLARE_MOD( MOD_DARTGUN ),
DECLARE_MOD( MOD_KNIFE ),
DECLARE_MOD( MOD_DISEASE ),
DECLARE_MOD( MOD_FAILED_OPERATION ),
DECLARE_MOD( MOD_WRENCH ),
DECLARE_MOD( MOD_HANDGREN ),
DECLARE_MOD( MOD_FLASHGREN ),
DECLARE_MOD( MOD_NAILGREN ),
DECLARE_MOD( MOD_CLUSTERGREN ),
DECLARE_MOD( MOD_NAPALMGREN ),
DECLARE_MOD( MOD_GASGREN ),
DECLARE_MOD( MOD_PULSEGREN ),
DECLARE_MOD( MOD_CHARGE ),
DECLARE_MOD( MOD_AUTOSENTRY_BULLET ),
DECLARE_MOD( MOD_AUTOSENTRY_ROCKET ),
DECLARE_MOD( MOD_AUTOSENTRY_EXPLODE ),
DECLARE_MOD( MOD_SUPPLYSTATION_EXPLODE ),
DECLARE_MOD( MOD_SINGLESHOTGUN ),
DECLARE_MOD( MOD_MINIGUN ),
DECLARE_MOD( MOD_CUSTOM ),
DECLARE_MOD( MOD_MIRROR ),
DECLARE_MOD( MOD_BEAM ),
DECLARE_MOD( MOD_MAPSENTRY ),
DECLARE_MOD( MOD_GASEXPLOSION ),
DECLARE_MOD( MOD_CRUSHEDBYSENTRY ),
DECLARE_MOD( MOD_MAPSENTRY_BULLET ),
DECLARE_MOD( MOD_MAPSENTRY_ROCKET ),
DECLARE_MOD( MOD_NODROP ),
DECLARE_MOD( MOD_SUPERNAILGUN ),
DECLARE_MOD( MOD_CRUSHEDBYSUPPLYSTATION ),
DECLARE_MOD( MOD_NEEDLE_PRICK ),
DECLARE_MOD( MOD_SWITCHTEAM ),
DECLARE_MOD( MOD_DISCONNECT )

#ifdef MOD_ENUMS
	, DECLARE_MOD( MOD_LASTONE )
#endif

#undef DECLARE_MOD

#endif
