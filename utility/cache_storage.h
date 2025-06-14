#pragma once

#include <core/small_vector.h>

template<class T, size_t N>
class cache_storage
{
public:
    constexpr cache_storage() noexcept = default;
    D_DISABLE_COPYMOVE_CA(cache_storage);

    using ref_count_type = int64_t;
    static_assert(std::is_signed_v<ref_count_type>);

    class value_type : public T
    {
    public:
        template<class... Args>
        constexpr value_type(Args&&... args) noexcept
            : T{ std::forward<Args>(args)... }
        {}

        constexpr void ref() noexcept
        {
            if (is_unused())
            {
                ref_count_ = 1;
            }
            else
            {
                ++ref_count_;
            }
        }

        [[nodiscard]]
        constexpr bool is_unused() const noexcept
        {
            return ref_count() < 1;
        }

        [[nodiscard]]
        constexpr ref_count_type ref_count() const noexcept
        {
            return ref_count_;
        }

        constexpr void deref(cache_storage& cache) noexcept
        {
            if (!is_unused())
            {
                --ref_count_;
                if (is_unused())
                {
                    ref_count_ = cache.unused_time_++;
                    D_ASSERT_OR_ASSUME(is_unused());
                }
            }
        }

    private:
        ref_count_type ref_count_{ 0 };
    };

    using storage_type = small_vector<value_type, N>;
    using pointer = typename storage_type::pointer;
    using const_pointer = typename storage_type::const_pointer;
    using reference = typename storage_type::reference;
    using const_reference = typename storage_type::const_reference;

    struct oldest_garbage_finder
    {
        const_pointer oldest_garbage{ nullptr };
        ref_count_type oldest_garbage_timemark{ 1 };

        constexpr void operator () (const_reference ref) noexcept
        {
            if (ref.ref_count() < oldest_garbage_timemark)
            {
                oldest_garbage_timemark = ref.ref_count();
                oldest_garbage = std::addressof(ref);
            }
        }

        constexpr operator pointer () const noexcept
        {
            return const_cast<pointer>(oldest_garbage);
        }
    };

    template<class... Args>
    [[nodiscard]] pointer try_emplace(Args&&... args) noexcept
    {
        static_assert(std::is_nothrow_constructible_v<value_type, Args...>);

        if (size() == capacity())
        {
            if (const auto garbage_for_rewrite = garbage())
            {
                std::destroy_at(garbage_for_rewrite);
                new (garbage_for_rewrite) value_type{ std::forward<Args>(args)... };
                return garbage_for_rewrite;
            }
        }

        return const_cast<pointer>(storage_.try_emplace_back(std::forward<Args>(args)...));
    }

    [[nodiscard]]
    constexpr size_t size() const noexcept
    {
        return storage_.size();
    }

    [[nodiscard]]
    constexpr size_t capacity() const noexcept
    {
        return storage_.capacity();
    }

    [[nodiscard]]
    constexpr size_t index(const_pointer p) const noexcept
    {
        const auto id = u_distance(storage_.cdata(), p);
        D_ASSERT_OR_ASSUME(id < storage_.size());
        return id;
    }

    [[nodiscard]]
    constexpr reference value(size_t index) const noexcept
    {
        return const_cast<reference>(storage_.value(index));
    }

    template<class F>
    [[nodiscard]] pointer select(const F& filter) const noexcept
    {
        const auto storage_cend = storage_.cend();
        const auto result = std::find_if(storage_.cbegin(), storage_cend, filter);
        return (result != storage_cend) ? const_cast<pointer>(result) : nullptr;
    }

    struct identical_or_garbage
    {
        pointer identical;
        pointer garbage;
    };

    template<class GF, class FF>
    [[nodiscard]] identical_or_garbage select_with_garbage(const GF& garbage_filter, const FF& final_filter) const noexcept
    {
        oldest_garbage_finder garbage_finder{};
        for (const auto& item : storage_)
        {
            if (garbage_filter(item))
            {
                if (final_filter(item))
                {
                    return
                    {
                        .identical{ const_cast<pointer>(std::addressof(item)) },
                        .garbage{ nullptr }
                    };
                }

                garbage_finder(item);
            }
        }

        return
        {
            .identical{ nullptr },
            .garbage{ garbage_finder }
        };
    }

private:
    [[nodiscard]]
    constexpr pointer garbage() const noexcept
    {
        return std::for_each(storage_.begin(), storage_.end(), oldest_garbage_finder{});
    }

private:
    storage_type storage_;
    ref_count_type unused_time_{ numeric_min_v<> };
};
