/*
 * Copyright 2010-2015 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *  http://aws.amazon.com/apache2.0
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */

/**
 * @file timer.c
 * @brief Linux implementation of the timer interface.
 */

#include <stddef.h>
#include <sys/types.h>
#include "timer_interface.h"
#include <Arduino.h>
#include <vmdatetime.h>

#include <LDateTime.h>

//sub time from timer
void Ltimersub(const timeval* tvp, const timeval* uvp, timeval* vvp){
  if (tvp->tv_sec < uvp->tv_sec){
  		vvp->tv_sec = 0;
  		return;
  }

  vvp->tv_sec = tvp->tv_sec - uvp->tv_sec;
  vvp->tv_usec = tvp->tv_usec - uvp->tv_usec;
  if (vvp->tv_usec < 0)
  {
     --vvp->tv_sec;
     vvp->tv_usec += 1000000;
  }
}

//add time to timer
void Ltimeradd(const timeval* tvp, const timeval* uvp, timeval* vvp){
  vvp->tv_sec = tvp->tv_sec + uvp->tv_sec;
  vvp->tv_usec = tvp->tv_usec + uvp->tv_usec;
}

//count timer and return true or false if the time ends up
char expired(Timer* timer)
{
	timeval now, res;
	VMUINT32 start;
	start = vm_ust_get_current_time();
	now.tv_sec = start/1000000;
	now.tv_usec = start%(now.tv_sec * 1000000);
	Ltimersub(&timer->end_time, &now, &res);

	return res.tv_sec == 0;
}

//add time to the timer in microseconds level
void countdown_ms(Timer* timer, unsigned int timeout)
{
	timeval now;
	VMUINT32 start;
	start = vm_ust_get_current_time();
	now.tv_sec = start / 1000000;
	now.tv_usec = start % (now.tv_sec * 1000000);
	timeval interval = { timeout / 1000, (timeout % 1000) * 1000 };

	Ltimeradd(&now, &interval, &timer->end_time);
}

//add time to the timer in seconds level
void countdown(Timer* timer, unsigned int timeout)
{
	timeval now;
	VMUINT32 start;
	start = vm_ust_get_current_time();
	now.tv_sec = start / 1000000;
	now.tv_usec = start % (now.tv_sec * 1000000);
	timeval interval = {timeout, 0};

	Ltimeradd(&now, &interval, &timer->end_time);
}

//calculate the left time in microseconds level
int left_ms(Timer* timer)
{
	timeval now, res;
	VMUINT32 start;
	start = vm_ust_get_current_time();
	now.tv_sec = start / 1000000;
	now.tv_usec = start % (now.tv_sec * 1000000);
	Ltimersub(&timer->end_time, &now, &res);
	//printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000;
}

void InitTimer(Timer* timer) {
	timer->end_time = (timeval ) { 0, 0 };
}