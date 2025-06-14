#include <core/buffer_view.h>
#include <core/span.h>

#include <vector>
#include <array>
#include <string>


namespace
{
    template<class ValueType>
    bool test_impl_impl(basic_buffer_view<ValueType> b, const void* data, size_t size) noexcept
    {
        using buffer_view_type = basic_buffer_view<ValueType>;
        constexpr bool immutable = std::is_const_v<ValueType>;
        static_assert(immutable == buffer_view_type::immutable);

        using byte_type = conditional_add_const_t<immutable, std::byte>;
        using void_type = conditional_add_const_t<immutable, void>;
        using word = uint16_t;
        using word_type = conditional_add_const_t<immutable, word>;

        static_assert(std::is_same_v<typename buffer_view_type::value_type, ValueType>);
        static_assert(std::is_same_v<typename buffer_view_type::void_pointer, void_type*>);
        static_assert(std::is_same_v<typename buffer_view_type::pointer, ValueType*>);
        static_assert(std::is_same_v<typename buffer_view_type::reference, ValueType&>);
        static_assert(std::is_same_v<typename buffer_view_type::const_reference, const ValueType&>);
        static_assert(std::is_same_v<typename buffer_view_type::iterator, ValueType*>);
        static_assert(std::is_same_v<typename buffer_view_type::const_iterator, const ValueType*>);

        static_assert(std::is_same_v<decltype(b.void_data()), void_type*>);
        static_assert(std::is_same_v<decltype(b.data()), ValueType*>);
        static_assert(std::is_same_v<decltype(interpret<std::byte>(b)), basic_buffer_view<byte_type>>);
        static_assert(std::is_same_v<decltype(interpret<word>(b)), basic_buffer_view<word_type>>);

        const auto test_size_bytes = sizeof(ValueType) * size;
        const buffer_view_type right_b{ memory_construct, const_cast<void_type*>(data), test_size_bytes };
        const buffer_view_type right_data_b{ memory_construct, const_cast<void_type*>(data), 0 };
        const buffer_view_type right_size_b{ memory_construct, static_cast<void_type*>(nullptr), test_size_bytes };
        const auto bdata = b.data();
        const auto bsize = b.size();
        const auto bytes = interpret<std::byte>(b);
        const auto words = interpret<word>(b);

        D_ASSERT(b == right_b);
        D_ASSERT(right_b == b);
        D_ASSERT(!(b != right_b));
        D_ASSERT(!(right_b != b));
        D_ASSERT(!(b == right_data_b));
        D_ASSERT(!(right_data_b == b));
        D_ASSERT(b != right_data_b);
        D_ASSERT(right_data_b != b);
        D_ASSERT(!(b == right_size_b));
        D_ASSERT(!(right_size_b == b));
        D_ASSERT(b != right_size_b);
        D_ASSERT(right_size_b != b);

        D_ASSERT(bdata == data);
        D_ASSERT(bsize == size);
        D_ASSERT(bytes.data() == data);
        D_ASSERT(bytes.size() == test_size_bytes);
        D_ASSERT(words.data() == data);
        D_ASSERT(words.size() == test_size_bytes / sizeof(word));

        static_assert(std::is_same_v<decltype(b.front()), ValueType&>);
        static_assert(std::is_same_v<decltype(b.cfront()), const ValueType&>);
        static_assert(std::is_same_v<decltype(b.back()), ValueType&>);
        static_assert(std::is_same_v<decltype(b.cback()), const ValueType&>);
        static_assert(std::is_same_v<decltype(b[0u]), ValueType&>);
        static_assert(std::is_same_v<decltype(b.begin()), ValueType*>);
        static_assert(std::is_same_v<decltype(b.cbegin()), const ValueType*>);
        static_assert(std::is_same_v<decltype(b.end()), ValueType*>);
        static_assert(std::is_same_v<decltype(b.cend()), const ValueType*>);
        static_assert(std::is_same_v<decltype(bytes.data()), byte_type*>);
        static_assert(std::is_same_v<decltype(words.data()), word_type*>);

        auto& front = b.front();
        auto& cfront = b.cfront();
        auto& back = b.back();
        auto& cback = b.cback();
        auto& mean = b[size / 2];
        const auto begin = b.begin();
        const auto cbegin = b.cbegin();
        const auto end = b.end();
        const auto cend = b.cend();
        const auto test_cbegin = static_cast<const ValueType*>(data);
        const auto test_cend = test_cbegin + size;

        D_ASSERT(std::addressof(front) == test_cbegin);
        D_ASSERT(std::addressof(cfront) == test_cbegin);
        D_ASSERT(std::addressof(back) == std::prev(test_cend));
        D_ASSERT(std::addressof(cback) == std::prev(test_cend));
        D_ASSERT(std::addressof(mean) == test_cbegin + size / 2);
        D_ASSERT(begin == test_cbegin);
        D_ASSERT(cbegin == test_cbegin);
        D_ASSERT(end == test_cend);
        D_ASSERT(cend == test_cend);

        return immutable;
    }

    bool test_const_impl(const_byte_buffer_view b, const void* data, size_t size) noexcept
    {
        static_assert(std::is_same_v<decltype(b.data()), const std::byte*>);
        static_assert(std::is_same_v<decltype(b.void_data()), const void*>);
        const auto immutable = test_impl_impl(b, data, size);
        D_ASSERT(immutable);
        return immutable;
    }

    bool test_mut_impl(byte_buffer_view b, const void* data, size_t size) noexcept
    {
        static_assert(std::is_same_v<decltype(b.data()), std::byte*>);
        static_assert(std::is_same_v<decltype(b.void_data()), void*>);
        const auto immutable_result = test_impl_impl(b, data, size);
        D_ASSERT(!immutable_result);

        D_ASSERT(test_const_impl(b, data, size));


        {
            byte_buffer_view mb{};
            mb = b;
            D_ASSERT(test_const_impl(mb, data, size));
        }

        {
            const_byte_buffer_view cb{};
            cb = b;
            D_ASSERT(test_const_impl(cb, data, size));
        }

        return immutable_result;
    }

    bool test_impl(const_byte_buffer_view b, const void* data, size_t size) noexcept
    {
        return test_const_impl(b, data, size);
    }

    bool test_impl(byte_buffer_view b, const void* data, size_t size) noexcept
    {
        return test_mut_impl(b, data, size);
    }


    template<class T>
    struct add_const_container_or_span
    {
        using type = std::add_const_t<T>;
    };

    template<class T, size_t N>
    struct add_const_container_or_span<span<T, N>>
    {
        using type = const span<const T, N>;
    };

    template<class T>
    struct add_const_container_or_span<const T> : add_const_container_or_span<T>
    {};

    template<class T>
    using add_const_container_or_span_t = typename add_const_container_or_span<T>::type;

    template<class T>
    struct remove_const_container_or_span
    {
        using type = std::remove_const_t<T>;
    };

    template<class T, size_t N>
    struct remove_const_container_or_span<span<T, N>>
    {
        using type = span<std::remove_const_t<T>, N>;
    };

    template<class T>
    struct remove_const_container_or_span<const T> : remove_const_container_or_span<T>
    {};

    template<class T>
    using remove_const_container_or_span_t = typename remove_const_container_or_span<T>::type;



    template<class C>
    constexpr void test_static_asserts() noexcept
    {
        static_assert(!private_detail_buffer_view::has_void_data<C>::value);
        static_assert(has_size_bytes_v<C>);
        static_assert(has_std_data_void_compatible<false, remove_const_container_or_span_t<C>>::value);
        static_assert(has_std_data_void_compatible<true, remove_const_container_or_span_t<C>>::value);
        static_assert(!has_std_data_void_compatible<false, add_const_container_or_span_t<C>>::value);
        static_assert(has_std_data_void_compatible<true, add_const_container_or_span_t<C>>::value);
    }

    template<class C>
    constexpr size_t value_type_size_testimpl(const C& c) noexcept
    {
        using decayed_value_type = std::decay_t<std::remove_pointer_t<std::decay_t<decltype(std::data(c))>>>;
        if constexpr (std::is_same_v<decayed_value_type, void>)
        {
            return 1u;
        }
        else
        {
            return sizeof(decayed_value_type);
        }
    }

    template<class C>
    void test(const C& c) noexcept
    {
        test_static_asserts<C>();

        const void* data{ std::data(c) };
        size_t size{ size_bytes(c) };
        D_ASSERT(size == std::size(c) * value_type_size_testimpl(c));

        {
            const auto success_overload_and_select_immutable = test_impl(c, data, size);
            D_ASSERT(success_overload_and_select_immutable);
        }

        if constexpr (is_span_v<C>)
        {
            D_ASSERT(test_const_impl(C(c), data, size));
        }

        {
            const_byte_buffer_view cb{};
            cb = c;
            const auto immutable = test_impl(c, data, size);
            D_ASSERT(immutable);
        }
    }

    template<class C>
    void test(C& c) noexcept
    {
        test_static_asserts<C>();

        const void* data{ std::data(c) };
        size_t size{ size_bytes(c) };
        D_ASSERT(size == std::size(c) * value_type_size_testimpl(c));

        {
            const auto immutable = test_mut_impl(c, data, size);
            D_ASSERT(!immutable);
        }

        {
            byte_buffer_view mb{};
            mb = c;
            const auto immutable = test_impl(mb, data, size);
            D_ASSERT(!immutable);
        }

        {
            const_byte_buffer_view cb{};
            cb = c;
            const auto immutable = test_impl(cb, data, size);
            D_ASSERT(immutable);
        }

        if constexpr (std::is_array_v<C>)
        {
            test(std::as_const(c));
        }
        else
        {
            add_const_container_or_span_t<C> const_c{ c };
            test(const_c);
        }
    }

    template<class T, size_t n>
    void test_span(span<T, n> c) noexcept
    {
        conditional_add_const_t<std::is_const_v<T>, span<T, n>>& ref = c;

        test(ref);
    }

    struct my_buffer
    {
        uint8_t bytes[5]{ 1, 2, 3, 4, 5 };

        constexpr const void* data() const noexcept
        {
            return bytes;
        }

        constexpr void* data() noexcept
        {
            return bytes;
        }

        constexpr size_t size() const noexcept
        {
            return std::size(bytes);
        }
    };

    struct cmy_buffer
    {
        static constexpr uint8_t bytes[5]{ 1, 2, 3, 4, 5 };

        constexpr const void* data() const noexcept
        {
            return bytes;
        }

        constexpr size_t size() const noexcept
        {
            return std::size(bytes);
        }
    };
}

void test_buffer_view() noexcept
{
    static_assert(!std::is_same_v<byte_buffer_view, const_byte_buffer_view>);
    static_assert(std::is_same_v<decl_view_type_t<byte_buffer_view>, byte_buffer_view::view_type>);
    static_assert(std::is_same_v<decl_null_type_t<byte_buffer_view>, byte_buffer_view::null_type>);
    static_assert(std::is_same_v<decl_null_type_t<byte_buffer_view>, nullmem_t>);
    static_assert(std::is_trivially_copyable_v<byte_buffer_view>);

    {
        static_assert(1_uz == sizeof_v<void>);
        static_assert(1_uz == sizeof_v<std::byte>);
        static_assert(4_uz == sizeof_v<std::int32_t>);
    }

    {
        struct sbv
        {
            size_t size_bytes() const noexcept { return 321u; }
        };

        struct sv
        {
            size_t size() const noexcept { return 123u; }
        };

        struct dv
        {
            using value_type = std::array<char, 11_uz>;
        };

        struct ddv
        {
            using v_t = std::array<char, 15_uz>;
            const v_t* data() const noexcept { return nullptr; }
        };

        struct sdv : sv, dv
        {};

        struct sddv : sv, ddv
        {};

        struct sbsdddv : sbv, sv, dv, ddv
        {};

        static_assert(has_size_bytes<sbv>::value);
        static_assert(!has_size_bytes<sv>::value);
        static_assert(!has_size_bytes<dv>::value);
        static_assert(!has_size_bytes<ddv>::value);
        static_assert(has_size_bytes<sdv>::value);
        static_assert(has_size_bytes<sddv>::value);
        static_assert(has_size_bytes<sbsdddv>::value);
    }

    {
        std::vector v{ 1, 2, 3, 4, 5 };
        std::array a{ 1, 2, 3, 4, 5 };
        std::u16string s{ u"12345" };
        int m[]{ 1, 2, 3, 4, 5 };
        my_buffer my{};
        span ispm{ m };
        span ispv{ v };

        test(v);
        test(a);
        test(s);
        test(m);
        test(my);
        test_span(ispm);
        test_span(ispv);
    }

    {
        const std::vector cv{ 1, 2, 3, 4, 5 };
        constexpr std::array ca{ 1, 2, 3, 4, 5 };
        const std::u16string cs{ u"12345" };
        constexpr int cm[]{ 1, 2, 3, 4, 5 };
        constexpr my_buffer c_my{};
        const span cispm{ cm };
        const span cispv{ cv };

        test(cv);
        test(ca);
        test(cs);
        test(cm);
        test(c_my);
        test_span(cispm);
        test_span(cispv);
    }
}

