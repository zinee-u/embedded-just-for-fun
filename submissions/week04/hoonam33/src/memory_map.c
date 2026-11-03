#include "mission.h"

MemoryRegion memory_region(uint32_t address)
{
    /* TODO M1: Datasheet 근거로 Flash/SRAM/GPIOA/RCC 경계를 반영하세요.
       단일 주소 숫자만 분류합니다. Pointer 변환이나 역참조는 금지합니다. */


    if(address >= 0x08000000 && address < 0x08010000) {
        return MEMORY_FLASH;
    } else if(address >= 0x20000000 && address < 0x20002000) {
        return MEMORY_SRAM;
    } else if(address >= 0x48000000 && address < 0x48000400) {
        return MEMORY_GPIOA;
    } else if(address >= 0x40021000 && address < 0x40021400) {
        return MEMORY_RCC;
    }
    
    (void)address;
    return MEMORY_UNKNOWN;
}
