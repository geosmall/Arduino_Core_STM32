#ifndef __INV_COMMON_H__
#define __INV_COMMON_H__

/*
 * Macro to convert INV_MSG() to PrintLine logger function.
 * This avoids having to modify the Icm426xx driver by
 * basically ignoring the log level and directly printing.
 * from:
 * #define INV_MSG(level, ...)        _INV_MSG(level, __VA_ARGS__)
 * #define _INV_MSG(level, ...)       inv_msg(level, __VA_ARGS__)
 * void INV_EXPORT inv_msg(int level, const char * str, ...);
 * to:
 * extern "C" void hw_PrintLine(const char* format, VA... va)
 */
#define INV_MSG(level, ...) hw_PrintLine(__VA_ARGS__)

// Work with either C or C++ code
#ifdef __cplusplus
extern "C" {
#endif

void hw_PrintLine(const char* format, ...);

#ifdef __cplusplus
}
#endif

#endif //__INV_COMMON_H__