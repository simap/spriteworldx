//---------------------------------------------------------------------------------------
// SWParticles.c - by Matthew Foodim, with changes by Anders Bjšrklund and Vern Jensen. 
//         Updated 7-11-03 for SWX by stin
//
// Note: NewParticle currently does not add any new particles if there is no room in the
// gParticleArray.
//---------------------------------------------------------------------------------------

#include <SWIncludes.h>

#include <SWParticles.h>
//#include <HardwareInterface.h>

#include "BlitKernel.h"

//---------------------------------------------------------------------------------------
//  Global variables
//---------------------------------------------------------------------------------------

ParticlePtr     gParticleArray = NULL;          // An array containing data for each particle.
long            gLowestUnusedElement;           // Start at this element when inserting a new particle
long            gLastElementInUse;              // The highest element that has an active particle.
long            gNumberOfParticlesActive;       // Number of array elements currently in use
float           gGravity;                       // downward acceleration
float           gHorizGravity;                      // horiz acceleration
unsigned long   gNumArrayElements;              // Size of array, unsigned to avoid the 68K optimizer bug
SWBoolean       gParticlesInitialized = false;  // Was InitParticles called successfully?

//---------------------------------------------------------------------------------------
//  Unroll code for the different depths, to avoid any per-pixel depth checks
//  Macros, to avoid the bug-prone huge amounts of copy and paste.
//  The optimizer will make it the same, anyway.
//      "Better living through pre-processor"                       --afb
//---------------------------------------------------------------------------------------

/*#define FOREACH_DEPTH_DO(depth,instructions)  
    switch ( depth )    \
    {   case  8:    { int DEPTH=8;  instructions } break;   \
        case 16:    { int DEPTH=16; instructions } break;   \
        case 32:    { int DEPTH=32; instructions } break; }

#define GET_DEPTH_PIXEL(frame,x,y)              \
    ( DEPTH==8 ?    *SW_PIXELPTR8(frame,x,y) :              \
    ( DEPTH==16 ?   *SW_PIXELPTR16(frame,x,y) :             \
                    *SW_PIXELPTR32(frame,x,y) ))

#define SET_DEPTH_PIXEL(frame,x,y,c)            \
      if(DEPTH==8)  *SW_PIXELPTR8(frame,x,y)=c;  else       \
    { if(DEPTH==16) *SW_PIXELPTR16(frame,x,y)=c; else       \
                    *SW_PIXELPTR32(frame,x,y)=c; }
*/

//---------------------------------------------------------------------------------------
//  InitParticles
//---------------------------------------------------------------------------------------

SWError InitParticles(long maxNumParticles, float gravity)
{
    SWError err = kNoError;
    
    gNumArrayElements = maxNumParticles;
    gGravity = gravity;
        gHorizGravity = 0;
    
        // Allocate memory for the particle array
    gParticleArray = (ParticlePtr) malloc(sizeof(Particle) * gNumArrayElements);
    
    if (gParticleArray == NULL)
    {
        err = kMemoryAllocationError;
    }
    else
    {
        ClearParticles();
    }
    
    gLowestUnusedElement = 0;
    gLastElementInUse = 0;
    gNumberOfParticlesActive = 0;
    
    if (err == kNoError)
        gParticlesInitialized = true;
    else
        gParticlesInitialized = false;
    
    return  err;
}

//---------------------------------------------------------------------------------------
//  ExitParticles
//---------------------------------------------------------------------------------------

void ExitParticles()
{
    if (gParticlesInitialized)
    {
        if (gParticleArray != NULL)
            free(gParticleArray);
    
        gNumberOfParticlesActive = 0;
        gNumArrayElements = 0;
        
        gParticlesInitialized = false;
    }
}

//---------------------------------------------------------------------------------------
//  ClearParticles
//---------------------------------------------------------------------------------------

void ClearParticles( void )
{   
    long    curParticle;
    
    SW_ASSERT(gParticleArray != NULL);
    
    for (curParticle = 0; curParticle < gNumArrayElements; curParticle++)
    {
        gParticleArray[curParticle].lifeRemaining = 0;
    }
    
    gLowestUnusedElement = 0;
    gLastElementInUse = 0;
    gNumberOfParticlesActive = 0;
}

//---------------------------------------------------------------------------------------
//  NewParticle - Create a new particle
//---------------------------------------------------------------------------------------

void SetHorizGravity(float gravity)
{
    gHorizGravity = gravity;
}

//---------------------------------------------------------------------------------------
//  NewParticle - Create a new particle
//---------------------------------------------------------------------------------------

void NewParticle(SpriteWorldPtr spriteWorldP,
    int red,
        int green,
        int blue,
    short   horizLoc,
    short   vertLoc,
    float   horizSpeed,
    float   vertSpeed,
    short lifeRemaining)
{
    Uint32  theColor;
        
        theColor = SDL_MapRGB(spriteWorldP->workFrameP->frameSurfaceP->format, red, green, blue);
        
        if (gParticlesInitialized == false)
        return;
    
        // Add a particle only if there is room.
    if (gNumberOfParticlesActive < gNumArrayElements)
    {
            // Find the lowest unused array element
            // (Note: this assumes there *is* an available element.)
        while (gParticleArray[gLowestUnusedElement].lifeRemaining > 0)
            gLowestUnusedElement++;
        
        SW_ASSERT(gLowestUnusedElement < gNumArrayElements);
        
        gParticleArray[gLowestUnusedElement].lifeRemaining = lifeRemaining;
        gParticleArray[gLowestUnusedElement].color = theColor;
        gParticleArray[gLowestUnusedElement].horizSpeed = horizSpeed;
        gParticleArray[gLowestUnusedElement].vertSpeed = vertSpeed;
        gParticleArray[gLowestUnusedElement].horizLoc = horizLoc;
        gParticleArray[gLowestUnusedElement].vertLoc = vertLoc;
        gParticleArray[gLowestUnusedElement].oldHorizLoc = horizLoc;
        gParticleArray[gLowestUnusedElement].oldVertLoc = vertLoc;
                gParticleArray[gLowestUnusedElement].oldColor = getpixel(spriteWorldP->backFrameP->frameSurfaceP, horizLoc, vertLoc);
            // If the new particle is higher than gLastElementInUse, extend it.
        if (gLowestUnusedElement > gLastElementInUse)
            gLastElementInUse = gLowestUnusedElement;
    
        gNumberOfParticlesActive++;
        gLowestUnusedElement++;
    }
}

#if 0
#pragma mark -
#endif

//---------------------------------------------------------------------------------------
//  EraseParticlesOffscreen
//---------------------------------------------------------------------------------------

void    EraseParticlesOffscreen( SpriteWorldPtr spriteWorldP )
{
    Particle        *particlePtr;
        SDL_Surface     *screen = spriteWorldP->workFrameP->frameSurfaceP;
    FramePtr        workFrameP = spriteWorldP->workFrameP;
                    //backFrameP = spriteWorldP->backFrameP;
    int             visBoundsLeft, visBoundsRight, visBoundsTop, visBoundsBottom;
    int             horizLoc, vertLoc;
    long            index, count = gLastElementInUse;
    
    SW_ASSERT(spriteWorldP != NULL);
    //SW_ASSERT(workFrameP != NULL && workFrameP->isFrameLocked);
    //SW_ASSERT(workFrameP->frameDepth >= 8);
    SW_ASSERT(gParticlesInitialized == true);
    
        
        if (spriteWorldP->frameHasOccurred == true)
        {
                visBoundsLeft = workFrameP->frameRect.left;
                visBoundsRight = workFrameP->frameRect.right;
                visBoundsTop = workFrameP->frameRect.top;
                visBoundsBottom = workFrameP->frameRect.bottom;


        for (index = 0, particlePtr = gParticleArray; index <= count; index++, particlePtr++)
        {
            if (particlePtr->lifeRemaining>0)
            {
                horizLoc = particlePtr->oldHorizLoc;
                vertLoc = particlePtr->oldVertLoc;

                if ((horizLoc<visBoundsRight)&&(horizLoc>visBoundsLeft)&&
                    (vertLoc<visBoundsBottom)&&(vertLoc>visBoundsTop))
                {
                     BKPutPixel(screen, horizLoc, vertLoc, particlePtr->oldColor);
                                            // Update just the part of the display that we've changed 
                                        if ( workFrameP->isVideoSurface )
                                        SDL_UpdateRect(screen, horizLoc, vertLoc, 1, 1);
                }
            }
        }
        }
}


//---------------------------------------------------------------------------------------
//  EraseParticlesScrollingOffscreen
//---------------------------------------------------------------------------------------

void    EraseParticlesScrollingOffscreen( SpriteWorldPtr spriteWorldP )
{
    Particle    *particlePtr;
        SDL_Surface *screen = spriteWorldP->workFrameP->frameSurfaceP;
    FramePtr    workFrameP = spriteWorldP->workFrameP;
                //backFrameP = spriteWorldP->backFrameP;
    int         visBoundsLeft, visBoundsRight, visBoundsTop, visBoundsBottom;
    int         horizLoc, vertLoc;
    long        index, count = gLastElementInUse;

    SW_ASSERT(spriteWorldP != NULL);
    //SW_ASSERT(spriteWorldP->workFrameP != NULL && workFrameP->isFrameLocked);
    //SW_ASSERT(workFrameP->frameDepth >= 8);
    SW_ASSERT(gParticlesInitialized == true);
    
        
        if (spriteWorldP->frameHasOccurred == true)
        {
            visBoundsLeft = spriteWorldP->oldVisScrollRect.left;
            visBoundsRight = spriteWorldP->oldVisScrollRect.right;
            visBoundsTop = spriteWorldP->oldVisScrollRect.top;
            visBoundsBottom = spriteWorldP->oldVisScrollRect.bottom;
            
            for (index = 0, particlePtr = gParticleArray; index <= count; index++, particlePtr++)
            {
                if (particlePtr->lifeRemaining>0)
                {
                    horizLoc = particlePtr->oldHorizLoc;
                    vertLoc = particlePtr->oldVertLoc;
    
                    if ((horizLoc<visBoundsRight)&&(horizLoc>visBoundsLeft)&&
                            (vertLoc<visBoundsBottom)&&(vertLoc>visBoundsTop))
                    {
                                    // Make the particle's points local to the offscreen area
                                    // we have to offset to the *old* ScrollRectOffset - which we have to calculate
                            horizLoc -= spriteWorldP->backRect.right * (spriteWorldP->oldVisScrollRect.left / spriteWorldP->backRect.right);
                            vertLoc -= spriteWorldP->backRect.bottom * (spriteWorldP->oldVisScrollRect.top / spriteWorldP->backRect.bottom);
    
                                    // Wrap the pixel if it's hanging off one side of the offscreen area
                            if (horizLoc >=  workFrameP->frameRect.right)
                                    horizLoc -= workFrameP->frameRect.right;
                            else if (horizLoc < workFrameP->frameRect.left)
                                    horizLoc += workFrameP->frameRect.right;
    
                            if (vertLoc >= workFrameP->frameRect.bottom)
                                    vertLoc -= workFrameP->frameRect.bottom;
                            else if (vertLoc < workFrameP->frameRect.top)
                                    vertLoc += workFrameP->frameRect.bottom;
    
                            BKPutPixel(screen, horizLoc, vertLoc, particlePtr->oldColor);
                                // Update just the part of the display that we've changed 
                            if ( workFrameP->isVideoSurface )
                            SDL_UpdateRect(screen, horizLoc, vertLoc, 1, 1);
    
                    }
                }
            }
        }
    
}


//---------------------------------------------------------------------------------------
//  DrawParticlesOffscreen
//---------------------------------------------------------------------------------------

void    DrawParticlesOffscreen( SpriteWorldPtr spriteWorldP )
{
        Particle    *particlePtr;
        SDL_Surface *screen = spriteWorldP->workFrameP->frameSurfaceP;
        FramePtr    screenFrame = spriteWorldP->screenFrameP;
        int     visBoundsLeft, visBoundsRight, visBoundsTop, visBoundsBottom;
        int     horizLoc, vertLoc;
        long        index, count = gLastElementInUse;

    SW_ASSERT(spriteWorldP != NULL);
    //SW_ASSERT(workFrameP != NULL && workFrameP->isFrameLocked);
    //SW_ASSERT(workFrameP->frameDepth >= 8);
    SW_ASSERT(gParticlesInitialized == true);


        if (spriteWorldP->frameHasOccurred == true)
        {
            visBoundsLeft = screenFrame->frameRect.left;
            visBoundsRight = screenFrame->frameRect.right;
            visBoundsTop = screenFrame->frameRect.top;
            visBoundsBottom = screenFrame->frameRect.bottom;
            
            for (index = 0, particlePtr = gParticleArray; index <= count; index++, particlePtr++)
            {
                    if (particlePtr->lifeRemaining>1)
                    {
                            horizLoc = particlePtr->horizLoc;
                            vertLoc = particlePtr->vertLoc;
                            
                            if ((horizLoc<visBoundsRight)&&(horizLoc>visBoundsLeft)&&
                            (vertLoc<visBoundsBottom)&&(vertLoc>visBoundsTop))
                            {
                                    BKPutPixel(screen, horizLoc, vertLoc, particlePtr->color);
                                        
                                            // Update just the part of the display that we've changed 
                                    if ( screenFrame->isVideoSurface )
                                    SDL_UpdateRect(screen, horizLoc, vertLoc, 1, 1);
                            }
                     }
              }
        }
}


//---------------------------------------------------------------------------------------
//  DrawParticlesScrollingOffscreen
//---------------------------------------------------------------------------------------

void    DrawParticlesScrollingOffscreen( SpriteWorldPtr spriteWorldP )
{
    Particle    *particlePtr;
        SDL_Surface *screen = spriteWorldP->workFrameP->frameSurfaceP;
    FramePtr    workFrameP = spriteWorldP->workFrameP;
    int         visBoundsLeft, visBoundsRight, visBoundsTop, visBoundsBottom;
    int         horizLoc, vertLoc;
    long        index, count = gLastElementInUse;

    SW_ASSERT(spriteWorldP != NULL);
    //SW_ASSERT(workFrameP != NULL && workFrameP->isFrameLocked);
    //SW_ASSERT(workFrameP->frameDepth >= 8);
    SW_ASSERT(gParticlesInitialized == true);
        
        
        if (spriteWorldP->frameHasOccurred == true)
        {
                visBoundsLeft = spriteWorldP->visScrollRect.left;
                visBoundsRight = spriteWorldP->visScrollRect.right;
                visBoundsTop = spriteWorldP->visScrollRect.top;
                visBoundsBottom = spriteWorldP->visScrollRect.bottom;
    
        for (index = 0, particlePtr = gParticleArray; index <= count; index++, particlePtr++)
        {
            if (particlePtr->lifeRemaining>1)
            {
                horizLoc = particlePtr->horizLoc;
                vertLoc = particlePtr->vertLoc;

                if ((horizLoc<visBoundsRight)&&(horizLoc>visBoundsLeft)&&
                    (vertLoc<visBoundsBottom)&&(vertLoc>visBoundsTop))
                {
                        // Make the particle's points local to the offscreen area
                    horizLoc -= spriteWorldP->horizScrollRectOffset;
                    vertLoc -= spriteWorldP->vertScrollRectOffset;

                        // Wrap the pixel if it's hanging off one side of the offscreen area
                    if (horizLoc >=  workFrameP->frameRect.right)
                        horizLoc -= workFrameP->frameRect.right;
                    else if (horizLoc < workFrameP->frameRect.left)
                        horizLoc += workFrameP->frameRect.right;

                    if (vertLoc >= workFrameP->frameRect.bottom)
                        vertLoc -= workFrameP->frameRect.bottom;
                    else if (vertLoc < workFrameP->frameRect.top)
                        vertLoc += workFrameP->frameRect.bottom;
                
                    BKPutPixel(screen, horizLoc, vertLoc, particlePtr->color);
                                        
                                            // Update just the part of the display that we've changed 
                                        if ( workFrameP->isVideoSurface )
                                        SDL_UpdateRect(screen, horizLoc, vertLoc, 1, 1);

                }
            }
        }
        }
    
}

#if 0
#pragma mark -
#endif

//---------------------------------------------------------------------------------------
//  UpdateParticlesInWindow - moves particles, and also draws them to the window
//---------------------------------------------------------------------------------------

void    UpdateParticlesInWindow( SpriteWorldPtr spriteWorldP )
{
     Particle   *particlePtr;
         SDL_Surface    *screen = spriteWorldP->screenFrameP->frameSurfaceP;
     FramePtr   screenFrame = spriteWorldP->screenFrameP;
     int        visBoundsLeft, visBoundsRight, visBoundsTop, visBoundsBottom;
     int        horizLoc, vertLoc;
     long       index, count = gLastElementInUse;

    SW_ASSERT(spriteWorldP != NULL);
    //SW_ASSERT(windowFrameP->frameDepth >= 8);
    SW_ASSERT(gParticlesInitialized == true);

    if (spriteWorldP->frameHasOccurred == true)
        {
            visBoundsLeft = screenFrame->frameRect.left;
            visBoundsRight = screenFrame->frameRect.right;
            visBoundsTop = screenFrame->frameRect.top;
            visBoundsBottom = screenFrame->frameRect.bottom;
    
            //START_32_BIT_MODE
    
            //FOREACH_DEPTH_DO
            //(
                    //spriteWorldP->pixelDepth,
                    for (index = 0, particlePtr = gParticleArray; index <= count; index++, particlePtr++)
                    {
                                // Lock the screen for direct access to the pixels 
                            if ( SDL_MUSTLOCK(screen) ) {
                                if ( SDL_LockSurface(screen) < 0 ) {
                                    fprintf(stderr, "Can't lock screen: %s\n", SDL_GetError());
                                    return;
                                }
                            }

                            
                            if (particlePtr->lifeRemaining > 0)
                            {
                                    horizLoc = particlePtr->oldHorizLoc;
                                    vertLoc = particlePtr->oldVertLoc;
                                    
                                            // First erase the particle from its old position
                                    if ((horizLoc < visBoundsRight) && (horizLoc > visBoundsLeft) &&
                                            (vertLoc < visBoundsBottom) && (vertLoc > visBoundsTop))
                                    {
                                            
                                            BKPutPixel(screen, horizLoc, vertLoc, particlePtr->oldColor);
                                             
                                                   // Update just the part of the display that we've changed 
                                            if ( screenFrame->isVideoSurface )
                                            SDL_UpdateRect(screen, horizLoc, vertLoc, 1, 1);
                                    }
                                    
                                    horizLoc = particlePtr->horizLoc;
                                    vertLoc = particlePtr->vertLoc;
    
                                            // Now draw the particle in its new position
                                    if ((horizLoc < visBoundsRight) && (horizLoc > visBoundsLeft) &&
                                            (vertLoc < visBoundsBottom) && (vertLoc > visBoundsTop))
                                    {
                                           // particlePtr->oldColor = getpixel(screen, horizLoc, vertLoc);                                    
                                            BKPutPixel(screen, horizLoc, vertLoc, particlePtr->color);
                                        
                                                // Update just the part of the display that we've changed 
                                            if ( screenFrame->isVideoSurface )
                                            SDL_UpdateRect(screen, horizLoc, vertLoc, 1, 1);
                                    }
                            }
                                // Unlock the display
                             if ( SDL_MUSTLOCK(screen) )                                                                            SDL_UnlockSurface(screen);
                                            

                    }
            //)
            
            //END_32_BIT_MODE
            
                    // Move them *after* drawing them!
            MoveParticles();
        }
}


//---------------------------------------------------------------------------------------
//  UpdateParticlesInScrollingWindow - moves particles, but does not need to draw them,
//  since the entire offscreen is copied to the screen in a scrolling animation.
//---------------------------------------------------------------------------------------

void    UpdateParticlesInScrollingWindow( SpriteWorldPtr spriteWorldP )
{
    SW_UNUSED(spriteWorldP);
    SW_ASSERT(gParticlesInitialized == true);

    // we do not need to draw them here, since the entire offscreen is copied to the screen
    
    MoveParticles();
}


/*void DrawHardwareParticles( SpriteWorldPtr spriteWorldP )
{
#if SW_68K
    #pragma unused(spriteWorldP)
    // hardware is not available for 68K
#else
    Particle    *particlePtr;
    FramePtr    workFrameP = spriteWorldP->workFrameP;
    int         visBoundsLeft, visBoundsRight, visBoundsTop, visBoundsBottom;
    int         horizLoc, vertLoc;
    long        index, count = gLastElementInUse;

    RGBColor        rgb;

    SW_ASSERT(spriteWorldP != NULL);
    SW_ASSERT(workFrameP != NULL && workFrameP->isFrameLocked);
    SW_ASSERT(workFrameP->frameDepth >= 8);
    SW_ASSERT(gParticlesInitialized == true);

    visBoundsLeft = spriteWorldP->visScrollRect.left;
    visBoundsRight = spriteWorldP->visScrollRect.right;
    visBoundsTop = spriteWorldP->visScrollRect.top;
    visBoundsBottom = spriteWorldP->visScrollRect.bottom;
    
    for (index = 0, particlePtr = gParticleArray; index <= count; index++, particlePtr++)
    {
        if (particlePtr->lifeRemaining>1)
        {
            horizLoc = SW_FIX2INT(particlePtr->horizLoc);
            vertLoc = SW_FIX2INT(particlePtr->vertLoc);

            if ((horizLoc<visBoundsRight)&&(horizLoc>visBoundsLeft)&&
                (vertLoc<visBoundsBottom)&&(vertLoc>visBoundsTop))
            {
                    // Make the particle's points local to the offscreen area
                horizLoc -= visBoundsLeft;
                vertLoc -= visBoundsTop;

                SWGetColorFromValue(workFrameP,&rgb, particlePtr->color );
                
                SWHardwarePutPixel( horizLoc, vertLoc, &rgb );
            }
        }
    }
#endif

        // Move them *after* drawing them!
    MoveParticles();
}*/

//---------------------------------------------------------------------------------------
//  MoveParticles - called internally by UpdateParticlesInScrollingWindow and
//  UpdateParticlesInWindow.
//---------------------------------------------------------------------------------------

void    MoveParticles( void )
{
    Particle        *particlePtr;
    long            index,  count = gLastElementInUse;
    
    if (gParticlesInitialized == false)
        return;
    
    for (index = 0, particlePtr = gParticleArray; index <= count; index++, particlePtr++)
    {
        SW_ASSERT(index < gNumArrayElements);
        
        if (particlePtr->lifeRemaining > 0)
        {
            particlePtr->lifeRemaining--;
            
            if (particlePtr->lifeRemaining == 0)
            {
                gNumberOfParticlesActive--;
                
                    // If this dead particle opened up a space, update gLowestUnusedElement.
                if (index < gLowestUnusedElement)
                    gLowestUnusedElement = index;
            }
            else
            {
                particlePtr->oldHorizLoc = particlePtr->horizLoc;
                particlePtr->oldVertLoc = particlePtr->vertLoc;

                particlePtr->horizLoc += particlePtr->horizSpeed;
                particlePtr->vertLoc += particlePtr->vertSpeed;

                particlePtr->vertSpeed += gGravity;
                                particlePtr->horizSpeed += gHorizGravity;
            }
        }
    }
    
    if (gNumberOfParticlesActive <= 0)
    {
        gLastElementInUse = 0;
    }
    else
    {
            // See if the gLastElementInUse isn't in use any more.
            // If not, find the next element that still is.
        while (gParticleArray[gLastElementInUse].lifeRemaining == 0 && gLastElementInUse > 0)
            gLastElementInUse--;
    }
}

