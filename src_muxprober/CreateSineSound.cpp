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
#include <ACStdLib.hpp>
using namespace ACStdLib;
using namespace ACStdLib::Multimedia;

void CreateSineSound(Packet &packet, uint8 nChannels, uint32 sampleRate)
{
	uint32 nSamplesPerChannel, nSamples, i;
	float32 *pCurrent;

	const float32 frequency = 440; //A 440 Hz
	const uint32 duration = 1; //seconds

	nSamplesPerChannel = duration * sampleRate;
	nSamples = nSamplesPerChannel * nChannels;

	packet.Allocate(nSamples * sizeof(float32));

	pCurrent = (float32 *)packet.GetData();
	for(i = 0; i < nSamples; i++)
	{
		*pCurrent++ = (float32)sin(i * (2.0 * PI * frequency) / sampleRate);
	}
}