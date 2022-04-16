#include <Project/CrcDriver.h>
#include <crc.h>

namespace
{
constexpr auto crcHandle = &hcrc;
}

auto CrcDriver::instance() -> CrcDriver&
{
    static CrcDriver instance;
    return instance;
}

template <typename T>
static std::uint32_t checkCRC(std::span<T> const data, std::uint32_t initialValue = 0xffffffff)
{
    // Some CRC peripherals support setting the initial values with
    // __HAL_CRC_INITIALCRCVALUE_CONFIG(crcHandle, initialValue); Simple ones, like f407,
    // don't, but they all support DR_RESET
    __HAL_CRC_DR_RESET(crcHandle); // initial value == 0xffffffff

    // Some CRC peripheral will take std::uint32_t*, but will work with different data size
    // defined in initialisation: (7,8,16,32)bit Simple others, like f407, work only with u32
    // with no setup required.

    // Cast any pointer to u32, remove const because hal sucks :P
    auto cbegin = reinterpret_cast<std::uint32_t const*>(data.data());
    auto begin = const_cast<std::uint32_t*>(cbegin);
    return HAL_CRC_Accumulate(crcHandle, begin, data.size());
}

std::uint32_t CrcDriver::checkWord(
    std::span<std::uint32_t const> data, std::uint32_t initialValue) const
{
    return checkCRC(data, initialValue);
}

std::uint32_t CrcDriver::checkByte(
    std::span<char const> data, std::uint32_t initialValue) const
{
    return checkCRC(data, initialValue);
}
