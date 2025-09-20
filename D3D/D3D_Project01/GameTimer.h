#pragma once

// d3d12 å ����
class CGameTimer
{
public:
	CGameTimer();

	float GetTotalTime() const;
	float GetDeltaTime() const;

	void Reset();
	void Start();
	void Stop();
	void Tick();	// �� �����Ӹ��� ȣ��
private:
	double seconds_per_count{};
	double delta_time{-1.0};

	/*
	time ���� enum ���� = > enum�� ���� �迭�� ����
	ex) time[PAUSED];
	*/
	enum eTimeType {
		BASE, PAUSED, STOP, PREV, CURR,
		TIMECOUNT
	};
	__int64 time[TIMECOUNT]{};

	bool is_stop{};
};

