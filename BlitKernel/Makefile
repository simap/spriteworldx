# This is a Makefile for GNU Make.
# $Id: Makefile,v 1.1 2005/06/05 22:15:32 afb Exp $
#
# written by Anders F Bjoerklund <afb@users.sourceforge.net>

SHELL=/bin/sh
PREFIX=/usr/local

OS=$(shell uname || uname -s)
ARCH=$(shell arch || uname -m)

#################################################################

ifeq ("$(OS)","Darwin")

GL_CFLAGS = -DHAVE_OPENGL
GL_LIBS = -framework OpenGL

SDL_CFLAGS = -I/Library/Frameworks/SDL.framework/Headers \
             -I/Library/Frameworks/SDL_image.framework/Headers \
             -DHAVE_SDL_IMAGE
SDL_LIBS = -lSDLMain \
           -framework SDL \
           -framework SDL_image \
           -framework Cocoa \
           -framework Carbon

else

GL_CFLAGS = -DHAVE_OPENGL
GL_LIBS = -L/usr/X11R6/lib -lGL -lm

SDL_CFLAGS = $(shell sdl-config --cflags) -DHAVE_SDL_IMAGE
SDL_LIBS = $(shell sdl-config --libs) -lSDL_image

endif

BLITKERNEL = \
	BlitKernelSDL.o \
	BlitKernelGL.o

#################################################################

CC=cc
AR=ar
RANLIB=ranlib
INSTALL=install

WARNINGS=-Wall
OPTIMIZATIONS=-O2
INCLUDES=
EXTRA=-pipe -g

CFLAGS=$(WARNINGS) $(OPTIMIZATIONS) $(EXTRA) $(INCLUDES) $(GL_CFLAGS) $(SDL_CFLAGS)
LDFLAGS=$(GL_LIBS) $(SDL_LIBS)

all: libBlitKernel.a example

.PHONY: clean all docs install

libBlitKernel.a: $(BLITKERNEL)
	$(AR) rc $@ $(BLITKERNEL)
	$(RANLIB) $@

example: example.o libBlitKernel.a
	$(CC) -o $@ $< libBlitKernel.a $(LDFLAGS)

docs: Doxyfile
	doxygen $<

clean:
	rm -f *.o
	rm -f libBlitKernel.a example
	rm -rf html latex

test: example
	./example

install: BlitKernel.h libBlitKernel.a
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/include
	$(INSTALL) -p BlitKernel.h $(DESTDIR)$(PREFIX)/include
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -p libBlitKernel.a $(DESTDIR)$(PREFIX)/lib

