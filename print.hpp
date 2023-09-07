#pragma once

#include <iostream>
#include <mutex>

<<<<<<< HEAD
template <typename T, typename... Args>
void print(T&& first, Args&&... args)
{
=======
template<typename T, typename... Args>
void print(T &&first, Args &&...args) {
>>>>>>> 6eba88305f037d296a18d55a228d6267110dec94
    static std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << first;
    if constexpr (sizeof...(args) != 0) {
        print(std::forward<Args>(args)...);
    }
}

<<<<<<< HEAD
template <typename... Args>
void println(Args&&... args)
{
=======
template<typename... Args>
void println(Args &&...args) {
>>>>>>> 6eba88305f037d296a18d55a228d6267110dec94
    static std::mutex mtx;
    std::unique_lock<std::mutex> lock(mtx);
    if constexpr (sizeof...(args) != 0) {
        print(std::forward<Args>(args)...);
    }
    std::cout << std::endl;
}
