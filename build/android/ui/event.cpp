#include <ui/event.h>

#include <android/input.h>

namespace ui
{
    static_assert(event::action_mask == AMOTION_EVENT_ACTION_MASK);
    static_assert(mouse_event::p_index_mask == AMOTION_EVENT_ACTION_POINTER_INDEX_MASK);
    static_assert(mouse_event::p_index_shift == AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
    static_assert(event_style::mouse_down == to_event_style(AMOTION_EVENT_ACTION_DOWN));
    static_assert(event_style::mouse_up == to_event_style(AMOTION_EVENT_ACTION_UP));
    static_assert(event_style::mouse_move == to_event_style(AMOTION_EVENT_ACTION_MOVE));


    mouse_event mouse_event::instance(const AInputEvent* input_e) noexcept
    {
        D_ASSERT(input_e);

        if (AINPUT_EVENT_TYPE_MOTION == AInputEvent_getType(input_e))
        {
            return mouse_event{ AMotionEvent_getAction(input_e), input_e };
        }

        return { -1, nullptr };
    }

    float mouse_event::x_by_index(size_t index) const noexcept
    {
        return AMotionEvent_getX(input_e_, index);
    }

    float mouse_event::y_by_index(size_t index) const noexcept
    {
        return AMotionEvent_getY(input_e_, index);
    }
    
    size_t mouse_event::number_of_positions() const
    {
        return AMotionEvent_getPointerCount(input_e_);
    }
}