/*
	simple stand-alone example program showing how to use BlitKernel
	Copyright (C) 2005 Anders F Bjoerklund <afb@users.sourceforge.net>

	"You see children know such a lot now, they soon don't believe in
	fairies, and every time a child says, `I don't believe in fairies,'
	there is a fairy somewhere that falls down dead." -- J.M. Barrie
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "BlitKernel.h"

#define WIDTH 640
#define HEIGHT 480

#define NB_PARTICLES 400
#define MS_DELAY 20

struct Particle
{
	int pixels[2]; //xy
	unsigned char colors[4]; // RGBA
};

int main(int argc, char* argv[])
{
	SDL_Event event;
	SDL_bool done = SDL_FALSE;
	SDL_Surface *logo = NULL;
	SDL_Surface *screen = NULL;
	Uint32 flags = 0;
	SDL_bool usegl = SDL_TRUE;
	
#ifdef HAVE_OPENGL
	struct BK_GL_func *gl = NULL;
	struct BK_GL_caps *caps = NULL;
	BK_GL_Surface *gllogo = NULL;
#endif

	struct Particle particles[NB_PARTICLES], *p;
	int i, f;

	for (i = 1; i < argc; i++)
	{
		if (strcmp("-nogl",argv[i]) == 0)
			usegl = SDL_FALSE;
		if (strcmp("-fullscreen",argv[i]) == 0)
			flags |= SDL_FULLSCREEN;
	}
	
	if (SDL_Init(SDL_INIT_VIDEO)<0)
	{
		printf("Unable to init SDL : %s\n",SDL_GetError());
		exit(1);
	}

	atexit(SDL_Quit);

	logo = BKLoadTGA_RW(SDL_RWFromFile("example.tga","rb"),1);

#ifdef HAVE_OPENGL
	if (usegl)
	{
	gl = BKLoadGLfunctions();
	if (gl == NULL)
	{
		printf("Unable to load OpenGL : %s\n",SDL_GetError());
		exit(1);
	}

	if (SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER,1)<0)
	{
		printf("Unable to set GL attribute : %s\n",SDL_GetError());
		exit(1);
	}

	if ((screen = SDL_SetVideoMode(WIDTH,HEIGHT,0,flags | SDL_OPENGL))==NULL)
	{
		printf("Unable to open video mode : %s\n",SDL_GetError());
		exit(1);
	}

	// must set up video mode, *before* checking caps
	caps = BKLoadGLcapabilities();

	if (logo != NULL)
	{
		gllogo = BKCreateNewGLSurface(logo,NULL);
	}
	}
	else
#endif
	{
	if ((screen = SDL_SetVideoMode(WIDTH,HEIGHT,0,flags | SDL_HWSURFACE | SDL_DOUBLEBUF))==NULL)
	{
		printf("Unable to open video mode : %s\n",SDL_GetError());
		exit(1);
	}
	
	BKDitherAlphaChannel(logo);

	SDL_SetColorKey(logo,SDL_SRCCOLORKEY,SDL_MapRGBA(logo->format,255,0,0,0));
	BKConvertAlphaChannelToColorKey(logo, NULL);
	SDL_SetAlpha(logo,0,SDL_ALPHA_OPAQUE);

	logo = SDL_DisplayFormat(logo);
	}
	
	for(i=0;i<NB_PARTICLES;i++)
	{
		p = &particles[i];

		p->colors[0]=100;
		p->colors[1]=100;
		p->colors[2]=rand()%250+5;
		p->colors[3]=rand()%150+105;

		p->pixels[0]=rand()%WIDTH;
		p->pixels[1]=rand()%HEIGHT;
	}

	SDL_WM_SetCaption( "BlitKernel", NULL );

#ifdef HAVE_OPENGL
	if (usegl)
	{
	gl->glViewport(0,0,WIDTH,HEIGHT);
	
	gl->glMatrixMode(GL_PROJECTION);
	gl->glLoadIdentity();
	gl->glOrtho(0,WIDTH,HEIGHT,0,-1,+1);
	
	gl->glMatrixMode(GL_MODELVIEW);
	gl->glLoadIdentity();
	
	gl->glEnable(GL_BLEND);
	gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	gl->glEnable(GL_POINT_SMOOTH);
	gl->glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
	gl->glPointSize(4.0f);
	}
#endif

	f = 0;
	do
	{
	#ifdef HAVE_OPENGL
		if (usegl)
		{
		gl->glClearColor(0.0f,0.0f,0.0f,0.0f);
		gl->glClear(GL_COLOR_BUFFER_BIT);
		}
		else
	#endif
		SDL_FillRect(screen,NULL,SDL_MapRGBA(screen->format,0,0,0,0));

	#ifdef HAVE_OPENGL
		if (usegl)
		gl->glBegin(GL_POINTS);
		else
	#endif
		if (SDL_MUSTLOCK(screen))
			SDL_LockSurface(screen);

		for(i=0;i<NB_PARTICLES;i++)
		{
			p = &particles[i];

				// get a little sine action going...
			p->pixels[0] = (WIDTH/2) + ((3*WIDTH)/8) *
				cosf( (f + i) * (M_PI / 180.f)) * sinf( i * (M_PI / 90.0f) );
			p->pixels[1] = (HEIGHT/2) + ((3*HEIGHT)/8) *
				sinf( (f + i) * (M_PI / 180.f)) * cosf( i * (M_PI / 45.0f) );

	#ifdef HAVE_OPENGL
			if (usegl)
			{
			gl->glColor4ub(p->colors[0],p->colors[1],p->colors[2],p->colors[3]);
			gl->glVertex2i(p->pixels[0],p->pixels[1]);
			}
			else
	#endif
			BKPutPixel(screen,p->pixels[0],p->pixels[1], SDL_MapRGBA(
				screen->format, p->colors[0],p->colors[1],p->colors[2],p->colors[3]));
		}

	#ifdef HAVE_OPENGL
		if (usegl)
		gl->glEnd();
		else
	#endif
		if (SDL_MUSTLOCK(screen))
			SDL_UnlockSurface(screen);

	#ifdef HAVE_OPENGL
		if (gllogo != NULL)
		{
			gl->glColor4ub(255,255,255, 230 + 25 * sinf( f * (M_PI / 180.f)));
			gl->glMatrixMode(GL_MODELVIEW);
			gl->glPushMatrix();
				gl->glLoadIdentity();
				gl->glTranslatef(WIDTH/2,HEIGHT/2,0.5f);
				gl->glScalef(2.0f,2.0f,1.0f);
				gl->glRotatef(f%360,0.0f,0.0f,1.0f);
				gl->glTranslatef(-logo->w/2,-logo->h/2,0);
				BKBlitGLSurface(gllogo);
			gl->glPopMatrix();
		}
		else
	#endif
		if (logo != NULL)
		{
			SDL_Rect dstRect;

			dstRect.x = WIDTH/2-logo->w/2;
			dstRect.y = HEIGHT/2-logo->h/2;
			dstRect.w = logo->w;
			dstRect.h = logo->h;
			
			SDL_BlitSurface(logo, NULL, screen, &dstRect);
		}

	#ifdef HAVE_OPENGL
		if (usegl)
		SDL_GL_SwapBuffers();
		else
	#endif
		SDL_Flip(screen);
	
		while(SDL_PollEvent(&event))
		{
//			if(event.type & SDL_KEYDOWN)
//				done=SDL_TRUE;
			if (  SDL_GetMouseState( NULL, NULL ) )
				done=SDL_TRUE;
			if(event.type == SDL_QUIT)
                 done=SDL_TRUE;
		}

		SDL_Delay(MS_DELAY);
		f++;
	}
	while (!done);

#ifdef HAVE_OPENGL
	if (gllogo)
		BKFreeGLSurface(gllogo);
#endif
	if (logo)
		SDL_FreeSurface(logo);

	SDL_Quit();
	return 0;
}
