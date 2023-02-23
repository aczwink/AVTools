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
//Local
#include "AFilter.h"

class CLink
{
    friend class CSource;
    friend class CDecoder;
    friend class CEncoder;
private:
    //Variables
    AFilter *pSource;
    AFilter *pDestination;
    uint16 sourcePinIndex;
    uint16 destinationPinIndex;
    CLinkedList<void *> dataQueue;
    CCriticalSection dataQueueLock;
    bool moreDataFollows;
public:
    //Constructor
    CLink(AFilter *pSource, uint16 sourcePinIndex, AFilter *pDestination, uint16 destinationPinIndex);
    //Functions
    void AddData(void *pData);
    void *GetData();
    bool IsFinished();
    //Inline
    inline const AFilter *GetDestination() const
    {
        return this->pDestination;
    }

    inline uint16 GetDestinationPinIndex() const
    {
        return this->destinationPinIndex;
    }

    inline const AFilter *GetSource() const
    {
        return this->pSource;
    }

    inline uint16 GetSourcePinIndex() const
    {
        return this->sourcePinIndex;
    }
};