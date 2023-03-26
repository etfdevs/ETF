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

// q_shared.c -- stateless support routines that are included in each code dll
#include "q_shared.h"

/*
============================================================================

PARSING

============================================================================
*/

static	char	com_token[MAX_TOKEN_CHARS];
static	char	com_parsename[MAX_TOKEN_CHARS];
static	int		com_lines;
static  int		com_tokenline;

static int backup_lines;
static int backup_tokenline;
static const char    *backup_text;

void COM_BeginParseSession( const char *name )
{
	com_lines = 1;
	com_tokenline = 0;
	Com_sprintf(com_parsename, sizeof(com_parsename), "%s", name);
}


void COM_BackupParseSession( const char **data_p )
{
	backup_lines = com_lines;
	backup_tokenline = com_tokenline;
	backup_text = *data_p;
}


void COM_RestoreParseSession( const char **data_p )
{
	com_lines = backup_lines;
	com_tokenline = backup_tokenline;
	*data_p = backup_text;
}


/*void COM_SetCurrentParseLine( int line )
{
	com_lines = line;
}*/


int COM_GetCurrentParseLine( void )
{
	if ( com_tokenline )
	{
		return com_tokenline;
	}

	return com_lines;
}


const char *COM_Parse( const char **data_p )
{
	return COM_ParseExt( data_p, qtrue );
}


void FORMAT_PRINTF(1, 2) COM_ParseError( const char *format, ... )
{
	va_list argptr;
	static char string[4096];

	va_start( argptr, format );
	Q_vsnprintf (string, sizeof(string), format, argptr);
	va_end( argptr );

	Com_Printf( "ERROR: %s, line %d: %s\n", com_parsename, COM_GetCurrentParseLine(), string );
}


void FORMAT_PRINTF(1, 2) COM_ParseWarning( const char *format, ... )
{
	va_list argptr;
	static char string[4096];

	va_start (argptr, format);
	Q_vsnprintf (string, sizeof(string), format, argptr);
	va_end (argptr);

	Com_Printf("WARNING: %s, line %d: %s\n", com_parsename, com_lines, string);
}

/*
==============
COM_Parse

Parse a token out of a string
Will never return NULL, just empty strings

If "allowLineBreaks" is qtrue then an empty
string will be returned if the next token is
a newline.
==============
*/
static const char *SkipWhitespace( const char *data, qboolean *hasNewLines ) {
	int c;

	while( (c = *data) <= ' ') {
		if( !c ) {
			return NULL;
		}
		if( c == '\n' ) {
			com_lines++;
			*hasNewLines = qtrue;
		}
		data++;
	}

	return data;
}


int COM_Compress( char *data_p ) {
	const char *in;
	char *out;
	int c;
	qboolean newline = qfalse, whitespace = qfalse;

	in = out = data_p;
	while ((c = *in) != '\0') {
		// skip double slash comments
		if ( c == '/' && in[1] == '/' ) {
			while (*in && *in != '\n') {
				in++;
			}
		// skip /* */ comments
		} else if ( c == '/' && in[1] == '*' ) {
			while ( *in && ( *in != '*' || in[1] != '/' ) ) 
				in++;
			if ( *in ) 
				in += 2;
			// record when we hit a newline
		} else if ( c == '\n' || c == '\r' ) {
			newline = qtrue;
			in++;
			// record when we hit whitespace
		} else if ( c == ' ' || c == '\t') {
			whitespace = qtrue;
			in++;
			// an actual token
		} else {
			// if we have a pending newline, emit it (and it counts as whitespace)
			if (newline) {
				*out++ = '\n';
				newline = qfalse;
				whitespace = qfalse;
			} else if (whitespace) {
				*out++ = ' ';
				whitespace = qfalse;
			}
			// copy quoted strings unmolested
			if (c == '"') {
				*out++ = c;
				in++;
				while (1) {
					c = *in;
					if (c && c != '"') {
						*out++ = c;
						in++;
					} else {
						break;
					}
				}
				if (c == '"') {
					*out++ = c;
					in++;
				}
			} else {
				*out++ = c;
				in++;
			}
		}
	}

	*out = '\0';

	return out - data_p;
}


const char *COM_ParseExt( const char **data_p, qboolean allowLineBreaks )
{
	int c = 0, len;
	qboolean hasNewLines = qfalse;
	const char *data;

	data = *data_p;
	len = 0;
	com_token[0] = '\0';
	com_tokenline = 0;

	// make sure incoming data is valid
	if ( !data )
	{
		*data_p = NULL;
		return com_token;
	}

	// RF, backup the session data so we can unget easily
	COM_BackupParseSession( data_p );

	while ( 1 )
	{
		// skip whitespace
		data = SkipWhitespace( data, &hasNewLines );
		if ( !data )
		{
			*data_p = NULL;
			return com_token;
		}
		if ( hasNewLines && !allowLineBreaks )
		{
			*data_p = data;
			return com_token;
		}

		c = *data;

		// skip double slash comments
		if ( c == '/' && data[1] == '/' )
		{
			data += 2;
			while (*data && *data != '\n') {
				data++;
			}
		}
		// skip /* */ comments
		else if ( c == '/' && data[1] == '*' )
		{
			data += 2;
			while ( *data && ( *data != '*' || data[1] != '/' ) )
			{
				if ( *data == '\n' )
				{
					com_lines++;
				}
				data++;
			}
			if ( *data )
			{
				data += 2;
			}
		}
		else
		{
			break;
		}
	}

	// token starts on this line
	com_tokenline = com_lines;

	// handle quoted strings
	if ( c == '"' )
	{
		data++;
		while ( 1 )
		{
			c = *data++;
			if ( c == '\\' && *( data ) == '\"' )
			{
				// Arnout: string-in-string
				if ( len < (int)ARRAY_LEN( com_token )-1 )
				{
					com_token[ len ] = '\"';
					len++;
				}
				data++;

				while ( 1 )
				{
					c = *data++;

					if ( !c )
					{
						com_token[ len ] = '\0';
						*data_p = data;
						break;
					}
					if ( ( c == '\\' && *( data ) == '\"' ) )
					{
						if ( len < (int)ARRAY_LEN( com_token )-1 )
						{
							com_token[ len ] = '\"';
							len++;
						}
						data++;
						c = *data++;
						break;
					}
					if ( len < (int)ARRAY_LEN( com_token )-1 )
					{
						com_token[ len ] = c;
						len++;
					}
				}
			}
			if ( c == '\"' || c == '\0' )
			{
				com_token[ len ] = '\0';
				*data_p = data;
				return com_token;
			}
			if ( c == '\n' )
			{
				com_lines++;
			}
			if ( len < (int)ARRAY_LEN( com_token )-1 )
			{
				com_token[ len ] = c;
				len++;
			}
		}
	}

	// parse a regular word
	do
	{
		if ( len < (int)ARRAY_LEN( com_token )-1 )
		{
			com_token[ len ] = c;
			len++;
		}
		data++;
		c = *data;
	} while ( c > ' ' );

	com_token[ len ] = '\0';

	*data_p = data;
	return com_token;
}
	




/*
==================
COM_MatchToken
==================
*/
static void COM_MatchToken( const char **buf_p, const char *match ) {
	const char *token;

	token = COM_Parse( buf_p );
	if ( strcmp( token, match ) ) {
		Com_Error( ERR_DROP, "MatchToken: %s != %s", token, match );
	}
}


/*
=================
SkipBracedSection

The next token should be an open brace or set depth to 1 if already parsed it.
Skips until a matching close brace is found.
Internal brace depths are properly skipped.
=================
*/
qboolean SkipBracedSection( const char **program, int depth ) {
	const char			*token;

	do {
		token = COM_ParseExt( program, qtrue );
		if( token[1] == 0 ) {
			if( token[0] == '{' ) {
				depth++;
			}
			else if( token[0] == '}' ) {
				depth--;
			}
		}
	} while( depth && *program );

	return ( depth == 0 );
}


/*
=================
SkipRestOfLine
=================
*/
void SkipRestOfLine( const char **data ) {
	const char *p;
	int		c;

	p = *data;

	if ( !*p )
		return;

	while ( (c = *p) != '\0' ) {
		p++;
		if ( c == '\n' ) {
			com_lines++;
			break;
		}
	}

	*data = p;
}


void Parse1DMatrix( const char **buf_p, int x, float *m ) {
	const char	*token;
	int		i;

	COM_MatchToken( buf_p, "(" );

	for (i = 0 ; i < x; i++) {
		token = COM_Parse( buf_p );
		m[i] = atof( token );
	}

	COM_MatchToken( buf_p, ")" );
}


void Parse2DMatrix( const char **buf_p, int y, int x, float *m ) {
	int		i;

	COM_MatchToken( buf_p, "(" );

	for (i = 0 ; i < y ; i++) {
		Parse1DMatrix (buf_p, x, m + i * x);
	}

	COM_MatchToken( buf_p, ")" );
}


void Parse3DMatrix( const char **buf_p, int z, int y, int x, float *m ) {
	int		i;

	COM_MatchToken( buf_p, "(" );

	for (i = 0 ; i < z ; i++) {
		Parse2DMatrix (buf_p, y, x, m + i * x*y);
	}

	COM_MatchToken( buf_p, ")" );
}


/*
===============
Com_ParseInfos
===============
*/
int Com_ParseInfos( const char *buf, int max, char infos[][MAX_INFO_STRING] ) {
	const char  *token;
	int count;
	char key[MAX_TOKEN_CHARS];

	count = 0;

	while ( 1 ) {
		token = COM_Parse( &buf );
		if ( !token[0] ) {
			break;
		}
		if ( strcmp( token, "{" ) ) {
			Com_Printf( "Missing { in info file\n" );
			break;
		}

		if ( count == max ) {
			Com_Printf( "Max infos exceeded\n" );
			break;
		}

		infos[count][0] = 0;
		while ( 1 ) {
			token = COM_Parse( &buf );
			if ( !token[0] ) {
				Com_Printf( "Unexpected end of info file\n" );
				break;
			}
			if ( !strcmp( token, "}" ) ) {
				break;
			}
			Q_strncpyz( key, token, sizeof( key ) );

			token = COM_ParseExt( &buf, qfalse );
			if ( !token[0] ) {
				token = "<NULL>";
			}
			Info_SetValueForKey( infos[count], key, token );
		}
		count++;
	}

	return count;
}

int QDECL Com_sprintf( char *dest, int size, const char *fmt, ...) {
	int		ret;
	va_list		argptr;

	va_start (argptr,fmt);
	ret = Q_vsnprintf (dest, size, fmt, argptr);
	va_end (argptr);
	if (ret == -1) {
		Com_Printf ("Com_sprintf: overflow of %i bytes buffer\n", size);
		return 0;
	}
	return ret;
}

/*
============
va

does a varargs printf into a temp buffer, so I don't need to have
varargs versions of all text functions.
FIXME: make this buffer size safe someday

Ridah, modified this into a circular list, to further prevent stepping on
previous strings
============
*/
const char * QDECL va( const char *format, ... ) {
	char	*buf;
	va_list		argptr;
	#define	MAX_VA_STRING	32000
	static int		index = 0;
	static char		string[2][MAX_VA_STRING];	// in case va is called by nested functions
	int ret;

	buf = string[ index ];
	index ^= 1;

	va_start (argptr, format);
	ret = Q_vsnprintf(buf, sizeof(string[0]), format, argptr);
	va_end (argptr);

	if ( ret == -1 ) {
		Com_Printf( "va(): overflow of %i bytes buffer\n", MAX_VA_STRING );
	}

	return buf;
}

/*
=============
TempVector

(SA) this is straight out of g_utils.c around line 210

This is just a convenience function
for making temporary vectors for function calls
=============
*/
/*float	*tv( float x, float y, float z ) {
	static	int		index;
	static	vec3_t	vecs[8];
	float	*v;

	// use an array so that multiple tempvectors won't collide
	// for a while
	v = vecs[index];
	index = (index + 1)&7;

	v[0] = x;
	v[1] = y;
	v[2] = z;

	return v;
}*/

/*
=====================================================================

  INFO STRINGS

=====================================================================
*/

static qboolean Q_strkey( const char *str, const char *key, int key_len )
{
	int i;

	for ( i = 0; i < key_len; i++ )
	{
		if ( locase[ (byte)str[i] ] != locase[ (byte)key[i] ] )
		{
			return qfalse;
		}
	}

	return qtrue;
}


/*
===============
Info_ValueForKey

Searches the string for the given
key and returns the associated value, or an empty string.
===============
*/
const char *Info_ValueForKey( const char *s, const char *key )
{
	static	char value[2][BIG_INFO_VALUE];	// use two buffers so compares
											// work without stomping on each other
	static	int	valueindex = 0;
	const char *v, *pkey;
	char	*o, *o2;
	int		klen, len;
	
	if ( !s || !key || !*key )
		return "";

	klen = (int)strlen( key );

	if ( *s == '\\' )
		s++;

	while (1)
	{
		pkey = s;
		while ( *s != '\\' )
		{
			if ( *s == '\0' )
				return "";
			++s;
		}
		len = (int)(s - pkey);
		s++; // skip '\\'

		v = s;
		while ( *s != '\\' && *s !='\0' )
			s++;

		if ( len == klen && Q_strkey( pkey, key, klen ) )
		{
			o = o2 = value[ valueindex ^= 1 ];
			if ( (int)(s - v) >= BIG_INFO_VALUE )
			{
				Com_Error( ERR_DROP, "Info_ValueForKey: oversize infostring [%s] [%s]", s, key );
			}
			else 
			{
				while ( v < s )
					*o++ = *v++;
			}
			*o = '\0';
			return o2;
		}

		if ( *s == '\0' )
			break;

		s++;
	}

	return "";
}


#define MAX_INFO_TOKENS ((MAX_INFO_STRING/3)+2)

static const char *info_keys[ MAX_INFO_TOKENS ];
static const char *info_values[ MAX_INFO_TOKENS ];
static int info_tokens;

/*
===================
Info_Tokenize

Tokenizes all key/value pairs from specified infostring
NOT suitable for big infostrings
===================
*/
void Info_Tokenize( const char *s )
{
	static char tokenBuffer[ MAX_INFO_STRING ];
	char *o = tokenBuffer;

	info_tokens = 0;
	*o = '\0';

	for ( ;; )
	{
		while ( *s == '\\' ) // skip leading/trailing separators
			s++;

		if ( *s == '\0' )
			break;

		info_keys[ info_tokens ] = o;
		while ( *s != '\\' )
		{
			if ( *s == '\0' )
			{
				*o = '\0'; // terminate key
				info_values[ info_tokens++ ] = o;
				return;
			}
			*o++ = *s++;
		}
		*o++ = '\0'; // terminate key
		s++; // skip '\\'

		info_values[ info_tokens++ ] = o;
		while ( *s != '\\' && *s != '\0' )
		{
			*o++ = *s++;
		}
		*o++ = '\0';
	}
}


/*
===================
Info_ValueForKeyToken

Fast lookup from tokenized infostring
===================
*/
const char *Info_ValueForKeyToken( const char *key )
{
	int i;

	for ( i = 0; i < info_tokens; i++ ) 
	{
		if ( Q_stricmp( info_keys[ i ], key ) == 0 )
		{
			return info_values[ i ];
		}
	}

	return "";
}


/*
===================
Info_NextPair

Used to iterate through all the key/value pairs in an info string
===================
*/
const char *Info_NextPair( const char *s, char *key, char *value ) {
	char	*o;

	if ( *s == '\\' ) {
		s++;
	}

	key[0] = '\0';
	value[0] = '\0';

	o = key;
	while ( *s != '\\' ) {
		if ( !*s ) {
			*o = '\0';
			return s;
		}
		*o++ = *s++;
	}
	*o = '\0';
	s++;

	o = value;
	while ( *s != '\\' && *s ) {
		*o++ = *s++;
	}
	*o = '\0';

	return s;
}


/*
===================
Info_RemoveKey
===================
*/
int Info_RemoveKey( char *s, const char *key )
{
	char	*start;
	const char 	*pkey;
	int		key_len, len;

	key_len = (int) strlen( key );

	while (1)
	{
		start = s;
		if ( *s == '\\' )
			s++;
		pkey = s;
		while ( *s != '\\' )
		{
			if ( *s == '\0' )
				return 0;
			++s;
		}
		len = (int)(s - pkey);
		++s; // skip '\\'

		while ( *s != '\\' && *s != '\0' )
			++s;

		if ( len == key_len && Q_strkey( pkey, key, key_len ) )
		{
			memmove( start, s, strlen( s ) + 1 ); // remove this part
			return (int)(s - start);
		}

		if ( *s == '\0' )
			break;
	}

	return 0;
}


/*
==================
Info_Validate

Some characters are illegal in info strings because they
can mess up the server's parsing
==================
*/
qboolean Info_Validate( const char *s )
{
	for ( ;; )
	{
		switch ( *s++ )
		{
		case '\0':
			return qtrue;
		case '\"':
		case ';':
			return qfalse;
		default:
			break;
		}
	}
}


/*
==================
Info_ValidateKeyValue

Some characters are illegal in key values because they
can mess up the server's parsing
==================
*/
qboolean Info_ValidateKeyValue( const char *s )
{
	for ( ;; )
	{
		switch ( *s++ )
		{
		case '\0':
			return qtrue;
		case '\\':
		case '\"':
		case ';':
			return qfalse;
		default:
			break;
		}
	}
}


/*
==================
Info_SetValueForKey_s

Changes or adds a key/value pair
==================
*/
qboolean Info_SetValueForKey_s( char *s, int slen, const char *key, const char *value ) {
	char	newi[BIG_INFO_STRING+2];
	int		len1, len2;

	len1 = (int)strlen( s );

	if ( len1 >= slen ) {
		Com_Error( ERR_DROP, "Info_SetValueForKey(%s): oversize infostring", key );
		return qfalse;
	}

	if ( !key || !Info_ValidateKeyValue( key ) || *key == '\0' ) {
		Com_Printf( S_COLOR_YELLOW "Invalid key name: '%s'\n", key );
		return qfalse;
	}

	if ( value && !Info_ValidateKeyValue( value ) ) {
		Com_Printf( S_COLOR_YELLOW "Invalid value name: '%s'\n", value );
		return qfalse;
	}

	len1 -= Info_RemoveKey( s, key );
	if ( !value || !*value )
		return qtrue;

	len2 = Com_sprintf( newi, sizeof( newi ), "\\%s\\%s", key, value );

	if ( len1 + len2 >= slen )
	{
		Com_Printf( S_COLOR_YELLOW "Info string length exceeded for key '%s'\n", key );
		return qfalse;
	}

	strcpy( s + len1, newi );
	return qtrue;
}


void *Q_LinearSearch( const void *key, const void *ptr, size_t count,
	size_t size, cmpFunc_t cmp )
{
	size_t i;
	for ( i = 0; i < count; i++ )
	{
		if ( cmp( key, ptr ) == 0 ) return (void *)ptr;
		ptr = (const char *)ptr + size;
	}
	return NULL;
}

//====================================================================
