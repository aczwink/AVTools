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
using namespace StdXX::Multimedia;

void CreateSineSound(Packet &packet, uint8 nChannels, uint32 sampleRate, float32 frequency)
{
	const uint32 duration = 1; //seconds

	uint32 nSamplesPerChannel = duration * sampleRate;
	uint32 nSamples = nSamplesPerChannel * nChannels;

	packet.Allocate(nSamples * sizeof(float32));

	float32* current = (float32 *)packet.GetData();
	for(uint32 i = 0; i < nSamples; i++)
	{
		*current++ = (float32)sin(i * (2.0 * PI * frequency) / sampleRate);
	}
}