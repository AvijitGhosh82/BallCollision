#include <SDL/SDL.h>
#include <math.h>
#define SCREEN_WIDTH  640
#define SCREEN_HEIGHT 480
#define SPRITE_SIZE    32


// gcc game.c -lm `sdl-config --cflags --libs`


void set_pixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * 4;
    *(Uint32 *)target_pixel = pixel;
}


void draw_circle(SDL_Surface *surface, int n_cx, int n_cy, int radius, Uint32 pixel)
{
    // if the first pixel in the screen is represented by (0,0) (which is in sdl)
    // remember that the beginning of the circle is not in the middle of the pixel
    // but to the left-top from it:
 
    double error = (double)-radius;
    double x = (double)radius -0.5;
    double y = (double)0.5;
    double cx = n_cx - 0.5;
    double cy = n_cy - 0.5;
 
    while (x >= y)
    {
        set_pixel(surface, (int)(cx + x), (int)(cy + y), pixel);
        set_pixel(surface, (int)(cx + y), (int)(cy + x), pixel);
 
        if (x != 0)
        {
            set_pixel(surface, (int)(cx - x), (int)(cy + y), pixel);
            set_pixel(surface, (int)(cx + y), (int)(cy - x), pixel);
        }
 
        if (y != 0)
        {
            set_pixel(surface, (int)(cx + x), (int)(cy - y), pixel);
            set_pixel(surface, (int)(cx - y), (int)(cy + x), pixel);
        }
 
        if (x != 0 && y != 0)
        {
            set_pixel(surface, (int)(cx - x), (int)(cy - y), pixel);
            set_pixel(surface, (int)(cx - y), (int)(cy - x), pixel);
        }
 
        error += y;
        ++y;
        error += y;
 
        if (error >= 0)
        {
            --x;
            error -= x;
            error -= x;
        }
    }
}


void fill_circle(SDL_Surface *surface, int cx, int cy, int radius, Uint32 pixel)
{
    // Note that there is more to altering the bitrate of this 
    // method than just changing this value.  See how pixels are
    // altered at the following web page for tips:
    //   http://www.libsdl.org/intro.en/usingvideo.html
    static const int BPP = 4;
 
    double r = (double)radius;
    double dy;
 
    for (dy = 1; dy <= r; dy += 1.0)
    {
        // This loop is unrolled a bit, only iterating through half of the
        // height of the circle.  The result is used to draw a scan line and
        // its mirror image below it.
 
        // The following formula has been simplified from our original.  We
        // are using half of the width of the circle because we are provided
        // with a center and we need left/right coordinates.
 
        double dx = floor(sqrt((2.0 * r * dy) - (dy * dy)));
        int x = cx - dx;
 
        // Grab a pointer to the left-most pixel for each half of the circle
        Uint8 *target_pixel_a = (Uint8 *)surface->pixels + ((int)(cy + r - dy)) * surface->pitch + x * BPP;
        Uint8 *target_pixel_b = (Uint8 *)surface->pixels + ((int)(cy - r + dy)) * surface->pitch + x * BPP;
 
        for (; x <= cx + dx; x++)
        {
            *(Uint32 *)target_pixel_a = pixel;
            *(Uint32 *)target_pixel_b = pixel;
            target_pixel_a += BPP;
            target_pixel_b += BPP;
        }
    }
}


int main(int argc, char *argv[])
{
    static const int width = 640;
    static const int height = 480;
    static const int max_radius = 64;
 
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
      return 1;
 
    atexit(SDL_Quit);
 
    SDL_Surface *screen = SDL_SetVideoMode(width, height, 0, SDL_DOUBLEBUF);
 
    if (screen == NULL)
        return 2;

 
    while(1)
    {
        // int dum=0;
        // for(dum=0;dum<=90000000;dum++);


        // SDL_FillRect(screen, NULL, 0x000000);
        SDL_Event event; 
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
                return 0;
        }
 
        int x = rand() % (width - 4) + 2;
        int y = rand() % (height - 4) + 2;
        int r = rand() % (max_radius - 10) + 10;
        int c = ((rand() % 0xff) << 16) +
                ((rand() % 0xff) << 8) +
                (rand() % 0xff);
 
        if (r >= 4)
        {
            if (x < r + 2)
                x = r + 2;
            else if (x > width - r - 2)
                x = width - r - 2;
 
            if (y < r + 2)
                y = r + 2;
            else if (y > height - r - 2)
                y = height - r - 2;
        }
 
        SDL_LockSurface(screen);
 
        fill_circle(screen, x, y, r, 0xff000000 + c);
        // draw_circle(screen, x, y, r, 0xffffffff);
 
        SDL_FreeSurface(screen);
 
        SDL_Flip(screen);
    }
 
    return 0;
}

// int main ( int argc, char *argv[] )
// {
//   SDL_Surface *screen, *temp, *sprite, *grass;
//   SDL_Rect rcSprite, rcGrass;
//   SDL_Event event;
//   Uint8 *keystate;

//   int colorkey, gameover;

//   /* initialize SDL */
//   SDL_Init(SDL_INIT_VIDEO);

//   /* set the title bar */
//   SDL_WM_SetCaption("SDL Move", "SDL Move");

//   /* create window */
//   screen = SDL_SetVideoMode(SCREEN_WIDTH, SCREEN_HEIGHT, 0, 0);

//   /* load sprite */
//   temp   = SDL_LoadBMP("sprite.bmp");
//   sprite = SDL_DisplayFormat(temp);
//   SDL_FreeSurface(temp);

//   /* setup sprite colorkey and turn on RLE */
//   colorkey = SDL_MapRGB(screen->format, 255, 0, 255);
//   SDL_SetColorKey(sprite, SDL_SRCCOLORKEY | SDL_RLEACCEL, colorkey);

//   /* load grass */
//   temp  = SDL_LoadBMP("grass.bmp");
//   grass = SDL_DisplayFormat(temp);
//   SDL_FreeSurface(temp);

//   /* set sprite position */
//   rcSprite.x = 0;
//   rcSprite.y = 0;

//   gameover = 0;

//   /* message pump */
//   while (!gameover)
//   {
//     /* look for an event */
//     if (SDL_PollEvent(&event)) {
//       /* an event was found */
//       switch (event.type) {
//         /* close button clicked */
//         case SDL_QUIT:
//           gameover = 1;
//           break;

//         /* handle the keyboard */
//         case SDL_KEYDOWN:
//           switch (event.key.keysym.sym) {
//             case SDLK_ESCAPE:
//             case SDLK_q:
//               gameover = 1;
//               break;
//           }
//           break;
//       }
//     }

//     /* handle sprite movement */
//     keystate = SDL_GetKeyState(NULL);
//     if (keystate[SDLK_LEFT] ) {
//       rcSprite.x -= 2;
//     }
//     if (keystate[SDLK_RIGHT] ) {
//       rcSprite.x += 2;
//     }
//     if (keystate[SDLK_UP] ) {
//       rcSprite.y -= 2;
//     }
//     if (keystate[SDLK_DOWN] ) {
//       rcSprite.y += 2;
//     }
//     /* collide with edges of screen */
//     if ( rcSprite.x < 0 ) {
//       rcSprite.x = 0;
//     }
//     else if ( rcSprite.x > SCREEN_WIDTH-SPRITE_SIZE ) {
//       rcSprite.x = SCREEN_WIDTH-SPRITE_SIZE;
//     }
//     if ( rcSprite.y < 0 ) {
//       rcSprite.y = 0;
//     }
//     else if ( rcSprite.y > SCREEN_HEIGHT-SPRITE_SIZE ) {
//       rcSprite.y = SCREEN_HEIGHT-SPRITE_SIZE;
//     }

//     /* draw the grass */
//     int x,y;
//     for (x = 0; x < SCREEN_WIDTH / SPRITE_SIZE; x++) {
//       for (y = 0; y < SCREEN_HEIGHT / SPRITE_SIZE; y++) {
//         rcGrass.x = x * SPRITE_SIZE;
//         rcGrass.y = y * SPRITE_SIZE;
//         SDL_BlitSurface(grass, NULL, screen, &rcGrass);
//       }
//     }
//     /* draw the sprite */
//     SDL_BlitSurface(sprite, NULL, screen, &rcSprite);

//     /* update the screen */
//     SDL_UpdateRect(screen, 0, 0, 0, 0);
//   }

//   /* clean up */
//   SDL_FreeSurface(sprite);
//   SDL_FreeSurface(grass);
//   SDL_Quit();

//   return 0;
// }