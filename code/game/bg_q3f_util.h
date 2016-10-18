/*
===============
Clip Handling Functions			-- JT
===============
*/


#ifndef _BG_Q3F_UTIL_H
#define _BG_Q3F_UTIL_H

void Q3F_SetClipValue(int weapon, int value, playerState_t *playstate);
int Q3F_GetClipValue(int weapon, playerState_t *playstate);
int Q3F_GetAmmoNumInClip(int ammotype, playerState_t *playstate);
void Q3F_CapClips(playerState_t *playstate);

#endif
