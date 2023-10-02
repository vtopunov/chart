#include <core/functional.h>

#include <cerrno>


void test_functional() noexcept
{
    D_ASSERT(!errno);
}