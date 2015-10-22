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

#include <LDateTime.h>

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

void Ltimeradd(const timeval* tvp, const timeval* uvp, timeval* vvp){
  vvp->tv_sec = tvp->tv_sec + uvp->tv_sec;
  vvp->tv_usec = tvp->tv_usec + uvp->tv_usec;
}

char expired(Timer* timer)
{
	timeval now, res;
	unsigned int rtc;
	// gettimeofday(&now, NULL);
	LDateTime.getRtc(&rtc);
	now.tv_sec = rtc;
	now.tv_usec = 0;
	Ltimersub(&timer->end_time, &now, &res);
	return res.tv_sec == 0;
}

void countdown_ms(Timer* timer, unsigned int timeout)
{
	timeval now;
	unsigned int rtc;
	// gettimeofday(&now, NULL);
	LDateTime.getRtc(&rtc);
	now.tv_sec = rtc;
	now.tv_usec = 0;
	timeval interval;
	if ((timeout%1000)>1)
		interval = {(timeout / 1000) + 1, 0};
	else
		interval = {(timeout / 1000), 0};
	Ltimeradd(&now, &interval, &timer->end_time);
}

void countdown(Timer* timer, unsigned int timeout)
{
	timeval now;
	unsigned int rtc;
	// gettimeofday(&now, NULL);
	LDateTime.getRtc(&rtc);
	now.tv_sec = rtc;
	now.tv_usec = 0;
	timeval interval = {timeout, 0};
	Ltimeradd(&now, &interval, &timer->end_time);
}

int left_ms(Timer* timer)
{
	timeval now, res;
	unsigned int rtc;
	// gettimeofday(&now, NULL);
	LDateTime.getRtc(&rtc);
	now.tv_sec = rtc;
	now.tv_usec = 0;
	timer->end_time.tv_usec = 0;
	Ltimersub(&timer->end_time, &now, &res);
	//printf("left %d ms\n", (res.tv_sec < 0) ? 0 : res.tv_sec * 1000 + res.tv_usec / 1000);
	return (res.tv_sec < 0) ? 0 : res.tv_sec * 1000;
}

void InitTimer(Timer* timer) {
	timer->end_time = (timeval ) { 0, 0 };
}