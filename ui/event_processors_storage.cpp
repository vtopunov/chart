#include "event_processors_storage.h"


namespace ui
{
    namespace
    {
        struct by_window
        {
            window_handle_t window;

            constexpr bool operator () (const event_processor_note& value) const noexcept
            {
                return window == value.window;
            }
        };

        template<size_t N>
        size_t erase_for_window(small_vector<event_processor_note, N>& items, window_handle_t window, event_processor_note* first) noexcept
        {
            D_ASSERT(first >= items.begin());
            D_ASSERT(first <= items.end());

            return items.erase(std::remove_if(first, items.end(), by_window{ window }), items.cend());
        }

        template<size_t N>
        size_t erase_for_window(small_vector<event_processor_note, N>& items, window_handle_t window) noexcept
        {
            return erase_for_window(items, window, items.begin());
        }

        template<size_t N>
        bool destroy_processor_force(small_vector<event_processor_note, N>& items, event_processor_resource processor) noexcept
        {
            {
                const auto last = items.cend();

                for (auto it = items.cbegin(); it != last; ++it)
                {
                    if (it->processor == processor)
                    {
                        items.erase(it);
                        return true;
                    }
                }
            }

            return false;
        }
    }

    event_processors_storage& event_processors_global() noexcept
    {
        static event_processors_storage map;
        return map;
    }

    bool event_processors_storage::destroy_processor(event_processor_resource processor) noexcept
    {
        if (nulleventprocessor == processor)
        {
            return false;
        }

        if (lock_)
        {
            for (auto& item : items_)
            {
                if (item.processor == processor)
                {
                    update_first_garbage(std::addressof(item));
                    item.mark_as_garbage();
                    return true;
                }
            }

            return destroy_processor_force(back_items_, processor);
        }

        return destroy_processor_force(items_, processor);
    }

    size_t event_processors_storage::close_window(window_handle_t window) noexcept
    {
        size_t count{ 0_uz };

        if (lock_)
        {
            {
                const auto last = items_.end();
                const auto find = [window, last] (event_processor_note* first) noexcept
                {
                    return std::find_if(first, last, by_window{ window });
                };

                if (auto it = find(items_.begin()); last != it)
                {
                    update_first_garbage(it);

                    do
                    {
                        it->mark_as_garbage();
                        ++count; ++it;
                        it = find(it);
                    }
                    while (last != it);
                }
            }

            count += erase_for_window(back_items_, window);
        }
        else
        {
            count = erase_for_window(items_, window);
        }

        return count;
    }

    event_processor_resource event_processors_storage::create(window_handle_t window, event_callback_t callback) noexcept
    {
        if (event_processor_note::garbage_mark == window) [[unlikely]]
        {
            return event_processor_resource::null;
        }

        event_processor_note* item_opt{ nullptr };

        if (lock_ && items_.size() == items_.capacity()) [[unlikely]]
        {
            item_opt = back_items_.try_emplace_back
            (
                std::move(callback),
                window,
                event_processor_resource::null
            );
        }
        else [[likely]]
        {
            item_opt = items_.try_emplace_back
            (
                std::move(callback),
                window,
                event_processor_resource::null
            );
        }
        
        if (!item_opt) [[unlikely]]
        {
            return event_processor_resource::null;
        }

        const auto processor = desctiptor_generator_();
        item_opt->processor = processor;
        return processor;
    }

    void event_processors_storage::unlock_and_collecting() noexcept
    {
        if (lock_.unlock()) [[likely]]
        {
            if (first_garbage_) [[unlikely]]
            {
                erase_for_window
                (
                    items_,
                    event_processor_note::garbage_mark,
                    std::exchange(first_garbage_, nullptr)
                );
            }

            if (const auto back_size = back_items_.size()) [[unlikely]]
            {
                bool is_memory{ true };

                {
                    const auto old_capacity = items_.capacity();
                    const auto require_capacity = items_.size() + back_size;
                    if (old_capacity < require_capacity)
                    {
                         const auto new_capacity = std::max
                         (
                             require_capacity,
                             optimal_memory_growth(old_capacity)
                         );

                         is_memory = items_.try_reserve(new_capacity);
                         D_ASSERT(is_memory);
                    }
                }

                if (is_memory)
                {
                    for (auto& back_item : back_items_)
                    {
                        items_.emplace_back(std::move(back_item));
                    }
                }

                back_items_.clear();
            }
        }
    }

    void event_processors_storage::update_first_garbage(event_processor_note* new_value) noexcept
    {
        if (first_garbage_)
        {
            if (new_value < first_garbage_)
            {
                first_garbage_ = new_value;
            }
        }
        else
        {
            first_garbage_ = new_value;
        }
    }
}
