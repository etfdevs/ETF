/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

Enemy Territory Fortress
Copyright (C) 2000-2006 Quake III Fortress (Q3F) Development Team / Splash Damage Ltd.
Copyright (C) 2005-2026 Enemy Territory Fortress Development Team

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

#include "../../game/g_local.h"

static dllSyscall_t syscall = (dllSyscall_t)-1;

Q_EXPORT void dllEntry( dllSyscall_t syscallptr ) {
	syscall = syscallptr;
}

uintptr_t G_GetSyscall(void) {
    return (uintptr_t)syscall;
}

/*static int PASSFLOAT( float x ) {
	byteAlias_t fi;
	fi.f = x;
	return fi.i;
}*/

#include "../shared/g_shared_syscalls.c"

void	trap_Print( const char *str ) {
	SystemCall( G_PRINT, str );
}

void NORETURN trap_Error( const char *str ) {
	SystemCall( G_ERROR, str );
	exit( 1 );
}

int		trap_Milliseconds( void ) {
	return SystemCall_NoArgs( G_MILLISECONDS );
}
int		trap_Argc( void ) {
	return SystemCall_NoArgs( G_ARGC );
}

void	trap_Argv( int n, char *buffer, int bufferLength ) {
	SystemCall( G_ARGV, n, buffer, bufferLength );
}

int		trap_FS_FOpenFile( const char *qpath, fileHandle_t *f, fsMode_t mode ) {
	return SystemCall( G_FS_FOPEN_FILE, qpath, f, mode );
}

void	trap_FS_Read( void *buffer, int len, fileHandle_t f ) {
	SystemCall( G_FS_READ, buffer, len, f );
}

int		trap_FS_Write( const void *buffer, int len, fileHandle_t f ) {
	return SystemCall( G_FS_WRITE, buffer, len, f );
}

int		trap_FS_Rename( const char *from, const char *to ) {
	return SystemCall( G_FS_RENAME, from, to );
}

void	trap_FS_FCloseFile( fileHandle_t f ) {
	SystemCall( G_FS_FCLOSE_FILE, f );
}

int trap_FS_GetFileList(  const char *path, const char *extension, char *listbuf, int bufsize ) {
	return SystemCall( G_FS_GETFILELIST, path, extension, listbuf, bufsize );
}

void	trap_SendConsoleCommand( int exec_when, const char *text ) {
	SystemCall( G_SEND_CONSOLE_COMMAND, exec_when, text );
}

void	trap_Cvar_Register( vmCvar_t *cvar, const char *var_name, const char *value, int flags, int extflags ) {
	if ( cvar_notabcomplete && (extflags & EXT_CVAR_NOTABCOMPLETE) ) {
		flags |= cvar_notabcomplete;
	}
	if ( cvar_nodefault && (extflags & EXT_CVAR_NODEFAULT) ) {
		flags |= cvar_nodefault;
	}
	if ( cvar_archive_nd && (extflags & EXT_CVAR_ARCHIVE_ND) ) {
		flags |= cvar_archive_nd;
	}
	else if (!(flags & CVAR_ARCHIVE) && (extflags & EXT_CVAR_ARCHIVE_ND)) {
		// unsupported archive_nd so still add archive
		flags |= CVAR_ARCHIVE;
	}
	if ( cvar_developer && (extflags & EXT_CVAR_DEVELOPER) ) {
		flags |= cvar_developer;
	}

	SystemCall( G_CVAR_REGISTER, cvar, var_name, value, flags );
}

void	trap_Cvar_Update( vmCvar_t *cvar ) {
	SystemCall( G_CVAR_UPDATE, cvar );
}

void trap_Cvar_Set( const char *var_name, const char *value ) {
	SystemCall( G_CVAR_SET, var_name, value );
}

int trap_Cvar_VariableIntegerValue( const char *var_name ) {
	return SystemCall( G_CVAR_VARIABLE_INTEGER_VALUE, var_name );
}

void trap_Cvar_VariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	SystemCall( G_CVAR_VARIABLE_STRING_BUFFER, var_name, buffer, bufsize );
}

void trap_Cvar_LatchedVariableStringBuffer( const char *var_name, char *buffer, int bufsize ) {
	SystemCall( G_CVAR_LATCHEDVARIABLESTRINGBUFFER, var_name, buffer, bufsize );
}

void trap_LocateGameData( gentity_t *gEnts, int numGEntities, int sizeofGEntity_t,
						 playerState_t *clients, int sizeofGClient ) {
	SystemCall( G_LOCATE_GAME_DATA, gEnts, numGEntities, sizeofGEntity_t, clients, sizeofGClient );
}

/*void trap_DropClient( int clientNum, const char *reason, int length ) {
	SystemCall( G_DROP_CLIENT, clientNum, reason, length );
}*/

void trap_SendServerCommand( int clientNum, const char *text ) {
	// rain - #433 - commands over 1022 chars will crash the
	// client engine upon receipt, so ignore them
	// CHRUKER: b001 - Truncating the oversize server command before writing it to the log
	if( strlen( text ) > 1022 ) {
		G_LogPrintf( "trap_SendServerCommand( client %d ) length exceeds 1022.\n", clientNum );
		G_LogPrintf( "text [%.950s]... truncated\n", text ); 
		return;
	}
	SystemCall( G_SEND_SERVER_COMMAND, clientNum, text );
}

void trap_SetConfigstring( int num, const char *string ) {
	SystemCall( G_SET_CONFIGSTRING, num, string );
}

void trap_GetConfigstring( int num, char *buffer, int bufferSize ) {
	SystemCall( G_GET_CONFIGSTRING, num, buffer, bufferSize );
}

void trap_GetUserinfo( int num, char *buffer, int bufferSize ) {
	SystemCall( G_GET_USERINFO, num, buffer, bufferSize );
}

void trap_SetUserinfo( int num, const char *buffer ) {
	SystemCall( G_SET_USERINFO, num, buffer );
}

void trap_GetServerinfo( char *buffer, int bufferSize ) {
	SystemCall( G_GET_SERVERINFO, buffer, bufferSize );
}

void trap_SetBrushModel( gentity_t *ent, const char *name ) {
	SystemCall( G_SET_BRUSH_MODEL, ent, name );
}

void trap_Trace( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	SystemCall( G_TRACE, results, start, mins, maxs, end, passEntityNum, contentmask );
}

void trap_TraceNoEnts( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	SystemCall( G_TRACE, results, start, mins, maxs, end, -2, contentmask );
}

void trap_TraceCapsule( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	SystemCall( G_TRACECAPSULE, results, start, mins, maxs, end, passEntityNum, contentmask );
}

void trap_TraceCapsuleNoEnts( trace_t *results, const vec3_t start, const vec3_t mins, const vec3_t maxs, const vec3_t end, int passEntityNum, int contentmask ) {
	SystemCall( G_TRACECAPSULE, results, start, mins, maxs, end, -2, contentmask );
}

int trap_PointContents( const vec3_t point, int passEntityNum ) {
	return SystemCall( G_POINT_CONTENTS, point, passEntityNum );
}


qboolean trap_InPVS( const vec3_t p1, const vec3_t p2 ) {
	return SystemCall( G_IN_PVS, p1, p2 );
}

qboolean trap_InPVSIgnorePortals( const vec3_t p1, const vec3_t p2 ) {
	return SystemCall( G_IN_PVS_IGNORE_PORTALS, p1, p2 );
}

void trap_AdjustAreaPortalState( gentity_t *ent, qboolean open ) {
	SystemCall( G_ADJUST_AREA_PORTAL_STATE, ent, open );
}

qboolean trap_AreasConnected( int area1, int area2 ) {
	return SystemCall( G_AREAS_CONNECTED, area1, area2 );
}

void trap_LinkEntityExt( gentity_t *ent, const char *file, int line ) {
	if(trap_Cvar_VariableIntegerValue("developer") && ent /*&& ent->inuse */&& !ent->r.bmodel && VectorCompare(ent->r.currentOrigin, vec3_origin)) {
		G_Printf("WARNING: BBOX entity {%s} is being linked at world origin, this is probably a bug\n", ent->classname);
		if (ent->s.eType > ET_EVENTS) {
			G_Printf("TempEntity Event: %d [%s]\n", ent->s.eType - ET_EVENTS, eventnames[ent->s.eType - ET_EVENTS]);
		}
		G_Printf("File: %s, Line: %i\n", file, line );
	}

	SystemCall( G_LINKENTITY, ent );
}

void trap_UnlinkEntity( gentity_t *ent ) {
	SystemCall( G_UNLINKENTITY, ent );
}


int trap_EntitiesInBox( const vec3_t mins, const vec3_t maxs, int *list, int maxcount ) {
	return SystemCall( G_ENTITIES_IN_BOX, mins, maxs, list, maxcount );
}

qboolean trap_EntityContact( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return SystemCall( G_ENTITY_CONTACT, mins, maxs, ent );
}

qboolean trap_EntityContactCapsule( const vec3_t mins, const vec3_t maxs, const gentity_t *ent ) {
	return SystemCall( G_ENTITY_CONTACTCAPSULE, mins, maxs, ent );
}

int trap_BotAllocateClient( int clientNum ) {
	return SystemCall( G_BOT_ALLOCATE_CLIENT, clientNum );
}

void trap_BotFreeClient( int clientNum ) {
	SystemCall( G_BOT_FREE_CLIENT, clientNum );
}

int trap_GetSoundLength(sfxHandle_t sfxHandle) {
	return SystemCall( G_GET_SOUND_LENGTH, sfxHandle );
}

sfxHandle_t	trap_RegisterSound( const char *sample, qboolean compressed ) {
	return SystemCall( G_REGISTERSOUND, sample, compressed );
}

void trap_GetUsercmd( int clientNum, usercmd_t *cmd ) {
	SystemCall( G_GET_USERCMD, clientNum, cmd );
}

qboolean trap_GetEntityToken( char *buffer, int bufferSize ) {
	return SystemCall( G_GET_ENTITY_TOKEN, buffer, bufferSize );
}

int trap_DebugPolygonCreate(int color, int numPoints, vec3_t *points) {
	return SystemCall( G_DEBUG_POLYGON_CREATE, color, numPoints, points );
}

void trap_DebugPolygonDelete(int id) {
	SystemCall( G_DEBUG_POLYGON_DELETE, id );
}

int trap_RealTime( qtime_t *qtime ) {
	return SystemCall( G_REAL_TIME, qtime );
}

void trap_SnapVector( float *v ) {
	SystemCall( G_SNAPVECTOR, v );
}

qboolean trap_GetTag( int clientNum, int tagFileNumber, char *tagName, orientation_t *ori ) {
	return SystemCall( G_GETTAG, clientNum, tagFileNumber, tagName, ori );
}

qboolean trap_LoadTag( const char* filename ) {
	return SystemCall( G_REGISTERTAG, filename );
}

// BotLib traps start here
int trap_PC_AddGlobalDefine( const char *define ) {
	return SystemCall( BOTLIB_PC_ADD_GLOBAL_DEFINE, define );
}

int trap_PC_LoadSource( const char *filename ) {
	return SystemCall( BOTLIB_PC_LOAD_SOURCE, filename );
}

int trap_PC_FreeSource( int handle ) {
	return SystemCall( BOTLIB_PC_FREE_SOURCE, handle );
}

int trap_PC_ReadToken( int handle, pc_token_t *pc_token ) {
	return SystemCall( BOTLIB_PC_READ_TOKEN, handle, pc_token );
}

int trap_PC_SourceFileAndLine( int handle, char *filename, int *line ) {
	return SystemCall( BOTLIB_PC_SOURCE_FILE_AND_LINE, handle, filename, line );
}

int trap_PC_UnReadToken( int handle ) {
	return SystemCall( BOTLIB_PC_UNREAD_TOKEN, handle );
}

int trap_BotGetSnapshotEntity( int clientNum, int sequence ) {
	return SystemCall( BOTLIB_GET_SNAPSHOT_ENTITY, clientNum, sequence );
}

int trap_BotGetServerCommand(int clientNum, char *message, int size) {
	return SystemCall( BOTLIB_GET_CONSOLE_MESSAGE, clientNum, message, size );
}

void trap_BotUserCommand(int clientNum, usercmd_t *ucmd) {
	SystemCall( BOTLIB_USER_COMMAND, clientNum, ucmd );
}

void trap_EA_Command(int client, const char *command) {
	SystemCall( BOTLIB_EA_COMMAND, client, command );
}

void trap_PbStat ( int clientNum , char *category , char *values ) {
	SystemCall( PB_STAT_REPORT , clientNum , category , values ) ;
}

void trap_SendMessage( int clientNum, char *buf, int buflen ) {
	SystemCall( G_SENDMESSAGE, clientNum, buf, buflen );
}

messageStatus_t trap_MessageStatus( int clientNum ) {
	return (messageStatus_t)SystemCall( G_MESSAGESTATUS, clientNum );
}

// extension interface

qboolean trap_GetValue( char *value, int valueSize, const char *key ) {
	return (qboolean)SystemCall( dll_com_trapGetValue, value, valueSize, key );
}

int trap_FS_Delete( const char *filename ) {
	return SystemCall( dll_trap_FS_Delete, filename );
}
