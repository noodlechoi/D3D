#include "GameTimer.h"
#include "stdafx.h"

CGameTimer::CGameTimer()
{
	__int64 countPerSec;
	QueryPerformanceFrequency((LARGE_INTEGER*)&countPerSec);
	seconds_per_count = 1.0 / (double)countPerSec;
}

float CGameTimer::GetTotalTime() const
{
	if (is_stop) {
		return (float)(((time[STOP] - time[PAUSED]) - time[BASE]) * seconds_per_count);
	}
	else {
		return (float)(((time[CURR] - time[PAUSED]) - time[BASE]) * seconds_per_count);
	}
	return -1.0f;
}

float CGameTimer::GetDeltaTime() const
{
	return (float)delta_time;
}

void CGameTimer::Reset()
{
	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

	time[BASE] = currTime;
	time[PREV] = currTime;
	time[STOP] = 0;
	is_stop = false;
}


void CGameTimer::Start()
{
	__int64 startTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&startTime);

	if (is_stop) {
		// stop 시간과 start 시간의 차이 계산
		time[PAUSED] += (startTime - time[STOP]);

		time[PREV] = startTime;
		time[STOP] = 0;
		is_stop = false;
	}
}

void CGameTimer::Stop()
{
	if (!is_stop) {
		__int64 currTime;
		QueryPerformanceCounter((LARGE_INTEGER*)&currTime);

		time[STOP] = currTime;
		is_stop = true;
	}
}

void CGameTimer::Tick()
{
	if (is_stop) {
		delta_time = 0.0;
		return;
	}

	__int64 currTime;
	QueryPerformanceCounter((LARGE_INTEGER*)&currTime);
	time[CURR] = currTime;

	delta_time = (time[CURR] - time[PREV]) * seconds_per_count;

	time[PREV] = time[CURR];

	if (delta_time < 0.0) {
		delta_time = 0.0;
	}
}