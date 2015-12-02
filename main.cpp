#include <iostream>
#include <string>
#include "ThreadPool.h"


DWORD WINAPI task1(LPVOID param)
{
	std::string info = *(std::string*)param;
	info += "This is task 1\n\n";
	std::cout << info << std::endl;
	return 0;
}

DWORD WINAPI task2(LPVOID param)
{
	std::string info = *(std::string*)param;
	info += "This is task 2\n\n";
	std::cout << info << std::endl;
	return 0;
}

DWORD WINAPI task3(LPVOID param)
{
	std::string info = *(std::string*)param;
	info += "This is task 3\n\n";
	std::cout << info << std::endl;
	return 0;
}

void main()
{
	ThreadPool tp(5, 2);

	tp.Enqueue(new Task(task1, NULL, 1, THREAD_PRIORITY_LOWEST));
	tp.Enqueue(new Task(task2, NULL, 2, THREAD_PRIORITY_LOWEST));
	tp.Enqueue(new Task(task3, NULL, 1, THREAD_PRIORITY_LOWEST));
	tp.Enqueue(new Task(task1, NULL, 3, THREAD_PRIORITY_NORMAL));
	tp.Enqueue(new Task(task2, NULL, 1, THREAD_PRIORITY_LOWEST));
	tp.Enqueue(new Task(task3, NULL, 3, THREAD_PRIORITY_NORMAL));
	tp.Enqueue(new Task(task1, NULL, 2, THREAD_PRIORITY_HIGHEST)); 
	tp.Enqueue(new Task(task2, NULL, 2, THREAD_PRIORITY_LOWEST));
	tp.Enqueue(new Task(task3, NULL, 3, THREAD_PRIORITY_HIGHEST));


	/*tp.Enqueue(new Task(task1, NULL, 1, THREAD_PRIORITY_NORMAL));
	tp.Enqueue(new Task(task2, NULL, 2, THREAD_PRIORITY_NORMAL));
	tp.Enqueue(new Task(task3, NULL, 1, THREAD_PRIORITY_NORMAL));
	tp.Enqueue(new Task(task1, NULL, 3, THREAD_PRIORITY_NORMAL));
	tp.Enqueue(new Task(task2, NULL, 1, THREAD_PRIORITY_NORMAL));
	tp.Enqueue(new Task(task3, NULL, 3, THREAD_PRIORITY_NORMAL));
	tp.Enqueue(new Task(task1, NULL, 2, THREAD_PRIORITY_NORMAL)); 
	tp.Enqueue(new Task(task2, NULL, 2, THREAD_PRIORITY_NORMAL));
	tp.Enqueue(new Task(task3, NULL, 3, THREAD_PRIORITY_NORMAL));*/

	Sleep(2000);
	std::cout << "Program has ended. Plese press any key to exit...\n" << std::endl;
	getchar();
}