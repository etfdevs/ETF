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

/*
**	g_q3f_string.c
**
**	Search for and handle strings in a tree.
**
*/

#include "g_local.h"

typedef struct q3f_stringnode_s {
	struct q3f_stringnode_s *left, *right;
	unsigned char refcount;		// A waste of space, really, but LCC doesn't like bytes.
	char str[1];
} q3f_stringnode_t;

#define MAXREFCOUNT	255


q3f_stringnode_t *stringroot;

qboolean G_Q3F_AddString( char **target, char *str )
{
	// Add a new string to the array.

	q3f_stringnode_t *curr, *parent;
	int cmpresult;

	if( !stringroot )
	{
		stringroot = G_Alloc( sizeof(q3f_stringnode_t) );
		stringroot->left = NULL;
		stringroot->right = NULL;
		stringroot->refcount = MAXREFCOUNT;
		stringroot->str[0] = 0;
	}

	if( !str )			// Return empty strings for null arguments
		str = "";

	curr = stringroot;
	while( curr )
	{
		if( !(cmpresult = strcmp( str, curr->str )) )
			break;		// Break loop if we get a match
		parent = curr;
		curr = (cmpresult < 0) ? curr->left : curr->right;
		if( curr == parent )
			G_Error( "Horrific error in string code!" );
	}

	if( curr )
	{
		// Existing entry, just bump the reference count

		if( curr->refcount < MAXREFCOUNT )
			curr->refcount++;
	}
	else {
		// New entry, add it to the parent

		if( !(curr = G_Alloc( sizeof(q3f_stringnode_t) + strlen( str )) ) )
			return( qfalse );
		strcpy( curr->str, str );
		curr->refcount++;
		if( cmpresult < 0 )			// Attach it to the parent
			parent->left = curr;
		else parent->right = curr;
	}

	*target = curr->str;
	return( qtrue );
}

void G_Q3F_RemString( char **target )
{
	// Remove an existing string from the array

	q3f_stringnode_t *curr, *parent, *defunct, *defunctparent;
	char *str;
	int cmpresult;

	if( !*target || !**target )
		return;			// It's NULL or "" (the root string)

	str = *target;
	*target = NULL;

	curr = stringroot;
	while( curr )
	{
		if( !(cmpresult = strcmp( str, curr->str )) )
			break;		// Break loop if we get a match
		parent = curr;
		curr = (cmpresult < 0) ? curr->left : curr->right;
	}
	if( !curr )
		return;			// String wasn't found *shrug*

	if(	(curr->refcount == MAXREFCOUNT) || --curr->refcount )
		return;			// Ref count decreased, return

	// We now have to unlink the node, and repair the tree.
	// This involves finding the lowest numbered (?) node to the right
	// of the unlinked number, and putting it in. The replacement node
	// may have it's own right-child (but no left), which is attached
	// to the replacement. Or something ;)

	if( !curr->left )
	{
		// No left node, just stick the right node in it place
		if( parent->left == curr )
			parent->left = curr->right;
		else parent->right = curr->right;
		G_Free( curr );			// And remove it now it's unlinked.
		return;
	}
	if( !curr->right )
	{
		// No right node, just stick the left node in it place
		if( parent->left == curr )
			parent->left = curr->left;
		else parent->right = curr->left;
		G_Free( curr );			// And remove it now it's unlinked.
		return;
	}

	defunct			= curr;
	defunctparent	= parent;

	parent = curr;
	curr = curr->right;
	while( curr->left )
	{
		parent = curr;
		curr = curr->left;		// Just traverse left for the smallest number
	}

	if( defunct != parent )
	{
		// curr is being inserted into the place of defunct
		parent->left = curr->right;		// Patch up this gap (there is no curr->left)

		if( defunctparent->left == defunct )
			defunctparent->left = curr;
		else defunctparent->right = curr;
		curr->left = defunct->left;		// And now curr has dumped it's dependant node,
		curr->right = defunct->right;	// Put it in palce of defunct;
	}
	else {
		// curr is replacing it's own parent

		if( defunctparent->left == defunct )
			defunctparent->left = curr;
		else defunctparent->right = curr;
		if( defunct->left != curr )
			curr->left = defunct->left;		// And now curr has dumped it's dependant node,
		if( defunct->right != curr )
			curr->right = defunct->right;	// Put it in place of defunct;
	}

	G_Free( defunct );			// And remove it now it's unlinked.
}

char *G_Q3F_GetString( char *str )
{
	// Get a string from the tree without adding it.

	q3f_stringnode_t *curr;
	int cmpresult;

	if( !str )			// Return empty strings for null arguments
		str = "";

	curr = stringroot;
	while( curr )
	{
		if( !(cmpresult = strcmp( str, curr->str )) )
			break;		// Break loop if we get a match
		curr = (cmpresult < 0) ? curr->left : curr->right;
	}
	return( curr ? curr->str : "" );	// No match
}

void Q3F_Svcmd_GameStrings_f( void )
{
	int nodecount;
	q3f_stringnode_t *node;
	q3f_stringnode_t **nodestack;
	char *statestack;
	int total, left, right;

	if( !(nodestack = G_Alloc( sizeof(q3f_stringnode_t *) * 1000 )) )
		return;
	if( !(statestack = G_Alloc( sizeof(char) * 1000 )) )
	{
		G_Free( nodestack );
		return;
	}
	
	nodecount = 0;
	total = left = right = 0;

	G_Printf( "Game strings:\n" );
	node = stringroot->right;		// Skip the empty string
	nodestack[nodecount] = stringroot;
	while( nodecount >= 0 )
	{
		node = nodestack[nodecount];
		if( !statestack[nodecount] )	// Have we traversed left yet?
		{
			statestack[nodecount] = 1;	// Return in state 1.
			if( node->left )
			{
				left++;
				if( nodecount >= 1000 )
				{
					G_Free( nodestack );
					G_Printf( "Stack overflow. (That's a LOT of strings :)\n" );
					return;
				}
				nodestack[++nodecount] = node->left;	// Add this node for next pass
				statestack[nodecount] = 0;
			}
		}
		else {
			total++;
			if( node->refcount == MAXREFCOUNT )
				G_Printf( "  MAX  %s\n", node->str );
			else G_Printf( "  %dx  %s\n", node->refcount, node->str );
			if( node->right )
			{
				// Replace our entry on the stack with this one.
				right++;
				nodestack[nodecount] = node->right;
				statestack[nodecount] = 0;
			}
			else {
				nodecount--;		// Return to previous entries
			}
		}
	}
	G_Free( statestack );
	G_Free( nodestack );

	G_Printf( "%d strings (%d left/%d right).\n", total, left, right );
	G_Printf( "Status complete.\n" );
}

#include "g_q3f_mapdata.h"

/*void TestStringStuff()
{
	// Check it all works.

	int modeindex, loopindex, charindex, strlength;
	char x[200];
	char *target;
	q3f_array_t *array;
	int index;

	return;

	// Random mayhem to string/memory
	for( modeindex = 0; modeindex < 2; modeindex++ )
	{
		srand( 12345 );
		for( loopindex = 0; loopindex < 10000; loopindex++ )
		{
			strlength = rand() % 200;
			for( charindex = 0; charindex < strlength; charindex++ )
				x[charindex] = '0' + (rand() % 10);
			x[charindex] = 0;
			target = x;
			if( modeindex == (loopindex & 1) )
				G_Q3F_AddString( &target, x );
			else G_Q3F_RemString( &target );
		}
	}

		// Remove all strings
	while( stringroot->right )
	{
		target = stringroot->right->str;
		G_Q3F_RemString( &target );
	}

		// Add strings into an array
	if( !(array = G_Q3F_ArrayCreate()) )
		G_Error( "Failed to create array?" );
	for( loopindex = 0; loopindex < 1000; loopindex++ )
	{
		strlength = rand() % 100;
		for( charindex = 0; charindex < strlength; charindex++ )
			x[charindex] = '0' + (rand() % 10);
		x[charindex] = 0;
		target = x;
		G_Q3F_ArrayAdd( array, Q3F_TYPE_STRING, 0, (int) x );
	}

		// And then delete everything from the array.
	while( 1 )
	{
		index = -1;
		if( !G_Q3F_ArrayTraverse( array, &index ) )
			break;
		G_Q3F_ArrayDel( array, index );
	}

		// And attempt to cleanup afterwards
	G_DefragmentMemory();

		// What, no crashes? :)
	G_Printf( "String test OK.\n" );
}*/
