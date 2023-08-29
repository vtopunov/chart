#include <ui/event.h>

#include <android/input.h>

namespace ui
{
    namespace
    {
        static_assert(event::action_mask == AMOTION_EVENT_ACTION_MASK);
        static_assert(event::p_index_mask == AMOTION_EVENT_ACTION_POINTER_INDEX_MASK);
        static_assert(event::p_index_shift == AMOTION_EVENT_ACTION_POINTER_INDEX_SHIFT);
        static_assert(event_style::mouse_down == to_event_style(AMOTION_EVENT_ACTION_DOWN));
        static_assert(event_style::mouse_up == to_event_style(AMOTION_EVENT_ACTION_UP));
        static_assert(event_style::mouse_move == to_event_style(AMOTION_EVENT_ACTION_MOVE));

    
        int32_t motion_action(const AInputEvent* input_e) noexcept
        {
            D_ASSERT(input_e);
            return (AINPUT_EVENT_TYPE_MOTION == AInputEvent_getType(input_e)) 
                ? AMotionEvent_getAction(input_e) 
                : event::invalid_action;
        }
    }

    event::event(const AInputEvent* input_e) noexcept
        : input_e_{ input_e }
        , action_{ motion_action(input_e) }
    {}

    event::_coordinate_value_type event::_x_coordinate(size_t index) const noexcept
    {
        D_ASSERT(index < _size());
        return numeric_cast<_coordinate_value_type>(AMotionEvent_getX(input_e_, index));
    }

    event::_coordinate_value_type event::_y_coordinate(size_t index) const noexcept
    {
        D_ASSERT(index < _size());
        return numeric_cast<_coordinate_value_type>(AMotionEvent_getY(input_e_, index));
    }
    
    size_t event::_size() const noexcept
    {
        return numeric_cast<size_t>(AMotionEvent_getPointerCount(input_e_));
    }
}