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
#include <ACStdLib.h>
using namespace ACStdLib;
//Local
#include "globals.h"
#include "CSinkNode.h"
#include "CSourceNode.h"
//Namespaces
using namespace ACStdLib::Multimedia;

//Global Variables
CSourceNode *g_pLastSource = nullptr;

//Prototypes
bool CheckArguments();
bool EvaluateArguments(const CLinkedList<CString> &refArgs);
void PrintManual();

int32 Main(const CString &refProgramName, const CLinkedList<CString> &refArgs)
{
    if(EvaluateArguments(refArgs))
    {
        g_filtergraph.Run();

        return EXIT_SUCCESS;
    }
    else
    {
        PrintManual();
        return EXIT_FAILURE;
    }

    PrintManual();
    return EXIT_SUCCESS;
}

void PrintManual()
{
    stdOut
            << "Usage: " << endl
            << "  transcoder" << " [file+]" << endl << endl
            << "    in file [inopt+]   an input file" << endl
            << endl
            << "    out file [outopt+]   an output file" << endl
            << endl
            << "    inopt:" << endl;
}

bool EvaluateInArgs(CLinkedListConstIterator<CString> &refIt, const CLinkedListConstIterator<CString> &refItEnd)
{
    CFileInputStream *pFile;
    const IFormat *pFormat;
    ADemuxer *pDemuxer;
    CSourceNode *pSourceNode;
    CPath path;

    //
    path = *refIt;
    ++refIt;
    if(!path.Exists())
    {
        stdErr << "File '" << path << "' does not exist." << endl;
        return false;
    }

    pFile = new CFileInputStream(path);
    pFormat = IFormat::Find(*pFile);
    if(!pFormat)
    {
        //second try by extension
        pFormat = IFormat::FindByExtension(path.GetFileExtension());
        if(pFormat)
        {
            stdOut << "Warning: File format couldn't be identified else than by extension. This is might be not avoidable but is unsafe." << endl;
        }
        else
        {
            stdErr << "No format could be found for file '" << path << "'. Either the format is not supported or this is not a valid media file." << endl;
            delete pFile;
            return false;
        }
    }

    pDemuxer = pFormat->CreateDemuxer(*pFile);
    if(!pDemuxer)
    {
        stdErr << "No demuxer is available for format '" << pFormat->GetName() << "'." << endl;
        delete pFile;
        return false;
    }

    pDemuxer->ReadHeader();
    if(!pDemuxer->FindStreamInfo())
    {
        stdErr << "Not all info could be gathered for file '" << path << "'. Expect errors..." << endl;
    }

    pSourceNode = new CSourceNode(pFile, pDemuxer);
    g_pLastSource = pSourceNode;

    g_filtergraph.AddNode(pSourceNode);

    //eval args
    while(refIt != refItEnd)
    {
        if(*refIt == "out")
        {
            break;
        }
        else
        {
            ASSERT(0);
        }
    }

    return true;
}

bool EvaluateOutArgs(CLinkedListConstIterator<CString> &refIt, const CLinkedListConstIterator<CString> &refItEnd)
{
    CFileOutputStream *pFile;
    const IFormat *pFormat;
    AMuxer *pMuxer;
    CSinkNode *pSink;
    CPath path;

    //
    path = *refIt;
    ++refIt;

    pFormat = IFormat::FindByExtension(path.GetFileExtension());
    if(!pFormat)
    {
        stdErr << "No format could be found for file '" << path << "'." << endl;
        return false;
    }

    pFile = new CFileOutputStream(path);

    pMuxer = pFormat->CreateMuxer(*pFile);
    if(!pMuxer)
    {
        stdErr << "No muxer is available for format '" << pFormat->GetName() << "'." << endl;
        delete pFile;
        return false;
    }

    pSink = new CSinkNode(pFormat, pFile, pMuxer);
    g_filtergraph.AddNode(pSink);

    //eval args
    while(refIt != refItEnd)
    {
        if(*refIt == "out")
        {
            break;
        }
        else
        {
            ASSERT(0);
        }
    }

    pSink->ConnectAll(*g_pLastSource);

    pMuxer->WriteHeader();

    return true;
}

bool EvaluateArguments(const CLinkedList<CString> &refArgs)
{
    auto &refIt = refArgs.begin();
    while(refIt != refArgs.end())
    {
        if(*refIt == "in")
        {
            ++refIt;
            if(!EvaluateInArgs(refIt, refArgs.end()))
                return false;
        }
        else if(*refIt == "out")
        {
            ++refIt;
            if(!EvaluateOutArgs(refIt, refArgs.end()))
                return false;
        }
        else
        {
            return false;
        }
    }

    return CheckArguments();
}

bool CheckArguments()
{
    //there must be at least one source and one sink
    if(!g_pLastSource)
        return false;

    return true;
}