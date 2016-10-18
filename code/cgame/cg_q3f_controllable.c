/*
**	cg_q3f_controllable.c
**
**	Client-side code for handling 'controllable' objects, such as vehicles
**	or gun turrets.
*/

#include "cg_local.h"
#include "../game/bg_q3f_controllable.h"

#define CUSTOM_ITEM_MAX	32
#define CUSTOM_STRING_MAX 2048

static char Custom_StringSpace[CUSTOM_STRING_MAX];
static int Custom_StringUsed;
static struct {
	int key;
	char *keystring;
	char *title;
	char *command;
} Custom_Items[CUSTOM_ITEM_MAX];
static int Custom_ItemsUsed;

qboolean G_FS_ReadToken( fileHandle_t handle, int *filedone, int filelen, char *buffer, int size ) {
	char c;
	int oldsize = size;
	qboolean slash = qfalse;
	qboolean quote = qfalse;
	qboolean started = qfalse;
	if (!size)
		return qfalse;
	while (*filedone < filelen && size>1) {
		(*filedone)++;
		trap_FS_Read(&c, 1 , handle );
		switch (c) {
		case 0:
			return qfalse;
		case 10:
		case 13:
			if (!started)
				break;
			goto thatsit;
		case '"':
			if (quote)
				goto thatsit;
            quote = qtrue;           
			break;
		case '\t':
		case ' ':
			if (quote)  {
				*buffer++ = c;
				size--;
			} else if (started)
				goto thatsit;
			break;
		default:
			started = qtrue;
			*buffer++ = c;
			size--;
		}
	}
thatsit:
	*buffer = 0;
	return size < oldsize;
}


void CG_Q3F_CustomMenuShow( const char * filename ) {
	int  index, len;
	fileHandle_t handle;
	int filelen, filedone;
	char buffer[1024], *line;
	char fullname[MAX_QPATH];

	Custom_StringUsed = 0;
	Custom_ItemsUsed = 0;

	if (!filename[0]) {
		Q_strncpyz(fullname, "/ui/usermenu/default_main.cfg", MAX_QPATH);
	} else if (!strchr(filename, '/')) {
		Q_strncpyz(fullname, "/ui/usermenu/", MAX_QPATH);
		Q_strcat(fullname, MAX_QPATH, filename);
	} else {
        Q_strncpyz(fullname, filename, MAX_QPATH);
	}

	filelen = trap_FS_FOpenFile( fullname, &handle, FS_READ );
	if (!handle || !filelen) {
		Com_Printf("UserMenu failed to open %s\n", fullname );
		return;
	}
	index = 0;filedone = 0;
	while (G_FS_ReadToken( handle, &filedone, filelen, buffer, sizeof( buffer ))) {
		len = strlen(buffer) + 1;
		line = &Custom_StringSpace[Custom_StringUsed];
		if (len  > (CUSTOM_STRING_MAX - Custom_StringUsed)) {
			Com_Printf("Overflowed custom menu string space.\n");
			Custom_ItemsUsed = 0;
			return;
		}
		Custom_StringUsed += len;
		Q_strncpyz(line, buffer, len );
		switch ( index ) {
		case 0:
			Custom_Items[Custom_ItemsUsed].key = buffer[0];
			Custom_Items[Custom_ItemsUsed].keystring = "a";
			index = 1;
			break;
		case 1:
			Custom_Items[Custom_ItemsUsed].title = line;
			index = 2;
			break;
		case 2:
			Custom_Items[Custom_ItemsUsed].command = line;
			index = 0;
//			Custom_ItemsUsed++;
			if (++Custom_ItemsUsed >= CUSTOM_ITEM_MAX)
				break;
			break;
		}
	}
	trap_FS_FCloseFile( handle );
	if (!Custom_ItemsUsed) {
		Com_Printf("UserMenu failed to read anything from %s\n", fullname );
		return;
	}
	CG_EventHandling( CGAME_EVENT_CUSTOMMENU, qfalse );
}


qboolean CG_Q3F_CustomMenuExecKey( int key ) {
	int i;
	for (i=0;i<Custom_ItemsUsed;i++) {
		if ( Custom_Items[i].key == key )
			return qtrue;
	}
	return qfalse;
}

void CG_Q3F_CustomMenuKeyEvent( int key ) {
	int i;
	for (i=0;i<Custom_ItemsUsed;i++) {
		if ( Custom_Items[i].key == key ) {
			CG_EventHandling( CGAME_EVENT_NONE, qfalse );
			trap_SendConsoleCommand(Custom_Items[i].command);
		}
	}
}

int CG_Q3F_CustomMenuItems( void ) {
	if (cgs.eventHandling != CGAME_EVENT_CUSTOMMENU)
		return 0;
	if (cg.scoreBoardShowing)
		return 0;
	return Custom_ItemsUsed;
}

const char * CG_Q3F_CustomMenuGetItem(int index, int column) {
	static char buf[2];

	if (index < 0 || index >= Custom_ItemsUsed)
		return "";

	switch (column) {
	case 0:
		buf[0] = Custom_Items[index].key;
		buf[1] = 0;
		return buf;
//		return Custom_Items[index].keystring;
	case 1:
		return Custom_Items[index].title;
	}
	return "";
}