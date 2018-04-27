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

//
// g_mem.c
//
// Golliwog: All rewritten to allow deallocation


#include "g_local.h"


#define POOLSIZE		(384 * 1024)
#define	FREEMEMCOOKIE	((int)0xDEADBE3F)	// Any unlikely to be used value
#define	ROUNDBITS		31					// Round to 32 bytes

struct freememnode {
	// Size of ROUNDBITS
	int cookie, size;				// Size includes node (obviously)
	struct freememnode *prev, *next;
};

static char		memoryPool[POOLSIZE];
static struct freememnode *freehead;
static int		freemem;

void QDECL G_MemLogPrintf( const char *fmt, ... ) {
	va_list		argptr;
	char		string[1024];
	int			min, tens, sec;
	int			offset;

	if ( !level.memLogFile ) {
		return;
	}

	sec = level.time / 1000;

	min = sec / 60;
	sec -= min * 60;
	tens = sec / 10;
	sec -= tens * 10;

	Com_sprintf( string, sizeof(string), "%3i:%i%i ", min, tens, sec );
	offset = strlen(string);

	va_start( argptr, fmt );
	Q_vsnprintf( string + offset, sizeof(string) - offset, fmt, argptr );
	va_end( argptr );

	trap_FS_Write( string, strlen( string ), level.memLogFile );
}

void *G_Alloc( int size )
{
	// Find a free block and allocate.
	// Does two passes, attempts to fill same-sized free slot first.

	struct freememnode *fmn, *prev, *next, *smallest;
	int allocsize, smallestsize;
	char *endptr;
	int *ptr;

	allocsize = ( size + sizeof(int) + ROUNDBITS ) & ~ROUNDBITS;		// Round to 32-byte boundary
	ptr = NULL;

	G_MemLogPrintf("Attempting Alloc of %i bytes\n", allocsize);

	smallest = NULL;
	smallestsize = POOLSIZE + 1;		// Guaranteed not to miss any slots :)
	for( fmn = freehead; fmn; fmn = fmn->next )
	{
		if( fmn->cookie != FREEMEMCOOKIE )
			G_Error( "G_Alloc: Memory corruption detected!" );

		if( fmn->size >= allocsize )
		{
			// We've got a block
			if( fmn->size == allocsize )
			{
				G_MemLogPrintf("Free Block Found: %p\n", fmn);

				// Same size, just remove

				prev = fmn->prev;
				next = fmn->next;

				if(prev == next) {
					G_Error( "G_Alloc: mem chain corrupt");
				}

				if(next && next->prev != fmn) {
					G_Error( "G_Alloc: mem chain corrupt");
				}

				if(prev && prev->next != fmn) {
					G_Error( "G_Alloc: mem chain corrupt");
				}

				if( prev )
					prev->next = next;			// Point previous node to next
				if( next )
					next->prev = prev;			// Point next node to previous
				if( fmn == freehead )
					freehead = next;			// Set head pointer to next
				ptr = (int *) fmn;
				break;							// Stop the loop, this is fine
			}
			else {
				// Keep track of the smallest free slot
				if( fmn->size < smallestsize )
				{
					smallest = fmn;
					smallestsize = fmn->size;
				}
			}
		}
	}
	if( !ptr && smallest )
	{
		// We found a slot big enough
		smallest->size -= allocsize;
		
		G_MemLogPrintf("Allocing From Smallest Larger Block: %p\n", smallest);
		G_MemLogPrintf("Resizing Block To: %i\n", smallest->size);

		endptr = (char *) smallest + smallest->size;
		ptr = (int *) endptr;

		G_MemLogPrintf("Location of New Block: %p\n", ptr);
	}

	if( ptr )
	{
		freemem -= allocsize;
		G_MemLogPrintf("Freemem: %i\n", freemem);

		if( g_debugAlloc.integer )
			G_Printf( "G_Alloc of %i bytes (%i left)\n", allocsize, freemem );
		memset( ptr, 0, allocsize );
		*ptr++ = allocsize;				// Store a copy of size for deallocation

		G_MemLogPrintf("Returning Memory Adress At: %p\n", ptr);
		return( (void *) ptr );
	}

	G_MemLogPrintf("Memory Allocation Failure\n");
	G_Error( "G_Alloc: failed on allocation of %i bytes", size );
	return( NULL );
}

void G_Free( void *ptr )
{
	// Release allocated memory, add it to the free list.

	struct freememnode *fmn, *parent;
	char *freeend;
	int *freeptr;

	if( !ptr ) {
		return;
	}

	freeptr = ptr;
	freeptr--;

	G_MemLogPrintf("Freeing Memory At: %p, Memblock Pos: %p, Memblock Size: %i\n", ptr, freeptr, *freeptr);

	if(*freeptr <= 0) {
		G_MemLogPrintf("WARNING: Attempt to free memory memory <= 0 bytes: %i\n", *freeptr);
		return;
	}

	freemem += *freeptr;
	if( g_debugAlloc.integer )
		G_Printf( "G_Free of %i bytes (%i left)\n", *freeptr, freemem );

	for( fmn = freehead; fmn; )
	{
		freeend = ((char *) fmn) + fmn->size;
		if( ((char *) freeptr >= (char *) fmn) && ((char *) freeptr < freeend)) {
			freemem -= *freeptr;
			G_MemLogPrintf("WARNING: Attempt to free memory in free space\n");
			return;
		}

		if( freeend == (char *) freeptr )
		{
			G_MemLogPrintf("Merging Memory To Existing Block: %p, size: %i\n", fmn, fmn->size);
			// Released block can be merged to an existing node			
			fmn->size += *freeptr;		// Add size of node.
			G_MemLogPrintf("New Size: %i\n", fmn->size);

			memset(freeptr, 0, *freeptr);
			return;
		}

		parent = fmn;
		fmn = fmn->next;
		if( fmn == parent || (fmn && fmn->next == parent) ) {
#ifdef DEBUG_MEM
			G_MemDebug_Close();
#endif
			G_Error( "Horrific error in mem code!" );
		}
	}
	// No merging, add to head of list
	fmn = (struct freememnode *) freeptr;
	fmn->size = *freeptr;				// Set this first to avoid corrupting *freeptr
	fmn->cookie = FREEMEMCOOKIE;
	fmn->prev = NULL;
	fmn->next = freehead;
	freehead->prev = fmn;

	G_MemLogPrintf("Head Free Memory Block: %p, Size: %i, Next: %p, Prev: %p\n", freehead, freehead->size, freehead->next, freehead->prev);

	freehead = fmn;

	G_MemLogPrintf("Creating New Free Memory Block: %p, Size: %i, Next: %p, Prev: %p\n", fmn, fmn->size, fmn->next, fmn->prev);
}

void G_InitMemory( void )
{
	// Set up the initial node

	freehead = (struct freememnode *) memoryPool;
	freehead->cookie = FREEMEMCOOKIE;
	freehead->size = POOLSIZE;
	freehead->next = NULL;
	freehead->prev = NULL;
	freemem = sizeof(memoryPool);

	G_MemLogPrintf("Initializing memory:\n");
}

void G_DefragmentMemory( void )
{
	// If there's a frenzy of deallocation and we want to
	// allocate something big, this is useful. Otherwise...
	// not much use.

	struct freememnode *startfmn, *endfmn, *fmn, *prev, *next;

	G_MemLogPrintf("Defragging memory:\n");

	for( startfmn = freehead; startfmn; )
	{
		endfmn = (struct freememnode *)(((char *) startfmn) + startfmn->size);
		for( fmn = freehead; fmn; )
		{
			if( fmn->cookie != FREEMEMCOOKIE )
				G_Error( "G_DefragmentMemory: Memory corruption detected!" );

			if( fmn == endfmn )
			{
				G_MemLogPrintf("Merging Free Mem Nodes: %p, %i -> %p, %i\n", startfmn, startfmn->size, fmn, fmn->size);

				// We can add fmn onto startfmn.
				prev = fmn->prev;
				next = fmn->next;

				if(prev == next) {
					G_Error( "G_Alloc: mem chain corrupt");
				}

				if(next && next->prev != fmn) {
					G_Error( "G_Alloc: mem chain corrupt");
				}

				if(prev && prev->next != fmn) {
					G_Error( "G_Alloc: mem chain corrupt");
				}

				if( prev )
					prev->next = next;
				if( next )
					next->prev = prev;
				if( fmn == freehead )
					freehead = next;			// Set head pointer to next
				startfmn->size += fmn->size;
				memset( fmn, 0, sizeof(struct freememnode) );	// A redundant call, really.

				startfmn = freehead;
				endfmn = fmn = NULL;				// Break out of current loop
			}
			else {
				fmn = fmn->next;
			}
		}
		if( endfmn )
			startfmn = startfmn->next;		// endfmn acts as a 'restart' flag here
	}
	
	G_MemLogPrintf("Ending Memory Defrag:\n");
}

void Svcmd_GameMem_f( void )
{
	// Give a breakdown of memory

	struct freememnode *fmn;

	G_Printf( "Game memory status: %i out of %i bytes allocated\n", POOLSIZE - freemem, POOLSIZE );

	for( fmn = freehead; fmn; fmn = fmn->next ) {
		G_Printf( "  %p: %d bytes free.\n", fmn, fmn->size );
	}
	G_Printf( "Status complete.\n" );
}

#ifdef DEBUG_MEM
void G_MemDebug_Init() {
	trap_FS_FOpenFile( "memory.log", &level.memLogFile, FS_APPEND );
	if ( !level.memLogFile ) {
		G_Printf( "WARNING: Couldn't open logfile: memory.log\n");
	} else {
		G_MemLogPrintf("------------------------------------------------------------\n" );
		G_MemLogPrintf("MemDebug Init:\n" );
	}
}

void G_MemDebug_Close() {
	if ( !level.memLogFile ) {
		return;
	}

	G_MemLogPrintf("MemDebug Shutdown:\n");
	G_MemLogPrintf("------------------------------------------------------------\n" );

	trap_FS_FCloseFile( level.memLogFile );
	level.memLogFile = 0;
}
#endif

