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
#include "CDecoderNode.h"

//Public methods
bool CDecoderNode::OutputsRaw() const
{
    return false;
}

void CDecoderNode::Run()
{
    SPacket *pPacket;

    while(this->IsDataAvailable())
    {
        //we are getting packets
        pPacket = (SPacket *)this->GetNextData();

        this->pDecoder->Decode(*pPacket);

        FreePacket(*pPacket);
        MemFree(pPacket);
    }

    while(this->pDecoder->IsFrameReady())
    {
        this->outLinks[0]->AddData(this->pDecoder->GetNextFrame());
    }
}