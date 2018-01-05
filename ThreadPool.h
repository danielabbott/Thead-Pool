#ifdef __cplusplus
#pragma once
#endif

#ifndef _JOB_H_
#define _JOB_H_

#include <stdint.h>
#include <string.h>

struct Device;
typedef struct Device Device;

Device * create_device(const char * deviceName);

// Call this first
void thread_pool_init(unsigned int cpuWorkerThreads_);

void start_worker_threads();

// Return 0 when completed. Return 1 if job is to be put off (avoid doing this, it 
	// will cause the thread to sleep for 250 milliseconds).
typedef int (*job_func) (size_t parameter);

void create_job(Device * device, job_func function, uintptr_t parameter);

// Job will be run when all other jobs have finished
// Could be used for swapping buffers and polling input after drawing each frame of a game
void set_final_job(Device * device, job_func function, uintptr_t parameter);

//void create_jobs(int n, job_func * functions, uintptr_t * paramters);

#endif
