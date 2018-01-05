#include "Thread.h"
#include "ConditionVariable.h"
#include <windows.h>  
#include <process.h>
#include <stdint.h>
#include <assert.h>

struct _CV
{
	CONDITION_VARIABLE cond;
	CRITICAL_SECTION mutex;
};

struct _THREAD
{
	HANDLE thread;
	struct _CV cv;
};

Thread start_thread(void *(*startAddress) (uintptr_t), uintptr_t parameter)
{
	struct _THREAD * thread = (struct _THREAD *) malloc(sizeof(struct _THREAD));
	assert(thread);

	InitializeCriticalSection(&thread->cv.mutex);
	InitializeConditionVariable(&thread->cv.cond);

	thread->thread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)startAddress, (void *)parameter, 0, NULL);

	return (Thread)thread;
}

void join_thread(Thread thread)
{
	WaitForSingleObject(((struct _THREAD *)thread)->thread, INFINITE);
	free((struct _THREAD *) thread);
}

void thread_sleep(unsigned int milliseconds)
{
	Sleep(milliseconds);
}
