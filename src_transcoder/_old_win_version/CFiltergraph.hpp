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
#include <SJCMMLib.h>
#include <SJCWinLib.h>
//Local
#include "AFilter.h"
//Namespaces
using namespace SJCLib;
using namespace SJCMMLib;
using namespace SJCWinLib;

class CFiltergraph : public CWindow
{
    friend class CMainToolBar;
private:
    //Variables
    CToolTip tooltip;
    CArray<AFilter *> filters;
    CLinkedList<CLink *> links;
    AFilter *pCurrentElementBeingMoved;
    AFilter *pCurrentElementBeingConnected;
    uint16 outputPinForCurrentElementBeingConnected;
    //Message-callbacks
    void OnLeftMouseButtonDown(uint16 keys, uint16 x, uint16 y);
    void OnLeftMouseButtonUp(uint16 keys, uint16 x, uint16 y);
    void OnMouseMove(uint16 keys);
    void OnPaint();
    void OnRightMouseButtonUp(uint16 keys, uint16 x, uint16 y);
    //Functions
    void CopyCodecValues(EDataType type, ADecoder *pDecoder, AEncoder *pEncoder);
    void CreateLink(AFilter *pSource, uint16 sourcePinIndex, AFilter *pDestination, uint16 destinationPinIndex);
    void OnChange();
    void RegisterWindowClass(const CModule &refModule) const;
public:
    //Constructor
    CFiltergraph();
    //Functions
    void AddFilter(AFilter *pFilter);
    void Create(const CWindow &refParentWindow);
};