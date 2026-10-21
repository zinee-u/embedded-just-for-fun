#include "mission.h"

MemoryRegion memory_region(uint32_t address)
{
    /* TODO M1: Datasheet 근거로 Flash/SRAM/GPIOA/RCC 경계를 반영하세요.
       단일 주소 숫자만 분류합니다. Pointer 변환이나 역참조는 금지합니다. */
    /*
    (void)address;
    return MEMORY_UNKNOWN; => 어느 주소가 들어오더라도 MEMORY_UNKOWN을 반환
    */
    /* Flash: 64 KiB */
    if (address >= 0x08000000U && address <= 0x0800FFFFU) {
        return MEMORY_FLASH;
    }

    /* SRAM: 8 KiB */
    if (address >= 0x20000000U && address <= 0x20001FFFU) {
        return MEMORY_SRAM;
    }

    /* GPIOA: 1 KiB */
    if (address >= 0x48000000U && address <= 0x480003FFU) {
        return MEMORY_GPIOA;
    }

    /* RCC: 1 KiB */
    if (address >= 0x40021000U && address <= 0x400213FFU) {
        return MEMORY_RCC;
    }

    return MEMORY_UNKNOWN;
}
