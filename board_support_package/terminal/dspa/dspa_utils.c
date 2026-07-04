#include <string.h>
#include "dspa_utils.h"

typedef union
{
    du32  _u32;
    du16  _u16[2];
    du8   _u8[4];
    float _float;
} int2_long_float_t;

//------------------------------------------------------------------------------------------
int string_to_buf(char *const string, du8 *const buf)
{
    int len, i;
    len = strlen(string);
    for (i = 0; i < len; i++) buf[i] = string[i];
    buf[len] = 0x00;//zero-ending
    return len + 1;
}
//------------------------------------------------------------------------------------------
int u16_to_buf(const du16 value, du8 *const buf)
{
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;

    return 2;
}
//------------------------------------------------------------------------------------------
du16 u16_from_buf(du8 *const  buf)
{
    du16 tmp = buf[0];
    tmp |= (buf[1] << 8) & 0xFF00;

    return tmp;
}
//------------------------------------------------------------------------------------------
int u32_to_buf(const du32 value, du8 *const buf)
{
    buf[0] = value & 0xFF;
    buf[1] = (value >> 8) & 0xFF;
    buf[2] = (value >> 16) & 0xFF;
    buf[3] = (value >> 24) & 0xFF;

    return 4;
}
//------------------------------------------------------------------------------------------
du32 u32_from_buf(du8 *const buf)
{
    du32 tmp = buf[0];

    tmp |= (buf[1] << 8) & 0xFF00;
    tmp |= (buf[2] << 16) & 0xFF0000;
    tmp |= (buf[3] << 24) & 0xFF000000;

    return tmp;
}
//------------------------------------------------------------------------------------------
int float_to_buf(const float value, du8 *const buf)
{
    int2_long_float_t tmp;

    tmp._float = value;

    buf[0] = tmp._u8[0];
    buf[1] = tmp._u8[1];
    buf[2] = tmp._u8[2];
    buf[3] = tmp._u8[3];

    return 4;
}
//------------------------------------------------------------------------------------------
float float_from_buf(du8 *const buf)
{
    int2_long_float_t tmp;

    tmp._u8[0] = buf[0];
    tmp._u8[1] = buf[1];
    tmp._u8[2] = buf[2];
    tmp._u8[3] = buf[3];

    return tmp._float;
}
