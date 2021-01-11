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
**	cg_q3f_panel.h
**
**	Headers for panel rendering.
*/

#ifndef	_CG_Q3F_PANEL_H
#define	_CG_Q3F_PANEL_H

#include "cg_local.h"
//#include "..\game\bg_public.h"

	// These are layer offsets used by CG_Q3F_PanelPrepareCoords() etc. to prevent z-fighting
#define PANEL_LAYER_BACK	1.0f		// Background layer
#define	PANEL_LAYER_MAIN	4.0f		// Main layer for panel rendering
#define	PANEL_LAYER_MAIN2	7.0f		// Second layer for panel rendering.
#define	PANEL_LAYER_FORE	10.0f		// Layer for foreground shader.
#define	PANEL_LAYER_TRANS	13.0f		// Layer for special transition requirements.

	// CG_Q3F_PanelDrawString() behaviour is controlled by these.
#define	PANEL_STR_LEFT				0x0000
#define	PANEL_STR_CENTER			0x0001
#define	PANEL_STR_RIGHT				0x0002
#define	PANEL_STR_JUSTIFY			0x0003

#define	PANEL_STR_PROPORTIONAL		0x0004
#define	PANEL_STR_WRAP				0x0008
#define	PANEL_STR_SPACEBREAK		0x0010
#define	PANEL_STR_COLOUR			0x0020
#define	PANEL_STR_COLOR				PANEL_STR_COLOUR

	// The rendering function callback. Simple enough for anyone to work out.
typedef int (panelRenderFunc_t)();
	// The transition function callback. (Usually) only used internally.
typedef int (panelTransitionFunc_t)( int mode, int msec );

	// Struct defining a panel transition.
typedef struct {
	int id, offTime, onTime;
	panelTransitionFunc_t *renderFunc;
} panelTransition_t;

	// Struct holding useful information about the current panel being rendered.
typedef struct panelData_s {
	vec3_t origin, angles;					// Origin and angles of panel.
	float surfaceoffset;					// Current surface offset.
	float width, height, aspect;			// Predefined width and height of entity, and aspect ratio.
	vec3_t forward, right, up;				// Results from AngleVectors on the angles field.
	vec3_t viewforward;						// Pointing towards the viewer (reverse of view direction).
	vec3_t xscalecoord, yscalecoord;		// Number of 'real' units per 'panel' unit.
	vec3_t corners[4];						// Each corner of the panel after projection into the world.
	panelTransition_t *trans;				// A pointer to the panel transition in use.
	char buff[1024];						// A scratch buffer, used by CG_Q3F_PanelDrawString() among others.
	vec4_t transrgba;						// The RGBA after the transition, should be respected during rendering to allow fades.
	void *data;								// Callback data for the rendering function

	float clipxmin, clipxmax;				// Clipping limits when rendering polygons.
	float clipymin, clipymax;

	int polyCount, frametime;				// Counter for number of polygons drawn each frame.
} panelData_t;


	// This global is used for _all_ panel renders (they're single-threaded).
extern panelData_t panel;


	// Multiply the specified colour with that of the specified team. Set all colour fields to 1.0 for no conversion.
void CG_Q3F_PanelCalculateTeamColour( int team, vec4_t colour );

	// Prepare a set of mapping coordinates at the specified offset from the surface.
void CG_Q3F_PanelPrepareCoords( float offset );

	// Draw a panel-mapped polygon.
void CG_Q3F_PanelDrawPoly( float x, float y, float w, float h, float sx, float ty, float sw, float th, vec4_t rgba, qhandle_t shader, qboolean clip );

	// Fill the specified panel layer with the specified panel.
void CG_Q3F_PanelFill( int shader, float offset );

	// Draw a shader to the panel.
void CG_Q3F_PanelDrawShader( float x, float y, float width, float height, vec4_t rgba, qhandle_t shader, qboolean clip );

	// Draw a letter to the panel.
void CG_Q3F_PanelDrawChar( float x, float y, float width, float height, int ch, vec4_t rgba, qboolean clip );

	// Draw a string to the panel, with formatting.
void CG_Q3F_PanelDrawString( char *str, float x, float y, float size, float maxx, float maxy, int flags, vec4_t rgba );

	// Calculate the required character size and number of lines to fit the specified string into
	// the given area.
void CG_Q3F_PanelFitString( int *numlines, float *size, char *str, int flags, float w, float h );

	// Main rendering function.
qboolean CG_Q3F_RenderPanel( qhandle_t backshader, qhandle_t foreshader, int transitionid, vec3_t origin, vec3_t angles, float width, float height, float maxdist, panelRenderFunc_t *renderFunc, void *renderData, int activetime, int inactivetime, qboolean clientsided, qboolean twosided );

	// ET_PANEL rendering function
void CG_Q3F_Panel( centity_t *cent );

#endif//_CG_Q3F_PANEL_H
