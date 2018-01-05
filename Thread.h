#ifdef __cplusplus
#pragma once
#endif

#ifndef _THREAD_H_
#define _THREAD_H_

#include <stdint.h>
#include "Mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef uintptr_t Thread;

	/* Starts a new thread which will run the function pointed to by startAddress */
	/* paramteter will be passed as the parameter to the startAddress fuction */
	Thread start_thread(void *(*startAddress) (uintptr_t), uintptr_t parameter);

	/* Waits for the given thread to finish executing then destroys the thread object */
	void join_thread(Thread);

	void thread_sleep(unsigned int milliseconds);

#ifdef __cplusplus
}
#endif

#endif
