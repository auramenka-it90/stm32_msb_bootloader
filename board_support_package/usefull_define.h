/* --- START OF FILE usefull_define.h --- */

#ifndef USEFULL_DEFINE_H_
#define USEFULL_DEFINE_H_


#include <stdint.h>
#include <stdbool.h>
#include <math.h>

#define         ON      true
#define         OFF     false


#define         B0      (1U<<0)
#define         B1      (1U<<1)
#define         B2      (1U<<2)
#define         B3      (1U<<3)
#define         B4      (1U<<4)
#define         B5      (1U<<5)
#define         B6      (1U<<6)
#define         B7      (1U<<7)
#define         B8      (1U<<8)
#define         B9      (1U<<9)
#define         B10     (1U<<10)
#define         B11     (1U<<11)
#define         B12     (1U<<12)
#define         B13     (1U<<13)
#define         B14     (1U<<14)
#define         B15     (1U<<15)
#define         B16     (1U<<16)
#define         B17     (1U<<17)
#define         B18     (1U<<18)
#define         B19     (1U<<19)
#define         B20     (1U<<20)
#define         B21     (1U<<21)
#define         B22     (1U<<22)
#define         B23     (1U<<23)
#define         B24     (1U<<24)
#define         B25     (1U<<25)
#define         B26     (1U<<26)
#define         B27     (1U<<27)
#define         B28     (1U<<28)
#define         B29     (1U<<29)
#define         B30     (1U<<30)
#define         B31     (1U<<31)


#define         _PI     3.14159265359f
#define         _2PI    (2.0f * _PI)
#define         _PI2    (_PI / 2.0f)


/* ========================================================================= */
/*                           ANGLE UNIT CONVERSIONS                          */
/* ========================================================================= */

/* Radians -> Degrees */
#define         _RAD_2_DEGREE       (180.0f / _PI)

/* Degrees -> Radians */
#define         _DEGREE_2_RAD       (_PI / 180.0f)


#define         _ANGLE_1_SEC        (_PI / (180.0f * 3600.0f))
#define         _ANGLE_10_SEC       (10.0f * _ANGLE_1_SEC)
#define         _ANGLE_30_SEC       (30.0f * _ANGLE_1_SEC)
#define         _ANGLE_40_SEC       (40.0f * _ANGLE_1_SEC)


#define         _ANGLE_1_MIN        (60.0f * _ANGLE_1_SEC)
#define         _ANGLE_5_MIN        (5.0f * _ANGLE_1_MIN)
#define         _ANGLE_10_MIN       (10.0f * _ANGLE_1_MIN)
#define         _ANGLE_30_MIN       (30.0f * _ANGLE_1_MIN)


#define         _ANGLE_1_DEGREE     (_PI / 180.0f)
#define         _ANGLE_5_DEGREE     (5.0f * _ANGLE_1_DEGREE)
#define         _ANGLE_10_DEGREE    (10.0f * _ANGLE_1_DEGREE)
#define         _ANGLE_15_DEGREE    (15.0f * _ANGLE_1_DEGREE)
#define         _ANGLE_20_DEGREE    (20.0f * _ANGLE_1_DEGREE)
#define         _ANGLE_30_DEGREE    (30.0f * _ANGLE_1_DEGREE)
#define         _ANGLE_45_DEGREE    (45.0f * _ANGLE_1_DEGREE)
#define         _ANGLE_90_DEGREE    (90.0f * _ANGLE_1_DEGREE)
#define         _ANGLE_180_DEGREE   _PI

/* Radians -> Degrees alternative macros */
#define         RAD_TO_DEG(rad)     ((rad) * _RAD_2_DEGREE)
#define         DEG_TO_RAD(deg)     ((deg) * _DEGREE_2_RAD)

/* Radians -> Arcseconds */
#define         RAD_TO_SEC(rad)     (((rad) * (180.0f * 3600.0f)) / _PI)
#define         SEC_TO_RAD(sec)     ((sec) * _ANGLE_1_SEC)

/* Radians -> Arcminutes */
#define         RAD_TO_MIN(rad)     (((rad) * (180.0f * 60.0f)) / _PI)
#define         MIN_TO_RAD(min)     ((min) * _ANGLE_1_MIN)

/* Angle normalization to range [0, 2pi) и [-pi, pi) */
#define         NORMALIZE_2PI(rad)  (fmodf((rad), _2PI))
#define         NORMALIZE_PI(rad)   (fmodf((rad) + _PI, _2PI) - _PI)


typedef union {
    float       f;
    uint32_t    u32;
    uint16_t    u16[2];
    uint8_t     u8[4];
} t_float_uint;


#endif /* USEFULL_DEFINE_H_ */

/* --- END OF FILE usefull_define.h --- */
