#ifndef DSPA_SIGDEFS_H_
#define DSPA_SIGDEFS_H_

// Standard includes
#include <float.h>
#include <stddef.h>
#ifdef __cplusplus

// Standard C++ includes
#include <cfloat>
#include <limits>
#include <type_traits>

#endif /* __cplusplus */

// Project includes
#include "dspa.h"
#include "dspa_config.h"
#include "dspa_signals.h"

#define DEFAULT_FLT_MANT_DIG 24
#define DEFAULT_DBL_MANT_DIGIT 53

/// @brief Signal attribute enumeration.
typedef enum SIG_Attribute
{
	/* legacy enumeration tokens */

	/// @brief No attributes. Default value.
	SIG_ATTR_NONE = 0,
	/// @brief Allows writing value to the signal.
	SIG_ATTR_CONTROL = (1UL << 0),
	/// @brief Unused.
	SIG_ATTR_NODE = (1UL << 1),
	/// @brief Unused.
	SIG_ATTR_SIGNATURE = (1UL << 2),
	/// @brief Allows reading value from the signal.
	SIG_ATTR_READ = (1UL << 3),
	/// @brief Signal value used in SaveSettings routine.
	SIG_ATTR_SETTING = (1UL << 4),
	/// @brief Allows passing signal value to the telemerty routine.
	SIG_ATTR_TELEMETRY = (1UL << 5),
} SIG_Attribute_t;

#ifdef __cplusplus
#define SIG_DSCR_NAME(X) const_cast<char *>(X)
#define SIG_DSCR_VALUE(X) (void *)(X)
#else
#define SIG_DSCR_NAME(X) X
#define SIG_DSCR_VALUE(X) X
#endif /* __cplusplus */

#define SIG_PARENT_VALUE(...) #__VA_ARGS__

#define SIG_ROOT NULL

#define SIG_PARENT(LITERAL, ...) static char LITERAL[] = SIG_PARENT_VALUE(__VA_ARGS__)
#define SIG_PARENT_PTR(LITERAL) static char *LITERAL##_ptr = LITERAL

#define SIG_NAME_ARR(name) arr_##name
#define SIG_NAME_CNT(name) cnt_##name

#define SIGNALS_BEGIN(name) static Signal_t SIG_NAME_ARR(name)[] = {
#define SIGNALS_END(name)                          \
	}                                              \
	;                                              \
	static const unsigned int SIG_NAME_CNT(name) = \
		(sizeof(SIG_NAME_ARR(name))) / (sizeof(Signal_t));

#define SIG_INIT(name) SIG_Init(SIG_NAME_ARR(name), SIG_NAME_CNT(name))

#if DSPA_NEWLIB_FEATURE_ENABLED

#define DSPA_USE_SIGNAL_TEMPLATES 1

#define DSPA_TEMPLATE_TYPE T

#if DSPA_USE_SIGNAL_TEMPLATES
#define GENERIC_SIGNAL_VALUETYPE DSPA_TEMPLATE_TYPE
#define GENERIC_SIGNAL_TEMPLATE template <class DSPA_TEMPLATE_TYPE>
#else
/*
 * use of 'auto' in parameter declaration only available with '-std=c++20' or '-fconcepts'
 */
#define GENERIC_SIGNAL_VALUETYPE auto
#define GENERIC_SIGNAL_TEMPLATE
#endif /* DSPA_USE_SIGNAL_TEMPLATES */

#endif /* DSPA_NEWLIB_FEATURE_ENABLED */

#if DSPA_SIG_NOMACRO_FEATURE_ENABLED

/* Private functions */

__nodiscard __always_inline __constexpr static SIG_Descriptor GenericDescriptor(
	const char *const name,
	const SIG_Type_t type,
	const du32 attributes)
{
	return {
		SIG_DSCR_NAME(name),
		SCL_STD,
		type,
		attributes,
		(du16)~0,
		SIG_DEFAULT_PERIOD,
		SMT_NONE,
		1.0f,
	};
}

GENERIC_SIGNAL_TEMPLATE
__nodiscard __always_inline __constexpr static Signal_t GenericSignal(
	const char *const name,
	const SIG_Type_t type,
	const du32 attributes,
	GENERIC_SIGNAL_VALUETYPE &Value,
	void *parent)
{
	return {
		GenericDescriptor(name, type, +attributes),
		SIG_DSCR_VALUE(parent),
		SIG_DSCR_VALUE(&Value),
		0,
		SCT_NONE,
		invalid_handle,
		0,
	};
}

#else

#define GenericDescriptor(name, type, attributes) { \
	SIG_DSCR_NAME(name), /*name*/                   \
	SCL_STD,			 /*structure*/              \
	type,				 /*type*/                   \
	(du32)attributes,	 /*attributes*/             \
	(du16)~0,			 /*parent_node*/            \
	SIG_DEFAULT_PERIOD,	 /*period*/                 \
	SMT_NONE,			 /*meas_physics*/           \
	1.0f				 /*coefficient*/            \
}

#define GenericSignal(name, type, attr, Value, parent) { \
	GenericDescriptor(name, type, attr),                 \
	SIG_DSCR_VALUE(parent), /*pParentSignal*/            \
	SIG_DSCR_VALUE(&Value), /*pValue*/                   \
	0,						/*Value_size*/               \
	SCT_NONE,				/*ctrl*/                     \
	invalid_handle,			/*handle*/                   \
	0						/*tel_sample_offset*/        \
}

#endif /* DSPA_SIG_NOMACRO_FEATURE_ENABLED */

#if DSPA_SIG_ATTR_ENUM_FEATURE_ENABLED
/// @brief Cpp-style signal attribute enumeration.
/// @sa SIG_Attribute
typedef enum class SIG_Attr : du32
{
	/* canonical enumeration tokens */

	/// @see SIG_Attribute#SIG_ATTR_NONE
	NONE = SIG_ATTR_NONE,

	/// @see SIG_Attribute#SIG_ATTR_READ
	R = (SIG_ATTR_READ),
	/// @see SIG_Attribute#SIG_ATTR_CONTROL
	W = (SIG_ATTR_CONTROL),
	/// @see SIG_Attribute#SIG_ATTR_TELEMETRY
	T = (SIG_ATTR_TELEMETRY),
	/// @see SIG_Attribute#SIG_ATTR_SETTING
	S = (SIG_ATTR_SETTING),

	/* combined enumeration tokens */

	RW = (R | W),
	TR = (T | R),
	TRW = (TR | W)
} SIG_Attr_t;

typedef SIG_Attr_t SIG_ATTR_TYPE;
#define SIG_ATTR_N SIG_Attr_t::NONE
#define SIG_ATTR_R SIG_Attr_t::R
#define SIG_ATTR_RW SIG_Attr_t::RW
#define SIG_ATTR_T SIG_Attr_t::T
#define SIG_ATTR_S SIG_Attr_t::S
#define SIG_ATTR_TR SIG_Attr_t::TR
#define SIG_ATTR_TRW SIG_Attr_t::TRW

#else

typedef SIG_Attribute_t SIG_ATTR_TYPE;
#define SIG_ATTR_N SIG_ATTR_NONE
#define SIG_ATTR_R SIG_ATTR_READ
#define SIG_ATTR_RW (SIG_ATTR_READ | SIG_ATTR_CONTROL)
#define SIG_ATTR_T SIG_ATTR_TELEMETRY
#define SIG_ATTR_S SIG_ATTR_SETTING
#define SIG_ATTR_TR (SIG_ATTR_T | SIG_ATTR_R)
#define SIG_ATTR_TRW (SIG_ATTR_T | SIG_ATTR_RW)

#endif /* DSPA_SIG_ATTR_ENUM_FEATURE_ENABLED */

#if FLT_MANT_DIG == DEFAULT_FLT_MANT_DIG
#define stype_float STYPE_FLOAT
#else
THR_WARNING("sizeof(float) is less than 4 bytes. Float signals are not allowed.");
#endif

#if DBL_MANT_DIG == DEFAULT_DBL_MANT_DIGIT
#define stype_double STYPE_DOUBLE
#elif DBL_MANT_DIG == DEFAULT_FLT_MANT_DIG
THR_MESSAGE("sizeof(double) is 4 bytes. Double signals assumed as Float signals.");
#define stype_double STYPE_FLOAT
#else
THR_MESSAGE("sizeof(double) is nor 4 and nor 8 bytes. Double signals are not allowed.");
#endif

#if LDBL_MANT_DIG == DEFAULT_DBL_MANT_DIGIT
#define stype_long_double STYPE_DOUBLE
#elif LDBL_MANT_DIG == DEFAULT_FLT_MANT_DIG
THR_MESSAGE("sizeof(long double) is 4 bytes. Long double signals assumed as Float signals.");
#define stype_double STYPE_FLOAT
#else
THR_MESSAGE("sizeof(long double) is nor 4 and nor 8 bytes. Long double signals are not allowed.");
#endif

#if DSPA_NEWLIB_FEATURE_ENABLED

/* STRING */

template <class T>
struct is_stype_string
	: std::integral_constant<
		  bool,
		  std::is_same_v<char *, std::remove_const_t<std::decay_t<T>>>>
{
};

template <class T>
static __inline __constexpr bool is_stype_string_v = is_stype_string<T>::value;

/* BOOL */

template <class T>
struct is_stype_bool
	: std::integral_constant<
		  bool,
		  std::is_same_v<bool, T>>
{
};

template <class T>
static __inline __constexpr bool is_stype_bool_v = is_stype_bool<T>::value;

/* FLOAT */

template <class T>
struct is_stype_float
	: std::integral_constant<
		  bool,
		  std::is_floating_point_v<T> &&
			  sizeof(T) == sizeof(float)>
{
};

template <class T>
static __inline __constexpr bool is_stype_float_v = is_stype_float<T>::value;

/* DOUBLE */

template <class T>
struct is_stype_double
	: std::integral_constant<
		  bool,
		  std::is_floating_point_v<T> &&
			  sizeof(T) == sizeof(double)>
{
};

template <class T>
static __inline __constexpr bool is_stype_double_v = is_stype_double<T>::value;

/* BYTE */

template <class T>
struct is_byte
	: std::integral_constant<
		  bool,
		  (sizeof(T) == sizeof(du8)) &&
			  !is_stype_string_v<T> &&
			  !is_stype_bool_v<T>>
{
};

template <class T>
static __inline __constexpr bool is_byte_v = is_byte<T>::value;

template <class T>
struct is_stype_ubyte
	: std::integral_constant<
		  bool,
		  is_byte_v<T> && is_unsigned_type_v<T>>
{
};

template <class T>
static __inline __constexpr bool is_stype_ubyte_v = is_stype_ubyte<T>::value;

template <class T>
struct is_stype_byte
	: std::integral_constant<
		  bool,
		  is_byte_v<T> && is_signed_type_v<T>>
{
};

template <class T>
static __inline __constexpr bool is_stype_byte_v = is_stype_byte<T>::value;

/* SHORT */

template <class T>
struct is_short
	: std::integral_constant<
		  bool,
		  (sizeof(T) == sizeof(du16)) &&
			  !is_stype_string_v<T>>
{
};

template <class T>
static __inline __constexpr bool is_short_v = is_short<T>::value;

template <class T>
struct is_stype_ushort
	: std::integral_constant<
		  bool,
		  is_short_v<T> && is_unsigned_type_v<T>>
{
};

template <class T>
static __inline __constexpr bool is_stype_ushort_v = is_stype_ushort<T>::value;

template <class T>
struct is_stype_short
	: std::integral_constant<
		  bool,
		  is_short_v<T> && is_signed_type_v<T>>
{
};

template <class T>
static __inline __constexpr bool is_stype_short_v = is_stype_short<T>::value;

/* LONG */

template <class T>
struct is_long
	: std::integral_constant<
		  bool,
		  (sizeof(T) == sizeof(du32)) &&
			  !is_stype_float_v<T> &&
			  !is_stype_string_v<T>>
{
};

template <class T>
static __inline __constexpr bool is_long_v = is_long<T>::value;

template <class T>
struct is_stype_ulong
	: std::integral_constant<
		  bool,
		  is_long_v<T> && is_unsigned_type_v<T>>
{
};

template <class T>
static __inline __constexpr bool is_stype_ulong_v = is_stype_ulong<T>::value;

template <class T>
struct is_stype_long
	: std::integral_constant<
		  bool,
		  is_long_v<T> && is_signed_type_v<T>>
{
};

template <class T>
static __inline __constexpr bool is_stype_long_v = is_stype_long<T>::value;

/* LONG LONG */

template <class T>
struct is_long_long
	: std::integral_constant<
		  bool,
		  (sizeof(T) == sizeof(du64)) &&
			  !is_stype_double_v<T> &&
			  !is_stype_string_v<T>>
{
};

template <class T>
static __inline __constexpr bool is_long_long_v = is_long_long<T>::value;

template <class T>
struct is_stype_ulong_long
	: std::integral_constant<
		  bool,
		  is_long_long_v<T> && is_unsigned_type_v<T>>
{
};

template <class T>
static __inline __constexpr bool is_stype_ulong_long_v = is_stype_ulong_long<T>::value;

template <class T>
struct is_stype_long_long
	: std::integral_constant<
		  bool,
		  is_long_long_v<T> && is_signed_type_v<T>>
{
};

template <class T>
static __inline __constexpr bool is_stype_long_long_v = is_stype_long_long<T>::value;

#endif /* DSPA_NEWLIB_FEATURE_ENABLED */

#if DSPA_STRICT_SIGNALS_FEATURE_ENABLED

/* Public functions */

template <class T, std::enable_if_t<is_stype_string_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _STRING_R_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_cvref_t<decltype(Value)>;
	return GenericSignal(name, STYPE_STRING, +(add_attributes | SIG_ATTR_R),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_string_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _STRING_RW_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_reference_t<decltype(Value)>;
	return GenericSignal(name, STYPE_STRING, +(add_attributes | SIG_ATTR_RW),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_ubyte_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _U8_R_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_cvref_t<decltype(Value)>;
	return GenericSignal(name, STYPE_BYTE, +(add_attributes | SIG_ATTR_TR),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_ubyte_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _U8_RW_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_reference_t<decltype(Value)>;
	return GenericSignal(name, STYPE_BYTE, +(add_attributes | SIG_ATTR_TRW),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_byte_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _INT8_R_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_cvref_t<decltype(Value)>;
	return GenericSignal(name, STYPE_SBYTE, +(add_attributes | SIG_ATTR_TR),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_byte_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _INT8_RW_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_reference_t<decltype(Value)>;
	return GenericSignal(name, STYPE_SBYTE, +(add_attributes | SIG_ATTR_TRW),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_ushort_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _U16_R_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_cvref_t<decltype(Value)>;
	return GenericSignal(name, STYPE_USHORT, +(add_attributes | SIG_ATTR_TR),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_ushort_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _U16_RW_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_reference_t<decltype(Value)>;
	return GenericSignal(name, STYPE_USHORT, +(add_attributes | SIG_ATTR_TRW),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_short_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _INT16_R_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_cvref_t<decltype(Value)>;
	return GenericSignal(name, STYPE_SHORT, +(add_attributes | SIG_ATTR_TR),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_short_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _INT16_RW_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_reference_t<decltype(Value)>;
	return GenericSignal(name, STYPE_SHORT, +(add_attributes | SIG_ATTR_TRW),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_ulong_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _U32_R_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_cvref_t<decltype(Value)>;
	return GenericSignal(name, STYPE_ULONG, +(add_attributes | SIG_ATTR_TR),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_ulong_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _U32_RW_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_reference_t<decltype(Value)>;
	return GenericSignal(name, STYPE_ULONG, +(add_attributes | SIG_ATTR_TRW),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_long_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _INT32_R_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_cvref_t<decltype(Value)>;
	return GenericSignal(name, STYPE_LONG, +(add_attributes | SIG_ATTR_TR),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_long_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _INT32_RW_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_reference_t<decltype(Value)>;
	return GenericSignal(name, STYPE_LONG, +(add_attributes | SIG_ATTR_TRW),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_ulong_long_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _U64_R_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_cvref_t<decltype(Value)>;
	return GenericSignal(name, STYPE_UINT64, +(add_attributes | SIG_ATTR_TR),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_ulong_long_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _U64_RW_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_reference_t<decltype(Value)>;
	return GenericSignal(name, STYPE_UINT64, +(add_attributes | SIG_ATTR_TRW),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_long_long_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _INT64_R_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_cvref_t<decltype(Value)>;
	return GenericSignal(name, STYPE_INT64, +(add_attributes | SIG_ATTR_TR),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_long_long_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _INT64_RW_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_reference_t<decltype(Value)>;
	return GenericSignal(name, STYPE_INT64, +(add_attributes | SIG_ATTR_TRW),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_bool_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _BOOL_R_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_cvref_t<decltype(Value)>;
	return GenericSignal(name, STYPE_BOOL, +(add_attributes | SIG_ATTR_TR),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_bool_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _BOOL_RW_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_reference_t<decltype(Value)>;
	return GenericSignal(name, STYPE_BOOL, +(add_attributes | SIG_ATTR_TRW),
						 const_cast<target_type &>(Value), parent);
}

#ifdef stype_float

template <class T, std::enable_if_t<is_stype_float_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _FLOAT_R_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_cvref_t<decltype(Value)>;
	return GenericSignal(name, stype_float, +(add_attributes | SIG_ATTR_TR),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_float_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _FLOAT_RW_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_reference_t<decltype(Value)>;
	return GenericSignal(name, stype_float, +(add_attributes | SIG_ATTR_TRW),
						 const_cast<target_type &>(Value), parent);
}

#endif /* stype_float */

#ifdef stype_double

template <class T, std::enable_if_t<is_stype_double_v<std::remove_cv_t<T>> && std::is_same_v<std::remove_cv_t<T>, double>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _DOUBLE_R_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_cvref_t<decltype(Value)>;
	return GenericSignal(name, stype_double, +(add_attributes | SIG_ATTR_TR),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_double_v<std::remove_cv_t<T>> && std::is_same_v<std::remove_cv_t<T>, double>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _DOUBLE_RW_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_reference_t<decltype(Value)>;
	return GenericSignal(name, stype_double, +(add_attributes | SIG_ATTR_TRW),
						 const_cast<target_type &>(Value), parent);
}

#endif /* stype_double */

#ifdef stype_long_double

template <class T, std::enable_if_t<is_stype_double_v<std::remove_cv_t<T>> && std::is_same_v<std::remove_cv_t<T>, long double>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _DOUBLE_R_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_cvref_t<decltype(Value)>;
	return GenericSignal(name, stype_long_double, +(add_attributes | SIG_ATTR_TR),
						 const_cast<target_type &>(Value), parent);
}

template <class T, std::enable_if_t<is_stype_double_v<std::remove_cv_t<T>> && std::is_same_v<std::remove_cv_t<T>, long double>, bool> = true>
__nodiscard __always_inline __constexpr static Signal_t _DOUBLE_RW_(
	const char *const name, T &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_reference_t<decltype(Value)>;
	return GenericSignal(name, stype_long_double, +(add_attributes | SIG_ATTR_TRW),
						 const_cast<target_type &>(Value), parent);
}

#endif /* stype_long_double */

#else

#define _STRING_RW_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_STRING, +SIG_ATTR_RW,  \
				  Value, parent_signal)

#define _STRING_R_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_STRING, +SIG_ATTR_R,  \
				  Value, parent_signal)

#define _U8_RW_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_BYTE,              \
				  +SIG_ATTR_TRW,                 \
				  Value, parent_signal)

#define _U8_R_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_BYTE,             \
				  +SIG_ATTR_TR,                 \
				  Value, parent_signal)

#define _INT8_RW_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_SBYTE,               \
				  +SIG_ATTR_TRW,                   \
				  Value, parent_signal)

#define _INT8_R_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_SBYTE,              \
				  +SIG_ATTR_TR,                   \
				  Value, parent_signal)

#define _INT16_RW_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_SHORT,                \
				  +SIG_ATTR_TRW,                    \
				  Value, parent_signal)

#define _INT16_R_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_SHORT,               \
				  +SIG_ATTR_TR,                    \
				  Value, parent_signal)

#define _U16_RW_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_USHORT,             \
				  +SIG_ATTR_TRW,                  \
				  Value, parent_signal)

#define _U16_R_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_USHORT,            \
				  +SIG_ATTR_TR,                  \
				  Value, parent_signal)

#define _INT32_RW_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_LONG,                 \
				  +SIG_ATTR_TRW,                    \
				  Value, parent_signal)

#define _INT32_R_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_LONG,                \
				  +SIG_ATTR_TR,                    \
				  Value, parent_signal)

#define _U32_RW_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_ULONG,              \
				  +SIG_ATTR_TRW,                  \
				  Value, parent_signal)

#define _U32_R_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_ULONG,             \
				  +SIG_ATTR_TR,                  \
				  Value, parent_signal)

#define _BOOL_RW_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_BOOL,                \
				  +SIG_ATTR_TRW,                   \
				  Value, parent_signal)

#define _BOOL_R_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_BOOL,               \
				  +SIG_ATTR_TR,                   \
				  Value, parent_signal)

#ifdef stype_float

#define _FLOAT_RW_(name, Value, parent_signal, ...) \
	GenericSignal(name, stype_float,                \
				  +SIG_ATTR_TRW,                    \
				  Value, parent_signal)

#define _FLOAT_R_(name, Value, parent_signal, ...) \
	GenericSignal(name, stype_float,               \
				  +SIG_ATTR_TR,                    \
				  Value, parent_signal)

#endif /* stype_float */

#define _INT64_RW_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_INT64,                \
				  +SIG_ATTR_TRW,                    \
				  Value, parent_signal)

#define _INT64_R_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_INT64,               \
				  +SIG_ATTR_TR,                    \
				  Value, parent_signal)

#define _U64_RW_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_UINT64,             \
				  +SIG_ATTR_TRW,                  \
				  Value, parent_signal)

#define _U64_R_(name, Value, parent_signal, ...) \
	GenericSignal(name, STYPE_UINT64,            \
				  +SIG_ATTR_TR,                  \
				  Value, parent_signal)

#ifdef stype_double

#define _DOUBLE_RW_(name, Value, parent_signal, ...) \
	GenericSignal(name, stype_double,                \
				  +SIG_ATTR_TRW,                     \
				  Value, parent_signal)

#define _DOUBLE_R_(name, Value, parent_signal, ...) \
	GenericSignal(name, stype_double,               \
				  +SIG_ATTR_TR,                     \
				  Value, parent_signal)

#endif /* stype_double */

#endif /* DSPA_STRICT_SIGNALS_FEATURE_ENABLED */

/* signal aliases */

#define _BYTE_RW_ _U8_RW_
#define _BYTE_R_ _U8_R_
#define _I8_R_ _INT8_R_
#define _I8_RW_ _INT8_RW_
#define _I16_R_ _INT16_R_
#define _I16_RW_ _INT16_RW_
#define _I32_R_ _INT32_R_
#define _I32_RW_ _INT32_RW_
#define _I64_R_ _INT64_R_
#define _I64_RW_ _INT64_RW_

#if DSPA_AUTO_SIGNALS_FEATURE_ENABLED

template <class T, std::enable_if_t<is_stype_string_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static SIG_Type_t GetType()
{
	return STYPE_STRING;
}

static_assert(GetType<char *>() == STYPE_STRING);
static_assert(GetType<char *const>() == STYPE_STRING);
static_assert(GetType<char[]>() == STYPE_STRING);

template <class T, std::enable_if_t<is_stype_bool_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static SIG_Type_t GetType()
{
	return STYPE_BOOL;
}

static_assert(GetType<bool>() == STYPE_BOOL);

template <class T, std::enable_if_t<is_stype_byte_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static SIG_Type_t GetType()
{
	return STYPE_SBYTE;
}

static_assert(GetType<dint8>() == STYPE_SBYTE);

template <class T, std::enable_if_t<is_stype_ubyte_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static SIG_Type_t GetType()
{
	return STYPE_BYTE;
}

static_assert(GetType<du8>() == STYPE_BYTE);

template <class T, std::enable_if_t<is_stype_short_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static SIG_Type_t GetType()
{
	return STYPE_SHORT;
}

static_assert(GetType<dint16>() == STYPE_SHORT);

template <class T, std::enable_if_t<is_stype_ushort_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static SIG_Type_t GetType()
{
	return STYPE_USHORT;
}

static_assert(GetType<du16>() == STYPE_USHORT);

template <class T, std::enable_if_t<is_stype_long_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static SIG_Type_t GetType()
{
	return STYPE_LONG;
}

static_assert(GetType<dint32>() == STYPE_LONG);

template <class T, std::enable_if_t<is_stype_ulong_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static SIG_Type_t GetType()
{
	return STYPE_ULONG;
}

static_assert(GetType<du32>() == STYPE_ULONG);

template <class T, std::enable_if_t<is_stype_long_long_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static SIG_Type_t GetType()
{
	return STYPE_INT64;
}

static_assert(GetType<dint64>() == STYPE_INT64);

template <class T, std::enable_if_t<is_stype_ulong_long_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static SIG_Type_t GetType()
{
	return STYPE_UINT64;
}

static_assert(GetType<du64>() == STYPE_UINT64);

#ifdef stype_float

template <class T, std::enable_if_t<is_stype_float_v<std::remove_cv_t<T>>, bool> = true>
__nodiscard __always_inline __constexpr static SIG_Type_t GetType()
{
	return STYPE_FLOAT;
}

static_assert(GetType<float>() == STYPE_FLOAT);

#endif /* stype_float */

#ifdef stype_double

template <class T, std::enable_if_t<is_stype_double_v<std::remove_cv_t<T>> && std::is_same_v<std::remove_cv_t<T>, double>, bool> = true>
__nodiscard __always_inline __constexpr static SIG_Type_t GetType()
{
	return stype_double;
}

static_assert(GetType<double>() == stype_double);

#endif /* stype_double */

#ifdef stype_long_double

template <class T, std::enable_if_t<is_stype_double_v<std::remove_cv_t<T>> && std::is_same_v<std::remove_cv_t<T>, long double>, bool> = true>
__nodiscard __always_inline __constexpr static SIG_Type_t GetType()
{
	return stype_long_double;
}

static_assert(GetType<long double>() == stype_long_double);

#endif /* stype_long_double */

template <class T, std::enable_if_t<is_stype_string_v<T>, bool> = true>
__nodiscard __always_inline __constexpr static int VerifyAttribute(const int &attribute)
{
	return +attribute & +(~SIG_ATTR_T); // string-type signals are not allowed in telemetry mode
}

template <class T, std::enable_if_t<!is_stype_string_v<T>, bool> = true>
__nodiscard __always_inline __constexpr static int VerifyAttribute(const int &attribute)
{
	return attribute;
}

GENERIC_SIGNAL_TEMPLATE
__nodiscard __always_inline __constexpr static Signal_t _AUTO_R_(
	const char *const name,
	GENERIC_SIGNAL_VALUETYPE &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_cvref_t<decltype(Value)>;

	return GenericSignal(
		name,
		GetType<target_type>(),
		+VerifyAttribute<target_type>(+(add_attributes | SIG_ATTR_TR)),
		const_cast<target_type &>(Value),
		parent);
}

GENERIC_SIGNAL_TEMPLATE
__nodiscard __always_inline __constexpr static Signal_t _AUTO_RW_(
	const char *const name,
	GENERIC_SIGNAL_VALUETYPE &Value, void *parent,
	const SIG_ATTR_TYPE add_attributes = SIG_ATTR_N)
{
	using target_type = std::remove_reference_t<decltype(Value)>;

	return GenericSignal(
		name,
		GetType<target_type>(),
		+VerifyAttribute<target_type>(+(add_attributes | SIG_ATTR_TRW)),
		const_cast<target_type &>(Value),
		parent);
}

#define U8_R _AUTO_R_
#define U16_R _AUTO_R_
#define U32_R _AUTO_R_
#define U64_R _AUTO_R_
#define I8_R _AUTO_R_
#define I16_R _AUTO_R_
#define I32_R _AUTO_R_
#define I64_R _AUTO_R_
#define BOOL_R _AUTO_R_
#define FLOAT_R _AUTO_R_
#define DOUBLE_R _AUTO_R_
#define STRING_R _AUTO_R_

#define U8_RW _AUTO_RW_
#define U16_RW _AUTO_RW_
#define U32_RW _AUTO_RW_
#define U64_RW _AUTO_RW_
#define I8_RW _AUTO_RW_
#define I16_RW _AUTO_RW_
#define I32_RW _AUTO_RW_
#define I64_RW _AUTO_RW_
#define BOOL_RW _AUTO_RW_
#define FLOAT_RW _AUTO_RW_
#define DOUBLE_RW _AUTO_RW_
#define STRING_RW _AUTO_RW_

#else

#define U8_R _U8_R_
#define U16_R _U16_R_
#define U32_R _U32_R_
#define U64_R _U64_R_
#define I8_R _I8_R_
#define I16_R _I16_R_
#define I32_R _I32_R_
#define I64_R _I64_R_
#define BOOL_R _BOOL_R_
#define FLOAT_R _FLOAT_R_
#define DOUBLE_R _DOUBLE_R_
#define STRING_R _STRING_R_

#define U8_RW _U8_RW_
#define U16_RW _U16_RW_
#define U32_RW _U32_RW_
#define U64_RW _U64_RW_
#define I8_RW _I8_RW_
#define I16_RW _I16_RW_
#define I32_RW _I32_RW_
#define I64_RW _I64_RW_
#define BOOL_RW _BOOL_RW_
#define FLOAT_RW _FLOAT_RW_
#define DOUBLE_RW _DOUBLE_RW_
#define STRING_RW _STRING_RW_

#endif /* DSPA_AUTO_SIGNALS_FEATURE_ENABLED */

#undef DEFAULT_FLT_MANT_DIG
#undef DEFAULT_DBL_MANT_DIGIT

#endif /* DSPA_SIGDEFS_H_ */
