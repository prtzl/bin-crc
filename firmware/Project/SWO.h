#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
#endif

    void SWO_PrintChar(char const c, uint8_t const portNo);
    void SWO_PrintString(char const* s, uint8_t const portNumber);
    void SWO_Print(char const* str);

#ifdef __cplusplus
}
#endif