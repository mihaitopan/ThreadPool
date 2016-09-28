#include "ThreadPool.h"
#include <iostream>
#include <limits>


// --- --- --- Object --- --- ---
int CreateObject(const char* name, void** ptr) {
	if (std::string(name) == "ThreadPool") {
		*ptr = new ThreadPool::ThreadPool;
		return 0;
	}
	return 1;
}

int DestroyObject(const char* name, void* ptr) {
	if (std::string(name) == "ThreadPool") {
		ThreadPool::IThreadPool* myptr = (ThreadPool::IThreadPool*)ptr;
		delete myptr;
		return 0;
	}
	return 1;
}
// --- --- ---  --- --- ---  --- --- ---


// --- --- --- Thread pool --- --- ---
ThreadPool::ThreadPool::ThreadPool() {
	// create exit event, semaphore and initialize critical section
	this->taskRequestedEv[0] = CreateSemaphore(NULL, 0, LONG_MAX, NULL);
	this->taskRequestedEv[1] = CreateEvent(NULL, TRUE, FALSE, TEXT("Exit")); // manual reset
	InitializeCriticalSectionAndSpinCount(&this->crtSct_queue, 0x00000400);
}

DWORD __stdcall ThreadPool::ThreadPool::MyThreadFunction(LPVOID LpParam) {
	ThreadPool* that = (ThreadPool*)LpParam;
	IWorkItem* workItem = nullptr;
	HANDLE workEvent;

	DWORD dwWaitResult;
	while (true) {
		dwWaitResult = WaitForMultipleObjects(2, that->taskRequestedEv, FALSE, INFINITE);

		switch (dwWaitResult) {
		case WAIT_OBJECT_0 + 0:  // OperationRequestEvent

			// get and remove task
			EnterCriticalSection(&that->crtSct_queue);
			workItem = that->tasks.front().workItem;
			workEvent = that->tasks.front().taskCompletedEv;
			that->tasks.pop();
			LeaveCriticalSection(&that->crtSct_queue);

			workItem->Execute(); // execute task
			SetEvent(workEvent); // signal task is done
			delete workItem; // deallocate workItem
			break;

		case WAIT_OBJECT_0 + 1: // ExitEvent
			return 0;
		}
	}

	return 0;
}

int ThreadPool::ThreadPool::init(int num_threads) {
	// create all threads
	for (int i = 0; i < num_threads; i++) {
		this->threads.push_back(CreateThread(NULL, 0, &ThreadPool::MyThreadFunction, (LPVOID)this, 0, NULL));
	}

	return 0;
}

void ThreadPool::ThreadPool::uninit() {
	// set exit event
	SetEvent(this->taskRequestedEv[1]);
	
	// wait for each thread and close all  handles 
	for (std::vector<HANDLE>::iterator it = this->threads.begin(); it != this->threads.end(); ++it) {
		WaitForSingleObject(*it, INFINITE);
		CloseHandle(*it);
	}
}

HANDLE ThreadPool::ThreadPool::add(IWorkItem* workItem) {
	HANDLE dupWorkEvent;
	HANDLE workEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
	Package package = Package(workItem, workEvent);

	// push a task
	EnterCriticalSection(&this->crtSct_queue);
	this->tasks.push(package);
	LeaveCriticalSection(&this->crtSct_queue);

	// increase the semaphore count
	ReleaseSemaphore(this->taskRequestedEv[0], 1, NULL);

	// return a duplicated handle with restricted rights to the user
	DuplicateHandle(GetCurrentProcess(), workEvent, GetCurrentProcess(), &dupWorkEvent, SYNCHRONIZE, FALSE, 0);
	return dupWorkEvent;
}

ThreadPool::ThreadPool::~ThreadPool() {
	// close handles, uninitialize critical section
	DeleteCriticalSection(&this->crtSct_queue);
	CloseHandle(this->taskRequestedEv[0]);
	CloseHandle(this->taskRequestedEv[1]);
}
// --- --- ---  --- --- ---  --- --- ---