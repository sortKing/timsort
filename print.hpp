#pragma once

#include <iostream>
#include <mutex>

template<typename T, typename... Args>
void print(T &&first, Args &&...args) {
    static std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << first;
    if constexpr (sizeof...(args) != 0) {
        print(std::forward<Args>(args)...);
    }
}

template<typename... Args>
void println(Args &&...args) {
    static std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    if constexpr (sizeof...(args) != 0) {
        print(std::forward<Args>(args)...);
    }
    std::cout << std::endl;
}
