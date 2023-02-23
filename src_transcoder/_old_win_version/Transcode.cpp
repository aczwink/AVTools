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
//Main Header
#include "Transcode.h"

//Global Variables
CArray<AFilter *> g_refFilters;

//CTranscoderThread Callbacks
uint32 CTranscoderThread::Procedure()
{
    bool isFinished;

    isFinished = false;

    while(!isFinished)
    {
        isFinished = true;
        repeat(g_refFilters.GetNoOfElements(), i)
        {
            if(!g_refFilters[i]->filterLock.TryEnter()) //if i can't get in, somebody else is in
            {
                isFinished = false;
                continue; //skip
            }

            if(!g_refFilters[i]->IsFinished())
            {
                g_refFilters[i]->Run();
                isFinished = false;
            }

            g_refFilters[i]->filterLock.Leave();
        }
    }

#ifdef _DEBUG
    stdOut << "Bye from transcoder thread" << endl;
#endif

    return 0;
}

void Transcode(const CArray<AFilter *> &refFilters)
{
    uint16 nThreads;
    CTranscoderThread *pThreads;

    g_refFilters = refFilters;
    nThreads = MIN(refFilters.GetNoOfElements(), 8);
    //nThreads = 8;

    repeat(g_refFilters.GetNoOfElements(), i)
    {
        g_refFilters[i]->InitForTranscoding();
    }

    pThreads = new CTranscoderThread[nThreads];
    repeat(nThreads, i)
    {
        pThreads[i].Create();
    }
}