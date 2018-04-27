/*
===========================================================================

Wolfenstein: Enemy Territory GPL Source Code
Copyright (C) 1999-2010 id Software LLC, a ZeniMax Media company.

Enemy Territory Fortress
Copyright (C) 2000-2006 Quake III Fortress (Q3F) Development Team / Splash Damage Ltd.
Copyright (C) 2005-2018 Enemy Territory Fortress Development Team

OpenJK
Copyright (C) 2000 - 2013, Raven Software, Inc.
Copyright (C) 2001 - 2013, Activision, Inc.
Copyright (C) 2013 - 2015, OpenJK contributors

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

#pragma once

#include "q_string.h"

#if defined(__cplusplus)
extern "C" {
#endif

///////////////////////////////////////////////////////////////////////////
//
//      FILE EXTENSIONS
//
///////////////////////////////////////////////////////////////////////////
const char *COM_GetExtension( const char *name );
void COM_StripExtension( const char *in, char *out, int destsize );
qboolean COM_CompareExtension(const char *in, const char *ext);
void COM_DefaultExtension( char *path, int maxSize, const char *extension );


///////////////////////////////////////////////////////////////////////////
//
//      PATHS
//
///////////////////////////////////////////////////////////////////////////
void COM_FixPath( char *pathname ); // - unixifies a pathname
char *COM_SkipPath (char *pathname);
// FIXME: should include the slash, otherwise
// backing to an empty path will be wrong when appending a slash
void Com_ExtractFilePath (const char *path, char *dest);


#if defined(__cplusplus)
} // extern "C"
#endif
