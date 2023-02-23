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
//SJCWinLib
#include <SJCWinLib.h>
//Local
#include "CFiltergraph.h"
//Namespaces
using namespace SJCWinLib;

class CMainToolBar : public CToolBar
{
private:
    //Message-callbacks
    bool OnClick(uint16 commandId);
};

class CMainWindow : public CFrame
{
    friend class CMainToolBar;
    friend class CFiltergraph;
private:
    //Variables
    CMainToolBar toolbar;
    CTab tabs;
    CPointer<CFiltergraph> pGraph;
    //Message-callbacks
    bool OnCreate();
    void OnDestroy();
    void OnSize(WPARAM resizingType, uint32 newWidth, uint32 newHeight);
};