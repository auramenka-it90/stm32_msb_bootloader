#ifndef DSPA_UTILS_H_
#define DSPA_UTILS_H_

#include "dspa_config.h"
#include "dspa.h"
#include "freertos.h"
#include "task.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int   string_to_buf  (char *const string, du8 *const buf);
int   u16_to_buf     (const du16 value, du8* const buf);
du16  u16_from_buf   (du8 *const buf);
int   u32_to_buf     (const du32 value, du8 *const  buf);
du32  u32_from_buf   (du8 *const buf);
int   float_to_buf   (const float value, du8 *const buf);
float float_from_buf (du8 *const buf);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* DSPA_UTILS_H_ */
