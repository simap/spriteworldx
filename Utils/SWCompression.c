#ifndef __SWCOMMON__
#include <SWCommonHeaders.h>
#endif

/**************************************************************
	LZSS.C -- A Data Compression Program
***************************************************************
	4/6/1989 Haruhiko Okumura
	Use, distribute, and modify this program freely.
	Please send me your improved versions.
		PC-VAN		SCIENCE
		NIFTY-Serve	PAF01022
		CompuServe	74050,1022
***************************************************************
    Modified for SDL by afb.
**************************************************************/
#include <stdlib.h>
#include <string.h>

#ifndef __SWCOMPRESSION__
#include "SWCompression.h"
#endif 

#define N        4096   /* size of ring buffer, power of two */
#define F          18   /* upper limit for match_length */
#define THRESHOLD   2   /* encode string into position and length
                           if match_length is greater than this */
#define OFTEN     '\0'  /* any character that will appear often. */

#undef NIL
#define NIL         N   /* index for root of binary search trees */

#undef EOF
#define EOF        -1   /* end of file, or error occured during read */


static void InitTree(SWCompressPtr state);
static void InsertNode(SWCompressPtr state, int r);
static void DeleteNode(SWCompressPtr state, int p);


struct SWCompress
{
unsigned int
        textsize,   /* text size counter */
        codesize;   /* code size counter */
unsigned char  code_buf[17];
    /*  code_buf[1..16] saves eight units of code, and
        code_buf[0] works as eight flags, "1" representing that the unit
        is an unencoded letter (1 byte), "0" a position-and-length pair 
        (2 bytes).  Thus, eight units require at most 16 bytes of code.  */
unsigned char
        text_buf[N + F - 1];    /* ring buffer of size N,
            with extra F-1 bytes to facilitate string comparison */
int     match_position, match_length,  /* of longest match.  These are
            set by the InsertNode() procedure. */
        lson[N + 1], rson[N + 257], dad[N + 1];  /* left & right children &
            parents -- These constitute binary search trees. */
};

struct SWDecompress
{
unsigned char
        text_buf[N + F - 1];    /* ring buffer of size N,
            with extra F-1 bytes to facilitate string comparison */
};

//---------------------------------------------------------------------------------------

SWError SWCompressInit(SWCompressPtr *newState)
{
    SWCompressPtr state;
    
    *newState = NULL;
    state = (SWCompressPtr) calloc(1, sizeof(struct SWCompress));
    if (state == NULL)
        return kMemoryAllocationError;
        
    *newState = state;
    return kNoError;
}

void SWCompressExit(SWCompressPtr *oldState)
{
    SWCompressPtr state;

    state = *oldState;
    if (state != NULL)
        free(state);
    *oldState = NULL;
}

SWError SWDecompressInit(SWDecompressPtr *newState)
{
    SWDecompressPtr state;
    
    *newState = NULL;
    state = (SWDecompressPtr) calloc(1, sizeof(struct SWDecompress));
    if (state == NULL)
        return kMemoryAllocationError;
        
    *newState = state;
    return kNoError;
}

void SWDecompressExit(SWDecompressPtr *oldState)
{
    SWDecompressPtr state;

    state = *oldState;
    if (state != NULL)
        free(state);
    *oldState = NULL;
}

long SWCompress(const void *buf, long size, void *data, long length)
{
    SWCompressPtr state;
    SDL_RWops *in, *out;
    long pos, len;
    
    in = SDL_RWFromConstMem(buf,size);
    out = SDL_RWFromMem(data,length);
    pos = SDL_RWtell(out);

    SWCompressInit(&state);
    SWCompress_RW(state, in, out);
    SWCompressExit(&state);

    len = SDL_RWtell(out) - pos;
    SDL_FreeRW(in);
    SDL_FreeRW(out);

    return len;
}

long SWDecompress(const void *buf, long size, void *data, long length)
{
    SWDecompressPtr state;
    SDL_RWops *in, *out;
    long pos, len;
    
    in = SDL_RWFromConstMem(buf,size);
    out = SDL_RWFromMem(data,length);
    pos = SDL_RWtell(out);

    SWDecompressInit(&state);
    SWDecompress_RW(state, in, out);
    SWDecompressExit(&state);

    len = SDL_RWtell(out) - pos;
    SDL_FreeRW(in);
    SDL_FreeRW(out);

    return len;
}

//---------------------------------------------------------------------------------------

static void InitTree(SWCompressPtr state)  /* initialize trees */
{
    int  i;

    /* For i = 0 to N - 1, rson[i] and lson[i] will be the right and
       left children of node i.  These nodes need not be initialized.
       Also, dad[i] is the parent of node i.  These are initialized to
       NIL (= N), which stands for 'not used.'
       For i = 0 to 255, rson[N + i + 1] is the root of the tree
       for strings that begin with character i.  These are initialized
       to NIL.  Note there are 256 trees. */

    for (i = N + 1; i <= N + 256; i++)
        state->rson[i] = NIL;
    
    for (i = 0; i < N; i++)
        state->dad[i] = NIL;
}

static void InsertNode(SWCompressPtr state, int r)
    /* Inserts string of length F, text_buf[r..r+F-1], into one of the
       trees (text_buf[r]'th tree) and returns the longest-match position
       and length via the global variables match_position and match_length.
       If match_length = F, then removes the old node in favor of the new
       one, because the old one will be deleted sooner.
       Note r plays double role, as tree node and position in buffer. */
{
    int  i, p, cmp;
    unsigned char  *key;

    cmp = 1;
    key = &state->text_buf[r];
    p = N + 1 + key[0];

    state->rson[r] = state->lson[r] = NIL;
    state->match_length = 0;
    for ( ; ; ) {
        if (cmp >= 0) {
            if (state->rson[p] != NIL)
                p = state->rson[p];
            else {
                state->rson[p] = r;
                state->dad[r] = p;
                return;
            }
        }
        else {
            if (state->lson[p] != NIL)
                p = state->lson[p];
            else {
                state->lson[p] = r;
                state->dad[r] = p;
                return;
            }
        }
        for (i = 1; i < F; i++)
            if ((cmp = key[i] - state->text_buf[p + i]) != 0)
                break;
        if (i > state->match_length) {
            state->match_position = p;
            if ((state->match_length = i) >= F)
                break;
        }
    }
    state->dad[r] = state->dad[p];
    state->lson[r] = state->lson[p];
    state->rson[r] = state->rson[p];

    state->dad[state->lson[p]] = r;
    state->dad[state->rson[p]] = r;
    if (state->rson[state->dad[p]] == p)
        state->rson[state->dad[p]] = r;
    else
        state->lson[state->dad[p]] = r;
    state->dad[p] = NIL;  /* remove p */
}

static void DeleteNode(SWCompressPtr state, int p)  /* deletes node p from tree */
{
    int  q;

    if (state->dad[p] == NIL)
        return;  /* not in tree */
    if (state->rson[p] == NIL)
        q = state->lson[p];
    else if (state->lson[p] == NIL)
        q = state->rson[p];
    else {
        q = state->lson[p];
        if (state->rson[q] != NIL) {
            do {
                q = state->rson[q];
            } while (state->rson[q] != NIL);
            state->rson[state->dad[q]] = state->lson[q];
            state->dad[state->lson[q]] = state->dad[q];
            state->lson[q] = state->lson[p];
            state->dad[state->lson[p]] = q;
        }
        state->rson[q] = state->rson[p];
        state->dad[state->rson[p]] = q;
    }
    state->dad[q] = state->dad[p];
    if (state->rson[state->dad[p]] == p)
        state->rson[state->dad[p]] = q;
    else
        state->lson[state->dad[p]] = q;

    state->dad[p] = NIL;
}

//---------------------------------------------------------------------------------------

static int SDL_getc(SDL_RWops *in)
{
  unsigned char buf;
  if (SDL_RWread(in, &buf, sizeof(buf), 1) == 1)
    return buf;
  else
    return EOF;
}

static int SDL_putc(int c, SDL_RWops *out)
{
  unsigned char buf = c;
  if (SDL_RWwrite(out, &buf, sizeof(buf), 1) == 1)
    return buf;
  else
    return EOF;
}

void SWCompress_RW(SWCompressPtr state, SDL_RWops *in, SDL_RWops *out)
{
    int  i, c, len, r, s, last_match_length, code_buf_ptr;
    unsigned char mask;

    InitTree(state);  /* initialize trees */

    state->code_buf[0] = 0;
        
    code_buf_ptr = mask = 1;
    s = 0;
    r = N - F;

    for (i = s; i < r; i++)
    {
        state->text_buf[i] = OFTEN;  /* Clear the buffer with
        any character that will appear often. */
    }
        
    for (len = 0; len < F && (c = SDL_getc(in)) != EOF; len++ )
    {
        state->text_buf[r + len] = c;  /* Read F bytes into the last F bytes of the buffer */
    }
    
    state->textsize = len;
    if (state->textsize == 0)
        return;  /* text of size zero */
    
    for (i = 1; i <= F; i++)
        InsertNode(state, r - i);  /* Insert the F strings,
        each of which begins with one or more 'space' characters.  Note 
        the order in which these strings are inserted.  This way,
        degenerate trees will be less likely to occur. */
    InsertNode(state, r);  /* Finally, insert the whole string just read.  The
        global variables match_length and match_position are set. */
    do {
        if (state->match_length > len)
            state->match_length = len;  /* match_length
            may be spuriously long near the end of text. */
        if (state->match_length <= THRESHOLD) {
            state->match_length = 1;  /* Not long enough match.  Send one byte. */
            state->code_buf[0] |= mask;  /* 'send one byte' flag */
            state->code_buf[code_buf_ptr++] = state->text_buf[r];  /* Send uncoded. */
        } else {
            state->code_buf[code_buf_ptr++] = (unsigned char) state->match_position;
            state->code_buf[code_buf_ptr++] = (unsigned char)
                (((state->match_position >> 4) & 0xf0)
                | (state->match_length - (THRESHOLD + 1)));  /* Send position and
                    length pair. Note match_length > THRESHOLD. */
        }
        if ((mask <<= 1) == 0) {  /* Shift mask left one bit. */
            for (i = 0; i < code_buf_ptr; i++)  /* Send at most 8 units of */
            {
                SDL_putc(state->code_buf[i],out); /* code together */
            }
            
            state->codesize += code_buf_ptr;
            state->code_buf[0] = 0;  code_buf_ptr = mask = 1;
        }
        last_match_length = state->match_length;
        
        for (i = 0; i < last_match_length &&
                (c = SDL_getc(in)) != EOF; i++ )
        {
            DeleteNode(state, s);       /* Delete old strings and */
            state->text_buf[s] = c; /* read new bytes */
            if (s < F - 1) state->text_buf[s + N] = c;  /* If the position is
                near the end of buffer, extend the buffer to make
                string comparison easier. */
            s = (s + 1) & (N - 1);
            r = (r + 1) & (N - 1);
                /* Since this is a ring buffer, increment the position
                     modulo N. */
            InsertNode(state, r);   /* Register the string in text_buf[r..r+F-1] */
        }
    
        while (i++ < last_match_length) {   /* After the end of text, */
            DeleteNode(state, s);                   /* no need to read, but */
            s = (s + 1) & (N - 1);
            r = (r + 1) & (N - 1);
            if (--len) InsertNode(state, r);        /* buffer may not be empty. */
        }
    } while (len > 0);  /* until length of string to be processed is zero */

    if (code_buf_ptr > 1) {     /* Send remaining code. */
        for (i = 0; i < code_buf_ptr; i++)
        {
            SDL_putc(state->code_buf[i],out);
        }
        state->codesize += code_buf_ptr;
    }
}

void SWDecompress_RW(SWDecompressPtr state, SDL_RWops *in, SDL_RWops *out)
{
    int  i, j, k, r, c;
    unsigned int  flags;

    flags = 0;
    r = N - F;
 
    for (i = 0; i < r; i++)
    {
        state->text_buf[i] = OFTEN;  /* Clear the buffer with
        any character that will appear often. */
    }
        
    for ( ; ; )
    {
        flags >>= 1;
        if ((flags & 256) == 0)
        {
            if ( (c = SDL_getc(in)) == EOF)
                break;
        
            flags = c | 0xff00;     /* uses higher byte cleverly */
        }                           /* to count eight */
        
        if (flags & 1)
        {
            if ( (c = SDL_getc(in)) == EOF)
                break;
            
            SDL_putc(c,out);
             
            state->text_buf[r++] = c;
            r &= (N - 1);
        }
        else
        {
            if ( (i = SDL_getc(in)) == EOF)
                break;
            if ( (j = SDL_getc(in)) == EOF)
                break;
        
            i |= (j & 0xf0) << 4;
            j = (j & 0x0f) + THRESHOLD;

            for (k = 0; k <= j; k++)
            {
                c = state->text_buf[(i + k) & (N - 1)];
                
                SDL_putc(c,out);
                
                state->text_buf[r++] = c;
                r &= (N - 1);
            }
        }
    }
}

