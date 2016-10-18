/*
**	cg_q3f_scanner.h
**
**	Recon Scanner client-side functions
*/

#ifndef __CG_Q3F_SCANNER_H
#define __CG_Q3F_SCANNER_H

#include "q_shared.h"
#include "cg_local.h"


typedef struct scannerdata_s
{
	int x;
	int y;
	int z;
	int team;
	int relativeheight;
	int last_updated;
} scannerdata_t;

void CG_Q3F_ScannerDraw();
void CG_Q3F_CheckForScannerData(struct centity_s *cent);
/* void CG_UpdateScanner(struct centity_s *cent); */
void CG_Q3F_ColorForTeam(vec4_t hcolor, int teamnum);
void CG_DrawScanner( rectDef_t *rect );

#define Q3F_SCANNER_RANGE_DIVISOR			10		// Must be same as in g_q3f_scanner.h
#define Q3F_SCANNER_BLIP_FADEOUT			1000	// How long does an un-updated blip live?

#endif	//__CG_Q3F_SCANNER_H
