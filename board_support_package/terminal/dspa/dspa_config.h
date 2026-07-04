#ifndef CONFIG_H_
#define CONFIG_H_

#define DSPA_PROT_VERSION 3

/* C++ environment required. */
#define DSPA_NEWLIB_FEATURE_NEEDED 1

/* replace legacy signal descriptors macros with cpp methods. */
#define DSPA_SIG_NOMACRO_FEATURE_NEEDED 1

/* replace legacy signals macros with corresponding cpp methods. */
#define DSPA_STRICT_SIGNALS_FEATURE_NEEDED 1

/* enable auto-type signal cpp method. */
#define DSPA_AUTO_SIGNALS_FEATURE_NEEDED 1

/* replace legacy attributes enumeration with cpp class. */
#define DSPA_SIG_ATTR_ENUM_FEATURE_NEEDED 1

#if defined(__cplusplus) && __cplusplus
#define DSPA_NEWLIB_FEATURE_ENABLED DSPA_NEWLIB_FEATURE_NEEDED
#else
#define DSPA_NEWLIB_FEATURE_ENABLED 0
#endif

#if DSPA_NEWLIB_FEATURE_ENABLED
#define DSPA_SIG_NOMACRO_FEATURE_ENABLED DSPA_SIG_NOMACRO_FEATURE_NEEDED
#define DSPA_STRICT_SIGNALS_FEATURE_ENABLED DSPA_STRICT_SIGNALS_FEATURE_NEEDED
#define DSPA_AUTO_SIGNALS_FEATURE_ENABLED DSPA_AUTO_SIGNALS_FEATURE_NEEDED
#define DSPA_SIG_ATTR_ENUM_FEATURE_ENABLED DSPA_SIG_ATTR_ENUM_FEATURE_NEEDED
#else
#define DSPA_SIG_NOMACRO_FEATURE_ENABLED 0
#define DSPA_STRICT_SIGNALS_FEATURE_ENABLED 0
#define DSPA_AUTO_SIGNALS_FEATURE_ENABLED 0
#define DSPA_SIG_ATTR_ENUM_FEATURE_ENABLED 0
#endif /* DSPA_NEWLIB_FEATURE_ENABLED */

#if __has_include("FreeRTOS.h") && __has_include("task.h")

#include "FreeRTOS.h"
#include "task.h"
#define _CRITICAL_BASETYPE UBaseType_t
#define _ENTER_CRITICAL taskENTER_CRITICAL();
#define _EXIT_CRITICAL taskEXIT_CRITICAL();
#define _ENTER_CRITICAL_FROM_ISR taskENTER_CRITICAL_FROM_ISR();
#define _EXIT_CRITICAL_FROM_ISR(X) taskEXIT_CRITICAL_FROM_ISR(X);

#elif __has_include("builtins.h")

#include "builtins.h"
#define _CRITICAL_BASETYPE unsigned int
#define _ENTER_CRITICAL _CRITICAL_BASETYPE tmp_cli = cli();
#define _EXIT_CRITICAL sti(tmp_cli);
#define _ENTER_CRITICAL_FROM_ISR cli()
#define _EXIT_CRITICAL_FROM_ISR(X) sti(X)

#endif

#ifndef _CRITICAL_BASETYPE
THR_MESSAGE("_CRITICAL_BASETYPE not defined.");
#define _CRITICAL_BASETYPE int
#endif /* _CRITICAL_BASETYPE */

#ifndef _ENTER_CRITICAL
THR_MESSAGE("_ENTER_CRITICAL not defined.");
#define _ENTER_CRITICAL
#endif

#ifndef _EXIT_CRITICAL
THR_MESSAGE("_EXIT_CRITICAL not defined.");
#define _EXIT_CRITICAL
#endif

#ifndef _ENTER_CRITICAL_FROM_ISR
THR_MESSAGE("_ENTER_CRITICAL_FROM_ISR not defined.");
#define _ENTER_CRITICAL_FROM_ISR 0
#endif

#ifndef _EXIT_CRITICAL_FROM_ISR
THR_MESSAGE("_EXIT_CRITICAL_FROM_ISR not defined.");
#define _EXIT_CRITICAL_FROM_ISR(X)
#endif

// Signals
#define SIG_NUM_SIGNALS_ARR 16 // amount of signal arrays
#define SIG_MAX_SIGNALS_2_CONTROL 16
#define SIG_DEFAULT_PERIOD (100 /*us*/ * 100)

// Telemetry
#define TEL_BUFFER_SIZE (1024 * 8)
#define TEL_MAX_SIGNALS 16
#define TEL_SIZE_SAMPLE_BUFFER (TEL_MAX_SIGNALS * 4)

// Dispatcher
#define DSPA_DISPATCHER_RX_BUF_SIZE (1024 * 2)
#define DSPA_DISPATCHER_TX_BUF_SIZE (1024 * 8)
#define DISPATCHER_TIMEOUT_MS 100 /*ms*/

// General
#define ENABLE_THREAD_SAFE // enable thread safe logic (cli/sti logic)

#ifdef ENABLE_THREAD_SAFE
#define DSPA_ENTER_CRITICAL _ENTER_CRITICAL
#define DSPA_ENTER_CRITICAL_FROM_ISR _ENTER_CRITICAL_FROM_ISR
#define DSPA_EXIT_CRITICAL _EXIT_CRITICAL
#define DSPA_EXIT_CRITICAL_FROM_ISR(X) _EXIT_CRITICAL_FROM_ISR(X)
#else
#define DSPA_ENTER_CRITICAL
#define DSPA_ENTER_CRITICAL_FROM_ISR
#define DSPA_EXIT_CRITICAL
#define DSPA_EXIT_CRITICAL_FROM_ISR(X)
#endif

#endif /* CONFIG_H_ */
