#include "unity.h"
#include "debug.h"

void setUp(void) { dbg_init(); }
void tearDown(void) { }

void test_dbg_info_does_not_crash(void)
{
    dbg_info("Test message %d", 42);
    TEST_ASSERT_NOT_NULL(dbg_get_buffer());
    TEST_ASSERT_GREATER_THAN(0, dbg_get_index());
}

int main(void)
{
    UNITY_BEGIN();
    RUN_TEST(test_dbg_info_does_not_crash);
    return UNITY_END();
}
