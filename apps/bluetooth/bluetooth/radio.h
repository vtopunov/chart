#pragma once

#include <core/resource.h>

#include <bluetooth/fwd.h>


namespace bluetooth
{
    struct radio_resource : default_resource<HANDLE>
    {
        struct deleter_type
        {
            void operator () (radio_resource radio) const noexcept
            {
                if (radio)
                {
                    D_ASSERT_OR_UNUSED(CloseHandle(radio));
                }
            }
        };
    };

    using unique_radio = unique_resource<radio_resource>;

    struct radio_info : BLUETOOTH_RADIO_INFO
    {
        radio_info(radio_resource radio) noexcept
            : BLUETOOTH_RADIO_INFO{ .dwSize{ sizeof(BLUETOOTH_RADIO_INFO) } }
        {
            D_ASSERT_OR_UNUSED(ERROR_SUCCESS == BluetoothGetRadioInfo(radio, this));
        }
    };

    struct radio_search
    {
        struct iterator
        {
            struct container_resource : default_resource<HBLUETOOTH_RADIO_FIND>
            {
                struct deleter_type
                {
                    void operator () (container_resource radios) const noexcept
                    {
                        if (radios)
                        {
                            D_ASSERT_OR_UNUSED(BluetoothFindRadioClose(radios));
                        }
                    }
                };
            };

            unique_resource<container_resource> radios;
            unique_radio radio;

            iterator(const BLUETOOTH_FIND_RADIO_PARAMS& params) noexcept
            {
                as_mutable(radios.r().handle) = BluetoothFindFirstRadio
                (
                    std::addressof(params),
                    as_mutable_pointer(std::addressof(radio.r().handle))
                );
            }

            void next() noexcept
            {
                unique_radio next_radio{};

                if (!BluetoothFindNextRadio(radios, as_mutable_pointer(std::addressof(next_radio.r().handle))))
                {
                    next_radio.reset();
                }

                radio = std::move(next_radio);
            }

            constexpr radio_resource operator * () const noexcept
            {
                return radio;
            }

            constexpr explicit operator bool() const noexcept
            {
                return !!radio;
            }

            iterator& operator ++ () noexcept
            {
                next();
                return *this;
            }
        };

        iterator begin() const noexcept
        {
            return BLUETOOTH_FIND_RADIO_PARAMS{ .dwSize{ sizeof(BLUETOOTH_FIND_RADIO_PARAMS) } };
        }

        constexpr dummy_end<iterator> end() const noexcept
        {
            return {};
        }
    };
}