#ifndef LAB2_THREADPOOL
#define LAB2_THREADPOOL

#include <Windows.h>
#include <vector>
#include <string>

#include "easylogging++.h"

INITIALIZE_EASYLOGGINGPP

#define DEFAULT_THREAD_COUNT 3
typedef DWORD (WINAPI *FUNC)(LPVOID lpParam);

struct TaskParam
{
	LPVOID *lParam;
};

class Task
{
public:
	Task(LPTHREAD_START_ROUTINE lpStartAddress, LPVOID lpParameter, int task_priority, int thread_priority)
	{
		this->lpStartAddress = lpStartAddress;
		this->lpParameter = lpParameter;
		this->task_priority = task_priority;
		this->thread_priority = thread_priority;
	}

	LPTHREAD_START_ROUTINE GetStartAddress()
	{
		return lpStartAddress;
	}

	LPVOID GetlpParameter()
	{
		return lpParameter;
	}

	int GetTaskPriority()
	{
		return task_priority;
	}

	int GetThreadPriority()
	{
		return thread_priority;
	}

private:
	LPTHREAD_START_ROUTINE lpStartAddress;
	LPVOID lpParameter;
	int task_priority;
	int thread_priority;
};

class ThreadPool
{
private:

	class Worker
	{
	private:
		bool enabled;
		HANDLE hThread;
	public:
		Worker()
		{
			enabled = true;
		}

		bool Enabled()
		{
			return enabled;
		}

		void SetEnabled(bool b)
		{
			enabled = b;
		}

		HANDLE GetThreadHandle()
		{
			return hThread;
		}

		void SetThreadHandle(HANDLE h)
		{
			hThread = h;
		}
	};

	int max_thread_count;
	int current_thread_count;
	std::vector<Worker> workers;
	std::vector<Task *> tasks;
	HANDLE hMutex;
	HANDLE hDispatcherThread;

	void CreateWorkers()
	{
		std::string info = "";
		info = std::to_string(current_thread_count);
		info += " workers have been successfully created";

		for(int i=0; i<current_thread_count; i++)
		{
			workers.push_back(Worker());
		}
		LOG(INFO) << info;
	}

	int GetFreeWorkerIndex()
	{
		for(int i = 0; i < workers.size(); i++)
			if(workers[i].Enabled())
				return i;
		return -1;
	}

	void SortByTaskPriority()
	{
		for(int i = 0; i < tasks.size() - 1; i++)
			for(int j = i + 1; j < tasks.size(); j++)
				if(tasks[j]->GetTaskPriority() < tasks[i]->GetTaskPriority())
				{
					Task *temp = tasks[i];
					tasks[i] = tasks[j];
					tasks[j] = temp;
				}
	}

	static DWORD WINAPI dispatcher_proc(LPVOID param)
	{
		while(true)
		{
			Task *t;
			ThreadPool *tp = (ThreadPool*)param;
			int task_count;

			do
			{
				Sleep(1000);
				WaitForSingleObject(tp->hMutex, INFINITE);
				task_count = tp->tasks.size();
				ReleaseMutex(tp->hMutex);
			}
			while(task_count == 0);


			WaitForSingleObject(tp->hMutex, INFINITE);
			int free_worker_index;
			tp->SortByTaskPriority();
			while(tp->tasks.size() != 0)
			{
				t = tp->tasks[tp->tasks.size() -1]; 
				tp->tasks.pop_back();
				free_worker_index = tp->GetFreeWorkerIndex();
				if(free_worker_index != -1)
				{
					tp->workers[free_worker_index].SetEnabled(false);
					tp->workers[free_worker_index].SetThreadHandle(CreateThread(NULL, 0, task_proc, t, CREATE_SUSPENDED, NULL));
					SetThreadPriority(tp->workers[free_worker_index].GetThreadHandle(), t->GetThreadPriority());
					ResumeThread(tp->workers[free_worker_index].GetThreadHandle());
				}
				else
				{
					if(tp->current_thread_count < tp->max_thread_count)
					{
						tp->workers.push_back(Worker());
						tp->workers[tp->workers.size() - 1].SetEnabled(false);
						tp->workers[tp->workers.size() - 1].SetThreadHandle(CreateThread(NULL, 0, task_proc, t, CREATE_SUSPENDED, NULL));
						SetThreadPriority(tp->workers[tp->workers.size() - 1].GetThreadHandle(), t->GetThreadPriority());
						ResumeThread(tp->workers[tp->workers.size() - 1].GetThreadHandle());
						tp->current_thread_count = tp->workers.size();
					}
					else
					{
						LOG(ERROR) << "Unable to perform a task. Max thread limit is achieved.\n";
						delete t;
					}
				}
				for(int i = 0; i < tp->workers.size(); i++)
				{
					if(!tp->workers[i].Enabled())
					{
						int code = WaitForSingleObject(tp->workers[i].GetThreadHandle(), 0);
						if(code == WAIT_OBJECT_0)
						{
							CloseHandle(tp->workers[i].GetThreadHandle());
							tp->workers[i] = Worker();
						}
					}
				}


			}
			ReleaseMutex(tp->hMutex);
		}
	}

	static DWORD WINAPI task_proc(LPVOID lParam)
	{
		Task *t = (Task*)lParam;
		std::string priority;
		std::string info;
		FUNC func = t->GetStartAddress();
		
		switch (GetThreadPriority(GetCurrentThread()))
		{
		case THREAD_PRIORITY_NORMAL:
			priority = "Normal";
			break;
		case THREAD_PRIORITY_LOWEST:
			priority = "Lowest";
			break;
		case THREAD_PRIORITY_HIGHEST:
			priority = "Highest";
			break;
		default:
			priority = "Other";
			break;
		}
		info = "";
		info += "\nThread ID: ";
		info += std::to_string(GetCurrentThreadId());
		info += "\n";
		info += "Thread priority: ";
		info += priority;
		info += "\n";
		info += "Task priority: ";
		info += std::to_string(t->GetTaskPriority());
		info += "\n";

		try
		{
			func(&info);
		}
		catch(std::exception &e)
		{
			LOG(ERROR) << e.what();
		}

		return 0;
	}

public:
	ThreadPool(int max_thread_count, int current_thread_count = DEFAULT_THREAD_COUNT)
	{
		this->max_thread_count = max_thread_count;
		this->current_thread_count = current_thread_count;
		hMutex = CreateMutex(NULL, false, NULL);
		hDispatcherThread = CreateThread(NULL, 0, dispatcher_proc, this, NULL, NULL);
		CreateWorkers();
	}

	~ThreadPool()
	{
		for(int i = 0; i < workers.size(); i++)
		{
			TerminateThread(workers[i].GetThreadHandle(), 0);
		}
		workers.clear();
		tasks.clear();
		TerminateThread(hDispatcherThread, 0);
		CloseHandle(hMutex);
	}

	void Enqueue(Task *task)
	{	
		WaitForSingleObject(hMutex, INFINITE);
		tasks.push_back(task);
		LOG(INFO) << "New task was added.";
		ReleaseMutex(hMutex);
	}

};

#endif