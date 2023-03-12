/*
 * Copyright (c) 2017-2023 Amir Czwink (amir130@hotmail.de)
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
#include <StdXX.hpp>
//Namespaces
using namespace StdXX;
using namespace StdXX::Multimedia;

class StreamHandler
{
public:
	//Members
	uint32 frameCounter;
	Stream *sourceStream;
	FileOutputStream *pOutput;
	Muxer *muxer;
	ComputePixmapResampler* resampler;

	//Constructor
	inline StreamHandler(Stream *sourceStream = nullptr) : sourceStream(sourceStream), resampler(nullptr)
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
	Prober(const FileSystem::Path &refPath);

	//Methods
	void Probe(bool headerOnly);

private:
	//Members
	const FileSystem::Path &path;
	FileInputStream input;
	const Format *format;
	UniquePointer<Demuxer> demuxer;
	UniquePointer<IPacket> currentPacket;
	uint32 packetCounter;
	uint32 totalFrameCounter;
	BinaryTreeMap<uint32, StreamHandler> streams;

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