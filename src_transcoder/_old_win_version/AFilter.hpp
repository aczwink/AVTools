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
//SJC
#include <SJCLib.h>
#include <SJCWinLib.h>
//Namespaces
using namespace SJCLib;
using namespace SJCWinLib;

//Forward Declarations
class CLink;

//Enums
enum EFilterState
{
    FILTERSTATE_NOTOK,
    FILTERSTATE_ACCEPTED,
    FILTERSTATE_OK
};

enum EFilterType
{
    FILTERTYPE_SOURCE,
    FILTERTYPE_SINK,
    FILTERTYPE_ENCODER,
    FILTERTYPE_DECODER,
};

class AFilter
{
    friend class CTranscoderThread;
private:
    //Variables
    uint16 x; //center
    uint16 y; //center
    uint16 height;
    uint16 nInputPins;
    uint16 nOutputPins;
    CCriticalSection filterLock;
protected:
    //Variables
    bool moreDataFollows;
    CLink **ppInputLinks;
    CLink **ppOutputLinks;
public:
    //Constructor
    AFilter();
    //Abstract
    virtual CString GetTitle() const = NULL;
    virtual EFilterType GetType() const = NULL;
    virtual void InitForTranscoding() const = NULL;
    virtual void Run() = NULL;
    virtual CString ToString() const = NULL;
    //Functions
    CRect GetInputPinRect(uint32 pinIndex) const;
    CRect GetOutputPinRect(uint32 pinIndex) const;
    EFilterState IsOk() const;
    void SetNoOfInputPins(uint16 nPins);
    void SetNoOfOutputPins(uint16 nPins);
    //Overrideable
    virtual CRect GetRect() const;
    //Inline
    inline void AddInputLink(uint16 pinIndex, CLink *pLink)
    {
        this->ppInputLinks[pinIndex] = pLink;
    }

    inline void AddOutputLink(uint16 pinIndex, CLink *pLink)
    {
        this->ppOutputLinks[pinIndex] = pLink;
    }

    inline uint16 GetNoOfInputPins() const
    {
        return this->nInputPins;
    }

    inline uint16 GetNoOfOutputPins() const
    {
        return this->nOutputPins;
    }

    inline bool IsFinished() const
    {
        return !this->moreDataFollows;
    }

    inline void SetCoord(uint16 x, uint16 y)
    {
        this->x = x;
        this->y = y;
    }
};