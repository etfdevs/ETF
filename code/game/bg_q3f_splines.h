/*
**	bg_q3f_splines.h
**
**	Common functions for bezier spline math
*/

#ifndef	__BG_Q3F_SPLINES_H
#define	__BG_Q3F_SPLINES_H

#include "q_shared.h"

void MirrorVtxInVtx( vec3_t d, const vec3_t p, const vec3_t m );

// Have to hardcode maxsegments (grr, no malloc)
#define Q3F_MAXSPLINESEGMENTS 32

typedef struct Q3F_QuadSpline_s {
	int		nSegments;
	vec3_t	ControlPoint[3];
	//vec3_t	*SegmentVtx;
	vec3_t	SegmentVtx[Q3F_MAXSPLINESEGMENTS + 1];
	//float	length;
} Q3F_QuadSpline_t;

typedef struct Q3F_CubicSpline_s {
	int		nSegments;
	vec3_t	ControlPoint[4];
	//vec3_t	*SegmentVtx;
	vec3_t	SegmentVtx[Q3F_MAXSPLINESEGMENTS + 1];
	//float	length;
} Q3F_CubicSpline_t;

void BG_Q3F_QuadSpline_ComputeSegments ( Q3F_QuadSpline_t *spline );
float BG_Q3F_QuadSpline_Length ( Q3F_QuadSpline_t *spline );
void BG_Q3F_QuadSpline_Position( vec3_t src, vec3_t dst, float relpos, Q3F_QuadSpline_t *spline );
void BG_Q3F_QuadSpline_PositionAngle( vec3_t src_angle, vec3_t dst_angle, float relpos, Q3F_QuadSpline_t *spline );

void BG_Q3F_CubicSpline_ComputeSegments ( Q3F_CubicSpline_t *spline );
float BG_Q3F_CubicSpline_Length ( Q3F_CubicSpline_t *spline );
void BG_Q3F_CubicSpline_Position( vec3_t src, vec3_t dst, float relpos, Q3F_CubicSpline_t *spline );
void BG_Q3F_CubicSpline_PositionAngle( vec3_t src_angle, vec3_t dst_angle, float relpos, Q3F_CubicSpline_t *spline );

// General Spline Calculation Functions
void BG_Q3F_EvaluateSplineTrajectory( const trajectory_t *tr, Q3F_QuadSpline_t *qspline, Q3F_CubicSpline_t *cspline, int atTime, vec3_t result );
void BG_Q3F_EvaluateSplineTrajectoryAngle( const trajectory_t *tr, Q3F_QuadSpline_t *qspline, Q3F_CubicSpline_t *cspline, int atTime, vec3_t result );
//void BG_Q3F_EvaluateSplineTrajectoryDelta( const trajectory_t *tr, Q3F_QuadSpline_t *qspline, Q3F_CubicSpline_t *cspline, int atTime, vec3_t result );

#endif
