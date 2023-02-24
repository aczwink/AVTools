//Class header
#include "CPlaybackHandler.h"
//Local
#include "globals.h"

static void PrintTime(uint64 startTime, const CFraction &refTimeScale, CLogger &refLogger)
{
	if(startTime == UINT64_MAX)
		refLogger << "Unknown";
	else
		refLogger << TimeToString(startTime, refTimeScale);
	refLogger << endl;
}

//Constructor
CPlaybackHandler::CPlaybackHandler(const CPath &refPath, CLogger &refLogger) : input(refPath), refLogger(refLogger), videoClock(CFunction<void()>(&CPlaybackHandler::OnVideoClock, this))
{
	uint64 endTime;
	AStream *pStream;
	const ICodec *pCodec;
	ADecoder *pDecoder;

	this->videoStreamIndex = UINT32_MAX;
	this->audioStreamIndex = UINT32_MAX;

	this->playing = false;
	this->pAudioVoice = nullptr;
	this->nSubmittedAudioBuffers = 0;
	this->pNextAudioPacket = nullptr;
	this->pNextVideoPacket = nullptr;
	this->currentSyncTime = 0;
	this->mediaPosDelay = 0;

	//first find format
	this->pFormat = IFormat::Find(this->input);
	if(!this->pFormat)
	{
		//second try by extension
		this->pFormat = IFormat::FindByExtension(refPath.GetFileExtension());
		if(this->pFormat)
		{
			this->refLogger << "Warning: File format couldn't be identified else than by extension. This might be not avoidable but is unsafe." << endl;
		}
		else
		{
			this->refLogger << "No format could be found for file '" << refPath.GetString() << "'. Either the format is not supported or this is not a valid media file." << endl;
			return;
		}
	}
	
	//Print at least the format in case we have no demuxer
	this->refLogger << "Input file: " << refPath.GetString() << endl
		<< "Container format: " << pFormat->GetName() << endl;

	//get demuxer
	this->pDemuxer = this->pFormat->CreateDemuxer(this->input);
	if(!this->pDemuxer)
	{
		this->refLogger << "No demuxer is available for format '" << pFormat->GetName() << "'." << endl;
		return;
	}

	//for most formats reading the header should give all the info needed
	this->pDemuxer->ReadHeader();

	if(!this->pDemuxer->FindStreamInfo())
	{
		this->refLogger << "Failed finding all stream info. Expect Errors." << endl;
	}
	
	this->refLogger << endl;
	
	this->refLogger << "Start time: ";
	PrintTime(this->pDemuxer->GetStartTime(), this->pDemuxer->GetTimeScale(), this->refLogger);
	this->refLogger << "End time: ";
	endTime = UINT64_MAX;
	if(this->pDemuxer->GetStartTime() != UINT64_MAX && this->pDemuxer->GetDuration() != UINT64_MAX)
		endTime = this->pDemuxer->GetStartTime() + this->pDemuxer->GetDuration();
	PrintTime(endTime, this->pDemuxer->GetTimeScale(), this->refLogger);
	this->refLogger << "Duration: ";
	PrintTime(this->pDemuxer->GetDuration(), this->pDemuxer->GetTimeScale(), this->refLogger);
	//time scale
	this->refLogger << "Time scale: ";
	if(this->pDemuxer->GetTimeScale() == CFraction())
	{
		this->refLogger << "Unknown";
	}
	else
	{
		this->refLogger << this->pDemuxer->GetTimeScale().numerator << " / " << this->pDemuxer->GetTimeScale().denominator;
	}
	this->refLogger << endl;
	//Bitrate
	this->refLogger << "Bitrate: ";
	if(this->pDemuxer->GetBitRate())
		this->refLogger << FormatBitSize(this->pDemuxer->GetBitRate(), 2) << "/s";
	else
		this->refLogger << "Unknown";
	this->refLogger << endl;
	
	this->refLogger << endl;

	this->PrintMetaInfo();

	//check streams
	for(uint32 i = 0; i < this->pDemuxer->GetNumberOfStreams(); i++)
	{
		pStream = pDemuxer->GetStream(i);
		pCodec = pStream->GetCodec();
		pDecoder = pStream->GetDecoder();

		this->refLogger << "Stream " << i << " - ";

		switch(pStream->GetType())
		{
		case EDataType::Audio:
			this->refLogger << "Audio" << endl;
			break;
		case EDataType::Subtitle:
			this->refLogger << "Subtitle" << endl;
			break;
		case EDataType::Video:
			this->refLogger << "Video" << endl;
			break;
		}

		//start time
		refLogger << "    Start time: ";
		PrintTime(pStream->startTime, pStream->timeScale, this->refLogger);

		//duration
		this->refLogger << "    Duration: ";
		if(pStream->duration == UINT64_MAX)
			this->refLogger << "Unknown";
		else
			this->refLogger << TimeToString(pStream->duration, pStream->timeScale);
		this->refLogger << endl;

		//time scale
		this->refLogger << "    Time scale: ";
		if(pStream->timeScale == CFraction())
		{
			this->refLogger << "Unknown";
		}
		else
		{
			this->refLogger << pStream->timeScale.numerator << " / " << pStream->timeScale.denominator;
		}
		this->refLogger << endl;
		
		//Bit rate
		this->refLogger << "    Bitrate mode: " << (pStream->vbr ? "Variable (VBR)" : "Constant (CBR)") << endl;
		this->refLogger << "    ";
		if(pStream->vbr)
			this->refLogger << "Average bitrate: ";
		else
			this->refLogger << "Bitrate: ";
		if(pStream->bitRate)
			this->refLogger << FormatBitSize(pStream->bitRate, 2) << "/s";
		else
			this->refLogger << "Unknown";
		this->refLogger << endl;
		
		//type specific
		switch(pStream->GetType())
		{
		case EDataType::Audio:
			{
				CAudioStream *const& refpAudioStream = (CAudioStream *)pStream;
				
				if(pStream->AllInfoIsAvailable())
				{
					//audio playback is only possible if really all info is available
					this->audioStreams.Insert(i, refpAudioStream);
					if(this->audioStreamIndex == UINT32_MAX)
						this->audioStreamIndex = i;
				}

				//sample rate
				this->refLogger << "    Sample rate: ";
				if(refpAudioStream->sampleRate == 0)
					this->refLogger << "Unknown";
				else
					this->refLogger << refpAudioStream->sampleRate << " Hz";

				//channels
				this->refLogger << endl
					<< "    Channels: " << refpAudioStream->nChannels << " (";
				switch(refpAudioStream->nChannels)
				{
				case 1:
					this->refLogger << "Mono";
					break;
				case 2:
					this->refLogger << "Stereo";
					break;
				default:
					this->refLogger << "unknown";
				}

				this->refLogger << ")" << endl;
			}
			break;
		case EDataType::Subtitle:
			{
				CSubtitleStream *const& refpSubtitleStream = (CSubtitleStream *)pStream;

				this->subtitleStreams.Insert(i, refpSubtitleStream);
			}
			break;
		case EDataType::Video:
			{
				AVideoDecoder *pVideoDecoder;
				CFraction aspectRatio;

				CVideoStream *const& refpVideoStream = (CVideoStream *)pStream;

				pVideoDecoder = (AVideoDecoder *)pDecoder;
				aspectRatio = refpVideoStream->GetAspectRatio();

				if(pDecoder)
				{
					this->videoStreams.Insert(i, refpVideoStream);
					if(this->videoStreamIndex == UINT32_MAX)
						this->videoStreamIndex = i;
				}
				
				//resolution
				this->refLogger << "    Resolution: ";
				if(refpVideoStream->width == 0 || refpVideoStream->height == 0)
					this->refLogger << "Unknown";
				else
					this->refLogger << refpVideoStream->width << "x" << refpVideoStream->height;
				this->refLogger << endl;

				//aspect ratio
				this->refLogger << "    Aspect ratio: ";
				if(aspectRatio.numerator == 0 || aspectRatio.denominator == 0)
					this->refLogger << "Unknown";
				else
					this->refLogger << aspectRatio.numerator << ":" << aspectRatio.denominator;
				this->refLogger << endl;

				this->refLogger << "    Frame pixel format: ";
				if(pDecoder)
					this->refLogger << ToString(pVideoDecoder->GetPixelFormat());
				else
					this->refLogger << "Unknown";
				this->refLogger << endl;
			}
			break;
		}

		//codec
		this->refLogger << "    Codec: ";
		if(pCodec)
		{
			this->refLogger << pCodec->GetName();
		}
		else
		{
			this->refLogger << "Unknown";
		}
		this->refLogger << endl;

		if(!pDecoder)
		{
			this->refLogger << "    There is no decoder available!" << endl;
		}

		this->refLogger << endl;
	}

	//set up playback voice
	this->InitAudioVoice();
}

//Destructor
CPlaybackHandler::~CPlaybackHandler()
{
	this->Stop();

	if(this->pDemuxer)
		delete this->pDemuxer;
	if(this->pAudioVoice)
		delete this->pAudioVoice;

	if(this->pNextVideoPacket)
	{
		FreePacket(*this->pNextVideoPacket);
		MemFree(this->pNextVideoPacket);
	}

	if(this->pNextAudioPacket)
	{
		FreePacket(*this->pNextAudioPacket);
		MemFree(this->pNextAudioPacket);
	}
}

//Eventhandlers
void CPlaybackHandler::OnAudioBufferPlayed(const void *pBuffer)
{
	this->submittedAudioBuffersLock.Lock();
	this->nSubmittedAudioBuffers--;
	this->submittedAudioBuffersLock.Unlock();
	
	MemFree((void *)pBuffer);
}

void CPlaybackHandler::OnVideoClock()
{
	int32 nextVideoDelay, nextAudioDelay;
	float64 elapsed;
	AStream *pVideoStream, *pAudioStream;

	if(!this->masterClockLock.TryLock())
		return;
	
	//get elapsed time
	elapsed = (QueryHighPrecisionClockCounter() - this->lastTime) / (float64)QueryHighPrecisionClockFrequency();
	this->lastTime = QueryHighPrecisionClockCounter();

	this->currentSyncTime += elapsed;
	
	//check video
	if(this->videoStreamIndex != UINT32_MAX)
	{
		pVideoStream = this->videoStreams[this->videoStreamIndex];

		if(this->pNextVideoPacket)
		{
			this->videoFrameDelay -= elapsed;
			if(this->videoFrameDelay <= 0.001)
				g_pMainWnd->UpdatePicture(); //next image can be requested
		}
		else
		{
			this->pNextVideoPacket = g_pVideoDecodeThread->TryGetNextOutputPacket();
			if(this->pNextVideoPacket)
			{
				if(this->pNextVideoPacket->pts == UINT64_MAX)
					this->videoFrameDelay = 0.02; //next frame in 20ms hence 50FPS
				else
					this->videoFrameDelay = (this->pNextVideoPacket->pts * pVideoStream->timeScale.numerator / (float64)pVideoStream->timeScale.denominator) - this->currentSyncTime;
			}
		}
	}

	//check audio
	if(this->pAudioVoice)
	{
		if(this->pNextAudioPacket)
		{
			this->audioFrameDelay -= elapsed;
			if(this->audioFrameDelay < 0.001)
			{
				this->submittedAudioBuffersLock.Lock();
				if(this->nSubmittedAudioBuffers < this->pAudioVoice->GetMaximumNumberOfQueuedBuffers())
				{
					//send in new audio buffer
					this->pAudioVoice->SubmitBuffer(this->pNextAudioPacket->pData, this->pNextAudioPacket->size);
					this->nSubmittedAudioBuffers++;

					MemFree(this->pNextAudioPacket);
					this->pNextAudioPacket = nullptr;
				}
				this->submittedAudioBuffersLock.Unlock();
			}
		}

		if(!this->pNextAudioPacket)
		{
			//get next packet
			pAudioStream = this->audioStreams[this->audioStreamIndex];

			this->pNextAudioPacket = g_pAudioDecodeThread->TryGetNextOutputPacket();
			if(this->pNextAudioPacket)
			{
				if(this->pNextAudioPacket->pts == UINT64_MAX)
					this->audioFrameDelay = 0; //send buffer in immediately
				else
					this->audioFrameDelay = (this->pNextAudioPacket->pts * pAudioStream->timeScale.numerator / (float64)pAudioStream->timeScale.denominator) - this->currentSyncTime;
			}
		}
	}

	//check if we need to update gui slider
	this->mediaPosDelay -= elapsed;
	if(this->mediaPosDelay <= 0)
	{
		float64 pos;
		
		this->mediaPosDelay = 0.2; //every 200ms update slider pos
		pos = this->currentSyncTime / this->pDemuxer->GetTimeScale();
		
		g_pMainWnd->UpdateMediaPos(pos);
	}

	//check if we are at end

	if(this->currentSyncTime >= this->pDemuxer->GetDuration() * this->pDemuxer->GetTimeScale())
	{
		this->Stop();
		this->masterClockLock.Unlock();
		return;
	}

	//set next clock
	nextVideoDelay = int32(this->videoFrameDelay * 1000);
	if(nextVideoDelay <= 0)
		nextVideoDelay = 1;
	nextAudioDelay = int32(this->audioFrameDelay * 1000);
	if(nextAudioDelay <= 0)
		nextAudioDelay = 1;
	
	if(nextVideoDelay < nextAudioDelay)
		this->videoClock.StartOnce(nextVideoDelay);
	else
		this->videoClock.StartOnce(nextAudioDelay);

	this->masterClockLock.Unlock();
}

//Private methods
void CPlaybackHandler::InitAudioVoice()
{
	AAudioDecoder *pDecoder;
	SAudioPlaybackFormat playbackFormat;
	
	if(this->pAudioVoice)
	{
		delete this->pAudioVoice;
		this->pAudioVoice = nullptr;
	}

	if(this->audioStreamIndex != UINT32_MAX)
	{
		CAudioStream *const& refpStream = this->audioStreams[this->audioStreamIndex];
		
		playbackFormat.nChannels = refpStream->nChannels;
		playbackFormat.sampleRate = refpStream->sampleRate;
		playbackFormat.sampleType = EAudioSampleType::PCM_S16LE;
		
		this->pAudioVoice = new CAudioVoice(playbackFormat);
		this->pAudioVoice->BindBufferEndEvent(CFunction<void(const void *)>(&CPlaybackHandler::OnAudioBufferPlayed, this));
		
		this->nSubmittedAudioBuffers = 0;
	}
}

void CPlaybackHandler::Play()
{
	g_pDemuxerThread->SetStreamIndices(this->videoStreamIndex, this->audioStreamIndex);
	g_pAudioDecodeThread->SetStreamIndex(this->audioStreamIndex);
	g_pVideoDecodeThread->SetStreamIndex(this->videoStreamIndex);
	
	g_pDemuxerThread->Run();
	g_pAudioDecodeThread->Run();
	g_pVideoDecodeThread->Run();

	this->lastTime = QueryHighPrecisionClockCounter();
	this->videoClock.StartOnce(1);

	if(this->pAudioVoice)
		this->pAudioVoice->Start();
}

void CPlaybackHandler::PrintMetaInfo()
{
	this->refLogger << "Meta information:" << endl;
	
	this->PrintMetaTag("Album", this->pDemuxer->metaInfo.album);
	this->PrintMetaTag("Artist", this->pDemuxer->metaInfo.artist);
	this->PrintMetaTag("Composer", this->pDemuxer->metaInfo.composer);
	this->PrintMetaTag("Title", this->pDemuxer->metaInfo.title);
	this->PrintMetaTagUInt("Track number", this->pDemuxer->metaInfo.trackNumber);
	this->PrintMetaTagUInt("Year", this->pDemuxer->metaInfo.year);

	if(!this->pDemuxer->metaInfo.additionalTags.IsEmpty())
	{
		this->refLogger << "  Additional tags:" << endl;
		for(const auto &refKV : this->pDemuxer->metaInfo.additionalTags)
		{
			this->refLogger << "    " << refKV.key << " => " << refKV.value << endl;
		}
	}

	this->refLogger << endl;
}

void CPlaybackHandler::Stop()
{
	this->videoClock.Stop();

	if(this->pAudioVoice)
		this->pAudioVoice->Stop();

	g_pAudioDecodeThread->Stop();
	g_pVideoDecodeThread->Stop();
	g_pDemuxerThread->Stop();

	while(g_pAudioDecodeThread->IsWorking());
	while(g_pVideoDecodeThread->IsWorking());
	while(g_pDemuxerThread->IsWorking());
}

//Public methods
void *CPlaybackHandler::GetNextPicture(uint16 &refWidth, uint16 &refHeight)
{
	void *pPictureData;
	
	if(!this->pNextVideoPacket)
		return nullptr;

	CVideoStream *const& refpStream = this->videoStreams[this->videoStreamIndex];
	
	pPictureData = this->pNextVideoPacket->pData;
	refWidth = refpStream->width;
	refHeight = refpStream->height;

	MemFree(this->pNextVideoPacket);
	this->pNextVideoPacket = nullptr;
	
	return pPictureData;
}

void CPlaybackHandler::TogglePlayPause()
{
	if(this->playing)
	{
		this->playing = false;
		this->Stop();
	}
	else
	{
		this->playing = true;
		this->Play();
	}
}