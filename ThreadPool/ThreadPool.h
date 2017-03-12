#pragma once
#include "IThreadPool.h"
#include <vector>
#include <queue>

namespace ThreadPool {
	class ThreadPool : public IThreadPool {
	private:
		class Package {
		public:
			IWorkItem* workItem;
			HANDLE taskCompletedEv;
		public:
			Package(IWorkItem* WorkItem, HANDLE TaskCompletedEv) : workItem(WorkItem), taskCompletedEv(TaskCompletedEv) {}
			~Package() {}
		};

		std::vector<HANDLE> threads;
		std::queue<Package> tasks;
		HANDLE taskRequestedEv[2];
		CRITICAL_SECTION crtSct_queue; // critical section is preferred in this case to mutex because 
									   // it's whole purpose lies inside this process
						// depending on the number of threads the mutex can sometimes achieve a better performance
	public:
		ThreadPool();
		static DWORD __stdcall MyThreadFunction(LPVOID LpParam);
		int init(int num_threads) override;
		void uninit() override;
		HANDLE add(IWorkItem* workItem) override;
		~ThreadPool();
	};
}
