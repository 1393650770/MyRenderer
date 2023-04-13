#include "TaskScheduler.h"
#include<iostream>
#include <assert.h>

namespace MXRender
{
	//判断提交到任务队列的任务数，用于判断线程的任务完全是否完成
	static int message_num = 0;

	ThreadPool::ThreadWorker::ThreadWorker(ThreadPool* pool, int id) :owner_pool(pool), id(id)
	{
	}

	void ThreadPool::ThreadWorker::operator()()
	{
		std::function<void()> func;
		bool bis_deque = false;

		while (owner_pool->bis_running)
		{
			{
				std::unique_lock<std::mutex> lock(owner_pool->mutex);

				//任务队列为空，则阻塞当前线程
				if (owner_pool->message_queue.size() == 0)
				{
					owner_pool->mainthread_condition.notify_all();
					owner_pool->condition.wait(lock);
				}

				func = owner_pool->message_queue.pop(bis_deque);
			}
			if (bis_deque)
			{
				func();
				{
					std::unique_lock<std::mutex> lock(owner_pool->messagenum_mutex);
					if (owner_pool->message_num > 0)
					{
						owner_pool->message_num--;
						if (owner_pool->message_num == 0)
						{
							owner_pool->mainthread_condition.notify_all();
						}
					}
					else if(owner_pool->message_num == 0)
					{
						owner_pool->mainthread_condition.notify_all();
					}
				}
			}
			else
			{
				owner_pool->mainthread_condition.notify_all();
			}
		}
	}


	void ThreadPool::add_message()
	{
		{
			std::unique_lock<std::mutex> lock(messagenum_mutex);
			message_num++;
		}
	}

	ThreadPool::ThreadPool() :bis_running(true)
	{
		//Init(thread_num);
	}

	ThreadPool::~ThreadPool()
	{
		close_theadpool();
	}

	void ThreadPool::init(int thread_num, int cpu_logic_operator_num)
	{
		thread_list = std::vector<std::thread>(thread_num);
		for (size_t i = 0; i < thread_num; i++)
		{
			thread_list.at(i) = std::thread(ThreadWorker(this, i));
			thread_id_list.push_back(thread_list[i].get_id());
		}

		cpu_logicoperator_num = cpu_logic_operator_num;

	}

	void ThreadPool::close_theadpool()
	{
		bis_running = false;
		//唤醒所有线程，并等待他们执行完毕；
		condition.notify_all();
		for (size_t i = 0; i < thread_list.size(); i++)
		{
			if (thread_list.at(i).joinable())
			{
				thread_list.at(i).join();
			}
		}
	}

	void ThreadPool::join()
	{
		std::unique_lock<std::mutex> lock(messagenum_mutex);
		if (message_num > 0)
		{
			std::unique_lock<std::mutex> lock(mainthread_mutex);
			mainthread_condition.wait(lock);
		}
	}

	unsigned int ThreadPool::get_max_thread_num()
	{
		return thread_list.size();
	}

	unsigned int ThreadPool::get_current_thread_id()
	{
		for (unsigned int i=0;i< thread_id_list.size();i++)
		{
			if (thread_id_list[i]==std::this_thread::get_id())
			{
				return i;
			}
		}
		assert(false);
		return 0;
	}

	std::future<void> ThreadPool::excute_task_graph(TaskGraph* task_graph)
	{
		task_graph->compile();
		auto lambda = [&, this]()
		{
			std::queue<std::future<void>> que;
			int que_cout=task_graph->parallel_task_nums.front();
			task_graph->parallel_task_nums.pop();
			while (que_cout != 0)
			{
				int id=task_graph->topology_diagram.front();
				task_graph->topology_diagram.pop();
				que.emplace(std::move(submit_message(&TaskGraph::execute_task,task_graph, id)));
				que_cout--;
				if (que_cout==0)
				{
					if (task_graph->parallel_task_nums.empty()==false)
					{
						que_cout = task_graph->parallel_task_nums.front();
						task_graph->parallel_task_nums.pop();
						while (que.empty() == false)
						{
							que.front().wait();
							que.pop();
						}
					}
				}
			}
		};

		return submit_message(lambda);
		
	}

	void ThreadPool::add_task_graph_to_todolist(const TaskGraph& task_graph)
	{
		task_graph_queue.push(task_graph);
	}

	void ThreadPool::excute_todolist_taskgraph()
	{
		while (task_graph_queue.size() != 0)
		{
			bool pop_result=true;
			TaskGraph task_graph= task_graph_queue.pop(pop_result);
			if (pop_result)
			{
				excute_task_graph(&task_graph).wait();
			}
		}
	}

	bool TaskGraph::add_task_node(int id, const std::string& name, const std::function<void()>& func, const std::unordered_set<int>& next_tasks)
	{
		if (task_graph.find(id) != task_graph.end()) 
		{
			// 任务节点已存在，更新任务节点的状态和执行函数
			std::cout<<"任务节点已存在:id="<<id<<std::endl;
			TaskNode& node = task_graph[id];
			node.id = id;
			node.name = name;
			node.is_executed = false;
			node.func = func;
			node.next_tasks = next_tasks;
			for (int next_id : next_tasks) 
			{
				task_graph[next_id].pre_tasks.insert(id);
				auto it = indegree_zero.find(next_id);
				if (it != indegree_zero.end())
				{
					indegree_zero.erase(it);
				}
			}
			if (next_tasks.empty())
			{
				outdegree_zero.insert(id);
			}
			else
			{
				auto it = outdegree_zero.find(id);
				if (it !=outdegree_zero.end())
				{
					outdegree_zero.erase(it);
				}
			}
			return false;
		}
		else 
		{
			// 添加新的任务节点
			TaskNode node;
			node.id = id;
			node.name = name;
			node.is_executed = false;
			node.func = func;
			node.next_tasks = next_tasks;
			task_graph[id] = node;

			indegree_zero.insert(id);
			// 将当前任务的后继任务添加到前继任务列表中
			for (int next_id : next_tasks) 
			{
				task_graph[next_id].pre_tasks.insert(id);
				auto it = indegree_zero.find(next_id);
				if (it != indegree_zero.end())
				{
					indegree_zero.erase(it);
				}
			}
			if (next_tasks.empty())
			{
				outdegree_zero.insert(id);
			}
			return true;
		}
	}

	void TaskGraph::execute_task(int task_id)
	{
		auto iter = task_graph.find(task_id);
		if (iter != task_graph.end()&& iter->second.is_executed==false)
		{
			TaskNode& node = iter->second;
			node.func();
			node.is_executed=true;
		}
		else {
			std::cerr << "Task " << task_id << " does not exist" << std::endl;
		}
	}


	void TaskGraph::compile()
	{
		std::queue<int> que;
		for (auto& it : indegree_zero)
		{
			que.push(it);
		}

		int all_count = 0,n=task_graph.size(),que_cout=que.size();
		parallel_task_nums.push(que_cout);

		while (que_cout!=0)
		{
			int cur = que.front();
			que.pop();
			topology_diagram.push(cur);
			all_count++;
			que_cout--;
			TaskNode& node= task_graph[cur];

			if (auto it = outdegree_zero.find(cur) != outdegree_zero.end())
			{
				outdegree_zero.erase(it);
			}
			for (int next_id : node.next_tasks)
			{
				auto it = task_graph[next_id].pre_tasks.find(node.id);
				if (it != task_graph[next_id].pre_tasks.end())
				{
					task_graph[next_id].pre_tasks.erase(it);
					if (task_graph[next_id].pre_tasks.empty())
					{
						que.push(next_id);
					}
				}
			}
			if (que_cout==0)
			{
				que_cout=que.size();
				parallel_task_nums.push(que_cout);
			}
		}
		if (all_count != n) 
		{
			std::cout<<"Error:拓扑图有环"<<std::endl;
			std::abort();
		}

	}

}