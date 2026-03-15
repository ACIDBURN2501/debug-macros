# dbg


[![Run Unity Tests](https://github.com/ACIDBURN2501/debug-macros/actions/workflows/test.yml/badge.svg)](https://github.com/ACIDBURN2501/debug-macros/actions/workflows/test.yml)  [![Build Documentation](https://github.com/ACIDBURN2501/debug-macros/actions/workflows/docs.yml/badge.svg)](https://github.com/ACIDBURN2501/debug-macros/actions/workflows/docs.yml)

Lightweight debug logging for embedded systems with RAM-based circular buffer and JTAG inspection.

## Features

- **RAM-based circular buffer**: Log messages stored in static RAM, viewable via JTAG
- **Multiple log levels**: info, warn, error, trace with optional runtime filtering
- **Assert with halt**: `dbg_assert()` logs failure then halts via `ESTOP0`
- **Overflow detection**: Callback mechanism for buffer wraparound events
- **Optional timestamps**: User-provided timestamp callback for time-stamped logs
- **Thread-safe option**: Optional atomic operations for multi-core safety
- **Zero overhead in release**: All macros become `((void)0)` when `DEBUG` undefined
- **Compliance-aware design goals**: Small auditable codebase with static allocation, explicit contracts, and unit-test coverage.

## Using the Library

### As a Meson subproject

```meson
debug_dep = dependency('debug-macros', fallback: ['debug-macros', 'lib_debug'])
```

For subproject builds, include the public header directly:

```c
#include "debug.h"
```

### As an installed dependency

If the library is installed system-wide, include the namespaced header path:

```c
#include <debug-macros/debug.h>
```

## Building

```sh
# Library only (release)
meson setup build --buildtype=release
meson compile -C build

# With unit tests
meson setup build --buildtype=debug
meson compile -C build
meson test -C build --verbose
```

The test suite includes 31 unit tests covering basic logging, overflow detection,
callbacks, timestamps, log level filtering, and edge cases.

## Quick Start

```c
#include "debug.h"

void my_function(void)
{
        dbg_enter();
        dbg_info("Processing data %d", 42);
        dbg_assert(data != NULL);
        dbg_exit();
}

int main(void)
{
        /* Initialise debug system */
        dbg_init();

        /* Log messages */
        dbg_info("System starting");
        dbg_warn("Low battery");
        dbg_error("Critical failure");

        /* View buffer via JTAG */
        const char *log = dbg_get_buffer();
        size_t len = dbg_get_index();

        return 0;
}
```

## API Reference

### Configuration Macros

| Macro | Description |
|-------|-------------|
| `DEBUG` | Define to enable debugging; undefine for zero-overhead release builds |
| `DBG_BUFFER_SIZE` | Total buffer size in bytes (default: 2048) |
| `DBG_LINE_MAX` | Maximum log line length (default: 128) |
| `DBG_THREAD_SAFE` | Define for atomic operations (multi-core safety) |
| `DBG_WITH_TIMESTAMP` | Define to enable timestamp support |
| `DBG_WITH_LOG_LEVELS` | Define to enable log level filtering |

### Type Definitions

| Type | Description |
|------|-------------|
| `dbg_level_t` | Log severity level enum (TRACE, DEBUG, INFO, WARN, ERROR, NONE) |
| `dbg_overflow_callback_t` | Callback type for overflow events |
| `dbg_timestamp_callback_t` | Callback type for timestamp generation |

### Initialisation Functions

```c
void dbg_init(void);
void dbg_clear(void);
```

### Core Logging Functions

```c
void dbg_log(const char *fmt, ...);
const char *dbg_get_buffer(void);
size_t dbg_get_index(void);
```

### Overflow Detection

```c
uint32_t dbg_get_overflow_count(void);
void dbg_set_overflow_callback(dbg_overflow_callback_t cb);
```

### Timestamp Support (requires `DBG_WITH_TIMESTAMP`)

```c
void dbg_set_timestamp_callback(dbg_timestamp_callback_t cb);
```

### Buffer Query Functions

```c
size_t dbg_get_buffer_size(void);
size_t dbg_get_available_space(void);
size_t dbg_get_line_max(void);
```

### Log Level Functions (requires `DBG_WITH_LOG_LEVELS`)

```c
void dbg_set_level(dbg_level_t level);
dbg_level_t dbg_get_level(void);
```

### Log Level Enum

| Value | Description |
|-------|-------------|
| `DBG_LEVEL_TRACE` (0) | Verbose trace messages |
| `DBG_LEVEL_DEBUG` (1) | Debug-level messages |
| `DBG_LEVEL_INFO` (2) | Informational messages |
| `DBG_LEVEL_WARN` (3) | Warning messages |
| `DBG_LEVEL_ERROR` (4) | Error messages |
| `DBG_LEVEL_NONE` (5) | Disable all logging |

### Logging Macros

```c
dbg_info(fmt, ...)
    Log informational message with "INFO:  " prefix

dbg_warn(fmt, ...)
    Log warning message with "WARN:  " prefix

dbg_error(fmt, ...)
    Log error message with "ERROR: " prefix

dbg_trace(fmt, ...)
    Log trace message with "TRACE: " prefix

dbg_enter()
    Log function entry with "ENTER: function_name()"

dbg_exit()
    Log function exit with "EXIT:  function_name()"

dbg_assert(expr)
    Assert expression; log failure and halt via ESTOP0 if false
```

## Use Cases

### Basic Logging

```c
#include "debug.h"

void sensor_read(void)
{
        dbg_enter();
        
        int value = read_sensor();
        dbg_info("Sensor value: %d", value);
        
        dbg_assert(value >= 0 && value <= 1023);
        
        dbg_exit();
}
```

### Overflow Monitoring

```c
#include "debug.h"

static void overflow_handler(uint32_t count)
{
        /* Log to EEPROM or trigger alarm */
        dbg_error("Buffer overflow #%lu", (unsigned long)count);
        save_log_to_eeprom();
}

int main(void)
{
        dbg_init();
        dbg_set_overflow_callback(overflow_handler);
        
        /* ... application code ... */
        
        /* Check for overflows */
        if (dbg_get_overflow_count() > 0) {
                /* Handle lost log data */
        }
}
```

### Timestamped Logging

```c
#include "debug.h"

/* User-provided timestamp callback */
static const char *
timestamp_cb(char *buf, size_t buf_size)
{
        uint32_t ticks = get_system_ticks();
        snprintf(buf, buf_size, "[%lu]", (unsigned long)ticks);
        return buf;
}

int main(void)
{
        dbg_init();
        dbg_set_timestamp_callback(timestamp_cb);
        
        dbg_info("System started");
        /* Output: [12345] INFO:  System started */
}
```

### Log Level Filtering

```c
#include "debug.h"

#ifdef DBG_WITH_LOG_LEVELS

int main(void)
{
        dbg_init();
        
        /* Only show warnings and errors */
        dbg_set_level(DBG_LEVEL_WARN);
        
        dbg_trace("This won't appear");
        dbg_info("This won't appear either");
        dbg_warn("This will appear");
        dbg_error("This will also appear");
}

#endif
```

### JTAG Inspection

After a system halt or during debugging:

```c
/* In your debugging session */
const char *log_buffer = dbg_get_buffer();
size_t log_length = dbg_get_index();

/* In CCS Memory View:
 * - Address: dbg_get_buffer()
 * - Length: dbg_get_index()
 * - Format: ASCII
 */
```

## Advanced Configuration

### Custom Buffer Sizes

```c
/* Define before including debug.h */
#define DBG_BUFFER_SIZE (4096u)   /* 4KB buffer */
#define DBG_LINE_MAX    (256u)    /* 256 byte lines */
#include "debug.h"
```

### Thread-Safe Mode

```c
/* Define for multi-core systems */
#define DBG_THREAD_SAFE
#include "debug.h"
```

### Enable All Features

```c
#define DEBUG
#define DBG_THREAD_SAFE
#define DBG_WITH_TIMESTAMP
#define DBG_WITH_LOG_LEVELS
#include "debug.h"
```

## Notes

| Topic | Note |
|-------|------|
| **Buffer overflow** | Circular buffer wraps silently; use `dbg_get_overflow_count()` or callback to detect |
| **Thread safety** | When `DBG_THREAD_SAFE` defined: safe for concurrent reads with single writer; multiple writers must be externally serialized |
| **Line truncation** | Messages longer than `DBG_LINE_MAX` are truncated |
| **NULL safety** | `dbg_log()` safely handles NULL format strings |
| **Release builds** | Undefine `DEBUG` for zero-overhead release builds |
| **Initialisation** | Call `dbg_init()` once before any logging |
| **ESTOP0** | `dbg_assert()` uses TI C2000 `ESTOP0`; may need porting for other targets |
| **Callback context** | Overflow callback called synchronously; keep implementation minimal |
