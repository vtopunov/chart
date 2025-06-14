#include <bluetooth/debug.h>

#include <span>


namespace bluetooth
{
    namespace debug
    {
        namespace
        {
#pragma pack(push, 1)
            struct hexbyte
            {
                char hi;
                char lo;
            };

            struct hexword
            {
                hexbyte hex;
                char delim;
            };
#pragma pack(pop)

            constexpr hexbyte make_hexbyte(uint8_t byte) noexcept
            {
                constexpr char hex[]
                {
                    '0', '1', '2', '3',
                    '4', '5', '6', '7',
                    '8', '9', 'a', 'b',
                    'c', 'd', 'e', 'f'
                };

                return { .hi{ hex[byte >> 4] }, .lo{ hex[byte & 0x0f] } };
            }

            constexpr hexword make_hexword(uint8_t byte, char delim) noexcept
            {
                return { .hex{ make_hexbyte(byte) }, .delim{ delim } };
            }

            std::string services(ULONG value) noexcept
            {
                using namespace std::string_view_literals;

                static constexpr std::string_view name_of_service[] =
                {
                    "Limited Discoverable Mode",
                    "LE audio",
                    "Reserved",
                    "Positioning",
                    "Networking",
                    "Rendering",
                    "Capturing",
                    "Object Transfer",
                    "Audio",
                    "Telephony",
                    "Information"
                };

                constexpr std::string_view delim{ ", " };

                std::string result{};
                for (const auto name : name_of_service)
                {
                    const bool bit_is_set = value & 1;

                    if (bit_is_set)
                    {
                        result += name;
                    }

                    if (!(value >>= 1))
                    {
                        break;
                    }

                    if (bit_is_set)
                    {
                        result += delim;
                    }
                }

                D_ASSERT(!value);

                return result;
            }
        }

        std::string mac_to_string(const BLUETOOTH_ADDRESS_STRUCT& address) noexcept
        {
            const auto& bytes = address.rgBytes;

            const hexword string[] =
            {
                make_hexword(bytes[5], ':'),
                make_hexword(bytes[4], ':'),
                make_hexword(bytes[3], ':'),
                make_hexword(bytes[2], ':'),
                make_hexword(bytes[1], ':'),
                make_hexword(bytes[0], '\0')
            };

            return { std::addressof(string->hex.hi), sizeof(string) - 1u };
        }

        std::string name_class_of_device(ULONG value) noexcept
        {
            static constexpr std::string_view uncategorized{ "Uncategorized" };

            static constexpr std::string_view computer_minor_device_classes[] =
            {
                uncategorized,
                "Desktop workstation",
                "Server-class computer",
                "Laptop",
                "Handheld PC/PDA(clamshell)",
                "Palm-size PC/PDA",
                "Wearable computer(watch size)",
                "Tablet"
            };

            static constexpr std::string_view  phone_minor_device_classes[] =
            {
                uncategorized,
                "Cellular",
                "Cordless",
                "Smartphone",
                "Wired modem or void gateway",
                "Common ISDN access",
            };

            static constexpr auto lan_network_minor_device_classes = [] (ULONG value) noexcept -> std::string
            {
                static constexpr std::string_view msg[] =
                {
                    "Fully available",
                    "1% to 17% utilized",
                    "17% to 33% utilized",
                    "33% to 50% utilized",
                    "50% to 67% utilized",
                    "67% to 83% utilized",
                    "83% to 99% utilized",
                    "No service available",
                };

                constexpr int mask{ 0x07 };
                static_assert((mask + 1u) == std::size(msg));

                return std::string{ msg[(value >> 3) & mask] };
            };

            static constexpr std::string_view audio_video_minor_device_classes[] =
            {
                uncategorized,
                "Headset",
                "Hands-free Device",
                "(Reserved)",
                "Microphone",
                "Loudspeaker",
                "Headphones",
                "Portable Audio",
                "Car Audio",
                "Set-top box",
                "HiFi Audio",
                "VCR",
                "Video Camera",
                "Camcorder",
                "Video Monitor",
                "Video Display and Loudspeaker",
                "Video Conferencing",
                "(Reserved)",
                "Gaming/Toy"
            };

            static constexpr auto peripheral_minor_device_classes = [] (ULONG value) noexcept -> std::string
            {
                static constexpr std::string_view upper_classes[] =
                {
                    uncategorized,
                    "Keyboard",
                    "Pointing Device",
                    "Keyboard/Pointing device"
                };

                static constexpr std::string_view lower_classes[] =
                {
                    uncategorized,
                    "Joystick",
                    "Gamepad",
                    "Remote Control",
                    "Sensing Device",
                    "Digitizer Tablet",
                    "Card Reader",
                    "Digital Pen",
                    "Handheld Scanner",
                    "Handheld Gestural Input Device"
                };

                constexpr int upper_mask{ 0x03 };
                constexpr int lower_mask{ 0x0f };
                static_assert((upper_mask + 1u) == std::size(upper_classes));

                const auto upper_index = (value >> 4) & upper_mask;
                const auto lower_index = value & lower_mask;

                std::string result{ upper_classes[upper_index] };
                result += '/';
                result += (lower_index < std::size(lower_classes)) ? lower_classes[lower_index] : uncategorized;
                return result;
            };

            static constexpr auto imaging_minor_device_classes = [] (ULONG value) noexcept -> std::string
            {
                static constexpr struct 
                {
                    int mask;
                    std::string_view name;
                } classes[] = 
                {
                    { 0b000100, "Display" },
                    { 0b001000, "Camera"  },
                    { 0b010000, "Scanner" },
                    { 0b100000, "Printer" }
                };

                std::string_view result{};
                for (const auto& device_class : classes) 
                {
                    if (value & device_class.mask) 
                    { 
                        result = device_class.name;
                        break;
                    }
                }

                return std::string{ result };
            };

            static constexpr std::string_view wearable_minor_device_classes[] =
            {
                "N/A",
                "Wristwatch",
                "Pager",
                "Jacket",
                "Helmet",
                "Glasses",
                "Pin"
            };

            static constexpr std::string_view toy_minor_device_classes[] =
            {
                "N/A",
                "Robot",
                "Hehicle",
                "Doll/Action Figure",
                "Controller",
                "Game",
            };

            static constexpr std::string_view health_minor_device_classes[] =
            {
                "Undefined",
                "Blood Pressure Monitor",
                "Thermometer",
                "Weighing Scale",
                "Glucose Meter",
                "Pulse Oximeter",
                "Heart/Pulse Rate Monitor",
                "Health Data Display",
                "Step Counter",
                "Body Composition Analyzer",
                "Peak Flow Monitor",
                "Medication Monitor",
                "Knee Prosthesis",
                "Ankle Prosthesis",
                "Generic Health Manager",
                "Personal Mobility Device",
            };

            struct device_class
            {
                using minor_string_factory_type = std::string(*) (ULONG) noexcept;

                std::string_view major{};
                std::span<const std::string_view> minor_array{};
                minor_string_factory_type minor_string_factory{ nullptr };
            };

            static constexpr device_class uncategorized_device{ uncategorized };

            static constexpr device_class major_device_classes[] =
            {
                {"Miscellaneous"},
                {"Computer", computer_minor_device_classes },
                {"Phone", phone_minor_device_classes },
                {"LAN/Network Access point", {}, lan_network_minor_device_classes },
                {"Audio/Video", audio_video_minor_device_classes },
                {"Peripheral", {}, peripheral_minor_device_classes },
                {"Imaging", {}, imaging_minor_device_classes },
                {"Wearable", wearable_minor_device_classes },
                {"Toy", toy_minor_device_classes },
                {"Health", health_minor_device_classes }
            };

            static constexpr std::string_view major_prefix{ ": " };

            const auto major = GET_COD_MAJOR(value);
            const auto minor = GET_COD_MINOR(value);
            const auto services_mask = GET_COD_SERVICE(value);

            const auto& major_device_class = (major < std::size(major_device_classes)) ? major_device_classes[major] : uncategorized_device;

            std::string result{ major_device_class.major };

            if (const auto minor_array_size = major_device_class.minor_array.size(); minor_array_size || minor)
            {
                const auto minor_device_class
                    = (minor < minor_array_size)
                    ? major_device_class.minor_array[minor]
                    : uncategorized;

                result += major_prefix;
                result += minor_device_class;
            }

            if (major_device_class.minor_string_factory)
            {
                const auto minor_device_class = major_device_class.minor_string_factory(minor);
                if (minor_device_class.size())
                {
                    result += major_prefix;
                    result += minor_device_class;
                }
            }

            if (const auto services_string = services(services_mask); services_string.size())
            {
                result += " (";
                result += services_string;
                result += ')';
            }

            return result;
        }
    }
}
