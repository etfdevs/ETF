/*
**	g_q3f_mapdata.h
**
**	keypair and array handling.
**
*/

#ifndef __G_Q3F_MAPDATA_H
#define __G_Q3F_MAPDATA_H

#include "g_local.h"

typedef struct q3f_data_s {		// This should be padded to 8 bytes. (I think)
	union {
		int intdata;
		char *strdata;
		float floatdata;
//		double *doubledata;
		struct gentity_s *entitydata;
		struct q3f_array_s *arraydata;
		struct q3f_keypairarray_s *keypairarraydata;
		void *ptrdata;
	} d;
	char type, flags;
} q3f_data_t;

typedef struct q3f_array_s {
	int used, max;
	q3f_data_t *data;
} q3f_array_t;

typedef struct q3f_keypair_s {
	char *key;
	q3f_data_t value;
} q3f_keypair_t;

typedef struct q3f_keypairarray_s {
	int used, max;
	q3f_keypair_t *data;
} q3f_keypairarray_t;

enum {
	Q3F_TYPE_NULL	= 0,
	Q3F_TYPE_INTEGER,
	Q3F_TYPE_STRING,
	Q3F_TYPE_FLOAT,
//	Q3F_TYPE_DOUBLE,
	Q3F_TYPE_ENTITY,
	Q3F_TYPE_ARRAY,
	Q3F_TYPE_KEYPAIRARRAY,
	Q3F_TYPE_OTHER
};


// Utility functions

q3f_array_t *G_Q3F_ArrayCreate();
void G_Q3F_ArrayDestroy( q3f_array_t *array );
int G_Q3F_ArrayAdd( q3f_array_t *array, char type, char flags, int data );
void G_Q3F_ArrayDel( q3f_array_t *array, int index );
q3f_data_t *G_Q3F_ArrayTraverse( q3f_array_t *array, int *index );
void G_Q3F_ArrayConsolidate( q3f_array_t *array );
void G_Q3F_ArraySort( q3f_array_t *array );
q3f_data_t *G_Q3F_ArrayFind( q3f_array_t *array, int value );
q3f_array_t *G_Q3F_ArrayCopy( q3f_array_t *array );

q3f_keypairarray_t *G_Q3F_KeyPairArrayCreate();
void G_Q3F_KeyPairArrayDestroy( q3f_keypairarray_t * );
int G_Q3F_KeyPairArrayAdd( q3f_keypairarray_t *array, char *key, char type, char flags, int data );
void G_Q3F_KeyPairArrayDel( q3f_keypairarray_t *array, char *key );
q3f_keypair_t *G_Q3F_KeyPairArrayTraverse( q3f_keypairarray_t *array, int *index );
void G_Q3F_KeyPairArrayConsolidate( q3f_keypairarray_t *array );
void G_Q3F_KeyPairArraySort( q3f_keypairarray_t *array );
q3f_keypair_t *G_Q3F_KeyPairArrayFind( q3f_keypairarray_t *array, char *key );
q3f_keypairarray_t *G_Q3F_KeyPairArrayCopy( q3f_keypairarray_t *array );

#endif//__G_Q3F_MAPDATA_H
