/*
 *  rectangles.c
 *  written by Holmes Futrell
 *  use however you want
 */

#include <yarp/os/all.h>
#include <yarp/os/impl/NameConfig.h>
#include "SDL.h"
#include <time.h>


#define SCREEN_WIDTH 320
#define SCREEN_HEIGHT 480

int rectModifier = 0;

int
randomInt(int min, int max)
{
    return min + rand() % (max - min + 1);
}

void
render(SDL_Renderer *renderer)
{

    Uint8 r, g, b;

    /* Clear the screen */
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    /*  Come up with a random rectangle */
    SDL_Rect rect;
    rect.w = randomInt(64+rectModifier, 128+rectModifier);
    rect.h = randomInt(64+rectModifier, 128+rectModifier);
    rect.x = randomInt(0, SCREEN_WIDTH);
    rect.y = randomInt(0, SCREEN_HEIGHT);

    /* Come up with a random color */
    r = randomInt(50, 255);
    g = randomInt(50, 255);
    b = randomInt(50, 255);
    SDL_SetRenderDrawColor(renderer, r, g, b, 255);

    /*  Fill the rectangle in the color */
    SDL_RenderFillRect(renderer, &rect);

    /* update screen */
    SDL_RenderPresent(renderer);
}

int
main(int argc, char *argv[])
{
    //yarp stuff
    yarp::os::Network yarp;
    yarp::os::impl::NameConfig nc;
    nc.setManualConfig("10.0.1.32", 10000);
    yarp::os::BufferedPort<yarp::os::Bottle> port;

    yarp::os::ConstString portName = "/YarpSDLiOS";
    port.open(portName);

    
    
    SDL_Window *window;
    SDL_Renderer *renderer;
    int done;
    SDL_Event event;

    /* initialize SDL */
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("Could not initialize SDL\n");
        return 1;
    }

    /* seed random number generator */
    srand(time(NULL));

    /* create window and renderer */
    window =
        SDL_CreateWindow(NULL, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT,
                         SDL_WINDOW_OPENGL);
    if (!window) {
        printf("Could not initialize Window\n");
        return 1;
    }

    renderer = SDL_CreateRenderer(window, -1, 0);
    if (!renderer) {
        printf("Could not create renderer\n");
        return 1;
    }

    /* Enter render loop, waiting for user to quit */
    done = 0;
    while (!done) {
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                done = 1;
            }
            else if (event.type == SDL_APP_TERMINATING) {
                done = 1; //this works; but need to kill app by holding home and then up swiping
            }
            else if (event.type == SDL_APP_WILLENTERBACKGROUND) {
                //this doesn't quite work properly - only gets it after app comes back!
                if (!port.isClosed()) {
                    SDL_Log("app going into background. closing port...");
                    port.close();
                }
            }
            else if (event.type == SDL_APP_DIDENTERFOREGROUND) {
                SDL_Log("app coming back into foreground");
                if (port.isClosed()) {
                    SDL_Log("opening...");
                    port.open(portName);
                }
            }
        }
        render(renderer);
        SDL_Delay(1);
        
        
        if (port.getPendingReads()) {
            yarp::os::Bottle* bot = port.read();
            SDL_Log("received bot = %s\n", bot->toString().c_str());
            rectModifier += 200;
            if (rectModifier > 500)
                rectModifier = 0;
        }
    }

    /* shutdown SDL */
    SDL_Quit();
    
    port.close();

    return 0;
}
