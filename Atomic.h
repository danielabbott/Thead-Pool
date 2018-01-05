#ifdef __cplusplus
#pragma once
#endif

#ifndef _ATOMIC_H_
#define _ATOMIC_H_

#if defined(__cplusplus)

#include <atomic>

typedef std::atomic<int> AtomicInt;

static inline int atomic_increment(AtomicInt * atomic)
{
	return ++(*atomic);
}

static inline int atomic_decrement(AtomicInt * atomic)
{
	return --(*atomic);
}

static inline void atomic_set(AtomicInt * atomic, int value)
{
	*atomic = value;
}

static inline int atomic_get(AtomicInt * atomic)
{
	return *atomic;
}

#elif defined(_WIN32)

#include <Windows.h>

// Must be aligned to 4-byte boundary (shouldn't be a problem)
typedef volatile long AtomicInt;

static inline int atomic_increment(AtomicInt * atomic)
{
	// InterlockedIncrement returns the incremented value
	return InterlockedIncrement(atomic);
}

static inline int atomic_decrement(AtomicInt * atomic)
{
	return InterlockedDecrement(atomic);
}

static inline void atomic_set(AtomicInt * atomic, long value)
{
	*atomic = value;
}

static inline int atomic_get(AtomicInt * atomic)
{
	return *atomic;
}

#elif __STDC_VERSION__>=201112L
// C11

#include <stdatomic.h>

typedef atomic_int AtomicInt;

static inline int atomic_increment(volatile AtomicInt * atomic)
{
	// atomic_fetch_add returns the value before addition
	return atomic_fetch_add(atomic, 1) + 1;
}

static inline int atomic_decrement(volatile AtomicInt * atomic)
{
	return atomic_fetch_sub(atomic, 1) - 1;
}

static inline void atomic_set(AtomicInt * atomic, int value)
{
	atomic_store(atomic, value);
}

static inline int atomic_get(AtomicInt * atomic)
{
	return atomic_load(atomic);
}

#else
#error "No supported atomic type implementations found"
#endif


#endif
