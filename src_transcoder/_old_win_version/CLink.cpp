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
#include "CLink.h"
//Local
#include "CSource.h"

//Constructor
CLink::CLink(AFilter *pSource, uint16 sourcePinIndex, AFilter *pDestination, uint16 destinationPinIndex)
{
    this->pSource = pSource;
    this->sourcePinIndex = sourcePinIndex;
    this->pDestination = pDestination;
    this->destinationPinIndex = destinationPinIndex;
    this->moreDataFollows = true;
}

//Public Functions
void CLink::AddData(void *pData)
{
    this->dataQueueLock.Enter();
    this->dataQueue.InsertTail(pData);
    this->dataQueueLock.Leave();
}

void *CLink::GetData()
{
    void *pData;

    pData = NULL;
    this->dataQueueLock.Enter();
    if(!this->dataQueue.IsEmpty())
        pData = this->dataQueue.PopFront();
    this->dataQueueLock.Leave();

    return pData;
}

bool CLink::IsFinished()
{
    bool ret;

    if(this->moreDataFollows)
        return false;
    this->dataQueueLock.Enter();
    ret = this->dataQueue.IsEmpty();
    this->dataQueueLock.Leave();

    return ret;
}