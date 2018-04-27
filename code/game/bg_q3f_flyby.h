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
**	bg_q3f_flyby.h
**
**	Flyby camera system
*/

#ifndef __BG_Q3F_FLYBY_H
#define __BG_Q3F_FLYBY_H

#include "bg_q3f_splines.h"

#define SPD_VERSION			4
//#define SPD_IDENT			((('S'<<SPD_VERSION)+4)+(('P'<<SPD_VERSION)+8)+(('D'<<SPD_VERSION)+16)+((SPD_VERSION<<SPD_VERSION)+24))
#define SPD_IDENT			(('S'<<(SPD_VERSION+4))+('P'<<(SPD_VERSION+8))+('D'<<(SPD_VERSION+16))+(SPD_VERSION<<(SPD_VERSION+24)))
#define Q3F_MAX_PATHS		16 // Maximum of paths
#define Q3F_MAX_SPLINESPERPATH 512
#define MAXSTRINGSIZE		256

typedef struct spdheader_s
{
	int		version;
	int		numCamPaths;
	int		numSplines;
	char	pathname[Q3F_MAX_PATHS][MAXSTRINGSIZE];
} spdheader_t;

typedef struct camspline_s
{
	//int		pathindex;									// Index of path that this spline is part of (FIXME: is this needed?)
	//int		splineindex;								// Index of the bspline (FIXME: is this needed?)
	struct	camspline_s *next;								// Next bspline on the path
	vec3_t	origin;											// Origin of this point
	vec3_t	dir;											// Angle at this point
	vec3_t	lookat;											// Look towards this point
	int		speed;											// Speed at this point
	int		roll;											// Roll at this point
} camspline_t;

typedef struct campath_s
{
	//int					pathindex;						// Index of this path (FIXME: is this needed?)
	int					numsplines;							// Number of bsplines of this path
	Q3F_CubicSpline_t	splines[Q3F_MAX_SPLINESPERPATH];	// The bsplines
	camspline_t			camsplines[Q3F_MAX_SPLINESPERPATH];	// Raw coord data
	trajectory_t		camtraj;							// Trajectory data for cam
	int					currtrajindex;						// Current spline the cam is on
	char				pathname[MAXSTRINGSIZE];
} campath_t;

qboolean UMC_ReadLineSkipEmpty( fileHandle_t f, int *bytesread, int filelen, char *buff, int buffsize );
int BG_Q3F_GetPathIndex( char *pathname, campath_t* campaths );
int BG_Q3F_LoadCamPaths( char *mapname, campath_t* campaths );
int BG_Q3F_LocateFlybyPath( int numPaths, campath_t* campaths );

#endif
