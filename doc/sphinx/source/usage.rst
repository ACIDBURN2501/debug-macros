Usage Examples
==============

This page provides practical examples of using the `debug` module's macros and functions
in real embedded applications. These are especially helpful for inspecting system behaviour
over JTAG by reviewing the log buffer contents in memory.

Basic Logging
-------------

.. code-block:: c

    #include "debug.h"

    void init_system(void)
    {
        dbg_info("System initialisation started");
    }

    void control_loop(void)
    {
        dbg_enter();

        dbg_trace("Reading sensor values...");
        // Simulated operation
        int val = 42;
        dbg_info("Sensor value = %d", val);

        dbg_exit();
    }

Assertions
----------

.. code-block:: c

    void check_bounds(int index)
    {
        dbg_assert(index >= 0 && index < MAX_ENTRIES);
        dbg_info("Accessing index %d", index);
    }

    // If the assertion fails, ESTOP0 is triggered and a message is logged.

Viewing the Log Buffer
----------------------

After a system pause via JTAG, use your memory browser (e.g., in Code Composer Studio)
to inspect the log:

- Buffer base: ``dbg_get_buffer()``
- Length used: ``dbg_get_index()``

The log is stored as a newline-separated ASCII buffer.

Clearing or Initialising the Log
--------------------------------

.. code-block:: c

    void reset_logs(void)
    {
        dbg_clear();  // same as dbg_init()
    }

Saving to EEPROM (Optional)
---------------------------

If desired, you may save the buffer to EEPROM on fault:

.. code-block:: c

    void fault_handler(void)
    {
        dbg_error("FAULT detected");
        save_debug_log_to_eeprom();
        system_reset();
    }

    // save_debug_log_to_eeprom() is a user-defined integration.


