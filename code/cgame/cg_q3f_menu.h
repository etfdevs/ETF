/*
**	cg_q3f_menu.h
**
**	In-game menu functions.
*/

#ifndef __CG_Q3F_MENU_H
#define __CG_Q3F_MENU_H

#include "q_shared.h"
#include "../ui_new/ui_shared.h"

void CG_Q3F_MenuDraw();						// Render menu every frame
void CG_Q3F_MenuInit();						// Prepare a new menu
void CG_Q3F_MenuCancel( qboolean command );	// 'Cancel' a menu
void CG_Q3F_MenuCommand();					// Process a menu command
void CG_Q3F_UserMenuCommand( void );		// Display a user menu
qboolean CG_Q3F_MenuChoice( int choice );	// Select a menu option (called from weapon)
void CG_Q3F_MenuNumCommand( void );			// Process 'menucmd' command.
qboolean CG_Q3F_ShowingSentryUpgradeMenu();	// 'Upgrade Autosentry' menu is being shown
qboolean CG_Q3F_ShowingSupplyStationUpgradeMenu(); // 'Upgrade Supply Station' menu is being shown
void CG_Q3F_AutoTeamMenu();					// Automatically throw up a menu if required
void CG_Q3F_StartSpectate( void );			// start spectating mode (free flight)
void CG_Q3F_SpectatorTypeMenu();			// Spectator options
void CG_Q3F_DrawMenuBox();
void CG_Q3F_Menu_DrawItem(int pos, int optnum, rectDef_t *rect, float scale, vec4_t color, int textStyle, 
									  int textalignment, float text_x, float text_y, fontStruct_t *font);
qboolean CG_Q3F_GetMenuAlpha(rectDef_t* in, rectDef_t* out, float* alpha);
void CG_Q3F_DrawMenuTitle(rectDef_t *rect, float scale, vec4_t color, int textStyle, 
									  int textalignment, float text_x, float text_y, fontStruct_t *font, const char* text);
void CG_Q3F_StoreClassinfo( void );
int CG_Q3F_GetMenuState( void );
//void ReadyMenu( void ); // Ensiform - This is static
//void VoteMenu( void ); // Ensiform - This is static
void MenuCheckVoteTally( void );


typedef enum {
	Q3F_MENUSTATE_NONE,
	Q3F_MENUSTATE_READY,
	Q3F_MENUSTATE_FADING
} cg_q3f_menustates_t;

#define Q3F_MENU_FADETIME	1000					// Milliseconds to fade out
#define	Q3F_MENU_STARTX		0						// Offset from left edge
#define	Q3F_MENU_STARTY		SMALLCHAR_HEIGHT * 6	// Offset from top edge
#define	Q3F_MENU_OPACITY	0.6						// How opaque the background is
#define	Q3F_MENU_CHOICETIME	50						// Time required between menu selections

#endif	//__CG_Q3F_MENU_H
