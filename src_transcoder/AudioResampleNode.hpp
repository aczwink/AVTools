/*
 * Copyright (c) 2023-2024 Amir Czwink (amir130@hotmail.de)
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
//Local
#include "Node.hpp"

class AudioResampleNode : public Node
{
public:
	//Constructor
	inline AudioResampleNode(const DecodingParameters& sourceParameters, const AudioSampleFormat& targetFormat) :
			sourceParameters(sourceParameters), targetFormat(targetFormat)
	{
		this->outputPorts.Resize(1);
		this->outputPorts[0] = nullptr;
	}

	bool CanProcess() const override
	{
		return this->IsDataAvailable();
	}

	PortFormat GetInputFormat(uint32 inputPortNumber) const override
	{
		return {
			.packetsOrFrames = false,
			.frameParameters = this->sourceParameters,
		};
	}

	PortFormat GetOutputFormat(uint32 outputPortNumber) const override
	{
		DecodingParameters codingParameters = this->sourceParameters;
		codingParameters.audio.sampleFormat = targetFormat;
		return {
			.packetsOrFrames = false,
			.frameParameters = codingParameters,
		};
	}

	void ProcessNextEntity() override
	{
		NodeData data = this->GetNextData();
		auto buffer = data.frame->GetAudioBuffer()->Resample(*this->sourceParameters.audio.sampleFormat, this->targetFormat);

		NodeData outputData;
		outputData.frame = new Frame(buffer);
		this->outputPorts[0]->AddData(Move(outputData));
	}

private:
	//State
	DecodingParameters sourceParameters;
	AudioSampleFormat targetFormat;
};