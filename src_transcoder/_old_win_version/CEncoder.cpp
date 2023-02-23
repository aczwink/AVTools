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
#include "CEncoder.h"
//Local
#include "CLink.h"

//Constructor
CEncoder::CEncoder(uint16 x, uint16 y)
{
    this->pEncoder = NULL;
    this->SetCoord(x, y);
    this->SetNoOfInputPins(1);
    this->SetNoOfOutputPins(1);
}

//Public Functions
CString CEncoder::GetTitle() const
{
    return "Encoder";
}

EFilterType CEncoder::GetType() const
{
    return FILTERTYPE_ENCODER;
}

void CEncoder::InitForTranscoding() const
{
}

void CEncoder::Run()
{
    CLink *pInputLink, *pOutputLink;
    IFrame *pFrame;

    pInputLink = this->ppInputLinks[0];
    pOutputLink = this->ppOutputLinks[0];
    pFrame = NULL;

    pInputLink->dataQueueLock.Enter();
    if(!pInputLink->dataQueue.IsEmpty())
    {
        pFrame = (IFrame *)pInputLink->dataQueue.PopFront();
    }
    pInputLink->dataQueueLock.Leave();

    if(pFrame)
    {
        SPacket *pPacket;

        pPacket = (SPacket *)malloc(sizeof(*pPacket));
        this->pEncoder->Encode(pPacket, pFrame);

        pFrame->Release();
        delete pFrame;

        pOutputLink->AddData(pPacket);
    }

    if(pInputLink->IsFinished())
    {
        pOutputLink->moreDataFollows = false;
        this->moreDataFollows = false;

#ifdef _DEBUG
        stdOut << "Bye from encoder" << endl;
#endif
    }
}

CString CEncoder::ToString() const
{
    CString result;

    if(this->pEncoder)
    {
        result += this->pEncoder->GetName() + "\r\n";
    }

    result += "Input Pins:\r\n";
    result += "0 -> raw data\r\n";
    result += "Output Pins:\r\n";
    result += "0 -> encoded data";

    return result;
}