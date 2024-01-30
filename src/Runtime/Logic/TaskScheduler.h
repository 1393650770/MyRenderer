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

	//���������ԭ�ӱ�����ָ��ȥ��������ʵ��
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
		int id; // ����ID
		std::string name=""; // ��������
		bool is_executed; // �����Ƿ���ִ��
		std::function<void()> func; // ����ִ�к���
		std::unordered_set<int> next_tasks; // ��������б�
		std::unordered_set<int> pre_tasks;//ǰ�������б�
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
		//���õ��̹߳�����
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

		//ѭ�����������ж��ύ��������е��������������ж��̵߳�������ȫ�Ƿ����
		int message_num = 0;
		int cpu_logicoperator_num = 0;
		void add_message();

		//�������
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

		//��ʼ���̳߳�
		void init(int thread_num = 8, int cpu_logic_operator_num = 7);

		//�ȴ�����������ɣ����ر��̳߳�
		void close_theadpool();

		//�ȴ������������
		void join();

		unsigned int get_max_thread_num();

		unsigned int get_current_thread_id();

		//�ύ������Ϣ����
		template <typename F, typename... Args>
		auto submit_message(F&& f, Args &&...args)->std::future<decltype(f(args...))>
		{
			// ���Ӻ����Ͳ������壬���⺯�����ͣ���������ֵ����
			std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);

			//�����װ��һ������ָ�����Ա��ܹ����ƹ���
			auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);

			//ʹ��lambda�����ٴη�װ
			std::function<void()> wrapper_func = [task_ptr]()
			{
				(*task_ptr)();
			};

			//����װ֮��ĺ���ѹ�����
			message_queue.push(wrapper_func);
			add_message();

			if (thread_list.size() < cpu_logicoperator_num && message_queue.size() > thread_list.size())
			{
				thread_list.push_back(std::thread(ThreadWorker(this, thread_list.size())));
			}
			//����һ���߳�
			condition.notify_one();

			return task_ptr->get_future();
		};

		template <typename C, typename F, typename... Args>
		auto submit_message(F C::* f, C* obj, Args&&... args) -> std::future<decltype((obj->*f)(args...))>
		{
			// ���ӳ�Ա�����Ͳ������壬���⺯�����ͣ���������ֵ����
			std::function<decltype((obj->*f)(args...))()> func = std::bind((f), obj, std::forward<Args>(args)...);

			//�����װ��һ������ָ�����Ա��ܹ����ƹ���
			auto task_ptr = std::make_shared<std::packaged_task<decltype((obj->*f)(args...))()>>(func);

			//ʹ��lambda�����ٴη�װ
			std::function<void()> wrapper_func = [task_ptr]() {
				(*task_ptr)();
			};

			//����װ֮��ĺ���ѹ�����
			message_queue.push(wrapper_func);
			add_message();

			if (thread_list.size() < cpu_logicoperator_num && message_queue.size() > thread_list.size()) {
				thread_list.push_back(std::thread(ThreadWorker(this, thread_list.size())));
			}
			//����һ���߳�
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

