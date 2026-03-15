/*
 * @license MIT
 *
 * @file debug.h
 *
 * @brief
 *    Lightweight debug logging for embedded systems with JTAG-based
 *    inspection.
 *
 * @details
 *    This module provides debug logging suitable for embedded systems that
 *    lack traditional stdout or UART interfaces. Logging is stored in a
 *    RAM-based circular buffer and can be inspected live via JTAG.
 *
 *    When the `DEBUG` macro is defined, functions and macros such as
 *    `dbg_info()`, `dbg_assert()`, and `dbg_trace()` become active. If
 *    `DEBUG` is undefined, all macros become no-ops via `((void)0)` for
 *    safe compilation with zero overhead.
 *
 *    The debug log is stored in a static character buffer. Macros like
 *    `dbg_info()` write formatted strings into this buffer. The content
 *    can be retrieved via `dbg_get_buffer()` and viewed using tools like
 *    CCS memory view.
 *
 * @design
 *    - Static allocation with configurable buffer sizes
 *    - Circular buffer with overflow detection
 *    - Optional thread safety via atomic operations
 *    - Optional timestamp support via callback
 *    - Optional log level filtering
 *    - Zero overhead in release builds (DEBUG undefined)
 */

#ifndef DEBUG_H
#define DEBUG_H

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>

/**
 * @def DEBUG
 * @brief Define this macro to enable debug logging macros.
 *
 * @contract
 *    - When defined: All debug functions and macros are active
 *    - When undefined: All debug macros expand to ((void)0) with zero overhead
 *    - Should be defined in debug builds, undefined in release builds
 */

/**
 * @def DBG_BUFFER_SIZE
 * @brief Total number of bytes allocated for the debug log buffer.
 *
 * @contract
 *    - Minimum recommended: 256 bytes
 *    - Default: 2048 bytes
 *    - Must be a power of 2 for optimal performance (not enforced)
 *    - Configured at compile time only
 */

/**
 * @def DBG_LINE_MAX
 * @brief Maximum length of a single formatted log line (excluding newline).
 *
 * @contract
 *    - Messages longer than this are truncated
 *    - Minimum: 16 bytes
 *    - Default: 128 bytes
 *    - Configured at compile time only
 */

/**
 * @def DBG_THREAD_SAFE
 * @brief Define this macro to enable thread-safe logging using atomic
 * operations. Leave undefined for single-core systems to minimize overhead.
 *
 * @contract
 *    - When defined: Uses C11 atomics for thread-safe access
 *    - When undefined: No synchronization, faster on single-core
 *    - Safe for concurrent reads with single writer when defined
 *    - Multiple concurrent writers must be externally serialized
 */

/**
 * @def DBG_WITH_TIMESTAMP
 * @brief Define this macro to enable timestamp support in log entries.
 *
 * @contract
 *    - When defined: Timestamp callback can be registered
 *    - When undefined: Timestamp code is not compiled in
 *    - Requires user to provide timestamp callback via
 * dbg_set_timestamp_callback()
 */

/**
 * @def DBG_WITH_LOG_LEVELS
 * @brief Define this macro to enable log level filtering.
 *
 * @contract
 *    - When defined: Log level filtering is available
 *    - When undefined: All messages are logged
 *    - Default level is DBG_LEVEL_TRACE (all messages)
 */

/* Default configuration */
#ifndef DEBUG
#define DEBUG
#endif

#ifndef DBG_BUFFER_SIZE
#define DBG_BUFFER_SIZE (2048u)
#endif

#ifndef DBG_LINE_MAX
#define DBG_LINE_MAX (128u)
#endif

/* Configuration validation */
#if DBG_BUFFER_SIZE < 16
#error DBG_BUFFER_SIZE must be at least 16 bytes
#endif

#if DBG_LINE_MAX < 16
#error DBG_LINE_MAX must be at least 16 bytes
#endif

#if DBG_LINE_MAX >= DBG_BUFFER_SIZE
#error DBG_LINE_MAX must be less than DBG_BUFFER_SIZE
#endif

#ifdef DBG_WITH_LOG_LEVELS

/**
 * @enum dbg_level_t
 * @brief Log severity levels.
 *
 * @contract
 *    - Lower values = more verbose
 *    - Higher values = more severe
 *    - DBG_LEVEL_NONE filters all messages
 */
typedef enum {
        DBG_LEVEL_TRACE = 0, /**< Verbose trace messages */
        DBG_LEVEL_DEBUG = 1, /**< Debug-level messages */
        DBG_LEVEL_INFO = 2,  /**< Informational messages */
        DBG_LEVEL_WARN = 3,  /**< Warning messages */
        DBG_LEVEL_ERROR = 4, /**< Error messages */
        DBG_LEVEL_NONE = 5   /**< Disable all logging */
} dbg_level_t;

#endif

#ifdef DEBUG

/**
 * @brief Initialise the debug log buffer.
 *
 * @contract
 *    Pre:  None
 *    Post: Buffer is zeroed, index is 0, overflow count is 0
 *    Thread safety: Safe to call from any context
 *    Reentrancy: Not reentrant
 *
 * @note Call once at system startup before any logging.
 */
void dbg_init(void);

/**
 * @brief Write a formatted message to the debug log.
 *
 * @param fmt Format string (printf-style). @nonnull
 * @param ... Format arguments matching format specifiers
 *
 * @contract
 *    Pre:  fmt must not be NULL
 *    Post: Message appended to buffer, or buffer wrapped if full
 *    Thread safety: Safe if DBG_THREAD_SAFE defined, else caller must serialize
 *    Reentrancy: Not reentrant
 *
 * @note Messages longer than DBG_LINE_MAX are truncated.
 * @note Buffer wraps around when full; overflow count is incremented.
 */
void dbg_log(const char *fmt, ...);

/**
 * @brief Clear the debug log buffer.
 *
 * @contract
 *    Pre:  None
 *    Post: Buffer is zeroed, index is 0 (same as dbg_init)
 *    Thread safety: Safe to call from any context
 *    Reentrancy: Not reentrant
 *
 * @note Alias for dbg_init(); provided for semantic clarity.
 */
void dbg_clear(void);

/**
 * @brief Retrieve a pointer to the debug buffer for inspection.
 *
 * @return Pointer to static character buffer holding the log.
 *
 * @contract
 *    Pre:  None
 *    Post: None (read-only access)
 *    Thread safety: Reader must synchronize with writers if DBG_THREAD_SAFE
 * undefined Reentrancy: Reentrant (read-only)
 *
 * @note Use dbg_get_index() to determine valid data length.
 * @note Buffer content may change if logging continues; copy if needed.
 */
const char *dbg_get_buffer(void);

/**
 * @brief Get the current write index in the debug log.
 *
 * @return Index (in bytes) of the next write position (0 to DBG_BUFFER_SIZE-1).
 *
 * @contract
 *    Pre:  None
 *    Post: None (read-only access)
 *    Thread safety: Safe if DBG_THREAD_SAFE defined
 *    Reentrancy: Reentrant (read-only)
 *
 * @note Returns 0 when buffer is empty or has just wrapped.
 * @note Valid log data is at dbg_get_buffer()[0..dbg_get_index()-1]
 */
size_t dbg_get_index(void);

/**
 * @brief Get the number of times the debug buffer has overflowed.
 *
 * @return Count of buffer overflow events since last dbg_init().
 *
 * @contract
 *    Pre:  None
 *    Post: None (read-only access)
 *    Thread safety: Safe if DBG_THREAD_SAFE defined
 *    Reentrancy: Reentrant (read-only)
 *
 * @note Overflow occurs when buffer is full and new data causes wraparound.
 * @note Count is reset to 0 by dbg_init().
 */
uint32_t dbg_get_overflow_count(void);

/**
 * @brief Callback function type for debug buffer overflow events.
 *
 * @param count Current overflow count after this overflow event.
 *
 * @contract
 *    - Called synchronously when overflow occurs
 *    - Should be fast; called from logging path
 *    - count parameter is the new overflow count (1, 2, 3, ...)
 */
typedef void (*dbg_overflow_callback_t)(uint32_t count);

/**
 * @brief Set the callback function for overflow events.
 *
 * @param cb Callback function to call on overflow, or NULL to disable.
 *
 * @contract
 *    Pre:  None
 *    Post: Callback registered for future overflow events
 *    Thread safety: Safe if DBG_THREAD_SAFE defined
 *    Reentrancy: Not reentrant
 *
 * @note Callback is called synchronously when overflow occurs.
 * @note Keep callback implementation minimal; on critical path.
 */
void dbg_set_overflow_callback(dbg_overflow_callback_t cb);

/**
 * @brief Callback function type for getting timestamps.
 *
 * @param buf Buffer to write timestamp string into.
 * @param buf_size Size of the buffer.
 * @return Pointer to buf on success, or NULL on error.
 *
 * @contract
 *    - Called synchronously before each log message
 *    - buf is guaranteed to be at least 32 bytes
 *    - Should write null-terminated string
 *    - Return NULL to skip timestamp for this message
 */
typedef const char *(*dbg_timestamp_callback_t)(char *buf, size_t buf_size);

/**
 * @brief Set the callback function for timestamps.
 *
 * @param cb Callback function to call for timestamps, or NULL to disable.
 *
 * @contract
 *    Pre:  None
 *    Post: Callback registered for future log messages
 *    Thread safety: Safe if DBG_THREAD_SAFE defined
 *    Reentrancy: Not reentrant
 *    Requires: DBG_WITH_TIMESTAMP must be defined
 *
 * @note Callback is called before each log message.
 * @note Timestamp is prepended to the log message with a space.
 */
void dbg_set_timestamp_callback(dbg_timestamp_callback_t cb);

/**
 * @brief Get the total size of the debug buffer.
 *
 * @return Total buffer size in bytes (DBG_BUFFER_SIZE).
 *
 * @contract
 *    Pre:  None
 *    Post: None (read-only)
 *    Thread safety: Reentrant (returns constant)
 *
 * @note Useful for diagnostic purposes or UI display.
 */
size_t dbg_get_buffer_size(void);

/**
 * @brief Get the available space in the buffer before wraparound.
 *
 * @return Available bytes before the buffer would wrap (0 to DBG_BUFFER_SIZE).
 *
 * @contract
 *    Pre:  None
 *    Post: None (read-only)
 *    Thread safety: Safe if DBG_THREAD_SAFE defined
 *    Reentrancy: Reentrant (read-only)
 *
 * @note Returns DBG_BUFFER_SIZE when buffer is empty.
 * @note Returns 0 when buffer is completely full.
 * @note Useful for determining if large log will cause overflow.
 */
size_t dbg_get_available_space(void);

/**
 * @brief Get the maximum line length.
 *
 * @return Maximum length of a single log line (DBG_LINE_MAX).
 *
 * @contract
 *    Pre:  None
 *    Post: None (read-only)
 *    Thread safety: Reentrant (returns constant)
 *
 * @note Messages longer than this are truncated.
 */
size_t dbg_get_line_max(void);

#ifdef DBG_WITH_LOG_LEVELS

/**
 * @brief Set the minimum log level to display.
 *
 * @param level Minimum log level. Messages with level < this are filtered.
 *
 * @contract
 *    Pre:  level must be valid dbg_level_t value
 *    Post: Level set; affects subsequent log macro calls
 *    Thread safety: Safe if DBG_THREAD_SAFE defined
 *    Reentrancy: Not reentrant
 *
 * @note Default level is DBG_LEVEL_TRACE (all messages logged)
 * @note Setting to DBG_LEVEL_NONE disables all logging
 * @note Filtering happens at macro level for zero overhead
 */
void dbg_set_level(dbg_level_t level);

/**
 * @brief Get the current minimum log level.
 *
 * @return Current minimum log level.
 *
 * @contract
 *    Pre:  None
 *    Post: None (read-only)
 *    Thread safety: Safe if DBG_THREAD_SAFE defined
 *    Reentrancy: Reentrant (read-only)
 */
dbg_level_t dbg_get_level(void);

#endif /* DBG_WITH_LOG_LEVELS */

/**
 * @brief Log an informational message.
 *
 * @param fmt Format string (printf-style). @nonnull
 * @param ... Format arguments matching format specifiers
 *
 * @contract
 *    - Prefixes message with "INFO:  "
 *    - Filtered if level < DBG_LEVEL_INFO (when DBG_WITH_LOG_LEVELS defined)
 *    - Same constraints as dbg_log()
 *
 * @see dbg_log
 */
#ifdef DBG_WITH_LOG_LEVELS
#define dbg_info(fmt, ...)                                                     \
        do {                                                                   \
                if (dbg_get_level() <= DBG_LEVEL_INFO)                         \
                        dbg_log("INFO:  " fmt, ##__VA_ARGS__);                 \
        } while (0)
#else
#define dbg_info(fmt, ...) dbg_log("INFO:  " fmt, ##__VA_ARGS__)
#endif

/**
 * @brief Log a warning message.
 *
 * @param fmt Format string (printf-style). @nonnull
 * @param ... Format arguments matching format specifiers
 *
 * @contract
 *    - Prefixes message with "WARN:  "
 *    - Filtered if level < DBG_LEVEL_WARN (when DBG_WITH_LOG_LEVELS defined)
 *    - Same constraints as dbg_log()
 *
 * @see dbg_log
 */
#ifdef DBG_WITH_LOG_LEVELS
#define dbg_warn(fmt, ...)                                                     \
        do {                                                                   \
                if (dbg_get_level() <= DBG_LEVEL_WARN)                         \
                        dbg_log("WARN:  " fmt, ##__VA_ARGS__);                 \
        } while (0)
#else
#define dbg_warn(fmt, ...) dbg_log("WARN:  " fmt, ##__VA_ARGS__)
#endif

/**
 * @brief Log an error message.
 *
 * @param fmt Format string (printf-style). @nonnull
 * @param ... Format arguments matching format specifiers
 *
 * @contract
 *    - Prefixes message with "ERROR: "
 *    - Filtered if level < DBG_LEVEL_ERROR (when DBG_WITH_LOG_LEVELS defined)
 *    - Same constraints as dbg_log()
 *
 * @see dbg_log
 */
#ifdef DBG_WITH_LOG_LEVELS
#define dbg_error(fmt, ...)                                                    \
        do {                                                                   \
                if (dbg_get_level() <= DBG_LEVEL_ERROR)                        \
                        dbg_log("ERROR: " fmt, ##__VA_ARGS__);                 \
        } while (0)
#else
#define dbg_error(fmt, ...) dbg_log("ERROR: " fmt, ##__VA_ARGS__)
#endif

/**
 * @brief Log a trace-level message (for verbose function-level tracing).
 *
 * @param fmt Format string (printf-style). @nonnull
 * @param ... Format arguments matching format specifiers
 *
 * @contract
 *    - Prefixes message with "TRACE: "
 *    - Filtered if level < DBG_LEVEL_TRACE (when DBG_WITH_LOG_LEVELS defined)
 *    - Same constraints as dbg_log()
 *
 * @see dbg_log
 */
#ifdef DBG_WITH_LOG_LEVELS
#define dbg_trace(fmt, ...)                                                    \
        do {                                                                   \
                if (dbg_get_level() <= DBG_LEVEL_TRACE)                        \
                        dbg_log("TRACE: " fmt, ##__VA_ARGS__);                 \
        } while (0)
#else
#define dbg_trace(fmt, ...) dbg_log("TRACE: " fmt, ##__VA_ARGS__)
#endif

/**
 * @brief Assert that an expression is true, otherwise halt.
 *
 * @param expr Expression to evaluate. Must be true.
 *
 * @contract
 *    Pre:  expr must evaluate to true
 *    Post: If expr is false: error logged, then system halts via ESTOP0
 *    Thread safety: Same as dbg_log()
 *
 * @note Logs assertion failure with file and line number before halting.
 * @note Uses TI C2000 ESTOP0 instruction; may need porting for other targets.
 * @note Expands to ((void)0) when DEBUG is undefined.
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
 *
 * @contract
 *    - Logs "ENTER: function_name()"
 *    - Uses __func__ for function name
 *    - Same constraints as dbg_log()
 *
 * @example
 *    void my_function(void) {
 *        dbg_enter();
 *        // function body
 *        dbg_exit();
 *    }
 */
#define dbg_enter() dbg_log("ENTER: %s()", __func__)

/**
 * @brief Mark function exit in the debug log.
 *
 * @contract
 *    - Logs "EXIT:  function_name()"
 *    - Uses __func__ for function name
 *    - Same constraints as dbg_log()
 *
 * @see dbg_enter
 */
#define dbg_exit()  dbg_log("EXIT:  %s()", __func__)

#else /* DEBUG not defined */
/*
 * Ensure no action taken when DEBUG is not defined.
 * All macros expand to ((void)0) for zero overhead in release builds.
 */
#define dbg_init()                     ((void)0)
#define dbg_log(...)                   ((void)0)
#define dbg_clear()                    ((void)0)
#define dbg_get_buffer()               ((const char *)0)
#define dbg_get_index()                (0U)
#define dbg_get_overflow_count()       (0U)
#define dbg_set_overflow_callback(cb)  ((void)0)
#define dbg_set_timestamp_callback(cb) ((void)0)
#define dbg_get_buffer_size()          (0U)
#define dbg_get_available_space()      (0U)
#define dbg_get_line_max()             (0U)
#ifdef DBG_WITH_LOG_LEVELS
#define dbg_set_level(lvl) ((void)0)
#define dbg_get_level()    (DBG_LEVEL_NONE)
#endif

#define dbg_info(...)    ((void)0)
#define dbg_warn(...)    ((void)0)
#define dbg_error(...)   ((void)0)
#define dbg_trace(...)   ((void)0)
#define dbg_assert(expr) ((void)0)
#define dbg_enter()      ((void)0)
#define dbg_exit()       ((void)0)

#endif /* DEBUG */

#endif /* DEBUG_H */
