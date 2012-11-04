///--------------------------------------------------------------------------------------
//	StaticText.h
//
//	By David Beck. Requires SWX and SDL_ttf.
//
//	Created: 07/15/03
///--------------------------------------------------------------------------------------


#ifndef __STATICTEXT__
#define __STATICTEXT__

#include <SWIncludes.h>

#define kMaxStringLength 512
#define kMaxPathLength 256

enum SWSTError
{
	kSTInitTTFError = 5500,
	kSTHasNotBeenInitedError,
	kSTStringTooLongError,
	kSTPathTooLongError,
	kSTCouldntOpenFontError,
	kSTSizeTextError,
	kSTTextRenderingError
};

typedef enum SWSTError SWSTError;

enum STAlign
{
	STAlign_Left,
	STAlign_Center,
	STAlign_Right
};

enum STRendering
{
	STRendering_Solid,	// no anti-aliasing, flat text color on transparent bkgnd
	STRendering_Smooth,	// anti-aliased text on solid background (back color)
	STRendering_Blended	// anti-aliased text alpha blended on transparent bkgnd
};

typedef struct
{
	SpriteRec sprite;
	
	char string[ kMaxStringLength ];
	
	char fontPath[ kMaxPathLength ];
	int fontSize;
	int fontStyle;
	int align;
	int wrap;
	SWRect bounds;
	
	int rendering;
	SDL_Color textColor;
	SDL_Color backColor;
	
} StaticTextRec, *StaticTextPtr;

	// must be called before creating any static texts
SWError st_Init( int shouldInitTTF );

	// call during cleanup
void st_Quit();

	// Newly created static texts have default settings which
	// can be changed using the following functions
void st_SetDefaultStyle( int bold, int italic, int underline );
void st_SetDefaultAlign( int align );
void st_SetDefaultRendering( int rendering );	// see STRendering enums
void st_SetDefaultTextColor( Uint8 r, Uint8 g, Uint8 b );
void st_SetDefaultBackColor( Uint8 r, Uint8 g, Uint8 b );

SWError st_CreateStaticText(
	const char* string,
	const char* fontPath,
	int fontSize,
	SWRect* bounds,
	int wrap,
	StaticTextPtr* newStaticTextPP );

	// st_DisposeStaticText static text is analagous to SWDisposeSprite,
	// The static text is not removed from its layer or from the animation.
void st_DisposeStaticText( StaticTextPtr* stPP );
void st_RemoveStaticTextFromAnimation( SpriteWorldPtr spriteWorldP, StaticTextPtr st, int disposeOfStaticText );

	// The following functions may be used to alter the text/appearance of
	// a static text after it has been created. Each function automatically re-renders
	// the text's frame so the effects will be visible onscreen after the next call
	// to SWUpdateSpriteWorld or SWAnimateSpriteWorld.
SWError st_SetText( StaticTextPtr st, const char* string );
SWError st_SetFont( StaticTextPtr st, const char* fontPath );
SWError st_SetFontSize( StaticTextPtr st, int size );
SWError st_SetStyle( StaticTextPtr st, int bold, int italic, int underline );
SWError st_SetAlign( StaticTextPtr st, int align );
SWError st_SetRendering( StaticTextPtr st, int rendering );
SWError st_SetTextColor( StaticTextPtr st, Uint8 r, Uint8 g, Uint8 b );
SWError st_SetBackColor( StaticTextPtr st, Uint8 r, Uint8 g, Uint8 b );

#endif