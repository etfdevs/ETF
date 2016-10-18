#include "q_shared.h"
#include "bg_public.h"

#ifdef QAGAME
#include "g_local.h"
#else
#include "../ui_new/ui_shared.h"
#endif

const char *String_Alloc(const char *p);

typedef struct mapInfoKeyHandler_s
{
	char *keyname;
	qboolean (*func)(mapInfo* info, char* value, char** buf);
} mapInfoKeyHandler_t;

static qboolean MapInfo_KeyHandlerMap(mapInfo* info, char* value, char** buf) {
	info->mapName = String_Alloc(value);
	info->imageName = String_Alloc(va("levelshots/%s", value));
	info->levelShot = -1;
	info->cinematic = -1;
	return qtrue;
}

static qboolean MapInfo_KeyHandlerLongName(mapInfo* info, char* value, char** buf) {
	info->mapName = String_Alloc(value);
	return qtrue;
}

static qboolean MapInfo_KeyHandlerGameIndicies(mapInfo* info, char* value, char** buf) {
	info->gameIndicies = String_Alloc(value);
	return qtrue;
}

static qboolean MapInfo_KeyHandlerType(mapInfo* info, char* value, char** buf) {
	if( strstr( value, MAPINFO_TYPE ) ) {
		return qtrue;
	}
	return qfalse;
}

static qboolean GameInDex_KeyHandlerLongName(gameIndexInfo_t* info, char* value, char** buf) {
	Q_strncpyz(info->name, va("%d: %s", info->number, value), 256);

	return qtrue;
}

static qboolean GameInDex_KeyHandlerMapinfo(gameIndexInfo_t* info, char* value, char** buf) {
	char buff[1024];
	int i;
	char *p;
	
	for(i = 0, p = value; *p && i < 1024-1; p++, i++) {
		if( *p == '\\') {
			p++;
			switch(*p) {
				case '\\': buff[i] = '\\'; 
					break;
				case 'n': buff[i] = '\n';
					break;
				default:
					i--;
			}
		} else {
			buff[i] = *p;
		}
	}
	buff[i] = '\0';

	info->description = String_Alloc(buff);

	return qtrue;
}

typedef struct gameIndexKeyHandler_s
{
	char *keyname;
	qboolean (*func)(gameIndexInfo_t* info, char* value, char** buf);
} gameIndexKeyHandler_t;

gameIndexKeyHandler_t gameIndexDefKeyHandlers[] = {
	{"longname",		GameInDex_KeyHandlerLongName},
	{"mapinfo",			GameInDex_KeyHandlerMapinfo},
	{NULL, NULL}
};

static qboolean MapInfo_KeyHandlerGameIndexDef(mapInfo* info, char* value, char** buf) {
	char	*token;
	char	key[MAX_TOKEN_CHARS];
	gameIndexKeyHandler_t* handler;

	info->gameIndiciesInfo[info->numGameIndicies].number = atoi(value);
	info->gameIndiciesInfo[info->numGameIndicies].description = NULL;

	info->numGameIndicies++;

	token = COM_Parse( buf );
	if ( !token[0] ) {
		return qfalse;
	}
	
	if ( strcmp( token, "{" ) ) {
		COM_ParseError( "Missing { in info file" );
		//Com_Printf( "Missing { in info file\n" );
		return qfalse;
	}

	while ( 1 ) {
		token = COM_ParseExt( buf, qtrue );
		if ( !token[0] ) {
			COM_ParseError( "Unexpected end of info file" );
			//Com_Printf( "Unexpected end of info file\n" );
			return qfalse;
		}

		if ( !strcmp( token, "}" ) ) {
			return qtrue;
		}
		Q_strncpyz( key, token, sizeof( key ) );

		token = COM_ParseExt( buf, qfalse );
		if ( !token[0] ) {
			strcpy( token, "<NULL>" );		
		}

		for(handler = gameIndexDefKeyHandlers; handler->keyname; handler++) {
			if(!Q_stricmp(key, handler->keyname)) {
				if(!handler->func(&info->gameIndiciesInfo[info->numGameIndicies-1], token, &value))
					return qfalse;
				break;
			}
		}
	}
}


mapInfoKeyHandler_t mapInfoKeyHandlers[] = {
	{"map",				MapInfo_KeyHandlerMap},
	{"longname",		MapInfo_KeyHandlerLongName},
	{"type",			MapInfo_KeyHandlerType},
	{"gameindices",		MapInfo_KeyHandlerGameIndicies},
	{"gameindexDef",	MapInfo_KeyHandlerGameIndexDef},
	{NULL, NULL}
};

/*
===============
BG_ParseInfos
===============
*/
qboolean BG_ParseInfos( char *buf, mapInfo* miList, int* index) {
	char	*token;
	char	key[MAX_TOKEN_CHARS];
	mapInfoKeyHandler_t* handler;

	memset(&miList[*index], 0, sizeof(mapInfo));

	token = COM_Parse( &buf );
	if ( !token[0] ) {
		return qfalse;
	}
	
	if ( Q_stricmp( token, "mapinfo" ) ) {
		COM_ParseError( "File is not a valid mapinfo" );
		//Com_Printf( "File is not a valid mapinfo\n" );
		return qfalse;
	}

	token = COM_Parse( &buf );
	if ( !token[0] ) {
		return qfalse;
	}
	
	if ( Q_stricmp( token, "{" ) ) {
		COM_ParseError( "Missing { in info file" );
		//Com_Printf( "Missing { in info file\n" );
		return qfalse;
	}

	while ( 1 ) {
		token = COM_ParseExt( &buf, qtrue );
		if ( !token[0] ) {
			COM_ParseError( "Unexpected end of info file" );
			//Com_Printf( "Unexpected end of info file\n" );
			return qfalse;
		}
		if ( !Q_stricmp( token, "}" ) ) {
			(*index)++;
			return qtrue;
		}
		Q_strncpyz( key, token, sizeof( key ) );

		token = COM_ParseExt( &buf, qfalse );
		if ( !token[0] ) {
			strcpy( token, "<NULL>" );		
		}

		for(handler = mapInfoKeyHandlers; handler->keyname; handler++) {
			if(!Q_stricmp(key, handler->keyname)) {
				if(!handler->func(&miList[*index], token, &buf))
					return qfalse;
				break;
			}
		}
	}
}

/*
===============
BG_LoadMapInfoFromFile
===============
*/
qboolean BG_LoadMapInfoFromFile( char *filename, displayContextDef_t* DC, mapInfo* miList, int* index ) {
	int				len;
	fileHandle_t	f;
	char			buf[MAX_MAPINFOS_TEXT];
	char			rawmapname[1024];
	//char* p;

	len = DC->openFile( filename, &f, FS_READ );
	if ( !f ) {
		DC->Print( va( S_COLOR_RED "file not found: %s\n", filename ) );
		return( qfalse );
	}
	if ( len >= MAX_MAPINFOS_TEXT ) {
		DC->Print( va( S_COLOR_RED "file too large: %s is %i, max allowed is %i", filename, len, MAX_MAPINFOS_TEXT ) );
		DC->closeFile( f );
		return( qfalse );
	}

	DC->fRead( buf, len, f );
	buf[len] = 0;
	DC->closeFile( f );

	/* Ensiform - Add this so we can check what files cause the Unexpected end of info file error */
	COM_BeginParseSession(filename);
	if(BG_ParseInfos(buf, miList, index)) {
		COM_StripExtension( COM_SkipPath( filename ), rawmapname, sizeof(rawmapname) );
		miList[(*index)-1].mapLoadName = String_Alloc( rawmapname );
	}

	return( qtrue );
}

