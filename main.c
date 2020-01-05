#include <stdio.h>
#include <stdbool.h>
#include <sys/time.h>
#include <SDL2/SDL.h>

#include "RTE_Log.h"
#include "RTE_Memory.h"

MEMORY_ALIGN_BYTES(static uint8_t bank_0[1024*1024]) = {0};

static size_t user_data_out (uint8_t *data,size_t length)
{
    printf("%.*s", (int)length, (char *)data);
    return length;
}

static uint64_t user_get_tick(void)
{
    return SDL_GetTicks();
}

static bool app_run_flag = true;

int main( int argc, char* args[] )
{
    log_init(NULL, user_data_out, NULL, NULL, user_get_tick);
    LOG_INFO("ENTRY", "Helloworld!");
    memory_pool(BANK_0, bank_0, 1024*1024);
    memory_demon(BANK_0);
    if( SDL_Init( SDL_INIT_VIDEO ) < 0 ) {
        printf( "SDL could not initialize! SDL_Error: %s\n", SDL_GetError() );
    }
    while(app_run_flag) {
        LOG_INFO("ENTRY", "simulator running...");
        SDL_Delay(1000);
    }
    SDL_Quit();
    return 0;
}