#include <atomic>
#include <iostream>
#include <new>

template<typename X, typename Y>
class Data {
public:
    Data<X, Y>(X x, Y y): x_(x), y_(y) {};

    [[nodiscard]] std::atomic<X>& x() { return x_; }
    [[nodiscard]] std::atomic<Y>& y() { return y_; }

private:
    alignas(std::hardware_destructive_interference_size) std::atomic<X> x_;
    alignas(std::hardware_destructive_interference_size) std::atomic<Y> y_;
};

int main() {
    Data data(0, 0);

    data.x().fetch_add(1);
    data.y().fetch_add(1);

    std::cout << data.x().load() << std::endl;
    std::cout << data.y().load() << std::endl;
}
