//Class header
#include "CDecoderThread.h"
//Local
#include "globals.h"

//Private methods
SPacket *CDecoderThread::GetNextInputPacket()
{
	SPacket *pPacket;

	this->inputPacketQueueLock.Lock();
	while(this->inputPacketQueue.IsEmpty())
	{
		this->inputPacketQueueSignal.Wait(this->inputPacketQueueLock);
		
		if(this->shutdown)
		{
			this->inputPacketQueueLock.Unlock();
			return nullptr;
		}
		if(!this->work)
		{
			this->inputPacketQueueLock.Unlock();
			return nullptr;
		}
	}
	pPacket = this->inputPacketQueue.PopFront();
	this->inputPacketQueueLock.Unlock();

	return pPacket;
}

int32 CDecoderThread::ThreadMain()
{
	SPacket *pPacket, *pReEncodedPacket;
	AFrame *pFrame;

	while(true)
	{
		//check if kill thread
		if(this->shutdown)
			break;

		//wait for work
		if(this->WaitForWork())
			continue;

		//check if we can work
		if(!this->pDecoder)
		{
			this->Stop();
			continue;
		}

		//lets work
		this->working = true;

		if(this->pDecoder->IsFrameReady())
		{
			pFrame = this->pDecoder->GetNextFrame();
			
			//encode as the desired playback format
			pReEncodedPacket = (SPacket *)MemAlloc(sizeof(*pReEncodedPacket));
			this->pEncoder->Encode(*pFrame, *pReEncodedPacket);

			//this packet is ready to be played
			this->AddOutputPacket(pReEncodedPacket);

			delete pFrame;

			continue; //go to begin
		}

		//We have no more frames for playback... decode new ones
		pPacket = this->GetNextInputPacket();
		if(pPacket)
		{
			this->pDecoder->Decode(*pPacket);
			FreePacket(*pPacket);
			MemFree(pPacket);
		}
	}

	this->working = false;

	return EXIT_SUCCESS;
}

bool CDecoderThread::WaitForWork()
{
	//wait for the instruction to work
	if(!this->work)
	{
		this->working = false;
		this->workSignal.Wait();
		if(this->work)
		{
			if(this->streamIndex != UINT32_MAX)
				this->pDecoder = g_pPlaybackHandler->GetDemuxer()->GetStream(this->streamIndex)->GetDecoder();
		}
		
		return true;
	}

	return false;
}

//Public methods
void CDecoderThread::FlushInputQueue()
{
	SPacket *pPacket;

	this->inputPacketQueueLock.Lock();
	while(!this->inputPacketQueue.IsEmpty())
	{
		pPacket = this->inputPacketQueue.PopFront();
		FreePacket(*pPacket);
		MemFree(pPacket);
	}
	this->inputPacketQueueLock.Unlock();
}

void CDecoderThread::FlushOutputQueue()
{
	SPacket *pPacket;

	this->outputPacketQueueLock.Lock();
	while(!this->outputPacketQueue.IsEmpty())
	{
		pPacket = this->outputPacketQueue.PopFront();
		FreePacket(*pPacket);
		MemFree(pPacket);
	}
	this->outputPacketQueueLock.Unlock();
}

SPacket *CDecoderThread::TryGetNextOutputPacket()
{
	SPacket *pPacket;

	pPacket = nullptr;
	
	this->outputPacketQueueLock.Lock();
	if(!this->outputPacketQueue.IsEmpty())
		pPacket = this->outputPacketQueue.PopFront();
	this->outputPacketQueueLock.Unlock();
	
	return pPacket;
}