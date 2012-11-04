//---------------------------------------------------------------------------------------
/// @file SWCompression.h
/// Constants, structures, and function prototypes for LZSS compression/decompression
//---------------------------------------------------------------------------------------

#ifndef __SWCOMPRESSION__
#define __SWCOMPRESSION__

#ifndef __SWCOMMON__
#include <SWCommonHeaders.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* compression (write/pack) */

typedef struct SWCompress *SWCompressPtr;

SWError SWCompressInit(SWCompressPtr *newState);
void SWCompressExit(SWCompressPtr *oldState);

long SWCompress(const void *inData, long inLength, void *outData, long outLength);
void SWCompress_RW(SWCompressPtr state, SDL_RWops *in, SDL_RWops *out);

/* decompression (read/unpack) */

typedef struct SWDecompress *SWDecompressPtr;

SWError SWDecompressInit(SWDecompressPtr *newState);
void SWDecompressExit(SWDecompressPtr *oldState);

long SWDecompress(const void *inData, long inLength, void *outData, long outLength);
void SWDecompress_RW(SWDecompressPtr state, SDL_RWops *in, SDL_RWops *out);

#ifdef __cplusplus
}
#endif

#endif // __SWCOMPRESSION__
