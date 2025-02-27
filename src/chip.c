#include "chip.h"
#include <stdlib.h>
#include <string.h>
#include <SDL2/SDL.h>

#define UNUSED(v) (void)v
typedef void(*ChipOperation)(Chip*, uint16_t, uint8_t, uint8_t);

const uint8_t sprites_data[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0,  // 0
        0x20, 0x60, 0x20, 0x20, 0x70,  // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0,  // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0,  // 3
        0x90, 0x90, 0xF0, 0x10, 0x10,  // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0,  // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0,  // 6
        0xF0, 0x10, 0x20, 0x40, 0x40,  // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0,  // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0,  // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90,  // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0,  // B
        0xF0, 0x80, 0x80, 0x80, 0xF0,  // C
        0xE0, 0x90, 0x90, 0x90, 0xE0,  // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0,  // E
        0xF0, 0x80, 0xF0, 0x80, 0x80   // F
};

bool set_pixel(Chip* chip, uint8_t x, uint8_t y);
void clear_screen(Chip* chip);

void opcode_0nnn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy){
    UNUSED(vx); UNUSED(vy);
    if(opcode == 0x00E0){
        clear_screen(chip);
        return;
    }

    if(opcode == 0x00EE){
        if(chip->stack_counter == 0){
            //TODO: Bad rom
            return;
        }
        chip->program_counter = chip->stack[--chip->stack_counter];
    }
}

void opcode_1nnn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy){
    UNUSED(vx); UNUSED(vy);
    chip->program_counter = (opcode & 0xFFF);
}

void opcode_2nnn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy){
    UNUSED(vx); UNUSED(vy);
    if(chip->stack_counter+1 >= CHIP_MAX_STACK){
        return;
    }
    chip->stack[chip->stack_counter++] = chip->program_counter;
    chip->program_counter = (opcode & 0xFFF);
}

void opcode_3nnn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy){
    UNUSED(vy);
    if(chip->registers[vx] == (opcode & 0xFF)){
        chip->program_counter += 2;
    }
}

void opcode_4nnn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy){
    UNUSED(vy);
    if(chip->registers[vx] != (opcode & 0xFF)){
        chip->program_counter += 2;
    }
}

void opcode_5nnn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy){
    if((opcode & 0xF) != 0) return;
    if(chip->registers[vx] == chip->registers[vy]){
        chip->program_counter += 2;
    }
}

void opcode_6nnn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy){
    UNUSED(vy);
    chip->registers[vx] = (opcode & 0xFF);
}

void opcode_7nnn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy){
    UNUSED(vy);
    chip->registers[vx] = chip->registers[vx] + (opcode & 0xFF);
}

void opcode_8nnn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy){
    uint8_t last_bits = opcode & 0xF;
    if(last_bits == 0x0){
        chip->registers[vx] = chip->registers[vy];
        return;
    }

    if(last_bits == 0x1){
        chip->registers[vx] = chip->registers[vx] | chip->registers[vy];
        return;
    }

    if(last_bits == 0x2){
        chip->registers[vx] = chip->registers[vx] & chip->registers[vy];
        return;
    }

    if(last_bits == 0x3){
        chip->registers[vx] = chip->registers[vx] ^ chip->registers[vy];
        return;
    }

    if(last_bits == 0x4){
        uint16_t sum = chip->registers[vx] + chip->registers[vy];
        bool flag = false;
        if(sum > 0xFF) flag = true;
        chip->registers[vx] = (uint8_t) sum & 0xFF;
        chip->registers[0xF] = (uint8_t) flag;
        return;
    }

    if(last_bits == 0x5){
        // TODO: Handle underflow
        bool flag = false;
        if(chip->registers[vx] >= chip->registers[vy]) flag = true;
        chip->registers[vx] = chip->registers[vx] - chip->registers[vy];
        chip->registers[0xF] = (uint8_t) flag;
        return;
    }

    if(last_bits == 0x6){
        bool flag = chip->registers[vx] & 0x1;
        chip->registers[vx] = chip->registers[vx] >> 1;
        chip->registers[0xF] = flag;
        return;
    }

    if(last_bits == 0x7){
        bool flag = 0;
        if(chip->registers[vy] >= chip->registers[vx]) flag = true;
        chip->registers[vx] = chip->registers[vy] - chip->registers[vx];
        chip->registers[0xF] = flag;
        return;
    }

    if(last_bits == 0xE){
        //TODO: Check test suite
        chip->registers[0xF] = (chip->registers[vx] & 0x80) >> 7;
        chip->registers[vx] = (chip->registers[vx] << 1) & 0xFF;
        return;
    }
}

void opcode_9nnn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy){
    if((opcode & 0xF) != 0) return;
    if(chip->registers[vx] != chip->registers[vy]){
        chip->program_counter += 2;
    }
}

void opcode_Annn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy){
    UNUSED(vx); UNUSED(vy);
    chip->addres_register = (opcode & 0xFFF);
}

void opcode_Bnnn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy){
    UNUSED(vy);
    chip->program_counter = chip->registers[vx] + (opcode & 0xFFF);
}

void opcode_Cnnn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy){
    UNUSED(vy);
    int random = rand() & 0xFF;
    chip->registers[vx] = random & (opcode & 0xFF);
}

void opcode_Dnnn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy) {
    uint8_t width = 8;
    uint8_t height = (opcode & 0xF);

    chip->registers[0xF] = 0;

    for (uint8_t row = 0; row < height; row++) {
        uint8_t y_position = (chip->registers[vy] + row) % 32;

        uint8_t sprite = chip->memory[chip->addres_register + row];

        for (uint8_t col = 0; col < width; col++) {
            uint8_t x_position = (chip->registers[vx] + col) % 64;
            if (sprite & (0x1 << (7 - col))) {
                if (set_pixel(chip, x_position, y_position)) {
                    chip->registers[0xF] = 1;
                }
            }
        }
    }
}

void opcode_Ennn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy){
    UNUSED(vy);
    uint8_t last_bits = (opcode & 0xFF);
    uint8_t keycheck = chip->registers[vx] & 0xF;
    if(last_bits == 0x9E && chip_keypressed(chip, keycheck)){
        chip->program_counter += 2;
        return;
    }
    if(last_bits == 0xA1 && !(chip_keypressed(chip, keycheck))){
        chip->program_counter += 2;
        return;
    }
}

void opcode_Fnnn(Chip* chip, uint16_t opcode, uint8_t vx, uint8_t vy){
    UNUSED(vy);
    uint8_t last_bits = (opcode & 0xFF);
    if(last_bits == 0x07){
        chip->registers[vx] = chip->delay_timer;
        return;
    }
    if(last_bits == 0x0A){
        chip->paused = true;
        chip->keyevent.wait_key_event = true;
        chip->keyevent.register_store = vx;
        return;
    }
    if(last_bits == 0x15){
        chip->delay_timer = chip->registers[vx];
        return;
    }
    if(last_bits == 0x18){
        chip->sound_timer = chip->registers[vx];
        return;
    }
    if(last_bits == 0x1E){
        chip->addres_register = chip->addres_register + chip->registers[vx];
        return;
    }
    if(last_bits == 0x29){
        chip->addres_register = (chip->registers[vx] & 0xF) * 5;
        return;
    }
    if(last_bits == 0x33){
        uint16_t addr = chip->addres_register;
        chip->memory[addr] = (uint16_t) chip->registers[vx] / 100;
        chip->memory[addr+1] = (uint16_t) (chip->registers[vx] % 100) / 10;
        chip->memory[addr+2] = (uint16_t) chip->registers[vx] % 10;
        return;
    }

    if(last_bits == 0x55){
        for(uint8_t register_index=0; register_index<((opcode & 0xF00) >> 8)+1; register_index++){
            chip->memory[chip->addres_register + register_index] = chip->registers[register_index];
        }
        return;
    }

    if(last_bits == 0x65){
        for(uint8_t register_index=0; register_index<((opcode & 0xF00) >> 8)+1; register_index++){
            chip->registers[register_index] = chip->memory[chip->addres_register + register_index];
        }
        return;
    }
}

static ChipOperation ops[16] = {
    &opcode_0nnn,
    &opcode_1nnn,
    &opcode_2nnn,
    &opcode_3nnn,
    &opcode_4nnn,
    &opcode_5nnn,
    &opcode_6nnn,
    &opcode_7nnn,
    &opcode_8nnn,
    &opcode_9nnn,
    &opcode_Annn,
    &opcode_Bnnn,
    &opcode_Cnnn,
    &opcode_Dnnn,
    &opcode_Ennn,
    &opcode_Fnnn
};

Chip* chip_create(uint32_t speed, bool paused){
    Chip* chip = (Chip*) calloc(1, sizeof(Chip));
    if(!chip){
        return NULL;
    }
    chip->paused = paused;
    chip->tick_speed = speed;
    for(uint16_t i=0; i<sizeof(sprites_data)/sizeof(sprites_data[0]); i++){
        chip->memory[i] = sprites_data[i];
    }
    return chip;
}

void chip_destroy(Chip **chip){
    free(*chip);
    chip = NULL;
}

// 0000 0000 0000 0000 0000 0000 0000 0000 | 0000 0000 0000 0000 0000 0000 0000 0000
// Screen
bool set_pixel(Chip* chip, uint8_t x, uint8_t y){
    int index = y * 64 + x;
    chip->screen[index] = chip->screen[index] ^ 0x1;
    return chip->screen[index] == 0;
}

void clear_screen(Chip* chip){
    for(int index=0; index<64*32; index++){
        chip->screen[index] = 0;
    }
}

bool chip_getpixel(Chip* chip, uint8_t x, uint8_t y){
    return chip->screen[y * 64 + x] != 0;
}

// Keyboard

inline bool chip_keypressed(Chip* chip, uint8_t key){
    return (chip->keys & (0x1 << key)) != 0;
}

inline void chip_setkeypressed(Chip* chip, uint8_t key, bool value){
    chip->keys = value ? chip->keys | (0x1 << key) : chip->keys & ~(0x1 << key);
}

void chip_keydown(Chip* chip, uint8_t key){
    chip_setkeypressed(chip, key, true);
}

void chip_keyup(Chip* chip, uint8_t key){
    chip_setkeypressed(chip, key, false);
    if(chip->keyevent.wait_key_event){
        chip->paused = false;
        chip->registers[chip->keyevent.register_store] = key;
    }
}

// Tick loop
void chip_tick(Chip* chip){
    if(chip->paused) return;

    if(chip->delay_timer > 0){
        chip->delay_timer -= 1;
    }
    if(chip->sound_timer > 0){
        chip->sound_timer -= 1;
    }
    uint32_t remaining_ticks = chip->tick_speed;

    while(remaining_ticks != 0 && !chip->paused){
        uint16_t opcode = chip->memory[chip->program_counter] << 8 | chip->memory[chip->program_counter+1];
        chip->program_counter += 2;
        ops[(opcode & 0xF000) >> 12](chip, opcode, (opcode & 0xF00) >> 8, (opcode & 0xF0) >> 4);
        remaining_ticks -= 1;
    }
}

void chip_load_program(Chip* chip, FILE* file, uint16_t file_size, uint16_t offset){
    fread(&chip->memory[offset], sizeof(uint8_t), file_size, file);
    chip->program_counter = 0x200;
}
