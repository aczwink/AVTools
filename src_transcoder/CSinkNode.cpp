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
#include "CSinkNode.h"
//Local
#include "CDecoderNode.h"
#include "CFilterNode.h"
#include "CSourceNode.h"
#include "CEncoderNode.h"
#include "globals.h"

//Constructor
CSinkNode::CSinkNode(const IFormat *pFormat, CFileOutputStream *pFile, AMuxer *pMuxer)
{
    this->pFormat = pFormat;
    this->pFile = pFile;
    this->pMuxer = pMuxer;
}

//Destructor
CSinkNode::~CSinkNode()
{
    delete this->pFile;
    delete this->pMuxer;
}

//Public methods
void CSinkNode::ConnectAll(CSourceNode &refSourceNode)
{
    bool streamCopy;
    uint32 i;
    AStream *pSourceStream, *pDestStream;
    ADemuxer *pDemuxer;
    ANode *pConnectionPoint;

    pDemuxer = refSourceNode.GetDemuxer();

    for(i = 0; i < pDemuxer->GetNumberOfStreams(); i++)
    {
        //get stream
        pSourceStream = pDemuxer->GetStream(i);

        //create stream in sink
        switch(pSourceStream->GetType())
        {
            case EDataType::Audio:
                pDestStream = new CAudioStream();
                break;
            case EDataType::Subtitle:
                pDestStream = new CSubtitleStream();
                break;
            case EDataType::Video:
                pDestStream = new CVideoStream();
                break;
            default:
                NOT_IMPLEMENTED_ERROR;
                pDestStream = nullptr;
        }
        pMuxer->AddStream(pDestStream);

        //check if stream copy is possible
        streamCopy = this->pFormat->GetSupportedCodecs(pSourceStream->GetType()).Contains(pSourceStream->GetCodec()->GetId());

        //check if stream copy is possible
        if(streamCopy)
        {
            //stream copy
            pDestStream->SetCodec(pSourceStream->GetCodec());

            //directly connect with source
            pConnectionPoint = &refSourceNode;
        }
        else
        {
            //de- and reencode
            pDestStream->SetCodec(this->pFormat->GetDefaultCodec(pSourceStream->GetType()));

            //fill filter graph
            pConnectionPoint = refSourceNode.GetOutputLink(i)->FollowPath();
            if(pConnectionPoint->OutputsRaw())
            {
                CDecoderNode *pDecoderNode;

                //insert a decoder

                pDecoderNode = new CDecoderNode(pSourceStream->GetDecoder());
                g_filtergraph.AddNode(pDecoderNode);
                pConnectionPoint->AddOutputLink(pDecoderNode);
                pConnectionPoint = pConnectionPoint->FollowPath();
            }

            if(!pConnectionPoint->OutputsRaw())
            {
                CEncoderNode *pEncoderNode;

                //insert an encoder
                pEncoderNode = new CEncoderNode(pDestStream->GetEncoder());
                g_filtergraph.AddNode(pEncoderNode);
                pConnectionPoint->AddOutputLink(pEncoderNode);
                pConnectionPoint = pConnectionPoint->FollowPath();
            }
        }

        //link filtergraph
        pConnectionPoint->AddOutputLink(this);

        //copy typeless parameters
        pDestStream->timeScale = pSourceStream->timeScale;
        pDestStream->startTime = pSourceStream->startTime;
        pDestStream->duration = pSourceStream->duration;
        pDestStream->vbr = pSourceStream->vbr;
        pDestStream->bitRate = pSourceStream->bitRate;

        //copy codec parameters that haven't been specified
        switch(pSourceStream->GetType())
        {
            case EDataType::Audio:
            {
                CAudioStream *const& refpSourceAudioStream = (CAudioStream *)pSourceStream;
                CAudioStream *const& refpDestAudioStream = (CAudioStream *)pDestStream;

                refpDestAudioStream->nChannels = refpSourceAudioStream->nChannels;
                refpDestAudioStream->sampleRate = refpSourceAudioStream->sampleRate;
            }
                break;
            case EDataType::Video:
            {
                CVideoStream *const& refpSourceVideoStream = (CVideoStream *)pSourceStream;
                CVideoStream *const& refpDestVideoStream = (CVideoStream *)pDestStream;

                refpDestVideoStream->height = refpSourceVideoStream->height;
                refpDestVideoStream->width = refpSourceVideoStream->width;

                /*
                //check if we need to transfer to another color space
                decoderColorSpace = GetColorSpace(pVideoDecoder->GetPixelFormat());
                encoderColorSpace = pVideoEncoder->GetClosestColorSpace(decoderColorSpace); //virtual EColorSpace GetClosestColorSpace(EColorSpace desiredColorSpace) const = NULL;
                if(decoderColorSpace != encoderColorSpace)
                {
                    switch(decoderColorSpace)
                    {
                    case COLORSPACE_RGBA:
                        {
                            switch(encoderColorSpace)
                            {
                            case COLORSPACE_RGB:
                                {
                                    pColorConverter = new CRGBAToRGBConverter();
                                }
                                break;
                            default:
                                ASSERT(0);
                            }
                        }
                        break;
                    default:
                        ASSERT(0);
                    }

                    //insert filter
                    pConnectionPoint = refSourceNode.GetOutputLink(i)->FollowPath();
                    if(!pConnectionPoint->OutputsRaw())
                    {
                        CFilterNode *pFilterNode;

                        pFilterNode = new CFilterNode(pColorConverter);
                        g_filtergraph.AddNode(pFilterNode);
                        pConnectionPoint->AddOutputLink(pFilterNode);
                    }
                }
                */
            }
                break;
        }
    }
}

bool CSinkNode::OutputsRaw() const
{
    return false; //outputs nothing but whatever
}

void CSinkNode::Run()
{
    SPacket *pPacket;

    while(this->IsDataAvailable())
    {
        //we are getting packets
        pPacket = (SPacket *)this->GetNextData();

        this->pMuxer->WritePacket(*pPacket);

        FreePacket(*pPacket);
        MemFree(pPacket);
    }

    this->pMuxer->Finalize();
}