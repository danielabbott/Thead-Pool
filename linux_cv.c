#include "ConditionVariable.h"
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

struct _CV
{
	pthread_cond_t cond;
	pthread_mutex_t mutex;
};

extern pthread_mutex_t matrixTemplate;

ConditionVariable create_condition_variable()
{
	struct _CV * cv = (struct _CV *) malloc(sizeof(struct _CV));
	assert(cv);

	memcpy(&cv->mutex, &matrixTemplate, sizeof(pthread_mutex_t));
	assert(!pthread_cond_init(&cv->cond, NULL));

	return (ConditionVariable)cv;
}

void wait_on_condition_variable(ConditionVariable cv_, Mutex mutexToRelease)
{
	struct _CV * cv = (struct _CV *)cv_;

	pthread_mutex_lock(&cv->mutex);
	if (mutexToRelease)
		release_mutex_lock(mutexToRelease);
	pthread_cond_wait(&cv->cond, &cv->mutex);
   	pthread_mutex_unlock(&cv->mutex);
}

void notify_condition_variable(ConditionVariable cv_)
{
	struct _CV * cv = (struct _CV *)cv_;

	pthread_mutex_lock(&cv->mutex);
	pthread_cond_signal(&cv->cond);
   	pthread_mutex_unlock(&cv->mutex);
}

void destroy_condition_variable(ConditionVariable cv)
{
	free((struct _CV *) cv);
}
