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
#include "SinkNode.hpp"

//Constructor
SinkNode::SinkNode(const ContainerFormat *pFormat, FileOutputStream *pFile, class Muxer *pMuxer)
{
    this->format = pFormat;
    this->pFile = pFile;
    this->muxer = pMuxer;
    this->headerWritten = false;
    this->finalized = false;
}

//Destructor
SinkNode::~SinkNode()
{
    delete this->pFile;
    delete this->muxer;
}

//Public methods
bool SinkNode::CanProcess() const
{
    return !this->finalized;
}

PortFormat SinkNode::GetInputFormat(uint32 inputPortNumber) const
{
    return {
        .packetsOrFrames = true,
        .frameParameters = this->muxer->GetStream(inputPortNumber)->codingParameters,
    };
}

PortFormat SinkNode::GetOutputFormat(uint32 outputPortNumber) const
{
    NOT_IMPLEMENTED_ERROR; //TODO: implement me
    return {}; //outputs nothing but whatever
}

void SinkNode::ProcessNextEntity()
{
    if(!this->headerWritten)
    {
        this->muxer->WriteHeader();
        this->headerWritten = true;
    }

    if(this->IsDataAvailable())
    {
        //we are getting packets
        NodeData nodeData = this->GetNextData();

        this->muxer->WritePacket(*nodeData.packet);
    }
    else if(!this->IsMoreInputFromInputsPortsExpected())
    {
        this->muxer->Finalize();
        this->finalized = true;
    }
}