#include "ConditionVariable.h"
#include "Mutex.h"
#include <Windows.h>
#include <assert.h>

struct _CV
{
	CONDITION_VARIABLE cond;
	CRITICAL_SECTION mutex;
};

ConditionVariable create_condition_variable()
{
	struct _CV * cv = (struct _CV *) malloc(sizeof(struct _CV));
	assert(cv);

	InitializeCriticalSection(&cv->mutex);
	InitializeConditionVariable(&cv->cond);

	return (ConditionVariable)cv;
}

void wait_on_condition_variable(ConditionVariable cv_, Mutex mutexToRelease)
{
	struct _CV * cv = (struct _CV *)cv_;

	EnterCriticalSection(&cv->mutex);
	if (mutexToRelease)
		release_mutex_lock(mutexToRelease);
	SleepConditionVariableCS(&cv->cond, &cv->mutex, INFINITE);
	LeaveCriticalSection(&cv->mutex);
}

void wait_on_condition_variable_2(ConditionVariable cv_, RWLock rwLockToRelease)
{
	struct _CV * cv = (struct _CV *)cv_;

	EnterCriticalSection(&cv->mutex);
	if (rwLockToRelease)
		release_rwlock(rwLockToRelease);
	SleepConditionVariableCS(&cv->cond, &cv->mutex, INFINITE);
	LeaveCriticalSection(&cv->mutex);
}

void notify_condition_variable(ConditionVariable cv_)
{
	struct _CV * cv = (struct _CV *)cv_;

	EnterCriticalSection(&cv->mutex);
	WakeConditionVariable(&cv->cond);
	LeaveCriticalSection(&cv->mutex);
}

void destroy_condition_variable(ConditionVariable cv)
{
	free((struct _CV *) cv);
}
