//---------------------------------------------------------------------------------------
/// @file SWProperties.h
/// Constants, structures, and function prototypes for property files
//---------------------------------------------------------------------------------------

#ifndef __SWPROPERTIES__
#define __SWPROPERTIES__

#ifndef __SWCOMMON__
#include <SWCommonHeaders.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

/*
    SW Property files are similar to Windows .INI files and UNIX .conf files,
    they are ASCII text files with optional comments and delimiting Sections.
    
    They look like this:
    # Comment
    [Section]
    foo = 42 ; another comment
    baz = true

    Example:
    int foo = SWGetPropertyInteger( p, "Section/foo", 42 );
    
    You can also save property values out to files, in a very similar manner.
    (the comments are not saved/preserved from loading, just the actual values)

    Example:
    SWSetPropertyString( p, "Section/key", "value" );
    
    Will save like this:
    [Section]
    key = value

*/

typedef struct SWProperties     *SWPropertiesPtr;

SWError SWCreateProperties(SWPropertiesPtr *newProperties);
void SWDisposeProperties(SWPropertiesPtr *oldProperties);

SWError SWLoadProperties(SWPropertiesPtr properties, char *filename);
SWError SWLoadProperties_RW(SWPropertiesPtr properties, SDL_RWops *ops);

SWError SWSaveProperties(SWPropertiesPtr properties, char *filename);
SWError SWSaveProperties_RW(SWPropertiesPtr properties, SDL_RWops *ops);

int SWGetPropertyInteger(SWPropertiesPtr properties, char *name, int defaultValue);
float SWGetPropertyFloat(SWPropertiesPtr properties, char *name, float defaultValue);
char *SWGetPropertyString(SWPropertiesPtr properties, char *name, char *defaultValue);
SWBoolean SWGetPropertyBoolean(SWPropertiesPtr properties, char *name, SWBoolean defaultValue);

SWError SWSetPropertyInteger(SWPropertiesPtr properties, char *name, int value);
SWError SWSetPropertyFloat(SWPropertiesPtr properties, char *name, float value);
SWError SWSetPropertyString(SWPropertiesPtr properties, char *name, char *value);
SWError SWSetPropertyBoolean(SWPropertiesPtr properties, char *name, SWBoolean value);

#ifdef __cplusplus
}
#endif

#endif // __SWPROPERTIES__

