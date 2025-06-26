#ifndef __OMNIBOT_BASICTYPES_WIN32_H__
#define __OMNIBOT_BASICTYPES_WIN32_H__

// Basic Variable Types.

typedef char						obint8;
typedef unsigned char				obuint8;
typedef short						obint16;
typedef unsigned short				obuint16;
typedef int							obint32;
typedef unsigned int				obuint32;
typedef float						obReal;
typedef void*						obvoidp;

// Windows platforms.
#ifdef _MSC_VER

	typedef __int64						obint64;
	typedef unsigned __int64			obuint64;

#else

	typedef long long int				obint64;
	typedef unsigned long long int		obuint64;

#endif

#endif
