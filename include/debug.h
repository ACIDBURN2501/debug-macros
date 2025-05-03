/*
 * @licence MIT
 *
 * @file: debug.h
 *
 * @brief
 *    Debugging and logging utilities for embedded systems using JTAG-based
 *    inspection.
 *
 * @details
 *    This module provides lightweight debug logging suitable for embedded
 *    systems that lack traditional stdout or UART interfaces. Logging is stored
 *    in a RAM-based circular buffer and can be inspected live via JTAG.
 *
 *    When the `DEBUG` macro is defined, functions and macros such as
 *    `dbg_info()`, `dbg_assert()`, and `dbg_trace()` become active. If `DEBUG`
 *    is undefined, all macros become no-ops via `((void)0)` for safe
 *    compilation.
 *
 *    The debug log is stored in a static character buffer. Macros like
 *    `dbg_info()` write formatted strings into this buffer. The content can be
 *    retrieved via `dbg_get_buffer()` and viewed using tools like CCS memory
 *    view.
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @def DEBUG
 * @brief Define this macro to enable debug logging macros.
 */

/**
 * @def DBG_BUFFER_SIZE
 * @brief Total number of bytes allocated for the debug log buffer.
 */

/**
 * @def DBG_LINE_MAX
 * @brief Maximum length of a single formatted log line (excluding newline).
 */

#define DEBUG
#define DBG_BUFFER_SIZE (2048u)
#define DBG_LINE_MAX    (128u)

#ifdef DEBUG

/**
 * @brief Initialise the debug log buffer.
 *
 * This function zeroes out the internal buffer and resets the write index.
 */
void dbg_init(void);

/**
 * @brief Write a formatted message to the debug log.
 *
 * @param fmt Format string (printf-style)
 * @param ... Format arguments
 */
void dbg_log(const char *fmt, ...);

/**
 * @brief Clear the debug log buffer.
 *
 * Alias for dbg_init(); resets the buffer to all zero and the write index to 0.
 */
void dbg_clear(void);

/**
 * @brief Retrieve a pointer to the debug buffer for inspection.
 *
 * @return Pointer to static character buffer holding the log.
 */
const char *dbg_get_buffer(void);

/**
 * @brief Get the current write index in the debug log.
 *
 * @return Index (in bytes) of the last written log line.
 */
size_t dbg_get_index(void);

/**
 * @brief Log an informational message.
 * @param fmt Format string
 * @param ... Format arguments
 */
#define dbg_info(fmt, ...)  dbg_log("INFO:  " fmt, ##__VA_ARGS__)

/**
 * @brief Log a warning message.
 * @param fmt Format string
 * @param ... Format arguments
 */
#define dbg_warn(fmt, ...)  dbg_log("WARN:  " fmt, ##__VA_ARGS__)

/**
 * @brief Log an error message.
 * @param fmt Format string
 * @param ... Format arguments
 */
#define dbg_error(fmt, ...) dbg_log("ERROR: " fmt, ##__VA_ARGS__)

/**
 * @brief Log a trace-level message (for verbose function-level tracing).
 * @param fmt Format string
 * @param ... Format arguments
 */
#define dbg_trace(fmt, ...) dbg_log("TRACE: " fmt, ##__VA_ARGS__)

/**
 * @brief Assert that an expression is true, otherwise halt.
 *
 * Writes an error to the debug log, then triggers a halt (`ESTOP0`).
 *
 * @param expr Expression to evaluate
 */
#define dbg_assert(expr)                                                       \
        do {                                                                   \
                if (!(expr)) {                                                 \
                        dbg_log("ASSERT FAILED: %s (%s:%d)", #expr, __FILE__,  \
                                __LINE__);                                     \
                        while (1) {                                            \
                                __asm__(" ESTOP0");                            \
                        }                                                      \
                }                                                              \
        } while (0)

/**
 * @brief Mark function entry in the debug log.
 */
#define dbg_enter() dbg_log("ENTER: %s()", __func__)

/**
 * @brief Mark function exit in the debug log.
 */
#define dbg_exit()  dbg_log("EXIT:  %s()", __func__)

#else /* DEBUG not defined */
/*
 * Ensure no action taken when DEBUG is not defined.
 */
#define dbg_init()       ((void)0)
#define dbg_log(...)     ((void)0)
#define dbg_clear()      ((void)0)
#define dbg_get_buffer() ((const char *)0)
#define dbg_get_index()  (0U)

#define dbg_info(...)    ((void)0)
#define dbg_warn(...)    ((void)0)
#define dbg_error(...)   ((void)0)
#define dbg_trace(...)   ((void)0)
#define dbg_assert(expr) ((void)0)
#define dbg_enter()      ((void)0)
#define dbg_exit()       ((void)0)

#endif /* DEBUG */

#endif /* DEBUG_H */
