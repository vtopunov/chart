#include <os/os.h>

#include <ui/event_processors_storage.h>


namespace
{
    struct collector
    {
        using counter_t = uint16_t;
        static constexpr auto max_copies = std::numeric_limits<counter_t>::max();
        static constexpr size_t max_values{ 16384_uz };

        ptrdiff_t max_id{ 0 };
        counter_t all_copies[max_values]{};

        [[nodiscard]]
        constexpr bool is_valid(ptrdiff_t id) const noexcept
        {
            return id >= 0 && id <= max_id;
        }

        constexpr void destroy(ptrdiff_t id) noexcept
        {
            D_ASSERT(is_valid(id));

            {
                auto& copies = all_copies[id];
                D_ASSERT(copies > 0);
                --copies;
            }
        }

        constexpr void create(ptrdiff_t id) noexcept
        {
            D_ASSERT(id >= 0 && id < max_values);
            max_id = std::max(max_id, id);

            {
                auto& copies = all_copies[id];
                D_ASSERT(!copies);
                ++copies;
            }
        }

        constexpr void dup(ptrdiff_t id) noexcept
        {
            D_ASSERT(is_valid(id));

            {
                auto& copies = all_copies[id];
                D_ASSERT(copies > 0 && copies < max_copies);
                ++copies;
            }
        }

        [[nodiscard]]
        constexpr bool equal(const collector& right) const  noexcept
        {
            return right.max_id == max_id &&
                !memcmp(all_copies, right.all_copies, (max_id + 1_uz) * sizeof(counter_t));
        }

        ~collector() noexcept
        {
            for (ptrdiff_t i = 0; i <= max_id; ++i)
            {
                D_ASSERT(!all_copies[i]);
            }
        }
    };

    struct test_callback
    {
        static constexpr ptrdiff_t invalid{ 0 };
        inline static collector collect{};

        ptrdiff_t id_;

        test_callback() noexcept
            : id_{ generate_unique() }
        {
            collect.create(id_);
        }

        constexpr test_callback(const test_callback& right) noexcept
            : id_{ right.id_ }
        {
            collect.dup(right.id_);
        }

        constexpr test_callback& operator = (const test_callback& right) noexcept
        {
            if (this != std::addressof(right))
            {
                collect.destroy(id_);
                collect.dup(right.id_);
                id_ = right.id_;
            }

            return *this;
        }

        constexpr test_callback(test_callback&& right) noexcept
            : id_{ std::exchange(right.id_, invalid) }
        {
            D_ASSERT(collect.is_valid(id_));
        }

        constexpr test_callback& operator = (test_callback&& right) noexcept
        {
            swap(right);
            return *this;
        }

        constexpr void swap(test_callback& right) noexcept
        {
            std::swap(id_, right.id_);
        }

        std::optional<ui::event_result_t> operator () (const ui::event&) noexcept
        {
            return std::nullopt;
        }

        ~test_callback() noexcept
        {
            if (id_ != invalid)
            {
                collect.destroy(id_);
            }
        }

        static ptrdiff_t generate_unique() noexcept
        {
            static ptrdiff_t unique{ invalid };
            return ++unique;
        }
    };

    [[nodiscard]]
    ui::window_handle_t generate_unique_window_handle() noexcept
    {
        static ui::window_handle_t unique{ nullptr };
        return ++unique;
    }

    struct test_countainer
    {
        static constexpr auto static_size = ui::event_processors_storage::static_size;

        struct note
        {
            ui::event_processor_resource processor;
            ui::window_handle_t window;
        };

        note items[static_size + 1u];
        size_t count{ 0u };

        note& create(ui::event_processors_storage& storage, ui::window_handle_t w) noexcept
        {
            auto& item = items[count++];
            item = { storage.create(w, test_callback{}), w };
            return item;
        }

        void test(const ui::event_processors_storage::locked_storage& lock) noexcept
        {
            size_t counter{ 0 };
            for (const auto& item : lock)
            {
                D_ASSERT(counter < count);
                const auto& test_item = items[counter];
                ++counter;

                D_ASSERT(test_item.processor == item.processor);
                D_ASSERT(test_item.window == item.window);
            }

            D_ASSERT(counter == count);
        }

        void destroy_processor_for_lock(ui::event_processors_storage& storage, ui::event_processor_resource processor) noexcept
        {
            for (size_t i = 0; i < count; ++i)
            {
                auto& item = items[i];
                if (processor == item.processor)
                {
                    item.window = nullptr;
                    break;
                }
            }

            D_ASSERT(storage.destroy_processor(processor));
        }

        void destroy_processor(ui::event_processors_storage& storage, ui::event_processor_resource processor) noexcept
        {
            D_ASSERT(1u == remove_test_if([processor] (const note& n) { return processor == n.processor; }));
            D_ASSERT(storage.destroy_processor(processor));
        }

        size_t close_window_for_lock(ui::event_processors_storage& storage, ui::window_handle_t window) noexcept
        {
            D_ASSERT(window);
            size_t counter{ 0 };
            for (size_t i = 0; i < count; ++i)
            {
                auto& item = items[i];
                if (window == item.window)
                {
                    item.window = nullptr;
                    ++counter;
                }
            }

            D_ASSERT(counter);
            D_ASSERT(counter == storage.close_window(window));

            return counter;
        }

        size_t close_window(ui::event_processors_storage& storage, ui::window_handle_t window) noexcept
        {
            D_ASSERT(window);
            const auto counter = remove_test_if([window] (const note& n) { return window == n.window; });
            D_ASSERT(counter == storage.close_window(window));
            return counter;
        }

        size_t collect() noexcept
        {
            return remove_test_if([] (const note& n) { return !n.window; });
        }

        template<class Pr>
        size_t remove_test_if(Pr pr) noexcept
        {
            const auto items_end = items + count;
            const auto count_of_removed = narrow<size_t>(items_end - std::remove_if(items, items_end, pr));
            D_ASSERT(count_of_removed <= count);
            count -= count_of_removed;
            return count_of_removed;
        }
    };
}


void test_event_processors_storage() noexcept
{
    constexpr ui::window_handle_t null_w{ nullptr };

    const ui::window_handle_t w[]
    {
        null_w + 1,
        null_w + 2,
        null_w + 3
    };

    ui::event_processors_storage storage{};
    test_countainer test{};

    for (size_t i = 0u; i < storage.static_size; ++i)
    {
        test.create(storage, w[i % std::size(w)]);
    }

    {
        {
            const auto lock = storage.lock();
            const auto processor = test.create(storage, w[0]).processor;

            size_t count{ 0 };
            for (const auto& item : lock)
            {
                D_ASSERT(processor != item.processor);
                ++count;
            }

            D_ASSERT(count + 1u == test.count);
        }

        test.test(storage.lock());
    }


    {
        {
            const auto lock0 = storage.lock();
            {
                const auto lock1 = storage.lock();
                test.destroy_processor_for_lock(storage, lock1.begin()->processor);
                test.test(lock1);
            }
            test.test(lock0);
        }

        D_ASSERT(1u == test.collect());
        test.test(storage.lock());
    }

    {
        test.destroy_processor(storage, test.items->processor);
        test.test(storage.lock());
    }

    {
        size_t count{ 0 };
        {
            const auto lock0 = storage.lock();
            const auto lock1 = storage.lock();
            {
                const auto lock2 = storage.lock();
                count = test.close_window_for_lock(storage, lock2.begin()->window);
                test.test(lock2);
            }
            test.test(lock0);
        }

        D_ASSERT(count == test.collect());
        test.test(storage.lock());
    }

    {
        test.close_window(storage, test.items->window);
        test.test(storage.lock());
    }
}