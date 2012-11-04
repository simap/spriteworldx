# This is a Makefile for GNU Make.
# $Id: Makefile,v 1.10 2006/06/18 19:56:42 afb Exp $
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
           -framework Cocoa

else

GL_CFLAGS = -DHAVE_OPENGL
GL_LIBS = -L/usr/X11R6/lib -lGL

SDL_CFLAGS = $(shell sdl-config --cflags) -DHAVE_SDL_IMAGE
SDL_LIBS = $(shell sdl-config --libs) -lSDL_image

endif

SPRITEWORLD = .

SRCDIR = $(SPRITEWORLD)/Sources
INCDIR = $(SPRITEWORLD)/Headers
UTILDIR = $(SPRITEWORLD)/Utils
BKDIR = $(SPRITEWORLD)/BlitKernel
LIBDIR = $(SPRITEWORLD)/Libraries

OBJECTS = \
	Sprite.o \
	SpriteCreation.o \
	SpriteFrame.o \
	SpriteLayer.o \
	SpriteWorld.o \
	SpriteWorldUtils.o \
	Scrolling.o \
	Tiling.o \

BLITKERNEL = \
	BlitKernelSDL.o \
	BlitKernelGL.o


UTILS= \
	SWStats.o \
	SWParticles.o \
	SWProperties.o \
	SWCompression.o \
	#

MAINLIB = libSpriteWorldX.a
HELPERLIB = libBlitKernel.a

#################################################################

CC=cc
CXX=c++
AR=ar
RANLIB=ranlib
INSTALL=install

WARNINGS=-Wall
OPTIMIZATIONS=-O2
INCLUDES=-I$(INCDIR) -I$(BKDIR) -I$(UTILDIR)
EXTRA=-pipe -g

CFLAGS=$(WARNINGS) $(OPTIMIZATIONS) $(EXTRA) $(INCLUDES) $(GL_CFLAGS) $(SDL_CFLAGS)
CXXFLAGS=$(CFLAGS)
LDFLAGS=$(GL_LIBS) $(SDL_LIBS)

VPATH= $(SRCDIR):$(INCDIR):$(BKDIR):$(UTILDIR)

all: $(LIBDIR)/$(HELPERLIB) $(LIBDIR)/$(MAINLIB)

.PHONY: clean all docs dist install

$(LIBDIR)/$(MAINLIB) : $(OBJECTS) $(UTILS) $(BLITKERNEL)
	mkdir -p $(LIBDIR)
	$(AR) rc $@ $(OBJECTS) $(UTILS) $(BLITKERNEL)
	$(RANLIB) $@

$(LIBDIR)/$(HELPERLIB) : $(BLITKERNEL)
	mkdir -p $(LIBDIR)
	$(AR) rc $@ $(BLITKERNEL)
	$(RANLIB) $@

$(OBJECTS) : SWCommonHeaders.h

docs: Doxyfile
	doxygen $<
	-cd html && zip -0 -r refman.zip index.html doxygen.* *.html *.png
	-cd latex && make refman.pdf

dist:
	tar czf SpriteWorldX.tar.gz -C .. SpriteWorldX \
	        --exclude SpriteWorldX.tar.gz --exclude CVS --exclude .DS_Store 

clean:
	rm -f *.o
	rm -rf html latex

SWClasses.o: SWCommonHeaders.h SpriteWorld.h Scrolling.h Tiling.h \
                               SpriteLayer.h Sprite.h SpriteFrame.h

classes: SWClasses.o

install: $(LIBDIR)/$(MAINLIB)
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/include
	$(INSTALL) -p $(INCDIR)/*.h $(INCDIR)/*.hpp $(UTILDIR)/*.h $(DESTDIR)$(PREFIX)/include
	$(INSTALL) -d $(DESTDIR)$(PREFIX)/lib
	$(INSTALL) -p $(LIBDIR)/lib*.a $(DESTDIR)$(PREFIX)/lib

