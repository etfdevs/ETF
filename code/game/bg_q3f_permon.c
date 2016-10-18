// Copyright (C) 1999-2000 Id Software, Inc.
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
