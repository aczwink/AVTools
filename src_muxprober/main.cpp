/*
 * Copyright (c) 2017,2021 Amir Czwink (amir130@hotmail.de)
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
using namespace StdXX::Multimedia;

void CreateSineSound(Packet &packet, uint8 nChannels, uint32 sampleRate);

static void AddAudioStream(Muxer& muxer)
{
	AudioStream* stream = new AudioStream;
	muxer.AddStream(stream);

	stream->SetCodingFormat(CodingFormatId::PCM_Float32LE);
	stream->sampleFormat = AudioSampleFormat(1, AudioSampleType::Float, false);
	stream->codingParameters.audio.sampleRate = 44100;
}

void Mux(const FileSystem::Path& outputPath)
{
	FileOutputStream file(outputPath);

	const Format* format = Format::FindByExtension(outputPath.GetFileExtension());
	Muxer* muxer = format->CreateMuxer(file);

	AddAudioStream(*muxer);

	muxer->WriteHeader();

	//write an audio packet
	Packet packet;
	CreateSineSound(packet, 1, 44100);
	packet.streamIndex = 0;
	muxer->WritePacket(packet);

	muxer->Finalize();

	delete muxer;
}

int32 Main(const String& programName, const FixedArray<String>& args)
{
	Mux(args[0]);

	return EXIT_SUCCESS;
}