#include <mimalloc.h>

#include <any>
#include <print>

namespace
{
    struct xz_struct
    {
        ~xz_struct() noexcept
        {
            std::print("xz_struct dtor");
        }
    };
}

int main() noexcept 
{
    {
        const xz_struct*const p_xz{ new xz_struct };
        std::destroy_at(p_xz);
        free(const_cast<void*>(static_cast<const void*>(p_xz)));
    }

    std::is_nothrow_constructible_v<std::any>;
    std::any axz;
    const auto xzg = mi_good_size(333333);
    const auto xz = mi_malloc(333333);
    const auto xzzz = mi_malloc_size(xz);
    const auto xzz = mi_usable_size(xz);
    mi_free(mi_realloc(xz,3333));
    mi_free_size(mi_malloc(333), 333);
    return !!xz + 0;
}