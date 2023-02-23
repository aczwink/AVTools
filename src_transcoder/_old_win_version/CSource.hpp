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
//Local
#include "AFilter.h"
//Namespaces
using namespace SJCMMLib;

//In this case this is already the demuxer
class CSource : public AFilter
{
    friend class CFiltergraph;
private:
    //Variables
    CString sourceName;
    CInputContext *pInputCtx;
public:
    //Constructor
    CSource(uint16 x, uint16 y);
    //Functions
    CString GetTitle() const;
    EFilterType GetType() const;
    void InitForTranscoding() const;
    void Run();
    CString ToString() const;
    //Inline
    inline void SetInputContext(CInputContext *pInputCtx)
    {
        this->pInputCtx = pInputCtx;
        this->SetNoOfOutputPins(pInputCtx->GetNumberOfStreams());
    }

    inline void SetSourceName(CString sourceName)
    {
        this->sourceName = sourceName;
    }
};