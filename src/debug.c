/**
 * @file debug.c
 * @brief
 *    Internal implementation of RAM-based debug logging for embedded systems.
 */

#include <stdio.h>
#include <string.h>

#include "../include/debug.h"
/**
 * @brief Internal circular buffer used to store debug log entries.
 */
static char dbg_buffer[DBG_BUFFER_SIZE];

/**
 * @brief Current byte offset for the next log write.
 */
static size_t dbg_index = 0u;

void
dbg_init(void)
{
        (void)memset(dbg_buffer, 0, sizeof(dbg_buffer));
        dbg_index = 0u;
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
        va_list args;
        int len;

        va_start(args, fmt);
        len = vsnprintf(line, DBG_LINE_MAX, fmt, args);
        va_end(args);

        if (len < 0) {
                return;
        }

        size_t to_write = (size_t)len;
        if (to_write >= DBG_LINE_MAX) {
                to_write = DBG_LINE_MAX - 1u;
        }

        if (dbg_index + to_write + 1u >= DBG_BUFFER_SIZE) {
                dbg_index = 0u;
        }

        (void)memcpy(&dbg_buffer[dbg_index], line, to_write);
        dbg_buffer[dbg_index + to_write] = '\n';
        dbg_index += to_write + 1u;
}

const char *
dbg_get_buffer(void)
{
        return dbg_buffer;
}

size_t
dbg_get_index(void)
{
        return dbg_index;
}
