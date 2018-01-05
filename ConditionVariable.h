#ifdef __cplusplus
#pragma once
#endif

#ifndef _CONDITION_VARIABLE_H_
#define _CONDITION_VARIABLE_H_

#include <stdint.h>
#include "Mutex.h"

#ifdef __cplusplus
extern "C" {
#endif

	typedef uintptr_t ConditionVariable;

	ConditionVariable create_condition_variable();
	void wait_on_condition_variable(ConditionVariable, Mutex mutexToRelease);
	void notify_condition_variable(ConditionVariable);
	void destroy_condition_variable(ConditionVariable);
	

#ifdef __cplusplus
}
#endif

#endif
