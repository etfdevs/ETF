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
