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
            event_handler_t callback;
            procedure_id_t id;
        };

        using map_procedures_t = small_flat_map<HWND, procedure_type, 4>;

        map_procedures_t& map_procedures() noexcept
        {
            static map_procedures_t map;
            return map;
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
            LRESULT result{ 0 };

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
        return DefWindowProcW(e.window_handle_, underlying_cast<UINT>(e.type_), e.word_parameter_, e.long_parameter_);
    }

    void event_dispatcher::close() noexcept
    {
        if (const auto id = std::exchange(procedure_id_, 0_z); id)
        {
            unregister_procedure(window_handle_, id);
        }
    }

    procedure_id_t generate_procedure_id() noexcept
    {
        static procedure_id_t id{ 0_z };
        return ++id;
    }

    event_dispatcher unsafe_register_event_handler(window_view window, procedure_id_t procedure_id, event_handler_t event_handler) noexcept
    {
        assert(window.handle_);
        assert(event_handler);
        assert(procedure_id);

        map_procedures().force_insert(window.handle_, procedure_type{ std::move(event_handler), procedure_id });

        return
        {
            window.handle_,
            procedure_id
        };
    }

    safe_event_dispatcher register_event_handler(window_view window, event_handler_t event_handler) noexcept
    {
        return unsafe_register_event_handler(window, generate_procedure_id(), std::move(event_handler));
    }

    void unregister_all_procedures(HWND window_handle) noexcept
    {
        map_procedures().erase(window_handle);
    }
}
