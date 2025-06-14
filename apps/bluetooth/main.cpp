#include <iostream_ex.h>

#include <bluetooth/bluetooth.h>

#pragma comment(lib, "Bthprops.lib")

namespace
{
    template<class T>
    constexpr const char* yes_to_string(const T& value) noexcept
    {
        return (value) ? "yes" : "no";
    };
}

int main() noexcept
{
    std::locale::global(std::locale(""));

    constexpr auto divstr = L"--------------------------------------------------------------------------------------------------------";

    std::wcout << L"Поиск Bluetooth-адаптера: last error = " << GetLastError() << std::endl;
    for (const auto radio : bluetooth::radio_search{})
    {
        {
            const bluetooth::radio_info radio_info{ radio };
            std::wcout << L"Найден Bluetooth-адаптер: " << radio_info.szName << std::endl;
        }

        std::wcout << L"Поиск устройств Bluetooth: last error = " << GetLastError() << std::endl;
        size_t n_device{ 0u };
        for (const auto& dev_info : bluetooth::device_search
            {
                .timeout_multiplier{ 3 },
                .radio{ radio }
            })
        {
            ++n_device;

            std::wcout
                << divstr
                << L"\nИмя устройства: " << dev_info.szName
                << L"\nТип устройства: " << bluetooth::debug::name_class_of_device(dev_info.ulClassofDevice)
                << L"\nАдрес: " << bluetooth::debug::mac_to_string(dev_info.Address)
                << L"\nПодключено: " << yes_to_string(dev_info.fConnected)
                << L"\nЗапомнено: " << yes_to_string(dev_info.fRemembered)
                << L"\nАвторизовано: " << yes_to_string(dev_info.fAuthenticated)
                << std::endl;
        }

        if (n_device)
        {
            std::wcout << divstr << std::endl;
            std::wcout << L"Найдено " << n_device << L" Bluetooth устройства\n" << std::endl;
        }
        else
        {
            std::wcout << L"Устройства Bluetooth не найдены: last error = " << GetLastError() << '\n' << std::endl;
        }
    }

    return 0;
}
