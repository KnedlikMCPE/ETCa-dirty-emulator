#include <iostream>
#include <fstream>
#include <cmath>
#include <SDL.h>

int main(int argc, char** argv) {
    // SDL
    SDL_Event event;
    SDL_Renderer* renderer;
    SDL_Window* window;

    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_CreateWindowAndRenderer(640, 480, 0, &window, &renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);
    // SDL

    int params = 3;
    if (argc != params) {
        std::cout << "INVALID ARGUMENTS" << std::endl;
        std::cout << "USAGE: etca_emu <memory size> <ROM location>" << std::endl;
        std::cout << "ALL NUMBERS HAVE TO BE IN BASE 10" << std::endl;
        return 1;
    }
    uint8_t memory[atoi(argv[1])];
    char* path = argv[2];

    int16_t REGS[8] = {
            0, 0, 0, 0, 0, 0, 0, 0
    };

    // 0 = ZERO, 1 = SIGN, 2 = CARRY, 3 = OVERFLOW
    bool FLAGS[4] {
        false, false, false, false
    };

    const uint16_t CPUID1 = 0;
    const uint16_t CPUID2 = 0;
    const uint8_t FEAT = 0;

    int16_t PAGE = 0;

    std::ifstream ROM(path, std::ios::binary);
    if (!ROM) {
        printf("INVALID ROM");
        return -2;
    }
    int i = 0x8000;
    char byte;
    while (ROM.get(byte)) {
        std::cout << (int) byte << std::endl;
        memory[i] = byte;
        i++;
    }

    uint16_t counter = 0x8000;
    while (true) {
        uint16_t instruction = memory[counter] << 8;
        counter++;
        instruction = instruction | memory[counter];
        if ((instruction & 0b1111000000000000) == 0b0001000000000000) {
            uint8_t opcode = (instruction & 0b0000111100000000) >> 8;
            int16_t *REG_A = &REGS[instruction & 0b0000000011100000];
            int16_t *REG_B = &REGS[instruction & 0b0000000000011100];

            int16_t in_a;
            int16_t in_b;
            int32_t real_result;
            int16_t temp;
            switch (opcode) {
                case 0b0000:
                    in_a = *REG_A;
                    in_b = *REG_B;

                    real_result = (int32_t) *REG_A + (int32_t) *REG_B;
                    *REG_A = *REG_A + *REG_B;

                    FLAGS[0] = *REG_A == 0;
                    FLAGS[1] = (*REG_A & 0x8000) != 0;
                    FLAGS[2] = (real_result & 0x10000) != 0;
                    FLAGS[3] = (in_a & 0x8000 == in_b & 0x8000) && (in_a & 0x8000 != *REG_A & 0x8000);
                    break;

                case 0b0001:
                    in_a = *REG_A;
                    in_b = *REG_B;

                    real_result = (int32_t) *REG_A + (int32_t) ~*REG_B + 1;
                    *REG_A = *REG_A + ~*REG_B + 1;

                    FLAGS[0] = *REG_A == 0;
                    FLAGS[1] = (*REG_A & 0x8000) != 0;
                    FLAGS[2] = (real_result & 0x10000) != 0;
                    FLAGS[3] = (in_a & 0x8000 == in_b & 0x8000) && (in_a & 0x8000 != *REG_A & 0x8000);
                    break;

                case 0b0010:
                    in_a = *REG_A;
                    in_b = *REG_B;

                    real_result = (int32_t) *REG_B + (int32_t) ~*REG_A + 1;
                    *REG_A = ~*REG_A + *REG_B + 1;

                    FLAGS[0] = *REG_A == 0;
                    FLAGS[1] = (*REG_A & 0x8000) != 0;
                    FLAGS[2] = (real_result & 0x10000) != 0;
                    FLAGS[3] = (in_a & 0x8000 == in_b & 0x8000) && (in_a & 0x8000 != *REG_A & 0x8000);
                    break;

                case 0b0011:
                    in_a = *REG_A;
                    in_b = *REG_B;

                    real_result = (int32_t) *REG_A + (int32_t) ~*REG_B + 1;
                    temp = *REG_A + ~*REG_B + 1;

                    FLAGS[0] = temp == 0;
                    FLAGS[1] = (temp & 0x8000) != 0;
                    FLAGS[2] = (real_result & 0x10000) != 0;
                    FLAGS[3] = (in_a & 0x8000 == in_b & 0x8000) && (in_a & 0x8000 != temp & 0x8000);
                    break;

                case 0b0100:
                    *REG_A = *REG_A | *REG_B;

                    FLAGS[0] = *REG_A == 0;
                    FLAGS[1] = (*REG_A & 0x8000) != 0;
                    break;

                case 0b0101:
                    *REG_A = *REG_A ^ *REG_B;

                    FLAGS[0] = *REG_A == 0;
                    FLAGS[1] = (*REG_A & 0x8000) != 0;
                    break;

                case 0b0110:
                    *REG_A = *REG_A & *REG_B;

                    FLAGS[0] = *REG_A == 0;
                    FLAGS[1] = (*REG_A & 0x8000) != 0;
                    break;

                case 0b0111:
                    temp = *REG_A & *REG_B;

                    FLAGS[0] = temp == 0;
                    FLAGS[1] = (temp & 0x8000) != 0;
                    break;

                case 0b1000:
                    *REG_A = *REG_B;
                    break;

                case 0b1001:
                    *REG_A = *REG_B;
                    break;

                case 0b1010:
                    *REG_A = memory[PAGE << 16 + *REG_B];
                    break;

                case 0b1011:
                    memory[PAGE << 16 + *REG_B] = *REG_A;
                    break;

                case 0b1100:
                    *REG_A = (*REG_A << 5) | *REG_B;

                case 0b1110:
                    switch (*REG_B) {
                        case 0:
                            *REG_A = CPUID1;
                            break;
                        case 1:
                            *REG_A = CPUID2;
                            break;
                        case 2:
                            *REG_A = FEAT;
                            break;
                        case 65535:
                            *REG_A = PAGE;
                            break;
                        default:
                            break;
                    }
                    break;
                case 0b1111:
                    if (*REG_B == 65535) {
                        PAGE = *REG_A;
                    }
                    break;
                default:
                    printf("Unknown OPCODE");
                    return -1;
            }
        } else if ((instruction & 0b1111000000000000) == 0b0101000000000000) {
            uint8_t opcode = (instruction & 0b0000111100000000) >> 8;
            int16_t *REG_A = &REGS[instruction & 0b0000000011100000];
            int16_t REG_B = instruction & 0b0000000000011111;

            int16_t in_a;
            int16_t in_b;
            int32_t real_result;
            int16_t temp;
            switch (opcode) {
                case 0b0000:
                    in_a = *REG_A;
                    in_b = REG_B;

                    real_result = (int32_t) *REG_A + (int32_t) REG_B;
                    *REG_A = *REG_A + REG_B;

                    FLAGS[0] = *REG_A == 0;
                    FLAGS[1] = (*REG_A & 0x8000) != 0;
                    FLAGS[2] = (real_result & 0x10000) != 0;
                    FLAGS[3] = (in_a & 0x8000 == in_b & 0x8000) && (in_a & 0x8000 != *REG_A & 0x8000);
                    break;

                case 0b0001:
                    in_a = *REG_A;
                    in_b = REG_B;

                    real_result = (int32_t) *REG_A + (int32_t) ~REG_B + 1;
                    *REG_A = *REG_A + ~REG_B + 1;

                    FLAGS[0] = *REG_A == 0;
                    FLAGS[1] = (*REG_A & 0x8000) != 0;
                    FLAGS[2] = (real_result & 0x10000) != 0;
                    FLAGS[3] = (in_a & 0x8000 == in_b & 0x8000) && (in_a & 0x8000 != *REG_A & 0x8000);
                    break;

                case 0b0010:
                    in_a = *REG_A;
                    in_b = REG_B;

                    real_result = (int32_t) REG_B + (int32_t) ~*REG_A + 1;
                    *REG_A = ~*REG_A + REG_B + 1;

                    FLAGS[0] = *REG_A == 0;
                    FLAGS[1] = (*REG_A & 0x8000) != 0;
                    FLAGS[2] = (real_result & 0x10000) != 0;
                    FLAGS[3] = (in_a & 0x8000 == in_b & 0x8000) && (in_a & 0x8000 != *REG_A & 0x8000);
                    break;

                case 0b0011:
                    in_a = *REG_A;
                    in_b = REG_B;

                    real_result = (int32_t) *REG_A + (int32_t) ~REG_B + 1;
                    temp = *REG_A + ~REG_B + 1;

                    FLAGS[0] = temp == 0;
                    FLAGS[1] = (temp & 0x8000) != 0;
                    FLAGS[2] = (real_result & 0x10000) != 0;
                    FLAGS[3] = (in_a & 0x8000 == in_b & 0x8000) && (in_a & 0x8000 != temp & 0x8000);
                    break;

                case 0b0100:
                    *REG_A = *REG_A | REG_B;

                    FLAGS[0] = *REG_A == 0;
                    FLAGS[1] = (*REG_A & 0x8000) != 0;
                    break;

                case 0b0101:
                    *REG_A = *REG_A ^ REG_B;

                    FLAGS[0] = *REG_A == 0;
                    FLAGS[1] = (*REG_A & 0x8000) != 0;
                    break;

                case 0b0110:
                    *REG_A = *REG_A & REG_B;

                    FLAGS[0] = *REG_A == 0;
                    FLAGS[1] = (*REG_A & 0x8000) != 0;
                    break;

                case 0b0111:
                    temp = *REG_A & REG_B;

                    FLAGS[0] = temp == 0;
                    FLAGS[1] = (temp & 0x8000) != 0;
                    break;

                case 0b1000:
                    *REG_A = REG_B;
                    break;

                case 0b1001:
                    *REG_A = REG_B;
                    break;

                case 0b1010:
                    *REG_A = memory[PAGE << 16 + REG_B];
                    break;

                case 0b1011:
                    if ((PAGE << 16) + REG_B == 0xC) {
                        SDL_SetRenderDrawColor(renderer, memory[0x4], memory[0x6], memory[0x8], memory[0xA]);
                        SDL_RenderDrawPoint(renderer, memory[0x0], memory[0x2]);
                    }
                    memory[PAGE << 16 + REG_B] = *REG_A;
                    break;

                case 0b1100:
                    *REG_A = (*REG_A << 5) | REG_B;

                case 0b1110:
                    switch (REG_B) {
                        case 0:
                            *REG_A = CPUID1;
                            break;
                        case 1:
                            *REG_A = CPUID2;
                            break;
                        case 2:
                            *REG_A = FEAT;
                            break;
                        case 65535:
                            *REG_A = PAGE;
                            break;
                        default:
                            break;
                    }
                    break;
                case 0b1111:
                    if (REG_B == 65535) {
                        PAGE = *REG_A;
                    }
                    break;
                default:
                    printf("Unknown OPCODE");
                    return -1;
            }
        } else if ((instruction & 0b1110000000000000) == 0b1000000000000000) {
            uint8_t opcode = (instruction & 0b0000111100000000) >> 8;
            int16_t displacement = instruction & 0b0000000011111111;
            bool displacement_sign = instruction & 0b0001000000000000;
            if (displacement_sign) { displacement *= -1; }

            switch (opcode) {
                case 0b0000:
                    counter += displacement * FLAGS[0];
                    break;

                case 0b0001:
                    counter += displacement * !FLAGS[0];
                    break;

                case 0b0010:
                    counter += displacement * FLAGS[1];
                    break;

                case 0b0011:
                    counter += displacement * !FLAGS[1];
                    break;

                case 0b0100:
                    counter += displacement * FLAGS[2];
                    break;

                case 0b0101:
                    counter += displacement * !FLAGS[2];
                    break;

                case 0b0110:
                    counter += displacement * FLAGS[3];
                    break;

                case 0b0111:
                    counter += displacement * !FLAGS[3];
                    break;

                case 0b1000:
                    counter += displacement * (FLAGS[0] || FLAGS[2]);
                    break;

                case 0b1001:
                    counter += displacement * !(FLAGS[0] || FLAGS[2]);
                    break;

                case 0b1010:
                    counter += displacement * (FLAGS[1] != FLAGS[3]);
                    break;

                case 0b1011:
                    counter += displacement * (FLAGS[1] == FLAGS[3]);
                    break;

                case 0b1100:
                    counter += displacement * (FLAGS[0] || (FLAGS[1] != FLAGS[3]));
                    break;

                case 0b1101:
                    counter += displacement * (!FLAGS[0] && (FLAGS[1] == FLAGS[3]));
                    break;

                case 0b1110:
                    counter += displacement;
                    break;

                default:
                    break;
            }
        }
        counter++;
        // SDL
        SDL_RenderPresent(renderer);
        int i = 0;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                SDL_DestroyRenderer(renderer);
                SDL_DestroyWindow(window);
                SDL_Quit();
                return 0;
            }
            if (event.type == SDL_KEYDOWN && i < 0x10) {
                memory[0x10 + i] = event.key.keysym.scancode;
                i++;
            }
        }
        // SDL
    }

    return 0;
}