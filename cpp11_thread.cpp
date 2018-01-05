#include "Thread.h"
#include <thread>
#include <mutex>
#include <condition_variable>
#include "ConditionVariable.h"

extern "C" {

	struct _CV
	{
		std::condition_variable cond;
		std::mutex mutex;
	};

	struct _THREAD
	{
		std::thread thread;
		struct _CV cv;
	};

	Thread start_thread(void *(*startAddress) (uintptr_t), uintptr_t parameter)
	{
		struct _THREAD * thread = new struct _THREAD();
		
		thread->thread = std::thread(startAddress, (uintptr_t)parameter);
		return (Thread)thread;
	}

	void join_thread(Thread t)
	{
		((struct _THREAD *) t)->thread.join();
		delete (struct _THREAD *)t;
	}

	void thread_sleep(unsigned int milliseconds)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(milliseconds));
	}

}
