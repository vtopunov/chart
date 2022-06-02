#include <core/buffer_view.h>

#include <vector>
#include <array>
#include <string>

namespace
{
    template<bool immutable>
    bool test_impl_impl(basic_buffer_view<immutable> b, const void* data, size_t size) noexcept
    {
        using buffer_type = basic_buffer_view<immutable>;
        using byte_type = conditional_add_const_t<immutable, std::byte>;
        using void_type = conditional_add_const_t<immutable, void>;
        using word = uint16_t;
        using word_type = conditional_add_const_t<immutable, word>;
        using void_ptr = void_type*;
        using byte_ptr = byte_type*;
        using byte_cptr = const byte_type*;
        using byte_ref = byte_type&;
        using byte_cref = const byte_type&;
        using word_ptr = word_type*;
        using word_cptr = const word_type*;

        static_assert(std::is_same_v<typename buffer_type::element_type, byte_type>);
        static_assert(std::is_same_v<typename buffer_type::value_type, std::byte>);
        static_assert(std::is_same_v<typename buffer_type::data_pointer, void_ptr>);
        static_assert(std::is_same_v<typename buffer_type::pointer, byte_ptr>);
        static_assert(std::is_same_v<typename buffer_type::reference, byte_ref>);
        static_assert(std::is_same_v<typename buffer_type::const_reference, byte_cref>);
        static_assert(std::is_same_v<typename buffer_type::iterator, byte_ptr>);
        static_assert(std::is_same_v<typename buffer_type::const_iterator, byte_cptr>);

        static_assert(std::is_same_v<decltype(b.data()), void_ptr>);
        static_assert(std::is_same_v<decltype(b.as_bytes()), span<byte_type>>);
        static_assert(std::is_same_v<decltype(b.as_bytes_ptr()), byte_ptr>);
        static_assert(std::is_same_v<decltype(to_span<word>(b)), span<word_type>>);
        static_assert(std::is_same_v<decltype(to_ptr<word>(b)), word_ptr>);

        const auto bdata = b.data();
        const auto bsize = b.size();
        const auto bytes = b.as_bytes();
        const auto pbytes = b.as_bytes_ptr();
        const auto words = to_span<word>(b);
        const auto pwords = to_ptr<word>(b);
        const auto test_pbytes = static_cast<byte_cptr>(data);
        const auto test_pwords = static_cast<word_cptr>(data);

        D_ASSERT(bdata == data);
        D_ASSERT(bsize == size);
        D_ASSERT(bytes.data() == test_pbytes);
        D_ASSERT(bytes.size() == size);
        D_ASSERT(pbytes == test_pbytes);
        D_ASSERT(words.data() == test_pwords);
        D_ASSERT(words.size() == size / sizeof(word));
        D_ASSERT(pwords == test_pwords);
         
        static_assert(std::is_same_v<decltype(b.front()), byte_ref>);
        static_assert(std::is_same_v<decltype(b.cfront()), byte_cref>);
        static_assert(std::is_same_v<decltype(b.back()), byte_ref>);
        static_assert(std::is_same_v<decltype(b.cback()), byte_cref>);
        static_assert(std::is_same_v<decltype(b[0_uz]), byte_ref>);
        static_assert(std::is_same_v<decltype(b.begin()), byte_ptr>);
        static_assert(std::is_same_v<decltype(b.cbegin()), byte_cptr>);
        static_assert(std::is_same_v<decltype(b.end()), byte_ptr>);
        static_assert(std::is_same_v<decltype(b.cend()), byte_cptr>);

        auto& front = b.front();
        auto& cfront = b.cfront();
        auto& back = b.back();
        auto& cback = b.cback();
        auto& mean = b[size / 2];
        const auto begin = b.begin();
        const auto cbegin = b.cbegin();
        const auto end = b.end();
        const auto cend = b.cend();
        const auto test_end = test_pbytes + size;

        D_ASSERT(std::addressof(front) == test_pbytes);
        D_ASSERT(std::addressof(cfront) == test_pbytes);
        D_ASSERT(std::addressof(back) == std::prev(test_end));
        D_ASSERT(std::addressof(cback) == std::prev(test_end));
        D_ASSERT(std::addressof(mean) == test_pbytes + size / 2);
        D_ASSERT(begin == test_pbytes);
        D_ASSERT(cbegin == test_pbytes);
        D_ASSERT(end == test_end);
        D_ASSERT(cend == test_end);

        return immutable;
    }

    bool test_const_impl(const_buffer_view b, const void* data, size_t size) noexcept
    {
        static_assert(std::is_same_v<decltype(b.data()), const void*>);
        const auto immutable = test_impl_impl(b, data, size);
        D_ASSERT(immutable);
        return immutable;
    }

    bool test_mut_impl(buffer_view b, const void* data, size_t size) noexcept
    {
        static_assert(std::is_same_v<decltype(b.data()), void*>);
        const auto immutable_result = test_impl_impl(b, data, size);
        D_ASSERT(!immutable_result);

        D_ASSERT(test_const_impl(b, data, size));
        
        
        {
            buffer_view mb{};
            mb = b;
            D_ASSERT(test_const_impl(mb, data, size));
        }

        {
            const_buffer_view cb{};
            cb = b;
            D_ASSERT(test_const_impl(cb, data, size));
        }

        return immutable_result;
    }

    bool test_impl(const_buffer_view b, const void* data, size_t size) noexcept
    {
        return test_const_impl(b, data, size);
    }

    bool test_impl(buffer_view b, const void* data, size_t size) noexcept
    {
        return test_mut_impl(b, data, size);
    }

    template<class T>
    struct add_const_span
    {
        using type = std::add_const_t<T>;
    };

    template<class T, size_t N>
    struct add_const_span<span<T, N>>
    {
        using type = const span<const T, N>;
    };
   
    template<class T>
    struct add_const_span<const T> : add_const_span<T>
    {};

    template<class T>
    using add_const_span_t = typename add_const_span<T>::type;

    template<class T>
    struct remove_const_span
    {
        using type = std::remove_const_t<T>;
    };

    template<class T, size_t N>
    struct remove_const_span<span<T, N>>
    {
        using type = span<std::remove_const_t<T>, N>;
    };

    template<class T>
    struct remove_const_span<const T> : remove_const_span<T>
    {};

    template<class T>
    using remove_const_span_t = typename remove_const_span<T>::type;

    template<class C>
    constexpr size_t value_type_size_testimpl(const C& c) noexcept
    {
        using decayed_value_type = std::decay_t<std::remove_pointer_t<std::decay_t<decltype(std::data(c))>>>;
        if constexpr (std::is_same_v<decayed_value_type, void>)
        {
            return 1_uz;
        }
        else
        {
            return sizeof(decayed_value_type);
        }
    }

    template<class C>
    constexpr void test_static_asserts() noexcept
    {
        static_assert(!is_buffer_view_v<C>);
        static_assert(is_size_bytes_v<C>);
        static_assert(is_convertible_data<remove_const_span_t<C>, void*>::value);
        static_assert(is_convertible_data<remove_const_span_t<C>, const void*>::value);
        static_assert(!is_convertible_data<add_const_span_t<C>, void*>::value);
        static_assert(is_convertible_data<add_const_span_t<C>, const void*>::value);
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
            const_buffer_view cb{};
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
            buffer_view mb{};
            mb = c;
            const auto immutable = test_impl(mb, data, size);
            D_ASSERT(!immutable);
        }

        {
            const_buffer_view cb{};
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
            add_const_span_t<C> const_c{ c };
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
    static_assert(!std::is_same_v<buffer_view, const_buffer_view>);

    {
        using private_detail_size_bytes::size_of;

        static_assert(1_uz == size_of<void>());
        static_assert(1_uz == size_of<std::byte>());
        static_assert(4_uz == size_of<std::int32_t>());
    }

    {
        using private_detail_size_bytes::value_type_size;

        static_assert(1_uz == value_type_size<my_buffer>());
        static_assert(1_uz == value_type_size<const my_buffer>());
        static_assert(1_uz == value_type_size<cmy_buffer>());
        static_assert(1_uz == value_type_size<const cmy_buffer>());
        static_assert(1_uz == value_type_size<std::vector<char>>());
        static_assert(4_uz == value_type_size<std::vector<int32_t>>());
        static_assert(4_uz == value_type_size<std::array<int32_t, 1_uz>>());
        static_assert(4_uz == value_type_size<const std::array<int32_t, 1_uz>>());
        static_assert(4_uz == value_type_size<std::array<const int32_t, 1_uz>>());
        static_assert(4_uz == value_type_size<const std::array<const int32_t, 1_uz>>());
        static_assert(sizeof(ptrdiff_t) == value_type_size<const std::array<const int32_t*, 1_uz>>());
        static_assert(sizeof(ptrdiff_t) == value_type_size<const std::array<const int32_t*const, 1_uz>>());
    }

    {
        struct sbv
        {
            size_t size_bytes() const noexcept { return 321_uz; }
        };

        struct sv
        {
            size_t size() const noexcept { return 123_uz; }
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

        static_assert(is_size_bytes<sbv>::value);
        static_assert(!is_size_bytes<sv>::value);
        static_assert(!is_size_bytes<dv>::value);
        static_assert(!is_size_bytes<ddv>::value);
        static_assert(is_size_bytes<sdv>::value);
        static_assert(is_size_bytes<sddv>::value);
        static_assert(is_size_bytes<sbsdddv>::value);
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

