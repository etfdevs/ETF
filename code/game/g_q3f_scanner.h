/*
**	g_q3f_scanner.h
**
**	Server side functions for scanner handling
*/

#ifndef	__G_Q3F_SCANNER_H
#define	__G_Q3F_SCANNER_H

#include "g_local.h"

#define Q3F_SCANNER_CELL_TIME				600		// 1.5 cells a second
#define Q3F_SCANNER_UPDATE_INTERVAL			100
#define Q3F_SCANNER_RANGE_DIVISOR			10

void G_Q3F_Check_Scanner(struct gentity_s *ent);
void G_Q3F_Pack_Scanner_Update(struct gentity_s *ent, int clientNum);
int G_Q3F_FindNextMovingClient(struct gentity_s *ent, int lastnum, struct gclient_s *ignore);

#endif //__G_Q3F_SCANNER_H

