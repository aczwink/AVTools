#pragma once
#include <ACStdLib.h>
using namespace ACStdLib;
using namespace ACStdLib::Multimedia;
//Local
#include "CLogger.h"

class CPlaybackHandler
{
private:
	//Members
	CFileInputStream input;
	CLogger &refLogger;
	const IFormat *pFormat;
	ADemuxer *pDemuxer;
	CMap<uint32, CVideoStream *> videoStreams;
	CMap<uint32, CAudioStream *> audioStreams;
	CMap<uint32, CSubtitleStream *> subtitleStreams;
	uint32 videoStreamIndex;
	uint32 audioStreamIndex;
	CMutex masterClockLock;

	//Playback members
	bool playing;
	CTimer videoClock;
	CAudioVoice *pAudioVoice;
	uint32 nSubmittedAudioBuffers;
	CMutex submittedAudioBuffersLock;
	SPacket *pNextVideoPacket;
	SPacket *pNextAudioPacket;
	float64 videoFrameDelay;
	float64 audioFrameDelay;
	uint64 lastTime;
	float64 currentSyncTime;
	float64 mediaPosDelay;
	
	//Eventhandlers
	void OnAudioBufferPlayed(const void *pBuffer);
	void OnVideoClock();

	//Methods
	void InitAudioVoice();
	void Play();
	void PrintMetaInfo();
	void Stop();

	//Inline
	inline void PrintMetaTag(const CString &refKey, const CString &refValue)
	{
		if(!refValue.IsEmpty())
			this->refLogger << "  " << refKey << ": " << refValue << endl;
	}

	inline void PrintMetaTagUInt(const CString &refKey, uint64 value)
	{
		if(value)
			this->refLogger << "  " << refKey << ": " << value << endl;
	}

public:
	//Constructor
	CPlaybackHandler(const CPath &refPath, CLogger &refLogger);

	//Destructor
	~CPlaybackHandler();

	//Methods
	void *GetNextPicture(uint16 &refWidth, uint16 &refHeight);
	void TogglePlayPause();

	//Inline
	inline uint32 GetAudioStreamIndex() const
	{
		return this->audioStreamIndex;
	}

	inline const CMap<uint32, CAudioStream *> &GetAudioStreams() const
	{
		return this->audioStreams;
	}

	inline ADemuxer *GetDemuxer()
	{
		return this->pDemuxer;
	}

	inline uint32 GetVideoStreamIndex() const
	{
		return this->videoStreamIndex;
	}
	
	inline const CMap<uint32, CVideoStream *> &GetVideoStreams() const
	{
		return this->videoStreams;
	}

	inline bool IsPlaying() const
	{
		return this->playing;
	}
};