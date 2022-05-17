#include <core/value_type.h>

#include <core/assert.h>

void test_value_type() noexcept
{
    {
        struct my_vector0
        {
            struct value_type
            {};
        };

        static_assert(std::is_same_v<decl_value_type_t<my_vector0>, typename my_vector0::value_type>);
        static_assert(std::is_same_v<value_type_t<my_vector0>, typename my_vector0::value_type>);
    }

    {
        struct my_vector1
        {
            struct value_type_impl
            {};

            constexpr const value_type_impl* data() const noexcept
            {
                return nullptr;
            }
        };

        using type = value_type_t<my_vector1>;

        static_assert(std::is_same_v<decl_data_pointer_t<my_vector1>, const typename my_vector1::value_type_impl*>);
        static_assert(std::is_same_v<value_type_t<my_vector1>, const typename my_vector1::value_type_impl>);
    }

    
    {
        struct my_vector2
        {
            struct value_type_impl
            {};

            constexpr const value_type_impl* data() const noexcept
            {
                return nullptr;
            }

            constexpr value_type_impl* data() noexcept
            {
                return nullptr;
            }
        };

        static_assert(std::is_same_v<decl_data_pointer_t<my_vector2>, typename my_vector2::value_type_impl*>);
        static_assert(std::is_same_v<decl_data_pointer_t<const my_vector2>, const typename my_vector2::value_type_impl*>);
        static_assert(std::is_same_v<value_type_t<my_vector2>, typename my_vector2::value_type_impl>);
        static_assert(std::is_same_v<value_type_t<const my_vector2>, const typename my_vector2::value_type_impl>);
    }
   
    D_ASSERT(!errno);
}