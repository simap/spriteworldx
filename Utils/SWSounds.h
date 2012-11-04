//---------------------------------------------------------------------------------------
/// @file SWSounds.h
/// 	Constants, structures, and function prototypes for loading and playing sounds.
//
//	Created 11/13/96 By Vern Jensen
//	Various additions and file loading routines by Ken Pajala
//  adopted for SWX by afb
//---------------------------------------------------------------------------------------

#ifndef __SWSOUNDS__
#define __SWSOUNDS__

#ifndef __SWCOMMON__
#include <SWCommonHeaders.h>
#endif

#ifdef HAVE_CONFIG_H
#include "config.h"
#else

/// Set to 1 if you have the OpenAL headers
/* #undef HAVE_OPENAL */

/// Set to 1 if you have the SDL_sound library
/* #undef HAVE_SDL_SOUND */

#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef enum
{
	kFindEmptyChannel = 1,
	kPlaySoundInChannel,
	kReplaceSameSound
} PlayType;

typedef enum 
{
	kLoopForever = -1,
	kDoNotLoop = 0
	// any number greater than 0 specifies the number of times to loop
} LoopType;


SWError	CreateSoundChannels(int numChannels);
void	DisposeSoundChannels(void);

SWError	LoadSound(char *filenames[], int numSounds);
void	DisposeSounds(void);

void	PlaySound(
	int soundID, 
	int channelNum, 
	PlayType playType);


#ifdef __cplusplus
}
#endif

#endif /* __SWSOUNDS__ */


