//---------------------------------------------------------------------------------------
/// @file SWStats.h
//---------------------------------------------------------------------------------------

#ifndef __SWSTATS__
#define __SWSTATS__

#ifndef __SPRITEWORLD__
#include <SpriteWorld.h>
#endif


//---------------------------------------------------------------------------------------
// Definitions
//---------------------------------------------------------------------------------------


typedef enum
{
	kRightJustify = 0,
	kLeftJustify
} JustifyType;


	/// This structure is used for each stats box to tell the digit-drawing
	/// routines where to draw the numbers, and what number to draw.
typedef struct
{
	SpriteRec		srcSpriteP;		///< the Sprite
	long			theNum;			///< the current number (up to 10 digits long)
	short			numDigits;		///< how many digits can fit in the box
	JustifyType 		justification;	///< right or left justified
	int			fillWithZeros;	///< whether to display as 001 or just 1.
	DrawProcPtr		drawProc;		///< the DrawProc used to draw each digit
} StatsStruct, *StatsStructPtr;


//---------------------------------------------------------------------------------------
// Function Prototypes
//---------------------------------------------------------------------------------------

SpritePtr CreateStatsSpriteClone(
	SpritePtr masterSpriteP,
	short numDigits,
	JustifyType justification,
	int fillWithZeros);
	
SWError SetUpStatsSprite(
	SpritePtr statsSpriteP,
	SpriteLayerPtr dstSpriteLayer,
	DrawProcPtr drawProcP,
	short horizLoc,
	short vertLoc,
	long theNum);

/*SWError DrawPictInFrame(
	FramePtr dstFrameP, 
	short pictID);*/

void SetStatsSpriteDrawProc(
	SpritePtr srcSpriteP,
	DrawProcPtr drawProc);
	
void SetStatsSpriteNumber(
	SpritePtr srcSpriteP,
	long newNumber);

long GetStatsSpriteNumber(SpritePtr srcSpriteP);
	
void StatsItemDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	SWRect* srcRectP,
	SWRect* dstRectP);

short GetNumberLength(long theNum);


#endif /* __SWSTATS__ */


