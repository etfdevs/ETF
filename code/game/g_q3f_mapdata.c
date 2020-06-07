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
**	g_q3f_mapdata.c
**
**	Utility functions to handle arrays/keypairs.
**
*/


#include "g_local.h"
#include "g_q3f_mapdata.h"

q3f_array_t *G_Q3F_ArrayCreate()
{
	// Create a new array and data for it.

	q3f_array_t *newarray;

	if( !(newarray = G_Alloc( sizeof(q3f_array_t) )) )
		return( NULL );
	newarray->max = 4;			// Assuming 8 bytes per entry, this is one block
	if( (newarray->data = G_Alloc( sizeof(q3f_data_t) * 4 )) != NULL )
		return( newarray );
	G_Free( newarray );
	return( NULL );
}

void G_Q3F_ArrayDestroy( q3f_array_t *array )
{
	// Destroy this array

	int index;
	q3f_data_t *data;

	if( !array )
		return;

	index = -1;
	while( (data = G_Q3F_ArrayTraverse( array, &index )) != NULL )
	{
		// Free any strings before deallocating.
		if( data->type == Q3F_TYPE_STRING )
			G_Q3F_RemString( &data->d.strdata );
		else if( data->type == Q3F_TYPE_ARRAY )
			G_Q3F_ArrayDestroy( data->d.arraydata );
		else if( data->type == Q3F_TYPE_KEYPAIRARRAY )
			G_Q3F_KeyPairArrayDestroy( data->d.keypairarraydata );
	}
	G_Free( array->data );
	G_Free( array );
}

int G_Q3F_ArrayAdd( q3f_array_t *array, char type, char flags, uintptr_t data )
{
	// Add a new entry to the array

	q3f_data_t *ptr;
	int index;

	if( !array )
		return( -1 );

	if ( type < Q3F_TYPE_INTEGER || type > Q3F_TYPE_GENERICPTR ) {
		G_Error("G_Q3F_ArrayAdd: Bad type index!");
		return( -1 );
	}

	if( array->used >= array->max )
	{
		// Need to allocate a bigger chunk.

		if( !(ptr = G_Alloc( sizeof(q3f_data_t) * array->max * 2 )) )
			return( -1 );
		memcpy( ptr, array->data, sizeof(q3f_data_t) * array->max );
		array->max *= 2;
		G_Free( array->data );
		array->data = ptr;
	}

	for( index = 0; index <= array->max; index++ )
	{
		ptr = array->data + index;
		if( !ptr->type )
		{
			// We've got a slot, add it in.

			ptr->flags	= flags;
			ptr->type	= type;
			switch(type)
			{
				case Q3F_TYPE_INTEGER:
					ptr->d.intdata = (int)data;
					break;
				case Q3F_TYPE_STRING:
					G_Q3F_AddString( &ptr->d.strdata, (char *) data );
					break;
				case Q3F_TYPE_FLOAT:
					ptr->d.floatdata = (float)data;
					break;
				case Q3F_TYPE_ENTITY:
					ptr->d.entitydata = (struct gentity_s *)data;
					break;
				case Q3F_TYPE_ARRAY:
					ptr->d.arraydata = (struct q3f_array_s *)data;
					break;
				case Q3F_TYPE_KEYPAIRARRAY:
					ptr->d.keypairarraydata = (struct q3f_keypairarray_s *)data;
					break;
				case Q3F_TYPE_OTHER:
				//case Q3F_TYPE_GENERICPTR:
					ptr->d.ptrdata = data;
					break;
				default:
					// will never be reached due to above error
					break;
			}
			array->used++;
			return( index );
		}
	}
	return( -1 );		// Something horrible happened :(
}

void G_Q3F_ArrayDel( q3f_array_t *array, int index )
{
	// Remove an entry from the array

	q3f_data_t *ptr, *newdata;
	int newsize, newindex, oldindex;

	if( !array || (index > array->max) || (index == -1) )
		return;
	ptr = array->data + index;
	if( ptr->type == Q3F_TYPE_STRING )
		G_Q3F_RemString( &ptr->d.strdata );
	else if( ptr->type == Q3F_TYPE_ARRAY )
		G_Q3F_ArrayDestroy( ptr->d.arraydata );
	else if( ptr->type == Q3F_TYPE_KEYPAIRARRAY )
		G_Q3F_KeyPairArrayDestroy( ptr->d.keypairarraydata );
	ptr->type	= Q3F_TYPE_NULL;
	ptr->flags	= 0;
	ptr->d.ptrdata = 0;

	array->used--;
	if( array->used*2 <= array->max && array->max > 4 )
	{
		// Shrink the array

		newsize = (array->used > 4 ? array->used : 4);
		if( !(newdata = G_Alloc( sizeof(q3f_data_t) * newsize )) )
			return;		// Couldn't allocate, not a good sign...
		for( newindex = oldindex = 0; newindex < array->used && oldindex < array->max; oldindex++ )
		{
			// Scan through the old array, copying all entries to the new.
			ptr = array->data + oldindex;
			if( ptr->type )
			{
				memcpy( newdata + newindex, ptr, sizeof(q3f_data_t) );
				newindex++;
			}
		}
		G_Free( array->data );
		array->data = newdata;
		array->max = newsize;
	}
}

q3f_data_t *G_Q3F_ArrayTraverse( q3f_array_t *array, int *index )
{
	// Go through the array looking for a non-null entry.

	q3f_data_t *ptr;

	if( array )
	{
		while( ++*index < array->max )
		{
			ptr = array->data + *index;
			if( ptr->type )
				return( ptr );
		}
	}

	*index = -1;
	return( NULL );
}

void G_Q3F_ArrayConsolidate( q3f_array_t *array )
{
	// Minimize size of array.

	q3f_data_t *ptr, *newdata;
	int newindex, oldindex;

	if( !array )
		return;

	if( array->max > 4 )
	{
		// Shrink the array

		if( !(newdata = G_Alloc( sizeof(q3f_data_t) * array->used )) )
			return;		// Couldn't allocate, not a good sign...
		for( newindex = oldindex = 0; newindex < array->used && oldindex < array->max; oldindex++ )
		{
			// Scan through the old array, copying all entries to the new.
			ptr = array->data + oldindex;
			if( ptr->type )
			{
				memcpy( newdata + newindex, ptr, sizeof(q3f_data_t) );
				newindex++;
			}
		}
		G_Free( array->data );
		array->data = newdata;
		array->max = array->used;
	}
}

static int QDECL AS_SortFunc( const void *a, const void *b )
{
	// Comparison function
	const q3f_data_t *ad = (const q3f_data_t *)a;
	const q3f_data_t *bd = (const q3f_data_t *)b;

	if( !ad->type )
		return( 1 );
	if( !bd->type )
		return( -1 );

	assert(ad->type == bd->type); // arary's can't have two separate types anyway
	switch(ad->type) {
		case Q3F_TYPE_INTEGER:
		case Q3F_TYPE_FLOAT:
			if( ad->d.intdata == bd->d.intdata )
				return( 0 );
			return( (ad->d.intdata < bd->d.intdata) ? -1 : 1 );
		case Q3F_TYPE_STRING:
			// Ensiform: TODO should this not sort by actual string comparison?
			// Original code always uses integer sorts
			if( ad->d.strdata == bd->d.strdata )
				return( 0 );
			return( (ad->d.strdata < bd->d.strdata) ? -1 : 1 );
		default:
			if( ad->d.ptrdata == bd->d.ptrdata )
				return( 0 );
			return( (ad->d.ptrdata < bd->d.ptrdata) ? -1 : 1 );
	}
}
void G_Q3F_ArraySort( q3f_array_t *array )
{
	// Quicksort the array so that the find function is considerably faster.

	if( !array || !array->used )
		return;
	qsort( array->data, array->max, sizeof(q3f_data_t), &AS_SortFunc );
}

q3f_data_t *G_Q3F_ArrayFind( q3f_array_t *array, uintptr_t value )
{
	// Find the specified value

	int min, max, curr;
	q3f_data_t *ptr;

	if( !array || !array->used )
		return( NULL );
	min = 0;
	max = array->max;

	while( min <= max )
	{
		curr = ((max - min) >> 1) + min;	// Get middle index
		ptr = array->data + curr;
		if( !ptr->type || ptr->d.intdata > value )
			max = curr - 1;					// Search lower half
		else if( ptr->d.intdata < value )
			min = curr + 1;					// Search upper half
		else return( ptr );					// Found, return
	}
	return( NULL );
}

q3f_array_t *G_Q3F_ArrayCopy( q3f_array_t *array )
{
	// Copy an array, as well as any strings/arrays/keypairarrays etc.

	q3f_array_t *newarray;
	int index;
	q3f_data_t *data;

	if( !array || !array->used || !(newarray = G_Q3F_ArrayCreate()) )
		return( NULL );

	for( index = -1; (data = G_Q3F_ArrayTraverse( array, &index )) != NULL; )
	{
		if( data->type == Q3F_TYPE_ARRAY )
			G_Q3F_ArrayAdd( newarray, data->type, data->flags, (uintptr_t) G_Q3F_ArrayCopy( data->d.arraydata ) );
		else if( data->type == Q3F_TYPE_KEYPAIRARRAY )
			G_Q3F_ArrayAdd( newarray, data->type, data->flags, (uintptr_t) G_Q3F_KeyPairArrayCopy( data->d.keypairarraydata ) );
		else G_Q3F_ArrayAdd( newarray, data->type, data->flags, data->d.ptrdata );
	}
	return( newarray );
}

/* =========== BEGIN KEYPAIR ARRAYS =========== */

q3f_keypairarray_t *G_Q3F_KeyPairArrayCreate()
{
	// Create a new array and data for it.

	q3f_keypairarray_t *newarray;

	if( !(newarray = G_Alloc( sizeof(q3f_keypairarray_t) )) )
		return( NULL );
	newarray->max = 4;			// Assuming 8 bytes per entry, this is one block
	if( (newarray->data = G_Alloc( sizeof(q3f_keypair_t) * 4 )) != NULL )
		return( newarray );
	G_Free( newarray );
	return( NULL );
}

void G_Q3F_KeyPairArrayDestroy( q3f_keypairarray_t *array )
{
	// Destroy this array

	int index;
	q3f_keypair_t *data;

	if( !array )
		return;

	index = -1;
	while( (data = G_Q3F_KeyPairArrayTraverse( array, &index )) != NULL )
	{
		// Free any strings/subarrays before deallocating.
		G_Q3F_RemString( &data->key );
		if( data->value.type == Q3F_TYPE_STRING )
			G_Q3F_RemString( &data->value.d.strdata );
		else if( data->value.type == Q3F_TYPE_ARRAY )
			G_Q3F_ArrayDestroy( data->value.d.arraydata );
		else if( data->value.type == Q3F_TYPE_KEYPAIRARRAY )
			G_Q3F_KeyPairArrayDestroy( data->value.d.keypairarraydata );
	}
	G_Free( array->data );
	G_Free( array );
}

int G_Q3F_KeyPairArrayAdd( q3f_keypairarray_t *array, char *key, char type, char flags, uintptr_t data )
{
	// Add a new entry to the array

	q3f_keypair_t *ptr;
	int index;

	if( !array )
		return( -1 );

	if( array->used >= array->max )
	{
		// Need to allocate a bigger chunk.

		if( !(ptr = G_Alloc( sizeof(q3f_keypair_t) * array->max * 2 )) )
			return( -1 );
		memcpy( ptr, array->data, sizeof(q3f_keypair_t) * array->max );
		array->max *= 2;
		G_Free( array->data );
		array->data = ptr;
	}

	for( index = 0; index <= array->max; index++ )
	{
		ptr = array->data + index;
		if( !ptr->value.type )
		{
			// We've got a slot, add it in.

			ptr->value.flags	= flags;
			ptr->value.type		= type;
			G_Q3F_AddString( &ptr->key, (char *) key );
			switch(type)
			{
				case Q3F_TYPE_INTEGER:
					ptr->value.d.intdata = (int)data;
					break;
				case Q3F_TYPE_STRING:
					G_Q3F_AddString( &ptr->value.d.strdata, (char *) data );
					break;
				case Q3F_TYPE_FLOAT:
					ptr->value.d.floatdata = (float)data;
					break;
				case Q3F_TYPE_ENTITY:
					ptr->value.d.entitydata = (struct gentity_s *)data;
					break;
				case Q3F_TYPE_ARRAY:
					ptr->value.d.arraydata = (struct q3f_array_s *)data;
					break;
				case Q3F_TYPE_KEYPAIRARRAY:
					ptr->value.d.keypairarraydata = (struct q3f_keypairarray_s *)data;
					break;
				case Q3F_TYPE_OTHER:
				//case Q3F_TYPE_GENERICPTR:
					ptr->value.d.ptrdata = data;
					break;
				default:
					// will never be reached due to above error
					break;
			}
			array->used++;
			return( index );
		}
	}
	return( -1 );		// Something horrible happened :(
}

void G_Q3F_KeyPairArrayDel( q3f_keypairarray_t *array, char *key )
{
	// Remove an entry from the array

	q3f_keypair_t *ptr, *newdata;
	int newsize, newindex, oldindex;

	if( !array || !key || !*key )
		return;

	oldindex = -1;
	while( (ptr = G_Q3F_KeyPairArrayTraverse( array, &oldindex )) != NULL )
	{
		if( ptr->key == key )
			break;
	}
	if( oldindex == -1 )	// RR2DO2: /me twaps Golliwog.. fixing this to == -1 from = -1. And that 2 years into the project
		return;		// Not found in array?

	G_Q3F_RemString( &ptr->key );
	if( ptr->value.type == Q3F_TYPE_STRING )
		G_Q3F_RemString( &ptr->value.d.strdata );
	else if( ptr->value.type == Q3F_TYPE_ARRAY )
		G_Q3F_ArrayDestroy( ptr->value.d.arraydata );
	else if( ptr->value.type == Q3F_TYPE_KEYPAIRARRAY )
		G_Q3F_KeyPairArrayDestroy( ptr->value.d.keypairarraydata );

	ptr->value.type	= Q3F_TYPE_NULL;
	ptr->value.flags	= 0;
	ptr->value.d.ptrdata = 0;

	array->used--;
	if( array->used*2 < array->max && array->max > 4 )
	{
		// Shrink the array

		newsize = (array->used > 4 ? array->used : 4);
		if( !(newdata = G_Alloc( sizeof(q3f_keypair_t) * newsize )) )
			return;		// Couldn't allocate, not a good sign...
		for( newindex = oldindex = 0; newindex < array->used && oldindex < array->max; oldindex++ )
		{
			// Scan through the old array, copying all entries to the new.
			ptr = array->data + oldindex;
			if( ptr->value.type )
			{
				memcpy( newdata + newindex, ptr, sizeof(q3f_keypair_t) );
				newindex++;
			}
		}
		G_Free( array->data );
		array->data = newdata;
		array->max = newsize;
	}
}

q3f_keypair_t *G_Q3F_KeyPairArrayTraverse( q3f_keypairarray_t *array, int *index )
{
	// Go through the array looking for a non-null entry.

	q3f_keypair_t *ptr;

	if( array )
	{
		while( ++*index < array->max )
		{
			ptr = array->data + *index;
			if( ptr->value.type )
				return( ptr );
		}
	}

	*index = -1;
	return( NULL );
}

void G_Q3F_KeyPairArrayConsolidate( q3f_keypairarray_t *array )
{
	// Minimize size of array.

	q3f_keypair_t *ptr, *newdata;
	int newindex, oldindex;

	if( !array )
		return;

	if( array->max > 4 )
	{
		// Shrink the array

		if( !(newdata = G_Alloc( sizeof(q3f_keypair_t) * array->used )) )
			return;		// Couldn't allocate, not a good sign...
		for( newindex = oldindex = 0; newindex < array->used && oldindex < array->max; oldindex++ )
		{
			// Scan through the old array, copying all entries to the new.
			ptr = array->data + oldindex;
			if( ptr->value.type )
			{
				memcpy( newdata + newindex, ptr, sizeof(q3f_keypair_t) );
				newindex++;
			}
		}
		G_Free( array->data );
		array->data = newdata;
		array->max = array->used;
	}
}

static int QDECL KPAS_SortFunc( const void *a, const void *b )
{
	// Comparison function
	const q3f_keypair_t *akp = (const q3f_keypair_t *)a;
	const q3f_keypair_t *bkp = (const q3f_keypair_t *)b;

	if( !akp->value.type )
		return( 1 );
	if( !bkp->value.type )
		return( -1 );

	if( akp->key == bkp->key )
		return( 0 );
	return( (akp->key < bkp->key) ? -1 : 1 );
}
void G_Q3F_KeyPairArraySort( q3f_keypairarray_t *array )
{
	// Quicksort the array so that the find function is considerably faster.

	if( !array || !array->used )
		return;
	qsort( array->data, array->max, sizeof(q3f_keypair_t), &KPAS_SortFunc );
}

q3f_keypair_t *G_Q3F_KeyPairArrayFind( q3f_keypairarray_t *array, char *key )
{
	// Find the specified value

	int min, max, curr;
	q3f_keypair_t *ptr;

	if( !array || !array->used )
		return( NULL );
	min = 0;
	max = array->max;

	while( min <= max )
	{
		curr = ((max - min) >> 1) + min;	// Get middle index
		ptr = array->data + curr;
		if( !ptr->value.type || ptr->key > key )
			max = curr - 1;					// Search lower half
		else if( ptr->key < key )
			min = curr + 1;					// Search upper half
		else return( ptr );					// Found, return
	}
	return( NULL );
}

q3f_keypairarray_t *G_Q3F_KeyPairArrayCopy( q3f_keypairarray_t *array )
{
	// Copy an array, as well as any strings/arrays/keypairarrays etc.

	q3f_keypairarray_t *newarray;
	int index;
	q3f_keypair_t *data;

	if( !array || !array->used || !(newarray = G_Q3F_KeyPairArrayCreate()) )
		return( NULL );

	for( index = -1; (data = G_Q3F_KeyPairArrayTraverse( array, &index )) != NULL; )
	{
		if( data->value.type == Q3F_TYPE_ARRAY )
			G_Q3F_KeyPairArrayAdd( newarray, data->key, data->value.type, data->value.flags, (uintptr_t) G_Q3F_ArrayCopy( data->value.d.arraydata ) );
		else if( data->value.type == Q3F_TYPE_KEYPAIRARRAY )
			G_Q3F_KeyPairArrayAdd( newarray, data->key, data->value.type, data->value.flags, (uintptr_t) G_Q3F_KeyPairArrayCopy( data->value.d.keypairarraydata ) );
		else G_Q3F_KeyPairArrayAdd( newarray, data->key, data->value.type, data->value.flags, data->value.d.ptrdata );
	}
	return( newarray );
}
