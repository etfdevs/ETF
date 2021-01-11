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

//
#ifdef PERFLOG
#ifdef QAGAME
#include "g_local.h"
#else
#include "../cgame/cg_local.h"
#endif

#define MAX_PERFORMANCE_DEPTH 128

static fileHandle_t fPerMon;
static qboolean		perMonInited = qfalse;
static int			tabCount = 0;
static int			depth = 0;
static int			overflow = 0;

typedef struct {
	int time;
	const char* funcName;
} timeStamp_t;

static timeStamp_t timeStamps[MAX_PERFORMANCE_DEPTH];

void BG_Q3F_PerMonLogPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		string[1024];
	int			i;
	char		*s = string;

	if ( !perMonInited ) {
		return;
	}

	for(i = 0; i < tabCount; i++) {
		*s++ = '\t';
	}

	va_start( argptr, fmt );
	//vsprintf( string + tabCount , fmt, argptr );
	Q_vsnprintf( string + tabCount, sizeof(string), fmt, argptr );
	va_end( argptr );

	trap_FS_Write( string, strlen(string), fPerMon );
}

void BG_Q3F_PerformanceMonitorInit(char* filename) {
	trap_FS_FOpenFile(filename, &fPerMon, FS_APPEND);
	if(!fPerMon) {
		return;
	}
	
	perMonInited = qtrue;

	BG_Q3F_PerMonLogPrintf("Initing Perfomance Log\n");
	tabCount++;
}

void BG_Q3F_PerformanceMonitorShutdown() {
	if(!perMonInited) {
		return;
	}

	tabCount--;
	BG_Q3F_PerMonLogPrintf("Closing Perfomance Log\n");

	perMonInited = qfalse;

	trap_FS_FCloseFile(fPerMon);
}

void BG_Q3F_PerformanceMonitor_LogFunction(const char* funcName) {
	if(depth == MAX_PERFORMANCE_DEPTH) {
		overflow++;
		return;
	}

	timeStamps[depth].funcName	= funcName;
	timeStamps[depth].time		= trap_Milliseconds();

	BG_Q3F_PerMonLogPrintf("Logging Function: %s at time: %d\n", funcName, timeStamps[depth].time);
	
	tabCount++;
	depth++;	
}

void BG_Q3F_PerformanceMonitor_LogFunctionUpdate() {
	BG_Q3F_PerMonLogPrintf("Log For Function: %s Update. Time Taken: %d\n", timeStamps[depth-1].funcName, trap_Milliseconds() - timeStamps[depth-1].time);
}

void BG_Q3F_PerformanceMonitor_LogFunctionStop() {
	if(overflow) {
		overflow--;
		return;
	}

	tabCount--;
	depth--;

	BG_Q3F_PerMonLogPrintf("Log For Function: %s Complete. Time Taken: %d\n", timeStamps[depth].funcName, trap_Milliseconds() - timeStamps[depth].time);
}

static int traceCount;

void BG_Q3F_LogTrace() {
	traceCount++;
}

void BG_Q3F_FlushTraceBuffer() {
	BG_Q3F_PerMonLogPrintf("PmoveCount: %d\n", traceCount);

	traceCount = 0;
}
#endif	// PERFLOG
