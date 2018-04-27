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
**	cg_q3f_sounddict.c	
**
**	Plays 'sentences' from client-side-parsed strings.
*/

#include "cg_q3f_sounddict.h"

static char dictbuff[MAX_SOUND_DICT_BUFF];
static cg_q3f_sounddict_t sounddict[MAX_SOUND_DICT];
static int dictbuffsize, dictsize;

static char currsoundstring[MAX_STRING_CHARS];
static char *soundptr;
static int soundendtime;
static qboolean playingsingleletters;

/*
**	Load and parse the map file
*/

static char *AddSoundString( char *str )
{
	// Attempt to add a string to the dictionary

	char *ptr;

	if( (strlen( str ) + dictbuffsize) >= MAX_SOUND_DICT_BUFF )
		CG_Error( "Sound string dictionary full." );
	ptr = &dictbuff[dictbuffsize];
	strcpy( ptr, str );
	dictbuffsize += strlen( str ) + 1;
	return( ptr );
}

// Little comparison function for qsort routine
static int QDECL PSD_SortFunc( const void *a, const void *b )
	{ return( strcmp( ((cg_q3f_sounddict_t *) a)->key, ((cg_q3f_sounddict_t *) b)->key ) ); }

static qboolean sounddictionaryparsed;
void CG_Q3F_ParseSoundDictionary()
{
	// Initialise the sound dictionary, and parse the text 'map' file.
	// This might be better loaded in all at once before parsing...

	int filelen, currpos, linepos;
	fileHandle_t f;
	char linebuff[MAX_STRING_CHARS], curr;
	qboolean isspace;
	char *keyptr, *timeptr, *pathptr;

	if( sounddictionaryparsed )
		return;
	sounddictionaryparsed = qtrue;

		// Reset string buffer.
	memset( dictbuff, 0, sizeof(dictbuff) );
	dictbuffsize = 0;
	dictsize = 0;

	filelen = trap_FS_FOpenFile( SOUND_DICT_FILE, &f, FS_READ );
	if( filelen <= 0 )
		return;

	for( currpos = 0; currpos < filelen; )
	{
			// Read in a line of data

		memset( linebuff, 0, sizeof(linebuff) );
		for( keyptr = timeptr = pathptr = NULL, isspace = qtrue, linepos = 0; currpos < filelen; )
		{
			trap_FS_Read( &curr, 1, f );
			currpos++;

			if( curr == '\n' )
				break;			// End of line
			if( curr == '#' )
				linepos = MAX_STRING_CHARS;		// Comment token, ignore rest of line
			if( linepos >= MAX_STRING_CHARS - 1 )
				continue;		// End of buffer, just drop silently
			if( curr <= ' ' || curr >= 128 )
			{
				// whitespace
				if( isspace )
					continue;
				isspace = qtrue;
				linebuff[linepos++] = 0;
			}
			else {
				// A character
				if( isspace )
				{
					if( !keyptr )
						keyptr = &linebuff[linepos];
					else if( !timeptr )
						timeptr = &linebuff[linepos];
					else if( !pathptr )
						pathptr = &linebuff[linepos];
				}
				isspace = qfalse;
				linebuff[linepos++] = curr;
			}
		}

			// Store the entry
		if( keyptr && pathptr )
		{
			linebuff[MAX_STRING_CHARS - 1] = 0;		// Just in case
			sounddict[dictsize].time	= atoi( timeptr );
			if( sounddict[dictsize].time <= 0 )
			{
				CG_Printf( BOX_PRINT_MODE_CHAT, "Sound dictionary: invalid time for '%s'.\n", keyptr );
				continue;		// Skip to next entry
			}
			sounddict[dictsize].key		= AddSoundString( keyptr );
			sounddict[dictsize].path	= AddSoundString( pathptr );
			sounddict[dictsize].handle	= (sfxHandle_t) -1;	// Not loaded yet
			for( keyptr = sounddict[dictsize].key; *keyptr; keyptr++ )
			{
				if( *keyptr >= 'A' && *keyptr <= 'Z' )
					*keyptr |= 32;	// Coerce key to lowercase
			}
			if( ++dictsize > MAX_SOUND_DICT )
				CG_Error( "Sound dictionary table full." );
		}
	}

	trap_FS_FCloseFile( f );

		// Now sort the dictionary for fast lookup
	qsort( sounddict, dictsize, sizeof(cg_q3f_sounddict_t), &PSD_SortFunc );
}


/*
**	Play the string.
*/

static qboolean StartSound( char *soundname )
{
	// Locate and start playing a sound, or return false

	int min, max, curr, cmp;
	cg_q3f_sounddict_t *ptr;
	char letterbuff[2];

	min = 0;
	max = dictsize - 1;

	if( playingsingleletters )
	{
		letterbuff[0] = *soundname;
		letterbuff[1] = 0;
		soundname = letterbuff;
	}

	while( min <= max )
	{
		curr = ((max - min) >> 1) + min;	// Get middle index
		ptr = &sounddict[curr];
		cmp = strcmp( soundname, ptr->key );
		if( cmp < 0 )
			max = curr - 1;					// Search lower half
		else if( cmp > 0 )
			min = curr + 1;					// Search upper half
		else {
			if( ptr->handle == (sfxHandle_t) -1 )
				ptr->handle = trap_S_RegisterSound( ptr->path, qfalse );
			if( !ptr->handle )
			{
				CG_Printf( BOX_PRINT_MODE_CHAT, "Failed to play sounddict sound '%s' ('%s').\n", ptr->key, ptr->path );
				soundendtime = cg.time;	// May be more strings to follow
				return( qfalse );
			}
			trap_S_StartLocalSound( ptr->handle, CHAN_ANNOUNCER );
			soundendtime = ptr->time + cg.time;
			return( qtrue );				// Found, return
		}
	}
	return( qfalse );
}

void CG_Q3F_PlaySoundDict()
{
	// Play a currently running sample

	char letterbuff[2];

	if( !soundendtime || soundendtime > cg.time )
		return;
	
	// Check to see if we're playing 'single letter' samples
	if( playingsingleletters && *++soundptr )
	{
		letterbuff[0] = *soundptr;
		letterbuff[1] = 0;
		StartSound( letterbuff );
		return;
	}

	// Find start of next sample
	while( *soundptr )
		soundptr++;

	if( !*++soundptr )
	{
		// Double NULL, indicates end of string

		soundendtime = 0;
		return;
	}

	if( *soundptr == ' ' )
	{
		soundendtime = cg.time +	SOUND_DICT_SPACE_TIME;
		return;
	}
	if( *soundptr == ',' )
	{
		soundendtime = cg.time +	SOUND_DICT_COMMA_TIME;
		return;
	}
	if( *soundptr == '.' )
	{
		soundendtime = cg.time +	SOUND_DICT_PERIOD_TIME;
		return;
	}

	playingsingleletters = qfalse;
	if( !StartSound( soundptr ) )
	{
		playingsingleletters = qtrue;
		StartSound( soundptr );
	}
}

/*
**	Preprocess and start a new string
*/

void CG_Q3F_StartSoundString( const char *str )
{
	int index;
	qboolean isspace, ispunct;
	char curr;

	memset( currsoundstring, 0, sizeof(currsoundstring) );
	soundptr = currsoundstring;
	soundendtime = 0;
	isspace = ispunct = qfalse;

	for( index = 0; *str && index < (MAX_STRING_CHARS - 3); str++ )
	{
		curr = *str;
		if( curr <= ' ' || curr >= 128 )
		{
			if( isspace )
				continue;
			isspace = qtrue;
			currsoundstring[index++] = 0;	// String seperator
		}
		else {
			if( curr >= 'A' && curr <= 'Z' )
				curr |= 32;
			if( (curr >= 'a' && curr <= 'z') || (curr >= '0' && curr <= '9') )
			{
				if( isspace && !ispunct && index )
				{
					// Put a space in between words
					currsoundstring[index++] = ' ';	// Space code
					currsoundstring[index++] = 0;	// String seperator
				}
				isspace = ispunct = qfalse;
				currsoundstring[index++] = curr;
			}
			else if( (curr == ',' || curr == '.') && !ispunct )
			{
				if( !isspace )
					currsoundstring[index++] = 0;
				currsoundstring[index++] = curr;
				currsoundstring[index++] = 0;
				isspace = qtrue;
				ispunct = qtrue;
			}
		}
	}

	playingsingleletters = qfalse;
	if( !StartSound( soundptr ) )
	{
		playingsingleletters = qtrue;
		StartSound( soundptr );
	}
}
