/*
 * Copyright (c) 2017-2024 Amir Czwink (amir130@hotmail.de)
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
static void PrintTime(uint64 t, const TimeScale &timeScale)
{
	Math::Rational<uint64> timeScaleReduced = timeScale.Reduce();
	if(t == Unsigned<uint64>::Max())
		stdOut << "Unknown";
	else
	{
		const uint64 seconds = (t * timeScaleReduced).DivideAndRoundDown();
		const uint64 fractionalSeconds = (t * timeScaleReduced.numerator) % timeScaleReduced.denominator;
		TimeScale fractionalTimeScale = {1, timeScaleReduced.denominator};
		TimeScale nanoSecondsTimeScale = {1, 1000000000 };

		Time time;
		time = time.AddSecs( seconds );
		time = time.AddNanoseconds(fractionalTimeScale.Rescale(fractionalSeconds, nanoSecondsTimeScale));

		stdOut << time.ToISOString();
	}
	stdOut << endl;
}

//Constructor
Prober::Prober(const FileSystem::Path &path) : path(path), input(path)
{
	this->packetCounter = 0;
	this->totalFrameCounter = 0;
}

//Private methods
void Prober::FlushAudioFrame(uint32 streamIndex, AudioFrame &frame)
{
	const Stream &sourceStream = *this->streams[streamIndex].sourceStream;
	const AudioBuffer *audioBuffer = frame.GetAudioBuffer();

	//check if we need to resample
	EncoderContext *encoderContext = this->streams[streamIndex].muxer->GetStream(0)->GetEncoderContext();
	if (sourceStream.codingParameters.audio.sampleFormat->sampleType != AudioSampleType::S16)
	{
		AudioBuffer *resampled = audioBuffer->Resample(*sourceStream.codingParameters.audio.sampleFormat, AudioSampleFormat(sourceStream.codingParameters.audio.sampleFormat->nChannels, AudioSampleType::S16, false));
		AudioFrame resampledFrame(resampled);
		resampledFrame.pts = frame.pts;

		encoderContext->Encode(resampledFrame);
	}
	else
	{
		encoderContext->Encode(frame);
	}

	while (encoderContext->IsPacketReady())
	{
		Packet *packet = encoderContext->GetNextPacket();
		this->streams[streamIndex].muxer->WritePacket(*packet);
		delete packet;
	}
}

void Prober::FlushDecodedFrames()
{
	for(uint32 i = 0; i < this->demuxer->GetNumberOfStreams(); i++)
	{
		DecoderContext *const& refpDecoder = this->demuxer->GetStream(i)->GetDecoderContext();
		if(refpDecoder && refpDecoder->IsFrameReady())
		{
			this->FlushFrame(i, refpDecoder->GetNextFrame());
		}
	}
}

void Prober::FlushFrame(uint32 streamIndex, Frame *frame)
{
	stdOut << u8"Frame #" << this->totalFrameCounter << u8", stream frame #" << this->streams[streamIndex].frameCounter << endl;

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

void Prober::FlushVideoFrame(uint32 streamIndex, VideoFrame &frame)
{
	StreamHandler& streamHandler = this->streams[streamIndex];

	//create stream
	Stream* stream = new Stream(DataType::Video);

	stream->codingParameters.video.size = frame.GetPixmap()->GetSize();
	stream->timeScale = TimeScale(1, 1);
	stream->SetCodingFormat(CodingFormatId::RawVideo);
	stream->codingParameters.video.pixelFormat = PixelFormat(NamedPixelFormat::BGR_24);
	stream->SetEncoderContext(stream->codingParameters.codingFormat->GetBestMatchingEncoder()->CreateContext(*stream));

	EncoderContext *encoder = stream->GetEncoderContext();

	if (streamHandler.resampler)
	{
		//resample
		Pixmap* resampledPixmap = streamHandler.resampler->Run(*frame.GetPixmap());

		VideoFrame resampledFrame(resampledPixmap);
		resampledFrame.pts = frame.pts;

		encoder->Encode(resampledFrame);
	}
	else
	{
		encoder->Encode(frame);
	}
	encoder->Flush();

	//create file
	FileSystem::Path dir = u8"stream " + String::Number(streamIndex);
	String name = String::Number(this->streams[streamIndex].frameCounter) + u8".bmp";

	FileOutputStream file(dir / name, true);

	//mux
	Muxer *muxer = FormatRegistry::Instance().FindFormatByFileExtension(u8"bmp")->CreateMuxer(file);
	muxer->AddStream(stream);

	muxer->WriteHeader();
	while (encoder->IsPacketReady())
	{
		Packet *packet = encoder->GetNextPacket();
		muxer->WritePacket(*packet);
		delete packet;
	}
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
		const CodingFormat *codingFormat = stream->codingParameters.codingFormat;
		DecoderContext *decoderContext = stream->GetDecoderContext();

		stdOut << "Stream " << i << " - ";

		switch(stream->codingParameters.dataType)
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
		PrintTime(stream->duration, stream->timeScale);

		//time scale
		stdOut << "    Time scale: ";
		if(stream->timeScale == Math::Rational<uint64>())
		{
			stdOut << "Unknown";
		}
		else
		{
			stdOut << stream->timeScale.numerator << " / " << stream->timeScale.denominator;
		}
		stdOut << endl;

		//Bit rate
		stdOut << "    Bitrate mode: ";
		if(stream->codingParameters.vbr.HasValue())
			 stdOut << (*stream->codingParameters.vbr ? "Variable (VBR)" : "Constant (CBR)");
		else
			stdOut << u8"Unknown";
		stdOut << endl << "    ";
		if(stream->codingParameters.vbr.HasValue() and *stream->codingParameters.vbr)
			stdOut << "Average bitrate: ";
		else
			stdOut << "Bitrate: ";
		if(stream->codingParameters.bitRate)
			stdOut << String::FormatBinaryPrefixed(stream->codingParameters.bitRate, u8"b") << "/s";
		else
			stdOut << "Unknown";
		stdOut << endl;

		//type specific
		switch(stream->codingParameters.dataType)
		{
			case DataType::Audio:
			{
				//sample rate
				stdOut << "    Sample rate: ";
				if(stream->codingParameters.audio.sampleRate == 0)
					stdOut << "Unknown";
				else
					stdOut << stream->codingParameters.audio.sampleRate << " Hz";

				//channels
				stdOut << endl
					<< u8"    Sample format: ";
				if (stream->codingParameters.audio.sampleFormat.HasValue())
				{
					stdOut << endl;
					stdOut << u8"      Channels: " << stream->codingParameters.audio.sampleFormat->nChannels << endl;
					stdOut << u8"      Channel Layout: " << endl;
					for (uint8 i = 0; i < stream->codingParameters.audio.sampleFormat->nChannels; i++)
					{
						const auto &ch = stream->codingParameters.audio.sampleFormat->channels[i];
						stdOut << u8"        " << i + 1 << u8":";
						switch (ch.speaker)
						{
						case SpeakerPosition::Front_Left:
							stdOut << u8"Front left";
							break;
						case SpeakerPosition::Front_Right:
							stdOut << u8"Front right";
							break;
						case SpeakerPosition::Front_Center:
							stdOut << u8"Front center";
							break;
						case SpeakerPosition::LowFrequency:
							stdOut << u8"Low frequency";
							break;
						case SpeakerPosition::Side_Left:
							stdOut << u8"Side left";
							break;
						case SpeakerPosition::Side_Right:
							stdOut << u8"Side right";
							break;
						default:
							NOT_IMPLEMENTED_ERROR;
						}
						stdOut << endl;
					}
				}
				else
					stdOut << u8"Unknown" << endl;
			}
				break;
			case DataType::Subtitle:
			{
			}
			break;
			case DataType::Video:
			{
				//resolution
				stdOut << "    Resolution: ";
				if(stream->codingParameters.video.size.width == 0 || stream->codingParameters.video.size.height == 0)
					stdOut << "Unknown";
				else
					stdOut << stream->codingParameters.video.size.width << "x" << stream->codingParameters.video.size.height;
				stdOut << endl;

				//aspect ratio
				Math::Rational<uint16> aspectRatio = stream->codingParameters.video.AspectRatio();
				stdOut << "    Aspect ratio: ";
				if(aspectRatio.numerator == 0 || aspectRatio.denominator == 0)
					stdOut << "Unknown";
				else
					stdOut << aspectRatio.numerator << ":" << aspectRatio.denominator;
				stdOut << endl;

				//pixel format
				stdOut << u8"    Frame pixel format: ";
				if (stream->codingParameters.video.pixelFormat.HasValue())
				{
					stdOut << endl;
					stdOut << u8"        Number of planes: " << stream->codingParameters.video.pixelFormat->nPlanes << endl;
					stdOut << u8"        Color space: ";

					switch (stream->codingParameters.video.pixelFormat->colorSpace)
					{
					case ColorSpace::RGB:
						stdOut << u8"RGB";
						break;
					case ColorSpace::RGBA:
						stdOut << u8"RGBA";
						break;
					case ColorSpace::YCbCr:
						stdOut << u8"YCbCr";
						break;
					default:
						stdOut << u8"Unknown";
					}
					stdOut << endl;
				}
				else
				{
					stdOut << u8"Unknown" << endl;
				}

				/*
				VideoDecoder *videoDecoder = (VideoDecoder *)decoder;
				if(decoder)
					stdOut << ToString(videoDecoder->GetPixelFormat());
				else
					stdOut << "Unknown";
				stdOut << endl;*/
			}
				break;
		}

		//codec
		stdOut << "    Coding format: ";
		if(codingFormat)
		{
			stdOut << codingFormat->GetName();
		}
		else
		{
			stdOut << "Unknown";
		}
		stdOut << endl;

		const Decoder *decoder = stream->decoder;
		stdOut << u8"    Decoder: ";
		if (decoder)
		{
			stdOut << decoder->GetName() << endl;
		}
		else
		{
			stdOut << u8"Not available." << endl;
		}
		stdOut << endl;

		stdOut << endl;
	}
}

void Prober::ProcessPacket()
{
	stdOut << "Packet #" << this->packetCounter << endl
		   << "Input byte offset: " << this->input.QueryCurrentOffset() << " / " << this->input.QuerySize() << endl
		   << endl;

	DecoderContext *const& refpDecoder = this->demuxer->GetStream(this->currentPacket->GetStreamIndex())->GetDecoderContext();
	if(refpDecoder)
	{
		refpDecoder->Decode(*this->currentPacket);
	}
}

//Public methods
void Prober::Probe(bool headerOnly)
{
	//first find format
	this->format = FormatRegistry::Instance().ProbeFormat(this->input);
	if(!this->format)
	{
		//second try by extension
		this->format = FormatRegistry::Instance().FindFormatByFileExtension(this->path.GetFileExtension());
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
	if(this->demuxer.IsNull())
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
	PrintTime(this->demuxer->GetStartTime(), this->demuxer->TimeScale());

	stdOut << "End time: ";
	uint64 endTime = Unsigned<uint64>::Max();
	if(this->demuxer->GetStartTime() != Unsigned<uint64>::Max() && this->demuxer->GetDuration() != Unsigned<uint64>::Max())
		endTime = this->demuxer->GetStartTime() + this->demuxer->GetDuration();
	PrintTime(endTime, this->demuxer->TimeScale());

	stdOut << "Duration: ";
	PrintTime(this->demuxer->GetDuration(), this->demuxer->TimeScale());

	//time scale
	stdOut << "Time scale: ";
	if(this->demuxer->TimeScale() == Math::Rational<uint64>())
		stdOut << "Unknown";
	else
		stdOut << this->demuxer->TimeScale().numerator << " / " << this->demuxer->TimeScale().denominator;
	stdOut << endl;

	//Bitrate
	stdOut << "Bitrate: ";
	if(this->demuxer->GetBitRate())
		stdOut << String::FormatBinaryPrefixed(this->demuxer->GetBitRate(), u8"b") << "/s";
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
		Stream *sourceStream = this->demuxer->GetStream(i);
		DecoderContext *const& refpDecoder = sourceStream->GetDecoderContext();

		this->streams[i] = StreamHandler(sourceStream);
		if(refpDecoder)
		{
			String dir = u8"stream " + String::Number(i);

			switch(this->demuxer->GetStream(i)->codingParameters.dataType)
			{
				case DataType::Audio:
				{
					Stream *const& refpSourceStream = this->demuxer->GetStream(i);

					this->streams[i].pOutput = new FileOutputStream(dir + String(".wav"), true);
					this->streams[i].muxer = FormatRegistry::Instance().FindFormatByFileExtension(u8"wav")->CreateMuxer(*this->streams[i].pOutput);

					Stream *pDestStream = new Stream(DataType::Audio);
					this->streams[i].muxer->AddStream(pDestStream);

					pDestStream->timeScale = refpSourceStream->timeScale;

					pDestStream->SetCodingFormat(CodingFormatId::PCM_S16LE);
					pDestStream->codingParameters.audio.sampleFormat = AudioSampleFormat(refpSourceStream->codingParameters.audio.sampleFormat->nChannels, AudioSampleType::S16, false);
					pDestStream->codingParameters.audio.sampleRate = refpSourceStream->codingParameters.audio.sampleRate;

					//setup encoder
					const Encoder *encoder = pDestStream->codingParameters.codingFormat->GetBestMatchingEncoder();
					pDestStream->SetEncoderContext(encoder->CreateContext(*pDestStream));

					this->streams[i].muxer->WriteHeader();
				}
				break;
				case DataType::Video:
				{
					Stream* sourceStream = this->demuxer->GetStream(i);

					FileSystem::File workingDir(FileSystem::FileSystemsManager::Instance().OSFileSystem().GetWorkingDirectory());
					workingDir.Child(dir).CreateDirectory();

					if (*sourceStream->codingParameters.video.pixelFormat != PixelFormat(NamedPixelFormat::RGB_24))
					{
						this->streams[i].resampler = new ComputePixmapResampler(sourceStream->codingParameters.video.size, *sourceStream->codingParameters.video.pixelFormat);
						this->streams[i].resampler->ChangePixelFormat(PixelFormat(NamedPixelFormat::BGR_24));
					}
				}
				break;
			}
		}
	}

	//process input
	while(true)
	{
		this->FlushDecodedFrames();

		this->currentPacket = this->demuxer->ReadFrame();
		if(this->currentPacket.IsNull())
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
			delete this->streams[i].resampler;
		}
	}
}