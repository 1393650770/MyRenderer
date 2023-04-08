#pragma once
#ifndef _THREADPOOL_
#define _THREADPOOL_
#include <mutex>
#include<thread>
#include <functional>
#include <future>
#include <utility>
#include <queue>
namespace MXRender
{
	template<typename T>
	class MessageQueue;

	template<typename T>
	class MessageQueue
	{
	private:
		std::queue <T> que;
		mutable std::mutex mut;
		std::condition_variable condit;

	public:
		MessageQueue& operator=(const MessageQueue&) = delete;
		MessageQueue(const MessageQueue& other) = delete;
		MessageQueue() :que(), mut(), condit() {};
		virtual ~MessageQueue() {};
		void push(T mes)
		{
			std::lock_guard<std::mutex> lock(mut);
			que.push(mes);
		};
		T pop(bool& result_bool)
		{
			if (que.empty())
			{
				result_bool = false;
				return nullptr;
			}

			std::unique_lock<std::mutex> lock(mut);

			T result;
			result = std::move(que.front());
			que.pop();
			result_bool = true;
			return std::move(result);
		}

		int32_t size()
		{
			std::lock_guard <std::mutex> lock(mut);
			return que.size();
		}

		bool empty()
		{
			std::lock_guard <std::mutex> lock(mut);
			return que.empty();
		}
	};


	class ThreadPool
	{
	private:
		//内置的线程工作类
		class ThreadWorker
		{
		private:
			int id;
			ThreadPool* owner_pool;
		public:
			ThreadWorker(ThreadPool* pool, int id);
			~ThreadWorker() = default;

			void operator()();


		};

		//循环计数器，判断提交到任务队列的任务数，用于判断线程的任务完全是否完成
		int message_num = 0;
		int cpu_logicoperator_num = 0;
		void add_message();

		//任务队列
		MessageQueue<std::function<void()>> message_queue;
		std::vector<std::thread> thread_list;
		std::vector<std::thread::id> thread_id_list;
		bool bis_running;
		std::mutex mutex;
		std::mutex messagenum_mutex;
		std::mutex mainthread_mutex;
		std::condition_variable mainthread_condition;
		std::condition_variable condition;
	public:

		ThreadPool();
		virtual ~ThreadPool();
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;
		ThreadPool& operator=(ThreadPool&&) = delete;

		//初始化线程池
		void init(int thread_num = 8, int cpu_logic_operator_num = 7);

		//等待所有任务完成，并关闭线程池
		void close_theadpool();

		//等待所有任务完成
		void join();

		unsigned int get_max_thread_num();

		unsigned int get_current_thread_id();

		//提交任务到消息队列
		template <typename F, typename... Args>
		auto submit_message(F&& f, Args &&...args)->std::future<decltype(f(args...))>
		{
			// 连接函数和参数定义，特殊函数类型，避免左右值错误
			std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

			//将其封装到一个共享指针中以便能够复制构造
			auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

			//使用lambda进行再次封装
			std::function<void()> warpper_func = [task_ptr]()
			{
				(*task_ptr)();
			};

			//将封装之后的函数压入队列
			message_queue.push(warpper_func);
			add_message();

			if (thread_list.size() < cpu_logicoperator_num && message_queue.size() > thread_list.size())
			{
				thread_list.push_back(std::thread(ThreadWorker(this, thread_list.size())));
			}
			//唤醒一个线程
			condition.notify_one();

			return task_ptr->get_future();
		};

		template <typename C, typename F, typename... Args>
		auto submit_message( F&& f, C* obj, Args&&... args) -> std::future<decltype((obj->*f)(args...))>
		{
			// 连接成员函数和参数定义，特殊函数类型，避免左右值错误
			std::function<decltype((obj->*f)(args...))()> func = std::bind(std::forward<F>(f), obj, std::forward<Args>(args)...);

			//将其封装到一个共享指针中以便能够复制构造
			auto task_ptr = std::make_shared<std::packaged_task<decltype((obj->*f)(args...))()>>(func);

			//使用lambda进行再次封装
			std::function<void()> warpper_func = [task_ptr]() {
				(*task_ptr)();
			};

			//将封装之后的函数压入队列
			message_queue.push(warpper_func);
			add_message();

			if (thread_list.size() < cpu_logicoperator_num && message_queue.size() > thread_list.size()) {
				thread_list.push_back(std::thread(ThreadWorker(this, thread_list.size())));
			}
			//唤醒一个线程
			condition.notify_one();

			return task_ptr->get_future();
		}
	};
}
#endif //_THREADPOOL_

