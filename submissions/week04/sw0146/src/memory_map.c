#include "mission.h"

/* STM32F030R8 (DS9773 Rev 5) 기준. 크기는 Byte 단위이며 1 KiB = 1024 Byte. */
#define KIB(n)              ((uint32_t)(n) * 1024U)

#define FLASH_BASE_ADDR     0x08000000U   /* Table 2: Flash 64 KiB, Figure 10 */
#define FLASH_SIZE_BYTES    KIB(64)       /* 끝 주소 0x0800FFFF */
#define SRAM_BASE_ADDR      0x20000000U   /* Table 2: SRAM 8 KiB, Figure 10 */
#define SRAM_SIZE_BYTES     KIB(8)        /* 끝 주소 0x20001FFF */
#define GPIOA_BASE_ADDR     0x48000000U   /* Table 17: AHB2 GPIOA */
#define GPIOA_SIZE_BYTES    KIB(1)        /* 끝 주소 0x480003FF */
#define RCC_BASE_ADDR       0x40021000U   /* Table 17: AHB1 RCC */
#define RCC_SIZE_BYTES      KIB(1)        /* 끝 주소 0x400213FF */

typedef struct {
    uint32_t base;//시작점
    uint32_t size;//메모리 section size
    MemoryRegion region;
} RegionRange;

static const RegionRange k_regions[] = {
    { FLASH_BASE_ADDR, FLASH_SIZE_BYTES, MEMORY_FLASH },
    { SRAM_BASE_ADDR,  SRAM_SIZE_BYTES,  MEMORY_SRAM  },
    { GPIOA_BASE_ADDR, GPIOA_SIZE_BYTES, MEMORY_GPIOA },
    { RCC_BASE_ADDR,   RCC_SIZE_BYTES,   MEMORY_RCC   },
};

/* [base, base + size - 1] 양 끝 포함 판정.
   base + size를 계산하지 않고 offset을 비교해 0xFFFFFFFF 근처에서도
   uint32_t overflow 없이 동작한다. address < base이면 offset이 크게
   wrap-around 되어 자동으로 범위 밖이 된다. */
static bool in_range(uint32_t address, uint32_t base, uint32_t size)
{
    return (size != 0U) && ((address - base) <= (size - 1U));
}

MemoryRegion memory_region(uint32_t address)
{
    /* 주소 숫자만 비교한다. Pointer 변환이나 역참조는 하지 않는다. */
    for (uint32_t i = 0U; i < (uint32_t)(sizeof k_regions / sizeof k_regions[0]); ++i) {
        if (in_range(address, k_regions[i].base, k_regions[i].size)) {
            return k_regions[i].region;
        }
    }
    return MEMORY_UNKNOWN;
}
