#pragma once

#include <string>

#include <bluetooth/fwd.h>


namespace bluetooth
{
    namespace debug
    {
        std::string mac_to_string(const BLUETOOTH_ADDRESS_STRUCT& address) noexcept;

        std::string name_class_of_device(ULONG value) noexcept;
    }
}
