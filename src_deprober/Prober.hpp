/*
 * Copyright (c) 2017 Amir Czwink (amir130@hotmail.de)
 *
 * This file is part of AVTools.
 *
 * AVTools is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * AVTools is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with AVTools.  If not, see <http://www.gnu.org/licenses/>.
 */
#pragma once
#include <ACStdLib.hpp>
//Namespaces
using namespace ACStdLib;
using namespace ACStdLib::Multimedia;

class CStreamHandler
{
public:
	//Members
	uint32 frameCounter;
	FileOutputStream *pOutput;
	Muxer *muxer;

	//Constructor
	inline CStreamHandler()
	{
		this->frameCounter = 0;
		this->pOutput = nullptr;
		this->muxer = nullptr;
	}
};

class Prober
{
public:
	//Constructor
	Prober(const Path &refPath);

	//Destructor
	~Prober();

	//Methods
	void Probe(bool headerOnly);

private:
	//Members
	const Path &path;
	FileInputStream input;
	const Format *format;
	Demuxer *demuxer;
	Packet currentPacket;
	uint32 packetCounter;
	uint32 totalFrameCounter;
	Map<uint32, CStreamHandler> streams;

	//Methods
	void FlushAudioFrame(uint32 streamIndex, AudioFrame &frame);
	void FlushVideoFrame(uint32 streamIndex, VideoFrame &refFrame);
	void FlushDecodedFrames();
	void FlushFrame(uint32 streamIndex, Frame *frame);
	void PrintMetaInfo();
	void PrintStreamInfo();
	void ProcessPacket();

	//Inline
	inline void PrintMetaTag(const String &refKey, const String &refValue)
	{
		if(!refValue.IsEmpty())
			stdOut << "  " << refKey << ": " << refValue << endl;
	}

	inline void PrintMetaTagUInt(const String &refKey, uint64 value)
	{
		if(value)
			stdOut << "  " << refKey << ": " << value << endl;
	}
};