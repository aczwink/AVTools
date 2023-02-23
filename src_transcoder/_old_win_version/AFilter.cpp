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
#include "AFilter.h"
//Local
#include "CLink.h"
//Definitions
#define FILTER_DEFAULTWIDTH 200
#define FILTER_DEFAULTHEIGHT 100

//Constructor
AFilter::AFilter()
{
    this->height = FILTER_DEFAULTHEIGHT;
    this->nInputPins = 0;
    this->nOutputPins = 0;
    this->x = 0;
    this->y = 0;
    this->moreDataFollows = true;
}

//Public Functions
CRect AFilter::GetInputPinRect(uint32 pinIndex) const
{
    CRect rcInputPin, rcElement;

    rcElement = this->GetRect();

    rcInputPin.left = rcElement.left + 5;
    rcInputPin.top = rcElement.top + 5 + pinIndex * 5;
    rcInputPin.right = rcInputPin.left + 12; //12 is font size
    rcInputPin.bottom = rcInputPin.top + 12;

    return rcInputPin;
}

CRect AFilter::GetOutputPinRect(uint32 pinIndex) const
{
    CRect rcOutputPin, rcElement;

    rcElement = this->GetRect();

    rcOutputPin.left = rcElement.right - 5 - 12;
    rcOutputPin.top = rcElement.top + 5 + pinIndex * (17);
    rcOutputPin.right = rcElement.right - 5;
    rcOutputPin.bottom = rcOutputPin.top + 12;

    return rcOutputPin;
}

CRect AFilter::GetRect() const
{
    CRect rcElement;

    rcElement.left = this->x - FILTER_DEFAULTWIDTH/2;
    rcElement.top = this->y - this->height / 2;
    rcElement.right = rcElement.left + FILTER_DEFAULTWIDTH;
    rcElement.bottom = rcElement.top + this->height;

    return rcElement;
}

EFilterState AFilter::IsOk() const
{
    /*repeat(this->GetNoOfInputPins(), i)
    {
        if(!this->pInputPins[i].pElement)
            return 0;
    }*/

    repeat(this->GetNoOfOutputPins(), i)
    {
        if(!this->ppOutputLinks[i])
            return FILTERSTATE_ACCEPTED;
    }

    return FILTERSTATE_OK;
}

void AFilter::SetNoOfInputPins(uint16 nPins)
{
    this->nInputPins = nPins;
    this->ppInputLinks = (CLink **)malloc(sizeof(*this->ppInputLinks) * nPins);
    MemZero(this->ppInputLinks, sizeof(*this->ppInputLinks) * nPins);
}

void AFilter::SetNoOfOutputPins(uint16 nPins)
{
    CRect rcLastOutputPin;

    this->nOutputPins = nPins;
    this->ppOutputLinks = (CLink **)malloc(sizeof(*this->ppOutputLinks) * nPins);
    MemZero(this->ppOutputLinks, sizeof(*this->ppOutputLinks) * nPins);

    rcLastOutputPin = this->GetOutputPinRect(nPins-1);
    if(this->GetRect().bottom <= rcLastOutputPin.bottom)
    {
        this->height = (uint16)(rcLastOutputPin.bottom - this->GetOutputPinRect(0).top + 10);
    }
}