#include <core/value_type.h>

#include <core/assert.h>

#include <cerrno>

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
        static_assert(!is_data_pointer<my_vector0>::value);
        static_assert(!is_data_pointer<const my_vector0&>::value);
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

        static_assert(std::is_same_v<decl_data_pointer_t<my_vector1>, const typename my_vector1::value_type_impl*>);
        static_assert(std::is_same_v<value_type_t<my_vector1>, const typename my_vector1::value_type_impl>);
        static_assert(is_data_pointer<my_vector1>::value);
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
        static_assert(is_data_pointer<my_vector2>::value);
    }
   
    {
        struct my_vector3
        {
            struct value_type
            {
            };

            struct data_value_type
            {
            };

            constexpr const data_value_type* data() const noexcept
            {
                return nullptr;
            }

            constexpr data_value_type* data() noexcept
            {
                return nullptr;
            }
        };

        static_assert(std::is_same_v<decl_data_pointer_t<my_vector3>, typename my_vector3::data_value_type*>);
        static_assert(std::is_same_v<decl_data_pointer_t<const my_vector3>, const typename my_vector3::data_value_type*>);
        static_assert(std::is_same_v<typename private_detail_value_type::value_type_by_data_pointer<my_vector3>::type, typename my_vector3::data_value_type>);
        static_assert(std::is_same_v<value_type_t<my_vector3>, typename my_vector3::value_type>);
        static_assert(std::is_same_v<value_type_t<const my_vector3>, typename my_vector3::value_type>);
        static_assert(is_data_pointer<my_vector3>::value);
        static_assert(is_data_pointer<const my_vector3&>::value);
    }

    {
        static_assert(is_data_pointer<int[3]>::value);
        static_assert(std::is_same_v<value_type_t<int[3]>, int>);
    }

    D_ASSERT(!errno);
}