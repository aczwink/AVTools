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
#include "SourceNode.hpp"

//Constructor
SourceNode::SourceNode(FileInputStream* fileInputStream, Demuxer* demuxer)
{
    this->fileInputStream = fileInputStream;
    this->demuxer = demuxer;
    this->endOfPacketsReached = false;

    this->outputPorts.Resize(this->demuxer->GetNumberOfStreams());
    for(auto& ptr : this->outputPorts)
        ptr = nullptr;
}

//Destructor
SourceNode::~SourceNode()
{
    delete this->fileInputStream;
    delete this->demuxer;
}

//Public methods
bool SourceNode::CanProcess() const
{
    return !this->endOfPacketsReached;
}

PortFormat SourceNode::GetInputFormat(uint32 inputPortNumber) const
{
    //has no input port
    return PortFormat();
}

PortFormat SourceNode::GetOutputFormat(uint32 outputPortNumber) const
{
    return {
        .packetsOrFrames = true,
        .frameParameters = this->demuxer->GetStream(outputPortNumber)->codingParameters,
    };
}

void SourceNode::ProcessNextEntity()
{
    auto packet = this->demuxer->ReadFrame();
    if(packet == nullptr)
    {
        this->endOfPacketsReached = true;
        return;
    }

    NodeData nodeData;
    nodeData.packet = Move(packet);

    this->outputPorts[nodeData.packet->GetStreamIndex()]->AddData(Move(nodeData));
}