#include "event_processors_container.h"

namespace ui
{
    namespace
    {
        event_processor_resource new_event_processor_description() noexcept
        {
            static auto current = to_underlying(event_processor_resource::null);
            return safe_numeric_cast<event_processor_resource>(++current);
        }
    }

    event_processors_container& event_processors_global() noexcept
    {
        static event_processors_container map;
        return map;
    }

    bool event_processors_container::erase(event_processor_resource processor) noexcept
    {
        const auto last = items_.cend();

        for (auto it = items_.begin(); it != last; ++it)
        {
            if (it->processor == processor)
            {
                if (lock_)
                {
                    it->mark_as_garbage();
                }
                else
                {
                    items_.erase(it);
                }

                return true;
            }
        }

        return false;
    }

    size_t event_processors_container::erase(window_handle_t window) noexcept
    {
        size_t count{ 0_uz };

        if (lock_)
        {
            for (auto& item : items_)
            {
                if (item.window == window)
                {
                    item.mark_as_garbage();
                    ++count;
                }
            }
        }
        else
        {
            count = unlock_erase(window);
        }

        return count;
    }

    void event_processors_container::reset() noexcept
    {
        if (lock_)
        {
            for (auto& item : items_)
            {
                item.mark_as_garbage();
            }
        }
        else
        {
            items_.clear();
        }
    }


    event_processor_resource event_processors_container::insert(window_handle_t window, void* data, event_callback_t callback) noexcept
    {
        if ((event_processor_note::garbage_mark == window) || !callback)
        {
            return event_processor_resource::null;
        }

        const auto item_opt = items_.try_emplace_back
        (
            window,
            event_processor_resource::null,
            data,
            callback
        );

        if (!item_opt)
        {
            D_ASSERT(!"out of memory");
            return event_processor_resource::null;
        }

        const auto processor = new_event_processor_description();
        item_opt->processor = processor;
        return processor;

    }

    size_t event_processors_container::unlock_erase(window_handle_t window) noexcept
    {
        D_ASSERT(!lock_);

        auto last = items_.end();

        for (auto it = items_.begin(); it != last; )
        {
            if (it->window == window)
            {
                it->mark_as_garbage();
                std::swap(*it, *--last);
            }
            else
            {
                ++it;
            }
        }

        return items_.erase(last, items_.cend());
    }
}
