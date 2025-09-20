#pragma once

// d3d12 책 참고
class CGameTimer
{
public:
	CGameTimer();

	float GetTotalTime() const;
	float GetDeltaTime() const;

	void Reset();
	void Start();
	void Stop();
	void Tick();	// 매 프레임마다 호출
private:
	double seconds_per_count{};
	double delta_time{-1.0};

	/*
	time 관련 enum 선언 = > enum을 통해 배열에 접근
	ex) time[PAUSED];
	*/
	enum eTimeType {
		BASE, PAUSED, STOP, PREV, CURR,
		TIMECOUNT
	};
	__int64 time[TIMECOUNT]{};

	bool is_stop{};
};

