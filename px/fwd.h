#pragma once

#include <core/rectangle.h>
#include <core/margins.h>


using npx_t = uint32_t;
using npxf_t = realf_t;
using pxoff_t = int32_t;
static_assert(sizeof(pxoff_t) >= sizeof(npx_t));
static_assert(sizeof(npxf_t) >= sizeof(npx_t));

using pxvec = vec2<npx_t>;
using pxpoint = point2d<npx_t>;
using pxoffs = point2d<pxoff_t>;
using pxsizes = size2d<npx_t>;

using pxrectangle = rectangle<npx_t>;
using pxmargins = margins<npx_t>;

[[nodiscard]]
constexpr npx_t operator ""_npx(unsigned long long side) noexcept
{
    return narrow<npx_t>(side);
}

[[nodiscard]]
constexpr pxoff_t operator ""_pxoff(unsigned long long side) noexcept
{
    return narrow<pxoff_t>(static_cast<long long>(side));
}

constexpr pxsizes no_sizes{ 0_npx, 0_npx };
static_assert(md_is_eqnz(no_sizes));
static_assert(!no_sizes.has_positiven_mark());