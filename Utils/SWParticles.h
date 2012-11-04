//---------------------------------------------------------------------------------------
///	@file SWParticles.h
//---------------------------------------------------------------------------------------


#ifndef __SWPARTICLES__
#define __SWPARTICLES__



#ifndef __SWCOMMON__
#include <SWCommonHeaders.h>
#endif

#ifndef __SPRITEWORLD__
#include <SpriteWorld.h>
#endif

#ifndef __SPRITEFRAME__
#include <SpriteFrame.h>
#endif

#ifndef __SPRITE__
#include <Sprite.h>
#endif

//#include <SWFixed.h>
//#include <FixMath.h>
//typedef Fixed				SWFixed;
//#define SW_FLOAT2FIX(fl)	((fl) * 65536.0f)
//#define SW_INT2FIX(in)		((long) (in) << 16)
//#define SW_FIX2INT(fix)		((fix) >> 16)

#ifdef __cplusplus
extern "C" {
#endif


//---------------------------------------------------------------------------------------
/// (single pixel) particle struct
//---------------------------------------------------------------------------------------

typedef struct Particle
{
	short			lifeRemaining;		///< How many frames of animation is left for this particle.
                                                        // If it is 0 then this particle is not in the animation
	float			horizLoc;			///< Current horizontal position of the particle
	float			vertLoc;			///< Current vertical position of the particle
	float			oldHorizLoc;			///< Horizontal position of particle the previous frame
	float			oldVertLoc;			///< Vertical position of particle the previous frame
	float			horizSpeed;			///< To be used by the user to move the star
	float			vertSpeed;			///< To be used by the user to move the star
	Uint32			color;				///< The particle's color.
        Uint32			oldColor;			///< The particle's old color for screen updates.
} Particle, *ParticlePtr;

//---------------------------------------------------------------------------------------
//	Globals
//---------------------------------------------------------------------------------------

extern ParticlePtr		gParticleArray;				// An array containing data for each particle.
extern long				gLowestUnusedElement;		// Start at this element when inserting a new particle
extern long				gLastElementInUse;			// The highest element that has an active particle.
extern long				gNumberOfParticlesActive;	// Number of array elements currently in use
extern float			gGravity;					// downward acceleration
extern unsigned long	gNumArrayElements;			// Size of array, unsigned to avoid the 68K optimizer bug
extern SWBoolean		gParticlesInitialized;		// Was InitParticles called successfully?

//---------------------------------------------------------------------------------------
//	Function prototypes
//---------------------------------------------------------------------------------------

SWError InitParticles(long NumParticles, float Gravity);

void ExitParticles(void);

void ClearParticles( void );

void SetHorizGravity(float gravity);

void NewParticle(SpriteWorldPtr	spriteWorldP,
	int red,
        int green,
        int blue,
	short	horizLoc,
	short	vertLoc,
	float	horizSpeed,
	float	vertSpeed,
	short lifeRemaining);
	
	
void EraseParticlesOffscreen( SpriteWorldPtr spriteWorldP );

void EraseParticlesScrollingOffscreen( SpriteWorldPtr spriteWorldP );

void DrawParticlesOffscreen( SpriteWorldPtr spriteWorldP );

void DrawParticlesScrollingOffscreen( SpriteWorldPtr spriteWorldP );

void DrawHardwareParticles( SpriteWorldPtr spriteWorldP );

void UpdateParticlesInWindow( SpriteWorldPtr spriteWorldP );

void UpdateParticlesInScrollingWindow( SpriteWorldPtr spriteWorldP );

void MoveParticles( void );
	
void putpixel(SDL_Surface *surface, int x, int y, Uint32 pixel);

Uint32 getpixel(SDL_Surface *surface, int x, int y);

#ifdef __cplusplus
}
#endif

#endif /* __SWPARTICLES__ */
