#include <punani/punani.h>
#include <punani/tex.h>
#include "tex-internal.h"

void tex_get(struct _texture *tex)
{
	tex->t_ref++;
}

static SDL_Surface *new_surface(unsigned int x, unsigned int y, int alpha)
{
	SDL_Surface *surf;
	Uint32 rmask, gmask, bmask, amask;
	Uint32 flags = SDL_HWSURFACE;

	if ( alpha ) {
		flags |= SDL_SRCALPHA;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0x000000ff;
#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0xff000000;
#endif

	}else{
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
		rmask = 0xff000000;
		gmask = 0x00ff0000;
		bmask = 0x0000ff00;
		amask = 0;
#else
		rmask = 0x000000ff;
		gmask = 0x0000ff00;
		bmask = 0x00ff0000;
		amask = 0;
#endif

	}

	surf = SDL_CreateRGBSurface(flags, x, y, (alpha) ? 32 : 24,
					rmask, gmask, bmask, amask);
	if (surf == NULL)
		return NULL;

//	if ( !alpha )
//		surf->format->BytesPerPixel = 3;
	return surf;
}

SDL_Surface *tex_rgba(unsigned int x, unsigned int y)
{
	return new_surface(x, y, 1);
}

SDL_Surface *tex_rgb(unsigned int x, unsigned int y)
{
	return new_surface(x, y, 0);
}

SDL_Surface *texture_surface(texture_t tex)
{
	return tex->t_surf;
}