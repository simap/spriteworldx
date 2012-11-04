
#ifndef __SWCOMMON__
#include <SWCommonHeaders.h>
#endif

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#ifndef __SWPROPERTIES__
#include "SWProperties.h"
#endif 

struct SWProperty
{
    // for code simplicity, this is just a linked list
    struct SWProperty   *next;

    char *key;      // all key/value strings are dup'ed
    char *value;    // all values are saved as strings
};

struct SWProperties
{
    int                 numProperties;
    struct SWProperty   *head;
};

    // this function (strdup) is missing from old stdlibs
static char *local_strdup(const char *str);

#if macintosh
#define strtol(str,end,base)    atoi(str)
#define strtof(str,end)         atof(str)
#endif

    // similar to fgets, but for all types of line endings
static char *SDL_RWgets(SDL_RWops *ctx, char *str, int size);

//---------------------------------------------------------------------------------------
//  CreateProperties
//---------------------------------------------------------------------------------------

SWError SWCreateProperties(SWPropertiesPtr *newProperties)
{
    SWPropertiesPtr properties;
    properties = (SWPropertiesPtr) calloc(1, sizeof(struct SWProperties));
    *newProperties = properties;
    return (properties == NULL) ? kMemoryAllocationError : kNoError;
}

//---------------------------------------------------------------------------------------
//  DisposeProperties
//---------------------------------------------------------------------------------------

void SWDisposeProperties(SWPropertiesPtr *oldProperties)
{
    SWPropertiesPtr properties;
    struct SWProperty *p, *next;

    properties = *oldProperties;
    if (properties != NULL)
    {
        for (p = properties->head; p != NULL; p = next)
        {
            free(p->key);
            free(p->value);

            next = p->next;
            free(p);
        }
        free(properties);
    }
    *oldProperties = NULL;
}

//---------------------------------------------------------------------------------------
//  LoadProperties
//---------------------------------------------------------------------------------------

SWError SWLoadProperties(SWPropertiesPtr properties, char *filename)
{
   return SWLoadProperties_RW( properties, SDL_RWFromFile(filename, "r") );
}

//---------------------------------------------------------------------------------------
//  SWLoadProperties_RW
//---------------------------------------------------------------------------------------

SWError SWLoadProperties_RW(SWPropertiesPtr properties, SDL_RWops *ops)
{
    char *key, *value, *semi, *s, *p;
    char *section = NULL;
    char line[80];
    SWError err = kNoError;
    
    if (properties != NULL && ops != NULL)
    {
         while(1)
        {
            key = SDL_RWgets(ops, line, sizeof(line));
            if (key == NULL)
                break;
            
            if (key[0] == '#' || line[0] == '/')
                continue;   // comments
  
            semi = strchr(key, ';');
            if (semi != NULL)
                *semi = '\0';   // comment
                
            if (key[0] == '[')  // sections
            {
                key++;
                s = line + strlen(line) - 1;
                while (isspace(*s) && s > key)
                    *s-- = '\0';
                if (*s == ']')
                {
                    *s-- = '\0';
                        // trim whitespace (section)
                    while (isspace(*key) && *key)
                        key++;
                    while (isspace(*s) && s > key)
                        *s-- = '\0';
                
                    if (section != NULL)
                        free(section);
                    section = local_strdup(key);
                    //fprintf(stderr, "SECTION: \"%s\"\n", section);
                    continue;
                }
            }

            value = strchr(line, '=');
            if (value != NULL)
            {
                *value++ = '\0';
            
                    // trim whitespace (key)
                while (isspace(*key) && *key)
                    key++;
                s = value - 2;
                while (isspace(*s) && s > key)
                    *s-- = '\0';
                if (s < key)
                    continue; // empty
                
                    // trim whitespace (value)
                while (isspace(*value) && *value)
                    value++;
                s = value + strlen(value) - 1;
                while (isspace(*s) && s > value)
                    *s-- = '\0';
                if (s < value)
                    continue;
        
                if (section == NULL)
                    err = SWSetPropertyString(properties, key, value);
                else
                {
                    p = malloc(strlen(section) + 1 + strlen(key) + 1);
                    if (p != NULL)
                    {
                        s = p;
                        *s = '\0';
                        strcat(s, section); s += strlen(section);
                        *s++ = '/';
                        *s = '\0';
                        strcat(s, key); s += strlen(key);
                        *s = '\0';
                    
                        err = SWSetPropertyString(properties, p, value);
                        free(p);
                    }
                }
                
                if (err)
                 return err;
            }
        }
    
        if (section != NULL)
           free(section);
                    
        SDL_RWclose(ops);
        return kNoError;
    }
    else
        return kUnknownError;
}

//---------------------------------------------------------------------------------------
//  SaveProperties
//---------------------------------------------------------------------------------------

SWError SWSaveProperties(SWPropertiesPtr properties, char *filename)
{
   return SWSaveProperties_RW( properties, SDL_RWFromFile(filename, "w") );
}

//---------------------------------------------------------------------------------------
//  SWSaveProperties_RW
//---------------------------------------------------------------------------------------

SWError SWSaveProperties_RW(SWPropertiesPtr properties, SDL_RWops *ops)
{
    struct SWProperty *p;
    char *lastkey, *key, *slash;
    char line[80];
    
    if (properties != NULL && ops != NULL)
    {
        lastkey = NULL;
        for (p = properties->head; p != NULL; p = p->next)
        {
            key = p->key;
            
            slash = strrchr(key, '/');
            if (slash != NULL)
            {
                if (lastkey == NULL || strncmp(key, lastkey, slash - key) != 0)
                {
                    snprintf(line, sizeof(line), "[%s]\n", key);
                    line[ slash - key + 1] = ']';
                    line[ slash - key + 2] = '\n';
                    line[ slash - key + 3] = '\0';
                    SDL_RWwrite(ops, line, sizeof(char), strlen(line));
                
                    lastkey = key;
                }
                key = slash + 1;
            }
            
            snprintf(line, sizeof(line), "%s = %s\n", key, p->value);
            SDL_RWwrite(ops, line, sizeof(char), strlen(line));
        }
    
        SDL_RWclose(ops);
       return kNoError;
    }
    else
        return kUnknownError;
}

//---------------------------------------------------------------------------------------
//  GetPropertyString
//---------------------------------------------------------------------------------------

char *SWGetPropertyString(SWPropertiesPtr properties, char *name, char *defaultValue)
{
    struct SWProperty *p;
    char *value = defaultValue;
    
    if (properties != NULL && name != NULL)
    {
        for (p = properties->head; p != NULL; p = p->next)
        {
            if (strcmp(name, p->key) == 0)
            {
                value = p->value;
                break;
            }
        }
    }

    //fprintf(stderr, "GET %s=\"%s\"\n", name, value);
    return value;
}

//---------------------------------------------------------------------------------------
//  GetPropertyInteger
//---------------------------------------------------------------------------------------

int SWGetPropertyInteger(SWPropertiesPtr properties, char *name, int defaultValue)
{
    char *string = SWGetPropertyString(properties, name, NULL);
    return (string != NULL) ? (strtol(string, NULL, 10)) : defaultValue;
}

//---------------------------------------------------------------------------------------
//  GetPropertyFloat
//---------------------------------------------------------------------------------------

float SWGetPropertyFloat(SWPropertiesPtr properties, char *name, float defaultValue)
{
    char *string = SWGetPropertyString(properties, name, NULL);
    return (string != NULL) ? (strtof(string, NULL)) : defaultValue;
}

//---------------------------------------------------------------------------------------
//  GetPropertyBoolean
//---------------------------------------------------------------------------------------

SWBoolean SWGetPropertyBoolean(SWPropertiesPtr properties, char *name, SWBoolean defaultValue)
{
    char *string = SWGetPropertyString(properties, name, NULL);
    return (string != NULL) ? (strcmp(string, "true") == 0) : defaultValue;
}

//---------------------------------------------------------------------------------------
//  SetPropertyString
//---------------------------------------------------------------------------------------

SWError SWSetPropertyString(SWPropertiesPtr properties, char *name, char *value)
{
    struct SWProperty *p, *prev;

    //fprintf(stderr, "SET %s=\"%s\"\n", name, value);
               
    if (properties != NULL && name != NULL)
    {
        prev = NULL;
        for (p = properties->head; p != NULL; p = p->next)
        {
            prev = p;
            if (strcmp(name, p->key) == 0)
            {
                if (p->value != NULL)
                    free(p->value);
                p->value = local_strdup(value);
                return kNoError;
            }
        }
        
        p = (struct SWProperty *) calloc(1, sizeof(struct SWProperty));
        if (p == NULL)
             return kMemoryAllocationError;
        
        p->key = local_strdup(name);
        p->value = local_strdup(value);
    
        if (prev)
            prev->next = p;
        else
            properties->head = p;
        properties->numProperties++;
        return kNoError;
    }
    else
        return kUnknownError;
}

//---------------------------------------------------------------------------------------
//  SetPropertyInteger
//---------------------------------------------------------------------------------------

SWError SWSetPropertyInteger(SWPropertiesPtr properties, char *name, int value)
{
    char string[16];
    snprintf(string, sizeof(string), "%d", value);
    return SWSetPropertyString(properties, name, string);
}

//---------------------------------------------------------------------------------------
//  SetPropertyFloat
//---------------------------------------------------------------------------------------

SWError SWSetPropertyFloat(SWPropertiesPtr properties, char *name, float value)
{
    char string[256];
    snprintf(string, sizeof(string), "%f", value);
    return SWSetPropertyString(properties, name, string);
}

//---------------------------------------------------------------------------------------
//  SetPropertyBoolean
//---------------------------------------------------------------------------------------

SWError SWSetPropertyBoolean(SWPropertiesPtr properties, char *name, SWBoolean value)
{
    char *string = value ? "true" : "false";
    return SWSetPropertyString(properties, name, string);
}

//---------------------------------------------------------------------------------------

char *local_strdup(const char *str)
{
    char *res = malloc(strlen(str)+1);
    if (res != NULL) strcpy(res, str);
    return res;
}

char *SDL_RWgets(SDL_RWops *ctx, char *str, int size)
{
    int index = 0;
    while (index < size - 1)
    {
        if (SDL_RWread(ctx, &str[index], sizeof(char), 1) != 1)
            break;
        index++;
        if (str[index-1] == '\r' || str[index-1] == '\n')
            break;
    }
    str[index] = '\0';
    return (index > 0) ? str : NULL;
}


