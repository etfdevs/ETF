#pragma once

#include "q_math.h"

#if defined(__cplusplus)
extern "C" {
#endif

qboolean COM_BitCheck( const int array[], int bitNum );
void COM_BitSet( int array[], int bitNum );
void COM_BitClear( int array[], int bitNum );

#if defined(__cplusplus)
} // extern "C"
#endif
