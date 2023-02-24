//Class header
#include "CDemuxerThread.h"
using namespace ACStdLib::Multimedia;
//Local
#include "globals.h"

//Constructor
CDemuxerThread::CDemuxerThread()
{
	this->shutdown = false;
	this->work = false;
	this->working = false;
	this->videoStreamIndex = UINT32_MAX;
	this->audioStreamIndex = UINT32_MAX;
}

//Private methods
int32 CDemuxerThread::ThreadMain()
{
	SPacket *pPacket;
	ADemuxer *pDemuxer;

	pDemuxer = nullptr;
	
	while(true)
	{
		if(this->shutdown)
			break;
		
		//wait for the instruction to work
		if(!this->work)
		{
			this->working = false;
			this->workSignal.Wait();
			if(this->work)
			{
				//if we now have the instruction to work, get demuxer
				pDemuxer = g_pPlaybackHandler->GetDemuxer();
			}
			continue;
		}
		this->working = true;
		
		//read a packet
		pPacket = (SPacket *)MemAlloc(sizeof(*pPacket));
		if(pDemuxer->ReadFrame(*pPacket) == false)
		{
			MemFree(pPacket);
			this->work = false; //no more work
			continue;
		}

		//check if we want that packet
		if(pPacket->streamIndex == this->audioStreamIndex)
		{
			g_pAudioDecodeThread->AddInputPacket(pPacket);
		}
		else if(pPacket->streamIndex == this->videoStreamIndex)
		{
			g_pVideoDecodeThread->AddInputPacket(pPacket);
		}
		else
		{
			//not of interest
			FreePacket(*pPacket);
			MemFree(pPacket);
		}
	}

	this->working = false;
	
	return EXIT_SUCCESS;
}