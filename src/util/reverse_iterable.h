#pragma once

#include <iterator>

// Reversed iterable to for use in range-for loops.
template<typename T>
struct reverse_iterable {
    T& iterable;
};

template<typename T>
auto begin(reverse_iterable<T> w) {
    return std::rbegin(w.iterable);
}

template<typename T>
auto end(reverse_iterable<T> w) {
    return std::rend(w.iterable);
}

template<typename T>
reverse_iterable<T> make_reverse_iterable(T&& iterable) {
    return {iterable};
}
