#include "Debug.h"
#include "Exceptions.h"
#include "IObject.h"
#include "ThreadPool\IThreadPool.h"
#include <iostream>
#include <memory>
#include <string>

#define NR_OF_THREADS 2
#define INCREMENT_NR 100000

// create and implement Execute functions for IWorkItem overriter
class WorkItem : public ThreadPool::IThreadPool::IWorkItem {
public:
	WorkItem() {}
	void Execute() override { std::cout << "s "; Sleep(200); }
	~WorkItem() {}
};

int __cdecl main(int argc, char *argv[]) {
	_CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

	// validating arguments
	if (argc == 1 || argc > 5) {
		std::cout << "Invalid arguments\n";
		std::cout << "\t see -help\n";
		return 1;
	}

	// help
	if (argc == 2 && std::string(argv[1]) == "-help") {
		std::cout << "\t-help					possible arguments\n";
		std::cout << "\tthreadPool <nrThreads>			runs a threadPool class\n";
		return 0;
	}

	// thread pool
	if (argc == 3 && std::string(argv[1]) == "threadPool") {
		try { // verify values to be integers
			int nrThreads = stoi(std::string(argv[2])); nrThreads;
		} catch (...) {
			std::cout << "number of threads should be an integer" << std::endl;
			return 1;
		}

		// load ThreadPool lib
		HINSTANCE tlib = LoadLibrary("ThreadPool.dll");
		if (tlib == NULL) {
			throw LibException("Failed to load library.");
		}

		// load create and destroy functions
		p_CreateObject transport = (p_CreateObject)GetProcAddress(tlib, CREATE_OBJECT_FNAME);
		p_DestroyObject transportD = (p_DestroyObject)GetProcAddress(tlib, DESTROY_OBJECT_FNAME);
		if (!transport || !transportD) {
			throw LibException("Failed to load library functions.");
		}

		// create pool
		ThreadPool::IThreadPool* pool;
		if ((transport("ThreadPool", (void**)&pool) != 0)) {
			FreeLibrary(tlib);
			throw LibException("Failed to run library functions.");
		}

		// create work items
		ThreadPool::IThreadPool::IWorkItem* wi1 = new WorkItem();
		ThreadPool::IThreadPool::IWorkItem* wi2 = new WorkItem();
		ThreadPool::IThreadPool::IWorkItem* wi3 = new WorkItem();

		pool->init(stoi(std::string(argv[2])));

		HANDLE taskHandle1 = pool->add(wi1);
		HANDLE taskHandle2 = pool->add(wi2);
		HANDLE taskHandle3 = pool->add(wi3);
		
		// wait for events and close handles
		WaitForSingleObject(taskHandle1, INFINITE);
		CloseHandle(taskHandle1);
		WaitForSingleObject(taskHandle2, INFINITE);
		CloseHandle(taskHandle2);
		WaitForSingleObject(taskHandle3, INFINITE);
		CloseHandle(taskHandle3);

		pool->uninit();

		// unload thread pool
		if ((transportD("ThreadPool", (void*)pool) != 0)) {
			FreeLibrary(tlib);
			throw LibException("Failed to run library functions.");
		}

		// unload transport lib
		FreeLibrary(tlib);
		return 0;
	}

	std::cout << "Invalid arguments\n";
	std::cout << "\t see -help\n";
	return 1;
}
