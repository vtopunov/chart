#include <core/utility.h>
#include <core/assert.h>

#include <cerrno>

void test_utility() noexcept
{
    D_ASSERT(!errno);
}