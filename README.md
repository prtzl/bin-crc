# STM32 CRC-32 patcher

This project takes a binary file (suffix `.bin`) calculates CRC-32 with a given polynomial, the same way as the STM32 CRC HW peripheral does, for the entire bin file up to last four bytes. Calculated crc value is then copied on the last four bytes. Byte to word conversion uses little endian sequence.  

On hardware, CRC check with the same polynomial should run over the entire used flash section, including crc value (last word). Result should be 0, if the flash is un-compromised.  

## Workflow

Go into `./bin-crc`.  
Run `make` to compile the tool.  
Executable will be placed into `./bin-crc/build/bin-crc`.  

## Usage

To create a modified binary execute the tool with the following arguments:

```shell
./bin-crc/build/bin-crc <path to intput.bin> <path to output.bin> <optional 'c': corrupt 5th byte from the back>
```

If optional letter 'c' as 3rd argument is provided, calculated crc is corrupted. This is to test CRC checking on device end.

## Example

Example is made for `STM32F407VG` using my [example project](https://github.com/prtzl/stm32-cmake).  

```shell
# In ./firmware
make # build STM32 binary

# In ./bin-crc
make # builds the tool

./bin-crc/build/bin-crc ./firmware/build/firmware.bin ./firmware-crc.bin # creates new binary with crc
./bin-crc/build/bin-crc ./firmware/build/firmware.bin ./firmware-crc-bad.bin c # creates new binary with corrupted crc
```

To view the last part of the binary you can use `hexdump`:

```shell
hexdump -C ./firmware-crc<-bad>.bin | less
```

## How it works

### bin-crc

Calculating CRC-32 of the used flash is quite easy. The program takes the binary and converts it into a vector of words - packing 4 bytes together (little endian). CRC is then calculated on the new vector of words up until the last word, where the result is saved to. The word vector is converted back to a vector of bytes and saved into a new `.bin` file.  

If the flash is uncorrupted, calculating CRC over now entire bin file, the result should be 0. This is also checked in the program.  

### STM32

On STM32, CRC peripheral is used. The one on `STM32F407VG` is simple and only calculates words with a fixed polynomial `0x04C11DB7`. Some peripherals have more adjustments; for example I have implemented this on `STM32F303CC` as well, where the peripheral offers the choice of polynomial, data in and out size as well, but the default 32bit polynomial is the same as here.  

The checking is implemented in `./firmware/Project/FlashCheck.cpp`, where you will find C and C++ implementations. The important part is on the top of the file, where three constants are defined.  
`CRC_VAL` is special, because it includes linker directive telling it to be placed into a section named `.crc-section`. Normally, a constant would be placed into `.rodata` section. I have modified the [linker script](./firmware/CubeMX/STM32F407VGTx_FLASH.ld) to include section `.crc-section` at the end of flash, which is important. This will guarantee, that the variable will be placed at the end of flash. Do not use this section for anything else than this one constant.  

Next, two 32bit pointers are declared. First called `usedFlashBeginWord` points to the beginning of the flash, located on most STM32 at `0x08000000`. Next called `usedFlashEndWord` points to the location of `CRC_VAL` variable, which is stored at the end of flash. Using peripheral HAL function, CRC is calculated for the entire flash region. Check is successful, if the result is 0.
