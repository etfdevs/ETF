/*
**	cg_q3f_flyby.c
**
**	Flyby camera system
*/

#include "cg_local.h"

// Debug:
void CG_Q3F_DrawCamPaths( void ) {
	int segmentindex, splineindex, pathindex;
	vec3_t nullvec;
	vec3_t vec_lookat;

	VectorSet(nullvec, 0, 0, 0);
	
	for ( pathindex = 0; pathindex < cgs.camNumPaths; pathindex++ ) {
		for ( splineindex = 0; splineindex < cgs.campaths[pathindex].numsplines; splineindex++ ) {
			for ( segmentindex=0; segmentindex<cgs.campaths[pathindex].splines[splineindex].nSegments; segmentindex++ )
				CG_SpawnSmokeSprite(cgs.campaths[pathindex].splines[splineindex].SegmentVtx[segmentindex], 1, colorRed, 8, 8);
			VectorSet( vec_lookat, 200000, 200000, 200000 );
			if( !VectorCompare( cgs.campaths[pathindex].camsplines[splineindex].lookat, vec_lookat ) ) {
				CG_SpawnSmokeSprite(cgs.campaths[pathindex].camsplines[splineindex].lookat, 1, colorWhite, 8, 8);
			}
		}
	}
}
