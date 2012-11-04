
#ifndef __SWCOMMON__
#include <SWCommonHeaders.h>
#endif

#include <SWSounds.h>

#ifdef HAVE_OPENAL

#if defined(__APPLE__) && defined(__MACH__)
#include <OpenAL/al.h>
#else
#include <al.h>
#endif

static ALfloat listenerPos[] = [ 0.0,0.0,0.0 ];
static ALfloat listenerVel[] = [ 0.0,0.0,0.0 ];
static ALfloat listenerOri[] = [ 0.0,0.0,1.0 ];

static ALuint *sources = NULL;
static ALuint *buffers = NULL;

/// Load OpenAL functions, or return NULL if not available
static struct AL_func *LoadALfunctions(void);

#ifndef ALAPIENTRY
#define ALAPIENTRY    ///< needed for Windows compatibility
#endif

/// OpenAL function pointers
struct AL_func
{
	void (ALAPIENTRY *alListenerfv) ( ALenum pname, const ALfloat* param );
 

};
	

static SDL_bool              AL_functions_loaded = SDL_FALSE;
static struct AL_func        AL_functions;

#endif /* HAVE_OPENAL */

static SWError InitSounds(void);
static SWBoolean			gSoundInited = false;
static SWBoolean			gUseOpenAL = false;

typedef struct wave
{
	SDL_AudioSpec spec;
	Uint8   *sound;			/* Pointer to wave data */
	Uint32   soundlen;		/* Length of wave data */
	int      soundpos;		/* Current play position */
} wave;



SWError InitSounds()
{
	if ( SDL_Init(SDL_INIT_AUDIO) < 0 ) {
        fprintf(stderr, "Unable to init SDL: %s\n", SDL_GetError());
        return kUnknownError;
    }
    
#ifdef HAVE_OPENAL
	if (gUseOpenAL)
	{
		alListenerfv(AL_POSITION,listenerPos);
		alListenerfv(AL_VELOCITY,listenerVel);
		alListenerfv(AL_ORIENTATION,listenerOri);
	}
#endif

	gSoundInited = true;
	return kNoError;
}

SWError	CreateSoundChannels(int numChannels)
{
	if (!gSoundInited)
		InitSounds();



	return kUnknownError;
}

void	DisposeSoundChannels( void )
{

}

SWError	LoadSound( char *filenames[], int numSounds )
{
	if (!gSoundInited)
		InitSounds();


	return kUnknownError;
}

void	DisposeSounds( void )
{

}

void	PlaySound(
	int soundID, 
	int channelNum, 
	PlayType playType)
{

}


#ifdef HAVE_OPENAL

#if defined(__APPLE__) && defined(__MACH__)
#define WEAKLINK_OPENAL			1	// link dynamically to OpenAL ?
#define AL_LIB  				"/Library/Frameworks/OpenAL.framework"
#else
#define WEAKLINK_OPENAL    	  	0
#define AL_LIB                  NULL
#endif

#include <SDL/SDL_loadso.h>

static void *alLibrary = NULL;	// library object/bundle

#ifndef HAVE_OPENAL
#define OPENAL_AVAILABLE(l)     SDL_FALSE
#define OPENAL_FUNCTION(f)      NULL
#elif !WEAKLINK_OPENAL
#define OPENAL_AVAILABLE(l)     SDL_TRUE
#define OPENAL_FUNCTION(f)      &f
#elif WEAKLINK_OPENAL
#if defined(__APPLE__) && defined(__MACH__)
static void *LoadObject(char *filename);
static void *LoadFunction(void *object, char *function);
#define OPENAL_AVAILABLE(l)     (alLibrary = LoadObject(l) != NULL)
#define OPENAL_FUNCTION(f)      LoadFunction(alLibrary, #f);
#else
#define OPENAL_AVAILABLE(l)     (alLibrary = SDL_LoadObject(l) != NULL)
#define OPENAL_FUNCTION(f)      SDL_LoadFunction(alLibrary, #f);
#endif // end Mac OS X workaround
#endif // WEAKLINK_OPENAL

#if defined(__APPLE__) && defined(__MACH__) // workaround for missing SDL functionality

#include <CoreFoundation/CoreFoundation.h>

void *LoadObject(char *filename)
{
	OSStatus err = noErr;

	CFStringRef 	bundleStr;
	CFURLRef 		bundleURL;
	CFBundleRef		bundleRef;

	bundleStr = CFStringCreateWithCStringNoCopy( kCFAllocatorDefault, filename,
						kCFStringEncodingUTF8, kCFAllocatorDefault);

	bundleURL = CFURLCreateWithString(kCFAllocatorDefault, bundleStr, NULL);
	if (bundleURL == NULL)
		return NULL;
	CFRelease (bundleStr);
   
	bundleRef = CFBundleCreate(kCFAllocatorDefault, bundleURL);
	if (bundleRef == NULL)
		return NULL;
	CFRelease (bundleURL);

    return bundleRef;
}

void *LoadFunction(void *object, char *function)
{
    return CFBundleGetFunctionPointerForName (object,
                CFStringCreateWithCStringNoCopy (NULL,
                     function, CFStringGetSystemEncoding (), NULL));
}
#endif // end Mac OS X workaround

struct AL_func *LoadALfunctions(void)
{
    if (!AL_functions_loaded)
    {
        memset(&AL_functions, 0, sizeof(AL_functions));

        if (OPENAL_AVAILABLE(AL_LIB))
        {
            AL_functions.alListenerfv = OPENAL_FUNCTION(alListenerfv);
         }
        else
        {
            // HAVE_OPENAL is not available, or could not be loaded
        #if WEAKLINK_OPENAL
            printf("SDL_LoadObject: %s\n", SDL_GetError());
        #endif
            return NULL;
        }
        
        AL_functions_loaded = SDL_TRUE;
    }

    return &AL_functions;
}

#endif /* HAVE_OPENAL */

