#pragma once
<<<<<<< HEAD
#include <functional>
#include <future>
#include <mutex>
#include <random>
#include <thread>
#include <vector>

#include "cuque.hpp"
#include "type_traits.hpp"

namespace my {

class thread_pool;

class worker {
    friend thread_pool;
    enum {
        null,
        on,
        off,
    };
    std::thread th;
    my::cuque<std::function<void()>> task;
    std::mutex mtx;
    std::condition_variable cnd;
    std::atomic_int signal { on };
    std::atomic_int status { null };
    thread_pool* host;

public:
    template <class F, class... Args>
    std::future<my::invoke_result_t<F, Args...>> add(F&& fun, Args&&... args)
    {
        using type = std::packaged_task<my::invoke_result_t<F, Args...>()>;
        auto task_ptr = std::make_shared<type>(std::bind(std::move(fun), std::forward<Args>(args)...));
        task.emplace_back([=] { (*task_ptr)(); });
        cnd.notify_one();
        return task_ptr->get_future();
    }

private:
    void work();

public:
    worker(thread_pool* ptr)
        : host(ptr)
        , th([&] { work(); })
    {
    }
    ~worker()
    {
        signal = off;
        while (status != off) {
            cnd.notify_one();
        }
        th.join();
    }
};

class thread_pool {
    friend worker;
    std::mt19937 mt_rand { std::random_device {}() };
    std::mutex mtx;
    std::vector<worker*> pool;
    std::atomic_size_t count { 0 };

public:
    size_t size()
    {
        std::unique_lock<std::mutex> lock(mtx);
        return pool.size();
    }

private:
    int get_rand()
    {
        std::unique_lock<std::mutex> lock(mtx);
        std::uniform_int_distribution<> dis(0, pool.size() - 1);
        return dis(mt_rand);
    }
    bool task_take(std::function<void()>& ref)
    {
        return pool[get_rand()]->task.pull_front(ref);
    }
    void scheduler()
    {
        std::function<void()> fun;
        if (task_take(fun)) {
            pool[get_rand()]->task.emplace_front(std::move(fun));
        }
    }

public:
    template <class F, class... Args>
    std::future<my::invoke_result_t<F, Args...>> add(F&& fun, Args&&... args)
    {
        count += 1;
        auto result = pool[get_rand()]->add(std::forward<F>(fun), std::forward<Args>(args)...);
        scheduler();
        return result;
    }
    void wait()
    {
        while (count != 0) {
            std::function<void()> fun;
            if (task_take(fun)) {
                fun();
                count -= 1;
            } else {
                std::this_thread::yield();
            }
        }
    }
    thread_pool(size_t n)
    {
        for (size_t i = 0; i < n; ++i) {
            pool.emplace_back(new worker(this));
        }
    }
    ~thread_pool()
    {
        std::unique_lock<std::mutex> lock(mtx);
        for (auto ptr : pool) {
            delete ptr;
        }
    }
};

void worker::work()
{
    status = on;
    do {
        std::function<void()> fun;
        {
            std::unique_lock<std::mutex> lock(mtx);
            while (on == signal && !task.pull_front(fun)) {
                cnd.wait(lock);
            }
        }
        if (on == signal) {
            fun();
            host->count -= 1;
        } else {
            break;
        }
    } while (true);
    status = off;
}
=======

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
>>>>>>> 6eba88305f037d296a18d55a228d6267110dec94

} // namespace my
