/************************************************

The Tiny Encryption Algorithm (TEA) by 
David Wheeler and Roger Needham of the
Cambridge Computer Laboratory

************************************************/
#ifndef __BG_Q3F_TEA_H
#define __BG_Q3F_TEA_H

void BG_Q3F_Encipher(unsigned long *const v,unsigned long *const w, const unsigned long *const k);
void BG_Q3F_Decipher(unsigned long *const v,unsigned long *const w, const unsigned long *const k);
//void BG_Q3F_GetFileHash(unsigned long *const v);

#endif
