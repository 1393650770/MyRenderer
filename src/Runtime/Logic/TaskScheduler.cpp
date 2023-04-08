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

}