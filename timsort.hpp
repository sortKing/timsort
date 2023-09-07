#pragma once

#include "print.hpp"
#include "thread_pool.hpp"
#include <algorithm>
#include <vector>

namespace my {

    template<class Iter, class Cmp = std::less<typename std::iterator_traits<Iter>::value_type>>
    void insert_sort(Iter first, Iter last, Cmp cmp = Cmp{}) {
        for (Iter for_x = first + 1; for_x < last; ++for_x) {
            Iter for_y = for_x;
            auto tmp = for_y[0];
            for (; first < for_y && cmp(tmp, for_y[-1]); --for_y) {
                for_y[0] = for_y[-1];
            }
            for_y[0] = tmp;
        }
    }

    template<class Iter, class Cmp>
    void merge_left(Iter first, Iter div, Iter last, Cmp cmp) {
        using value_type = typename std::iterator_traits<Iter>::value_type;
        using pointer = typename std::iterator_traits<Iter>::pointer;
        ptrdiff_t len = div - first;
        auto ptr = (pointer) malloc(sizeof(value_type) * len);
        pointer _first = ptr;
        pointer _last = _first + len;
        for (; len-- != 0;) {
            new(std::addressof(_first[len])) value_type(std::move(first[len]));
        }
        while (div < last && _first < _last) {
            if (cmp(div[0], _first[0])) {
                new(std::addressof(*first++)) value_type(std::move(*div++));
            } else {
                new(std::addressof(*first++)) value_type(std::move(*_first++));
            }
        }
        while (_first < _last) {
            new(std::addressof(*first++)) value_type(std::move(*_first++));
        }
        free(ptr);
    }

    template<class Iter, class Cmp>
    void merge_right(Iter first, Iter div, Iter last, Cmp cmp) {
        using value_type = typename std::iterator_traits<Iter>::value_type;
        using pointer = typename std::iterator_traits<Iter>::pointer;
        ptrdiff_t len = last - div;
        auto ptr = (pointer) malloc(sizeof(value_type) * len);
        pointer _first = ptr;
        pointer _last = _first + len;
        for (; len != 0; --len) {
            new(std::addressof(_last[-len])) value_type(std::move(last[-len]));
        }
        while (first < div && _first < _last) {
            if (cmp(_last[-1], div[-1])) {
                new(std::addressof(*--last)) value_type(std::move(*--div));
            } else {
                new(std::addressof(*--last)) value_type(std::move(*--_last));
            }
        }
        while (_first < _last) {
            new(std::addressof(*--last)) value_type(std::move(*--_last));
        }
        free(ptr);
    }

    template<class Iter, class Cmp>
    void merge(Iter first, Iter div, Iter last, Cmp cmp) {
        while (first < div && !cmp(div[0], first[0])) {
            ++first;
        }
        while (div < last && !cmp(last[-1], div[-1])) {
            --last;
        }
        if (cmp(last[-1], first[0])) {
            if (last - div < div - first) {
                for (ptrdiff_t i = 0; i < last - div; ++i) {
                    std::swap(first[0], div[i]);
                    first += 1;
                }
            } else {
                for (ptrdiff_t i = 1; i <= div - first; ++i) {
                    last -= 1;
                    std::swap(last[0], div[-i]);
                }
            }
        }
        if (last - div < div - first) {
            merge_right(first, div, last, cmp);
        } else {
            merge_left(first, div, last, cmp);
        }
    }

    ptrdiff_t const INSERT_THRESHOLDS = 64;

    template<class Iter, class Cmp = std::less<typename std::iterator_traits<Iter>::value_type>>
    void merge_sort(Iter first, Iter last, Cmp cmp = {}) {
        ptrdiff_t len = last - first;
        if (INSERT_THRESHOLDS < len) {
            len /= 3;
            Iter left = first + len;
            Iter right = last - len;

            merge_sort(first, left, cmp);
            merge_sort(left, right, cmp);
            merge_sort(right, last, cmp);

            if (!cmp(left[0], left[-1]) && !cmp(right[0], right[-1])) {
                // Orderly
            } else if (cmp(right[-1], first[0]) && cmp(last[-1], left[0])) {
                while (len-- != 0) {
                    std::swap(first[len], right[len]);
                }
            } else {
                merge(left, right, last, cmp);
                merge(first, left, last, cmp);
            }
        } else {
            insert_sort(first, last, cmp);
        }
    }

    template<class Iter, class Cmp = std::less<typename std::iterator_traits<Iter>::value_type>>
    class timsort {
        using reference = typename std::iterator_traits<Iter>::reference;
        struct Run {
            Iter first;
            Iter last;
        };
        using Container = std::vector<Run>;

    public:
        ~timsort() = default;

    private:
        void merge_sort(Iter first, Iter last, Cmp cmp = {}) {
            ptrdiff_t len = last - first;
            if (INSERT_THRESHOLDS < len) {
                Iter div = first + (len >> 1);
                merge_sort(first, div, cmp);
                merge_sort(div, last, cmp);
                merge(first, div, last, cmp);
            } else {
                insert_sort(first, last, cmp);
            }
        }

        bool is_not_equal(reference left, reference right, Cmp const cmp) {
            return cmp(left, right) || cmp(right, left);
        }

        bool is_equal(reference left, reference right, Cmp const cmp) {
            return !is_not_equal(left, right, cmp);
        }

        void reverse_stable(Iter const first, Iter const last, Cmp const cmp) {
            std::reverse(first, last);
            for (Iter it = first; it < last;) {
                while (it + 1 < last && is_not_equal(it[0], it[1], cmp)) {
                    ++it;
                }
                Iter tmp = it;
                if (it < last) {
                    do {
                        ++it;
                    } while (it < last && is_equal(it[-1], it[0], cmp));
                }
                std::reverse(tmp, it);
            }
        }

        void get_run(Container &left, Iter const first, Iter const last, Cmp const cmp) {
            my::thread_pool pool{3};
            ptrdiff_t minRun = (last - first + 8) / 9;
            for (Iter it = first; it < last;) {
                Iter tmp = it;
                while (it + 1 < last && is_equal(it[0], it[1], cmp)) {
                    it += 1;
                }
                if (it + 1 < last && cmp(it[1], it[0])) {
                    do {
                        ++it;
                    } while (it < last && !cmp(it[-1], it[0]));
                    reverse_stable(tmp, it, cmp);
                } else {
                    do {
                        ++it;
                    } while (it < last && !cmp(it[0], it[-1]));
                }
                if (it - tmp < minRun) {
                    it = std::min(tmp + minRun, last);
                    pool.add([=] { my::merge_sort(tmp, it, cmp); });
                }
                left.push_back(Run{tmp, it});
            }
            pool.wait();
        }

        void merge_run(Container &left, Container &right, Cmp const cmp) {
            ptrdiff_t len = left.back().last - left.front().first;
            ptrdiff_t mean = len / left.size();
            auto pull_back = [&](Run &ref) {
                ref = left.back();
                left.pop_back();
            };
            println(left.size());
            while (!left.empty()) {
                Run run_a, run_b, run_c;
                ptrdiff_t a, b, c;
                switch (left.size()) {
                    default:
                        pull_back(run_a);
                        pull_back(run_b);
                        run_c = left.front();

                        a = run_a.last - run_a.first;
                        b = run_b.last - run_b.first;
                        c = run_c.last - run_c.first;

                        if ((a + b) / 3 > mean || (a + b) / 3 > c) {
                            right.push_back(run_a);
                            left.push_back(run_b);
                        } else {
                            if (run_a.last == run_b.first) {
                                merge(run_a.first, run_a.last, run_b.last, cmp);
                                left.push_back(Run{run_a.first, run_b.last});
                            } else if (run_b.last == run_a.first) {
                                merge(run_b.first, run_b.last, run_a.last, cmp);
                                left.push_back(Run{run_b.first, run_a.last});
                            }
                        }
                        break;
                    case 2:
                        pull_back(run_a);
                        pull_back(run_b);

                        if (run_a.last == run_b.first) {
                            merge(run_a.first, run_a.last, run_b.last, cmp);
                            right.push_back(Run{run_a.first, run_b.last});
                        } else if (run_b.last == run_a.first) {
                            merge(run_b.first, run_b.last, run_a.last, cmp);
                            right.push_back(Run{run_b.first, run_a.last});
                        }
                        break;
                    case 1:
                        pull_back(run_a);

                        right.push_back(run_a);
                        break;
                    case 0:
                        break;
                }
            }
        }

    public:
        void sort(Iter const first, Iter const last, Cmp const cmp = {}) {
            Container left;
            Container right;
            get_run(left, first, last, cmp);
            while (true) {
                if (left.size() > 1) {
                    merge_run(left, right, cmp);
                } else {
                    break;
                }
                if (right.size() > 1) {
                    merge_run(right, left, cmp);
                } else {
                    break;
                }
            }
        }
    };

} // namespace my
