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
**	cg_q3f_customshader.c
**
**	Custom shader rendering (i.e. custom polygons)
*/

#include "cg_local.h"

typedef struct parallaxlayer_s {
	float depth;
	qhandle_t shader;
} parallaxlayer_t;

typedef struct parallaxvertex_s {
	vec3_t vertex;
	float s, t;
} parallaxvertex_t;

typedef struct parallaxshader_s {
	struct parallaxshader_s *next;
	vec3_t mins, maxs, origin, normal;
	int numLayers, numPoints;
	float xscale, yscale, xoff, yoff, angle, ac, as;
	parallaxlayer_t *layers;
	parallaxvertex_t *vertices;
} parallaxshader_t;

#define	MAXPARALLAXPOINTS	16


/******************************************************************************
***** Render all custom polygons.
****/

static parallaxshader_t *firstshader;

//static int frame;
void CG_Q3F_RenderCustomShaders()
{
	polyVert_t verts[MAXPARALLAXPOINTS];
	vec3_t viewProjections[MAXPARALLAXPOINTS];
	float viewProjectionLengths[MAXPARALLAXPOINTS];
	parallaxshader_t *shader;
	parallaxlayer_t *layer;
	int layerIndex, pointIndex;//, polyIndex;
	float fovproduct, distance, projlen, s, t;
	vec3_t tmp, closest, originprojection;
	vec3_t axis[3];

	fovproduct = sin( (M_PI / 360) * (cg.refdef.fov_x > cg.refdef.fov_y ? cg.refdef.fov_x : cg.refdef.fov_y) );

	for( shader = firstshader; shader; shader = shader->next )
	{
		if(	DotProduct( cg.refdef.viewaxis[0], shader->normal ) > fovproduct ||
			!(trap_R_inPVS( cg.refdef.vieworg, shader->mins ) ||
			trap_R_inPVS( cg.refdef.vieworg, shader->maxs )) )
			continue;

		// Find the 'closest' point on the polygon plane.
		// I don't know where I'd have been if Mr Elusive hadn't told me about
		// the VectorRotate function... the tangent conneca-to-dah cosine... :)
		VectorSubtract( vec3_origin, shader->normal, axis[0] );
		MakeNormalVectors( axis[0], axis[1], axis[2] );
		VectorSubtract( vec3_origin, axis[2], axis[2] );
		VectorSubtract( shader->origin, cg.refdef.vieworg, tmp );
		VectorRotate( tmp, axis, closest );

		for( pointIndex = 0, distance = 0; pointIndex < shader->numPoints; pointIndex++ )
		{
			VectorSubtract( cg.refdef.vieworg, shader->vertices[pointIndex].vertex, viewProjections[pointIndex] );
			viewProjectionLengths[pointIndex] = VectorNormalize( viewProjections[pointIndex] );
			if( viewProjectionLengths[pointIndex] > distance )
				distance = viewProjectionLengths[pointIndex];
		}
			// This is a rule-of-thumb thing... how little the projection needs to be without
			// getting caught up in the depth buffer.
		distance = (5.0f * distance) / (float) (1 << cgs.glconfig.depthBits);
		for( pointIndex = 0; pointIndex < shader->numPoints; pointIndex++ )
			VectorScale( viewProjections[pointIndex], distance * viewProjectionLengths[pointIndex], viewProjections[pointIndex] );

		VectorSubtract( cg.refdef.vieworg, shader->origin, originprojection );
		VectorNormalize( originprojection );

		for( layerIndex = 0; layerIndex < shader->numLayers; layerIndex++ )
		{
			layer = &shader->layers[layerIndex];
			projlen = (float) (layerIndex + 1);
			for( pointIndex = 0; pointIndex < shader->numPoints; pointIndex++ )
			{
				VectorMA( shader->vertices[pointIndex].vertex, projlen, viewProjections[pointIndex], verts[pointIndex].xyz );
				*(int *) verts[pointIndex].modulate = ~0;

				// The texture coordinates are altered here to give the appropriate perspective.
				distance = layer->depth / closest[0];
				s = shader->vertices[pointIndex].s + (shader->vertices[pointIndex].s - closest[1] * shader->xscale) * distance;
				t = shader->vertices[pointIndex].t + (shader->vertices[pointIndex].t - closest[2] * shader->yscale) * distance;
				verts[pointIndex].st[0] = s * shader->ac - t * shader->as + shader->xoff;
				verts[pointIndex].st[1] = t * shader->ac + s * shader->as + shader->yoff;
			}
			trap_R_AddPolyToScene( shader->layers[layerIndex].shader, shader->numPoints, verts );
		}
	}
}


/******************************************************************************
***** Create the polygon def.
****/

static int QDECL ParallaxLayer_SortFunc( const void *a, const void *b )
{
	float f = ((parallaxlayer_t *) b)->depth - ((parallaxlayer_t *) a)->depth;
	return( f < 0 ? -1 : (f ? 1 : 0) );
}

#define MAXPARALLAXLAYERS		10
#define PARALLAXRADIUS			96
#define MAXPARALLAXFRAGMENTS	16
void CG_Q3F_ParallaxShader( char *spawnVars[], int numSpawnVars, vec3_t entOrigin, float xscale, float yscale, float xoff, float yoff, float angle )
{
	// Assumes it's being called from inside the spawn function

	int numLayers, index, numVertices, vertex;
	float depth;
	char *key, *value;
	parallaxlayer_t shaders[MAXPARALLAXLAYERS];
	vec3_t vertices[MAXPARALLAXPOINTS];
	vec3_t axis[3];
	vec3_t offsetOrigin, normal, offset;
	parallaxshader_t *shader;
	vec4_t plane, oldplane;

		// Work out all the desired layers & vertices.
	for( numLayers = numVertices = index = 0; index < numSpawnVars && numLayers < MAXPARALLAXLAYERS; index++ )
	{
		key		= spawnVars[2 * index];
		value	= spawnVars[2 * index + 1];
		if( !Q_strncmp( "depth", key, 5 ) )
		{
			// This is a depth layer.

			depth = atof( key + 5 );
			if( shaders[numLayers].shader = trap_R_RegisterShader( value ) )
				shaders[numLayers++].depth = depth;
		}
		if( !Q_strncmp( "vertex", key, 6 ) )
		{
			// This is a vertex.

			vertex = atoi( key + 6 );
			if( vertex > 0 && vertex <= MAXPARALLAXPOINTS )
			{
				sscanf( value, "%f %f %f",	&vertices[vertex - 1][0],
											&vertices[vertex - 1][1],
											&vertices[vertex - 1][2] );
				if( numVertices < vertex )
					numVertices = vertex;
			}
		}
	}
	qsort( shaders, numLayers, sizeof(parallaxlayer_t), &ParallaxLayer_SortFunc );


		// Check the vertices are ordered, and build an axis.
	if( numVertices < 3 )
	{
		CG_Printf( BOX_PRINT_MODE_CHAT, "Not enough vertices on parallax shader.\n" );
		return;
	}

	if( !PlaneFromPoints( oldplane, vertices[0], vertices[1], vertices[2] ) )
	{
		CG_Printf( BOX_PRINT_MODE_CHAT, "Degenerate plane on parallax shader.\n" );
		return;
	}
	VectorCopy( oldplane, normal );
	VectorCopy( oldplane, axis[0] );
	MakeNormalVectors( axis[0], axis[1], axis[2] );

	for( index = 3; index < numVertices; index++ )
	{
		if( !PlaneFromPoints( plane, vertices[0], vertices[1], vertices[index] ) )
			CG_Printf( BOX_PRINT_MODE_CHAT, "Degenerate plane on parallax shader.\n" );
		else if( plane[0] != oldplane[0] || plane[1] != oldplane[1] || plane[2] != oldplane[2] || plane[3] != oldplane[3] )
			CG_Printf( BOX_PRINT_MODE_CHAT, "Point %d out of plane on parallax shader.\n", index );
		else {
			VectorCopy4( plane, oldplane );
			continue;
		}
		return;
	}

		// Allocate memory for the shader info, and initialize.
	shader = CG_Q3F_AddBlock(	sizeof(parallaxshader_t) +
								sizeof(parallaxlayer_t) * numLayers +
								sizeof(parallaxvertex_t) * numVertices,
								//sizeof(markFragment_t) * numFragments,
								4 );
	VectorCopy( normal, shader->normal );
	shader->numLayers = numLayers;
	shader->numPoints = numVertices;
	shader->xscale = 1.0f / xscale;
	shader->yscale = 1.0f / yscale;
	shader->layers = (parallaxlayer_t *) (shader + 1);
	shader->vertices = (parallaxvertex_t *) (shader->layers + numLayers);
	memcpy( shader->layers, shaders, sizeof(parallaxlayer_t) * numLayers );

		// Set each point, and it's texture coordinates.
	VectorCopy( vertices[0], shader->mins );
	VectorCopy( vertices[0], shader->maxs );
	VectorCopy( vertices[0], shader->origin );
	for( index = 0; index < numVertices; index++ )
		AddPointToBounds( vertices[index], shader->mins, shader->maxs );
	shader->origin[0] = shader->mins[0] + 0.5 * (shader->maxs[0] - shader->mins[0]);
	shader->origin[1] = shader->mins[1] + 0.5 * (shader->maxs[1] - shader->mins[1]);
	shader->origin[2] = shader->mins[2] + 0.5 * (shader->maxs[2] - shader->mins[2]);

	shader->angle = angle;
	angle *= M_PI/180;
	shader->ac = cos( angle );
	shader->as = sin( angle );
	shader->xoff = (xoff * shader->ac - yoff * shader->as) * shader->xscale;
	shader->yoff = (yoff * shader->ac + xoff * shader->as) * -shader->yscale;
	for( index = 0; index < numVertices; index++ )
	{
			// Work out the X & Y offsets on the plane.
		VectorCopy( vertices[index], shader->vertices[index].vertex );
		VectorSubtract( vertices[index], shader->origin, offsetOrigin );
		VectorRotate( offsetOrigin, axis, offset );

		shader->vertices[index].s = offset[1] * shader->xscale;
		shader->vertices[index].t = offset[2] * shader->yscale;
	}

		// Add to the list of shaders.
	shader->next = firstshader;
	firstshader = shader;
}
