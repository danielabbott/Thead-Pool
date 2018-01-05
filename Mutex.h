#ifdef __cplusplus
#pragma once
#endif

#ifndef _MUTEX_H_
#define _MUTEX_H_

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#else
#include <stdbool.h>
#endif

	typedef uintptr_t Mutex;

	Mutex create_mutex();
	void aquire_mutex_lock(Mutex);
	void release_mutex_lock(Mutex);

	/* Returns 1 if the mutex is currently locked, 0 if it was not and has now been locked */
	int try_mutex_lock(Mutex);

	/* Mutex lock must be released before mutex destruction */
	void destroy_mutex(Mutex);

#ifdef __cplusplus
}
#endif

#endif
