/*
**	cg_q3f_sounddict.h	
**
**	Plays 'sentences' from client-side-parsed strings.
*/

#ifndef	__CG_Q3F_SOUNDDICT_H
#define	__CG_Q3F_SOUNDDICT_H

#include "cg_local.h"

#define	MAX_SOUND_DICT			200						// Maximum of 200 dictionary entries
#define	MAX_SOUND_DICT_BUFF		16384					// Maximum of 16k of strings
#define	SOUND_DICT_FILE			"sound/sounddict.txt"	// Dictionary of sounds
#define SOUND_DICT_PERIOD_TIME	500						// Pause on periods
#define SOUND_DICT_COMMA_TIME	300						// Pause on commas
#define SOUND_DICT_SPACE_TIME	0						// Pause between words

typedef struct cg_q3f_sounddict_s {
	char *key, *path;
	int time;
	sfxHandle_t handle;
} cg_q3f_sounddict_t;

extern cg_q3f_sounddict_t cg_q3f_sounddict[MAX_SOUND_DICT]; // Ensiform - Type was missing!

void CG_Q3F_ParseSoundDictionary();
void CG_Q3F_StartSoundString( const char *str );
void CG_Q3F_PlaySoundDict();

#endif

