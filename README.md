
SpriteWorld X (cross-platform)


SpriteWorld is a free game SDK for Macintosh written in C.
This is a portable cross-platform version of SpriteWorld.


Platforms:

-   Classic (Mac OS 9)
    
-   Carbon (Mac OS X)
    
-   Win32 (Windows 9x/ME/NT/XP)
    
-   Posix (Linux/FreeBSD/Unix)
    
        Any platform with SDL is "possible",
        but only the above have been tested.


Needed Requirements:

-   ANSI C, and the standard C library
    (stdlib, stdio)

-   SDL (the Simple Directmedia Layer)
    http://www.libsdl.org/


Optional Requirements:

-   OpenGL (graphics hardware API)
    http://www.opengl.org/

-   OpenAL (audio hardware API)
    http://www.openal.org/


-   SDL_image (for additional image formats)

        without this, only BMP images

-   SDL_sound (for additional sound formats)

        without this, only WAV sounds

-   zziplib (reading ZIP resource archives)
    zlib (inflate/deflate gzip compression)

        without this, only files / dirs

-   SDL_ttf (drawing text strings to surface)
    freetype ("TrueType" font format)

        without this, only bitmap fonts

-   SDL_mixer (mixing several sound channels)

        without this, only one channel


Changes from SpriteWorld 3.0 for Macintosh:

*   QuickDraw has now been replaced with SDL,
    and Macintosh toolbox with std libraries.

*   The "BlitPixie" and "Hardwarie" libraries
    have now been replaced with "BlitKernel".

*   The SpriteWorldUtils library is now always
    included in the main SpriteWorldX library.

*   HeaderDoc (inline header documentation) has been
    replaced with the Doxygen documentation instead.

*   The C++ "Classes" (wrappers) are now always
    included with the regular (.h) header files.

*   "SWHeaders" (pre-compiled headers) are now
    handled by each compiler, and not included.

*   Resources are replaced with regular files,
    and the resource files with zip archives.


