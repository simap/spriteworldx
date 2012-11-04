///--------------------------------------------------------------------------------------
//	StaticText.c
//
//	By David Beck. Requires SWX and SDL_ttf.
//
//	Created: 07/15/03
///--------------------------------------------------------------------------------------

#include <SDL.h>
#include <SDL_ttf.h>
#include <SWIncludes.h>

#include "StaticText.h"

///--------------------------------------------------------------------------------------
//	Private Function Prototypes
///--------------------------------------------------------------------------------------

SWError renderStaticText( StaticTextPtr st );
SWError wrapString( char* string, TTF_Font *font, int maxWidth );
SWError blitStringToFrame(
	char *string,
	TTF_Font *font,
	int align,
	int rendering,
	SDL_Color *textColor,
	SDL_Color *backColor,
	FramePtr frameP );
void alphaBlendTextSurfaceToFrame( SDL_Surface *textSurface, SDL_Surface *frameSurface, SDL_Rect *destRect );
void handleStaticText( SpritePtr stSP );

///--------------------------------------------------------------------------------------
//	Private Variables
///--------------------------------------------------------------------------------------

static int gSTHasBeenInitialized = false;
static int gInitializedTTF = 0;

static int gDefaultStyle = TTF_STYLE_NORMAL;
static int gDefaultAlign = STAlign_Left;
static int gDefaultRendering = STRendering_Blended;
static SDL_Color gDefaultTextColor = { 0x00, 0x00, 0x00, 0xFF };
static SDL_Color gDefaultBackColor = { 0xFF, 0xFF, 0xFF, 0xFF };

///--------------------------------------------------------------------------------------
//	Code
///--------------------------------------------------------------------------------------

SWError st_Init( int shouldInitTTF )
{
	SWError err = kNoError;
	
	if( ! gSTHasBeenInitialized )
	{
		if( shouldInitTTF )
		{
			if( TTF_Init() < 0 )
				err = kSTInitTTFError;
		}
		
		if( err == kNoError )
		{
			gInitializedTTF = shouldInitTTF;
			gSTHasBeenInitialized = true;
		}
	}
	
	return err;
}

void st_Quit()
{
	if( gInitializedTTF )
		TTF_Quit();
}

#if 0
#pragma mark -
#endif

void st_SetDefaultStyle( int bold, int italic, int underline )
{
	int renderStyle = TTF_STYLE_NORMAL;
	
	if ( bold )
		renderStyle |= TTF_STYLE_BOLD;
	
	if ( italic )
		renderStyle |= TTF_STYLE_ITALIC;
	
	if ( underline )
		renderStyle |= TTF_STYLE_UNDERLINE;
	
	gDefaultStyle = renderStyle;
}

void st_SetDefaultAlign( int align )
{
	gDefaultAlign = align;
}

void st_SetDefaultRendering( int rendering )
{
	gDefaultRendering = rendering;
}

void st_SetDefaultTextColor( Uint8 r, Uint8 g, Uint8 b )
{
	gDefaultTextColor.r = r;
	gDefaultTextColor.g = g;
	gDefaultTextColor.b = b;
}

void st_SetDefaultBackColor( Uint8 r, Uint8 g, Uint8 b )
{
	gDefaultBackColor.r = r;
	gDefaultBackColor.g = g;
	gDefaultBackColor.b = b;
}

#if 0
#pragma mark -
#endif

SWError st_CreateStaticText(
	const char* string,
	const char* fontPath,
	int fontSize,
	SWRect* bounds,
	int wrap,
	StaticTextPtr* newStaticTextPP )
{
	SWError err = kNoError;
	FramePtr frameP = NULL;
	StaticTextPtr tempStaticTextP = NULL;
	
	*newStaticTextPP = 0;
	
	if( ! gSTHasBeenInitialized )
		err = kSTHasNotBeenInitedError;
	
	if( err == kNoError )
		if( strlen( string ) > kMaxStringLength - 1 )
			err = kSTStringTooLongError;

	if( err == kNoError )
		if( strlen( fontPath ) > kMaxPathLength - 1 )
			err = kSTPathTooLongError;
	
	if( err == kNoError )
	{
		tempStaticTextP = malloc( sizeof( StaticTextRec ) );
		if( ! tempStaticTextP ) err = kMemoryAllocationError;
	}
	
	if( err == kNoError )
		err = SWCreateSprite( (SpritePtr*)&tempStaticTextP, tempStaticTextP, 1 );		

	if( err == kNoError )
	{
		SWSetSpriteMoveProc( (SpritePtr)tempStaticTextP, handleStaticText );
		
			// make the frame to which we will be copying each line of text
		err = SWCreateBlankFrame(
			&frameP,
			SW_RECT_WIDTH( *bounds ),
			SW_RECT_HEIGHT( *bounds ),
			SDL_GetVideoInfo()->vfmt->BitsPerPixel,
			true );
	}
				
	if( err == kNoError )
	{
		SWSetSpriteLocation( (SpritePtr)tempStaticTextP, bounds->left, bounds->top );
		err = SWAddFrame( (SpritePtr)tempStaticTextP, frameP );
	}
	
	if( err == kNoError )
	{
		strcpy( tempStaticTextP->string, string );
		strcpy( tempStaticTextP->fontPath, fontPath );
		tempStaticTextP->fontSize = fontSize;
		tempStaticTextP->fontStyle = gDefaultStyle;
		tempStaticTextP->wrap = wrap;
		tempStaticTextP->bounds = *bounds;
		tempStaticTextP->align = gDefaultAlign;
		tempStaticTextP->rendering = gDefaultRendering;
		tempStaticTextP->textColor = gDefaultTextColor;
		tempStaticTextP->backColor = gDefaultBackColor;
		
		err = SWSetCurrentFrameIndex( (SpritePtr)tempStaticTextP, 0 );
	}
		
	if( err == kNoError )
		err = renderStaticText( tempStaticTextP );

	if( err == kNoError )
	{
		SWLockSprite( (SpritePtr)tempStaticTextP );
		*newStaticTextPP = tempStaticTextP;
	}
	
	if( err != kNoError )
	{
			// clean up what we can
		if( tempStaticTextP ) st_DisposeStaticText( &tempStaticTextP );
		if( frameP ) SWDisposeFrame( &frameP );
	}
	
	SWSetStickyIfError( err );
	return err;
}

void st_DisposeStaticText( StaticTextPtr* stPP )
{
	SWDisposeSprite( (SpritePtr*)stPP ); 
}

void st_RemoveStaticTextFromAnimation( SpriteWorldPtr spriteWorldP, StaticTextPtr st, int disposeOfStaticText )
{
	SWRemoveSpriteFromAnimation( spriteWorldP, (SpritePtr)st, disposeOfStaticText );
}

#if 0
#pragma mark -
#endif

SWError st_SetText( StaticTextPtr st, const char* string )
{
	SWError err = kNoError;
	
	if( strlen( string ) > kMaxStringLength - 1 )
		err = kSTStringTooLongError;
	
	if( err == kNoError )
		strcpy( st->string, string );
	
	err = renderStaticText( st );
	
	SWSetStickyIfError( err );
	return err;
}

SWError st_SetFont( StaticTextPtr st, const char* fontPath )
{
	SWError err = kNoError;
	
	if( strlen( fontPath ) > kMaxPathLength - 1 )
		err = kSTPathTooLongError;
	
	if( err == kNoError )
		strcpy( st->fontPath, fontPath );
	
	err = renderStaticText( st );
	
	SWSetStickyIfError( err );
	return err;
}

SWError st_SetFontSize( StaticTextPtr st, int size )
{
	SWError err = kNoError;

	st->fontSize = size;
	err = renderStaticText( st );
		
	SWSetStickyIfError( err );
	return err;
}

SWError st_SetStyle( StaticTextPtr st, int bold, int italic, int underline )
{
	SWError err = kNoError;
	int renderStyle = TTF_STYLE_NORMAL;
	
	if ( bold )
		renderStyle |= TTF_STYLE_BOLD;
	
	if ( italic )
		renderStyle |= TTF_STYLE_ITALIC;
	
	if ( underline )
		renderStyle |= TTF_STYLE_UNDERLINE;
	
	st->fontStyle = renderStyle;

	err = renderStaticText( st );
	
	SWSetStickyIfError( err );
	return err;	
}

SWError st_SetAlign( StaticTextPtr st, int align )
{
	SWError err = kNoError;
	
	st->align = align;
	
	err = renderStaticText( st );
	
	SWSetStickyIfError( err );
	return err;		
}

SWError st_SetRendering( StaticTextPtr st, int rendering )
{
	SWError err = kNoError;

	st->rendering = rendering;
	err = renderStaticText( st );
	
	SWSetStickyIfError( err );
	return err;
}

SWError st_SetTextColor( StaticTextPtr st, Uint8 r, Uint8 g, Uint8 b )
{
	SWError err = kNoError;
	
	st->textColor.r = r;
	st->textColor.g = g;
	st->textColor.b = b;
	err = renderStaticText( st );
	
	SWSetStickyIfError( err );
	return err;
}

SWError st_SetBackColor( StaticTextPtr st, Uint8 r, Uint8 g, Uint8 b )
{
	SWError err = kNoError;
	
	st->backColor.r = r;
	st->backColor.g = g;
	st->backColor.b = b;
	err = renderStaticText( st );
	
	SWSetStickyIfError( err );
	return err;
}

#if 0
#pragma mark -
#endif

SWError renderStaticText( StaticTextPtr st )
{
	SWError err = kNoError;
	TTF_Font *font = NULL;
	FramePtr frameP;
	char wrappedString[ kMaxStringLength ];

	if( err == kNoError )
	{
			// load the font we will be using
		font = TTF_OpenFont( st->fontPath, st->fontSize );
		if( ! font ) err = kSTCouldntOpenFontError;
	}

	if( err == kNoError )
	{
		TTF_SetFontStyle( font, st->fontStyle );
		
		strcpy( wrappedString, st->string );
		
		if( st->wrap )
			err = wrapString( wrappedString, font, st->bounds.right - st->bounds.left );
	}

	if( err == kNoError )
	{
		frameP = st->sprite.curFrameP;
				
		err = blitStringToFrame(
			wrappedString,
			font,
			st->align,
			st->rendering,
			&st->textColor,
			&st->backColor,
			frameP );
		
		st->sprite.needsToBeDrawn = true;
	}

	if( font != NULL ) TTF_CloseFont( font );
	
	return err;
}

SWError wrapString( char* string, TTF_Font *font, int maxWidth )
{
	SWError err = kNoError;
	char *lineStart, *nextWord;
	char *lastPotentialBreak, *wordEnd;
	char oldWordBreakChar;
	int sizeX, sizeY;
	
	lineStart = nextWord = string;
	lastPotentialBreak = NULL;
	
		// step through the string word by word, checking each word
		// to see if the current line is too long. If so, turn
		// the last space into a new line char, and start back up from there.
	while( nextWord != NULL && *nextWord != '\0' )
	{
		
			// find next word break - either space or new line
		wordEnd = strpbrk( nextWord, " \n" );
		
		if( wordEnd != NULL )
		{
				// if we are not already at the end of the string, we must
				// set the char at wordEnd to null to before we can call
				// TTF_SizeText with our little substring, which runs from
				// the start of the current line to wordEnd
			oldWordBreakChar = *wordEnd;
			*wordEnd = '\0';
		}
		
		if( TTF_SizeText( font, lineStart, &sizeX, &sizeY ) < 0 )
			err = kSTSizeTextError;
		
		if( err != kNoError )
			break;
		
			// set our char at wordEnd back to normal,
			// since now we know the width of our substr.
		if( wordEnd != NULL )
			*wordEnd = oldWordBreakChar;
		
			// if we went over with that last word, we need to wrap
		if( sizeX > maxWidth )
		{
			if( lastPotentialBreak == NULL )
			{
					// can't wrap before this word, since it was the first on the line.
					// We just have to put a return after the word and move on, eventhough
					// the end of it might stick out of the bounding rect. But don't put the
					// return if we are at the very end of the string
				if( wordEnd != NULL )
				{
					*wordEnd = '\n';
					lineStart = nextWord = wordEnd + 1;
					lastPotentialBreak = NULL;
				}
			}
			else
			{
				*lastPotentialBreak = '\n';
				lineStart = nextWord = lastPotentialBreak + 1;
				lastPotentialBreak = NULL;
			}
		}
		else
		{
				// the line is still in bounds, so move on to the next word
			if( wordEnd != NULL )
			{
				nextWord = wordEnd + 1;
				
				if( *wordEnd == ' ' )
					lastPotentialBreak = wordEnd;
				else if( *wordEnd == '\n' )
				{
					lineStart = wordEnd;
					lastPotentialBreak = NULL;
				}
			}
			else
				nextWord = NULL;
		}
	}
	
	return err;
}

SWError blitStringToFrame(
	char *string,
	TTF_Font *font,
	int align,
	int rendering,
	SDL_Color *textColor,
	SDL_Color *backColor,
	FramePtr frameP )
{
	SWError err = kNoError;
	char *lineStart, *lineEnd;
	SDL_Surface *textSurface = NULL, *convertedTextSurface = NULL;
	int lineSkipHeight = TTF_FontLineSkip( font );
	SDL_Rect destRect;
	int lineWidth;
	
		// draw the background. Transparent for solid or blended,
		// solid bkgnd color for smooth.
	if( rendering == STRendering_Smooth )
		SDL_FillRect( frameP->frameSurfaceP, NULL,
			SDL_MapRGBA( frameP->frameSurfaceP->format, backColor->r, backColor->g, backColor->b, 0xFF ) );
	else
		SDL_FillRect( frameP->frameSurfaceP, NULL,
			SDL_MapRGBA( frameP->frameSurfaceP->format, backColor->r, backColor->g, backColor->b, 0x00 ) );
	
	destRect.x = frameP->frameRect.left;
	destRect.y = frameP->frameRect.top;
	destRect.w = SW_RECT_WIDTH( frameP->frameRect );
	
	lineStart = string;
	
		// render each line, one at a time, and blit them over to our frame
		// one after another. We have to start at the last line and work
		// our way to the first, since some characters hang over to the
		// line below, and we don't want those bits cut off.
	while(
		lineStart != NULL &&
		*lineStart != '\0' &&
		destRect.y < frameP->frameRect.bottom &&
		err == kNoError )
	{
		lineEnd = strchr( lineStart, '\n' );
		if( lineEnd != NULL )
		{
				// we need to insert a null char temporarily at the end of
				// this line, so that we can render just this text.
			*lineEnd = '\0';
		}
		
		TTF_SizeText( font, lineStart, &lineWidth, NULL );
		
		switch( rendering )
		{
			case STRendering_Solid:
				textSurface = TTF_RenderText_Solid( font, lineStart, *textColor );
				break;
			case STRendering_Smooth:
				textSurface = TTF_RenderText_Shaded( font, lineStart, *textColor, *backColor );
				break;
			case STRendering_Blended:
				textSurface = TTF_RenderText_Blended( font, lineStart, *textColor );
				break;
		}
				
		if( textSurface == NULL )
			err = kSTTextRenderingError;
			
		if( err == kNoError )
		{
				// put things back the way we found them, now that
				// the text has been rendered
			if( lineEnd != NULL )
				*lineEnd = '\n';
	
				// convert the text surface to the display format, so we can
				// blit it safely onto frameSurfaceP
			convertedTextSurface = SDL_ConvertSurface(
				textSurface,
				frameP->frameSurfaceP->format,
				SDL_SWSURFACE );
			
			if( convertedTextSurface == NULL )
				err = kSTTextRenderingError;
		}
		
		if( err == kNoError )
		{
			switch( align )
			{
				case STAlign_Left:
					destRect.x = 0;
					break;
				case STAlign_Center:
					destRect.x = (frameP->frameRect.right / 2) - (lineWidth / 2);
					break;
				case STAlign_Right:
					destRect.x = frameP->frameRect.right - lineWidth;
					break;
			}
					
				// if we don't unset the SRCALPHA flag, then the dest
				// surface alpha will remain untouched, and our text
				// wont show up using a std blit. This is the way SDL
				// is designed, although it is aknowledged to be
				// counterintuitive in the docs.
			if( SDL_SetAlpha( convertedTextSurface, 0, 0xFF ) < 0 )
				err = kSTTextRenderingError;
			
			if( SDL_SetColorKey( convertedTextSurface, SDL_SRCCOLORKEY, 
				SDL_MapRGBA( convertedTextSurface->format, 0xFF, 0xFF, 0xFF, 0x00 ) ) < 0 )
				err = kSTTextRenderingError;
		}
		
			// we use our own function for alpha bending since for
			// a RGBA to RGBA blit, SDL either leaves the dest alpha
			// untouched (if SRCALPHA is set), or replaces it completely
			// with the source alpha (if SRCALPHA is not set). In the first
			// option the text doesnt show up at all, and in the second hanging
			// characters such as 'g' and 'j' have their bottoms cut off
			// when the next line is blit. So we do our own blit pixel-by-pixel,
			// 'alpha-bending' (taking the max of src/dest alphas) as we go.
			// FWIK, this is rediculously slow on HW surfaces, but until
			// somebody comesup with a better solution...
		if( err == kNoError )
		{
			if( rendering == STRendering_Blended || rendering == STRendering_Solid )
				alphaBlendTextSurfaceToFrame( convertedTextSurface, frameP->frameSurfaceP, &destRect );
			else
			{
				if( err == kNoError )
					if( SDL_BlitSurface( convertedTextSurface, NULL, frameP->frameSurfaceP, &destRect ) < 0 )
						err = kSTTextRenderingError;
			}
		}
		
		if( err == kNoError )
		{	
			if( lineEnd == NULL )
				lineStart = NULL;
			else
				lineStart = lineEnd + 1;
			
			destRect.y += lineSkipHeight;
			
		}

		if( textSurface ) SDL_FreeSurface( textSurface );
		if( convertedTextSurface ) SDL_FreeSurface( convertedTextSurface );
	}
	
	return err;
}

void alphaBlendTextSurfaceToFrame( SDL_Surface *textSurface, SDL_Surface *frameSurface, SDL_Rect *destRect )
{
	int row, col, maxRow, maxCol;
	Uint32 *src;
	Uint32 *dst;
	
	// assumes both surfaces are same format.
	
	SDL_LockSurface( textSurface );
	SDL_LockSurface( frameSurface );
	
	maxRow = frameSurface->h - destRect->y - 1;
	maxCol = frameSurface->w - destRect->x - 1;
	
	for( row=0; row<textSurface->h; row++ )
	{
		if( row > maxRow ) break;
		
		src = &((Uint32*)textSurface->pixels)[ row * textSurface->w ];
		dst = &((Uint32*)frameSurface->pixels)[ ( destRect->y + row ) * frameSurface->w + destRect->x ];
		
		for( col=0; col<textSurface->w; col++ )
		{
			if( col > maxCol ) break;
			
			if( (Uint32)(*src & textSurface->format->Amask) > (Uint32)(*dst & frameSurface->format->Amask) )
				*dst = *src;
			
			src++;
			dst++;
		}
	}
	
	SDL_UnlockSurface( textSurface );
	SDL_UnlockSurface( frameSurface );
}

#if 0
#pragma mark -
#endif

void handleStaticText( SpritePtr stSP )
{
	StaticTextPtr st = (StaticTextPtr)stSP;
	
	// if we are in blended mode, than we need to update our
	// entire selves every frame to avoid drawing on top of
	
	if( st->rendering == STRendering_Blended )
		st->sprite.needsToBeDrawn = true;
}
