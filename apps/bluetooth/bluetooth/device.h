#pragma once

#include <bluetooth/radio.h>


namespace bluetooth
{
    struct device_search
    {
        static constexpr BLUETOOTH_DEVICE_SEARCH_PARAMS* _prop_data(void* self) noexcept
        {
            return static_cast<BLUETOOTH_DEVICE_SEARCH_PARAMS*>(self);
        }

        struct radio_property
        {
            constexpr radio_property() noexcept = default;

            constexpr radio_property(radio_resource radio) noexcept
            {
                _prop_data(this)->hRadio = radio;
            }
        };

        struct timeout_multiplier_property
        {
            constexpr timeout_multiplier_property() noexcept = default;

            constexpr timeout_multiplier_property(UCHAR timeout_multiplier) noexcept
            {
                _prop_data(this)->cTimeoutMultiplier = timeout_multiplier;
            }
        };

        BLUETOOTH_DEVICE_SEARCH_PARAMS params
        {
            .dwSize{ sizeof(BLUETOOTH_DEVICE_SEARCH_PARAMS) },
            .fReturnAuthenticated{ TRUE },
            .fReturnRemembered{ TRUE },
            .fReturnUnknown{ TRUE },
            .fReturnConnected{ TRUE },
            .fIssueInquiry{ TRUE }
        };

        D_NO_UNIQUE_ADDRESS timeout_multiplier_property timeout_multiplier{};
        D_NO_UNIQUE_ADDRESS radio_property radio{};

        struct iterator
        {
            struct deleter
            {
                void operator () (HBLUETOOTH_DEVICE_FIND hFindDevice) const noexcept
                {
                    if (hFindDevice)
                    {
                        D_CHECK(BluetoothFindDeviceClose(hFindDevice));
                    }
                }
            };

            unique_resource<HBLUETOOTH_DEVICE_FIND, deleter> hFindDevice{};
            BLUETOOTH_DEVICE_INFO_STRUCT info{ .dwSize{ sizeof(BLUETOOTH_DEVICE_INFO_STRUCT) } };

            iterator(const BLUETOOTH_DEVICE_SEARCH_PARAMS& params) noexcept
            {
                as_mutable(hFindDevice.r()) = BluetoothFindFirstDevice(&params, &info);
            }

            void next() noexcept
            {
                if (!BluetoothFindNextDevice(hFindDevice, std::addressof(info)))
                {
                    hFindDevice.reset();
                }
            }

            constexpr const BLUETOOTH_DEVICE_INFO_STRUCT& operator * () const noexcept
            {
                return info;
            }

            constexpr explicit operator bool() const noexcept
            {
                return !!hFindDevice;
            }

            iterator& operator ++ () noexcept
            {
                next();
                return *this;
            }
        };

        iterator begin() const noexcept
        {
            return params;
        }

        constexpr dummy_end<iterator> end() const noexcept
        {
            return {};
        }
    };

    static_assert(test_no_unique_address(&device_search::timeout_multiplier));
    static_assert(test_no_unique_address(&device_search::radio));
}
