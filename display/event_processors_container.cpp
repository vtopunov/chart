#include "event_processors_container.h"

namespace display
{
    namespace event_processor
    {
        namespace
        {
            processor_resource new_event_processor_description() noexcept
            {
                static auto current = to_underlying(processor_resource{nullprocessor});
                return underlying_cast<processor_resource>(++current);
            }
        }

        processors_container& processors_container_global() noexcept
        {
            static processors_container map;
            return map;
        }

        bool processors_container::erase(processor_resource processor) noexcept
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

        size_t processors_container::erase(window_resource window) noexcept
        {
            size_t count = 0u;

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

        void processors_container::reset() noexcept
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


        processor_resource processors_container::insert(window_resource window, void* data, event_callback_t callback) noexcept
        {
            D_ASSERT(window != garbage_mark);
            D_ASSERT(callback);

            const auto processor = new_event_processor_description();

            const auto item_opt = items_.try_emplace_back
            (
                window,
                processor,
                data,
                callback
            );

            return (item_opt) ? processor : nullprocessor;
        }

        size_t processors_container::unlock_erase(window_resource window) noexcept
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
}
