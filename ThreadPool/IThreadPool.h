#pragma once

#ifndef THREAD_POOL_H
#define THREAD_POOL_H
#include <windows.h>

extern "C" int __declspec(dllexport) CreateObject(const char* name, void** ptr);
extern "C" int __declspec(dllexport) DestroyObject(const char* name, void* ptr);

namespace ThreadPool {
	class IThreadPool {
	public:
		class IWorkItem {
		public:
			virtual void Execute() = 0;
			virtual ~IWorkItem() {}
		};

		virtual int init(int num_threads) = 0;
		virtual void uninit() = 0;

		// the workItem should be freed (by the thread pool) after the execution is completed  
		// the returned handle should be closed (by the user) if not needed
		virtual HANDLE add(IWorkItem* workItem) = 0;

		virtual ~IThreadPool() {}
	};
}

#endif