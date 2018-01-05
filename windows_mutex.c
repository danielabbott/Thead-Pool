#include "Mutex.h"
#include <stdlib.h>
#include <windows.h> 

Mutex create_mutex()
{
	CRITICAL_SECTION * criticalSection = malloc(sizeof(CRITICAL_SECTION ));
	InitializeCriticalSection(criticalSection);
	return (Mutex)criticalSection;
}

void aquire_mutex_lock(Mutex mutex)
{
	EnterCriticalSection((CRITICAL_SECTION  *)mutex);
}

void release_mutex_lock(Mutex mutex)
{
	LeaveCriticalSection((CRITICAL_SECTION  *)mutex);
}

int try_mutex_lock(Mutex mutex)
{
	return TryEnterCriticalSection((CRITICAL_SECTION  *)mutex);
}

void destroy_mutex(Mutex mutex)
{
	DeleteCriticalSection((CRITICAL_SECTION  *)mutex);
	free((CRITICAL_SECTION  *)mutex);
}
