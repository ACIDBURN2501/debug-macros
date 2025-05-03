# Debug Module for Embedded Systems

[![Run Unity Tests](https://github.com/ACIDBURN2501/debug-macros/actions/workflows/test.yml/badge.svg)](https://github.com/ACIDBURN2501/debug-macros/actions/workflows/test.yml) [![Build Documentation](https://github.com/ACIDBURN2501/debug-macros/actions/workflows/docs.yml/badge.svg)](https://github.com/ACIDBURN2501/debug-macros/actions/workflows/docs.yml)

This module provides lightweight debug logging suitable for embedded systems
that lack traditional stdout or UART interfaces. Logging is stored in a
RAM-based circular buffer and can be inspected live via JTAG.
 
When the `DEBUG` macro is defined, functions and macros such as `dbg_info()`,
`dbg_assert()`, and `dbg_trace()` become active. If `DEBUG` is undefined,
all macros become no-ops via `((void)0)` for safe compilation.
 
The debug log is stored in a static character buffer. Macros like `dbg_info()`
write formatted strings into this buffer. The content can be retrieved via
`dbg_get_buffer()` and viewed using tools like CCS memory view.

## Features
- Ring buffer logging to RAM
- Macros for info, warning, error, trace
- `dbg_assert()` with `ESTOP0` trigger
- Optional EEPROM backup support (via user integration)
- Fully disabled in release builds via `#undef DEBUG`

## Integration
```c
#define DEBUG
#include "debug.h"
```

## Example Usage
```c
void foo(void) {
    dbg_enter();
    dbg_info("Running foo()");
    dbg_assert(1 == 1);
    dbg_exit();
}
```

## Building the Project with Meson

This project uses [Meson](https://mesonbuild.com) as the build system.

```bash
meson setup builddir
meson compile -C builddir
```

## Running Unit Tests with Meson

Ensure Unity is available via `subprojects/unity.wrap` (already configured).

```bash
meson test -C builddir --print-errorlogs
```

## Building the HTML Documentation with Sphinx

To generate HTML documentation from the source and Doxygen comments:

1. Generate Doxygen XML:
    ```bash
    cd doc/sphinx
    doxygen Doxyfile
    ```

2. Build Sphinx HTML:
    ```bash
    make html
    ```

3. Open the docs:
    ```
    doc/sphinx/_build/html/index.html
    ```

> Note: Ensure you have installed `doxygen`, `python3-sphinx`, and `breathe` packages.
