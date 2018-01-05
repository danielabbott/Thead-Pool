#include "ThreadPool.h"
#include "Thread.h"
#include "Mutex.h"
#include "Atomic.h"
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <stdio.h>
#include "ConditionVariable.h"

struct Job;

typedef struct Job {
	struct Job * next; // points towards end/back of queue
	struct Job * prev; // points towards start/front of queue
	job_func     function;
	uintptr_t    parameter;
} Job;

typedef struct JobQueue {
	Mutex mutex;
	// size can be read without locking the mutex but use that value 
	// only as an estimate - it could change as soon as it is read
	int size;
	Job * front;
	Job * back;
} JobQueue;

struct Device
{
	const char * name;
	Thread thread;
	ConditionVariable conditionVariable;
	JobQueue jobQueue;
};

/* Prevents data races when creating the threads */
Mutex globalDeviceThreadsLock;

Device devices[64];

unsigned int deviceCount;
AtomicInt runningThreads;
unsigned int cpuWorkerThreads;

/* Data for 'final job' - job to run after all other jobs */
Device * finalJobDevice;
job_func finalJobFunction;
uintptr_t finalJobParameter;

/* Jobs in the queues + jos executing */
AtomicInt totalJobs;

AtomicInt killAllThreads = false;
ConditionVariable mainThreadCV;

void * thread_function(uintptr_t);

void thread_pool_init(unsigned int cpuWorkerThreads_)
{
	assert(cpuWorkerThreads_ <= 64);

	mainThreadCV = create_condition_variable();
	globalDeviceThreadsLock = create_mutex();
	runningThreads = 0;
	deviceCount = cpuWorkerThreads_;
	cpuWorkerThreads = cpuWorkerThreads_;
	totalJobs = 0;
	finalJobFunction = NULL;

	for(unsigned int i = 0; i < cpuWorkerThreads_; i++) {
		char * name = malloc(8);
		snprintf(name, 8, "CPU%u", i);
		devices[i].name = name;
		devices[i].jobQueue.mutex = create_mutex();
		devices[i].jobQueue.size = 0;
		devices[i].jobQueue.front = NULL;
		devices[i].jobQueue.back = NULL;
	}

}

Device * create_device(const char * deviceName)
{
	// Devices must be created before starting the threads
	assert(!atomic_get(&runningThreads));
	assert(deviceCount < 64);

	Device * device = &devices[deviceCount++];

	device->name = deviceName;
	device->jobQueue.mutex = create_mutex();
	device->jobQueue.size = 0;
	device->jobQueue.front = NULL;
	device->jobQueue.back = NULL;

	return device;
}

// Waits for all jobs to complete and then kills every thread
void wait_for_all_jobs_to_complete_and_kill();

void start_worker_threads()
{
	assert(deviceCount);

	bool noJobs = totalJobs == 0;

	aquire_mutex_lock(globalDeviceThreadsLock);
	for (unsigned int i = 0; i < deviceCount; i++) {
		devices[i].conditionVariable = create_condition_variable();
		devices[i].thread = start_thread(thread_function, i);
	}

	if(noJobs && finalJobFunction) {
		/* No jobs were specified, just a 'final job' */

		job_func func = finalJobFunction;
		finalJobFunction = NULL;

		create_job(finalJobDevice, func, finalJobParameter);
	}

	wait_for_all_jobs_to_complete_and_kill();
}

void create_job(Device * device, job_func function, uintptr_t parameter)
{
	assert(deviceCount);

	Job * job = malloc(sizeof(Job));
	job->function = function;
	job->parameter = parameter;

	if(!device) {
		// Pick a CPU worker thread. Failing that, pick any thread.
		unsigned int threadsToExamine = cpuWorkerThreads ? cpuWorkerThreads : deviceCount;

		/* Find the device with the least work */

		unsigned int lowest = 0;
		int lowestValue = devices[0].jobQueue.size;

		for(unsigned int i = 1; i < threadsToExamine; i++) {
			int x = devices[i].jobQueue.size;
			if(x < lowestValue) {
				lowestValue = x;
				lowest = i;
			}
		}

		device = &devices[lowest];
	}

	/* Add the job to the back of the queue */

	aquire_mutex_lock(device->jobQueue.mutex);

	atomic_increment(&totalJobs);

	if (device->jobQueue.front) {
		job->next = 0;
		job->prev = device->jobQueue.back;
		device->jobQueue.back->next = job;
		device->jobQueue.back = job;
	}
	else {
		device->jobQueue.back = device->jobQueue.front = job;
		job->next = job->prev = NULL;
	}

	device->jobQueue.size++;


	// If the start_worker_threads() has been called then wake the thread
	if(atomic_get(&runningThreads)) {
		notify_condition_variable(device->conditionVariable);
	}
	release_mutex_lock(device->jobQueue.mutex);
}

void set_final_job(Device * device, job_func function, uintptr_t parameter)
{
	finalJobDevice = device;
	finalJobFunction = function;
	finalJobParameter = parameter;
}

void * thread_function(uintptr_t deviceIndex)
{
	if(atomic_get(&runningThreads) == (int)deviceCount-1) {
		// We are the last thread to run.
		// The main thread can sleep now until all work is done.
		release_mutex_lock(globalDeviceThreadsLock);
	}

	atomic_increment(&runningThreads);

	Device * device = &devices[deviceIndex];

	while (!atomic_get(&killAllThreads)) {
		aquire_mutex_lock(device->jobQueue.mutex);
		if (device->jobQueue.size) {
			/* Run the first job */

			Job * job = device->jobQueue.front;
			assert(job);

			if (device->jobQueue.back == device->jobQueue.front) {
				device->jobQueue.back = device->jobQueue.front = NULL;
			}
			else {
				device->jobQueue.front = device->jobQueue.front->next;
				device->jobQueue.front->prev = NULL;
			}
			device->jobQueue.size--;

			release_mutex_lock(device->jobQueue.mutex);

			int ret = job->function(job->parameter);

			if(ret) {
				/* Do the job later */

				aquire_mutex_lock(device->jobQueue.mutex);

				device->jobQueue.size++;
				if(device->jobQueue.front) {
					device->jobQueue.back->next = job;
					job->prev = device->jobQueue.back;
					device->jobQueue.back = job;
				} else {
					device->jobQueue.back = device->jobQueue.front = job;
				}

				bool queueSizeWas1 = device->jobQueue.size == 1;

				release_mutex_lock(device->jobQueue.mutex);

				if(queueSizeWas1) {
					thread_sleep(250);
				}
			} else {
				if(!atomic_decrement(&totalJobs)) {
					/* If that was the last job then we need to run the 'final job' */

					if(finalJobFunction) {
						Device * dev = finalJobDevice;

						if(!dev) {
							/* No thread specified, any thread will do. */
							/* Use this thread so a context switch is not necessary */
							dev = device;
						}

						// Reuse the job allocation - no point in freeing it and allocating another
						job->function = finalJobFunction;
						job->parameter = finalJobParameter;

						dev->jobQueue.back = dev->jobQueue.front = job;
						job->next = job->prev = NULL;
						dev->jobQueue.size = 1;

						// Unset the 'final job' so it won't get run again
						finalJobFunction = NULL;

						// Every other thread is asleep. We don't need to use atomics.
						totalJobs++;

						if(dev != device)
							notify_condition_variable(dev->conditionVariable);
					} else {
						/* No final job, all work is done. The threads can be killed */

						free(job);
						atomic_set(&killAllThreads, true);
						notify_condition_variable(mainThreadCV);
						atomic_decrement(&runningThreads);
						return 0;
					}
				} else {
					/* There are more jobs. Continue loading. */

					free(job);
				}
			}
		}
		else {
			/* Sleep */

			// unlocking of job queue mutex and entering sleep is done atomically to prevent a data race
			wait_on_condition_variable(device->conditionVariable, device->jobQueue.mutex);
		}
	}
	atomic_decrement(&runningThreads);
	return 0;
}

void wait_for_all_jobs_to_complete_and_kill()
{
	// Wait for all threads to start
	aquire_mutex_lock(globalDeviceThreadsLock);

	// Sleep until all work is done
	wait_on_condition_variable(mainThreadCV, globalDeviceThreadsLock);

	// Wait for threads to terminate
	for(unsigned int i = 0; i < deviceCount; i++) {
		notify_condition_variable(devices[i].conditionVariable);
		join_thread(devices[i].thread);
	}
}
