#include <shader/library.h>

#include <memory>

namespace
{
    struct dummy_exports
    {
        static constexpr dummy apply(no_overload) noexcept
        {
            return dummy_v;
        }

        template<class>
        struct interface {};
    };

    struct dummy_vertex_shader
    {
        static constexpr auto source = ""_vert_glsl;
        using exports = dummy_exports;
    };

    struct dummy_fragment_shader
    {
        static constexpr auto source = ""_frag_glsl;
        using exports = dummy_exports;
    };

    using dummy_shader_libarary = shader_library<dummy_vertex_shader, dummy_fragment_shader>;

    using dummy_exports_pack = ttypes<dummy_exports>;

    using dummy_interface = typename dummy_exports::template interface<dummy_shader_libarary>;

    struct dummy_uniform : shader_export::uniform_base
    {};

    struct dummy_uniforms_exports
    {
        static constexpr dummy_uniform u0{ "u0"_zsv };
        static constexpr dummy_uniform u1{ "u1"_zsv };
        static constexpr dummy_uniform u2{ "u2"_zsv };
        static constexpr dummy_uniform u3{ "u3"_zsv };

        template<class Fn>
        static constexpr decltype(auto) apply(Fn&& fn) noexcept
        {
            return std::forward<Fn>(fn)(u0, u1, u2, u3);
        }
    };

    static constexpr auto dummy_pu0 = &(dummy_uniforms_exports::u0);
    static constexpr auto dummy_pu1 = &(dummy_uniforms_exports::u1);
    static constexpr auto dummy_pu2 = &(dummy_uniforms_exports::u2);
    static constexpr auto dummy_pu3 = &(dummy_uniforms_exports::u3);

    static constexpr std::array<const shader_export::uniform_base*, 4u> dummy_uniforms_v
    {
        {
            dummy_pu0,
            dummy_pu1,
            dummy_pu2,
            dummy_pu3
        }
    };

    constexpr struct forward_as_tuple_function
    {
        template<class... Args>
        constexpr auto operator () (Args&&... args) const noexcept
        {
            return std::forward_as_tuple(std::forward<Args>(args)...);
        }
    } forward_as_tuple_function_v{};

    static constexpr auto dummy_uniform_tuple_v = dummy_uniforms_exports::apply(forward_as_tuple_function_v);
}


int main() noexcept
{
    static_assert(std::is_same_v<dummy_exports_pack, shader_common::decl_exports_pack_t<dummy_shader_libarary> >);
    static_assert(std::is_same_v<dummy_exports, shader_common::decl_exports_t<dummy_vertex_shader> >);
    static_assert(std::is_same_v<dummy_exports, shader_common::decl_exports_t<dummy_fragment_shader> >);
    static_assert(std::is_same_v<dummy_interface, shader_common::decl_interface_t<dummy_exports, dummy_shader_libarary> >);

    static_assert(!shader_common::is_sampler_v<shader_export::pxfvec>);
    static_assert(shader_common::is_sampler_v<shader_export::sampler>);
    static_assert(!shader_common::is_sampler_v<shader_export::frame>);

    static_assert(shader_common::has_indexed_location_v<shader_export::pxfvec>);
    static_assert(!shader_common::has_indexed_location_v<shader_export::sampler>);
    static_assert(!shader_common::has_indexed_location_v<shader_export::frame>);

    static_assert(!shader_common::is_not_uniform_v<shader_export::pxfvec>);
    static_assert(!shader_common::is_not_uniform_v<shader_export::sampler>);
    static_assert(shader_common::is_not_uniform_v<shader_export::frame>);

    static_assert(std::is_same_v<ttypes<>, shader_common::export_pack_variables_t<dummy_exports_pack> >);
    static_assert(0_uz == shader_common::count_of_for_v<shader_common::is_sampler, dummy_exports_pack>);
    static_assert(0_uz == shader_common::count_of_for_v<shader_common::has_indexed_location, dummy_exports_pack>);
    static_assert(0_uz == shader_common::count_of_for_v<shader_common::is_not_uniform, dummy_exports_pack>);
    static_assert(0_uz == shader_common::count_of_locations_v<dummy_exports_pack>);

    {
        using namespace shader_common::private_detail_shader_common::private_detail_interface_conversion;

        {
            constexpr auto sz_t = sizeof(is_empty_base_helper<dummy>::empty_base_test_type);
            constexpr auto sz_s = sizeof(empty_base_sample_type);
            static_assert(sz_t == sz_s);
        }

        {
            constexpr auto a_t = alignof(is_empty_base_helper<dummy>::empty_base_test_type);
            constexpr auto a_s = alignof(empty_base_sample_type);
            static_assert(a_t == a_s);
        }

        static_assert(is_empty_base_v<dummy>);
    }

    static_assert(dummy_pu0 == dummy_uniforms_v[0]);
    static_assert(dummy_pu1 == dummy_uniforms_v[1]);
    static_assert(dummy_pu2 == dummy_uniforms_v[2]);
    static_assert(dummy_pu3 == dummy_uniforms_v[3]);

    static_assert(0_uz == find_n(make_span(dummy_uniforms_v), dummy_pu0));
    static_assert(1_uz == find_n(make_span(dummy_uniforms_v), dummy_pu1));
    static_assert(2_uz == find_n(make_span(dummy_uniforms_v), dummy_pu2));
    static_assert(3_uz == find_n(make_span(dummy_uniforms_v), dummy_pu3));

    static_assert(dummy_pu0 == &std::get<0>(dummy_uniform_tuple_v));
    static_assert(dummy_pu1 == &std::get<1>(dummy_uniform_tuple_v));
    static_assert(dummy_pu2 == &std::get<2>(dummy_uniform_tuple_v));
    static_assert(dummy_pu3 == &std::get<3>(dummy_uniform_tuple_v));

    return 0;
}
