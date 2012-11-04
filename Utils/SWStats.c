//---------------------------------------------------------------------------------------
// SWStats.c - by Vern Jensen, June 1999
//---------------------------------------------------------------------------------------

#include <SWStats.h>
#include <SWIncludes.h>
//#include <SpriteWorldUtils.h>
//#include <SWGameUtils.h>	// Needed for CenterRect and ResourceExists.

//---------------------------------------------------------------------------------------
// CreateStatsSpriteClone
//---------------------------------------------------------------------------------------

SpritePtr CreateStatsSpriteClone(
	SpritePtr masterSpriteP,
	short numDigits,
	JustifyType justification,
	int fillWithZeros)
{
	StatsStructPtr	spriteStructP;
	SpritePtr		newSpriteP;
	SWError			err = kNoError;
	
	SW_ASSERT(masterSpriteP != NULL);
	SW_ASSERT(masterSpriteP->maxFrames >= 10);
	
		// Allocate memory for the StatsStruct
	spriteStructP = (StatsStructPtr)calloc(1,sizeof(StatsStruct));
	if (!spriteStructP)
	{
            err = kMemoryAllocationError;
        }
        
	if (err == kNoError)
	{
		err = SWCloneSprite(masterSpriteP, &newSpriteP, spriteStructP);
	}
	
	if (err == kNoError)
	{
			// Set the Sprite's DrawProc to our special digit-drawing DrawProc.
		newSpriteP->frameDrawProc = StatsItemDrawProc;
		
			// Adjust the Sprite's width to match the width of all the digits put together.
		newSpriteP->drawData->scaledWidth = SW_RECT_WIDTH(newSpriteP->destFrameRect) * numDigits;
		newSpriteP->drawData->scaledHeight = SW_RECT_HEIGHT(newSpriteP->destFrameRect);
		SWSetCurrentFrameIndex(newSpriteP, newSpriteP->curFrameIndex);
		
		spriteStructP->numDigits = numDigits;
		spriteStructP->theNum = -1;
		spriteStructP->justification = justification;
		spriteStructP->fillWithZeros = fillWithZeros;
		spriteStructP->drawProc = SWStdSpriteDrawProc;
	}
	else
	{
		newSpriteP = NULL;
	}
	
	return newSpriteP;
}


//---------------------------------------------------------------------------------------
// SetUpStatsSprite
//---------------------------------------------------------------------------------------

SWError SetUpStatsSprite(
	SpritePtr statsSpriteP,
	SpriteLayerPtr dstSpriteLayer,
	DrawProcPtr drawProcP,
	short horizLoc,
	short vertLoc,
	long theNum)
{
	SWError	err = kNoError;
	
	if (statsSpriteP == NULL)
		err = -1;
	
	if (err == kNoError)
	{
		SWSetSpriteLocation(statsSpriteP, horizLoc, vertLoc);
		SWAddSprite(dstSpriteLayer, statsSpriteP);
		SetStatsSpriteDrawProc(statsSpriteP, drawProcP);
		SetStatsSpriteNumber(statsSpriteP, theNum);
	}
	
	return err;
}

/*
//---------------------------------------------------------------------------------------
//  DrawPictInFrame - useful for stats areas that have a background pict
//---------------------------------------------------------------------------------------

SWError DrawPictInFrame(
	FramePtr dstFrameP, 
	short pictID)
{
	PicHandle 	myPictH;
	GWorldPtr	saveGWorld;
	GDHandle	saveGDH;
	Rect		pictRect;
	SWError		err = kNoError;
	
	SW_ASSERT(dstFrameP->isFrameLocked);

	myPictH = GetPicture(pictID);
	if (myPictH == NULL)
		err = ResourceExists('PICT', pictID) ? memFullErr : resNotFound;

	if (err == kNoError)
	{
		pictRect = (**myPictH).picFrame;
		GetGWorld( &saveGWorld, &saveGDH );
		SetGWorld(dstFrameP->framePort, nil);
		CenterRect(&pictRect, &dstFrameP->frameRect);
		
		DrawPicture(myPictH, &pictRect);
		ReleaseResource((Handle)myPictH);
		SetGWorld( saveGWorld, saveGDH );
	}

	return err;
}
*/

//---------------------------------------------------------------------------------------
//	SetStatsSpriteDrawProc
//---------------------------------------------------------------------------------------

void SetStatsSpriteDrawProc(
	SpritePtr srcSpriteP,
	DrawProcPtr drawProc)
{
	StatsStructPtr	statsStructP = (StatsStructPtr)srcSpriteP;
	
	statsStructP->drawProc = drawProc;
}


//---------------------------------------------------------------------------------------
//	SetStatsSpriteNumber
//---------------------------------------------------------------------------------------

void SetStatsSpriteNumber(
	SpritePtr srcSpriteP,
	long newNumber)
{
	StatsStructPtr	statsStructP = (StatsStructPtr)srcSpriteP;
	
	if (statsStructP->theNum != newNumber)
	{
		statsStructP->theNum = newNumber;
		srcSpriteP->needsToBeDrawn = true;
	}
}


//---------------------------------------------------------------------------------------
//	GetStatsSpriteNumber
//---------------------------------------------------------------------------------------

long GetStatsSpriteNumber(SpritePtr srcSpriteP)
{
	return ((StatsStructPtr)srcSpriteP)->theNum;
}


//---------------------------------------------------------------------------------------
//	StatsItemDrawProc
//---------------------------------------------------------------------------------------

void StatsItemDrawProc(
	FramePtr srcFrameP,
	FramePtr dstFrameP,
	SWRect* srcRectP,
	SWRect* dstRectP)
{
	short			n, theDigit, width, index, fillValue;
	long			theNum;
	char			digitArray[10];			// Enough digits for a long
	StatsStructPtr		statsStructP;
	FramePtr		*srcFrameArray;
	SWRect			destRect, srcRect, frameRect, clippedDstRect;
                
	SW_ASSERT(gSWCurrentElementDrawData != NULL);
	
	statsStructP = (StatsStructPtr)gSWCurrentElementDrawData->parentSpriteP;
	srcFrameArray = gSWCurrentElementDrawData->parentSpriteP->frameArray;
	theNum = statsStructP->theNum;

	if (statsStructP->fillWithZeros)
		fillValue = 0;
	else
		fillValue = -1;

		// Fill the unused part of the display with either nothing or 0s.
	for (n=0; n < statsStructP->numDigits; n++)
		digitArray[n] = fillValue;
	
		// Put the number into the digitArray
	if (theNum >= 0)
	{
		if (statsStructP->justification == kLeftJustify)
			index = GetNumberLength(theNum)-1;
		else
			index = statsStructP->numDigits-1;

		do
		{		// Fill digitArray with the digits from the new number
			digitArray[index] = theNum%10;		// Get the right-most digit
			index--;
			theNum /= 10;						// Remove the right-most digit
		} while (theNum > 0 && index >= 0);
	}
	
	destRect = *dstRectP;
	frameRect = srcFrameArray[0]->frameRect;
	width = SW_RECT_WIDTH(frameRect);
	
		// Expand destRect's left and top to its original size before it was clipped
	if (srcRectP->top > frameRect.top)
		destRect.top += frameRect.top - srcRectP->top;
	if (srcRectP->left > frameRect.left)
		destRect.left += frameRect.left - srcRectP->left;
	
		// The destRect's width and height should be the size of one digit.
	destRect.bottom = destRect.top + SW_RECT_HEIGHT(frameRect);
	destRect.right = destRect.left + width;

	
		// Draw the number
	for (index = 0; index < statsStructP->numDigits; index++)
	{
		theDigit = digitArray[index];
		
		if (theDigit >= 0)
		{
			srcFrameP = srcFrameArray[theDigit];
			srcRect = srcFrameP->frameRect;
			
				// Clip each digit with the dstRect passed to this drawProc.
			clippedDstRect = destRect;
			SW_CLIP_DST_AND_SRC_RECT(clippedDstRect, srcRect, (*dstRectP));

				// Set the sprite's location (used by hardware drawprocs)
			gSWCurrentElementDrawData->horizLoc = clippedDstRect.left;
			gSWCurrentElementDrawData->vertLoc = clippedDstRect.top;

			if (gSWCurrentSpriteWorld != NULL)
			{
				gSWCurrentElementDrawData->horizLoc += gSWCurrentSpriteWorld->visScrollRect.left;
				gSWCurrentElementDrawData->vertLoc += gSWCurrentSpriteWorld->visScrollRect.top;
			}
			
				// Draw the digit
			(*statsStructP->drawProc)(srcFrameP, dstFrameP, &srcRect, &clippedDstRect);
		}
		
		destRect.left += width;
		destRect.right += width;
	}
}


//---------------------------------------------------------------------------------------
//	GetNumberLength - a pretty fast routine for getting the number of digits in a long.
//	Note: if the number is negative, this doesn't count the minus symbol as a digit.
//---------------------------------------------------------------------------------------

short GetNumberLength(long theNum)
{
	short	length;
	
	if (theNum < 0)
		theNum = -theNum;
	
	if (theNum < 10)
		length = 1;
	else if (theNum < 100)
		length = 2;
	else if (theNum < 1000)
		length = 3;
	else if (theNum < 10000)
		length = 4;
	else if (theNum < 100000)
		length = 5;
	else if (theNum < 1000000)
		length = 6;
	else if (theNum < 10000000)
		length = 7;
	else if (theNum < 100000000)
		length = 8;
	else if (theNum < 1000000000)
		length = 9;
	else
		length = 10;
	
	return length;
}


