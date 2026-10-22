#include <algorithm>
#include <chrono>
#include <cstddef>
#include <future>
#include <iostream>
#include <iterator>
#include <numeric>
#include <stdexcept>
#include <vector>

static constexpr std::size_t CHUNK = 10'000'000;

template <std::random_access_iterator It, typename T = std::iter_value_t<It>>
T computeSum(It begin, It end, T initial = T{}) {
    auto n = end - begin;

    if (n < 0) {
        throw std::logic_error("invalid range");
    }

    if (n == 0) {
        return initial;
    }

    if (static_cast<std::size_t>(n) <= CHUNK) {
        return std::accumulate(begin, end, initial);
    }

    auto partialSum = [](It first, It last) -> T {
        T res = *first;

        for (++first; first != last; ++first) {
            res = res + *first;
        }

        return res;
    };

    std::vector<std::future<T>> futures;
    futures.reserve(n / CHUNK + (n % CHUNK != 0));

    for (std::size_t i = 0; i < static_cast<std::size_t>(n); i += CHUNK) {
        auto first = begin + i;
        auto last = begin + std::min(i + CHUNK, static_cast<std::size_t>(n));

        futures.push_back(
            std::async(std::launch::async, partialSum, first, last));
    }

    T sum = initial;

    for (auto &future : futures) {
        sum = sum + future.get();
    }

    return sum;
}

int main() {
    std::vector<int> values(30'000'000);
    std::iota(values.begin(), values.end(), 1);

    auto start = std::chrono::steady_clock::now();
    auto parallel = computeSum(values.begin(), values.end(), 0LL);
    auto end = std::chrono::steady_clock::now();

    auto parallelTime =
        std::chrono::duration<double, std::milli>(end - start).count();

    start = std::chrono::steady_clock::now();
    auto sequential = std::accumulate(values.begin(), values.end(), 0LL);
    end = std::chrono::steady_clock::now();

    auto sequentialTime =
        std::chrono::duration<double, std::milli>(end - start).count();

    std::cout << "Parallel:   " << parallel << " (" << parallelTime << " ms)\n";
    std::cout << "Sequential: " << sequential << " (" << sequentialTime
              << " ms)\n";
    std::cout << "Correct:    " << std::boolalpha << (parallel == sequential)
              << '\n';
    std::cout << "Speedup:    " << sequentialTime / parallelTime << "x\n";
}