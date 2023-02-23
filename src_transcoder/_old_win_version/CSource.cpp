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
#include "CSource.h"
//Global
#include <iostream>
//Local
#include "CLink.h"

//Constructor
CSource::CSource(uint16 x, uint16 y)
{
    this->SetCoord(x, y);
    this->SetNoOfInputPins(0);
}

//Public Functions
CString CSource::GetTitle() const
{
    return "Input: " + this->sourceName;
}

EFilterType CSource::GetType() const
{
    return FILTERTYPE_SOURCE;
}

void CSource::InitForTranscoding() const
{
}

void CSource::Run()
{
    SPacket *pPacket;

    pPacket = (SPacket *)malloc(sizeof(*pPacket));
    if(this->pInputCtx->ReadFrame(*pPacket) == SJCMMLIB_ENDOFDATA)
    {
        this->moreDataFollows = false;
        free(pPacket);
        goto skipPacketHandling;
    }

    //handle packet
    if(this->ppOutputLinks[pPacket->streamIndex])
    {
        this->ppOutputLinks[pPacket->streamIndex]->AddData(pPacket);
    }
    else
    {
        free(pPacket->pData);
        free(pPacket);
    }

    skipPacketHandling:;

    if(!this->moreDataFollows)
    {
        repeat(this->GetNoOfOutputPins(), i)
        {
            if(this->ppOutputLinks[i])
            {
                this->ppOutputLinks[i]->moreDataFollows = false;
            }
        }

#ifdef _DEBUG
        stdOut << "Bye from Source" << endl;
#endif
    }
}

CString CSource::ToString() const
{
    CString result;

    result = "Input: " + this->sourceName + "\r\n";
    result += "Output Pins:\r\n";

    repeat(this->pInputCtx->GetNumberOfStreams(), i)
    {
        result += CString(i) + " -> ";
        switch(this->pInputCtx->GetStream(i)->GetType())
        {
            case DATATYPE_AUDIO:
                result += "Encoded Audio";
                break;
            case DATATYPE_SUBTITLE:
                result += "Encoded Subtitle";
                break;
            case DATATYPE_VIDEO:
                result += "Encoded Video";
                break;
        }
        result += "\r\n";
    }

    return result;
}