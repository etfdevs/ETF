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

