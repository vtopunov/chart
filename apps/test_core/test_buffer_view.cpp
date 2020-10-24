#include <core/buffer_view.h>

#include <vector>
#include <array>
#include <string>

namespace
{
    template<bool immutable>
    bool test_impl_impl(basic_buffer_view<immutable> b, const void* data, size_t size) noexcept
    {
        static_assert(std::is_same_v<add_const_if_t<true, void>, const void>);
        static_assert(std::is_same_v<add_const_if_t<true, std::byte>, const std::byte>);
        static_assert(std::is_same_v<add_const_if_t<false, void>, void>);
        static_assert(std::is_same_v<add_const_if_t<false, std::byte>, std::byte>);

        using buffer_type = basic_buffer_view<immutable>;
        using byte_type = add_const_if_t<immutable, std::byte>;
        using void_type = add_const_if_t<immutable, void>;
        using word = uint16_t;
        using word_type = add_const_if_t<immutable, word>;
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
        static_assert(std::is_same_v<decltype(b.as_bytes()), std::span<byte_type>>);
        static_assert(std::is_same_v<decltype(b.as_bytes_ptr()), byte_ptr>);
        static_assert(std::is_same_v<decltype(b.as_span<word>()), std::span<word_type>>);
        static_assert(std::is_same_v<decltype(b.as_ptr<word>()), word_ptr>);

        const auto bdata = b.data();
        const auto bsize = b.size();
        const auto bytes = b.as_bytes();
        const auto pbytes = b.as_bytes_ptr();
        const auto words = b.as_span<word>();
        const auto pwords = b.as_ptr<word>();
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
        static_assert(std::is_same_v<decltype(b[0u]), byte_ref>);
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

    template<class C>
    void test(const C& c) noexcept
    {
        static_assert(is_size_bytes_v<C>);
        static_assert(is_convertible_data<C, void*>::value);
        static_assert(is_convertible_data<C, const void*>::value);
        static_assert(!is_convertible_data<const C, void*>::value);
        static_assert(is_convertible_data<const C, const void*>::value);

        const void* data{ std::data(c) };
        size_t size{ size_bytes(c) };
        D_ASSERT(size / sizeof(*std::data(c)) == std::size(c));

        {
            const auto success_overload_and_select_immutable = test_impl(c, data, size);
            D_ASSERT(success_overload_and_select_immutable);
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
        static_assert(is_size_bytes_v<C>);
        static_assert(is_convertible_data<C, void*>::value);
        static_assert(is_convertible_data<C, const void*>::value);
        static_assert(!is_convertible_data<const C, void*>::value);
        static_assert(is_convertible_data<const C, const void*>::value);

        const void* data{ std::data(c) };
        size_t size{ size_bytes(c) };
        D_ASSERT(size / sizeof(*std::data(c)) == std::size(c));

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

        test(std::as_const(c));
    }
}

void test_buffer_view() noexcept
{
    static_assert(!std::is_same_v<buffer_view, const_buffer_view>);

    std::vector v{ 1, 2, 3, 4, 5 };
    std::array a{ 1, 2, 3, 4, 5 };
    std::u16string s{ u"12345" };
    int m[]{ 1, 2, 3, 4, 5 };

    test(v);
    test(a);
    test(s);
    test(m);
}

