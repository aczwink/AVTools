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
//Class header
#include "Prober.hpp"

//Local functions
static void PrintTime(uint64 startTime, const Fraction &refTimeScale)
{
	if(startTime == Natural<uint64>::Max())
		stdOut << "Unknown";
	else
		stdOut << TimeToString(startTime, refTimeScale);
	stdOut << endl;
}

//Constructor
Prober::Prober(const Path &path) : path(path), input(path)
{
	this->packetCounter = 0;
}

//Destructor
Prober::~Prober()
{
	delete this->demuxer;
}

//Private methods
void Prober::FlushAudioFrame(uint32 streamIndex, AudioFrame &frame)
{
	Packet packet;

	this->streams[streamIndex].muxer->GetStream(0)->GetEncoder()->Encode(frame, packet);
	this->streams[streamIndex].muxer->WritePacket(packet);
}

void Prober::FlushDecodedFrames()
{
	for(uint32 i = 0; i < this->demuxer->GetNumberOfStreams(); i++)
	{
		Decoder *const& refpDecoder = this->demuxer->GetStream(i)->GetDecoder();
		if(refpDecoder && refpDecoder->IsFrameReady())
		{
			this->FlushFrame(i, refpDecoder->GetNextFrame());
		}
	}
}

void Prober::FlushFrame(uint32 streamIndex, Frame *frame)
{
	Path dir;

	stdOut << "Frame #" << this->totalFrameCounter << ", stream frame #" << this->streams[streamIndex].frameCounter << endl;

	dir = ToString((uint64)streamIndex);

	//CFileOutputStream output(dir / ToString((uint64)this->streams[streamIndex].frameCounter));

	switch(frame->GetType())
	{
		case DataType::Audio:
			this->FlushAudioFrame(streamIndex, (AudioFrame &)*frame);
			break;
		case DataType::Video:
			this->FlushVideoFrame(streamIndex, (VideoFrame &)*frame);
			break;
	}

	stdOut << endl;

	delete frame;

	this->streams[streamIndex].frameCounter++;
	this->totalFrameCounter++;
}

void Prober::FlushVideoFrame(uint32 streamIndex, VideoFrame &refFrame)
{
	VideoStream *pStream;
	Packet packet;
	Path dir, name;

	//create stream
	pStream = new VideoStream;

	pStream->width = refFrame.GetImage()->GetWidth();
	pStream->height = refFrame.GetImage()->GetHeight();

	pStream->SetCodec(CodecId::RGB24);

	//encode frame
	Encoder *encoder = pStream->GetEncoder();

	encoder->Encode(refFrame, packet);

	//create file
	dir = "stream " + ToString((uint64)streamIndex);
	name = ToString((uint64)this->streams[streamIndex].frameCounter) + String(".bmp");

	FileOutputStream file(dir / name);

	//mux
	Muxer *muxer = Format::FindByExtension("bmp")->CreateMuxer(file);
	muxer->AddStream(pStream);

	muxer->WriteHeader();
	muxer->WritePacket(packet);
	muxer->Finalize();

	//clean up
	delete muxer;
}

void Prober::PrintMetaInfo()
{
	stdOut << "Meta information:" << endl;

	this->PrintMetaTag("Album", this->demuxer->metaInfo.album);
	this->PrintMetaTag("Artist", this->demuxer->metaInfo.artist);
	this->PrintMetaTag("Composer", this->demuxer->metaInfo.composer);
	this->PrintMetaTag("Title", this->demuxer->metaInfo.title);
	this->PrintMetaTagUInt("Track number", this->demuxer->metaInfo.trackNumber);
	this->PrintMetaTagUInt("Year", this->demuxer->metaInfo.year);

	if(!this->demuxer->metaInfo.additionalTags.IsEmpty())
	{
		stdOut << "  Additional tags:" << endl;
		for(const auto &refKV : this->demuxer->metaInfo.additionalTags)
		{
			stdOut << "    " << refKV.key << " => " << refKV.value << endl;
		}
	}

	stdOut << endl;
}

void Prober::PrintStreamInfo()
{
	for(uint32 i = 0; i < this->demuxer->GetNumberOfStreams(); i++)
	{
		Stream *stream = this->demuxer->GetStream(i);
		const Codec *codec = stream->GetCodec();
		Decoder *decoder = stream->GetDecoder();

		stdOut << "Stream " << i << " - ";

		switch(stream->GetType())
		{
			case DataType::Audio:
				stdOut << "Audio" << endl;
				break;
			case DataType::Subtitle:
				stdOut << "Subtitle" << endl;
				break;
			case DataType::Video:
				stdOut << "Video" << endl;
				break;
		}

		//start time
		stdOut << "    Start time: ";
		PrintTime(stream->startTime, stream->timeScale);

		//duration
		stdOut << "    Duration: ";
		if(stream->duration == Natural<uint64>::Max())
			stdOut << "Unknown";
		else
			stdOut << TimeToString(stream->duration, stream->timeScale);
		stdOut << endl;

		//time scale
		stdOut << "    Time scale: ";
		if(stream->timeScale == Fraction())
		{
			stdOut << "Unknown";
		}
		else
		{
			stdOut << stream->timeScale.numerator << " / " << stream->timeScale.denominator;
		}
		stdOut << endl;

		//Bit rate
		stdOut << "    Bitrate mode: " << (stream->vbr ? "Variable (VBR)" : "Constant (CBR)") << endl;
		stdOut << "    ";
		if(stream->vbr)
			stdOut << "Average bitrate: ";
		else
			stdOut << "Bitrate: ";
		if(stream->bitRate)
			stdOut << FormatBitSize(stream->bitRate, 2) << "/s";
		else
			stdOut << "Unknown";
		stdOut << endl;

		//type specific
		switch(stream->GetType())
		{
			case DataType::Audio:
			{
				AudioStream *const& refpAudioStream = (AudioStream *)stream;

				//sample rate
				stdOut << "    Sample rate: ";
				if(refpAudioStream->sampleRate == 0)
					stdOut << "Unknown";
				else
					stdOut << refpAudioStream->sampleRate << " Hz";

				//channels
				stdOut << endl
					   << "    Channels: " << refpAudioStream->nChannels << " (";
				switch(refpAudioStream->nChannels)
				{
					case 1:
						stdOut << "Mono";
						break;
					case 2:
						stdOut << "Stereo";
						break;
					default:
						stdOut << "unknown";
				}

				stdOut << ")" << endl;
			}
				break;
			case DataType::Subtitle:
			{
				SubtitleStream *const& refpSubtitleStream = (SubtitleStream *)stream;
			}
				break;
			case DataType::Video:
			{
				Fraction aspectRatio;

				VideoStream *const& refpVideoStream = (VideoStream *)stream;

				VideoDecoder *videoDecoder = (VideoDecoder *)decoder;
				aspectRatio = refpVideoStream->GetAspectRatio();

				//resolution
				stdOut << "    Resolution: ";
				if(refpVideoStream->width == 0 || refpVideoStream->height == 0)
					stdOut << "Unknown";
				else
					stdOut << refpVideoStream->width << "x" << refpVideoStream->height;
				stdOut << endl;

				//aspect ratio
				stdOut << "    Aspect ratio: ";
				if(aspectRatio.numerator == 0 || aspectRatio.denominator == 0)
					stdOut << "Unknown";
				else
					stdOut << aspectRatio.numerator << ":" << aspectRatio.denominator;
				stdOut << endl;

				stdOut << "    Frame pixel format: ";
				if(decoder)
					stdOut << ToString(videoDecoder->GetPixelFormat());
				else
					stdOut << "Unknown";
				stdOut << endl;
			}
				break;
		}

		//codec
		stdOut << "    Codec: ";
		if(codec)
		{
			stdOut << codec->GetName();
		}
		else
		{
			stdOut << "Unknown";
		}
		stdOut << endl;

		if(!decoder)
		{
			stdOut << "    There is no decoder available!" << endl;
		}

		stdOut << endl;
	}
}

void Prober::ProcessPacket()
{
	stdOut << "Packet #" << this->packetCounter << endl
		   << "Input byte offset: " << this->input.GetCurrentOffset() << " / " << this->input.GetSize() << endl
		   << endl;

	Decoder *const& refpDecoder = this->demuxer->GetStream(this->currentPacket.streamIndex)->GetDecoder();
	if(refpDecoder)
	{
		refpDecoder->Decode(this->currentPacket);
	}
}

//Public methods
void Prober::Probe(bool headerOnly)
{
	//first find format
	this->format = Format::Find(this->input);
	if(!this->format)
	{
		//second try by extension
		this->format = Format::FindByExtension(this->path.GetFileExtension());
		if(this->format)
		{
			stdOut << "Warning: File format couldn't be identified else than by extension. This might be not avoidable but is unsafe." << endl;
		}
		else
		{
			stdErr << "No format could be found for file '" << this->path << "'. Either the format is not supported or this is not a valid media file." << endl;
			return;
		}
	}

	//Print at least the format in case we have no demuxer
	stdOut << "Input file: " << this->path << endl
		   << "Container format: " << this->format->GetName() << endl;

	//get demuxer
	this->demuxer = this->format->CreateDemuxer(this->input);
	if(!this->demuxer)
	{
		stdErr << "No demuxer is available for the input format." << endl;
		return;
	}

	this->demuxer->ReadHeader();
	if(!this->demuxer->FindStreamInfo())
	{
		stdOut << "Failed finding all stream info. Expect Errors." << endl;
	}

	stdOut << endl;
	stdOut << "Start time: ";
	PrintTime(this->demuxer->GetStartTime(), this->demuxer->GetTimeScale());

	stdOut << "End time: ";
	uint64 endTime = Natural<uint64>::Max();
	if(this->demuxer->GetStartTime() != Natural<uint64>::Max() && this->demuxer->GetDuration() != Natural<uint64>::Max())
		endTime = this->demuxer->GetStartTime() + this->demuxer->GetDuration();
	PrintTime(endTime, this->demuxer->GetTimeScale());

	stdOut << "Duration: ";
	PrintTime(this->demuxer->GetDuration(), this->demuxer->GetTimeScale());

	//time scale
	stdOut << "Time scale: ";
	if(this->demuxer->GetTimeScale() == Fraction())
		stdOut << "Unknown";
	else
		stdOut << this->demuxer->GetTimeScale().numerator << " / " << this->demuxer->GetTimeScale().denominator;
	stdOut << endl;

	//Bitrate
	stdOut << "Bitrate: ";
	if(this->demuxer->GetBitRate())
		stdOut << FormatBitSize(this->demuxer->GetBitRate(), 2) << "/s";
	else
		stdOut << "Unknown";
	stdOut << endl;

	stdOut << endl;

	this->PrintMetaInfo();
	this->PrintStreamInfo();

	if(headerOnly)
	{
		//skip decoding
		return;
	}

	//map streams
	for(uint32 i = 0; i < this->demuxer->GetNumberOfStreams(); i++)
	{
		Decoder *const& refpDecoder = this->demuxer->GetStream(i)->GetDecoder();

		this->streams[i] = CStreamHandler();
		if(refpDecoder)
		{
			Path dir;

			dir = "stream " + ToString((uint64)i);

			switch(this->demuxer->GetStream(i)->GetType())
			{
				case DataType::Audio:
				{
					AudioStream *pDestStream;

					AudioStream *const& refpSourceStream = (AudioStream *)this->demuxer->GetStream(i);

					this->streams[i].pOutput = new FileOutputStream(dir + String(".wav"));
					this->streams[i].muxer = Format::FindByExtension("wav")->CreateMuxer(*this->streams[i].pOutput);

					pDestStream = new AudioStream();
					this->streams[i].muxer->AddStream(pDestStream);

					pDestStream->SetCodec(CodecId::PCM_S16LE);
					pDestStream->nChannels = refpSourceStream->nChannels;
					pDestStream->sampleRate = refpSourceStream->sampleRate;

					this->streams[i].muxer->WriteHeader();
				}
				break;
				case DataType::Video:
				{
					dir.CreateDirectory();
				}
				break;
			}
		}
	}

	//process input
	while(true)
	{
		this->FlushDecodedFrames();

		if(!this->demuxer->ReadFrame(this->currentPacket))
		{
			stdOut << "End of input" << endl;
			break;
		}

		this->ProcessPacket();
		this->packetCounter++;
	}

	//finalize streams
	for(uint32 i = 0; i < this->demuxer->GetNumberOfStreams(); i++)
	{
		if(this->streams[i].muxer)
		{
			this->streams[i].muxer->Finalize();
			delete this->streams[i].muxer;
			delete this->streams[i].pOutput;
		}
	}
}