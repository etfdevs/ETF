// Copyright (C) 1999-2000 Id Software, Inc.
//
// cg_scoreboard -- draw the scoreboard on top of the game screen
#include "cg_local.h"

void CG_Q3F_CalculateTeamTieString( int *order, int first, int last, char* buffer, int size )
{
	// Return a string like "Red, Blue and Yellow have 5"

	int i;
	char *s;
	char namebuffer[64];

	for( i = first; i <= last; i++ )
	{
		switch( order[i] )
		{
			case Q3F_TEAM_RED:		s = "Red";		break;
			case Q3F_TEAM_BLUE:		s = "Blue";		break;
			case Q3F_TEAM_YELLOW:	s = "Yellow";	break;
			case Q3F_TEAM_GREEN:	s = "Green";	break;
			default:				s = "?";
		}
		trap_Cvar_VariableStringBuffer(va("cg_%steam", s), namebuffer, 64);
		Q_strcat( buffer, size, namebuffer );
		Q_strcat( buffer, size, "^7" );
		if( i < last )
			Q_strcat( buffer, size, (i < (last - 1)) ? ", " : " and " );
	}

	if( last > first )
	{
		if( cg.teamScores[last] == cg.teamScores[first] )
			s = " are tied at ";
		else if( last - first > 1 )
			s = first ? " each have " : " lead with ";
		else s = first ? " both have " : " are tied at ";
	}
	else s = first ? " has " : " leads with ";
	Q_strcat( buffer, size, s );
	Q_strcat( buffer, size, va( "%d", cg.teamScores[order[first]-Q3F_TEAM_RED] ) );
}

void CG_Q3F_DrawScoreboardTeamScores(rectDef_t *rect, float scale, vec4_t color, int textStyle, int textalignment, float text_x, float text_y, fontStruct_t *font)
{
	int order[4];
	char *s, *p;
	int i, j, temp, y;
	char teamposbuff[1024];
	qboolean	useClr = qfalse;
	char		clrCode = 0;
	char		cacheClrCode = 0;

	// Find our teams
	for( i = 0; i < 4; i++ )
		order[i] = (cgs.teams & (1<<(Q3F_TEAM_RED+i))) ? (Q3F_TEAM_RED+i) : Q3F_TEAM_NUM_TEAMS;

	// Sort  them
	for( i = 0; i < 3; i++ )
	{
		if( order[i+1] < Q3F_TEAM_NUM_TEAMS && cg.teamScores[order[i]-Q3F_TEAM_RED] < cg.teamScores[order[i+1]-Q3F_TEAM_RED] )
		{
			temp = order[i];
			order[i] = order[i+1];
			order[i+1] = temp;
			i = -1;		// Reset loop (it's a swap-sort)
		}
	}

	*teamposbuff = '\0';

		// Make a string
	for( i = 0; i < 4 && order[i] < Q3F_TEAM_NUM_TEAMS; i = j )
	{
		for( j = i+1; j < 4 &&
			(cg.teamScores[order[j]-Q3F_TEAM_RED] >= cg.teamScores[order[i]-Q3F_TEAM_RED]) &&
			(order[j] < Q3F_TEAM_NUM_TEAMS) ; j++ );
			// i to j-1 are on the same score, so...
		CG_Q3F_CalculateTeamTieString( order, i, j-1, teamposbuff, sizeof(teamposbuff) );
		if( j < 4 && order[j] < Q3F_TEAM_NUM_TEAMS )
			Q_strcat( teamposbuff, sizeof(teamposbuff), ", " );
	}

	CG_ExpandingTextBox_AutoWrap( teamposbuff, scale, font, rect->w, sizeof(teamposbuff));
	y = rect->y + text_y;

	s = p = teamposbuff;
	while(*p) {
		if(Q_IsColorString(p)) {
			clrCode = *(p+1);
			p++;
		} else if(*p == '\n') {
			*p++ = '\0';

			CG_Text_Paint( rect->x + text_x , y, scale, color, useClr ? va("^%c%s", cacheClrCode, s) : s, 0, 0, textStyle, font, textalignment);

			if(clrCode) {
				cacheClrCode = clrCode;
				useClr = qtrue;
			}

			y += CG_Text_Height(s, scale, 0, font)+4;
			s = p;
		}
		else {
			p++;
		}
	}
}
