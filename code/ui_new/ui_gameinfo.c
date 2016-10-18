// Copyright (C) 1999-2000 Id Software, Inc.
//
//
// gameinfo.c
//

#include "ui_local.h"

qboolean BG_LoadMapInfoFromFile( char *filename, displayContextDef_t* DC, mapInfo* miList, int* index );

/*
=================
UI_ServersQsortCompare
=================
*/
static int QDECL UI_MapsQsortCompare( const void *arg1, const void *arg2 ) {
	mapInfo *a1;
	mapInfo *a2;

	a1 = (mapInfo *)arg1;
	a2 = (mapInfo *)arg2;

	return(Q_stricmp(a1->mapName, a2->mapName));
}


/*
===============
UI_ParseMapinfo
===============
*/
void UI_ParseMapInfo( void ) {
	int			numdirs;
	char		filename[128];
	char		dirlist[2048];
	char*		dirptr;
	int			i;
	int			dirlen;

	uiInfo.mapCount = 0;

	// get all mapinfos from .mapinfo files
	numdirs = trap_FS_GetFileList(MAPINFODIR, MAPINFOEXT, dirlist, 2048 );
	dirptr  = dirlist;
	for (i = 0; i < numdirs && uiInfo.mapCount < MAX_MAPS; i++, dirptr += dirlen+1) {
		dirlen = strlen(dirptr);
		//strcpy(filename, "maps/");
		//strcat(filename, dirptr);
		Com_sprintf( filename, sizeof(filename), "%s/%s", MAPINFODIR, dirptr );
		BG_LoadMapInfoFromFile(filename, &uiInfo.uiDC, uiInfo.mapList, &uiInfo.mapCount);
	}
	trap_Print( va( "%i mapinfos parsed\n", uiInfo.mapCount ) );
	if (UI_OutOfMemory()) {
		trap_Print(S_COLOR_YELLOW"WARNING: not enough memory in pool to load all mapinfos\n");
	}

	// slothy: sort map list
	qsort( &uiInfo.mapList[0], uiInfo.mapCount, sizeof(mapInfo), UI_MapsQsortCompare);
}

/*
=================
UI_ServersQsortCompare
=================
*/
static int QDECL UI_HudsQsortCompare( const void *arg1, const void *arg2 ) {
	return(Q_stricmp((char *)arg1, (char *)arg2));
}


/*
===============
UI_ParseHudInfo
===============
*/
void UI_ParseHudInfo( void ) {
	int			numdirs;
	char		filename[128];
	char		dirlist[2048];
	char*		dirptr;
	int			i;
	int			dirlen;
	char		curhud[128];
	char		hudname[128];
	char		wanthud[128];

	uiInfo.hudCount = 0;
	uiInfo.uiDC.curHudInt = 0;

	// find the current hud
	trap_Cvar_VariableStringBuffer("cg_hudfiles", filename, 128);
	COM_FixPath(filename);
	COM_StripExtension(filename, curhud, sizeof(curhud));
	dirptr = COM_SkipPath(curhud);
	Q_strncpyz(uiInfo.uiDC.curHudVariant, dirptr, 64);
	--dirptr;
	dirptr[0] = 0;
	dirptr = COM_SkipPath(curhud);
	Q_strncpyz(wanthud, dirptr, 128);

	// find all .hud files
	numdirs = trap_FS_GetFileList(HUDINFODIR, HUDINFOEXT, dirlist, sizeof(dirlist) );
	dirptr  = dirlist;
	for (i = 0; i < numdirs && uiInfo.hudCount < MAX_HUDS; i++, dirptr += dirlen+1) {
		dirlen = strlen(dirptr);
		if(strchr(dirptr, '/'))
			continue; 
		if(strchr(dirptr, '\\'))
			continue;
		Com_sprintf( filename, sizeof(filename), "%s/%s", HUDINFODIR, dirptr );
		// do something here to store hud name
		COM_StripExtension( dirptr, hudname, sizeof(hudname));
		Q_strncpyz(uiInfo.hudFiles[uiInfo.hudCount++], hudname, 64);
	}
#ifdef _DEBUG
	trap_Print( va( "%i huds found\n", uiInfo.hudCount ) );
#endif

	if(uiInfo.hudCount) {

		qsort( &uiInfo.hudFiles[0], uiInfo.hudCount, 64, UI_HudsQsortCompare);
		for(i = 0; i < uiInfo.hudCount; i++) {
			if(Q_stricmp(uiInfo.hudFiles[i], wanthud) == 0) {
				uiInfo.uiDC.curHudInt = i;
				break;
			}
		}
		// now to set:
		Q_strncpyz(uiInfo.uiDC.curHud, uiInfo.hudFiles[uiInfo.uiDC.curHudInt], 64);
		UI_ParseHudVariantInfo();
	} else {
		Q_strncpyz(uiInfo.uiDC.curHud, "", 64);
		uiInfo.hudVariationCount = 0;
		Q_strncpyz(uiInfo.uiDC.curHudVariant, "", 64);
		uiInfo.uiDC.curHudInt = -1;
	}
}

/*
===============
UI_ParseHudInfo
===============
*/
void UI_ParseHudVariantInfo( void ) {
	char			filename[128];
	char			want[64];
	int				handle;
	pc_token_t		token;

	uiInfo.hudVariationCount = 0;
	uiInfo.uiDC.curHudVarInt = 0;
	Q_strncpyz(want, uiInfo.uiDC.curHudVariant, 64);
	Q_strncpyz(uiInfo.uiDC.curHudVariant, "", 64);

	if(strlen(uiInfo.uiDC.curHud)) {

		Com_sprintf( filename, sizeof(filename), "%s/%s%s", HUDINFODIR, uiInfo.uiDC.curHud, HUDINFOEXT );

		handle = trap_PC_LoadSource(filename);
		if ( !handle ) {
			uiInfo.uiDC.Print( va( S_COLOR_RED "hud file not found: %s\n", uiInfo.uiDC.curHud ) );
			return;
		}

		if (!trap_PC_ReadToken(handle, &token)) {
			uiInfo.uiDC.Print( va( S_COLOR_RED "hud file empty: %s\n", uiInfo.uiDC.curHud ) );
			return;
		}

		while ( uiInfo.hudVariationCount < ETF_HUDVARCOUNT ) {
			Q_strncpyz(uiInfo.hudVariants[uiInfo.hudVariationCount], token.string, 64);

			if(Q_stricmp(token.string, want) == 0) 
				uiInfo.uiDC.curHudVarInt = uiInfo.hudVariationCount;

			++uiInfo.hudVariationCount;

			if (!trap_PC_ReadToken(handle, &token)) {
				break;
			}
		}

		trap_PC_FreeSource(handle);

#ifdef _DEBUG
		trap_Print( va( "%i huds variants found\n", uiInfo.hudVariationCount ) );
#endif
	}

	if(uiInfo.hudVariationCount) {
		// now to set:
		Q_strncpyz(uiInfo.uiDC.curHudVariant, uiInfo.hudVariants[uiInfo.uiDC.curHudVarInt], 64);
		uiInfo.uiDC.hudPreviewCin = -1;
	}
}

/*
===============
UI_ParseHudInfo
===============
*/
void UI_LoadHudPreview( void ) {

	if(strlen(uiInfo.uiDC.curHud)) {
		if(strlen(uiInfo.uiDC.curHudVariant)) {
			// start cinematic next time...
			uiInfo.uiDC.hudPreviewCin = -1;
		}
	}
}
