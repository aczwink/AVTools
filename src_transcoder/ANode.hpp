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
#pragma once
#include <ACStdLib.h>
using namespace ACStdLib;
//Namespaces
using namespace ACStdLib::Multimedia;

class ANode
{
protected:
    //Members
    CArray<ANode *> inLinks;
    CArray<ANode *> outLinks;
    CLinkedList<void *> dataInput;

    //Inline
    inline bool IsDataAvailable()
    {
        return !this->dataInput.IsEmpty();
    }

    inline void *GetNextData()
    {
        return this->dataInput.PopFront();
    }

public:
    //Destructor
    virtual ~ANode(){}

    //Abstract
    virtual bool OutputsRaw() const = NULL;
    virtual void Run() = NULL;

    //Methods
    ANode *FollowPath() const;

    //Inline
    inline void AddData(void *pData)
    {
        this->dataInput.InsertTail(pData);
    }

    inline void AddOutputLink(ANode *pNode)
    {
        pNode->inLinks.Push(this);
        this->outLinks.Push(pNode);
    }

    inline ANode *GetOutputLink(uint32 index)
    {
        return this->outLinks[index];
    }
};