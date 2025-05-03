.. Debug Macro documentation master file

Debug Module
============

Debugging and logging utilities for embedded systems using JTAG-based
inspection.
 

This module provides lightweight debug logging suitable for embedded
systems that lack traditional stdout or UART interfaces. Logging is stored
in a RAM-based circular buffer and can be inspected live via JTAG.
 
When the `DEBUG` macro is defined, functions and macros such as
`dbg_info()`, `dbg_assert()`, and `dbg_trace()` become active. If `DEBUG`
is undefined, all macros become no-ops via `((void)0)` for safe compilation.
 
The debug log is stored in a static character buffer. Macros like
`dbg_info()` write formatted strings into this buffer. The content can be
retrieved via `dbg_get_buffer()` and viewed using tools like CCS memory
view.

.. toctree::
   :maxdepth: 2
   :caption: Contents:
   
   api
   usage
