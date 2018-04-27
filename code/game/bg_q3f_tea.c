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

/************************************************

The Tiny Encryption Algorithm (TEA) by 
David Wheeler and Roger Needham of the
Cambridge Computer Laboratory

**** ANSI C VERSION ****

Notes:

TEA is a Feistel cipher with XOR and
and addition as the non-linear mixing
functions. 

Takes 64 bits of data in v[0] and v[1].
Returns 64 bits of data in w[0] and w[1].
Takes 128 bits of key in k[0] - k[3].

TEA can be operated in any of the modes
of DES. Cipher Block Chaining is, for example,
simple to implement.

n is the number of iterations. 32 is ample,
16 is sufficient, as few as eight may be OK.
The algorithm achieves good dispersion after
six iterations. The iteration count can be
made variable if required.

Note this is optimised for 32-bit CPUs with
fast shift capabilities. It can very easily
be ported to assembly language on most CPUs.

delta is chosen to be the real part of (the
golden ratio Sqrt(5/4) - 1/2 ~ 0.618034
multiplied by 2^32). 

************************************************/

#include "bg_q3f_tea.h"

#if 0
void BG_Q3F_Encipher(unsigned long *const v,unsigned long *const w, const unsigned long *const k)
{
	register unsigned long	y=v[0], z=v[1], sum=0, delta=0x9E3779B9, n=32;

	while(n-->0)
	{
		y+= (z<<4 ^ z>>5) + z ^ sum + k[sum&3];
		sum += delta;
		z+= (y<<4 ^ y>>5) + y ^ sum + k[sum>>11 & 3];
	}

	w[0]=y; w[1]=z;
}


void BG_Q3F_Decipher(unsigned long *const v,unsigned long *const w, const unsigned long *const k)
{
	register unsigned long	y=v[0], z=v[1], sum/*sum=0xC6EF3720*/, delta=0x9E3779B9, n=32;

	 /* sum = delta<<5, in general sum = delta<<t where t is t in 2^t=n */
	sum = delta<<5; // RR2DO2: if using 32, it's <<5

	while(n-->0)
	{
		z-= (y<<4 ^ y>>5) + y ^ sum + k[sum>>11 & 3];
		sum -= delta;
		y-= (z<<4 ^ z>>5) + z ^ sum + k[sum&3];
	}
   
	w[0]=y; w[1]=z;
}

#endif

