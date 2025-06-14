#pragma once

#include <string_view>

#include <core/narrow.h>


namespace private_detail_decode_utf
{
    template<size_t OctetCount, class U8, class Write>
    constexpr size_t decode_utf8(const U8* in, Write write) noexcept
    {
        static_assert(std::is_unsigned_v<U8> && sizeof(U8) == 1_uz);
        static_assert(OctetCount > 0_uz && OctetCount <= 4_uz);

        const auto code0 = *in;
        if (code0 < 0x80u)
        {
            write(code0);
            return 1u;
        }

        if constexpr (OctetCount > 1_uz)
        {
            if (code0 < 0xC2u)
            {
                return 1u;
            }

            constexpr auto offset1 = [](uint8_t code) noexcept
            {
                D_WARNING_PUSH;
                D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast);

                return static_cast<uint16_t>(code << 6);

                D_WARNING_POP;
            };

            if (code0 < 0xE0)
            {
                const auto code1 = *++in;
                if ((code1 & 0xC0) != 0x80u)
                {
                    return 1u;
                }

                constexpr uint16_t mark16{ 0x3080u };
                write(offset1(code0) + code1 - mark16);
                return 2u;
            }

            if constexpr (OctetCount > 2_uz)
            {
                constexpr auto offset2 = [](uint32_t code) noexcept
                {
                    return code << 12;
                };

                if (code0 < 0xF0u)
                {
                    const auto code1 = *++in;
                    if ((code1 & 0xC0) != 0x80u)
                    {
                        return 1u;
                    }

                    if (code0 == 0xE0u && code1 < 0xA0u)
                    {
                        return 1u;
                    }

                    const auto code2 = *++in;
                    if ((code2 & 0xC0u) != 0x80u)
                    {
                        return 1u;
                    }

                    constexpr uint32_t mark24{ 0xE2080ul };
                    write(offset2(code0) + offset1(code1) + code2 - mark24);
                    return 3u;
                }

                if constexpr (OctetCount > 3_uz)
                {
                    if (code0 < 0xF5u)
                    {
                        constexpr auto offset3 = [](uint32_t code) noexcept
                        {
                            return code << 18;
                        };

                        const auto code1 = *++in;
                        if ((code1 & 0xC0u) != 0x80u)
                        {
                            return 1u;
                        }

                        if (code0 == 0xF0u && code1 < 0x90u)
                        {
                            return 1u;
                        }

                        if (code0 == 0xF4u && code1 >= 0x90u)
                        {
                            return 1u;
                        }

                        const auto code2 = *++in;
                        if ((code2 & 0xC0u) != 0x80u)
                        {
                            return 1u;
                        }

                        const auto code3 = *++in;
                        if ((code3 & 0xC0u) != 0x80u)
                        {
                            return 1u;
                        }

                        constexpr uint32_t mark32{ 0x3C82080ul };
                        write(offset3(code0) + offset2(code1) + offset1(code2) + code3 - mark32);
                        return 4u;
                    }
                }
            }
        }

        return 1u;
    }

    template<size_t Size, class FirstIt, class LastIt>
    [[nodiscard]] constexpr bool in_size(FirstIt first, LastIt last) noexcept
    {
        static_assert(is_safe_narrowing_conversion<ptrdiff_t>(Size));

        D_WARNING_PUSH;
        D_WARNING_DISABLE_MSVC(W_do_not_use_static_cast);
        constexpr auto diff = static_cast<ptrdiff_t>(Size);
        D_WARNING_POP;

        return (last - first) >= diff;
    }

    template<size_t OctetCount, class U8, class Write>
    constexpr const U8* decode_utf8_finish(const U8* in, const U8* const end, Write write) noexcept
    {
        if constexpr (OctetCount > 0_uz)
        {
            if (in_size<OctetCount>(in, end))
            {
                in += decode_utf8<OctetCount>(in, write);
            }

            if constexpr (OctetCount > 1_uz)
            {
                in = decode_utf8_finish<OctetCount - 1_uz>(in, end, write);
            }
        }
        else
        {
            D_UNUSED(end);
            D_UNUSED(write);
        }

        return in;
    }
}

template<size_t OctetCount, class UChar, class Write>
constexpr const UChar* decode_utf(const UChar* in, const UChar* const end, Write write) noexcept
{
    using namespace private_detail_decode_utf;

    if constexpr (std::is_same_v<std::remove_cv_t<UChar>, char8_t>)
    {
        static_assert(OctetCount > 0_uz);

        while (in_size<OctetCount>(in, end))
        {
            in += decode_utf8<OctetCount>(in, write);
        }

        return decode_utf8_finish<OctetCount - 1_uz>(in, end, write);
    }
    else
    {
        // TODO: decode_utf16

        for (; in != end; ++in)
        {
            write(*in);
        }

        return in;
    }
}

template<size_t OctetCount, class UChar, class Write>
constexpr void decode_utf(std::basic_string_view<UChar> string, Write write) noexcept
{
    decode_utf<OctetCount>(string.data(), string.data() + string.size(), write);
}