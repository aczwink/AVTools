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
#include <StdXX.hpp>
using namespace StdXX;
using namespace StdXX::CommandLine;
using namespace StdXX::Multimedia;

void CreateRGB24Image(const Math::Size<uint16>& size, const Math::Vector3S& rgb, Packet& packet);
void CreateSineSound(Packet &packet, uint8 nChannels, uint32 sampleRate, float32 frequency);

static uint32 AddAudioStream(Muxer& muxer, uint32 sampleRate)
{
	Stream* stream = new Stream(DataType::Audio);
	uint32 streamIndex = muxer.AddStream(stream);

	stream->SetCodingFormat(CodingFormatId::PCM_Float32LE);
	stream->codingParameters.audio.sampleFormat = AudioSampleFormat(1, AudioSampleType::Float, false);
	stream->codingParameters.audio.sampleRate = sampleRate;

	return streamIndex;
}

static uint32 AddVideoStream(Muxer& muxer, const Math::Size<uint16>& size)
{
	Stream* stream = new Stream(DataType::Video);
	uint32 streamIndex = muxer.AddStream(stream);

	stream->SetCodingFormat(CodingFormatId::RawSinglePlaneVideo);
	stream->codingParameters.video.pixelFormat = NamedPixelFormat::BGR_24;
	stream->codingParameters.video.size = size;

	return streamIndex;
}

void Mux(const String& outputExtension, bool addAudio, bool addVideo, bool complex)
{
	FileSystem::Path outputPath = String(u8"out.") + outputExtension;
	FileOutputStream file(outputPath);

	const ContainerFormat* format = FormatRegistry::Instance().FindFormatByFileExtension(outputExtension);
	Muxer* muxer = format->CreateMuxer(file);

	uint32 sampleRate = 44100;
	Math::Size<uint16> videoSize = {720, 480};
	uint32 audioStreamIndex;
	uint32 videoStreamIndex;
	if(addAudio)
		audioStreamIndex = AddAudioStream(*muxer, sampleRate);
	if(addVideo)
		videoStreamIndex = AddVideoStream(*muxer, videoSize);

	muxer->WriteHeader();

	if(addAudio)
	{
		float32 baseFreq = 220; //A 440 Hz
		Packet packet;
		CreateSineSound(packet, 1, sampleRate, baseFreq);
		packet.streamIndex = audioStreamIndex;
		muxer->WritePacket(packet);

		if(complex)
		{
			float32 third = 5.0f/4.0f;
			CreateSineSound(packet, 1, sampleRate, baseFreq * third);
			muxer->WritePacket(packet);

			/*float32 fourth = 4.0f/3.0f;
			CreateSineSound(packet, 1, sampleRate, baseFreq * fourth);
			muxer->WritePacket(packet);*/

			float32 fifth = 3.0f/2.0f;
			CreateSineSound(packet, 1, sampleRate, baseFreq * fifth);
			muxer->WritePacket(packet);

			float32 octave = 2.0f;
			CreateSineSound(packet, 1, sampleRate, baseFreq * octave);
			muxer->WritePacket(packet);
		}
	}
	if(addVideo)
	{
		Packet packet;
		CreateRGB24Image(videoSize, {1.0f, 0.0f, 0.0f}, packet);
		packet.streamIndex = videoStreamIndex;
		muxer->WritePacket(packet);

		if(complex)
		{
			CreateRGB24Image(videoSize, {0.0f, 1.0f, 0.0f}, packet);
			packet.streamIndex = videoStreamIndex;
			muxer->WritePacket(packet);

			CreateRGB24Image(videoSize, {0.0f, 0.0f, 1.0f}, packet);
			packet.streamIndex = videoStreamIndex;
			muxer->WritePacket(packet);

			CreateRGB24Image(videoSize, {1.0f, 1.0f, 0.0f}, packet);
			packet.streamIndex = videoStreamIndex;
			muxer->WritePacket(packet);

			CreateRGB24Image(videoSize, {1.0f, 0.0f, 1.0f}, packet);
			packet.streamIndex = videoStreamIndex;
			muxer->WritePacket(packet);

			CreateRGB24Image(videoSize, {0.0f, 1.0f, 1.0f}, packet);
			packet.streamIndex = videoStreamIndex;
			muxer->WritePacket(packet);
		}
	}

	muxer->Finalize();

	delete muxer;
}

int32 Main(const String& programName, const FixedArray<String>& args)
{
	CommandLine::Parser commandLineParser(programName);
	commandLineParser.AddHelpOption();

	Option audioOption(u8'a', u8"audio", u8"Whether to include audio in the output");
	commandLineParser.AddOption(audioOption);

	Option complexOption(u8'c', u8"complex", u8"Whether to add more than just a single frame/sound");
	commandLineParser.AddOption(complexOption);

	Option videoOption(u8'v', u8"video", u8"Whether to include video in the output");
	commandLineParser.AddOption(videoOption);

	StringArgument outputExtensionArg(u8"outExtension", u8"File extension of the output file.");
	commandLineParser.AddPositionalArgument(outputExtensionArg);

	if(!commandLineParser.Parse(args))
	{
		stdErr << commandLineParser.GetErrorText() << endl;
		return EXIT_FAILURE;
	}

	if(commandLineParser.IsHelpActivated())
	{
		commandLineParser.PrintHelp();
		return EXIT_SUCCESS;
	}

	const MatchResult& matchResult = commandLineParser.ParseResult();

	Mux(outputExtensionArg.Value(matchResult), matchResult.IsActivated(audioOption), matchResult.IsActivated(videoOption), matchResult.IsActivated(complexOption));

	return EXIT_SUCCESS;
}