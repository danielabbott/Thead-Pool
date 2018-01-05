#include "Mutex.h"
#include <mutex>

Mutex create_mutex()
{
	std::mutex * m = new std::mutex();
	return (uintptr_t) m;
}

void aquire_mutex_lock(Mutex mutex)
{
	((std::mutex *)mutex)->lock();
}

void release_mutex_lock(Mutex mutex)
{
	((std::mutex *)mutex)->unlock();
}

int try_mutex_lock(Mutex mutex)
{
	return ((std::mutex *)mutex)->try_lock() ? 1 : 0;
}
