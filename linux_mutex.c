#include "Mutex.h"
#include <pthread.h>
#include <stdlib.h>
#include <string.h>

pthread_mutex_t matrixTemplate = PTHREAD_MUTEX_INITIALIZER;

Mutex create_mutex()
{
	pthread_mutex_t * pmutex = malloc(sizeof(pthread_mutex_t));
	memcpy(pmutex, &matrixTemplate, sizeof(pthread_mutex_t));
	return (Mutex)pmutex;
}

void aquire_mutex_lock(Mutex mutex)
{
	pthread_mutex_lock((pthread_mutex_t *)mutex);
}

void release_mutex_lock(Mutex mutex)
{
	pthread_mutex_unlock((pthread_mutex_t *)mutex);
}

int try_mutex_lock(Mutex mutex)
{
	return pthread_mutex_trylock((pthread_mutex_t *)mutex);
}

void destroy_mutex(Mutex mutex)
{
	pthread_mutex_destroy((pthread_mutex_t *)mutex);
	free((pthread_mutex_t *)mutex);
}
