#include "g_bot_interface.h"

#ifdef DREVIL_BOT_SUPPORT

#include "q_shared.h"
#include "g_local.h"
#include "g_q3f_team.h"
#include "g_q3f_playerclass.h"
#include "g_q3f_weapon.h"
#include "g_q3f_mapents.h"
#include "bg_q3f_util.h"

#include "BotExports.h"

#define OMNIBOT_MODNAME "etf"
#define OMNIBOT_MAX_NAVLINES 2048
#define OMNIBOT_MAX_RADIUS 8

//////////////////////////////////////////////////////////////////////////
// Static functions prefixed with fb are functions that are passed to the 
// bot dll so that it may call them when necessary. They are the bridge
// between the bot the game.
typedef enum
{
	BOT_DBG_NONE,
	BOT_DBG_LINE_WAYPOINT,
	BOT_DBG_LINE_PATH,
	BOT_DBG_RADIUS
} debugLineType;

typedef struct 
{
	float start[3];
	float end[3];
	float color[3];
	float dist;
	qboolean drawme;
	unsigned char type;
} debugLines_t;

typedef struct 
{
	float pos[3];
	float color[3];
	float radius;
} debugLinesRadius_t;

//-----------------------------------------------------------------

static int wp_compare(const void *_wp1, const void *_wp2)
{
	debugLines_t *wp1 = (debugLines_t*)_wp1;
	debugLines_t *wp2 = (debugLines_t*)_wp2;

	if(wp1->drawme == qfalse)
		return -1;
	if(wp2->drawme == qfalse)
		return 1;
	if(wp1->dist < wp2->dist)
		return -1;
	if(wp1->dist > wp2->dist)
		return 1;
	return 0;
}

//-----------------------------------------------------------------

int g_NumNavLines = 0;
debugLines_t	g_debugLines[OMNIBOT_MAX_NAVLINES];

int g_NumRadiusIndicators = 0;
debugLinesRadius_t g_debugRadius[OMNIBOT_MAX_RADIUS];

int	g_BotLoaded = 0;

//-----------------------------------------------------------------

static void obClearNavLines(int _clearNav, int _clearRadius)
{
	if(_clearNav)
		g_NumNavLines = 0;

	if(_clearRadius)
		g_NumRadiusIndicators = 0;
}

//-----------------------------------------------------------------

static void obAddTempDisplayLine(const int _client, const float start[3], const float end[3], const float color[3])
{
	gentity_t *tent = G_TempEntity(end, EV_BOT_DEBUG_LINE);
	if(_client == -1)
		tent->s.clientNum = (MAX_CLIENTS-1); // everyone
	else
		tent->s.clientNum = _client; // a specific person
	tent->s.angles2[0] = color[0];
	tent->s.angles2[1] = color[1];
	tent->s.angles2[2] = color[2];
	VectorCopy(start, tent->s.origin2);
}

//-----------------------------------------------------------------

static void obUpdateDrawnWaypoints(int _clientNum, float _radius)
{
	static int nextWpUpdate = 0;
	vec3_t playerPos;
	int i;

	if(!g_entities[_clientNum].client)
		return;

	VectorCopy(g_entities[_clientNum].r.currentOrigin, playerPos);

	// Do we need to draw the waypoints again?
	if(g_NumNavLines && (nextWpUpdate < level.time))
	{
		int iDrawnLines = 0;

		// Schedule the next update.
		nextWpUpdate = level.time + 2500;

		for(i = 0; i < g_NumNavLines; i++)
		{
			g_debugLines[i].drawme = qfalse;

			// Only display if it's in the pvs.
			if(trap_InPVS(g_debugLines[i].start, playerPos) ||
				trap_InPVS(g_debugLines[i].end, playerPos))
			{
				vec3_t towp1, towp2;
				float radSq = _radius * _radius;
				VectorSubtract(g_debugLines[i].start, playerPos, towp1);
				VectorSubtract(g_debugLines[i].end, playerPos, towp2);

				g_debugLines[i].dist = min(VectorLengthSquared(towp1), VectorLengthSquared(towp2));
				if(g_debugLines[i].dist > radSq)
					continue;

				g_debugLines[i].drawme = qtrue;
			}
		}

		// Draw the radius indicators.
		//for(i = 0; i < g_NumRadiusIndicators; i++)
		//{			
		//	gentity_t *tent = G_TempEntity(g_debugRadius[i].pos, EV_BOT_DEBUG_RADIUS);
		//	if(_clientNum == -1)
		//		tent->s.clientNum = (MAX_CLIENTS-1); // everyone
		//	else
		//		tent->s.clientNum = _clientNum; // a specific person
		//	tent->s.angles2[0] = g_debugRadius[i].color[0];
		//	tent->s.angles2[1] = g_debugRadius[i].color[1];
		//	tent->s.angles2[2] = g_debugRadius[i].color[2];
		//	tent->s.angles2[0] = g_debugRadius[i].color[0];
		//	tent->s.angles2[1] = g_debugRadius[i].color[1];
		//	tent->s.angles2[2] = g_debugRadius[i].color[2];

		//	tent->s.origin2[0] = g_debugRadius[i].radius;
		//	tent->s.origin2[1] = 9.0f;
		//	tent->s.origin2[2] = 0.0f;
		//}

		// Sort the points based on distance.
		qsort(g_debugLines, g_NumNavLines, sizeof(debugLines_t), wp_compare);

		// Render closest to farthest
		for(i = 0; i < g_NumNavLines; ++i)
		{
			if(g_debugLines[i].drawme && (iDrawnLines < 500))
			{
				// Draw it.
				switch(g_debugLines[i].type)
				{
				case BOT_DBG_LINE_WAYPOINT:
					{
						obAddTempDisplayLine(_clientNum, g_debugLines[i].start, g_debugLines[i].end, g_debugLines[i].color);
						++iDrawnLines;
						break;
					}
				case BOT_DBG_LINE_PATH:
					{
						// adjust the end of the line for paths so they slant down
						// toward the end of the connection instead of drawing on top
						// of each other.
						vec3_t end;
						VectorCopy(g_debugLines[i].end, end);
						end[2] -= 30.0f;
						obAddTempDisplayLine(_clientNum, g_debugLines[i].start, end, g_debugLines[i].color);
						++iDrawnLines;
						break;
					}
				}
			}
		}
	}
}

//-----------------------------------------------------------------

static void obAddDisplayWaypoint(const float pos[3], const float _color[3])
{
	if(g_NumNavLines < OMNIBOT_MAX_NAVLINES)
	{
		g_debugLines[g_NumNavLines].type = BOT_DBG_LINE_WAYPOINT;
		g_debugLines[g_NumNavLines].start[0] = pos[0];
		g_debugLines[g_NumNavLines].start[1] = pos[1];
		g_debugLines[g_NumNavLines].start[2] = pos[2] - 24.0f;
		g_debugLines[g_NumNavLines].end[0] = pos[0];
		g_debugLines[g_NumNavLines].end[1] = pos[1];
		g_debugLines[g_NumNavLines].end[2] = pos[2] + 40.0f;
		g_debugLines[g_NumNavLines].color[0] = _color[0];
		g_debugLines[g_NumNavLines].color[1] = _color[1];
		g_debugLines[g_NumNavLines].color[2] = _color[2];
		++g_NumNavLines;
	}
}

//-----------------------------------------------------------------

static void obAddDisplayPath(const float _start[3], const float _end[3], const float _color[3])
{
	if(g_NumNavLines < OMNIBOT_MAX_NAVLINES)
	{
		g_debugLines[g_NumNavLines].type = BOT_DBG_LINE_PATH;
		g_debugLines[g_NumNavLines].start[0] = _start[0];
		g_debugLines[g_NumNavLines].start[1] = _start[1];
		g_debugLines[g_NumNavLines].start[2] = _start[2] + 36.0f;
		g_debugLines[g_NumNavLines].end[0] = _end[0];
		g_debugLines[g_NumNavLines].end[1] = _end[1];
		g_debugLines[g_NumNavLines].end[2] = _end[2] + 36.0f;
		g_debugLines[g_NumNavLines].color[0] = _color[0];
		g_debugLines[g_NumNavLines].color[1] = _color[1];
		g_debugLines[g_NumNavLines].color[2] = _color[2];
		++g_NumNavLines;
	}
}

//-----------------------------------------------------------------

void obAddDisplayRadius(const float _pos[3], const float _radius, const float _color[3])
{
	/*if(g_NumRadiusIndicators < OMNIBOT_MAX_RADIUS)
	{
		g_debugRadius[g_NumRadiusIndicators].pos[0] = _pos[0];
		g_debugRadius[g_NumRadiusIndicators].pos[1] = _pos[1];
		g_debugRadius[g_NumRadiusIndicators].pos[2] = _pos[2];
		g_debugRadius[g_NumRadiusIndicators].color[0] = _color[0];
		g_debugRadius[g_NumRadiusIndicators].color[1] = _color[1];
		g_debugRadius[g_NumRadiusIndicators].color[2] = _color[2];
		g_debugRadius[g_NumRadiusIndicators].radius = _radius;
		++g_NumRadiusIndicators;
	}*/

	gentity_t *tent = G_TempEntity(_pos, EV_BOT_DEBUG_RADIUS);
	//if(_clientNum == -1)
		tent->s.clientNum = (MAX_CLIENTS-1); // everyone
	//else
	//	tent->s.clientNum = _clientNum; // a specific person
	tent->s.angles2[0] = _color[0];
	tent->s.angles2[1] = _color[1];
	tent->s.angles2[2] = _color[2];

	tent->s.origin2[0] = _radius; // radius
	tent->s.origin2[1] = 9.0f; // number of segments
	tent->s.origin2[2] = 0.0f;
}

//-----------------------------------------------------------------

void Bot_SendSoundEvent(int _client, int _sndtype, GameEntity _source)
{
	static BotUserData bud;
	bud.m_DataType = dtEntity;
	bud.udata.m_Entity = _source;
	Bot_Interface_SendEvent(PERCEPT_HEAR_SOUND, _client, _sndtype, 0.0f, &bud);
}

//-----------------------------------------------------------------

static void obPrintError(const char *_error)
{
	if(_error)
		G_Printf("%s%s\n", S_COLOR_RED, _error);
}

//-----------------------------------------------------------------

static void obPrintMessage(const char *_msg)
{
	if(_msg)
		G_Printf("%s%s\n", S_COLOR_GREEN, _msg);
}

//-----------------------------------------------------------------

static void obPrintScreenMessage(const int _client, const float _pos[3], const char *_msg)
{
	if(_msg)
		trap_SendServerCommand(_client, va("cp \"%s\"", _msg));
}

//-----------------------------------------------------------------

static int obBotChangeTeam(int _client, int _newteam)
{
	gentity_t *client = &g_entities[_client];

	// If a team was specified, try to choose it.
	if(_newteam > 0 && _newteam <= Q3F_TEAM_GREEN)
		if(SetTeam(client, g_q3f_teamlist[_newteam].name) == qtrue)
			return 0;

	// If there wasn't or there was a problem choosing it, get an auto assigned one.
	_newteam = G_Q3F_GetAutoTeamNum(_client);
	return SetTeam(client, g_q3f_teamlist[_newteam].name) == qtrue ? 1 : 0;
}

//-----------------------------------------------------------------

static int obBotChangeClass(int _client, int _newclass)
{
	gentity_t *client = &g_entities[_client];

	// If the class is 0, or we have trouble picking the specified class,
	// pick a random class
	if(_newclass < Q3F_CLASS_MAX)
	{
		if(_newclass == -1 || _newclass == 0 || !G_Q3F_ChangeClassCommand(client, g_q3f_classlist[_newclass]->s->commandstring))
			_newclass = G_Q3F_SelectRandomClass(client->client->sess.sessionTeam, client);
	}	
	return G_Q3F_ChangeClassCommand(client, g_q3f_classlist[_newclass]->s->commandstring) == qtrue ? 1 : 0;	
}

//-----------------------------------------------------------------

static void obBotDoCommand(int _client, char *_cmd)
{
	trap_EA_Command(_client, _cmd);
}

//-----------------------------------------------------------------

static int obAddbot( const char *_name )
{
	int				clientNum;
	gentity_t		*bot;
	char			*s;
	char			userinfo[MAX_INFO_STRING] = {0};
	static int		ipnum = 1;

	if(!_name || !_name[0])
		Info_SetValueForKey(userinfo, "name", va("omni-bot_%i", ipnum));
	else Info_SetValueForKey(userinfo, "name", _name);
	Info_SetValueForKey(userinfo, "rate", "25000");
	Info_SetValueForKey(userinfo, "snaps", "20");
	Info_SetValueForKey(userinfo, "ip", "localhost");

	// have the server allocate a client slot
	clientNum = trap_BotAllocateClient(0);	// Arnout: 0 means no prefered clientslot
	if (clientNum == -1) 
	{
		G_Printf(S_COLOR_RED "Unable to add bot.  All player slots are in use.\n");
		G_Printf(S_COLOR_RED "Start server with more 'open' slots (or check setting of sv_maxclients cvar).\n");
		return -1;
	}

	bot = &g_entities[clientNum];
	bot->r.svFlags |= SVF_BOT;
	bot->inuse = qtrue;

	// register the userinfo
	trap_SetUserinfo(clientNum, userinfo);

	// have it connect to the game as a normal client
	if(s = ClientConnect(clientNum, qtrue, qtrue)) 
		return -1;

	//obBotChangeTeam(clientNum, _team);
	//obBotChangeClass(clientNum, _playerclass);

	bot->client->ps.persistant[PERS_FLAGS] |= PF_AUTORELOAD;
	bot->client->pers.autoReload = 2;

	// Success!, return its client num.
	return clientNum;
}

//-----------------------------------------------------------------

static int obBotKickBot(const char *_name)
{
	int i;
	for(i = 0; i < MAX_CLIENTS; i++)
	{
		if(!g_entities[i].inuse)
			continue;
		if (!g_entities[i].client)
			continue;
		if(g_entities[i].client->pers.connected != CON_CONNECTED)
			continue;
		if (!(g_entities[i].r.svFlags & SVF_BOT))
			continue;

		if(!Q_stricmp(g_entities[i].client->pers.netname, _name))
		{
			trap_DropClient(i, "kicked", 7);
			return i;
		}
	}
	return -1;
}

//-----------------------------------------------------------------

static int obGetClientPosition(int _client, float _pos[3])
{
	if(_client >= 0)
	{
		const gentity_t *pEnt = &g_entities[_client];
		_pos[0] = pEnt->r.currentOrigin[0];
		_pos[1] = pEnt->r.currentOrigin[1];
		_pos[2] = pEnt->r.currentOrigin[2];
		return 1;
	}
	return 0;
}

//-----------------------------------------------------------------

static int obGetClientOrientation(int _client, float _fwd[3], float _right[3], float _up[3])
{
	if(_client >= 0)
	{
		const gentity_t *pEnt = &g_entities[_client];
		if(pEnt->client)
		{
			AngleVectors(pEnt->client->ps.viewangles, _fwd, _right, _up);
		} else
		{
			AngleVectors(pEnt->s.angles, _fwd, _right, _up);
		}
		return 1;
	}
	return 0;
}

//-----------------------------------------------------------------

static int obGetGameTime()
{
	return level.time;
}

//-----------------------------------------------------------------

static int obGetMaxNumPlayers()
{
	return level.maxclients;
}

//-----------------------------------------------------------------

static int obGetCurNumPlayers()
{
	return level.numConnectedClients;
}

//-----------------------------------------------------------------

static int obBotTraceLine(BotTraceResult *_result, const float _start[3], const float _end[3], 
						  const AABB *_pBBox ,int _mask, int _user, obBool _bUsePVS)
{
	trace_t tr;
	int iMask = 0;

	if((_bUsePVS == obfalse) || (qtrue == trap_InPVS(_start, _end)))
	{
		// Set up the collision masks
		if(_mask & TR_MASK_ALL)
			iMask |= MASK_ALL;
		else
		{
			if(_mask & TR_MASK_SOLID)
				iMask |= MASK_SOLID;
			if(_mask & TR_MASK_PLAYER)
				iMask |= MASK_PLAYERSOLID;
			if(_mask & TR_MASK_SHOT)
				iMask |= MASK_SHOT;
			if(_mask & TR_MASK_OPAQUE)
				iMask |= MASK_OPAQUE;
			if(_mask & TR_MASK_WATER)
				iMask |= MASK_WATER;
		}

		trap_Trace(&tr, _start, _pBBox ? _pBBox->m_Mins : NULL, _pBBox ? _pBBox->m_Maxs : NULL, _end, _user, iMask);

		if((tr.entityNum != ENTITYNUM_WORLD) && (tr.entityNum != ENTITYNUM_NONE))
			_result->m_HitEntity = &g_entities[tr.entityNum];
		else 
			_result->m_HitEntity = 0;

		// Fill in the bot traceflag.
		_result->m_Fraction = tr.fraction;
		_result->m_Endpos[0] = tr.endpos[0];
		_result->m_Endpos[1] = tr.endpos[1];
		_result->m_Endpos[2] = tr.endpos[2];
		_result->m_Normal[0] = tr.plane.normal[0];
		_result->m_Normal[1] = tr.plane.normal[1];
		_result->m_Normal[2] = tr.plane.normal[2];		
		
		_result->m_iUser1 = tr.surfaceFlags;
		_result->m_iUser2 = tr.contents;
	} else
	{
		// Not in PVS
		_result->m_Fraction = 0.0f;
		_result->m_HitEntity = 0;
	}
 
	/*tr.allsolid: if this is qtrue then the area (or some part of it) between the mins and maxs is inside a solid (a brush).
	tr.contents: the contents mask of the plane that the trace hit.
	tr.endpos: position where the trace ended.
	tr.entityNum: entity number of the entity that the trace hit (if it hit solid geometry then this will equal ENTITYNUM_WORLD).
	tr.fraction: this is the fraction of the vector between start and end where the trace ended. Therefore if this is less than 1.0 the trace hit something.
	tr.plane: contains information about the plane that was hit (most notably the surface normal) stored in a cplane_t structure.
	tr.startsolid: if this is qtrue then the trace started in a solid (brush).
	tr.surfaceFlags: surfaceflags of the surface that was hit (look in surfaceflags.h at the SURF_* constants to see the which ones there are).*/
	return 1;
}

//-----------------------------------------------------------------

static GameEntity obFindEntityByClassName(GameEntity _pStart, const char *_name)
{
	return (GameEntity)G_Find((gentity_t*)_pStart, FOFS(classname), _name);
}

//-----------------------------------------------------------------

static GameEntity obFindEntityInSphere(const float _pos[3], float _radius, GameEntity _pStart, const char *_name)
{
	// square it to avoid the square root in the distance check.
	float sqRad = _radius * _radius;
	vec3_t toent;	
	gentity_t *pEnt = (gentity_t *)_pStart;

	while((pEnt = G_Find(pEnt, FOFS(classname), _name)) != NULL)
	{
		VectorSubtract(_pos, pEnt->r.currentOrigin, toent);
		if(VectorLengthSquared(toent) < sqRad)
			break;
	}
	return (GameEntity)pEnt;
}

//-----------------------------------------------------------------

static int obGetEntityPosition(const GameEntity _ent, float _pos[3])
{
	gentity_t *ent = (gentity_t *)_ent;

	if(!ent->client)
	{
		// Need to do special case stuff for certain entities.
		if((ent->s.eType == ET_MOVER) ||
			!Q_stricmp(ent->classname, "trigger_multiple") || 
			!Q_stricmp(ent->classname, "func_commandpoint") ||
			!Q_stricmp(ent->classname, "plat_trigger") ||
			!Q_stricmp(ent->classname, "door_trigger"))
		{
			// Center of bounding box, cuz currentOrigin isn't valid.
			_pos[0] = (ent->r.absmax[0] + ent->r.absmin[0]) * 0.5f;
			_pos[1] = (ent->r.absmax[1] + ent->r.absmin[1]) * 0.5f;
			_pos[2] = (ent->r.absmax[2] + ent->r.absmin[2]) * 0.5f;
			return 1;
		}
	}

	// Clients and entities not caught above will return normal position.
	_pos[0] = ent->r.currentOrigin[0];
	_pos[1] = ent->r.currentOrigin[1];
	_pos[2] = ent->r.currentOrigin[2];

	return 1;
}

//-----------------------------------------------------------------

int obGetEntityFlags(const GameEntity _ent)
{
	int iFlags = 0;
	gentity_t *pEnt = (gentity_t *)_ent;
	if(pEnt)
	{
		// Set any flags.
		if(pEnt->health <= 0)
			iFlags |= ENT_FLAG_DEAD;
		if(pEnt->client->ps.pm_flags & PMF_DUCKED)
			iFlags |= ENT_FLAG_CROUCHED;		
		if(pEnt->client->ps.eFlags & EF_Q3F_SAVEME)
			iFlags |= ETF_ENT_SAVEME;
		if(pEnt->client->ps.eFlags & EF_Q3F_ARMORME)
			iFlags |= ETF_ENT_ARMORME;
		if(pEnt->client->ps.extFlags & EXTF_BURNING)
			iFlags |= ETF_ENT_BURNING;
		if(pEnt->client->ps.extFlags & EXTF_TRANQED)
			iFlags |= ETF_ENT_TRANQED;
		if(pEnt->client->ps.persistant[PERS_CURRCLASS] == Q3F_CLASS_AGENT)
		{
			if(G_Q3F_IsDisguised(pEnt))
			{
				iFlags |= ETF_ENT_FLAG_DISGUISED;
			}
			if(pEnt->client->ps.eFlags & EF_Q3F_INVISIBLE)
				iFlags |= ETF_ENT_FLAG_INVISIBLE;
		}
	}
	return iFlags;
}

//-----------------------------------------------------------------

int obGetEntityEyePosition(const GameEntity _ent, float _pos[3])
{
	if(obGetEntityPosition(_ent, _pos))
	{
		gentity_t *ent = (gentity_t *)_ent;
		if(ent->client)
		{
			_pos[2] += ent->client->ps.viewheight;
		}
		return 1;
	}
	return 1;
}

//-----------------------------------------------------------------

int obGetEntityBonePosition(const GameEntity _ent, int _boneid, float _pos[3])
{
	// ET doesnt really support bones
	return obGetEntityPosition(_ent, _pos);
}

//-----------------------------------------------------------------

static int obIsEntityAllied(const GameEntity _ent, const GameEntity _target)
{
	gentity_t *pEnt = (gentity_t *)_ent;
	gentity_t *pTarget = (gentity_t *)_target;

	if(!Q_stricmp("autosentry", pTarget->classname) || 
		!Q_stricmp("supplystation", pTarget->classname))
		pTarget = pTarget->parent;

	return G_Q3F_IsAllied(pEnt, pTarget);
}

//-----------------------------------------------------------------

static int obGetEntityOrientation(const GameEntity _ent, float _fwd[3], float _right[3], float _up[3])
{
	gentity_t *pEnt = (gentity_t *)_ent;
	if(pEnt->client)
	{
		AngleVectors(pEnt->client->ps.viewangles, _fwd, _right, _up);
	} else
	{
		AngleVectors(pEnt->s.angles, _fwd, _right, _up);
	}
	return 1;
}

//-----------------------------------------------------------------

static int obGetEntityVelocity(const GameEntity _ent, float _velocity[3])
{
	gentity_t *pEnt = (gentity_t *)_ent;
	_velocity[0] = pEnt->s.pos.trDelta[0];
	_velocity[1] = pEnt->s.pos.trDelta[1];
	_velocity[2] = pEnt->s.pos.trDelta[2];
	return 1;
}

//-----------------------------------------------------------------

static int obGetEntityWorldAABB(const GameEntity _ent, AABB *_aabb)
{
	gentity_t *pEnt = (gentity_t *)_ent;
	_aabb->m_Mins[0] = pEnt->r.absmin[0];
	_aabb->m_Mins[1] = pEnt->r.absmin[1];
	_aabb->m_Mins[2] = pEnt->r.absmin[2];
	_aabb->m_Maxs[0] = pEnt->r.absmax[0];
	_aabb->m_Maxs[1] = pEnt->r.absmax[1];
	_aabb->m_Maxs[2] = pEnt->r.absmax[2];
	return 1;
}

//-----------------------------------------------------------------

static int obGetEntityOwner(const GameEntity _ent)
{
	gentity_t *pEnt = (gentity_t *)_ent;
	// -1 means theres no owner.
	return ((pEnt->s.otherEntityNum == MAX_CLIENTS) ? -1 : pEnt->s.otherEntityNum);
}

//-----------------------------------------------------------------

static GameEntity obEntityFromID(const int _id)
{
	return (_id < MAX_GENTITIES && _id >= 0) ? (GameEntity)&g_entities[_id] : 0;
}

//-----------------------------------------------------------------

static int obIDFromEntity(const GameEntity _ent)
{
	gentity_t *pEnt = (gentity_t *)_ent;
	gentity_t *pStart = g_entities;
	int iIndex = pEnt - pStart;
	assert(iIndex >= 0);
	return (iIndex < MAX_GENTITIES) ? iIndex : -1;
}

//-----------------------------------------------------------------

static int obGetGoals()
{
	gentity_t *check;
	int i;

	// Search through the entities of the game for potential goals and register them
	// with the bot.
	for(i = 0; i < level.num_entities; ++i)
	{
		check = &g_entities[i];

		switch (check->s.eType)
		{
		case ET_GENERAL:
			{
				/*char *pGroupName = NULL;
				int iScoreValue = 0, i;
				q3f_keypair_t *pTS = NULL;*/

				//if(!Q_stricmp(check->classname, "trigger_multiple"))
				//{
				//	if(!check->mapdata || !check->mapdata->holding || 
				//		(check->mapdata->holding->data->type != Q3F_TYPE_STRING))
				//		continue;

				//	// we got the name of the object
				//	pGroupName = check->mapdata->holding->data->d.strdata;

				//	// make sure this goal is worth points, or we don't care about it.
				//	if(!check->mapdata->other)
				//		continue;

				//	// Look through the "array" for team score.
				//	for(i = 0; i < check->mapdata->other->used; ++i)
				//	{
				//		if(!Q_stricmp(check->mapdata->other->data[i].key, "teamscore"))
				//		{
				//			pTS = &check->mapdata->other->data[i];
				//			break;
				//		}
				//	}

				//	if(!pTS || !pTS->value.d.intdata)
				//		continue;

				//	// Finally, got the team score.
				//	if(pTS->value.type == Q3F_TYPE_STRING)
				//		iScoreValue = atoi(pTS->value.d.strdata);
				//	else if(pTS->value.type == Q3F_TYPE_INTEGER)							
				//		iScoreValue = pTS->value.d.intdata;

				//	if(iScoreValue <= 0)
				//		continue;

				//	// todo: get the team and flags(inactive, carried...)
				//	{
				//		// Register this capture point with the bot.
				//		// red == 2, blue == 4, yellow == 8, green == 16
				//		pfnBotAddGoal((GameEntity*)check, ETF_GOAL_CAPPOINT, check->mapdata->team, pGroupName, NULL);
				//		pfnBotLog(va("Found Cap Point \"%s\" worth %d for team %d", pGroupName, iScoreValue, check->mapdata->team));
				//	}
				//} else if(!Q_stricmp(check->classname, "func_commandpoint"))
				//{
				//	if(!check->mapdata)
				//		continue;

				//	// make sure this goal is worth points, or we don't care about it.
				//	if(!check->mapdata->other)
				//		continue;

				//	if(!check->mapdata->groupname || (check->mapdata->groupname->data->type != Q3F_TYPE_STRING))
				//		continue;

				//	// we got the name of the object
				//	pGroupName = check->mapdata->groupname->data->d.strdata;
				//	//Bot_AddGoal((GameEntity*)check, GOAL_COMMANDPOINT, check->mapdata->team, pGroupName);
				//	//Bot_Log(va("Found command point \"%s\" for team %d", pGroupName, check->mapdata->team));
				//}
			}
			break;
		case ET_Q3F_GOAL:
			// Figure out what kind of goal this is.
			if(!Q_stricmp(check->classname, "func_goalinfo"))
			{
				// assume this is a backback. todo: make this smarter
				if(check->mapdata && check->mapdata->give)
				{
					char *pGroupName = 0;
					if(check->mapdata->groupname)
						pGroupName = check->mapdata->groupname->data->d.strdata;

					if(g_BotFunctions.pfnBotLog)
						g_BotFunctions.pfnBotLog(va("Found backpack %x", check));
					//Bot_Log(va("Found backpack %x", check));
				}
			} else if(!Q_stricmp(check->classname, "func_goalitem"))
			{
				// This is going to be a flag.
				if(check->mapdata && check->mapdata->groupname)
				{
					const char *pGroupName = check->mapdata->groupname->data->d.strdata;

					// Register this capture point with the bot.
					if(g_BotFunctions.pfnBotAddGoal)
						g_BotFunctions.pfnBotAddGoal((GameEntity*)check, ETF_GOAL_FLAG, check->mapdata->team, pGroupName, NULL);
					if(g_BotFunctions.pfnBotLog)
						g_BotFunctions.pfnBotLog(va("Found Flag \"%s\" for team %d\n", pGroupName, check->mapdata->team));
					//Bot_AddGoal((GameEntity*)check, GOAL_FLAG, check->mapdata->team, pGroupName);
					//Bot_Log(va("Found Flag \"%s\" for team %d\n", pGroupName, check->mapdata->team));
				}
			}
			break;
		case ET_MOVER:
			{
				if(!Q_stricmp(check->classname, "func_plat"))
				{                    
				} 
				else if(!Q_stricmp(check->classname, "func_door"))
				{
				} 
				else if(!Q_stricmp(check->classname, "func_button"))
				{
				}
			}
			break;
		}
	}

	return 1;
}

//-----------------------------------------------------------------

static int obPrintEntitiesInRadius(const float _pos[3], float _radius)
{
	gentity_t *check;
	vec3_t toent;
	float sqRad = _radius*_radius;
	int i;
	char *pBuffer = va("Entities within %f radius", _radius);

	if(!g_BotFunctions.pfnBotLog)
		return 0;
	
	g_BotFunctions.pfnBotLog(pBuffer);
	obPrintMessage(pBuffer);

	for(i = 0; i < level.num_entities; ++i)
	{
		float pos[3];
		check = &g_entities[i];

		if (!check->inuse)
			continue;

		obGetEntityPosition(&check, pos);

		// Is it within the specified radius
		VectorSubtract(_pos, pos, toent);
		if(VectorLengthSquared(toent) > sqRad)
			continue;

		// Interpret the entity into the appropriate flags for the bot.
		switch (check->s.eType)
		{
		case ET_GENERAL:
			g_BotFunctions.pfnBotLog("ET_GENERAL");
			obPrintMessage("ET_GENERAL");
			break;
		case ET_PLAYER:
			g_BotFunctions.pfnBotLog("ET_PLAYER");
			obPrintMessage("ET_PLAYER");
			break;
		case ET_ITEM:
			g_BotFunctions.pfnBotLog("ET_ITEM");
			obPrintMessage("ET_ITEM");
			break;
		case ET_MISSILE:
			g_BotFunctions.pfnBotLog("ET_MISSILE");
			obPrintMessage("ET_MISSILE");
			break;
		case ET_MOVER:
			g_BotFunctions.pfnBotLog("ET_MOVER");
			obPrintMessage("ET_MOVER");
			break;
		case ET_BEAM:
			g_BotFunctions.pfnBotLog("ET_BEAM");
			obPrintMessage("ET_BEAM");
			break;
		case ET_PORTAL:
			g_BotFunctions.pfnBotLog("ET_PORTAL");
			obPrintMessage("ET_PORTAL");
			break;
		case ET_SPEAKER:
			g_BotFunctions.pfnBotLog("ET_SPEAKER");
			obPrintMessage("ET_SPEAKER");
			break;
		case ET_PUSH_TRIGGER:
			g_BotFunctions.pfnBotLog("ET_PUSH_TRIGGER");
			obPrintMessage("ET_PUSH_TRIGGER");
			break;
		case ET_TELEPORT_TRIGGER:
			g_BotFunctions.pfnBotLog("ET_TELEPORT_TRIGGER");
			obPrintMessage("ET_TELEPORT_TRIGGER");
			break;
		case ET_INVISIBLE:
			g_BotFunctions.pfnBotLog("ET_INVISIBLE");
			obPrintMessage("ET_INVISIBLE");
			break;
		case ET_Q3F_GRENADE:
			g_BotFunctions.pfnBotLog("ET_Q3F_GRENADE");
			obPrintMessage("ET_Q3F_GRENADE");
			break;
		case ET_Q3F_GOAL:
			g_BotFunctions.pfnBotLog("ET_Q3F_GOAL");
			obPrintMessage("ET_Q3F_GOAL");			
			break;		
		case ET_Q3F_AGENTDATA:
			g_BotFunctions.pfnBotLog("ET_Q3F_AGENTDATA");
			obPrintMessage("ET_Q3F_AGENTDATA");
			break;
		case ET_Q3F_SCANNERDATA:
			g_BotFunctions.pfnBotLog("ET_Q3F_SCANNERDATA");
			obPrintMessage("ET_Q3F_SCANNERDATA");
			break;
		case ET_SNIPER_DOT:
			g_BotFunctions.pfnBotLog("ET_SNIPER_DOT");
			obPrintMessage("ET_SNIPER_DOT");
			break;
		case ET_FLAME:
			g_BotFunctions.pfnBotLog("ET_FLAME");
			obPrintMessage("ET_FLAME");
			break;
		case ET_Q3F_SENTRY:
			g_BotFunctions.pfnBotLog("ET_Q3F_SENTRY");
			obPrintMessage("ET_Q3F_SENTRY");
			break;
		case ET_Q3F_SUPPLYSTATION:
			g_BotFunctions.pfnBotLog("ET_Q3F_SUPPLYSTATION");
			obPrintMessage("ET_Q3F_SUPPLYSTATION");
			break;
		case ET_Q3F_BEAM:
			g_BotFunctions.pfnBotLog("ET_Q3F_BEAM");
			obPrintMessage("ET_Q3F_BEAM");
			break;
		case ET_Q3F_MAPSENTRY:
			g_BotFunctions.pfnBotLog("ET_Q3F_MAPSENTRY");
			obPrintMessage("ET_Q3F_MAPSENTRY");
			break;
		case ET_Q3F_PANEL:
			g_BotFunctions.pfnBotLog("ET_Q3F_PANEL");
			obPrintMessage("ET_Q3F_PANEL");
			break;
		case ET_Q3F_FORCEFIELD:
			g_BotFunctions.pfnBotLog("ET_Q3F_FORCEFIELD");
			obPrintMessage("ET_Q3F_FORCEFIELD");
			break;
		}
		g_BotFunctions.pfnBotLog(check->classname);
		obPrintMessage(check->classname);
	}
	return 1;
}

//-----------------------------------------------------------------

static int obGetThreats()
{
	EntityInfo info;
	gentity_t *pEnd = &g_entities[level.num_entities];
	gentity_t *pCurrent = g_entities;

	// Loop through the game entities and tell the bot about all potential targets
	for( ; pCurrent < pEnd; pCurrent++)
	{
		// Skip bad/unused entities.
		if(!pCurrent->inuse)
			continue;

		// Skip spectators
		if(pCurrent->client && Q3F_IsSpectator(pCurrent->client))
			continue;

		// default data.
		info.m_EntityFlags = 0;
		info.m_EntityCategory = 0;
		info.m_EntityClass = ETF_CLASS_NULL;
		info.m_UserData.m_DataType = dtNone;

		// Check for entities that we are interested in.
		switch(pCurrent->s.eType)
		{
		case ET_GENERAL:
			{
				if(!Q_stricmp(pCurrent->classname, "plat_trigger") ||
					!Q_stricmp(pCurrent->classname, "door_trigger"))
					info.m_EntityCategory = ENT_CAT_MOVER;
				else
					continue;
				break;
			}
		case ET_PLAYER:
			{
				if(!pCurrent->client)
					continue;
				// Set the entity class.
				info.m_EntityClass = pCurrent->client->ps.persistant[PERS_CURRCLASS];
				// Set the entity category
				info.m_EntityCategory = ENT_CAT_SHOOTABLE | ENT_CAT_PLAYER;
				// Set any flags.
				info.m_EntityFlags = obGetEntityFlags(pCurrent);
				break;
			}
		case ET_Q3F_SENTRY:
			{
				info.m_EntityClass = ETF_CLASSEX_SENTRY;
				// Set the entity category
				info.m_EntityCategory = ENT_CAT_SHOOTABLE;
				// Set any flags.

				break;
			}
		case ET_Q3F_SUPPLYSTATION:
			{
				info.m_EntityClass = ETF_CLASSEX_SUPPLYSTATION;
				// Set the entity category
				info.m_EntityCategory = ENT_CAT_SHOOTABLE;
				// Set any flags.

				break;
			}
		case ET_MISSILE:
			{	
				// Set the entity category
				info.m_EntityCategory = ENT_CAT_PROJECTILE | ENT_CAT_AVOID;

				// "nail", "napalm", "flame"
				if(!Q_stricmp(pCurrent->classname, "grenade"))
					info.m_EntityClass = ETF_CLASSEX_GRENADE;
				else if(!Q_stricmp(pCurrent->classname, "rocket"))
					info.m_EntityClass = ETF_CLASSEX_ROCKET;
				else if(!Q_stricmp(pCurrent->classname, "pipe"))
					info.m_EntityClass = ETF_CLASSEX_PIPE;
				else
					continue;
				break;
			}
		case ET_MOVER:
			{
				/*if(!Q_stricmp(pCurrent->classname, "func_plat"))
					info.m_EntityCategory = ENT_CAT_MOVER;
				else*/
					continue;
				break;
			}
		case ET_PUSH_TRIGGER:
			continue;
		case ET_TELEPORT_TRIGGER:
			continue;
		case ET_Q3F_GRENADE:
			{
				// Set the entity category
				info.m_EntityCategory = ENT_CAT_PROJECTILE | ENT_CAT_AVOID;

				if(!Q_stricmp(pCurrent->classname, "charge"))
					info.m_EntityClass = ETF_CLASSEX_DETPACK;
				else
					info.m_EntityClass = ETF_CLASSEX_GRENADE;
				break;
			}
		case ET_Q3F_GOAL:
			{
				if(pCurrent->mapdata && pCurrent->mapdata->give)
				{
					info.m_EntityClass = ETF_CLASSEX_BACKPACK;
					// Set the entity category
					info.m_EntityCategory = ENT_CAT_PICKUP;
					// Set any flags.

					/*if(pCurrent->s.eType == ET_INVISIBLE)
					info.m_EntityFlags*/
				}
				else
				{
					continue;
				}
				break;
			}
		default:
			continue; // ignore this type.
		}

		if(g_BotFunctions.pfnBotAddThreatEntity)
			g_BotFunctions.pfnBotAddThreatEntity((GameEntity)pCurrent, &info);
	}
	return 1;
}

//-----------------------------------------------------------------

static int obInterfaceSendMessage(int _msg, const GameEntity _ent, const BotUserData *_in, BotUserData *_out)
{
	gentity_t *pEnt = (gentity_t *)_ent;
	bg_q3f_playerclass_t *cls = 0;;

	switch(_msg)
	{
		///////////////////////
		// General Messages. //
		///////////////////////
	case GEN_MSG_ISALIVE:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = pEnt->client && (pEnt->health > 0) && !Q3F_IsSpectator(pEnt->client);			
			break;
		}
	case GEN_MSG_ISRELOADING:
		{
			assert(pEnt->client);
			_out->m_DataType = dtInt;
			_out->udata.m_Int = (pEnt->client->ps.weaponstate >= WEAPON_RRAISING) &&
				(pEnt->client->ps.weaponstate <= WEAPON_RELOADING) ? 1 : 0;
			break;
		}
	case GEN_MSG_ISREADYTOFIRE:
		{
			assert(pEnt->client);
			_out->m_DataType = dtInt;
			_out->udata.m_Int = (pEnt->client->ps.weaponstate < WEAPON_RRAISING) ||
				(pEnt->client->ps.weaponstate > WEAPON_RELOADING) ? 1 : 0;
			break;
		}
	case GEN_MSG_ISALLIED:
		{
			gentity_t *pTarget = (gentity_t *)(_in->udata.m_Entity);
			assert(_in && (_in->m_DataType == dtEntity));
			if(pEnt->client)
			{
				_out->m_DataType = dtInt;
				// Look at the parent of sentries and supply stations.
				if(!Q_stricmp("autosentry", pTarget->classname) || 
					!Q_stricmp("supplystation", pTarget->classname))
					pTarget = pTarget->parent;
				_out->udata.m_Int = (G_Q3F_IsAllied(pEnt, pTarget) == qtrue) ? 1 : 0;
			} else
				_out->m_DataType = dtNone;
			break;
		}
	case GEN_MSG_ISHUMAN:
		{
			gentity_t *pTarget = (gentity_t *)(_in->udata.m_Entity);
			assert(_in && (_in->m_DataType == dtEntity));
			_out->m_DataType = dtInt;
            if(pTarget->inuse && pTarget->client && !(pTarget->r.svFlags & SVF_BOT))
				_out->udata.m_Int = 1;
			else
				_out->udata.m_Int = 0;
			break;
		}
	case GEN_MSG_GETEQUIPPEDWEAPON:
		{
			assert(_in == NULL);
			_out->m_DataType = dtInt;
			_out->udata.m_Int = pEnt->client->ps.weapon;
			break;
		}
	case GEN_MSG_GETCURRENTCLASS:
		{
			//_out->m_DataType = dtInt;
			//// Figure out the value based on class name or entity type.
			//if(pEnt->client && !Q_stricmp("player", pEnt->classname))
			//	_out->m_Int = pEnt->client->ps.persistant[PERS_CURRCLASS];
			//else 
			//{
			//	if(!Q_stricmp("autosentry", pEnt->classname))
			//		_out->m_Int = ETF_CLASSEX_SENTRY;
			//	else if(!Q_stricmp("supplystation", pEnt->classname))
			//		_out->m_Int = ETF_CLASSEX_SUPPLYSTATION;
			//}
			break;
		}
	case GEN_MSG_GETCURRENTTEAM:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = (pEnt->client != NULL) ? pEnt->client->ps.teamNum : 0;
			break;
		}
	case GEN_MSG_GETVIEWHEIGHT:
		{
			_out->m_DataType = dtFloat;
			_out->udata.m_Float = (pEnt->client != NULL) ? (float)pEnt->client->ps.viewheight : 0.0f;
			break;
		}
	case GEN_MSG_GETHEALTHARMOR:
		{
			cls = BG_Q3F_GetClass(&(pEnt->client->ps));
			_out->m_DataType = dt6_2byteFlags;
			_out->udata.m_2ByteFlags[0] = pEnt->client->ps.stats[STAT_HEALTH];
			_out->udata.m_2ByteFlags[1] = cls->maxhealth;
			_out->udata.m_2ByteFlags[2] = pEnt->client->ps.stats[STAT_ARMOR];
			_out->udata.m_2ByteFlags[3] = cls->maxarmour;	
			break;
		}	
		//////////////////////////////////
		// Game specific messages next. //
		//////////////////////////////////
		// Info.
	case ETF_MSG_ISGUNCHARGING:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = (pEnt->client->ps.weaponstate == WEAPON_AIMING) ? 1 : 0;
			break;
		}
	case ETF_MSG_ISINVISIBLE:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int =  (pEnt->s.eFlags & EF_Q3F_INVISIBLE) ? 1 : 0;
			break;
		}
	case ETF_MSG_ISDISGUISED:
		{
			_out->m_DataType = dt3_4byteFlags;
			if(pEnt->client && G_Q3F_IsDisguised(pEnt))
			{
				_out->udata.m_4ByteFlags[0] = pEnt->client->agentclass;
				_out->udata.m_4ByteFlags[1] = pEnt->client->agentteam;
				_out->udata.m_4ByteFlags[2] = 0;
			} else
				_out->udata.m_Int = 0;			
			break;
		}
	case ETF_MSG_ISBUILDING:
		break;
		// Effects
	case ETF_MSG_ISCONCED:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = (pEnt->client->ps.powerups[PW_Q3F_CONCUSS] > level.time) ? 1 : 0;			
			break;
		}
	case ETF_MSG_ISONFIRE:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = (pEnt->client->flames > 0) ? 1 : 0;			
			break;
		}
	case ETF_MSG_ISINFECTED:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = (pEnt->client->diseaseTime > level.time) ? 1 : 0;			
			break;
		}
	case ETF_MSG_ISGASSED:
		break;
	case ETF_MSG_ISTRANQED:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = (pEnt->client->lastgasTime+500>level.time) ? 1 : 0;
			break;
		}
	case ETF_MSG_ISBLIND:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = (pEnt->client->ps.powerups[PW_Q3F_FLASH] > level.time) ? 1 : 0;

			break;
		}
		// Powerups
	case ETF_MSG_HASQUAD:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = level.time - pEnt->client->ps.powerups[PW_QUAD];
			if(_out->udata.m_Int < 0) _out->udata.m_Int = 0;
			break;
		}
	case ETF_MSG_HASBATTLESUIT:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = level.time - pEnt->client->ps.powerups[PW_BATTLESUIT];
			if(_out->udata.m_Int < 0) _out->udata.m_Int = 0;
			break;
		}
	case ETF_MSG_HASHASTE:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = level.time - pEnt->client->ps.powerups[PW_HASTE];
			if(_out->udata.m_Int < 0) _out->udata.m_Int = 0;
			break;
		}
	case ETF_MSG_HASINVIS:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = level.time - pEnt->client->ps.powerups[PW_INVIS];
			if(_out->udata.m_Int < 0) _out->udata.m_Int = 0;
			break;
		}
	case ETF_MSG_HASREGEN:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = level.time - pEnt->client->ps.powerups[PW_REGEN];
			if(_out->udata.m_Int < 0) _out->udata.m_Int = 0;
			break;
		}
	case ETF_MSG_HASFLIGHT:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = level.time - pEnt->client->ps.powerups[PW_FLIGHT];
			if(_out->udata.m_Int < 0) _out->udata.m_Int = 0;
			break;
		}
	case ETF_MSG_HASINVULN:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = level.time - pEnt->client->ps.powerups[PW_Q3F_INVULN];
			if(_out->udata.m_Int < 0) _out->udata.m_Int = 0;
			break;
		}
	case ETF_MSG_HASAQUALUNG:
		{
			_out->m_DataType = dtInt;
			_out->udata.m_Int = level.time - pEnt->client->ps.powerups[PW_Q3F_AQUALUNG];
			if(_out->udata.m_Int < 0) _out->udata.m_Int = 0;
			break;
		}
		// Actions
	case ETF_MSG_USERECONSCANNER:
		{
			break;
		}
		// Get Info
	case ETF_MSG_GETPIPECOUNT:
		{
			gentity_t *pPipe = NULL;
			int iPlayerTeam = pEnt->client->sess.sessionTeam;
			int iPipeTeam, iTeamPipes = 0, iPlayerPipes = 0;
			while ((pPipe = G_Find(pPipe, FOFS(classname), "pipe")) != NULL)
			{
				if(!pPipe->parent || !pPipe->parent->client)
					continue;
				iPipeTeam = pPipe->parent->client->sess.sessionTeam;
				if(iPipeTeam == iPlayerTeam)
					iTeamPipes++;
				if(pPipe->parent == pEnt)
					iPlayerPipes++;
			}
			assert(_out);
			_out->m_DataType = dt3_4byteFlags;
			_out->udata.m_4ByteFlags[0] = iPlayerPipes;
			_out->udata.m_4ByteFlags[1] = iTeamPipes;
			break;
		}
	case ETF_MSG_GETAGENTINFO:
		{
			if(pEnt->client && pEnt->client->agentdata)
			{
				_out->m_DataType = dt6_2byteFlags;
				_out->udata.m_2ByteFlags[0] = pEnt->client->agentclass; // class disguised as
				_out->udata.m_2ByteFlags[1] = pEnt->client->agentteam; // team disguised as
                _out->udata.m_2ByteFlags[2] = pEnt->client->agentdata->s.torsoAnim; // class disguising as
				_out->udata.m_2ByteFlags[3] = pEnt->client->agentdata->s.weapon; // team disguising as
				_out->udata.m_2ByteFlags[4] = pEnt->client->agentdata->s.time; // time started disguising
				_out->udata.m_2ByteFlags[5] = pEnt->client->agentdata->s.time2; // time end disguising
			}
			break;
		}
	default:
		{
			assert(0 && "Unknown Interface Message");
		}
	}
	return 1;
}

//-----------------------------------------------------------------

static int obBotChangeWeapon(int _client, int _newweapon)
{
	return 1;
}

//-----------------------------------------------------------------

static int obBotGetWeaponAmmo(int _client, int _weapon, int *_curammo, int *_maxammo, int *_curclip, int *_maxclip)
{
	// Match the bots enum with the games enum.
	switch(_weapon)
	{
	case ETF_WP_AXE:
		_weapon = WP_AXE;
		break;
	case ETF_WP_SHOTGUN:
		_weapon = WP_SHOTGUN;
		break;
	case ETF_WP_SUPERSHOTGUN:
		_weapon = WP_SUPERSHOTGUN;
		break;
	case ETF_WP_NAILGUN:
		_weapon = WP_NAILGUN;
		break;
	case ETF_WP_SUPERNAILGUN:
		_weapon = WP_SUPERNAILGUN;
		break;
	case ETF_WP_GRENADE_LAUNCHER:
		_weapon = WP_GRENADE_LAUNCHER;
		break;
	case ETF_WP_ROCKET_LAUNCHER:
		_weapon = WP_ROCKET_LAUNCHER;
		break;
	case ETF_WP_SNIPER_RIFLE:
		_weapon = WP_SNIPER_RIFLE;
		break;
	case ETF_WP_RAILGUN:
		_weapon = WP_RAILGUN;
		break;
	case ETF_WP_FLAMETHROWER:
		_weapon = WP_FLAMETHROWER;
		break;
	case ETF_WP_MINIGUN:
		_weapon = WP_MINIGUN;
		break;
	case ETF_WP_ASSAULTRIFLE:
		_weapon = WP_ASSAULTRIFLE;
		break;
	case ETF_WP_DARTGUN:
		_weapon = WP_DARTGUN;
		break;
	case ETF_WP_PIPELAUNCHER:
		_weapon = WP_PIPELAUNCHER;
		break;
	case ETF_WP_NAPALMCANNON:
		_weapon = WP_NAPALMCANNON;
		break;
	default:
		_weapon = -1;
	}	

	if(_weapon != -1)
	{
		gentity_t *pClient = &g_entities[_client];
		int iAmmoType = Q3F_GetAmmoTypeForWeapon(_weapon);
		bg_q3f_weapon_t *pWeapon = BG_Q3F_GetWeapon(_weapon);
		g_q3f_playerclass_t *pPlayerClass = G_Q3F_GetClass(&pClient->client->ps);

		switch(pWeapon->ammotype)
		{
		case ETF_AMMO_SHELLS:
			*_maxammo = pPlayerClass->s->maxammo_shells;
			break;
		case ETF_AMMO_CELLS:
			*_maxammo = pPlayerClass->s->maxammo_cells;
			break;
		case ETF_AMMO_NAILS:
			*_maxammo = pPlayerClass->s->maxammo_nails;
			break;
		case ETF_AMMO_ROCKETS:
			*_maxammo = pPlayerClass->s->maxammo_rockets;
			break;
		case ETF_AMMO_MEDIKIT:
			*_maxammo = pPlayerClass->s->maxammo_medikit;
			break;
		default:
			*_maxammo = -1;
		}

		*_curammo = pClient->client->ps.ammo[iAmmoType];
		*_curclip = Q3F_GetClipValue(_weapon, &pClient->client->ps);
		*_maxclip = pWeapon->clipsize;
		return 1;
	}	
	return 0;
}

//-----------------------------------------------------------------

static int obBotGetCurrentAmmo(int _client, int _ammotype, int *_cur, int *_max)
{
	gentity_t *pClient = &g_entities[_client];
	g_q3f_playerclass_t *pPlayerClass = G_Q3F_GetClass(&pClient->client->ps);

	switch(_ammotype)
	{
	case ETF_AMMO_SHELLS:
		*_cur = pClient->client->ps.ammo[AMMO_SHELLS];
		*_max = pPlayerClass->s->maxammo_shells;
		break;
	case ETF_AMMO_NAILS:
		*_cur = pClient->client->ps.ammo[AMMO_NAILS];
		*_max = pPlayerClass->s->maxammo_nails;
		break;
	case ETF_AMMO_ROCKETS:
		*_cur = pClient->client->ps.ammo[AMMO_ROCKETS];
		*_max = pPlayerClass->s->maxammo_rockets;
		break;
	case ETF_AMMO_CELLS:
		*_cur = pClient->client->ps.ammo[AMMO_CELLS];
		*_max = pPlayerClass->s->maxammo_cells;
		break;
	case ETF_AMMO_MEDIKIT:
		*_cur = pClient->client->ps.ammo[AMMO_MEDIKIT];
		*_max = pPlayerClass->s->maxammo_medikit;
		break;
	case ETF_AMMO_CHARGE:
		*_cur = pClient->client->ps.ammo[AMMO_CHARGE];
		*_max = pPlayerClass->s->maxammo_charge;
		break;
	case ETF_AMMO_GRENADE1:
		*_cur = (pClient->client->ps.ammo[AMMO_GRENADES] & 0x00FF);
		*_max = pPlayerClass->s->gren1max;
		break;
	case ETF_AMMO_GRENADE2:
		*_cur = (pClient->client->ps.ammo[AMMO_GRENADES] >> 8);
		*_max = pPlayerClass->s->gren2max;
		break;
	default:
		*_cur = 0;
		*_max = 0;
	}
	return 1;
}

//-----------------------------------------------------------------

static void obUpdateBotInput(int _client, const ClientInput *_input)
{	
	gentity_t *bot = &g_entities[_client];
	static usercmd_t cmd;
	vec3_t angles, forward, right;
	memset(&cmd, 0, sizeof(cmd));

	cmd.identClient = _client;
	cmd.serverTime = level.time;

	// Bots dont need this flag, it causes problems, CYA
	bot->client->ps.pm_flags &= ~PMF_RESPAWNED;

	// Process the bot keypresses.
	if(_input->m_ButtonFlags & BOT_BUTTON_ATTACK1)
		cmd.buttons |= BUTTON_ATTACK;
	if(_input->m_ButtonFlags & BOT_BUTTON_WALK)
		cmd.buttons |= BUTTON_WALKING;
	if(_input->m_ButtonFlags & BOT_BUTTON_USE)
		cmd.buttons |= BUTTON_USE_HOLDABLE;
	if(_input->m_ButtonFlags & BOT_BUTTON_JUMP)
		cmd.upmove = 127;
	if(_input->m_ButtonFlags & BOT_BUTTON_CROUCH)
		cmd.upmove = -127;
	if(_input->m_ButtonFlags & BOT_BUTTON_RELOAD && (bot->client->ps.weaponstate == WEAPON_READY))
		obBotDoCommand(_client, "reload");

	// Convert the bots vector to angles and set the view angle to the orientation
	vectoangles(_input->m_Facing, angles);
	SetClientViewAngle(bot, angles);

	// Apparently this is how you calculate the usercmd_t angles
	//cmd.angles[PITCH] = ANGLE2SHORT(angles[PITCH]);
	//cmd.angles[YAW] = ANGLE2SHORT(angles[YAW]);
	//cmd.angles[ROLL] = ANGLE2SHORT(angles[ROLL]);
	//// subtract the delta angles
	//cmd.angles[0] -= bot->client->ps.delta_angles[0];
	//cmd.angles[1] -= bot->client->ps.delta_angles[1];
	//cmd.angles[2] -= bot->client->ps.delta_angles[2];

	// Apparently need to do this or bot->s.angles doesn't get set?
	//VectorCopy(angles, bot->s.angles);

	// Convert the move direction into forward and right moves to
	// take the bots orientation into account.
	AngleVectors(angles, forward, right, NULL);
	cmd.forwardmove = DotProduct(forward, _input->m_MoveDir) * 127;
	cmd.rightmove = DotProduct(right, _input->m_MoveDir) * 127;

	if(_input->m_ButtonFlags & BOT_BUTTON_RSTRAFE)
		cmd.rightmove = 127;
	if(_input->m_ButtonFlags & BOT_BUTTON_LSTRAFE)
		cmd.rightmove = -127;

	// Set the weapon
	if(!_input->m_CurrentWeapon)
		cmd.weapon = ETF_WP_AXE;
	else
		cmd.weapon = _input->m_CurrentWeapon;
	trap_BotUserCommand(_client, &cmd);

	//subtract the delta angles

	//bot->client->ps.viewangles[0] = AngleMod(bot->client->ps.viewangles[0] - SHORT2ANGLE(bot->client->ps.delta_angles[0]));
	//bot->client->ps.viewangles[1] = AngleMod(bot->client->ps.viewangles[1] - SHORT2ANGLE(bot->client->ps.delta_angles[1]));
	//bot->client->ps.viewangles[2] = AngleMod(bot->client->ps.viewangles[2] - SHORT2ANGLE(bot->client->ps.delta_angles[2]));

}

//-----------------------------------------------------------------

static const char *obGetClientName(int _client)
{
	if(g_entities[_client].client)
		return g_entities[_client].client->pers.netname;
	return 0;
}

//-----------------------------------------------------------------

static const char *obGetMapName()
{
	static char mapname[32];
	trap_Cvar_VariableStringBuffer( "mapname", mapname, sizeof(mapname) );
	return mapname;
}

//-----------------------------------------------------------------

static const char *obGetGameName() 
{
	return GAME_VERSION;
}

//-----------------------------------------------------------------

static const char *obGetModName() 
{
	return OMNIBOT_MODNAME;
}

//-----------------------------------------------------------------

static const char *obGetModVersion() 
{
	return FORTS_VERSION;
}

//-----------------------------------------------------------------

//////////////////////////////////////////////////////////////////

int Bot_Interface_Init(void)
{
	int iLoadResult = -1;
	memset(&g_InterfaceFunctions, 0, sizeof(g_InterfaceFunctions));

	g_InterfaceFunctions.pfnAddBot					= obAddbot;
	g_InterfaceFunctions.pfnRemoveBot				= obBotKickBot;	
	g_InterfaceFunctions.pfnChangeClass				= obBotChangeClass;
	g_InterfaceFunctions.pfnChangeTeam				= obBotChangeTeam;
	g_InterfaceFunctions.pfnGetClientPosition		= obGetClientPosition;
	g_InterfaceFunctions.pfnGetClientOrientation	= obGetClientOrientation;

	// Set up all the utility functions.
	g_InterfaceFunctions.pfnPrintError				= obPrintError;
	g_InterfaceFunctions.pfnPrintMessage			= obPrintMessage;
	g_InterfaceFunctions.pfnPrintScreenText			= obPrintScreenMessage;
	g_InterfaceFunctions.pfnTraceLine				= obBotTraceLine;
	g_InterfaceFunctions.pfnUpdateBotInput			= obUpdateBotInput;
	g_InterfaceFunctions.pfnBotCommand				= obBotDoCommand;
	g_InterfaceFunctions.pfnInterfaceSendMessage	= obInterfaceSendMessage;

	g_InterfaceFunctions.pfnGetMaxNumPlayers		= obGetMaxNumPlayers;
	g_InterfaceFunctions.pfnGetCurNumPlayers		= obGetCurNumPlayers;
	g_InterfaceFunctions.pfnGetMapName				= obGetMapName;
	g_InterfaceFunctions.pfnGetGameName				= obGetGameName;
	g_InterfaceFunctions.pfnGetModName				= obGetModName;
	g_InterfaceFunctions.pfnGetModVers				= obGetModVersion;
	g_InterfaceFunctions.pfnGetGameTime				= obGetGameTime;
	g_InterfaceFunctions.pfnGetClientName			= obGetClientName;
	g_InterfaceFunctions.pfnGetGoals				= obGetGoals;
	g_InterfaceFunctions.pfnGetThreats				= obGetThreats;

	// Entity info.
	g_InterfaceFunctions.pfnGetEntityFlags			= obGetEntityFlags;
	g_InterfaceFunctions.pfnGetEntityEyePosition	= obGetEntityEyePosition;
	g_InterfaceFunctions.pfnGetEntityBonePosition	= obGetEntityBonePosition;
	g_InterfaceFunctions.pfnGetEntityVelocity		= obGetEntityVelocity;
	g_InterfaceFunctions.pfnGetEntityPosition		= obGetEntityPosition;
	g_InterfaceFunctions.pfnGetEntityOrientation	= obGetEntityOrientation;
	g_InterfaceFunctions.pfnGetEntityWorldAABB		= obGetEntityWorldAABB;
	g_InterfaceFunctions.pfnGetEntityOwner			= obGetEntityOwner;

	g_InterfaceFunctions.pfnBotGetCurrentAmmo		= obBotGetCurrentAmmo;
	g_InterfaceFunctions.pfnBotGetWeaponAmmo		= obBotGetWeaponAmmo;

	g_InterfaceFunctions.pfnEntityFromID			= obEntityFromID;
	g_InterfaceFunctions.pfnIDFromEntity			= obIDFromEntity;

	// Waypoint functions.
	g_InterfaceFunctions.pfnAddDisplayWaypoint		= obAddDisplayWaypoint;
	g_InterfaceFunctions.pfnAddDisplayPath			= obAddDisplayPath;
	g_InterfaceFunctions.pfnAddTempDisplayLine		= obAddTempDisplayLine;
	g_InterfaceFunctions.pfnAddDisplayRadius		= obAddDisplayRadius;
	g_InterfaceFunctions.pfnClearDebugLines			= obClearNavLines;
	g_InterfaceFunctions.pfnFindEntityByClassName	= obFindEntityByClassName;
	g_InterfaceFunctions.pfnFindEntityInSphere		= obFindEntityInSphere;
	g_InterfaceFunctions.pfnPrintEntitiesInRadius	= obPrintEntitiesInRadius;

	// clear the debug arrays
	memset(&g_debugLines[0], 0, sizeof(g_debugLines));

	INITBOTLIBRARY("omnibot_etf.dll", "omnibot_etf.so", iLoadResult);
	if(iLoadResult != BOT_ERROR_NONE)
	{
		g_BotLoaded = 0;
		obPrintError(BOT_ERR_MSG(iLoadResult));
		return 0;
	}

	g_BotLoaded = 1;
	return 1;
	// Load the bot dll and get the exported functions.
//#ifndef __linux__
//	// try loading lib from inside the et dir
//	BOTLIBRARY = LoadLibrary(".\\omni-bot\\omnibot_etf.dll");
//	// try loading from path otherwise
//	if(BOTLIBRARY == NULL) 
//		BOTLIBRARY = LoadLibrary("omnibot_etf.dll");
//
//	if(BOTLIBRARY == NULL) 
//	{
//		TCHAR szBuf[80]; 
//		LPVOID lpMsgBuf;
//		DWORD dw = GetLastError(); 
//		FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
//			NULL, dw, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR) &lpMsgBuf, 0, NULL );
//		wsprintf(szBuf, "Omni-bot DLL load failure because: %s", lpMsgBuf); 
//		obPrintError(szBuf);
//		return 0;
//	}
//#else
//	BOTLIBRARY = dlopen("./omnibot_etf.so", RTLD_NOW);
//	dlerror();
//#endif
//
//	// Zero the bot functions struct.
//	memset(&g_BotFunctions, 0, sizeof(g_BotFunctions));
//
//	if(BOTLIBRARY)
//	{
//		obPrintMessage("Omni-bot Dll Loaded, getting functions...");
//		pfnGetBotFuncs = (pfnGetFunctionsFromDLL)GetProcAddress(BOTLIBRARY, "ExportBotFunctionsFromDLL");
//	} else
//	{
//		obPrintError("Unable to load Omni-bot DLL, Bots DISABLED.");
//		return 0;
//	}
//
//	// Make sure everything went well.
//#ifndef __linux__
//	if (!pfnGetBotFuncs)
//#else
//	if (dlerror())
//#endif
//	{
//		obPrintError("Unable to load Omni-bot DLL, Bots DISABLED.");
//		return 0;
//	}
//
//	obPrintMessage("Omni-bot DLL Loaded, Bots ENABLED.");
//
//	// Get all the bot functions from the bot.
//	pfnGetBotFuncs(&g_BotFunctions);
//
//	// Pass the list of functions to the bot for initialisation.
//	if(g_BotFunctions.pfnBotInitialise(NAVID_WP, &funcs) == 1)
//	{
//		g_BotLoaded = 1;
//		return 1;
//	} else
//		return 0;
}

//-----------------------------------------------------------------

int Bot_Interface_Shutdown( void )
{
	if(g_BotLoaded)
		g_BotFunctions.pfnBotShutdown();
	SHUTDOWNBOTLIBRARY;
	return 1;
}

//-----------------------------------------------------------------

void Bot_Interface_Update( )
{
	if(g_BotLoaded)
	{
		char buf[1024] = {0};
		int i;

		for(i = 0; i < MAX_CLIENTS; i++)
		{
			if(!g_entities[i].inuse)
				continue;
			if (!g_entities[i].client)
				continue;
			if(g_entities[i].client->pers.connected != CON_CONNECTED)
				continue;
			if (Q3F_IsSpectator(g_entities[i].client) && 
				(g_entities[i].client->sess.spectatorState == SPECTATOR_FOLLOW))
			{
				int iDestination = g_entities[i].client->sess.spectatorClient;
				Bot_Interface_SendEvent(MESSAGE_SPECTATED, iDestination, i, 0, NULL);
			}
			if (!(g_entities[i].r.svFlags & SVF_BOT))
			{
				// Waypoints should display for non-bots only.
				obUpdateDrawnWaypoints(i, 1000.0f);
				continue;
			}
			// fake handle server commands (to prevent server command overflow)
			while (trap_BotGetServerCommand(i, buf, sizeof(buf))) { }
		}

		// Call the libraries update.
		g_BotFunctions.pfnBotUpdate();
	}	
}

//-----------------------------------------------------------------

int Bot_Interface_ConsoleCommand()
{
	if(g_BotLoaded)
	{
		char token[MAX_TOKEN_CHARS] = {0};
		char buffer[MAX_TOKEN_CHARS] = {0};
		int ipos = 0;
		int i = 1;

		// TEMPORARY REMOVE THIS
#ifdef _DEBUG
		trap_Argv(1, token, sizeof(token));
		if(!Q_stricmp(token, "search"))
		{
			float rad;
			trap_Argv(2, token, sizeof(token));
			rad = atof(token);

			obPrintEntitiesInRadius(g_entities[0].r.currentOrigin, rad);
			return 1;
		}
#endif
		////////////
		do
		{
			// get the next token
			trap_Argv(i++, token, sizeof(token));
			// append it to the buffer
			strncpy(&buffer[ipos], token, strlen(token));
			ipos += strlen(token);
			buffer[ipos++] = ' ';
		} while(token[0]);

		assert(g_BotFunctions.pfnBotConsoleCommand);
		return g_BotFunctions.pfnBotConsoleCommand(buffer, strlen(buffer));
	}	
	return 1;
}

//-----------------------------------------------------------------

void Bot_Interface_LogOutput(const char *_txt)
{
	if(g_BotLoaded)	
		g_BotFunctions.pfnBotLog(_txt);
}

//-----------------------------------------------------------------

void Bot_Interface_SendEvent(int _eid, int _dest, int _source, float _delay, BotUserData * _data)
{
	if(g_BotLoaded)
		g_BotFunctions.pfnBotSendEvent(_eid, _dest, _source, _delay, _data);
}

//-----------------------------------------------------------------

void Bot_Interface_SendGlobalEvent(int _eid, int _source, float _delay, BotUserData * _data)
{
	if(g_BotLoaded)
		g_BotFunctions.pfnBotSendGlobalEvent(_eid, _source, _delay, _data);
}
//-----------------------------------------------------------------

void Bot_SendTrigger(const TriggerInfo *_triggerInfo)
{
	if(g_BotLoaded)
		g_BotFunctions.pfnBotSendTrigger(_triggerInfo);
}


#endif


