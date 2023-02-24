#pragma once
#include <ACStdLib.h>
using namespace ACStdLib;

class CDemuxerThread : public CThread
{
private:
	//Members
	bool shutdown;
	bool work;
	CConditionVariable workSignal;
	volatile bool working;
	uint32 videoStreamIndex;
	uint32 audioStreamIndex;

	//Methods
	int32 ThreadMain();

public:
	//Constructor
	CDemuxerThread();

	//Inline
	volatile inline bool IsWorking() const
	{
		return this->working;
	}

	inline void Run()
	{
		this->work = true;
		this->workSignal.Signal();
	}
	
	inline void SetStreamIndices(uint32 videoStreamIndex, uint32 audioStreamIndex)
	{
		this->videoStreamIndex = videoStreamIndex;
		this->audioStreamIndex = audioStreamIndex;
	}

	inline void Shutdown()
	{
		this->shutdown = true;
		this->work = false;

		this->workSignal.Signal();
		this->Join();
	}

	inline void Stop()
	{
		this->work = false;
	}
};