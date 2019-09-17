#include "event_handler.h"

#include <core/small_flat_map.h>
#include <core/underlying_cast.h>

#include <platform/windows/event.h>

namespace os_windows
{
    namespace
    {
        struct procedure_type
        {
            event_handler_type callback;
            procedure_id_t id;
        };

        using map_procedures_t = small_flat_map<HWND, procedure_type, 4>;

        map_procedures_t& map_procedures() noexcept
        {
            static map_procedures_t map;
            return map;
        }

        procedure_id_t generate_procedure_id() noexcept
        {
            static procedure_id_t id = 0;
            return id++;
        }

        procedure_id_t register_procedure(HWND window_handle, event_handler_type event_handler) noexcept
        {
            const auto procedure_id = generate_procedure_id();
            map_procedures().force_insert(window_handle, procedure_type{ std::move(event_handler), procedure_id });
            return procedure_id;
        }

        void unregister_procedure(HWND window_handle, procedure_id_t id) noexcept
        {
            auto& map = map_procedures();
            for (auto& item : map.items(window_handle))
            {
                if (item.value.id == id)
                {
                    map.erase(&item);
                    break;
                }
            }
        }
    }

    LRESULT CALLBACK window_procedure(HWND window_handle, UINT message, WPARAM word_parameter, LPARAM long_parameter) noexcept
    {
        const event e { window_handle, underlying_cast<event_type>(message), word_parameter, long_parameter };

        if (const auto procedures = std::as_const(map_procedures()).items(window_handle); !procedures.empty())
        {
            LRESULT result = 0;

            for (const auto& procedure : procedures)
            {
                if (const auto ret_code = procedure.value.callback(e))
                {
                    result = ret_code;
                    if (ret_code < 0)
                    {
                        break;
                    }
                }
            }

            return result;
        }

        return default_event_handler(e);
    }

    LRESULT default_event_handler(const event& e) noexcept
    {
        return DefWindowProcW(e.window_handle_, to_underlying(e.type_), e.word_parameter_, e.long_parameter_);
    }

    void event_dispatcher::close() noexcept
    {
        if (const auto id = release_procedure_id(); is_valid_procedure_id(id))
        {
            unregister_procedure(window_handle_, id);
        }
    }

    safe_event_dispatcher register_event_handler(window_view window, event_handler_type event_handler) noexcept
    {
        assert(window.handle_);
        assert(event_handler);

        return
        {
            event_dispatcher
            {
                window.handle_,
                register_procedure(window.handle_, std::move(event_handler))
            }
        };
    }

    void unregister_all_procedures(HWND window_handle) noexcept
    {
        map_procedures().erase(window_handle);
    }
}
