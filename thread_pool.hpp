#pragma once

#include "cuque.hpp"
#include "type_traits.hpp"
#include <functional>
#include <future>
#include <mutex>
#include <list>
#include <thread>
#include <vector>

namespace my {

    class thread_pool {
        std::mutex mtx;
        std::condition_variable cnd;
        my::cuque<std::function<void()>> task;
        std::atomic_size_t count{0};
        std::atomic_int th_num{0};
        std::list < std::thread > pool;

    public:
        size_t size() {
            return th_num;
        }

    private:

        void work() {
            do {
                std::function<void()> fun;
                {
                    std::unique_lock<std::mutex> lock(mtx);
                    while (!task.pull_front(fun)) {
                        cnd.wait(lock);
                    }
                }
                if (fun) {
                    fun();
                    count -= 1;
                } else {
                    break;
                }
            } while (true);
        }

    public:
        template<class F, class... Args>
        std::future<my::invoke_result_t<F, Args...>> add(F &&fun, Args &&...args) {
            using type = std::packaged_task<my::invoke_result_t<F, Args...>()>;
            auto task_ptr = std::make_shared<type>(std::bind(std::forward<F>(fun), std::forward<Args>(args)...));
            count += 1;
            task.emplace_back([=] { (*task_ptr)(); });
            cnd.notify_one();
            return task_ptr->get_future();
        }

        void wait() {
            std::function<void()> fun;
            while (count != 0) {
                if (task.pull_front(fun)) {
                    fun();
                    count -= 1;
                } else {
                    std::this_thread::yield();
                }
            }
        }

        explicit thread_pool(size_t n) {
            for (size_t i = n; i != 0; --i) {
                pool.emplace_back([&] {
                    th_num += 1;
                    work();
                    th_num -= 1;
                });
            }
        }

        ~thread_pool() {
            for (int i = th_num; i != 0; --i) {
                task.emplace_back();
                cnd.notify_one();
            }
            println("µÈ´ý");
            for (auto & th : pool) {
                th.join();
            }
            println("Íê³É");
        }
    };

} // namespace my
