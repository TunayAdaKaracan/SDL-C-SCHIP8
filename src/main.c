#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>
#include "chip.h"

#define SCREEN_SCALE 16

struct {
    SDL_Window* window;
    SDL_Renderer* renderer;
    Chip* chip;
} game;

bool create_window(void){
    srand(time(0));
    if(SDL_Init(SDL_INIT_EVERYTHING)){
        fprintf(stderr, "Error initting SDL: %s\n", SDL_GetError());
        return false;
    }

    game.window = SDL_CreateWindow("Chip-8 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 64 * SCREEN_SCALE, 32 * SCREEN_SCALE, 0);
    if(!game.window){
        fprintf(stderr, "Error creating SDL window: %s\n", SDL_GetError());
        return false;
    }

    game.renderer = SDL_CreateRenderer(game.window, -1, 0);
    if(!game.renderer){
        fprintf(stderr, "Error creating SDL Renderer: %s\n", SDL_GetError());
        return false;
    }

    game.chip = chip_create(60, true);
    if(!game.chip){
        fprintf(stderr, "Error creating Chip-8\n");
        return false;
    }
    return true;
}

uint8_t map_keyboard_to_key(int key){
    switch (key) {
        case SDLK_1:
            return 0x1;
        case SDLK_2:
            return 0x2;
        case SDLK_3:
            return 0x3;
        case SDLK_4:
            return 0xc;
        case SDLK_q:
            return 0x4;
        case SDLK_w:
            return 0x5;
        case SDLK_e:
            return 0x6;
        case SDLK_r:
            return 0xD;
        case SDLK_a:
            return 0x7;
        case SDLK_s:
            return 0x8;
        case SDLK_d:
            return 0x9;
        case SDLK_f:
            return 0xE;
        case SDLK_z:
            return 0xA;
        case SDLK_x:
            return 0x0;
        case SDLK_c:
            return 0xB;
        case SDLK_v:
            return 0xF;
        default:
            return 255;
    }
}

void destroy_window(void){
    SDL_DestroyRenderer(game.renderer);
    SDL_DestroyWindow(game.window);
    SDL_Quit();
    chip_destroy(&game.chip);
}

void setup(void){
    FILE* file = fopen("./roms/3-corax+.ch8", "r");
    if(!file){
        fprintf(stderr, "Couldn't open file\n");
        return;
    }

    fseek(file, 0L, SEEK_END);
    uint16_t file_size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    chip_load_program(game.chip, file, file_size, 0x200);
    SDL_Log("File loaded to memory successfully");

    game.chip->paused = false;
    fclose(file);
}

bool process_events(void){
    SDL_Event event;
    while(SDL_PollEvent(&event)){
        switch (event.type) {
            case SDL_QUIT:
                return true;
            case SDL_KEYDOWN:
                if(event.key.keysym.sym == SDLK_ESCAPE) return true;
                if(map_keyboard_to_key(event.key.keysym.sym) != 255){
                    chip_keydown(game.chip, map_keyboard_to_key(event.key.keysym.sym));
                }
                break;
            case SDL_KEYUP:
                if(map_keyboard_to_key(event.key.keysym.sym) != 255){
                    chip_keyup(game.chip, map_keyboard_to_key(event.key.keysym.sym));
                }
                break;
            default:
                break;
        }
    }

    return false;
}

void update(void){
    chip_tick(game.chip);
}

void render(void){
    for(uint8_t y=0; y<32; y++){
        for(uint8_t x=0; x<64; x++){
            if(chip_getpixel(game.chip, x, y)){
                SDL_SetRenderDrawColor(game.renderer, 255, 255, 255, 255);
            } else {
                SDL_SetRenderDrawColor(game.renderer, 0, 0, 0, 255);
            }
            SDL_RenderFillRect(game.renderer, &(struct SDL_Rect){.x=x*SCREEN_SCALE, .y=y*SCREEN_SCALE, .w=SCREEN_SCALE, .h=SCREEN_SCALE});
        }
    }

    SDL_RenderPresent(game.renderer);
}

int main(void){
    if(!create_window()){
        destroy_window();
        return -1;
    }

    setup();
    while(true){
        if(process_events()) break;
        update();
        render();

        // About 60FPS
        SDL_Delay(16);
    }

    destroy_window();

    return 0;
}
