#include "ConditionVariable.h"
#include "Mutex.h"
#include <cassert>
#include <condition_variable>
#include <mutex>

struct _CV
{
	std::condition_variable cond;
	std::mutex mutex;
};

ConditionVariable create_condition_variable()
{
	struct _CV * cv = new struct _CV();
	assert(cv);
	return (ConditionVariable)cv;
}

void wait_on_condition_variable(ConditionVariable cv_, Mutex mutexToRelease)
{
	struct _CV * cv = (struct _CV *)cv_;

	std::unique_lock<std::mutex> lk(cv->mutex);
	if(mutexToRelease)
		release_mutex_lock(mutexToRelease);
	cv->cond.wait(lk);
	lk.unlock();
}

void notify_condition_variable(ConditionVariable cv_)
{
	struct _CV * cv = (struct _CV *)cv_;

	cv->mutex.lock();
	cv->cond.notify_one();
	cv->mutex.unlock();
}

void destroy_condition_variable(ConditionVariable cv)
{
	free((struct _CV *) cv);
}
