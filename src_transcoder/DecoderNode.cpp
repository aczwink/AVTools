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
//Class Header
#include "DecoderNode.hpp"

//Public methods
bool DecoderNode::CanProcess() const
{
    return this->IsDataAvailable();
}

PortFormat DecoderNode::GetInputFormat(uint32 inputPortNumber) const
{
    return {
        .packetsOrFrames = true,
        .frameParameters = this->codingParameters,
    };
}

PortFormat DecoderNode::GetOutputFormat(uint32 outputPortNumber) const
{
    return {
        .packetsOrFrames = false,
        .frameParameters = this->codingParameters,
    };
}

void DecoderNode::ProcessNextEntity()
{
    NodeData data = this->GetNextData();
    this->decoderContext->Decode(*data.packet);

    while(this->decoderContext->IsFrameReady())
    {
        NodeData data;
        data.frame = this->decoderContext->GetNextFrame();

        this->outputPorts[0]->AddData(Move(data));
    }
}