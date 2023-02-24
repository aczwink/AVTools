#pragma once
#include <ACStdLib.h>
using namespace ACStdLib;
using namespace ACStdLib::Multimedia;

class CDecoderThread : public CThread
{
private:
	//Members
	bool shutdown;
	bool work;
	CConditionVariable workSignal;
	volatile bool working;
	uint32 streamIndex;
	ADecoder *pDecoder;
	IEncoder *pEncoder;
	CLinkedList<SPacket *> inputPacketQueue;
	CMutex inputPacketQueueLock;
	CConditionVariable inputPacketQueueSignal;
	CLinkedList<SPacket *> outputPacketQueue;
	CMutex outputPacketQueueLock;

	//Methods
	SPacket *GetNextInputPacket();
	bool WaitForWork();
	int32 ThreadMain();

	//Inline
	inline void AddOutputPacket(SPacket *pPacket)
	{
		this->outputPacketQueueLock.Lock();
		this->outputPacketQueue.InsertTail(pPacket);
		this->outputPacketQueueLock.Unlock();
	}

public:
	//Constructor
	inline CDecoderThread(ECodecId encodingCodec)
	{
		this->shutdown = false;
		this->work = false;
		this->working = false;
		this->streamIndex = UINT32_MAX;
		this->pEncoder = ICodec::GetCodec(encodingCodec)->CreateEncoder();
	}

	//Destructor
	inline ~CDecoderThread()
	{
		this->FlushInputQueue();
		this->FlushOutputQueue();

		delete this->pEncoder;
	}
	
	//Methods
	void FlushInputQueue();
	void FlushOutputQueue();
	SPacket *TryGetNextOutputPacket();

	//Inline
	inline void AddInputPacket(SPacket *pPacket)
	{
		this->inputPacketQueueLock.Lock();
		this->inputPacketQueue.InsertTail(pPacket);
		this->inputPacketQueueLock.Unlock();
		this->inputPacketQueueSignal.Signal();
	}
	
	volatile inline bool IsWorking() const
	{
		return this->working;
	}

	inline void Run()
	{
		this->work = true;
		this->workSignal.Signal();
	}

	inline void SetStreamIndex(uint32 streamIndex)
	{
		this->streamIndex = streamIndex;
	}

	inline void Shutdown()
	{
		this->shutdown = true;
		this->work = false;

		this->workSignal.Signal();
		this->inputPacketQueueSignal.Signal();
		this->Join();
	}

	inline void Stop()
	{
		this->work = false;
		this->inputPacketQueueSignal.Signal();
	}
};