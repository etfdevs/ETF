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
** g_q3f_scanner.c
*/

#include "g_q3f_scanner.h"
#include "g_local.h"
#include "bg_q3f_playerclass.h"
#include "g_bot_interface.h"

void G_Q3F_Check_Scanner(struct gentity_s *ent)
{
	gclient_t *client;
	int clientNum;

	if(ent->activator->client->ps.persistant[PERS_CURRCLASS] != Q3F_CLASS_RECON)
		return;

	if(!(ent->activator->client->ps.stats[STAT_Q3F_FLAGS] & 1<<FL_Q3F_SCANNER))
		return;

	if( ent->wait <= level.time )
	{
		// Consume a cell

		ent->activator->client->ps.ammo[AMMO_CELLS]--;
		ent->wait = level.time + Q3F_SCANNER_CELL_TIME;
	}

	if( ent->activator->client->ps.ammo[AMMO_CELLS] <= 0 )
	{
		// Run out of cells

		client = ent->activator->client;
		client->ps.ammo[AMMO_CELLS] = 0;
		client->ps.stats[STAT_Q3F_FLAGS] &= ~(1 << FL_Q3F_SCANNER);
		if( client->scanner_ent )
		{
			trap_SendServerCommand( ent->activator->s.number, "print \"Your scanner has run out of cells.\n\"" );

			if( client->scanner_ent->inuse && client->scanner_ent->s.eType == ET_Q3F_SCANNERDATA )
				G_FreeEntity( client->scanner_ent);
			else G_Printf( "Attempted to free '%s' as scanner data.\n", client->scanner_ent->classname );
			client->scanner_ent = NULL;
		}
		return;
	}

	if( level.time > ent->activator->client->next_scan )
	{
		ent->nextthink = level.time + Q3F_SCANNER_UPDATE_INTERVAL;
		clientNum = G_Q3F_FindNextMovingClient(ent->activator, ent->activator->client->last_scanned_client,
				ent->activator->client); // Hack!

		if(clientNum != -1)
		{
			//G_Printf("Event Queued at %d\n",level.time);
			//G_AddEvent(ent, EV_SCANNER_UPDATE, G_Q3F_Pack_Scanner_Update(ent, clientNum));
			
			ent->activator->client->last_scanned_client = clientNum;
			G_Q3F_Pack_Scanner_Update(ent, clientNum);
			#ifdef BUILD_BOTS
			Bot_Event_RadarDetectedEnemy( ent->activator, &g_entities[clientNum] );
			#endif
		}
		else
		{
			ent->s.time2 = 255;
		}
	}
}

int G_Q3F_FindNextMovingClient(struct gentity_s *ent, int lastnum, struct gclient_s *ignore)
{

	gclient_t *client;
	int i;

	lastnum++;

	i=0;
	while(i < MAX_CLIENTS)
	{
		
		client = level.gentities[lastnum].client;
		if(client && client->pers.connected == CON_CONNECTED)
		{
			if(client->sess.sessionTeam != Q3F_TEAM_SPECTATOR && client->ps.pm_type != PM_DEAD && VectorLength(client->ps.velocity) != 0 && client != ignore)	// Is it alive, and moving (and not me)?
			{
				if(abs((client->ps.origin[0] - ent->client->ps.origin[0])/Q3F_SCANNER_RANGE_DIVISOR) < 128 &&
					abs((client->ps.origin[1] - ent->client->ps.origin[1])/Q3F_SCANNER_RANGE_DIVISOR) < 128)
						return lastnum;
			}
		}
		lastnum++;
		i++;
		if(lastnum >= MAX_CLIENTS)
			lastnum = 0;
	}
	return -1;
}

void G_Q3F_Pack_Scanner_Update(gentity_t *ent, int clientNum )
{
	gclient_t *blip_client;
	int x,y,z;
//	int result;
	blip_client = level.gentities[clientNum].client;
	
	x = (blip_client->ps.origin[0] - ent->activator->client->ps.origin[0])/Q3F_SCANNER_RANGE_DIVISOR;
	y = (blip_client->ps.origin[1] - ent->activator->client->ps.origin[1])/Q3F_SCANNER_RANGE_DIVISOR;
	z = (blip_client->ps.origin[2] - ent->activator->client->ps.origin[2])/Q3F_SCANNER_RANGE_DIVISOR;
	ent->s.origin2[0] = (((y+128) & 0xff) << 8) + ((x+128) & 0xff);
	//ent->s.origin2[1] = (((int)(ent->s.origin2[1]) & 0xfffc) + (blip_client->ps.stats[PERS_TEAM] & 3));
	ent->s.origin2[1] = blip_client->ps.persistant[PERS_TEAM];
	ent->s.origin2[2] = z;
	ent->s.time2 = clientNum;

//		result =((blip_client->ps.stats[PERS_TEAM] & 3) << 10) +
//			((x & 63) << 5) + (y & 63);
//	G_Printf("Scanner Packed: x=%d, y=%d\n",x,y);

}

