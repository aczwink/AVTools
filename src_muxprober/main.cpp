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
#include <Std++.hpp>
using namespace ACStdLib;
using namespace ACStdLib::Multimedia;

void CreateSineSound(Packet &packet, uint8 nChannels, uint32 sampleRate);

static void AddAudioStream(Muxer &refMuxer)
{
	AudioStream *pStream;

	pStream = new AudioStream;
	refMuxer.AddStream(pStream);

	pStream->SetCodec(CodecId::PCM_Float32LE);
	pStream->nChannels = 1;
	pStream->sampleRate = 44100;
}

void Mux()
{
	const Format *format;
	Muxer *muxer;
	Packet packet;
	Path outputPath;

	outputPath = "muxprober_out.mkv";

	FileOutputStream file(outputPath);

	format = Format::FindByExtension("mkv");

	muxer = format->CreateMuxer(file);

	AddAudioStream(*muxer);

	muxer->WriteHeader();

	//write an audio packet
	CreateSineSound(packet, 1, 44100);
	packet.streamIndex = 0;
	muxer->WritePacket(packet);

	muxer->Finalize();

	delete muxer;
}

int32 Main(const String &refProgramName, const LinkedList<String> &refArgs)
{
	Mux();

	return EXIT_SUCCESS;
}