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
#include "CEncoderNode.h"

//Public methods
bool CEncoderNode::OutputsRaw() const
{
    return true;
}

void CEncoderNode::Run()
{
    AFrame *pFrame;
    SPacket *pPacket;

    while(this->IsDataAvailable())
    {
        //we are gettings frames
        pFrame = (AFrame *)this->GetNextData();
        pPacket = (SPacket *)MemAlloc(sizeof(*pPacket));

        this->pEncoder->Encode(*pFrame, *pPacket);

        this->outLinks[0]->AddData(pPacket);

        delete pFrame;
    }
}