#include <Project/CrcDriver.h>
#include <Project/FlashCheck.h>
#include <crc.h>
#include <cstdint>

namespace
{
// CRC_VAL will be placed at the end of used FLASH by the linker script
// crc-tool will calculate and replace value of CRC_VAL (last 4 bytes in .bin file)
// Location of CRC_VAL in flash will indicate end of used flash memory
std::uint32_t const __attribute__((section(".crc_section"))) __attribute__((__used__))
CRC_VAL = 105;
std::uint32_t const* const usedFlashBeginWord = reinterpret_cast<std::uint32_t*>(0x08000000);
std::uint32_t const* const usedFlashEndWord = &CRC_VAL;
constexpr std::uint32_t polynomial =
    79764919; // only polynomial in simple CRC peripherals, default one for u32
} // namespace

bool checkFlashCpp()
{
    // we want crc word as well: [firs,last)
    std::span flash(usedFlashBeginWord, usedFlashEndWord + 1);
    auto const crc = CrcDriver::instance().checkWord(flash); // C++
    return crc == 0;
}

// Short C implementation
static uint32_t crc32(uint32_t const* const begin, size_t size)
{
    __HAL_CRC_DR_RESET(&hcrc);
    return HAL_CRC_Accumulate(&hcrc, (uint32_t*)begin, size);
}

int checkFlashC()
{
    size_t const flashSize = (usedFlashEndWord - usedFlashBeginWord) + 1;
    uint32_t const crc = crc32(usedFlashBeginWord, flashSize); // C
    return !(crc == 0);
}
