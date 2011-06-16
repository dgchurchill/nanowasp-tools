#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "SDL.h"


#define FGR  238
#define FGG  166
#define FGB  0



inline void putpixel(SDL_Surface *surf, int x, int y, Uint32 col)
{
   Uint8 *p = (Uint8 *)surf->pixels + y * surf->pitch + x * 2;
   if ( (x < surf->w) && (y < surf->h) )
      *(Uint16 *)p = col;
}



int main(int argc, char *argv[])
{
   SDL_Surface *screen;
   SDL_PixelFormat *spf;
   Uint32 col;
   FILE *fin;
   int i, j, k, x, y;
   char c;


   if (SDL_Init(SDL_INIT_VIDEO) != 0)
   {
      fprintf(stderr, "init: SDL_Init failed - %s\n", SDL_GetError());
      return -1;
   }
   

   screen = SDL_SetVideoMode(320, 320, 16, 0);
   if (screen == NULL)
   {
      fprintf(stderr, "gui_init: SDL_SetVideoMode failed - %s\n",
              SDL_GetError());
      return 1;
   }

   spf = screen->format;
   col = SDL_MapRGB(spf, FGR, FGG, FGB);

   fin = fopen(argv[1], "r");
   for (i = 0; i < 256; i++)
   {
      x = (i % 16)*20;
      y = (i / 16)*20;

      for (j = 0; j < 16; j++)
      {
	 c = getc(fin);
	 for (k = 0; k < 8; k++)
	 {
	    if (c & 0x01)
	       putpixel(screen, x+(7-k), y+j, col);
	    c >>= 1;
	 }
      }
   }
   fclose(fin);

   SDL_UpdateRect(screen, 0, 0, 0, 0);

   while (!SDL_QuitRequested())
      usleep(100);

   SDL_Quit();

   return 0;
}


