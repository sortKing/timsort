#pragma once

#include <mutex>

namespace my {

    template<class Ty, class Alloc = std::allocator<Ty>>
    class cuque {
    public:
        using value_type = Ty;
        using allocator_type = Alloc;
        using size_type = size_t;
        using difference_type = ptrdiff_t;
        using reference = value_type &;
        using const_reference = value_type const &;
        using pointer = value_type *;
        using const_pointer = value_type const *;

    private:
        using Alty_traits = std::allocator_traits<allocator_type>;
        allocator_type alloc;
        pointer first{nullptr};
        pointer last{nullptr};
        pointer _front{nullptr};
        pointer _back{nullptr};
        size_type len{0};
        std::mutex mtx;

    public:
        size_type capacity() {
            std::unique_lock<std::mutex> lock(mtx);
            return last - first;
        }

        size_type size() {
            std::unique_lock<std::mutex> lock(mtx);
            return len;
        }

        bool empty() {
            std::unique_lock<std::mutex> lock(mtx);
            return len == 0;
        }

        reference front() {
            std::unique_lock<std::mutex> lock(mtx);
            return *_front;
        }

        reference back() {
            std::unique_lock<std::mutex> lock(mtx);
            return *_back;
        }

    private:
        void _pop_front() {
            Alty_traits::destroy(alloc, _front);
            if (++_front == last) {
                _front = first;
            }
            --len;
        }

        void _pop_back() {
            Alty_traits::destroy(alloc, _back);
            if (_back-- == first) {
                _back = last - 1;
            }
            --len;
        }

    public:
        void pop_front() {
            std::unique_lock<std::mutex> lock(mtx);
            _pop_front();
        }

        void pop_back() {
            std::unique_lock<std::mutex> lock(mtx);
            _pop_back();
        }

        bool pull_front(reference ref) {
            std::unique_lock<std::mutex> lock(mtx);
            if (len == 0) {
                return false;
            }
            ref = std::move(*_front);
            _pop_front();
            return true;
        }

        bool pull_back(reference ref) {
            std::unique_lock<std::mutex> lock(mtx);
            if (len == 0) {
                return false;
            }
            ref = std::move(*_back);
            _pop_back();
            return true;
        }

    private:
        void _clear() {
            while (len != 0) {
                _pop_back();
            }
        }

    public:
        void clear() {
            std::unique_lock<std::mutex> lock(mtx);
            _clear();
        }

        void swap(cuque &other) {
            std::unique_lock<std::mutex> lk(other.mtx);
            std::unique_lock<std::mutex> lock(mtx);
            std::swap(first, other.first);
            std::swap(last, other.last);
            std::swap(_front, other._front);
            std::swap(_back, other._back);
            std::swap(len, other.len);
        }

        ~cuque() {
            std::unique_lock<std::mutex> lock(mtx);
            _clear();
            alloc.deallocate(first, last - first);
            first = nullptr;
            last = nullptr;
            _front = nullptr;
            _back = nullptr;
            len = 0;
        }

    private:
        reference _at(size_type n) {
            if (_front < last - n) {
                return _front[n];
            } else {
                return first[n - (last - _front)];
            }
        }

        void _reserve(size_type n) {
            if (last - first >= n) {
                return;
            }
            pointer ptr = alloc.allocate(n);
            for (size_type i = 0; i < len; ++i) {
                Alty_traits::construct(alloc, std::addressof(ptr[i]), std::move(_at(i)));
            }
            alloc.deallocate(first, last - first);
            first = ptr;
            last = first + n;
            _front = first;
            _back = first + len - 1;
        }

    public:
        void reserve(size_type n) {
            std::unique_lock<std::mutex> lock(mtx);
            _reserve(n);
        }

        size_type _align(size_type n) {
            size_type la = 0;
            do {
                n >>= 1;
                ++la;
            } while (n != 0);
            size_type pa = 2;
            do {
                pa <<= 1;
                --la;
            } while (la != 0);
            pa -= pa >> 2;
            // println(pa);
            return pa;
        }

        template<class... Args>
        reference emplace_front(Args &&... args) {
            std::unique_lock<std::mutex> lock(mtx);
            if (last - first == len) {
                _reserve(_align(len));
            }
            if (_front-- == first) {
                _front = last - 1;
            }
            Alty_traits::construct(alloc, _front, std::forward<Args>(args)...);
            ++len;
            return *_front;
        }

        template<class... Args>
        reference emplace_back(Args &&... args) {
            std::unique_lock<std::mutex> lock(mtx);
            if (last - first == len) {
                _reserve(_align(len));
            }
            if (++_back == last) {
                _back = first;
            }
            Alty_traits::construct(alloc, _back, std::forward<Args>(args)...);
            ++len;
            return *_back;
        }

        void push_front(const_reference ref) { emplace_front(ref); }

        void push_front(value_type &&ref) { emplace_front(std::move(ref)); }

        void push_back(const_reference ref) { emplace_back(ref); }

        void push_back(value_type &&ref) { emplace_back(std::move(ref)); }
    };

} // namespace my
