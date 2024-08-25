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
#include "EncoderNode.hpp"

//Public methods
bool EncoderNode::CanProcess() const
{
    return this->IsDataAvailable();
}

PortFormat EncoderNode::GetInputFormat(uint32 inputPortNumber) const
{
    return {
        .packetsOrFrames = false,
        .frameParameters = this->encoderContext->Parameters(),
    };
}

PortFormat EncoderNode::GetOutputFormat(uint32 outputPortNumber) const
{
    return {
        .packetsOrFrames = true,
        .frameParameters = this->encoderContext->Parameters()
    };
}

void EncoderNode::ProcessNextEntity()
{
    NodeData data = this->GetNextData();
    this->encoderContext->Encode(*data.frame);

    while(this->encoderContext->IsPacketReady())
    {
        NodeData packet;
        packet.packet = this->encoderContext->GetNextPacket();
        this->outputPorts[0]->AddData(Move(packet));
    }
}