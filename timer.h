#ifndef WINGLET_TIMER_H
#define WINGLET_TIMER_H

#include <sys/timeb.h>
#include <stdio.h>
#include "defines.h"

struct Timer
{
	U64   startTime;   
	U64   stopTime;    
	U64   currentTime;
	U64   stopTimeDelta;
	timeb startBuffer;   
	timeb stopBuffer;   
	timeb currentBuffer;
	BOOLTYPE running;  


#pragma warning (disable: 4244)

	Timer()
	{
		running       = false;
		startTime     = 0;
		stopTime      = 0;
		stopTimeDelta = 0;
	}

	void init()
	{
		if (!running)
		{
			running = true;
			ftime(&startBuffer);
			startTime = startBuffer.time * 1000 + startBuffer.millitm + stopTimeDelta;
		}
		return;
	}

	void stop()
	{
		if (running)
		{
			running = false;
			ftime(&stopBuffer);
			stopTime = stopBuffer.time * 1000 + stopBuffer.millitm;
			stopTimeDelta = startTime - stopTime;
		}
		return;
	}

	void reset()
	{
		if (running)
		{
			ftime(&startBuffer);
			startTime = startBuffer.time * 1000 + startBuffer.millitm;
		}
		else
		{
			startTime = stopTime;
			stopTimeDelta = 0;
		}
		return;
	}

	void display()
	{
		if (running)
		{
			ftime(&currentBuffer);
			currentTime = currentBuffer.time * 1000 + currentBuffer.millitm;
			printf("%6.2f", (currentTime - startTime)/1000.0);
		}
		else
			printf("%6.2f", (stopTime - startTime)/1000.0);
		return;
	}

	void displayhms()
	{
		int hh, mm, ss;
		if (running)
		{
			ftime(&currentBuffer);
			currentTime = currentBuffer.time * 1000 + currentBuffer.millitm;
			hh = (currentTime - startTime)/1000/3600;
			mm = ((currentTime - startTime)-hh*3600000)/1000/60;
			ss = ((currentTime - startTime)-hh*3600000-mm*60000)/1000;
			printf("%02d:%02d:%02d", hh, mm, ss);
		}
		else
		{
			hh = (stopTime - startTime)/1000/3600;
			mm = ((stopTime - startTime)-hh*3600000)/1000/60;
			ss = ((stopTime - startTime)-hh*3600000-mm*60000)/1000;
			printf("%02d:%02d:%02d", hh, mm, ss);
		}
		return;
	}

	U64 getms()
	{
		if (running)
		{
			ftime(&currentBuffer);
			currentTime = currentBuffer.time * 1000 + currentBuffer.millitm;
			return (currentTime - startTime) ;
		}
		else
			return (stopTime - startTime);
	}

	U64 getsysms()
	{
		ftime(&currentBuffer);
		return (currentBuffer.time * 1000 + currentBuffer.millitm);
	}


};


#endif
