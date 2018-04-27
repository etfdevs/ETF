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

// cg_event.c -- handle entity events at snapshot or playerstate transitions

#include "cg_local.h"
#include "cg_q3f_scanner.h"

// for the voice chats
#include "../../ui/menudef.h"

/*
===================
CG_PlaceString

Also called by scoreboard drawing
===================
*/
const char	*CG_PlaceString( int rank ) {
	static char	str[64];
	char	*s, *t;

	if ( rank & RANK_TIED_FLAG ) {
		rank &= ~RANK_TIED_FLAG;
		t = "Tied for ";
	} else {
		t = "";
	}

	if ( rank == 1 ) {
		s = S_COLOR_BLUE "1st" S_COLOR_WHITE;		// draw in blue
	} else if ( rank == 2 ) {
		s = S_COLOR_RED "2nd" S_COLOR_WHITE;		// draw in red
	} else if ( rank == 3 ) {
		s = S_COLOR_YELLOW "3rd" S_COLOR_WHITE;		// draw in yellow
	} else if ( rank == 11 ) {
		s = "11th";
	} else if ( rank == 12 ) {
		s = "12th";
	} else if ( rank == 13 ) {
		s = "13th";
	} else if ( rank % 10 == 1 ) {
		s = va("%ist", rank);
	} else if ( rank % 10 == 2 ) {
		s = va("%ind", rank);
	} else if ( rank % 10 == 3 ) {
		s = va("%ird", rank);
	} else {
		s = va("%ith", rank);
	}

	Com_sprintf( str, sizeof( str ), "%s%s", t, s );
	return str;
}

/*
=============
CG_Obituary
=============
*/
static void CG_Obituary( entityState_t *ent, qboolean isally ) {
	int			mod;
	int			target, attacker;
	char		*message;
	char		*message2;
	char		*obit;
	const char	*targetInfo;
	const char	*attackerInfo;
	char		targetName[34];
	char		attackerName[34];
	char		tempbuf[32];
	clientInfo_t	*ci;
	int			r;
	int			mynum;

	target = ent->otherEntityNum;
	attacker = ent->otherEntityNum2;
	mod = ent->eventParm;
	mynum = cg.snap->ps.clientNum;

	if((attacker == mynum) && (target != mynum)) {
		cg.mykills++;
		cg.kills[target]++;
	}

	if(target == mynum) {
		cg.mydeaths++;
		cg.deaths[attacker]++;
	}

	if( mod == MOD_DISCONNECT )
		return;

	if( mod == MOD_CUSTOM && !cg_altObits.integer && !cg_filterObituaries.integer )
		return;			// Golliwog: Server handled this obit

	if ( target < 0 || target >= MAX_CLIENTS ) {
		CG_Error( "CG_Obituary: target out of range" );
	}
	ci = &cgs.clientinfo[target];

	if ( attacker < 0 || attacker >= MAX_CLIENTS ) {
		attacker = ENTITYNUM_WORLD;
		attackerInfo = NULL;
	} else {
		attackerInfo = CG_ConfigString( CS_PLAYERS + attacker );
	} 

	targetInfo = CG_ConfigString( CS_PLAYERS + target );
	if ( !targetInfo || !*targetInfo ) {
		return;
	}
	Q_strncpyz( targetName, Info_ValueForKey( targetInfo, "n" ), sizeof(targetName) - 2);
	Q_strcat( targetName, sizeof(targetName), S_COLOR_WHITE );

	// check for double client messages
	if ( !attackerInfo || !*attackerInfo ) {
		attacker = ENTITYNUM_WORLD;
		Q_strncpyz( attackerName, "noname", sizeof(attackerName) - 2 );
		Q_strcat( attackerName, sizeof(attackerName), S_COLOR_WHITE );
	} else {
		Q_strncpyz( attackerName, Info_ValueForKey( attackerInfo, "n" ), sizeof(attackerName) - 2);
		Q_strcat( attackerName, sizeof(attackerName), S_COLOR_WHITE );
		// check for kill messages about the current clientNum
		if ( target == cg.snap->ps.clientNum ) {
			Q_strncpyz( cg.killerName, attackerName, sizeof( cg.killerName ) );
		}
	}

	if(cg_altObits.integer && !cg_filterObituaries.integer)
	{
		qboolean islocal = qfalse;

		if((attacker == mynum) || (target == mynum))
			islocal = qtrue;

		if(cg.numObits == (MAX_OBITS)) 
		{
			memmove(&cg.obits[0], &cg.obits[1], sizeof(altObit_t) * (MAX_OBITS - 1));
			memset(&cg.obits[MAX_OBITS - 1], 0, sizeof(altObit_t));
			cg.numObits--;
		}
		if((attackerInfo != NULL) && (target != attacker)) {
			Q_strncpyz( cg.obits[cg.numObits].attacker, CG_Q3F_GetTeamColorString(atoi(Info_ValueForKey(attackerInfo, "t"))), 3);
			BG_cleanName(attackerName, tempbuf, sizeof(tempbuf), qfalse);
			Q_strcat( cg.obits[cg.numObits].attacker, 34, attackerName);
			cg.obits[cg.numObits].attacklen = CG_Text_Width(tempbuf, 0.2f, 0, NULL);
		}
		Q_strncpyz( cg.obits[cg.numObits].victim, CG_Q3F_GetTeamColorString(atoi(Info_ValueForKey(targetInfo, "t"))), 3);
		BG_cleanName(targetName, tempbuf, sizeof(tempbuf), qfalse);
		Q_strcat( cg.obits[cg.numObits].victim, 34, tempbuf);
		cg.obits[cg.numObits].viclen = CG_Text_Width(tempbuf, 0.2f, 0, NULL);
		cg.obits[cg.numObits].endtime = cg.time + SHOW_OBIT + (islocal ? 3000 : 0);		// obits stay visible for a few secs

		switch(mod)
		{
		case MOD_SWITCHTEAM:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[switchteam]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_SUICIDE:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[suicide]^7", sizeof(cg.obits[cg.numObits].mod));
			break;
			
		case MOD_HANDGREN:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[handgren]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_FLASHGREN:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[flash]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_CLUSTERGREN:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[cluster]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_GASGREN:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[gas]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_PULSEGREN:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[pulse]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_CHARGE:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[charge]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_GRENADE:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[directgrenade]^7", sizeof(cg.obits[cg.numObits].mod));
			break;
		case MOD_GRENADE_SPLASH:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[grenade]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_PIPE:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[pipebomb]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_ROCKET:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[directrocket]^7", sizeof(cg.obits[cg.numObits].mod));
			break;
		case MOD_ROCKET_SPLASH:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[rocket]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_RAILGUN:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[railgun]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_NAILGREN:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[nailgren]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_SUPERNAILGUN:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[supernailgun]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_NAILGUN:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[nailgun]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_SHOTGUN:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[shotgun]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_SINGLESHOTGUN:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[singleshotgun]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_MINIGUN:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[minigun]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_AXE:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[axe]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_KNIFE:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[knife]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_DISEASE:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[disease]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_NEEDLE_PRICK:
		case MOD_FAILED_OPERATION:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[syringe]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_WRENCH:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[wrench]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_FLAME:
		case MOD_FLAME_SPLASH:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[flame]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_NAPALMGREN:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[napalmgren]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_GASEXPLOSION:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[gasexplosion]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_SNIPER_RIFLE:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[sniped]^7", sizeof(cg.obits[cg.numObits].mod));
			break;
		case MOD_SNIPER_RIFLE_HEAD:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[headshot]^7", sizeof(cg.obits[cg.numObits].mod));
			break;
		case MOD_SNIPER_RIFLE_FEET:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[legshot]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_RIFLE_ASSAULT:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[autorifle]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_WATER:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[drown]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_SLIME:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[slime]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_LAVA:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[lava]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_TELEFRAG:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[telefrag]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_CRUSH:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[crush]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_FALLING:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[fall]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_MIRROR:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[mirror]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_DARTGUN:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[dartgun]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_AUTOSENTRY_ROCKET:
		case MOD_AUTOSENTRY_BULLET:
		case MOD_AUTOSENTRY_EXPLODE:
		case MOD_CRUSHEDBYSENTRY:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[sentry]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_MAPSENTRY:
		case MOD_MAPSENTRY_BULLET:
		case MOD_MAPSENTRY_ROCKET:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[mapsentry]^7", sizeof(cg.obits[cg.numObits].mod));
			break;

		case MOD_SUPPLYSTATION_EXPLODE:
		case MOD_CRUSHEDBYSUPPLYSTATION:
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[supply]^7", sizeof(cg.obits[cg.numObits].mod));
			break;
		default:
			// handled below
			break;
		}

		if(cg.obits[cg.numObits].mod[0] == 0)
			Q_strncpyz(cg.obits[cg.numObits].mod, "^1[unknown]^7", sizeof(cg.obits[cg.numObits].mod));

		if(isally)
			cg.obits[cg.numObits].mod[1] = '2'; 


		cg.obits[cg.numObits].modlen = (CG_Text_Width(cg.obits[cg.numObits].mod, 0.2f, 0, NULL) / 2);

		++cg.numObits;
	}

	if( mod == MOD_CUSTOM )
		return;			// Golliwog: Server handled this obit

	message2 = "";

	if ( mod < 0 || mod >= MOD_LASTONE || !modNames[ mod ]) {
		obit = "MOD_ILLEGAL";
	} else {
		obit = modNames[ mod ];
	}

	srand((ent->pos.trBase[0] + 1) * (ent->pos.trBase[1] + 1) * (ent->pos.trBase[2] + 1));

	// check for single client messages

	switch( mod ) {
	case MOD_SUICIDE:
		r = rand() % 2;
		switch(r) {
	case 0:	message = "suicides"; break;
	case 1: message = "quits life"; break;
		}
		break;
	case MOD_FALLING:
		r = rand() % 5;
		switch(r) {
	case 0:	message = "cratered"; break;
	case 1: message = ci->gender == GENDER_MALE ? "broke his legs" : "broke her legs"; break;
	case 2:	message = "felt the wrath of gravity"; break;
	case 3: message = "found out Newton was right"; break;
	case 4: message = "tried to fly"; break;
		}
		break;
	case MOD_CRUSH:
		r = rand() % 5;
		switch(r) {
	case 0:	message = "was squished"; break;
	case 1: message = "was piledriven"; break;
	case 2:	message = ci->gender == GENDER_MALE ? "forgot to wear his helmet" : "forgot to wear her helmet"; break;
	case 3: message = "was pressed flat"; break;
	case 4: message = "couldn't hold as much as Atlas"; break;
		}
		break;
	case MOD_WATER:
		r = rand() % 5;
		switch(r) {
	case 0:	message = "sank like a rock"; break;
	case 1: message = "sleeps in Davy Jones' locker. Yarrrrrr"; break;
	case 2:	message = ci->gender == GENDER_MALE ? "thought he could evolve gills" : "thought she could evolve gills"; break;
	case 3: message = "tried to find Nemo"; break;
	case 4: message = "couldn't find the sunken treasure"; break;
		}
		break;
	case MOD_SLIME:
		r = rand() % 3;
		switch(r) {
	case 0:	message = "melted"; break;
	case 1: message = "dissolved into nothingness"; break;
	case 2:	message = ci->gender == GENDER_MALE ? "dissolved himself" : "dissolved herself"; break;
		}
		break;
	case MOD_LAVA:
		r = rand() % 4;
		switch(r) {
	case 0:	message = "did a back flip into the lava"; break;
	case 1: message = "did a cannonball into the lava"; break;
	case 2:	message = ci->gender == GENDER_MALE ? "sacrificed himself to the lava god" : "sacrificed herself to the lava god"; break;
	case 3: message = "found out molten rock is hot"; break;
		}
		break;
	case MOD_FLAME:
		/* Ensiform - Added for misc_flamethrower */
		r = rand() % 5;
		switch(r) {
	case 0:	message = "was caramelized"; break;
	case 1:	message = "was cooked to 'well done'"; break;
	case 2:	message = "was burnt to a crisp"; break;
	case 3:	message = "was deep fried without batter"; break;
	case 4:	message = "was set ablaze"; break;
		}
		break;
	case MOD_TARGET_LASER:
		r = rand() % 2;
		switch(r) {
	case 0:	message = "saw the light"; break;
	case 1: message = "was vivisected by lasers"; break;
		}
		break;
	case MOD_TRIGGER_HURT:
		r = rand() % 2;
		switch(r) {
	case 0:	message = "was in the wrong place"; break;
	case 1: message = "died"; break;
		}
		break;
	default:
		message = NULL;
		break;
	}

	// 	message = ci->gender == GENDER_MALE ? his : hers;
	if ((message == NULL) && (attacker == target)) {
		switch (mod) {
		case MOD_GRENADE_SPLASH:
			r = rand() % 4;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "tripped on his own grenade" : "tripped on her own grenade"; break;
		case 1: message = ci->gender == GENDER_MALE ? "launched himself a present" : "launched herself a present"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "fed himself some pineapples" : "fed herself some pineapples"; break;
		case 3: message = ci->gender == GENDER_MALE ? "stepped on his own spam" : "stepped on her own spam"; break;
			}
			break;
		case MOD_ROCKET_SPLASH:
			r = rand() % 3;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "blew himself up" : "blew herself up"; break;
		case 1: message = ci->gender == GENDER_MALE ? "missed with his rocket" : "missed with her rocket"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "became disgusted with his own rocket aim" : "became disgusted with her own rocket aim"; break;
			}
			break;
		case MOD_FLAME_SPLASH:
			r = rand() % 4;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "burned himself" : "burned herself"; break;
		case 1: message = ci->gender == GENDER_MALE ? "incinerated himself" : "incinerated herself"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "couldn't take the heat" : "couldn't take the heat"; break;
		case 3: message = ci->gender == GENDER_MALE ? "burned through his own flame-retardant armor" : "burned through her own flame-retardant armor"; break;
			}
			break;
		case MOD_HANDGREN:
			r = rand() % 7;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "swallowed his own grenade" : "swallowed her own grenade"; break;
		case 1: message = ci->gender == GENDER_MALE ? "and his grenade had a violent divorce" : "and her grenade had a violent divorce"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "needed to see his grenade one last time" : "needed to see her grenade one last time"; break;
		case 3: message = ci->gender == GENDER_MALE ? "couldn't avoid his own grenade" : "couldn't avoid her own grenade"; break;
		case 4:	message = ci->gender == GENDER_MALE ? "miscalculated his grenade toss just a little bit" : "miscalculated her grenade toss just a little bit"; break;
		case 5: message = ci->gender == GENDER_MALE ? "played hide & seek with his own grenade, and lost" : "played hide & seek with her own grenade, and lost"; break;
		case 6: message = ci->gender == GENDER_MALE ? "threw the pin" : "threw the pin"; break;
			}
			break;
		case MOD_FLASHGREN:
			r = rand() % 3;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "was dazzled by his own grenade" : "was dazzled by her own grenade"; break;
		case 1: message = ci->gender == GENDER_MALE ? "was blinded by his own light" : "was blinded by her own light"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "burned out his own eye sockets" : "burned out her own eye sockets"; break;
			}
			break;
		case MOD_NAILGREN:
			r = rand() % 4;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "learned how his nail bomb worked" : "learned how her nail bomb worked"; break;
		case 1: message = ci->gender == GENDER_MALE ? "couldn't avoid his own nail bomb" : "couldn't avoid her own nail bomb"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "didn't take notice of his rotating disc of death" : "didn't take notice of her rotating disc of death"; break;
		case 3: message = ci->gender == GENDER_MALE ? "was hypnotized by his spinning nail bomb" : "was hypnotized by her spinning nail bomb"; break;
			}
			break;
		case MOD_CLUSTERGREN:
			message = ci->gender == GENDER_MALE ? "spammed himself with his cluster bomb" : "spammed herself with her cluster bomb";
			break;
		case MOD_NAPALMGREN:
			r = rand() % 3;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "toasted himself with his napalm grenade" : "toasted herself with her napalm grenade"; break;
		case 1: message = ci->gender == GENDER_MALE ? "walked into his own napalm spray" : "walked into her own napalm spray"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "started his day with a large serving of napalm" : "started her day with a large serving of napalm"; break;
			}
			break;
		case MOD_GASGREN:
			r = rand() % 2;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "succumbed to his own gas grenade" : "succumbed to her own gas grenade"; break;
		case 1: message = ci->gender == GENDER_MALE ? "discombobulated his senses with a lethal intake of his own gas" : "discombobulated her senses with a lethal intake of her own gas"; break;
			}
			break;
		case MOD_PULSEGREN:
			r = rand() % 4;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "was shocked by his pulse grenade" : "was shocked by her pulse grenade"; break;
		case 1: message = ci->gender == GENDER_MALE ? "rattles his bones" : "rattles her bones"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "fulgurated himself with his own pulse grenade" : "fulgurated herself with her own pulse grenade"; break;
		case 3: message = ci->gender == GENDER_MALE ? "exploded his own ammunition" : "exploded her own ammunition"; break;
			}
			break;
		case MOD_CHARGE:
			r = rand() % 4;
			switch(r) {
		case 0:	message = "underestimated the power of the HE charge"; break;
		case 1: message = ci->gender == GENDER_MALE ? "disintegrated himself with his own HE charge" : "disintegrated herself with her own HE charge"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "wanted to see what happened when his HE charge went off, and learned a valuable lesson" : "wanted to see what happened when her HE charge went off, and learned a valuable lesson"; break;
		case 3: message = "paints the town red"; break;
			}
			break;
		case MOD_AUTOSENTRY_BULLET:
			r = rand() % 3;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "picked a bad time to polish the barrel of his autosentry" : "picked a bad time to polish the barrel of her autosentry"; break;
		case 1: message = ci->gender == GENDER_MALE ? "looked down the barrel of his own autosentry" : "looked down the barrel of her own autosentry"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "stood on the wrong side of his autosentry" : "stood on the wrong side of her autosentry"; break;
			}
			break;
		case MOD_AUTOSENTRY_ROCKET:
			r = rand() % 3;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "tried to argue with his autosentry's rocket" : "tried to argue with her autosentry's rocket"; break;
		case 1: message = ci->gender == GENDER_MALE ? "was blown up by his own sentry gun" : "was blown up by her own sentry gun"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "got in the way of his autosentry's rocket" : "got in the way of her autosentry's rocket"; break;
			}
			break;
		case MOD_AUTOSENTRY_EXPLODE:
			r = rand() % 3;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "exploded along with his autosentry" : "exploded along with her autosentry"; break;
		case 1: message = ci->gender == GENDER_MALE ? "couldn't repair his autosentry in time" : "couldn't repair her autosentry in time"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "forgot to wear his safety mask" : "forgot to wear her safety mask"; break;
			}
			break;
		case MOD_SUPPLYSTATION_EXPLODE:
			r = rand() % 2;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "fell out with his supply station" : "fell out with her supply station"; break;
		case 1: message = ci->gender == GENDER_MALE ? "used his supply station as a suicide machine" : "used her supply station as a suicide machine"; break;
			}
			break;
		case MOD_GASEXPLOSION:
			message = ci->gender == GENDER_MALE ? "got trapped in his own inferno" : "got trapped in her own inferno";
			break;
		case MOD_CRUSHEDBYSENTRY:
			message = ci->gender == GENDER_MALE ? "learned just how heavy his autosentry really is" : "learned just how heavy her autosentry really is";
			break;
		case MOD_CRUSHEDBYSUPPLYSTATION:
			message = ci->gender == GENDER_MALE ? "learned just how heavy his supply station really is" : "learned just how heavy her supply station really is";
			break;
		case MOD_PIPE:
			r = rand() % 4;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "tripped over his own pipe trap" : "tripped over her own pipe trap"; break;
		case 1: message = ci->gender == GENDER_MALE ? "caught himself with his own pipe trap" : "caught herself with her own pipe trap"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "detonated himself" : "detonated herself"; break;
		case 3: message = ci->gender == GENDER_MALE ? "mistook himself for an enemy" : "mistook herself for an enemy"; break;
			}
			break;
		default:
			message = ci->gender == GENDER_MALE ? "killed himself" : "killed herself";
			break;
		}
	}

	if (message) {
		if(!cg_altObits.integer && !cg_filterObituaries.integer)
			CG_Printf( BOX_PRINT_MODE_CHAT, "%s %s.\n", targetName, message);
		if(cg_altObits.integer == 1 && !cg_filterObituaries.integer)
			CG_Printf( BOX_PRINT_MODE_CONSOLE, "%s %s.\n", targetName, message);
		if ( cg.matchLogFileHandle > 0 && cg.matchState <= MATCH_STATE_PLAYING) {
			CG_MatchLogAddLine(va("suicide %s %s\n", obit, Info_ValueForKey( targetInfo, "n" )));
		}
		return;
	}

	// check for kill messages from the current clientNum
	if ( attacker == cg.snap->ps.clientNum && !isally && !cg_filterObituaries.integer) {
		CG_Printf(BOX_PRINT_MODE_CENTER, "You fragged %s", targetName );
		// print the text message as well
	}

	if ( attacker != ENTITYNUM_WORLD ) {
		switch (mod) {
		case MOD_AXE:
			r = rand() % 4;
			switch(r) {
		case 0:	message = "couldn't handle"; message2 = "'s elite hacking skills"; break;
		case 1:	message = "was neatly murdered by"; break;
		case 2:	message = "was bludgeoned by"; message2 = "'s hatchet"; break;
		case 3:	message = "was chopped to bits by"; break;
			}
			break;
		case MOD_NAILGUN:
			r = rand() % 5;
			switch(r) {
		case 0:	message = "didn't think"; message2 = ci->gender == GENDER_MALE ? "'s nails would ever do enough damage to kill him" : "'s nails would ever do enough damage to kill her"; break;
		case 1:	message = "was nailed by"; break;
		case 2:	message = "was transfixed by"; message2 = "'s nails"; break;
		case 3:	message = "was finished off by"; message2 = "'s nailgun"; break;
		case 4:	message = "was nailed into place by"; break;
			}
			break;
		case MOD_SUPERNAILGUN:
			r = rand() % 5;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "had his body penetrated by" : "had her body penetrated by"; message2 = "'s super nailgun"; break;
		case 1:	message = "became saturated with nails from"; message2 = "'s super nailgun"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "had a hole punched through his heart by" : "had a hole punched through her heart by"; message2 = "'s super nailgun"; break;
		case 3:	message = "got nailed hard by"; break;
		case 4:	message = "was impaled on"; message2 = "'s nails"; break;
			}
			break;
		case MOD_SHOTGUN:
			r = rand() % 4;
			switch(r) {
		case 0:	message = "caught a mouthful of lead from"; message2 = "'s shotgun"; break;
		case 1:	message = "was shelled by"; message2 = "'s shotgun"; break;
		case 2:	message = "ate"; message2 = "'s buckshot ball"; break;
		case 3:	message = "'s torso is feeling drafty due to"; message2 = "'s super shotgun"; break;
			}
			break;
		case MOD_GRENADE:
			r = rand() % 3;
			switch(r) {
		case 0:	message = "ate"; message2 = "'s grenade"; break;
		case 1:	message = "played with"; message2 = "'s pineapple"; break;
		case 2:	message = "'s head was exploded by"; message2 = "'s grenade"; break;
			}
			break;
		case MOD_GRENADE_SPLASH:
			r = rand() % 3;
			switch(r) {
		case 0:	message = "failed to avoid"; message2 = "'s grenade"; break;
		case 1:	message = "received a pineapple enema from"; break;
		case 2:	message = "didn't see"; message2 = "'s grenade on the ground"; break;
			}
			break;
		case MOD_PIPE:
			r = rand() % 4;
			switch(r) {
		case 0:	message = "swallowed"; message2 = "'s pipe"; break;
		case 1:	message = "thought it was safe to cross"; message2 = "'s pipe trap"; break;
		case 2:	message = "tried to eat"; message2 = "'s banana"; break;
		case 3:	message = "took a pipebomb suppository from"; break;
			}
			break;
		case MOD_ROCKET:
			r = rand() % 5;
			switch(r) {
		case 0:	message = "straddled"; message2 = "'s rocket"; break;
		case 1:	message = "was pulverized by"; message2 = "'s rocket"; break;
		case 2:	message = "ate"; message2 = "'s rocket"; break;
		case 3:	message = "tried to get a better look at"; message2 = "'s rocket"; break;
		case 4:	message = "was reamed by"; message2 = "'s rocket"; break;
			}
			break;
		case MOD_ROCKET_SPLASH:
			r = rand() % 2;
			switch(r) {
		case 0:	message = "almost dodged"; message2 = "'s rocket"; break;
		case 1:	message = "couldn't escape"; message2 = "'s rocket"; break;
			}
			break;
		case MOD_FLAME:
			r = rand() % 5;
			switch(r) {
		case 0:	message = "was caramelized by"; message2 = "'s flame"; break;
		case 1:	message = "was cooked to 'well done' by"; message2 = "'s flame"; break;
		case 2:	message = "was burnt to a crisp by"; break;
		case 3:	message = "was deep fried without batter by"; message2 = "'s flame"; break;
		case 4:	message = "was set ablaze by"; message2 = "'s flame"; break;
			}
			break;
		case MOD_FLAME_SPLASH:
			r = rand() % 3;
			switch(r) {
		case 0:	message = "stood too close to"; message2 = "'s campfire"; break;
		case 1:	message = "was roasted alive by"; message2 = "'s flame"; break;
		case 2:	message = "went out in a blaze of glory thanks to"; break;
			}
			break;
		case MOD_RAILGUN:
			r = rand() % 3;
			switch(r) {
		case 0:	message = "was impaled by"; message2 = "'s rail"; break;
		case 1:	message = "was perforated by"; break;
		case 2:	message = "was railed by"; break;
			}
			break;
		case MOD_TELEFRAG:
			message = "tried to invade"; message2 = "'s personal space";
			break;
		case MOD_SNIPER_RIFLE:
			r = rand() % 5;
			switch(r) {
		case 0:	message = "was given an extra orifice by"; message2 = "'s sniper rifle"; break;
		case 1:	message = "was shot in the liver by"; break;
		case 2:	message = "is sniped by"; break;
		case 3:	message = "was picked off by"; break;
		case 4:	message = ci->gender == GENDER_MALE ? "had his organs shot out by" : "had her organs shot out by"; break;
			}
			break;
		case MOD_SNIPER_RIFLE_HEAD:
			r = rand() % 5;
			switch(r) {
		case 0:	message = "didn't see"; message2 = ci->gender == GENDER_MALE ? "'s large laser spot on his forehead" : "'s large laser spot on her forehead"; break;
		case 1:	message = ci->gender == GENDER_MALE ? "had his block knocked off by" : "had her block knocked off by"; message2 = "'s sniper rifle"; break;
		case 2:	message = "gets a bullet between the eyes from"; break;
		case 3:	message = ci->gender == GENDER_MALE ? "lost his head in" : "lost her head in"; message2 = "'s crosshair"; break;
		case 4:	message = ci->gender == GENDER_MALE ? "had his head taken off by" : "had her head taken off by"; break;
			}
			break;
		case MOD_SNIPER_RIFLE_FEET:
			r = rand() % 3;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "had his legs amputated by" : "had her legs amputated by"; message2 = "'s sniper round"; break;
		case 1:	message = "was kneecapped by"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "gets his legs blown off by" : "gets her legs blown off by"; break;
			}
			break;
		case MOD_RIFLE_ASSAULT:
			r = rand() % 3;
			switch(r) {
		case 0:	message = "received"; message2 = "'s bullet spray"; break;
		case 1:	message = "was killed by"; message2 = "'s resourceful auto rifle"; break;
		case 2:	message = "got filled with holes from"; message2 = "'s auto rifle"; break;
			}
			break;
		case MOD_DARTGUN:
			r = rand() % 3;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "had his insomnia cured by" : "had her insomnia cured by"; message2 = "'s tranquilizer dart"; break;
		case 1:	message = "was knocked out by"; break;
		case 2:	message = "is put to sleep by"; break;
			}
			break;
		case MOD_KNIFE:
			r = rand() % 7;
			switch(r) {
		case 0:	message = "was slashed and gutted by"; message2 = "'s knife"; break;
		case 1:	message = "is knifed by"; break;
		case 2:	message = "was mugged by"; break;
		case 3:	message = "was carved open by"; message2 = "'s knife"; break;
		case 4:	message = "was skewered by"; message2 = "'s knife"; break;
		case 5:	message = ci->gender == GENDER_MALE ? "had his organs fall out from" : "had her organs fall out from"; message2 = "'s knife incision"; break;
		case 6:	message = "was stabbed to death by"; break;
			}
			break;
		case MOD_DISEASE:
			r = rand() % 4;
			switch(r) {
		case 0:	message = "succumbed to"; message2 = "'s infection"; break;
		case 1:	message = "couldn't find the cure for"; message2 = "'s mysterious illness"; break;
		case 2:	message = "rotted to death due to"; message2 = "'s virus"; break;
		case 3:	message = "decomposed rapidly as a result of"; message2 = "'s disease"; break;
			}
			break;
		case MOD_NEEDLE_PRICK:
			r = rand() % 2;
			switch(r) {
		case 0: message = "felt"; message2 = "'s little prick"; break;
		case 1: message = "was penetrated by"; message2 = "'s needle prick"; break;
			}
			break;
		case MOD_FAILED_OPERATION:
			r = rand() % 2;
			switch(r) {
		case 0:	message = "didn't survive"; message2 = "'s operation"; break;
		case 1:	message = "was too weak to live through"; message2 = "'s needle prick"; break;
			}
			break;
		case MOD_WRENCH:
			r = rand() % 2;
			switch(r) {
		case 0:	message = "got a wrench-shaped dent from"; break;
		case 1:	message = ci->gender == GENDER_MALE ? "had his nuts tightened by" : "had her nipples twisted off by"; message2 = "'s wrench"; break;
			}
			break;
		case MOD_HANDGREN:
			r = rand() % 3;
			switch(r) {
		case 0:	message = "mistook"; message2 = "'s grenade for a pineapple"; break;
		case 1:	message = "caught"; message2 = "'s grenade"; break;
		case 2:	message = "jumped onto"; message2 = "'s grenade"; break;
			}
			break;
		case MOD_FLASHGREN:
			r = rand() % 3;
			switch(r) {
		case 0:	message = "was scorched by"; message2 = "'s flash grenade"; break;
		case 1:	message = ci->gender == GENDER_MALE ? "forgot to close his eyes when" : "forgot to close her eyes when"; message2 = "'s flash grenade exploded"; break;
		case 2:	message = ci->gender == GENDER_MALE ? "had his eyeballs melted by" : "had her eyeballs melted by"; message2 = "'s flashbang grenade"; break;
			}
			break;
		case MOD_NAILGREN:
			r = rand() % 2;
			switch(r) {
		case 0:	message = "was shredded by"; message2 = "'s nail bomb"; break;
		case 1:	message = "was thrashed by"; message2 = "'s nail grenade"; break;
			}
			break;
		case MOD_CLUSTERGREN:
			r = rand() % 4;
			switch(r) {
		case 0:	message = ci->gender == GENDER_MALE ? "found himself on the wrong end of" : "found herself on the wrong end of"; message2 = "'s cluster bomb spam"; break;
		case 1:	message = "couldn't hide from"; message2 = "'s cluster bomb"; break;
		case 2:	message = "was spammed by"; message2 = "'s cluster bomb"; break;
		case 3:	message = "ate"; message2 = "'s cluster bomb shrapnel"; break;
			}
			break;
		case MOD_NAPALMGREN:
			r = rand() % 3;
			switch(r) {
		case 0:	message = "found"; message2 = "'s napalm grenade too hot to handle"; break;
		case 1:	message = "was immolated by"; message2 = "'s napalm"; break;
		case 2:	message = "was burned alive by"; message2 = "'s napalm"; break;
			}
			break;
		case MOD_GASGREN:
			r = rand() % 5;
			switch(r) {
		case 0:	message = "expired from an overdose of"; message2 = "'s hallucinogenic gas"; break;
		case 1:	message = "liked"; message2 = "'s pretty colors"; break;
		case 2:	message = "asphyxiated on"; message2 = "'s gas"; break;
		case 3:	message = "had visions right before dying from"; message2 = "'s hallucinogenic gas"; break;
		case 4:	message = "suffocated on"; message2 = "'s noxious gases"; break;
			}
			break;
		case MOD_PULSEGREN:
			r = rand() % 4;
			switch(r) {
		case 0:	message = "was crisped by"; message2 = "'s pulse grenade"; break;
		case 1:	message = "was pulsated by"; message2 = "'s pulse grenade"; break;
		case 2:	message = "was vibrated to death by"; message2 = "'s pulse grenade"; break;
		case 3: message = "felt"; message2 = "'s pulse"; break;
			}
			break;
		case MOD_CHARGE:
			r = rand() % 3;
			switch(r) {
		case 0:	message = "learned the true power of"; message2 = "'s HE charge"; break;
		case 1:	message = "crumbled into dust after being exposed to"; message2 = "'s HE charge"; break;
		case 2:	message = "was vaporized by"; message2 = "'s HE charge explosion"; break;
			}
			break;
		case MOD_AUTOSENTRY_BULLET:
			r = rand() % 2;
			switch(r) {
		case 0:	message = "felt the wrath of"; message2 = "'s autosentry"; break;
		case 1:	message = ci->gender == GENDER_MALE ? "was caught with his pants down by" : "was caught with her skirt up by"; message2 = "'s autosentry"; break;
			}
			break;
		case MOD_AUTOSENTRY_ROCKET:
			r = rand() % 2;
			switch(r) {
		case 0:	message = "swallowed a rocket from"; message2 = "'s autosentry"; break;
		case 1:	message = "was blown up by"; message2 = "'s autosentry rocket"; break;
			}
			break;
		case MOD_AUTOSENTRY_EXPLODE:
			r = rand() % 4;
			switch(r) {
		case 0:	message = "was exploded along with"; message2 = "'s autosentry"; break;
		case 1:	message = "was wrecked in"; message2 = "'s autosentry explosion"; break;
		case 2:	message = "was a casualty statistic in"; message2 = "'s autosentry mishap"; break;
		case 3:	message = "stood too close to"; message2 = "'s autosentry when it blew up"; break;
			}
			break;
		case MOD_SUPPLYSTATION_EXPLODE:
			r = rand() % 3;
			switch(r) {
		case 0:	message = "lost an argument with"; message2 = "'s supply station"; break;
		case 1:	message = "didn't know that"; message2 = "'s supply station didn't accept pennies"; break;
		case 2:	message = "had their credit card declined by"; message2 = "'s supply station"; break;
			}
			break;
		case MOD_SINGLESHOTGUN:
			r = rand() % 4;
			switch(r) {
		case 0:	message = "was gunned down by"; break;
		case 1:	message = "was able to avoid everything except"; message2 = "'s single-barreled shotgun"; break;
		case 2:	message = "was dishonored by"; message2 = "'s single-barreled shotgun"; break;
		case 3:	message = "is picking"; message2 = ci->gender == GENDER_MALE ? "'s buckshot out of his body" : "'s buckshot out of her body"; break;
			}
			break;
		case MOD_MINIGUN:
			r = rand() % 6;
			switch(r) {
		case 0:	message = "was turned into humanslaw by"; message2 = "'s minigun"; break;
		case 1:	message = "was floored by a hail of lead from"; message2 = "'s minigun"; break;
		case 2:	message = "was grated by"; message2 = "'s minigun"; break;
		case 3:	message = "was reduced to chunks by"; message2 = "'s minigun"; break;
		case 4:	message = "was transformed into a bloody pulp by"; message2 = "'s minigun"; break;
		case 5:	message = ci->gender == GENDER_MALE ? "was cut down in his prime with" : "was cut down in her prime with"; message2 = "'s minigun"; break;
			}
			break;
		case MOD_MIRROR:
			message = "attempted to frag";
			break;
		case MOD_GASEXPLOSION:
			r = rand() % 3;
			switch(r) {
		case 0:	message = "joined"; message2 = "'s barbecue"; break;
		case 1:	message = "was transformed into a tasty morsel by"; message2 = "'s barbecue"; break;
		case 2:	message = "took part in"; message2 = "'s combustion experiment"; break;
			}
			break;
		case MOD_CRUSHEDBYSENTRY:
			message = "learned just how heavy";
			message2 = "'s autosentry really is";
			break;
		case MOD_CRUSHEDBYSUPPLYSTATION:
			message = "learned just how heavy";
			message2 = "'s supply station really is";
			break;
		default:
			message = "was killed by";
			break;
		}
		if (message) {
			if(!cg_altObits.integer && !cg_filterObituaries.integer) {
				CG_Printf( BOX_PRINT_MODE_CHAT, "%s %s %s%s%s.\n", 
					targetName, message,
					(isally ? "^sally^7 " : ""),
					attackerName, message2);
			}
			if(cg_altObits.integer == 1 && !cg_filterObituaries.integer) {
				CG_Printf( BOX_PRINT_MODE_CONSOLE, "%s %s %s%s%s.\n", 
					targetName, message,
					(isally ? "^sally^7 " : ""),
					attackerName, message2);
			}
			if ( cg.matchLogFileHandle > 0 && cg.matchState <= MATCH_STATE_PLAYING) {
				CG_MatchLogAddLine(va("%s %s \"%s\" %s\"%s\"\n", 
					"kill",
					obit, 
					Info_ValueForKey( targetInfo, "n" ),
					isally ? "^sally^7 " : "",
					attackerInfo ? Info_ValueForKey( attackerInfo, "n" ) : "World"
				));
			}
			return;
		}
	}

	// we don't know what it was
	if(!cg_altObits.integer && !cg_filterObituaries.integer) {
		CG_Printf( BOX_PRINT_MODE_CHAT, "%s died.\n", targetName);
	}
}

//==========================================================================

/*
===============
CG_UseItem
===============
*/
static void CG_UseItem( centity_t *cent ) {
	clientInfo_t *ci;
	int			itemNum, clientNum;
	gitem_t		*item;
	entityState_t *es;

	es = &cent->currentState;
	
	itemNum = es->event - EV_USE_ITEM0;
	if ( itemNum < 0 || itemNum > HI_NUM_HOLDABLE ) {
		itemNum = 0;
	}

	// print a message if the local player
	if ( es->number == cg.snap->ps.clientNum ) {
		if ( !itemNum ) {
			CG_LowPriority_Printf( BOX_PRINT_MODE_CENTER, "No item to use" );
		} else {
			item = BG_FindItemForHoldable( itemNum );
			CG_LowPriority_Printf( BOX_PRINT_MODE_CENTER, "Use %s", item->pickup_name );
		}
	}

	switch ( itemNum ) {
	default:
	case HI_NONE:
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.useNothingSound );
		break;

	case HI_TELEPORTER:
		break;

	case HI_MEDKIT:
		clientNum = cent->currentState.clientNum;
		if ( clientNum >= 0 && clientNum < MAX_CLIENTS ) {
			ci = &cgs.clientinfo[ clientNum ];
			ci->medkitUsageTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.medkitSound );
		break;
	}

}

/*
===============
CG_Q3F_UseItemFailed
===============
*/
static void CG_Q3F_UseItemFailed( centity_t *cent ) {
	CG_LowPriority_Printf( BOX_PRINT_MODE_CENTER, "No item to use" );

	trap_S_StartSound (NULL, cent->currentState.number, CHAN_BODY, cgs.media.useNothingSound );
}

/*
================
CG_ItemPickup

A new item was picked up this frame
================
*/
static void CG_ItemPickup( int itemNum ) {
	cg.itemPickup = itemNum;
	cg.itemPickupTime = cg.time;
	cg.itemPickupBlendTime = cg.time;
	// see if it should be the grabbed weapon
/*	if ( bg_itemlist[itemNum].giType == IT_WEAPON ) {
		// select it immediately
		if ( cg_autoswitch.integer && bg_itemlist[itemNum].giTag != WP_NAILGUN ) {
			cg.weaponSelectTime = cg.time;
			cg.weaponSelect = bg_itemlist[itemNum].giTag;
		}
	}*/
}

/*
================
CG_Q3F_Vibrate

Called to cause 'vibration' on impacts
================
*/

void CG_Q3F_Vibrate( int val, vec3_t position )
{
	vec3_t dist;
	float magnitude, oldmagnitude;//, oldoffset;
	int time, contents;

	VectorCopy( cg.snap->ps.origin, dist );
	if( cg.snap->ps.groundEntityNum == ENTITYNUM_NONE )
	{
		contents = CG_PointContents( dist, 0 );
		if ( !(contents & MASK_WATER) )
			val /= 2;		// Half vibration if not on ground / in water
	}
	VectorSubtract( dist, position, dist );

	magnitude = 1.0f - VectorLength( dist ) / 1000.0;
	if( magnitude <= 0 )
		return;
	magnitude = cg_impactVibration.value * val * magnitude * magnitude;

	if( cg.vibrateTime )
	{
		// Adjust for current vibration

		time			= (float)(cg.time - cg.vibrateTime);
		if( time < 20 )
			time = 20;
		oldmagnitude	= cg.vibrateMagnitude / (time / 20);
	}
	else {
		oldmagnitude	= 0;
	}

	cg.vibrateMagnitude	= magnitude + oldmagnitude;
	if( cg.vibrateMagnitude < 0 )
		cg.vibrateMagnitude = 0;

	cg.vibrateOffset	= Q_flrand(0.0f, 1.0f) * 360;	// Always on the 'up' half
	cg.vibrateTime		= cg.time;
}

/*
================
CG_PainEvent

Also called by playerstate transition
================
*/
void CG_PainEvent( centity_t *cent, int health ) {
	char	*snd;

	// don't do more than two pain sounds a second
	if ( cg.time - cent->pe.painTime < 500 ) {
		return;
	}

	if ( health < 25 ) {
		snd = "*pain25_1.wav";
	} else if ( health < 50 ) {
		snd = "*pain50_1.wav";
	} else if ( health < 75 ) {
		snd = "*pain75_1.wav";
	} else {
		snd = "*pain100_1.wav";
	}
	trap_S_StartSound( NULL, cent->currentState.number, CHAN_VOICE, 
		CG_CustomSound( cent->currentState.number, snd ) );

	// save pain time for programitic twitch animation
	cent->pe.painTime = cg.time;
	cent->pe.painDirection ^= 1;
}

/*
================
CG_GurpEvent
================
*/
void CG_GurpEvent( centity_t *cent, int health ) {
	char	*snd;
	playerEntity_t *pe;
	static int gurpsound_idx;

	if( cg.predictedPlayerEntity.currentState.number == cent->currentState.number )
		pe = &cg.predictedPlayerEntity.pe;
	else
		pe = &cent->pe;

	// don't do more than two pain sounds a second
	if ( cg.time - pe->painTime < 500 ) {
		return;
	}

	/*if ( health < 25 ) {
		snd = "*drown25_1.wav";
	} else if ( health < 50 ) {
		snd = "*drown50_1.wav";
	} else if ( health < 75 ) {
		snd = "*drown75_1.wav";
	} else {
		snd = "*drown100_1.wav";
	}*/
	switch ( gurpsound_idx ) {
	case 0:	snd = "*drown1.wav";	break;
	case 1:	snd = "*drown2.wav";	break;
	case 2:	snd = "*drown3.wav";	break;
	default:
	case 3:	snd = "*drown4.wav";	break;
	}

	trap_S_StartSound( NULL, cent->currentState.number, CHAN_VOICE, 
		CG_CustomSound( cent->currentState.number, snd ) );

	// globally cycle through the different gurn sounds
	gurpsound_idx = ( gurpsound_idx + 1 ) % 4;

	// save pain time for programitic twitch animation
	pe->painTime = cg.time;
	pe->painDirection ^= 1;
}

/*
==========================
CG_DebugBullet
==========================
*/
static void CG_DebugBox( entityState_t *es ) {
	localEntity_t	*le;
	
	le = CG_AllocLocalEntity( 10000 );
	le->leType = LE_DEBUG_BOX;
	le->lifeRate *= 10000/1000;

	VectorCopy( es->pos.trBase ,le->pos.trBase);
	VectorCopy( es->apos.trBase ,le->angles.trBase);
	VectorCopy( es->apos.trDelta, le->angles.trDelta );

	le->color[0] = (float)((es->constantLight >> 0  ) & 255) / 255;
	le->color[1] = (float)((es->constantLight >> 8  ) & 255) / 255;
	le->color[2] = (float)((es->constantLight >> 16 ) & 255) / 255;
	le->color[3] = (float)((es->constantLight >> 24 ) & 255) / 255;
}

static void CG_DebugBullet( entityState_t *es ) {
	localEntity_t	*le;
	refEntity_t		*re;

	le = CG_AllocLocalEntity( 10000 );
	re = &le->refEntity;

	le->leType = LE_FADE_RGB;
	le->lifeRate *= 10000/1000;

	if(!cgs.media.railCoreShader) {
		cgs.media.railCoreShader = trap_R_RegisterShader( "railCore" );
	}

	re->reType = RT_RAIL_CORE;
	re->customShader = cgs.media.railCoreShader;
	
	VectorCopy( es->pos.trBase, re->origin );
	VectorCopy( es->pos.trDelta, re->oldorigin );

	le->color[0] = (float)((es->constantLight >> 0  ) & 255) / 255;
	le->color[1] = (float)((es->constantLight >> 8  ) & 255) / 255;
	le->color[2] = (float)((es->constantLight >> 16 ) & 255) / 255;
	le->color[3] = (float)((es->constantLight >> 24 ) & 255) / 255;

	AxisClear( re->axis );
}



void CG_BotDebugLine(vec3_t start, vec3_t end, vec3_t color)
{
	localEntity_t	*le;
	refEntity_t		*re;
	le = CG_AllocLocalEntity(2000);
	re = &le->refEntity;
	le->leType = LE_CONST_RGB;
	if(!cgs.media.railCoreShader)
		cgs.media.railCoreShader = trap_R_RegisterShader("railCore");

	re->reType = RT_RAIL_CORE;
	re->customShader = cgs.media.railCoreShader;

	VectorCopy(start, re->origin);
	VectorCopy(end, re->oldorigin);

	le->color[0] = color[0];
	le->color[1] = color[1];
	le->color[2] = color[2];
	le->color[3] = 1.0f;

	AxisClear( re->axis );
}

void CG_BotDebugRadius(vec3_t pos, vec3_t info, vec3_t color)
{
	localEntity_t	*le;
	refEntity_t		*re;
	float fRadius = info[0];
	int iNumSteps = (int)info[1];
	vec3_t start, end;
	float fStepSize = 180.0f / (float)iNumSteps;
	int i;

	if(!cgs.media.railCoreShader)
		cgs.media.railCoreShader = trap_R_RegisterShader("railCore");

	VectorCopy(pos, start);
	VectorCopy(pos, end);
	start[1] += fRadius;
	end[1] -= fRadius;

	for(i = 0; i < iNumSteps; ++i)
	{
		le = CG_AllocLocalEntity(2000);
		re = &le->refEntity;
		le->leType = LE_CONST_RGB;		
		re->reType = RT_RAIL_CORE;
		re->customShader = cgs.media.railCoreShader;

		VectorCopy(start, re->origin);
		VectorCopy(end, re->oldorigin);

		le->color[0] = color[0];
		le->color[1] = color[1];
		le->color[2] = color[2];
		le->color[3] = 1.0f;

		AxisClear( re->axis );
		RotatePointAroundVertex(start, 0.0f, 0.0f, fStepSize, pos);
		RotatePointAroundVertex(end, 0.0f, 0.0f, fStepSize, pos);
	}
}

/*
================
CG_BurnEvent
================
*/
void CG_BurnEvent( centity_t *cent, int health ) {
	char	*snd;
	playerEntity_t *pe;
	static int burnsound_idx;

	if( cg.predictedPlayerEntity.currentState.number == cent->currentState.number )
		pe = &cg.predictedPlayerEntity.pe;
	else
		pe = &cent->pe;

	// don't do more than two pain sounds a second
	if ( cg.time - pe->painTime < 500 ) {
		return;
	}

	/*if ( health < 25 ) {
		snd = "*burn25_1.wav";
	} else if ( health < 50 ) {
		snd = "*burn50_1.wav";
	} else if ( health < 75 ) {
		snd = "*burn75_1.wav";
	} else {
		snd = "*burn100_1.wav";
	}*/
	switch ( burnsound_idx ) {
	case 0:	snd = "*burn1.wav";	break;
	case 1:	snd = "*burn2.wav";	break;
	case 2:	snd = "*burn3.wav";	break;
	default:
	case 3:	snd = "*burn4.wav";	break;
	}

	trap_S_StartSound( NULL, cent->currentState.number, CHAN_VOICE, 
		CG_CustomSound( cent->currentState.number, snd ) );

	// globally cycle through the different burn sounds
	burnsound_idx = ( burnsound_idx + 1 ) % 4;

	// save pain time for programitic twitch animation
	pe->painTime = cg.time;
	pe->painDirection ^= 1;
}

static void CG_Q3F_WaterSplash( centity_t *cent ) {
	trace_t			tr;
	vec3_t			trace_start;

	trap_S_StartSound (NULL, cent->currentState.number, CHAN_ITEM, cgs.media.watrInSound );

	// to get the origin, go up 64 units and trace down for a waterplane
	VectorCopy( cent->lerpOrigin, trace_start );
	trace_start[2] += 64;
	CG_Trace( &tr, trace_start, NULL, NULL, cent->lerpOrigin, ENTITYNUM_NONE, MASK_WATER );

	Spirit_RunScript( cgs.spirit.watersplash, tr.endpos, tr.endpos, axisDefault, 0 );
}

/*
==============
CG_EntityEvent

An entity has an event value
also called by CG_CheckPlayerstateEvents
==============
*/

#define	DEBUGNAME(x) if(cg_debugEvents.integer){CG_Printf(BOX_PRINT_MODE_CHAT, x"\n");}
void CG_EntityEvent( centity_t *cent, vec3_t position ) {
	entityState_t	*es;
	int				event, magnitude;
	vec3_t			dir;
	const char		*s;
	int				clientNum;
	//clientInfo_t	*ci;
	es = &cent->currentState;
	event = es->event;

	if ( cg_debugEvents.integer ) {
		CG_Printf( BOX_PRINT_MODE_CHAT, "ent:%3i  event:%3i ", es->number, event );
	}

	if ( !event ) {
		DEBUGNAME("ZEROEVENT");
		return;
	}

	clientNum = es->clientNum;
	if ( clientNum < 0 || clientNum >= MAX_CLIENTS ) {
		clientNum = 0;
	}
	//ci = &cgs.clientinfo[ clientNum ];


	switch ( event ) {
	//
	// movement generated events
	//
	case EV_FOOTSTEP:
		DEBUGNAME("EV_FOOTSTEP");
		if (cg_footsteps.integer && es->eventParm < 2 ) {
			if( es->eFlags & EF_Q3F_STEPMASK )
			{
				// Ensure spies have appropriate footstep sounds
				if( (es->eFlags & EF_Q3F_STEPMASK) == EF_Q3F_FOOTSTEPS )
					trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.footsteps[ FOOTSTEP_NORMAL ][rand()&3] );
				if( (es->eFlags & EF_Q3F_STEPMASK) == EF_Q3F_METALSTEPS )
					trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.footsteps[ FOOTSTEP_METAL ][rand()&3] );
			}
			else // No flags, we assume it's the default footstep sound
				trap_S_StartSound (NULL, es->number, CHAN_BODY, cgs.media.footsteps[ FOOTSTEP_NORMAL ][rand()&3] );
		}
		break;
	case EV_FOOTSTEP_METAL:
		DEBUGNAME("EV_FOOTSTEP_METAL");
		if (cg_footsteps.integer && es->eventParm < 2 ) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_METAL ][rand()&3] );
		}
		break;
	case EV_FOOTSPLASH:
		DEBUGNAME("EV_FOOTSPLASH");
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_WAVES);		
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_FOOTWADE:
		DEBUGNAME("EV_FOOTWADE");
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_WAVES);		
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;
	case EV_SWIM:
		DEBUGNAME("EV_SWIM");
		if (cg_footsteps.integer) {
			trap_S_StartSound (NULL, es->number, CHAN_BODY, 
				cgs.media.footsteps[ FOOTSTEP_SPLASH ][rand()&3] );
		}
		break;


	case EV_FALL_SHORT:
		DEBUGNAME("EV_FALL_SHORT");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.landSound );
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -8;
			cg.landTime = cg.time;
		}
		break;
	case EV_FALL_D31:
	case EV_FALL_D29:
	case EV_FALL_D27:
	case EV_FALL_D25:
	case EV_FALL_D23:
	case EV_FALL_D21:
	case EV_FALL_D19:
	case EV_FALL_D17:
	case EV_FALL_D15:
	case EV_FALL_D13:
	case EV_FALL_D11:
		DEBUGNAME("EV_FALL_D##");
		if( !(es->eFlags & EF_DEAD) )	// Golliwog: Don't play pain sound if they're dead.
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*fall1.wav" ) );
		cent->pe.painTime = cg.time;	// don't play a pain sound right after this
		if ( clientNum == cg.predictedPlayerState.clientNum ) {
			// smooth landing z changes
			cg.landChange = -24;
			cg.landTime = cg.time;
		}
		break;

	case EV_STEP_4:
	case EV_STEP_8:
	case EV_STEP_12:
	case EV_STEP_16:		// smooth out step up transitions
		DEBUGNAME("EV_STEP");
	{
		float	oldStep;
		int		delta;
		int		step;

		if ( clientNum != cg.predictedPlayerState.clientNum ) {
			break;
		}
		// if we are interpolating, we don't need to smooth steps
		if ( cg.demoPlayback || (cg.snap->ps.pm_flags & PMF_FOLLOW) || (cg.snap->ps.pm_flags & PMF_CHASE) ||
			cg_nopredict.integer || cgs.synchronousClients ) {
			break;
		}
		// check for stepping up before a previous step is completed
		delta = cg.time - cg.stepTime;
		if (delta < STEP_TIME) {
			oldStep = cg.stepChange * (STEP_TIME - delta) / STEP_TIME;
		} else {
			oldStep = 0;
		}

		// add this amount
		step = 4 * (event - EV_STEP_4 + 1 );
		cg.stepChange = oldStep + step;
		if ( cg.stepChange > MAX_STEP_CHANGE ) {
			cg.stepChange = MAX_STEP_CHANGE;
		}
		cg.stepTime = cg.time;
		break;
	}

	case EV_JUMP_PAD:
		DEBUGNAME("EV_JUMP_PAD");
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_JUMPPAD);
//		CG_Printf( "EV_JUMP_PAD w/effect #%i\n", es->eventParm );
//		Canabis, this just looks fucked, crappy smoke sprite on the jumppad
//		VectorSet4(color, 1, 1, 1, 0.5f );
//		CG_SpawnSmokeSprite( cent->lerpOrigin, 1000, color, 32, 40 );
		// boing sound at origin, jump sound on player
		trap_S_StartSound ( cent->lerpOrigin, -1, CHAN_VOICE, cgs.media.jumpPadSound );
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
		break;

	case EV_JUMP:
		DEBUGNAME("EV_JUMP");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*jump1.wav" ) );
		break;
	case EV_TAUNT:
		DEBUGNAME("EV_TAUNT");
		trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, "*taunt.wav" ) );
		break;
	case EV_WATER_TOUCH:
		DEBUGNAME("EV_WATER_TOUCH");
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_WAVES);		
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrInSound );
		break;
	case EV_WATER_LEAVE:
		DEBUGNAME("EV_WATER_LEAVE");
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_WAVES);		
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrOutSound );
		break;
	case EV_WATER_UNDER:
		DEBUGNAME("EV_WATER_UNDER");
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_WAVES);		
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.watrUnSound );
		if(cg.clientNum == es->number) {
			cg.waterundertime = cg.time + HOLDBREATHTIME;
		}
		break;
	case EV_WATER_CLEAR:
		DEBUGNAME("EV_WATER_CLEAR");
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_WAVES);		
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, CG_CustomSound( es->number, "*gasp.wav" ) );
		break;

	case EV_ITEM_PICKUP:
		DEBUGNAME("EV_ITEM_PICKUP");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];

			// powerups and team items will have a separate global sound, this one
			// will be played at prediction time
			if ( item->giType == IT_POWERUP /*|| item->giType == IT_TEAM*/) {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	cgs.media.n_healthSound );
			} else if (item->giType == IT_PERSISTANT_POWERUP) {
			} else {
				trap_S_StartSound (NULL, es->number, CHAN_AUTO,	trap_S_RegisterSound( item->pickup_sound, qfalse ) );
			}
			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}
		}
		break;

	case EV_GLOBAL_ITEM_PICKUP:
		DEBUGNAME("EV_GLOBAL_ITEM_PICKUP");
		{
			gitem_t	*item;
			int		index;

			index = es->eventParm;		// player predicted

			if ( index < 1 || index >= bg_numItems ) {
				break;
			}
			item = &bg_itemlist[ index ];
			// powerup pickups are global
			if( item->pickup_sound ) {
				trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, trap_S_RegisterSound( item->pickup_sound, qfalse ) );
			}

			// show icon and name on status bar
			if ( es->number == cg.snap->ps.clientNum ) {
				CG_ItemPickup( index );
			}
		}
		break;

	//
	// weapon events
	//
	case EV_NOAMMO:
		DEBUGNAME("EV_NOAMMO");
//		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.noAmmoSound );
		if ( es->number == cg.snap->ps.clientNum ) {
			CG_OutOfAmmoChange();
//			cg.shudderStart = 0;
		}
		break;
	case EV_ETF_DISCARD_AMMO:
		DEBUGNAME("EV_ETF_DISCARD_AMMO");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.discardSound );
		break;
	case EV_CHANGE_WEAPON:
		DEBUGNAME("EV_CHANGE_WEAPON");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.selectSound );

		// minigun spinning
		if( cent->pe.barrelSpinning && cent->currentState.weapon == WP_MINIGUN ) {
			trap_S_StartSound( cent->lerpOrigin, cent->currentState.number, CHAN_WEAPON, cgs.media.sfx_minigun_winddown );
			cent->pe.minigunTime = cg.time;
		} else {
			cent->pe.barrelSpinning = 0;
		}
		cent->pe.barrelAngle = 0;
		cent->pe.minigunTime = 0;
//		cg.shudderStart = 0;
		break;
	case EV_COCK_WEAPON:
		DEBUGNAME("EV_COCK_WEAPON");
		{
			int		agentclass, j;
			int		weaponNum;

			agentclass = 0;
			weaponNum = cent->currentState.weapon;

			// Search for agentdata
			for(j = 0; j < MAX_GENTITIES; j++)
			{
				if(cg_entities[j].currentState.eType == ET_Q3F_AGENTDATA &&	// Type of ent
					cg_entities[j].currentState.otherEntityNum == es->number)	// Ent owner
					break;		// Found a match
			}

			if(j < MAX_GENTITIES && cg_entities[j].currentState.torsoAnim)
			{
				agentclass = cg_entities[j].currentState.torsoAnim;
				weaponNum = BG_Q3F_GetRemappedWeaponFromWeaponNum(Q3F_CLASS_AGENT, agentclass, weaponNum);
			}
			else
			{
				agentclass = 0;
				weaponNum = cent->currentState.weapon;
			}


		if ( weaponNum == WP_GRENADE_LAUNCHER ||
			 weaponNum == WP_PIPELAUNCHER ||
			 weaponNum == WP_ROCKET_LAUNCHER ||
			 weaponNum == WP_NAPALMCANNON )
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.cockGrenSound );
		else
			trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.cockSound );
		}
		break;
	case EV_FIRE_WEAPON:
		DEBUGNAME("EV_FIRE_WEAPON");
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_GUNFIRE);
		CG_FireWeapon( cent );
		break;
		// JT
	case EV_WEAPON_AIM:
		DEBUGNAME("EV_WEAPON_AIM");
		break;
	case EV_WEAPON_START:
		DEBUGNAME("EV_WEAPON_START");
		/*
		if(es->number == cg.snap->ps.clientNum) {
			cg.shudderStart = cg.time;
		}*/

		break;
	case EV_WEAPON_END:
		DEBUGNAME("EV_WEAPON_END");
		/*
		if(es->number == cg.snap->ps.clientNum)
			cg.shudderStart = 0;
			*/
		break;

		// JT
	case EV_USE_ITEM0:
		DEBUGNAME("EV_USE_ITEM0");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM1:
		DEBUGNAME("EV_USE_ITEM1");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM2:
		DEBUGNAME("EV_USE_ITEM2");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM3:
		DEBUGNAME("EV_USE_ITEM3");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM4:
		DEBUGNAME("EV_USE_ITEM4");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM5:
		DEBUGNAME("EV_USE_ITEM5");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM6:
		DEBUGNAME("EV_USE_ITEM6");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM7:
		DEBUGNAME("EV_USE_ITEM7");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM8:
		DEBUGNAME("EV_USE_ITEM8");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM9:
		DEBUGNAME("EV_USE_ITEM9");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM10:
		DEBUGNAME("EV_USE_ITEM10");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM11:
		DEBUGNAME("EV_USE_ITEM11");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM12:
		DEBUGNAME("EV_USE_ITEM12");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM13:
		DEBUGNAME("EV_USE_ITEM13");
		CG_UseItem( cent );
		break;
	case EV_USE_ITEM14:
		DEBUGNAME("EV_USE_ITEM14");
		CG_UseItem( cent );
		break;
	case EV_DISEASE:
		DEBUGNAME("EV_DISEASE");
		break;
	case EV_ETF_USE_ITEM_FAILED:
		DEBUGNAME("EV_ETF_USE_ITEM_FAILED");
		CG_Q3F_UseItemFailed( cent );
		break;
	
	//=================================================================

	//
	// other events
	//
	case EV_PLAYER_TELEPORT_IN:
		DEBUGNAME("EV_PLAYER_TELEPORT_IN");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleInSound );
		CG_SpawnEffect( position);
		break;

	case EV_PLAYER_TELEPORT_OUT:
		DEBUGNAME("EV_PLAYER_TELEPORT_OUT");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.teleOutSound );
		CG_SpawnEffect(  position);
		break;

	case EV_ITEM_POP:
		DEBUGNAME("EV_ITEM_POP");
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;
	case EV_ITEM_RESPAWN:
		DEBUGNAME("EV_ITEM_RESPAWN");
		cent->miscTime = cg.time;	// scale up from this
		trap_S_StartSound (NULL, es->number, CHAN_AUTO, cgs.media.respawnSound );
		break;

	case EV_GRENADE_BOUNCE:
		DEBUGNAME("EV_GRENADE_BOUNCE");
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_GRENBOUNCE);
		// Golliwog: Changed so the origin is accurate (instead of global sounds).
		if ( rand() & 1 ) {
			trap_S_StartSound( es->pos.trBase, es->number, CHAN_AUTO, cgs.media.hgrenb1aSound );
		} else {
			trap_S_StartSound( es->pos.trBase, es->number, CHAN_AUTO, cgs.media.hgrenb2aSound );
		}
		// Golliwog.
		break;

	case EV_SCOREPLUM:
		DEBUGNAME("EV_SCOREPLUM");
		CG_ScorePlum( cent->currentState.otherEntityNum, cent->lerpOrigin, cent->currentState.time );
		break;

	//
	// missile impacts
	//
	case EV_MISSILE_HIT:
		DEBUGNAME("EV_MISSILE_HIT");
		ByteToDir( es->eventParm, dir );
		if(es->powerups & (1 << PW_QUAD))
			es->weapon |= 16;
		CG_MissileHitPlayer( es->weapon, position, dir, es->otherEntityNum2, es->otherEntityNum );
		switch( es->weapon & 15 )
		{
			case WP_NAILGUN:
			case WP_SUPERNAILGUN:		magnitude = 1;		break;
			case WP_RAILGUN:			magnitude = 10;		break;
			case WP_NAPALMCANNON:		magnitude = 40;		break;
			case WP_GRENADE_LAUNCHER:
			case WP_PIPELAUNCHER:
			case WP_ROCKET_LAUNCHER:	magnitude = 70;	break;
			default:					magnitude = 0;
		}
		CG_Q3F_Vibrate( magnitude, position );
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_EXPLOSION);
		break;

	case EV_MISSILE_MISS:
		DEBUGNAME("EV_MISSILE_MISS");
		ByteToDir( es->eventParm, dir );
		if(es->powerups & (1 << PW_QUAD))
			es->weapon |= 16;
		CG_MissileHitWall( es->weapon, es->otherEntityNum2, position, dir, IMPACTSOUND_DEFAULT );
		switch( es->weapon & 15 )
		{
			case WP_NAILGUN:
			case WP_SUPERNAILGUN:		magnitude = 1;		break;
			case WP_RAILGUN:			magnitude = 20;		break;
			case WP_NAPALMCANNON:		magnitude = 60;		break;
			case WP_GRENADE_LAUNCHER:
			case WP_PIPELAUNCHER:
			case WP_ROCKET_LAUNCHER:	magnitude = 100;	break;
			default:					magnitude = 0;
		}
		CG_Q3F_Vibrate( magnitude, position );
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_EXPLOSION);
		break;

	case EV_MISSILE_MISS_METAL:
		DEBUGNAME("EV_MISSILE_MISS_METAL");
		ByteToDir( es->eventParm, dir );
		if(es->powerups & (1 << PW_QUAD))
			es->weapon |= 16;
		switch( es->weapon & 15 )
		{
			case WP_NAILGUN:
			case WP_SUPERNAILGUN:		magnitude = 1;		break;
			case WP_RAILGUN:			magnitude = 20;		break;
			case WP_NAPALMCANNON:		magnitude = 60;		break;
			case WP_GRENADE_LAUNCHER:
			case WP_PIPELAUNCHER:
			case WP_ROCKET_LAUNCHER:	magnitude = 100;	break;
			default:					magnitude = 0;
		}
		CG_Q3F_Vibrate( magnitude, position );
		CG_MissileHitWall( es->weapon, es->otherEntityNum2, position, dir, IMPACTSOUND_METAL );
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_EXPLOSION);
		break;

	case EV_BULLET_HIT_WALL:
		DEBUGNAME("EV_BULLET_HIT_WALL");
		ByteToDir( es->eventParm, dir );
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD );
		break;

	case EV_BULLET_HIT_FLESH:
		DEBUGNAME("EV_BULLET_HIT_FLESH");
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qtrue, es->eventParm );
		break;

	case EV_SNIPER_HIT_WALL:
		DEBUGNAME("EV_SNIPER_HIT_WALL");
		if ( es->otherEntityNum == cg.predictedPlayerState.clientNum && cg_predictWeapons.integer )
			break;
		ByteToDir( es->eventParm, dir );
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qfalse, ENTITYNUM_WORLD );
		CG_Q3F_Vibrate( 5, position );
		break;
	case EV_SNIPER_HIT_FLESH:
		DEBUGNAME("EV_SNIPER_HIT_FLESH");
		if ( es->otherEntityNum == cg.predictedPlayerState.clientNum && cg_predictWeapons.integer )
			break;
		CG_Q3F_Vibrate( 2, position );
		CG_Bullet( es->pos.trBase, es->otherEntityNum, dir, qtrue, es->eventParm );
		break;
	case EV_SHOTGUN:
		DEBUGNAME("EV_SHOTGUN");
		if ( es->otherEntityNum == cg.predictedPlayerState.clientNum && cg_predictWeapons.integer )
			break;
		CG_ShotgunPattern( es->pos.trBase, es->origin2, es->otherEntityNum, es->eventParm );
		break;
	case EV_SINGLESHOTGUN:
		DEBUGNAME("EV_SINGLESHOTGUN");
		if ( es->otherEntityNum == cg.predictedPlayerState.clientNum && cg_predictWeapons.integer )
			break;		
		CG_SingleShotgunPattern( es->pos.trBase, es->origin2, es->otherEntityNum, es->eventParm );
		break;
	case EV_MINIGUN:
		DEBUGNAME("EV_MINIGUN");
		if ( es->otherEntityNum == cg.predictedPlayerState.clientNum) {
			cg.minigunLast = cg.time;
			cg.minigunHeat = es->frame;
			if (cg.minigunHeat < 1 )
				cg.minigunHeat = 1;	
			if (cg_predictWeapons.integer )
				break;
		}
		CG_MinigunPattern( es->pos.trBase, es->origin2, es->otherEntityNum, es->eventParm, es->frame );
		break;
	case EV_GENERAL_SOUND:
		DEBUGNAME("EV_GENERAL_SOUND");
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			if( s && *s )
				trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, s ) );
		}
		break;

	case EV_GLOBAL_SOUND:	// play from the player's head so it never diminishes
		DEBUGNAME("EV_GLOBAL_SOUND");
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			if( s && *s )
				trap_S_StartSound (NULL, cg.snap->ps.clientNum, CHAN_AUTO, CG_CustomSound( es->number, s ) );
		}
		break;

	case EV_GLOBAL_TEAM_SOUND:	// play from the player's head so it never diminishes
		{
			DEBUGNAME("EV_GLOBAL_TEAM_SOUND");
			break;
		}


	case EV_PAIN:
		// local player sounds are triggered in CG_CheckLocalSounds,
		// so ignore events on the player
		DEBUGNAME("EV_PAIN");
		if ( cent->currentState.number != cg.snap->ps.clientNum ) {
			CG_PainEvent( cent, es->eventParm );
		}
		break;

	case EV_DEATH1:
	case EV_DEATH2:
	case EV_DEATH3:
		DEBUGNAME("EV_DEATHx");
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, 
				CG_CustomSound( es->number, va("*death%i.wav", event - EV_DEATH1 + 1) ) );
		break;

	case EV_GURP:
		// gurp overrides painsounds
		DEBUGNAME("EV_GURP");
		CG_GurpEvent( cent, es->eventParm );
		break;

	case EV_DROWN:
		DEBUGNAME("EV_DROWN");
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, 
				CG_CustomSound( es->number, "*drown.wav" ) );
		break;

	case EV_BURN:
		// burn overrides painsounds
		DEBUGNAME("EV_BURN");
		CG_BurnEvent( cent, es->eventParm );
		break;

	case EV_BURNTODEATH:
		DEBUGNAME("EV_BURNTODEATH");
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, 
				CG_CustomSound( es->number, "*burn.wav" ) );

		break;

	case EV_OBITUARY:
		DEBUGNAME("EV_OBITUARY");
		CG_Obituary( es, qfalse );
		break;

	case EV_ALLYOBITUARY:
		DEBUGNAME("EV_ALLYBITUARY");
		CG_Obituary( es, qtrue );
		break;

	//
	// powerup events
	//
	case EV_POWERUP_QUAD:
		DEBUGNAME("EV_POWERUP_QUAD");
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_QUAD;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.quadSound );
		break;
	case EV_POWERUP_BATTLESUIT:
		DEBUGNAME("EV_POWERUP_BATTLESUIT");
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_BATTLESUIT;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.protectSound );
		break;
	case EV_POWERUP_PENTAGRAM:
		DEBUGNAME("EV_POWERUP_PENTAGRAM");
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_PENTAGRAM;
			cg.powerupTime = cg.time;
		}
		//trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.protectEvilSound );
		break;
	case EV_POWERUP_REGEN:
		DEBUGNAME("EV_POWERUP_REGEN");
		if ( es->number == cg.snap->ps.clientNum ) {
			cg.powerupActive = PW_REGEN;
			cg.powerupTime = cg.time;
		}
		trap_S_StartSound (NULL, es->number, CHAN_ITEM, cgs.media.regenSound );
		break;

	case EV_GIB_PLAYER:
		trap_S_StartSound( NULL, es->number, CHAN_BODY, cgs.media.gibSound );
		CG_GibPlayer( cent->lerpOrigin );
		break;

	case EV_STOPLOOPINGSOUND:
		DEBUGNAME("EV_STOPLOOPINGSOUND");
		es->loopSound = 0;
		break;

	// Golliwog: Sentry noises
	case EV_SENTRY_IDLESTART:
		DEBUGNAME("EV_SENTRY_IDLESTART");
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_SENTRY);
		trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.sentryStartSound );
		break;
	case EV_SENTRY_IDLESTOP:
		DEBUGNAME("EV_SENTRY_IDLESTOP");
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_SENTRY);
		trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.sentryStopSound );
		break;
	case EV_SENTRY_SPINUP:
		DEBUGNAME("EV_SENTRY_SPINUP");
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_SENTRY);
		trap_S_StartSound( NULL, es->number, CHAN_AUTO, cgs.media.sentrySpinupSound );
		break;
	case EV_SENTRY_EXPLOSION:
		DEBUGNAME( "EV_SENTRY_EXPLOSION" );
		CG_Q3F_Sentry_Explode( cent );
		trap_S_StartSound( cent->lerpOrigin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.sentryExplodeSound );
		CG_Q3F_Vibrate( 100, es->pos.trBase );
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_EXPLOSION);
		break;
	case EV_SENTRY_BUILD:
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_BUILD);
		trap_S_StartSound( NULL, cent->currentState.number, CHAN_AUTO, cgs.media.sentryBuildSound );
		break;
	case EV_SUPPLY_BUILD:
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_BUILD);
		trap_S_StartSound( NULL, cent->currentState.number, CHAN_AUTO, cgs.media.supplyBuildSound );
		break;

	case EV_PLACE_BUILDING:
		break;


	// Golliwog: The old classic
	case EV_MUZZLEFLASH:
		DEBUGNAME( "EV_MUZZLEFLASH" );
		trap_R_AddLightToScene( cent->lerpOrigin, (es->eventParm ? es->eventParm : 200), 1.f, 1.f, 1.f, 0.3f, 0, 0 );
		break;

	case EV_ETF_GRENADE_EXPLOSION:
		DEBUGNAME( "EV_ETF_GRENADE_EXPLOSION" );
		switch (es->eventParm) {
		case ETF_GRENDADE_EXPLOSION_CONCUSSION:
			Spirit_RunScript( cgs.spirit.explosion_concussion, es->pos.trBase, es->pos.trBase, axisDefault, 0 ); 
			break;
		case ETF_GRENDADE_EXPLOSION_FLASH:
			Spirit_RunScript( cgs.spirit.explosion_flash, es->pos.trBase, es->pos.trBase, axisDefault, 0 ); 
			break;
		default:
			if ( cg_no3DExplosions.value  ) {
				Spirit_RunScript( cgs.spirit.explosion_simple, es->pos.trBase, es->pos.trBase, axisDefault, 0 ); 
			} else {
				if(es->powerups & (1 << PW_QUAD)) {
					int team;
					int parent = es->otherEntityNum;

					if(parent >= 0 && parent < MAX_CLIENTS)
						team = cgs.clientinfo[parent].team;
					else
						team = Q3F_TEAM_FREE;
					Spirit_SetCustomShader( CG_Q3F_ShaderForQuad( team ) );
					Spirit_SetCustomColor( CG_Q3F_LightForQuad( team ) );
					Spirit_RunScript( cgs.spirit.explosion_quad, es->pos.trBase, es->pos.trBase, axisDefault,0 );
				}
				else
					Spirit_RunScript( cgs.spirit.explosion_normal, es->pos.trBase, es->pos.trBase, axisDefault, 0 ); 
			}
		}
		/* Do mark if damage is high enough */
		if( es->angles[1] >= 20 )
			CG_ExplosionMark( cent->lerpOrigin, es->angles[1] * 0.5, colorWhite );

		CG_Q3F_Vibrate( es->angles[1], es->pos.trBase );
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_EXPLOSION);
		break;
	case EV_ETF_EXPLOSION:
		DEBUGNAME( "EV_ETF_EXPLOSION" );
		if( es->angles[0] )
			CG_ExplosionMark( es->origin, es->angles[0], colorWhite );
		if( es->angles[1] )
			CG_Q3F_Vibrate( es->angles[1], es->origin );
		Spirit_RunScript( cgs.gameSpiritScript[es->legsAnim],es->origin, es->origin, axisDefault,
			(int)cent );
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_EXPLOSION);
		break;

	case EV_ETF_SUPPLYSTATION_EXPLOSION:
		DEBUGNAME( "EV_ETF_SUPPLYSTATION_EXPLOSION" );
		CG_Q3F_Supplystation_Explode( cent );
		trap_S_StartSound( cent->lerpOrigin, ENTITYNUM_WORLD, CHAN_AUTO, cgs.media.supplyExplodeSound );
		CG_Q3F_Vibrate( 100, es->pos.trBase );
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_EXPLOSION);
		break;

	case EV_ETF_GASEXPLOSION:
		DEBUGNAME( "EV_ETF_GASEXPLOSION" );
		CG_BurnGasSprites( cent );
		cent->miscTime = cg.time+1000;		//Disable it from adding new sprites
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_EXPLOSION);
		break;

	case EV_ETF_WATERSPLASH:
		DEBUGNAME( "EV_ETF_WATERSPLASH" );
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_WAVES);		
		CG_Q3F_WaterSplash( cent );
		break;

	case EV_DEBUG_LINE:
		DEBUGNAME("EV_DEBUG_LINE");
		CG_Beam( cent );
		break;

	case EV_DOOR:
		CG_Q3F_AddAlertIcon(cg.snap->ps.origin, Q3F_ALERT_DOOR);

		// RR2DO2: save on event data
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			if( s && *s )
				trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, s ) );
		}
		break;

	case EV_LIFT:
		CG_Q3F_AddAlertIcon(cg.snap->ps.origin, Q3F_ALERT_LIFT);

		// RR2DO2: save on event data
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			if( s && *s )
				trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, s ) );
		}
		break;

	case EV_VISUAL_TRIGGER:
		CG_Q3F_AddAlertIcon(cg.snap->ps.origin, Q3F_ALERT_TRIGGER);

		// RR2DO2: save on event data
		if ( cgs.gameSounds[ es->eventParm ] ) {
			trap_S_StartSound (NULL, es->number, CHAN_VOICE, cgs.gameSounds[ es->eventParm ] );
		} else {
			s = CG_ConfigString( CS_SOUNDS + es->eventParm );
			if( s && *s )
				trap_S_StartSound (NULL, es->number, CHAN_VOICE, CG_CustomSound( es->number, s ) );
		}
		break;

	case EV_VISUAL_NAILFIRE:
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_NAILGREN);
		break;

	case EV_DEBUG_DATA:
		DEBUGNAME("EV_DEBUG_DATA");
		switch (es->eventParm) {
		case 0:
			CG_DebugBullet( es );
			break;
		case 1:
			CG_DebugBox( es );
			break;
		}
		break;

	case EV_BOT_DEBUG_LINE:
		DEBUGNAME("EV_BOT_DEBUG_LINE");
		CG_BotDebugLine(es->origin2, es->pos.trBase, es->angles2);
		break;
	case EV_BOT_DEBUG_RADIUS:
		DEBUGNAME("EV_BOT_DEBUG_RADIUS");
		CG_BotDebugRadius(es->pos.trBase, es->origin2, es->angles2);
		break;
	case EV_HE_BEEP:
		DEBUGNAME("EV_HE_BEEP");
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, cgs.media.chargebeep1 );
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_HE_SET);
		break;

	case EV_HE_BEEP2:
		DEBUGNAME("EV_HE_BEEP2");
		trap_S_StartSound( NULL, es->number, CHAN_VOICE, cgs.media.chargebeep2 );
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_HE_SET);
		break;

	case EV_HE_EXPLODE:
		DEBUGNAME("EV_HE_EXPLODE");
		Spirit_RunScript( cgs.spirit.explosion_he, cent->lerpOrigin, cent->lerpOrigin, axisDefault, 0 );
		CG_ExplosionMark( cent->lerpOrigin, 80, colorWhite);
		CG_Q3F_Vibrate( 10000, cent->lerpOrigin );
		CG_Q3F_AddAlertIcon(cent->lerpOrigin, Q3F_ALERT_HE_BLOW);
		break;

	case EV_HEAL_PERSON:
		DEBUGNAME("EV_HEAL_PERSON");
		if ( es->eventParm == cg.predictedPlayerState.persistant[PERS_TEAM])
			trap_S_StartSound( cent->lerpOrigin, es->number, CHAN_BODY, cgs.media.medkitSound );
		break;

	case EV_DISCONNECT:
		DEBUGNAME("EV_DISCONNECT");
		cg.deaths[clientNum] = 0;
		cg.kills[clientNum] = 0;
		break;

	/*
	Ensiform - Should probably use this for spanner hits instead of server side G_Sound
	case EV_ARMOR_PERSON:
		DEBUGNAME("EV_HEAL_PERSON");
		if ( es->eventParm == cg.predictedPlayerState.persistant[PERS_TEAM])
			trap_S_StartSound( cent->lerpOrigin, es->number, CHAN_BODY, cgs.media.armorSound ); // sound/misc/ar2_pkup.wav
		break;
	*/

	case EV_ETF_FLAMETHROWER_EFFECT:
		DEBUGNAME("EV_ETF_FLAMETHROWER_EFFECT");
		CG_FireFlameChunks( cent, cent->currentState.origin, cent->currentState.apos.trBase, qtrue );
		break;

	default:
		DEBUGNAME("UNKNOWN");
		CG_Error( "Unknown event: %i", event );
		break;
	}
}


/*
==============
CG_CheckEvents

==============
*/
void CG_CheckEvents( centity_t *cent ) {
	int i;
	// RR2DO2: don't check for events during initialization
	if( cgs.initPhase || cg.infoScreenText[0] != 0 ) {
		return;
	}
	// check for event-only entities
	if ( cent->currentState.eType > ET_EVENTS ) {
		if (cent->previousEvent)
			return;
		cent->previousEvent = cg.time;
		cent->currentState.event = cent->currentState.eType - ET_EVENTS;
		BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin );
		CG_SetEntitySoundPosition( cent );
		CG_EntityEvent( cent, cent->lerpOrigin );
		return;
	}	

	if (cent->currentState.eventSequence != cent->previousEventSequence) {
		cent->previousEvent = cg.time;
		BG_EvaluateTrajectory( &cent->currentState.pos, cg.snap->serverTime, cent->lerpOrigin );
		CG_SetEntitySoundPosition( cent );
		if (cent->currentState.eventSequence < cent->previousEventSequence) 
			cent->previousEventSequence -= (1 << 8);	// eventSequence is sent as an 8-bit through network stream
		if (cent->currentState.eventSequence - cent->previousEventSequence > MAX_EVENTS) 
			cent->previousEventSequence = cent->currentState.eventSequence - MAX_EVENTS;
		for ( i = cent->previousEventSequence ; i != cent->currentState.eventSequence; i++ ) {
			cent->currentState.event = cent->currentState.events[ i & (MAX_EVENTS-1) ];
			if (!cent->currentState.event)
				continue;
			cent->currentState.eventParm = cent->currentState.eventParms[ i & (MAX_EVENTS-1) ];
			CG_EntityEvent( cent, cent->lerpOrigin );
		}
		cent->previousEventSequence = cent->currentState.eventSequence;
	}
}

