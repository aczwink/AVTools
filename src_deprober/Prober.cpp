/*
 * Copyright (c) 2017-2018 Amir Czwink (amir130@hotmail.de)
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
static void PrintTime(uint64 t, const Fraction &timeScale)
{
	Fraction timeScaleReduced = timeScale.Reduce();
	if(t == Natural<uint64>::Max())
		stdOut << "Unknown";
	else
	{
		Time time;
		time = time.AddSecs(t * timeScaleReduced);
		time = time.AddMSecs((t * timeScaleReduced.numerator) % timeScaleReduced.denominator);

		stdOut << time.ToISOString();
	}
	stdOut << endl;
}

//Constructor
Prober::Prober(const Path &path) : path(path), input(path)
{
	this->packetCounter = 0;
	this->totalFrameCounter = 0;
}

//Private methods
void Prober::FlushAudioFrame(uint32 streamIndex, AudioFrame &frame)
{
	const AudioStream &sourceStream = (const AudioStream &)*this->streams[streamIndex].sourceStream;
	const AudioBuffer *audioBuffer = frame.GetAudioBuffer();

	//check if we need to resample
	EncoderContext *encoderContext = this->streams[streamIndex].muxer->GetStream(0)->GetEncoderContext();
	if (sourceStream.sampleFormat->sampleType != AudioSampleType::S16)
	{
		AudioBuffer *resampled = audioBuffer->Resample(*sourceStream.sampleFormat, AudioSampleFormat(sourceStream.sampleFormat->nChannels, AudioSampleType::S16, false));
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
	VideoStream *pStream = new VideoStream;

	pStream->size = frame.GetPixmap()->GetSize();
	pStream->timeScale = Fraction(1, 1);
	pStream->SetCodingFormat(CodingFormatId::RawVideo);
	pStream->pixelFormat = PixelFormat(NamedPixelFormat::BGR_24);
	pStream->SetEncoderContext(pStream->GetCodingFormat()->GetBestMatchingEncoder()->CreateContext(*pStream));

	EncoderContext *encoder = pStream->GetEncoderContext();

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
	Path dir = u8"stream " + String::Number(streamIndex);
	String name = String::Number(this->streams[streamIndex].frameCounter) + u8".bmp";

	FileOutputStream file(dir / name, true);

	//mux
	Muxer *muxer = Format::FindByExtension(u8"bmp")->CreateMuxer(file);
	muxer->AddStream(pStream);

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
		const CodingFormat *codingFormat = stream->GetCodingFormat();
		DecoderContext *decoderContext = stream->GetDecoderContext();

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
		PrintTime(stream->duration, stream->timeScale);

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
			stdOut << String::FormatBinaryPrefixed(stream->bitRate, u8"b") << "/s";
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
					<< u8"    Sample format:";
				if (refpAudioStream->sampleFormat.HasValue())
				{
					stdOut << endl;
					stdOut << u8"      Channels: " << refpAudioStream->sampleFormat->nChannels << endl;
					stdOut << u8"      Channel Layout: " << endl;
					for (uint8 i = 0; i < refpAudioStream->sampleFormat->nChannels; i++)
					{
						const auto &ch = refpAudioStream->sampleFormat->channels[i];
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
				SubtitleStream *const& refpSubtitleStream = (SubtitleStream *)stream;
			}
				break;
			case DataType::Video:
			{
				VideoStream *const& refpVideoStream = (VideoStream *)stream;

				//resolution
				stdOut << "    Resolution: ";
				if(refpVideoStream->size.width == 0 || refpVideoStream->size.height == 0)
					stdOut << "Unknown";
				else
					stdOut << refpVideoStream->size.width << "x" << refpVideoStream->size.height;
				stdOut << endl;

				//aspect ratio
				Fraction aspectRatio = refpVideoStream->GetAspectRatio();
				stdOut << "    Aspect ratio: ";
				if(aspectRatio.numerator == 0 || aspectRatio.denominator == 0)
					stdOut << "Unknown";
				else
					stdOut << aspectRatio.numerator << ":" << aspectRatio.denominator;
				stdOut << endl;

				//pixel format
				stdOut << u8"    Frame pixel format: ";
				if (refpVideoStream->pixelFormat.HasValue())
				{
					stdOut << endl;
					stdOut << u8"        Number of planes: " << refpVideoStream->pixelFormat->nPlanes << endl;
					stdOut << u8"        Color space: ";

					switch (refpVideoStream->pixelFormat->colorSpace)
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

		const Decoder *decoder = decoderContext ? &decoderContext->GetDecoder() : nullptr;
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
		   << "Input byte offset: " << this->input.GetCurrentOffset() << " / " << this->input.GetSize() << endl
		   << endl;

	DecoderContext *const& refpDecoder = this->demuxer->GetStream(this->currentPacket.streamIndex)->GetDecoderContext();
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

			switch(this->demuxer->GetStream(i)->GetType())
			{
				case DataType::Audio:
				{
					AudioStream *const& refpSourceStream = (AudioStream *)this->demuxer->GetStream(i);

					this->streams[i].pOutput = new FileOutputStream(dir + String(".wav"), true);
					this->streams[i].muxer = Format::FindByExtension("wav")->CreateMuxer(*this->streams[i].pOutput);

					AudioStream *pDestStream = new AudioStream();
					this->streams[i].muxer->AddStream(pDestStream);

					pDestStream->timeScale = refpSourceStream->timeScale;

					pDestStream->SetCodingFormat(CodingFormatId::PCM_S16LE);
					pDestStream->sampleFormat = AudioSampleFormat(refpSourceStream->sampleFormat->nChannels, AudioSampleType::S16, false);
					pDestStream->sampleRate = refpSourceStream->sampleRate;

					//setup encoder
					const Encoder *encoder = pDestStream->GetCodingFormat()->GetBestMatchingEncoder();
					pDestStream->SetEncoderContext(encoder->CreateContext(*pDestStream));

					this->streams[i].muxer->WriteHeader();
				}
				break;
				case DataType::Video:
				{
					VideoStream *const& sourceStream = dynamic_cast<VideoStream *>(this->demuxer->GetStream(i));

					OSFileSystem::GetInstance().GetDirectory(OSFileSystem::GetInstance().GetWorkingDirectory())->CreateSubDirectory(dir);

					if (*sourceStream->pixelFormat != PixelFormat(NamedPixelFormat::RGB_24))
					{
						this->streams[i].resampler = new ComputePixmapResampler(sourceStream->size, *sourceStream->pixelFormat);
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
			delete this->streams[i].resampler;
		}
	}
}