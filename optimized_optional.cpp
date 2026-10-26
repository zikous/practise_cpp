#include <cstddef>
#include <iostream>
#include <memory>
#include <stdexcept>
#include <string>

template <typename T> struct isSmall {
    static constexpr bool value =
        sizeof(T) <= sizeof(void *) && alignof(T) <= alignof(void *);
};

template <typename T, bool B = isSmall<T>::value> class MyOptional;

template <typename T> class MyOptional<T, true> {
public:
    MyOptional() = default;

    MyOptional(T value) : engaged_(true) {
        std::construct_at(ptr(), std::move(value));
    }

    MyOptional(const MyOptional &) = delete;
    MyOptional &operator=(const MyOptional &) = delete;

    ~MyOptional() {
        if (hasValue())
            std::destroy_at(ptr());
    }

    bool hasValue() const { return engaged_; }
    explicit operator bool() const { return hasValue(); }

    const T &operator*() const { return *ptr(); }

    const T &value() const {
        if (!hasValue())
            throw std::logic_error("empty optional");
        return *ptr();
    }

private:
    T *ptr() { return std::launder(reinterpret_cast<T *>(inner_)); }
    const T *ptr() const {
        return std::launder(reinterpret_cast<const T *>(inner_));
    }

    alignas(T) std::byte inner_[sizeof(T)];
    bool engaged_ = false;
};

template <typename T> class MyOptional<T, false> {
public:
    MyOptional() = default;

    MyOptional(T value) : inner_(std::make_unique<T>(std::move(value))) {}

    MyOptional(const MyOptional &) = delete;
    MyOptional &operator=(const MyOptional &) = delete;

    bool hasValue() const { return inner_ != nullptr; }
    explicit operator bool() const { return hasValue(); }

    const T &operator*() const { return *inner_; }

    const T &value() const {
        if (!hasValue())
            throw std::logic_error("empty optional");
        return *inner_;
    }

private:
    std::unique_ptr<T> inner_;
};

int main() {
    static_assert(isSmall<int>::value);
    static_assert(!isSmall<std::string>::value);

    MyOptional<int> small(42);
    MyOptional<std::string> large("hello");
    MyOptional<int> empty;

    std::cout << "small: " << *small << " (" << sizeof(small) << " bytes)\n";
    std::cout << "large: " << large.value() << " (" << sizeof(large)
              << " bytes)\n";
    std::cout << "empty: " << std::boolalpha << static_cast<bool>(empty)
              << '\n';
}
