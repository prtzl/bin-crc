#pragma once

#include <cstdint>
#include <span>

// Uses built-in CRC peripheral
// CRC is reset before check with initial value 0xffffffff
class CrcDriver
{
public:
    CrcDriver(CrcDriver const&) = delete;
    CrcDriver(CrcDriver&&) = delete;
    CrcDriver& operator=(CrcDriver const&) = delete;
    CrcDriver& operator=(CrcDriver&&) = delete;

    static auto instance() -> CrcDriver&;
    std::uint32_t checkWord(
        std::span<std::uint32_t const> data, std::uint32_t initialValue = 0xffffffff) const;
    std::uint32_t checkByte(
        std::span<char const> cdata, std::uint32_t initialValue = 0xffffffff) const;

private:
    CrcDriver() = default;
};
