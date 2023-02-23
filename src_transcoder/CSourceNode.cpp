/*
 * Copyright (c) 2023 Amir Czwink (amir130@hotmail.de)
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
#include "CSourceNode.h"
//Local
#include "globals.h"

//Constructor
CSourceNode::CSourceNode(CFileInputStream *pFile, ADemuxer *pDemuxer)
{
    this->pFile = pFile;
    this->pDemuxer = pDemuxer;
}

//Destructor
CSourceNode::~CSourceNode()
{
    delete this->pFile;
    delete this->pDemuxer;
}

//Public methods
bool CSourceNode::OutputsRaw() const
{
    return true;
}

void CSourceNode::Run()
{
    SPacket *pPacket;

    while(true)
    {
        pPacket = (SPacket *)MemAlloc(sizeof(*pPacket));
        if(!this->pDemuxer->ReadFrame(*pPacket))
        {
            MemFree(pPacket);
            break;
        }

        this->outLinks[pPacket->streamIndex]->AddData(pPacket);
    }
}