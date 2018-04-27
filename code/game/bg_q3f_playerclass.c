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
**	q3f_playerclass.c
**
**	Utility functions and definition table for player classes.
**
*/

#include "g_local.h"
#include "bg_local.h"
#include "bg_public.h"
#include "bg_q3f_playerclass.h"
#include "bg_q3f_grenades.h"

#define	Q3F_CLASS_MAXSPEED_SCALE 1.00// 1.05	// Scale the class speeds by this amount

bg_q3f_playerclass_t bg_q3f_playerclass_null = {
	// A null class, given to you when you join the team for the first time
	"Null Class",
	"A null class, for those embarrassing moments when you find yourself without... :)",
	"",
	{"",						// Class sample
	""},						// Class sample
	"\0",					// Synonyms

	100,					// Max health
	200 * Q3F_CLASS_MAXSPEED_SCALE,			// Max speed
	100,					// Max armour

	{
		0,
		WP_AXE,					// Weapons
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	WP_AXE,					// Default Weapon

	Q3F_GREN_NONE,		Q3F_GREN_NONE,			// Grenade types
	2,					2,						// Initial grenades
	4,					4,						// Max grenades

	100, 100, 100, 100, 0, 0,		// Maximum values for ammo: shells/nails/rockets/cells/medikit/charge
	0,     0,   0,   0,	0, 0,		// Initial values for ammo: shells/nails/rockets/cells/medikit/charge

	0,						// Initial armour

	Q3F_ARMOUR_NONE,		// Max armour type
	Q3F_ARMOUR_NONE,		// Initial armour type
	0, 0,

	200,					// Mass

	//0, 0, 0,				// Agent 'masquerade' Weapons
	{0, 0, 0, 0},				// Weapon map
	
	{-15, -15, -24},		// Bounding Box: Mins
	{15, 15, 32}			// Bounding Box: Maxs

};

bg_q3f_playerclass_t bg_q3f_playerclass_recon = {
	// Recon class
	"Recon",
	"An extremely fast yet weak class - useful for flag runs/rapid response",
	"recon",
	{"sound/voices/hellchick/recon.wav",	// Class sample
	"sound/voices/xian/recon.wav"},		// Class sample
	"",	//"scout\0",				// Synonyms

	75,						// Max health
	440 * Q3F_CLASS_MAXSPEED_SCALE,	// Max speed
	30,						// Max armour
	
	{
		0,
		WP_AXE,					// Weapons
		WP_SHOTGUN,
		0,
		WP_NAILGUN,			//FALCON: Nailgun
		0,
		0,
		0,
		0,
		0,
	},

	WP_NAILGUN,			// Default Weapon

	Q3F_GREN_FLASH,		Q3F_GREN_CONCUSS,		// Grenade types
	2,					3,						// Initial grenades
	4,					4,						// Max grenades

	 50, 200,  25, 100, 0, 0,		// Maximum values for ammo: shells/nails/rockets/cells/medikit/charge
	 25, 100,   0,  50,	0, 0,		// Initial values for ammo: shells/nails/rockets/cells/medikit/charge

	25,						// Initial armour

	Q3F_ARMOUR_GREEN,	Q3F_ARMOUR_GREEN,	// Max/Initial armour type
	0,					0,					// Max/initial armour class

	200,					// Mass

	//WP_SHOTGUN, WP_SHOTGUN, WP_NAILGUN,		// Agent 'masquerade' Weapons
	{WP_AXE, WP_NAILGUN, WP_SHOTGUN, WP_SHOTGUN},	// Weapon map  keeg changed order

	{-15, -15, -24},		// Bounding Box: Mins
	{15, 15, 32}			// Bounding Box: Maxs

};

bg_q3f_playerclass_t bg_q3f_playerclass_sniper = {
	// Sniper class
	"Sniper",
	"The sniper allows for striking targets at long range with high accuracy",
	"sniper",
	{"sound/voices/hellchick/sniper.wav",	// Class sample
	"sound/voices/xian/sniper.wav"},			// Class sample
	"",						// Synonyms

	90,						// Max health
	280 * Q3F_CLASS_MAXSPEED_SCALE,	// Max speed
	50,						// Max armour
		
	{
		0,
		WP_AXE,					// Weapons
		WP_SNIPER_RIFLE,
		WP_ASSAULTRIFLE,			//FALCON: Assault Rifle
		WP_NAILGUN,			//FALCON: Nailgun
		0,
		0,
		0,
		0,
		0
	},

	WP_SNIPER_RIFLE,					// Default Weapon

	Q3F_GREN_NORMAL,	/*Q3F_GREN_FLARE,*/	Q3F_GREN_FLASH,			// Grenade types
	2,					2,						// Initial grenades
	2,					2,						// Max grenades

	75,  100,  25,  50, 0, 0,		// Maximum values for ammo: shells/nails/rockets/cells/medikit/charge
	65,   50,   0,   0, 0, 0, 		// Initial values for ammo: shells/nails/rockets/cells/medikit/charge

	0,						// Initial armour

	Q3F_ARMOUR_GREEN,	Q3F_ARMOUR_GREEN,				// Max/Initial armour type
	DAMAGE_Q3F_SHELL|DAMAGE_Q3F_NAIL, 0,				// Max/initial armour class

	200,					// Mass

	//WP_SNIPER_RIFLE, WP_ASSAULTRIFLE, WP_NAILGUN,	// Agent 'masquerade' Weapons
	{WP_AXE, WP_SNIPER_RIFLE, WP_ASSAULTRIFLE, WP_NAILGUN},	// Weapon map Keeg changed

	{-15, -15, -24},		// Bounding Box: Mins
	{15, 15, 32}			// Bounding Box: Maxs

};

bg_q3f_playerclass_t bg_q3f_playerclass_soldier = {
	// Soldier class
	"Soldier",
	"A general-purpose class, armed with a rocket launcher etc. etc.",
	"soldier",
	{"sound/voices/hellchick/soldier.wav",	// Class sample
	"sound/voices/xian/soldier.wav"},		// Class sample
	"",						// Synonyms

	100,					// Max health
	240 * Q3F_CLASS_MAXSPEED_SCALE,	// Max speed
	200,					// Max armour
	
	{
		0,
		WP_AXE,					// Weapons
		WP_SHOTGUN,
		WP_SUPERSHOTGUN,		//FALCON: Double Barrelled
		0,
		0,
		0,
		WP_ROCKET_LAUNCHER,
		0,
		0
	},

	WP_ROCKET_LAUNCHER,					// Default Weapon

	Q3F_GREN_NORMAL,	Q3F_GREN_NAIL,			// Grenade types
	4,					1,						// Initial grenades
	4,					2,						// Max grenades

	100, 100,  50,  50, 0, 0,		// Maximum values for ammo: shells/nails/rockets/cells/medikit/charge
	50,    0,  10,   0, 0, 0,		// Initial values for ammo: shells/nails/rockets/cells/medikit/charge

	100,						// Initial armour

	Q3F_ARMOUR_RED,		Q3F_ARMOUR_RED,				// Max/Initial armour type
	DAMAGE_Q3F_MASK,	0,							// Max/initial armour class

	200,					// Mass

	//WP_SHOTGUN, WP_SUPERSHOTGUN, WP_ROCKET_LAUNCHER,	// Agent 'masquerade' Weapons
	{WP_AXE, WP_ROCKET_LAUNCHER, WP_SUPERSHOTGUN, WP_SHOTGUN},	// Weapon map  Keeg changed order

	{-15, -15, -24},		// Bounding Box: Mins
	{15, 15, 32}			// Bounding Box: Maxs

};

bg_q3f_playerclass_t bg_q3f_playerclass_grenadier = {
	// Demoman class
	"Grenadier",
	"A great all round class with average strength, weaponry and speed",
	"grenadier",
	{"sound/voices/hellchick/grenadier.wav",	// Class sample
	"sound/voices/xian/grenadier.wav"},		// Class sample
	"gren\0",//"gren\0demo\0demoman\0",	// Synonyms

	90,						// Max health
	280 * Q3F_CLASS_MAXSPEED_SCALE,	// Max speed
	120,					// Max armour
	
	{
		0,
		WP_AXE,					// Weapons
		WP_SHOTGUN,
		0,
		0,
		0,
		WP_GRENADE_LAUNCHER, 
		WP_PIPELAUNCHER,
		0,
		0
	},

	WP_GRENADE_LAUNCHER,					// Default Weapon

	Q3F_GREN_NORMAL,	Q3F_GREN_CLUSTER,			// Grenade types
	4,					2,						// Initial grenades
	4,					4,						// Max grenades

	 75,  50,  50,  50, 0, 1,		// Maximum values for ammo: shells/nails/rockets/cells/medikit/charge
	 30,   0,  20,   0, 0, 1,		// Initial values for ammo: shells/nails/rockets/cells/medikit/charge

	50,						// Initial armour

	Q3F_ARMOUR_YELLOW,	Q3F_ARMOUR_YELLOW,			// Max/Initial armour type
	DAMAGE_Q3F_MASK,	0,							// Max/initial armour class

	200,					// Mass

	//WP_PIPELAUNCHER, WP_SHOTGUN, WP_GRENADE_LAUNCHER,	// Agent 'masquerade' Weapons
	{WP_AXE, WP_PIPELAUNCHER, WP_GRENADE_LAUNCHER, WP_SHOTGUN},  // Weapon map Keeg changed

	{-15, -15, -24},		// Bounding Box: Mins
	{15, 15, 32}			// Bounding Box: Maxs

};

bg_q3f_playerclass_t bg_q3f_playerclass_paramedic = {
	// Paramedic class
	"Paramedic",
	"A paramedic is a medium armored class who can heal his/her team mates",
	"paramedic",
	{"sound/voices/hellchick/paramedic.wav",	// Class sample
	"sound/voices/xian/paramedic.wav"},		// Class sample
	"para\0",//"para\0medic\0",						// Synonyms

	90,						// Max health
	320 * Q3F_CLASS_MAXSPEED_SCALE,	// Max speed
	100,					// Max armour
	
	{
		0,
		WP_AXE,					// Weapons
		WP_SHOTGUN,
		WP_SUPERSHOTGUN,		//FALCON: Double Barrelled
		0,
		WP_SUPERNAILGUN,
		0,
		0,
		0,
		0
	},

	WP_SUPERNAILGUN,					// Default Weapon

	Q3F_GREN_NORMAL,	Q3F_GREN_CONCUSS,		// Grenade types
	3,					2,						// Initial grenades
	3,					4,						// Max grenades

	 70, 150,  25,  50,	100, 0,	// Maximum values for ammo: shells/nails/rockets/cells/medikit/charge
	 50,  50,   0,   0,	50, 0,	// Initial values for ammo: shells/nails/rockets/cells/medikit/charge

	50,						// Initial armour

	Q3F_ARMOUR_YELLOW,	Q3F_ARMOUR_GREEN,			// Max/Initial armour type
	DAMAGE_Q3F_SHELL|DAMAGE_Q3F_NAIL|DAMAGE_Q3F_SHOCK|DAMAGE_Q3F_FIRE, 0,	// Max/initial armour class

	200,					// Mass

	//WP_SUPERSHOTGUN, WP_SUPERSHOTGUN, WP_SUPERNAILGUN,	// Agent 'masquerade' Weapons
	{WP_AXE, WP_SUPERNAILGUN, WP_SUPERSHOTGUN, WP_SHOTGUN}, // Weapon map  Keeg changed

	{-15, -15, -24},		// Bounding Box: Mins
	{15, 15, 32}			// Bounding Box: Maxs

};

bg_q3f_playerclass_t bg_q3f_playerclass_minigunner = {
	// Minigunner class
	"Minigunner",
	"The Minigunner can take more damage and give out more than any other class but is very slow",
	"minigunner",
	{"sound/voices/hellchick/minigunner.wav",	// Class sample
	"sound/voices/xian/minigunner.wav"},			// Class sample
	"mini\0gunner\0",						// Synonyms

	100,					// Max health
	200 * Q3F_CLASS_MAXSPEED_SCALE,	// Max speed
	300,					// Max armour
	
	{
		0,
		WP_AXE,					// Weapons
		WP_SHOTGUN,
		WP_SUPERSHOTGUN,		//FALCON: Double Barrelled
		0,
		0,
		0,
		WP_MINIGUN,		//FALCON: Chaingun
		0,
		0
	},

	WP_MINIGUN,					// Default Weapon

	Q3F_GREN_NORMAL,	Q3F_GREN_CLUSTER,			// Grenade types
	4,					1,						// Initial grenades
	4,					4,						// Max grenades

	200, 200,  25,  50,	0, 0, 	// Maximum values for ammo: shells/nails/rockets/cells/medikit/charge
	200,   0,   0,  24, 0, 0,		// Initial values for ammo: shells/nails/rockets/cells/medikit/charge

	200,					// Initial armour

	Q3F_ARMOUR_RED,		Q3F_ARMOUR_RED,			// Max/Initial armour type
	DAMAGE_Q3F_MASK,	0,						// Max/initial armour class

	500,					// Mass

	//WP_SUPERSHOTGUN, WP_SUPERSHOTGUN, WP_MINIGUN,	// Agent 'masquerade' Weapons
	{WP_AXE, WP_MINIGUN, WP_SUPERSHOTGUN, WP_SHOTGUN}, 	// Weapon map  Keeg changed

	{-15, -15, -24},		// Bounding Box: Mins
	{15, 15, 40}			// Bounding Box: Maxs		// Used to be 32.

};

bg_q3f_playerclass_t bg_q3f_playerclass_flametrooper = {
	// Flametrooper class
	"Flame Trooper",
	"The flame trooper is fairly fast making for a good lightweight attacker",
	"flametrooper",
	{"sound/voices/hellchick/flametrooper.wav",	// Class sample
	"sound/voices/xian/flametrooper.wav"},		// Class sample
	"flame\0trooper\0",					// Synonyms

	100,					// Max health
	300 * Q3F_CLASS_MAXSPEED_SCALE,	// Max speed
	150,					// Max armour
	
	{
		0,
		WP_AXE,					// Weapons
		WP_SHOTGUN,
		0,
		0,
		0,
		WP_FLAMETHROWER,		//FALCON: Flamethrower
		WP_NAPALMCANNON,	//FALCON: Napalm cannon
		0,
		0
	},

	WP_FLAMETHROWER,					// Default Weapon

	Q3F_GREN_NORMAL,	Q3F_GREN_NAPALM,			// Grenade types
	2,					2,						// Initial grenades
	3,					3,						// Max grenades

	 40,  50,  60, 200, 0, 0,		// Maximum values for ammo: shells/nails/rockets/cells/medikit/charge
	 20,  50,  15, 120, 0, 0, 		// Initial values for ammo: shells/nails/rockets/cells/medikit/charge

	50,						// Initial armour

	Q3F_ARMOUR_YELLOW,	Q3F_ARMOUR_YELLOW,		// Max/Initial armour type
	DAMAGE_Q3F_SHELL|DAMAGE_Q3F_NAIL|DAMAGE_Q3F_SHOCK|DAMAGE_Q3F_FIRE, DAMAGE_Q3F_FIRE, // Max/initial armour class

	200,					// Mass

	//WP_FLAMETHROWER, WP_SHOTGUN, WP_NAPALMCANNON,	// Agent 'masquerade' Weapons
	{WP_AXE, WP_NAPALMCANNON, WP_SHOTGUN, WP_FLAMETHROWER},  	// Weapon map Keeg changed

	{-15, -15, -24},		// Bounding Box: Mins
	{15, 15, 32}			// Bounding Box: Maxs

};

bg_q3f_playerclass_t bg_q3f_playerclass_agent = {
	// Agent class
	"Agent",
	"The agent is perfect for taking out enemy defence behind the lines",
	"agent",
	{"sound/voices/hellchick/agent.wav",	// Class sample
	"sound/voices/xian/agent.wav"},		// Class sample
	"",//"spy\0",						// Synonyms

	90,						// Max health
	300 * Q3F_CLASS_MAXSPEED_SCALE,	// Max speed
	100,					// Max armour
	
	{
		0,						// Weapons
		WP_AXE,					//FALCON: Knife
		WP_DARTGUN,				//FALCON: Dart gun
		WP_SUPERSHOTGUN,		//FALCON: Double Barrelled
		WP_NAILGUN,				//FALCON: Nailgun
		0,
		0,
		0,
		0,
		0
	},

	WP_DARTGUN,					// Default Weapon  keeg changed

	Q3F_GREN_NORMAL,	Q3F_GREN_GAS,			// Grenade types
	2,					2,						// Initial grenades
	4,					4,						// Max grenades

	 40, 100,  15,  100, 0, 0,		// Maximum values for ammo: shells/nails/rockets/cells/medikit/charge
	 40,  50,   0,  30, 0, 0,		// Initial values for ammo: shells/nails/rockets/cells/medikit/charge

	0,						// Initial armour

	Q3F_ARMOUR_YELLOW,	Q3F_ARMOUR_GREEN,		// Max/Initial armour type
	DAMAGE_Q3F_SHELL|DAMAGE_Q3F_NAIL|DAMAGE_Q3F_SHOCK|DAMAGE_Q3F_FIRE, 0,	// Max/initial armour class

	200,					// Mass

	//0, 0, 0,				// Agent 'masquerade' Weapons
	{WP_AXE, WP_DARTGUN, WP_SUPERSHOTGUN, WP_NAILGUN},	// Weapon map

	{-15, -15, -24},		// Bounding Box: Mins
	{15, 15, 32}			// Bounding Box: Maxs

};

bg_q3f_playerclass_t bg_q3f_playerclass_engineer = {
	// Engineer class
	"Engineer",
	"Engineers are the ultimate defence class coupled up with their automatic sentry guns",
	"engineer",
	{"sound/voices/hellchick/engineer.wav",	// Class sample
	"sound/voices/xian/engineer.wav"},		// Class sample
	"eng\0",						// Synonyms

	80,						// Max health
	300 * Q3F_CLASS_MAXSPEED_SCALE,	// Max speed
	50,						// Max armour
	
	{
		0,						// Weapons
		WP_AXE,					//FALCON: Spanner
		WP_RAILGUN,				//FALCON: Railgun
		WP_SUPERSHOTGUN,		//FALCON: Double Barrelled
		0,
		0,
		0,
		0,
		0,
		0
	},

	WP_RAILGUN,					// Default Weapon

	Q3F_GREN_NORMAL,	Q3F_GREN_EMP,			// Grenade types
	2,					2,						// Initial grenades
	4,					4,						// Max grenades

	 50,  50,  30, 200, 0, 0,		// Maximum values for ammo: shells/nails/rockets/cells/medikit/charge
	 30,  25,   0, 100, 0, 0, 		// Initial values for ammo: shells/nails/rockets/cells/medikit/charge

	25,						// Initial armour

	Q3F_ARMOUR_YELLOW,	Q3F_ARMOUR_GREEN,		// Max/Initial armour type
	DAMAGE_Q3F_SHELL|DAMAGE_Q3F_NAIL|DAMAGE_Q3F_SHOCK|DAMAGE_Q3F_FIRE, 0, // Max/initial armour class

	200,					// Mass

	//WP_RAILGUN, WP_SUPERSHOTGUN, WP_SUPERSHOTGUN,				// Agent 'masquerade' Weapons
	{WP_AXE, WP_SUPERSHOTGUN, WP_SUPERSHOTGUN, WP_RAILGUN}, 	// Weapon map 

	{-15, -15, -24},		// Bounding Box: Mins
	{15, 15, 32}			// Bounding Box: Maxs

};

bg_q3f_playerclass_t bg_q3f_playerclass_civilian = {
	// Civilian class
	"Civilian",
	"The Civilian is perfect for target practice",
	"civilian",
	{"sound/voices/hellchick/civilian.wav",	// Class sample
	""},										// Class sample
	"civ\0",					// Synonyms

	50,						// Max health
	300 * Q3F_CLASS_MAXSPEED_SCALE,	// Max speed
	0,						// Max armour
	
	{
		0,						// Weapons
		WP_AXE,
		0,
		0,
		0,
		0,
		0,
		0,
		0,
		0
	},

	WP_AXE,					// Default Weapon

	Q3F_GREN_NONE,		Q3F_GREN_NONE,			// Grenade types
	0,					0,						// Initial grenades
	0,					0,						// Max grenades

	  0,   0,   0,   0, 0, 0,		// Maximum values for ammo: shells/nails/rockets/cells/medikit/charge
	  0,   0,   0,   0, 0, 0,		// Initial values for ammo: shells/nails/rockets/cells/medikit/charge

	0,						// Initial armour

	Q3F_ARMOUR_NONE,	Q3F_ARMOUR_NONE,		// Max/Initial armour type
	0,					0,						// Max/initial armour class

	200,					// Mass

	{WP_AXE, WP_AXE, WP_AXE, WP_AXE},	// Weapon map

	{-15, -15, -24},		// Bounding Box: Mins
	{15, 15, 32}			// Bounding Box: Maxs
};

// Array of pointers to class structures
bg_q3f_playerclass_t *bg_q3f_classlist[Q3F_CLASS_MAX] = {
	&bg_q3f_playerclass_null,
	&bg_q3f_playerclass_recon,
	&bg_q3f_playerclass_sniper,
	&bg_q3f_playerclass_soldier,
	&bg_q3f_playerclass_grenadier,
	&bg_q3f_playerclass_paramedic,
	&bg_q3f_playerclass_minigunner,
	&bg_q3f_playerclass_flametrooper,
	&bg_q3f_playerclass_agent,
	&bg_q3f_playerclass_engineer,
	&bg_q3f_playerclass_civilian
};


	// Utility functions



bg_q3f_playerclass_t *BG_Q3F_GetClass( const playerState_t *ps )
{
	// Get the class pointer from the playerstate

	bg_q3f_playerclass_t *cls;

	if(	ps->persistant[PERS_CURRCLASS] < 1 ||
		ps->persistant[PERS_CURRCLASS] >= Q3F_CLASS_MAX )
		return( &bg_q3f_playerclass_null );
	cls = bg_q3f_classlist[ps->persistant[PERS_CURRCLASS]];
	return( cls ? cls : &bg_q3f_playerclass_null );
}

int CG_Q3F_GetClassNum(const char *classname)
{
	int i;
	
	i = atoi( classname );
	if( i )
		return( i );
	for ( i = Q3F_CLASS_NULL; i < Q3F_CLASS_MAX; i++ )
	{
		if (!Q_stricmp(classname, bg_q3f_classlist[i]->title))
			return i;
	}
	return 0;	// no valid class found!
}

int BG_Q3F_GetWeaponSlotFromWeaponNum( const playerState_t *ps, int num)
{
	int i;
	bg_q3f_playerclass_t *cls;
	cls = BG_Q3F_GetClass(ps);

	for(i=0; i < Q3F_NUM_WEAPONSLOTS; i++)
	{
		if(cls->weaponslot[i] == num)
			return i;
	}
	return WP_NONE;
}

int BG_Q3F_GetRemappedWeaponFromWeaponNum( int classNum, int otherClassNum, int weapNum ) {
	int i;

	if( classNum < Q3F_CLASS_NULL || classNum >= Q3F_CLASS_MAX) {
		return weapNum;
	}

	if( classNum == otherClassNum )
		return weapNum;

	for( i = 0; i < Q3F_NUM_WEAPONMAPSLOTS; i++ ) {
		if( bg_q3f_classlist[classNum]->weaponmap[i] == weapNum )
			return( bg_q3f_classlist[otherClassNum]->weaponmap[i] );
	}

	// failsafe, we should never get here
	return weapNum;
}
