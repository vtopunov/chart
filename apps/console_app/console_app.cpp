#include <thread>

using namespace std::chrono_literals;

int main() noexcept
{
    const auto now = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(200s);
    std::this_thread::sleep_until(now + 250s);
    return 0;
}
