#pragma once
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
		std::mutex mtx;
		std::condition_variable cnd;
		std::atomic_int signal { on };
		std::atomic_int status { null };
		my::cuque<std::function<void()>> task;

	public:
		template <class F, class... Args>
		std::future<my::invoke_result_t<F, Args...>> add(F&& fun, Args&&... args)
		{
			using type = std::packaged_task<my::invoke_result_t<F, Args...>()>;
			auto task_ptr = std::make_shared<type>(std::bind(std::forward<F>(fun), std::forward<Args>(args)...));
			task.emplace_back([=] { (*task_ptr)(); });
			cnd.notify_one();
			return task_ptr->get_future();
		}

	private:
		void work(thread_pool* host, std::shared_ptr<worker> _this);
		void free() {
			signal = off;
			cnd.notify_one();
		}

	public:
		worker()
		{
		}
		~worker()
		{
		}
	};

	class thread_pool {
		friend worker;
		std::mt19937 mt_rand { std::random_device {}() };
		std::mutex mtx;
		std::vector<std::shared_ptr<worker>> pool;
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
			std::uniform_int_distribution<> dis(0, (int)pool.size() - 1);
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
				}
				else {
					std::this_thread::yield();
				}
			}
		}
		explicit thread_pool(size_t n)
		{
			for (size_t i = 0; i < n; ++i) {
				auto ptr = std::make_shared<worker>();
				std::thread([=] {ptr->work(this, ptr); }).detach();
				pool.emplace_back(std::move(ptr));
			}
		}
		~thread_pool()
		{
			std::unique_lock<std::mutex> lock(mtx);
			for (auto& ptr : pool) {
				ptr->free();
			}
		}
	};

	void worker::work(thread_pool* host, std::shared_ptr<worker> _this)
	{
		status = on;
		while (on == signal) {
			std::unique_lock<std::mutex> lock(mtx);
			std::function<void()> fun;
			if (task.pull_front(fun)) {
				fun();
				host->count -= 1;
			}
			else {
				cnd.wait(lock);
			}
		}
		status = off;
		_this.reset();
	}

} // namespace my
