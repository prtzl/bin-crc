#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <span>
#include <vector>

// Check your CRC implementation
constexpr std::uint32_t crcInitialValue = 0xffffffff;
constexpr std::uint32_t crcPolynomial = 0x04C11DB7;

// CRC32 calculation compatible with the STM32 HW peripheral
static std::uint32_t STM_crc_32_word(std::uint32_t crc, std::uint32_t data)
{
    crc = crc ^ data;
    for (std::uint32_t i = 0; i < 32; i++)
    {
        if (crc & 0x80000000)
        {
            crc = (crc << 1) ^ crcPolynomial;
        }
        else
        {
            crc = (crc << 1);
        }
    }
    return crc;
}

// CRC32 calculation for a word buffer
static std::uint32_t crc32(
    std::span<std::uint32_t const> const data, std::uint32_t const initialValue)
{
    std::uint32_t outcrc = initialValue;
    for (auto const& word : data)
    {
        outcrc = STM_crc_32_word(outcrc, word);
    }
    return outcrc;
}

// Transforms word to byte array, little endian (lsb = array[0], msb = array[3])
static constexpr auto wordToBytesLittle(std::uint32_t const word) -> std::array<char, 4>
{
    return {
        static_cast<char>(word & 0x000000ff),
        static_cast<char>((word >> 8) & 0x000000ff),
        static_cast<char>((word >> 16) & 0x000000ff),
        static_cast<char>((word >> 24) & 0x000000ff)};
}

// Transforms bytes to word, little endian
static auto bytesToWordLittle(char const b1, char const b2, char const b3, char const b4)
    -> std::uint32_t
{
    std::uint32_t const u1 = *reinterpret_cast<unsigned char const*>(&b1);
    std::uint32_t const u2 = *reinterpret_cast<unsigned char const*>(&b2);
    std::uint32_t const u3 = *reinterpret_cast<unsigned char const*>(&b3);
    std::uint32_t const u4 = *reinterpret_cast<unsigned char const*>(&b4);
    return u1 | (u2 << 8) | (u3 << 16) | (u4 << 24);
}

// Helper functions for file system
namespace fs = std::filesystem;
static bool is_file(fs::path fpath)
{
    auto const status = fs::status(fpath).type();
    if (status != fs::file_type::regular)
    {
        return false;
    }
    return true;
}

static bool is_writable(fs::path fpath)
{
    auto const p = fs::status(fpath).permissions();
    return ((p & fs::perms::owner_write) != fs::perms::none)
        || ((p & fs::perms::group_write) != fs::perms::none)
        || ((p & fs::perms::others_write) != fs::perms::none);
}

static bool is_readable(fs::path fpath)
{
    auto const p = fs::status(fpath).permissions();
    return ((p & fs::perms::owner_read) != fs::perms::none)
        || ((p & fs::perms::group_read) != fs::perms::none)
        || ((p & fs::perms::others_read) != fs::perms::none);
}

int main(int argc, char** argv)
{
    if (argc < 3)
    {
        std::cerr << "Invalid input arguments: <path to intput.bin> <path to output.bin> "
                     "<optional 'c': corrupt crc result>\n";
        return 1;
    }

    // Get all inputs
    fs::path inputBinPath{argv[1]};
    fs::path outputBinPath{argv[2]};
    bool const corruptBin = [&argc, &argv]()
    {
        if (argc > 3)
        {
            if (*argv[3] == 'c')
            {
                return true;
            }
        }
        return false;
    }();

    // Check input and output file destinations
    if (!is_file(inputBinPath))
    {
        std::cerr << "Invalid input file path: " << inputBinPath.string() << '\n';
        return 1;
    }
    else if (!is_readable(inputBinPath))
    {
        std::cerr << "Input file is not readable!\n";
        return 1;
    }
    if (!is_writable(outputBinPath))
    {
        std::cerr << "Output file is not writable!\n";
        return 1;
    }

    // Read binary
    std::ifstream inputFile(inputBinPath, std::ios::binary);
    if (!inputFile.is_open())
    {
        std::cerr << "Could not open file!\n";
        return 1;
    }
    std::vector<char> const inputBytes(
        (std::istreambuf_iterator<char>(inputFile)), (std::istreambuf_iterator<char>()));
    inputFile.close();

    // It has to be aligned to 4B and non-empty
    if (inputBytes.empty())
    {
        std::cerr << "Input binary is empty!\n";
        return 1;
    }
    else if (inputBytes.size() % 4)
    {
        std::cerr << "Input binary is not aligned to 4B!\n";
        return 1;
    }

    // Copy original bytes to word buffer for calculation
    std::size_t const flashWordSize = inputBytes.size() / 4;
    std::vector<std::uint32_t> flashWords(flashWordSize);
    for (std::size_t i = 0; i < flashWords.size(); ++i)
    {
        flashWords[i] = bytesToWordLittle(
            inputBytes[i * 4],
            inputBytes[(i * 4) + 1],
            inputBytes[(i * 4) + 2],
            inputBytes[(i * 4) + 3]);
    }

    // Calculate CRC on the whole memory up until CRC word and place result on last
    // word in buffer
    auto const crcval =
        crc32(std::span{flashWords.begin(), flashWords.size() - 1}, crcInitialValue);
    std::cout << "CRC32:" << crcval << '\n';
    flashWords.back() = crcval; // Place crc at the end of memory

    // Corrupt file for device testing
    if (corruptBin)
    {
        std::cout << "Corrupted one before last word!\n";
        flashWords.back() += 105; // change CRC to something else
    }
    else
    {
        // Check if crc passes
        if (auto const crc = crc32(std::span{flashWords.begin(), flashWords.size()}, crcInitialValue))
        {
            std::cerr << "Crc check failes, got result: " << crc << '\n';
            return 1;
        }
    }

    // Convert output words into output bytes
    std::vector<char> outputBytes(inputBytes.size());
    for (std::size_t i = 0; i < flashWords.size(); ++i)
    {
        auto const bytes = wordToBytesLittle(flashWords[i]);
        outputBytes[(i * 4)] = bytes[0];
        outputBytes[(i * 4) + 1] = bytes[1];
        outputBytes[(i * 4) + 2] = bytes[2];
        outputBytes[(i * 4) + 3] = bytes[3];
    }

    // Check if original heads of byte streams match (all but last 4 bytes - crc)
    for (std::size_t i = 0; i < inputBytes.size() - 4; ++i)
    {
        if (inputBytes[i] != outputBytes[i])
        {
            std::cerr << "Program byte streams do not match!\n";
            return 1;
        }
    }

    // Save bytes to new bin file
    std::ofstream outputFile(outputBinPath);
    if (!outputFile.is_open())
    {
        std::cerr << "Could not open output file\n";
        return 1;
    }
    for (auto const& byte : outputBytes)
    {
        outputFile << byte;
    }
    outputFile.close();

    std::cout << "New binary: " << outputBinPath.string() << " saved!\n";
}
