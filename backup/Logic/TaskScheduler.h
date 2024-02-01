#pragma once
#ifndef _THREADPOOL_
#define _THREADPOOL_
#include <mutex>
#include<thread>
#include <functional>
#include <future>
#include <utility>
#include <queue>
#include <unordered_set>
namespace MXRender
{
	template<typename T>
	class MessageQueue;

	class ThreadPool;

	//这个可以用原子变量的指针去做无锁化实现
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
				return T();
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

	struct TaskNode {
		int id; // 任务ID
		std::string name=""; // 任务名称
		bool is_executed; // 任务是否已执行
		std::function<void()> func; // 任务执行函数
		std::unordered_set<int> next_tasks; // 后继任务列表
		std::unordered_set<int> pre_tasks;//前置任务列表
	};	

	class TaskGraph
	{
	private:
		std::unordered_map<int, TaskNode> task_graph;
		int already_add_but_dontuse_node_num=0;
		std::unordered_map<std::string, int> name_convert_to_id;
	public:
		std::unordered_set<int> indegree_zero;
		std::unordered_set<int> outdegree_zero;
		std::queue<int> topology_diagram;
		std::queue<int> parallel_task_nums;
		template <typename F, typename... Args>
		std::function<void()> get_task(F&& f, Args &&...args)
		{
			std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

			auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

			std::function<void()> wrapper_func = [task_ptr]()
			{
				(*task_ptr)();
			};

			return wrapper_func;
		
		}

		template <typename C, typename F, typename... Args>
		std::function<void()> get_task(F C::* f, C* obj, Args&&... args)
		{
			std::function<decltype((obj->*f)(args...))()> func = std::bind((f), obj, std::forward<Args>(args)...);

			auto task_ptr = std::make_shared<std::packaged_task<decltype((obj->*f)(args...))()>>(func);

			std::function<void()> wrapper_func = [task_ptr]() {
				(*task_ptr)();
			};

			return wrapper_func;
		}

		bool add_task_node(int id, const std::string& name, const std::function<void()>& func, const std::unordered_set<int>& pre_tasks);
		/*bool add_task_node(int id, const std::string& name, const std::function<void()>& func, const std::unordered_set<std::string>& pre_tasks);*/
		void execute_task(int task_id);
		void compile();
		TaskGraph()=default;
		virtual ~TaskGraph() = default;
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
		MessageQueue<TaskGraph> task_graph_queue;
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
			std::function<void()> wrapper_func = [task_ptr]()
			{
				(*task_ptr)();
			};

			//将封装之后的函数压入队列
			message_queue.push(wrapper_func);
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
		auto submit_message(F C::* f, C* obj, Args&&... args) -> std::future<decltype((obj->*f)(args...))>
		{
			// 连接成员函数和参数定义，特殊函数类型，避免左右值错误
			std::function<decltype((obj->*f)(args...))()> func = std::bind((f), obj, std::forward<Args>(args)...);

			//将其封装到一个共享指针中以便能够复制构造
			auto task_ptr = std::make_shared<std::packaged_task<decltype((obj->*f)(args...))()>>(func);

			//使用lambda进行再次封装
			std::function<void()> wrapper_func = [task_ptr]() {
				(*task_ptr)();
			};

			//将封装之后的函数压入队列
			message_queue.push(wrapper_func);
			add_message();

			if (thread_list.size() < cpu_logicoperator_num && message_queue.size() > thread_list.size()) {
				thread_list.push_back(std::thread(ThreadWorker(this, thread_list.size())));
			}
			//唤醒一个线程
			condition.notify_one();

			return task_ptr->get_future();
		}

		std::future<void> excute_task_graph(TaskGraph* task_graph);
		void add_task_graph_to_todolist(const TaskGraph& task_graph);
		void excute_todolist_taskgraph();
	};

	struct TaskSystem
	{
		ThreadPool thread_pool;
		TaskGraph task_graph;
	};
}
#endif //_THREADPOOL_

