/*
  Copyright (c) 2014 MediaTek Inc.  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License..

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. 
   See the GNU Lesser General Public License for more details.
*/



#ifndef _LINKITDateTime_h
#define _LINKITDateTime_h

//datetimeInfo struct
typedef struct
{
	int year;//year
	int mon;//month,begin from 1
	int day;//day,begin from 1
	int hour;//hour,24-hour
	int min;//minute
	int sec;//second
}datetimeInfo;

// LDatetimeClass is designed to get system time related function
class LDateTimeClass
{
    
// Constructor / Destructor  
public:
	LDateTimeClass();

// Method
public:

	// Return current system time 
	//
	// RETURNS
	// if succeed, return 0, otherwise return -1
	//
	// EXAMPLE
	//	<code>
	//	#include <LDateTime.h>
	//  datetimeInfo t;
	//  unsigned int rtc;
	//	void setup()
	//  {
	//    
	//  }
	//	void loop()
	//  {
	//      LDateTime.getTime(&t);
	//      LDateTime.getRtc(&rtc);
	//      delay(1000);
	//  }
	//	</code> 
	int getTime(
		datetimeInfo *time	// [OUT] datetime Info structure
		); 
	
	// Set system time 
	//
	// RETURNS
	// if succeed, return 0, otherwise return -1
	//
	// EXAMPLE
	//	<code>
	//	#include <LDateTime.h>
	//  datetimeInfo t;
	//	void setup()
	//  {
	//    
	//  }
	//	void loop()
	//  {
	//      LDateTime.getTime(&t);
	//      LDateTime.setTime(&t);
	//      delay(1000);
	//  }
	//	</code> 
	int setTime(
		datetimeInfo *time	// [OUT] datetime Info structure
		); 

	// Get the time since the Epoch (00:00:00 UTC, January 1, 1970),
	// measured in seconds.
	//
	// RETURNS
	// if succeed, return 0, otherwise failure.
	//
	// EXAMPLE
	//	<code>
	//	#include <LDateTime.h>
	//  datetimeInfo t;
	//  unsigned int rtc;
	//	void setup()
	//  {
	//    
	//  }
	//	void loop()
	//  {
	//      LDateTime.getTime(&t);
	//      LDateTime.getRtc(&rtc);
	//      delay(1000);
	//  }
	//	</code> 
	int getRtc(
		unsigned int *rtc	// [OUT] point to the time in seconds.
		);
};

//LDateTime object
extern LDateTimeClass LDateTime;

#endif

