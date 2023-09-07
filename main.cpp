<<<<<<< HEAD
=======
#include "print.hpp"
#include "timsort.hpp"

>>>>>>> 6eba88305f037d296a18d55a228d6267110dec94
#include <algorithm>
#include <chrono>
#include <random>

<<<<<<< HEAD
#include "print.hpp"
#include "timsort.hpp"

template <class Iter,
    class Cmp = std::less<typename std::iterator_traits<Iter>::value_type>>
void check(Iter first, Iter last, Cmp cmp = {})
{
=======
template<class Iter, class Cmp = std::less<typename std::iterator_traits<Iter>::value_type>>
void check(Iter first, Iter last, Cmp cmp = {}) {
>>>>>>> 6eba88305f037d296a18d55a228d6267110dec94
    while (++first < last) {
        if (cmp(first[0], first[-1])) {
            throw std::runtime_error("Sort error");
        }
    }
}

<<<<<<< HEAD
template <class Fun>
void Time(Fun&& fun)
{
=======
template<class Fun>
void Time(Fun &&fun) {
>>>>>>> 6eba88305f037d296a18d55a228d6267110dec94
    clock_t begin_clock = clock();
    auto begin_tick = std::chrono::steady_clock::now();
    fun();
    auto end_tick = std::chrono::steady_clock::now();
    std::chrono::duration<double> diff = end_tick - begin_tick;
    long time_clock = (clock() - begin_clock) / (CLOCKS_PER_SEC / 1000);
    println("clock:\t", time_clock, "ms");
    println("steady:\t", diff.count(), "s");
}

<<<<<<< HEAD
std::mt19937 mt_rand { std::random_device {}() };

template <class Iter,
    class Cmp = std::less<typename std::iterator_traits<Iter>::value_type>>
void test(Iter first, Iter last, Cmp cmp = {})
{
=======
std::mt19937 mt_rand(std::random_device{}());

template<class Iter, class Cmp = std::less<typename std::iterator_traits<Iter>::value_type>>
void test(Iter first, Iter last, Cmp cmp = {}) {
>>>>>>> 6eba88305f037d296a18d55a228d6267110dec94
    println("\nRandom:");
    Time([&] {
        ptrdiff_t len = last - first;
        ptrdiff_t div = len / 3;
        for (ptrdiff_t n = 0; n < div; ++n) {
            first[n] = mt_rand();
        }
        Iter _last = first + div;
        std::copy(first, _last, _last);
        std::copy(first, _last, _last + div);
    });

    my::timsort<Iter, Cmp> tim;
    auto sort = [&] { Time([&] { tim.sort(first, last, cmp); }); };
    // auto sort = [&] { Time([&] { my::merge_sort(first, last, cmp); }); };

    println("\nSort random:");
    sort();
    check(first, last, cmp);

    println("\nSort order:");
    sort();
    check(first, last, cmp);

    println("\nReverse:");
    Time([&] { std::reverse(first, last); });

    println("\nSort reverse order:");
    sort();
    check(first, last, cmp);
}

<<<<<<< HEAD
int main()
{
    size_t len = 50000 * (size_t)9999;
    std::vector<int> arr;
    arr.resize(len);
    size_t num = 1;
    while (num--) {
        test(arr.begin(), arr.end(), std::greater<int> {});
=======
int main() {
    size_t len = 5 * (size_t)999;
    std::vector<int> arr;
    arr.resize(len);
    size_t num = 100;
    while (num--) {
        test(arr.begin(), arr.end(), std::greater<int>{});
>>>>>>> 6eba88305f037d296a18d55a228d6267110dec94
    }
    return 0;
}
