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
#include "CSink.h"
//Local
#include "CLink.h"

//Constructor
CSink::CSink(uint16 x, uint16 y)
{
    this->pOutputCtx = NULL;
    this->SetCoord(x, y);
    this->SetNoOfOutputPins(0);
}

//Public Functions
CString CSink::GetTitle() const
{
    return "Output: " + this->destinationName;
}

EFilterType CSink::GetType() const
{
    return FILTERTYPE_SINK;
}

void CSink::InitForTranscoding() const
{
    this->pOutputCtx->GetMuxer()->WriteHeader(*this->pOutputCtx);
}

void CSink::Run()
{
    bool isFinished;
    SPacket *pPacket;

    isFinished = true;
    repeat(this->GetNoOfInputPins(), i)
    {
        if(this->ppInputLinks[i])
        {
            if(!this->ppInputLinks[i]->IsFinished())
            {
                pPacket = (SPacket *)this->ppInputLinks[i]->GetData();
                if(pPacket)
                {
                    this->pOutputCtx->GetMuxer()->WritePacket(*this->pOutputCtx, pPacket);
                    free(pPacket->pData);
                    free(pPacket);
                }
                isFinished = false;
            }
        }
    }

    if(isFinished)
    {
        this->pOutputCtx->GetMuxer()->Finalize(*this->pOutputCtx);
        this->pOutputCtx->GetOutputStream()->Flush();
        this->moreDataFollows = false;

        this->pOutputCtx->GetOutputStream()->Close();

#ifdef _DEBUG
        stdOut << "OUTPUT STREAM CLOSED... RERUN WILL CAUSE ACCESS VIOLATION" << endl;
		stdOut << "Bye from sink" << endl;
#endif
    }
}

CString CSink::ToString() const
{
    CString result;

    result = "Output: " + this->destinationName + "\r\n";
    result += "Input Pins:\r\n";
    repeat(this->pOutputCtx->GetNumberOfStreams(), i)
    {
        switch(this->pOutputCtx->GetStream(i)->GetType())
        {
            case DATATYPE_AUDIO:
                result += CString(i) + " -> Encoded Audio\r\n";
                break;
            case DATATYPE_SUBTITLE:
                result += CString(i) + " -> Encoded Subtitle\r\n";
                break;
            case DATATYPE_VIDEO:
                result += CString(i) + " -> Encoded Video\r\n";
                break;
        }
    }

    return result;
}