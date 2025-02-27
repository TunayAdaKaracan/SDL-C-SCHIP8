#pragma once
#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

#define CHIP_MAX_MEMORY 4096
#define CHIP_MAX_STACK 32
#define CHIP_PROGRAM_OFFSET 0x200

typedef struct Chip {
    uint32_t tick_speed;
    bool paused;

    uint8_t sound_timer;
    uint8_t delay_timer;

    uint16_t addres_register;
    uint16_t program_counter;
    uint8_t stack_counter;

    uint8_t registers[16];
    uint8_t memory[CHIP_MAX_MEMORY];
    //uint32_t screen[64];
    uint8_t screen[64*32];
    uint16_t stack[CHIP_MAX_STACK];

    uint16_t keys;

    struct {
        bool wait_key_event;
        uint8_t register_store;
    } keyevent;
} Chip;

// Create and manage
Chip* chip_create(uint32_t speed, bool paused);
void chip_destroy(Chip** chip);
void chip_load_program(Chip* chip, FILE* file, uint16_t file_size, uint16_t offset);

// Keyboard Controls
bool chip_keypressed(Chip* chip, uint8_t key);
void chip_setkeypressed(Chip* chip, uint8_t key, bool value);
void chip_keydown(Chip* chip, uint8_t key);
void chip_keyup(Chip* chip, uint8_t key);

// Update
void chip_tick(Chip* chip);

// Screen
bool chip_getpixel(Chip* chip, uint8_t x, uint8_t y);
