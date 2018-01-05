#include "Thread.h"
#ifndef __USE_MISC
#define __USE_MISC
#endif
#include <unistd.h>
#include <pthread.h>
#include <stdlib.h>
#include <assert.h>
#include <time.h>
#include <bits/time.h>
#include <stdbool.h>
#include <string.h>
#include "ConditionVariable.h"

struct _CV
{
	pthread_cond_t cond;
	pthread_mutex_t mutex;
};

struct _THREAD
{
	pthread_t pthread;
	struct _CV cv;
};

extern pthread_mutex_t matrixTemplate;

Thread start_thread(void *(*startAddress) (uintptr_t), uintptr_t parameter)
{
	struct _THREAD * thread = (struct _THREAD *) malloc(sizeof(struct _THREAD));
	assert(thread);

	memcpy(&thread->cv.mutex, &matrixTemplate, sizeof(pthread_mutex_t));
	assert(!pthread_cond_init(&thread->cv.cond, NULL));

	int ret = pthread_create (&thread->pthread, NULL, (void * (*)(void *))startAddress, (void *) parameter);
	assert(!ret);

	return (Thread)thread;
}

void join_thread(Thread thread)
{
	pthread_join(((struct _THREAD *)thread)->pthread, NULL);
	free((struct _THREAD *)thread);
}

void thread_sleep(unsigned int milliseconds)
{
	usleep(milliseconds*1000);
}
