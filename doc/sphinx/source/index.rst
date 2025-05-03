.. Debug Macro documentation master file

Debug Module
============

Debugging and logging utilities for embedded systems using JTAG-based
inspection.
 

This module provides lightweight debug logging suitable for embedded
systems that lack traditional stdout or UART interfaces. Logging is stored
in a RAM-based circular buffer and can be inspected live via JTAG.

.. toctree::
   :maxdepth: 2
   :caption: Contents:
   
   api
   usage
