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
#include "CDecoder.h"
//Local
#include "CLink.h"

//Constructor
CDecoder::CDecoder(uint16 x, uint16 y)
{
    this->pDecoder = NULL;
    this->SetCoord(x, y);
    this->SetNoOfInputPins(1);
    this->SetNoOfOutputPins(1);
}

//Public Functions
CString CDecoder::GetTitle() const
{
    return "Decoder";
}

EFilterType CDecoder::GetType() const
{
    return FILTERTYPE_DECODER;
}

void CDecoder::InitForTranscoding() const
{
}

void CDecoder::Run()
{
    CLink *pInputLink, *pOutputLink;
    SPacket *pPacket;

    pInputLink = this->ppInputLinks[0];
    pOutputLink = this->ppOutputLinks[0];
    pPacket = NULL;

    pInputLink->dataQueueLock.Enter();
    if(!pInputLink->dataQueue.IsEmpty())
    {
        pPacket = (SPacket *)pInputLink->dataQueue.PopFront();
    }
    pInputLink->dataQueueLock.Leave();

    if(pPacket)
    {
        IFrame *pFrame;

        switch(this->pDecoder->GetDataType())
        {
            case DATATYPE_AUDIO:
                pFrame = new CAudioFrame;
                break;
        }

        this->pDecoder->Decode(pFrame, pPacket);
        free(pPacket->pData);
        free(pPacket);

        pOutputLink->AddData(pFrame);
    }

    if(pInputLink->IsFinished())
    {
        pOutputLink->moreDataFollows = false;
        this->moreDataFollows = false;

#ifdef _DEBUG
        stdOut << "Bye from decoder" << endl;
#endif
    }
}

CString CDecoder::ToString() const
{
    CString result;

    if(this->pDecoder)
    {
        result += this->pDecoder->GetName() + "\r\n";
    }

    result += "Input Pins:\r\n";
    result += "0 -> encoded data\r\n";
    result += "Output Pins:\r\n";
    result += "0 -> raw data";

    return result;
}