/**
 * @file debug.c
 * @brief
 *    Internal implementation of RAM-based debug logging for embedded systems.
 */

#include <stdio.h>
#include <string.h>

#ifdef DBG_THREAD_SAFE
#include <stdatomic.h>
#endif

#include "dbg.h"
/**
 * @brief Internal circular buffer used to store debug log entries.
 */
static char dbg_buffer[DBG_BUFFER_SIZE];

/**
 * @brief Current byte offset for the next log write.
 */
#ifdef DBG_THREAD_SAFE
static _Atomic size_t dbg_index = 0u;
#else
static size_t dbg_index = 0u;
#endif

/**
 * @brief Counter for buffer overflow events.
 */
#ifdef DBG_THREAD_SAFE
static _Atomic uint32_t dbg_overflow_count = 0u;
#else
static uint32_t dbg_overflow_count = 0u;
#endif

/**
 * @brief Callback function for overflow events.
 */
#ifdef DBG_THREAD_SAFE
static _Atomic dbg_overflow_callback_t dbg_overflow_cb = NULL;
#else
static dbg_overflow_callback_t dbg_overflow_cb = NULL;
#endif

/**
 * @brief Callback function for timestamps.
 */
#ifdef DBG_THREAD_SAFE
static _Atomic dbg_timestamp_callback_t dbg_timestamp_cb = NULL;
#else
static dbg_timestamp_callback_t dbg_timestamp_cb = NULL;
#endif

#ifdef DBG_WITH_LOG_LEVELS
/**
 * @brief Current minimum log level.
 */
#ifdef DBG_THREAD_SAFE
static _Atomic dbg_level_t dbg_current_level = DBG_LEVEL_TRACE;
#else
static dbg_level_t dbg_current_level = DBG_LEVEL_TRACE;
#endif
#endif

void
dbg_init(void)
{
        (void)memset(dbg_buffer, 0, sizeof(dbg_buffer));
#ifdef DBG_THREAD_SAFE
        atomic_store(&dbg_index, 0u);
        atomic_store(&dbg_overflow_count, 0u);
        atomic_store(&dbg_overflow_cb, NULL);
        atomic_store(&dbg_timestamp_cb, NULL);
#ifdef DBG_WITH_LOG_LEVELS
        atomic_store(&dbg_current_level, DBG_LEVEL_TRACE);
#endif
#else
        dbg_index = 0u;
        dbg_overflow_count = 0u;
        dbg_overflow_cb = NULL;
        dbg_timestamp_cb = NULL;
#ifdef DBG_WITH_LOG_LEVELS
        dbg_current_level = DBG_LEVEL_TRACE;
#endif
#endif
}

void
dbg_set_overflow_callback(dbg_overflow_callback_t cb)
{
#ifdef DBG_THREAD_SAFE
        atomic_store(&dbg_overflow_cb, cb);
#else
        dbg_overflow_cb = cb;
#endif
}

void
dbg_set_timestamp_callback(dbg_timestamp_callback_t cb)
{
#ifdef DBG_THREAD_SAFE
        atomic_store(&dbg_timestamp_cb, cb);
#else
        dbg_timestamp_cb = cb;
#endif
}

void
dbg_clear(void)
{
        dbg_init();
}

void
dbg_log(const char *fmt, ...)
{
        char line[DBG_LINE_MAX];
#ifdef DBG_WITH_TIMESTAMP
        char ts_buf[32];
#endif
        va_list args;
        int len;

        if (fmt == NULL) {
                return;
        }

#ifdef DBG_WITH_TIMESTAMP
        dbg_timestamp_callback_t ts_cb;
#ifdef DBG_THREAD_SAFE
        ts_cb = atomic_load(&dbg_timestamp_cb);
#else
        ts_cb = dbg_timestamp_cb;
#endif
        if (ts_cb != NULL) {
                const char *ts = ts_cb(ts_buf, sizeof(ts_buf));
                if (ts != NULL) {
                        len = snprintf(line, DBG_LINE_MAX, "%s ", ts);
                        if (len < 0 || (size_t)len >= DBG_LINE_MAX) {
                                return;
                        }
                        va_start(args, fmt);
                        len += vsnprintf(line + len, DBG_LINE_MAX - len, fmt,
                                         args);
                        va_end(args);
                        if (len < 0) {
                                return;
                        }
                } else {
                        va_start(args, fmt);
                        len = vsnprintf(line, DBG_LINE_MAX, fmt, args);
                        va_end(args);
                }
        } else {
                va_start(args, fmt);
                len = vsnprintf(line, DBG_LINE_MAX, fmt, args);
                va_end(args);
        }
#else
        va_start(args, fmt);
        len = vsnprintf(line, DBG_LINE_MAX, fmt, args);
        va_end(args);
#endif

        if (len < 0) {
                return;
        }

        size_t to_write = (size_t)len;
        if (to_write >= DBG_LINE_MAX) {
                to_write = DBG_LINE_MAX - 1u;
        }

#ifdef DBG_THREAD_SAFE
        size_t current_idx = atomic_load(&dbg_index);
        size_t new_idx;
        uint32_t new_count;

        if (current_idx + to_write + 1u >= DBG_BUFFER_SIZE) {
                new_idx = 0u;
                new_count =
                    (uint32_t)atomic_fetch_add(&dbg_overflow_count, 1u) + 1u;
                dbg_overflow_callback_t cb = atomic_load(&dbg_overflow_cb);
                if (cb != NULL) {
                        cb(new_count);
                }
        } else {
                new_idx = current_idx;
        }

        (void)memcpy(&dbg_buffer[new_idx], line, to_write);
        dbg_buffer[new_idx + to_write] = '\n';
        atomic_store(&dbg_index, new_idx + to_write + 1u);
#else
        if (dbg_index + to_write + 1u >= DBG_BUFFER_SIZE) {
                dbg_index = 0u;
                dbg_overflow_count++;
                if (dbg_overflow_cb != NULL) {
                        dbg_overflow_cb(dbg_overflow_count);
                }
        }

        (void)memcpy(&dbg_buffer[dbg_index], line, to_write);
        dbg_buffer[dbg_index + to_write] = '\n';
        dbg_index += to_write + 1u;
#endif
}

const char *
dbg_get_buffer(void)
{
        return dbg_buffer;
}

size_t
dbg_get_index(void)
{
#ifdef DBG_THREAD_SAFE
        return atomic_load(&dbg_index);
#else
        return dbg_index;
#endif
}

uint32_t
dbg_get_overflow_count(void)
{
#ifdef DBG_THREAD_SAFE
        return (uint32_t)atomic_load(&dbg_overflow_count);
#else
        return dbg_overflow_count;
#endif
}

size_t
dbg_get_buffer_size(void)
{
        return DBG_BUFFER_SIZE;
}

size_t
dbg_get_available_space(void)
{
#ifdef DBG_THREAD_SAFE
        size_t idx = atomic_load(&dbg_index);
#else
        size_t idx = dbg_index;
#endif
        return DBG_BUFFER_SIZE - idx;
}

size_t
dbg_get_line_max(void)
{
        return DBG_LINE_MAX;
}

#ifdef DBG_WITH_LOG_LEVELS

void
dbg_set_level(dbg_level_t level)
{
#ifdef DBG_THREAD_SAFE
        atomic_store(&dbg_current_level, level);
#else
        dbg_current_level = level;
#endif
}

dbg_level_t
dbg_get_level(void)
{
#ifdef DBG_THREAD_SAFE
        return atomic_load(&dbg_current_level);
#else
        return dbg_current_level;
#endif
}

#endif
