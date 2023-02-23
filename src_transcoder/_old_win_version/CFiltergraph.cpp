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
#include "CFiltergraph.h"
//Local
#include "CDecoder.h"
#include "CEncoder.h"
#include "CEditSinkDialog.h"
#include "CEditSourceDialog.h"
#include "CLink.h"
#include "CSink.h"
#include "CSource.h"
#include "globals.h"
//Definitions
#define CFILTERGRAPH_CLASSNAME "CFiltergraph"

//Enums
enum
{
    TRACKMENU_INSERTSOURCE = 1,
    TRACKMENU_INSERTSINK,
    TRACKMENU_INSERTENCODER,
    TRACKMENU_INSERTDECODER,
};

//Constructor
CFiltergraph::CFiltergraph()
{
    this->pCurrentElementBeingMoved = NULL;
    this->pCurrentElementBeingConnected = NULL;
    this->outputPinForCurrentElementBeingConnected = 0;
}

//Message-callbacks
void CFiltergraph::OnLeftMouseButtonDown(uint16 keys, uint16 x, uint16 y)
{
    CCursor cursor;
    POINT pt;

    GetCursorPos(&pt);
    this->ScreenToClient(&pt);

    //Check if clicked an element
    repeat(this->filters.GetNoOfElements(), i)
    {
        if(this->filters[i]->GetRect().PointInRect(pt))
        {
            //Check if clicked an output pin
            repeat(this->filters[i]->GetNoOfOutputPins(), j)
            {
                if(this->filters[i]->GetOutputPinRect(j).PointInRect(pt))
                {
                    foreach(it, this->links)
                    {
                        if(it.GetValue()->GetSource() == this->filters[i] && it.GetValue()->GetSourcePinIndex() == j)
                        {
                            delete it.GetValue();
                            this->links.Delete(it.GetIndex());
                        }
                    }
                    this->pCurrentElementBeingConnected = this->filters[i];
                    this->outputPinForCurrentElementBeingConnected = j;
                    this->OnChange();
                    return;
                }
            }

            cursor.LoadSystemCursor(IDC_SIZEALL);
            this->pCurrentElementBeingMoved = this->filters[i];
            cursor.SetCursor();
            return;
        }
    }
}

void CFiltergraph::OnLeftMouseButtonUp(uint16 keys, uint16 x, uint16 y)
{
    CRect rcElement;
    CCursor cursor;
    POINT pt;

    if(this->pCurrentElementBeingConnected)
    {
        pt.x = x;
        pt.y = y;

        //Check if clicked an input pin
        repeat(this->filters.GetNoOfElements(), i)
        {
            repeat(this->filters[i]->GetNoOfInputPins(), j)
            {
                rcElement = this->filters[i]->GetInputPinRect(j);

                if(rcElement.PointInRect(pt))
                {
                    this->CreateLink(this->pCurrentElementBeingConnected, this->outputPinForCurrentElementBeingConnected, this->filters[i], j);
                    this->tooltip.UpdateTipText(this->filters.Find(this->pCurrentElementBeingConnected), this->pCurrentElementBeingConnected->ToString());
                    this->tooltip.UpdateTipText(i, this->filters[i]->ToString());
                    this->pCurrentElementBeingConnected = NULL;
                    this->outputPinForCurrentElementBeingConnected = 0;
                    this->Invalidate();
                    this->OnChange();
                    return;
                }
            }
        }

        //here is no input pin
        this->pCurrentElementBeingConnected = NULL;
        this->outputPinForCurrentElementBeingConnected = 0;
        this->MessageBoxA("You must connect to an input pin", "Can't connect pins", MB_ICONERROR);
        this->Invalidate();
        return;
    }

    this->pCurrentElementBeingMoved = NULL;
    cursor.LoadSystemCursor(IDC_ARROW);
    cursor.SetCursor();
}

void CFiltergraph::OnMouseMove(uint16 keys)
{
    POINT pt;

    GetCursorPos(&pt);
    this->ScreenToClient(&pt);

    if(this->pCurrentElementBeingConnected)
    {
        this->Invalidate();
        return;
    }

    if(this->pCurrentElementBeingMoved)
    {
        this->pCurrentElementBeingMoved->SetCoord((uint16)pt.x, (uint16)pt.y);
        this->tooltip.UpdateToolRect(this->filters.Find(this->pCurrentElementBeingMoved), this->pCurrentElementBeingMoved->GetRect());
        this->Invalidate();
        return;
    }
}

void CFiltergraph::OnPaint()
{
    CPaintDC dc(this);
    CPointer<CDeviceContext> pDC;
    CPointer<CBitmap> pBmp;
    CRect rcClient, rcElement, rcText, rcInputPin, rcInputPinText, rcOutputPin, rcOutputPinText;
    AFilter *pFilter;
    TEXTMETRIC tm;
    CColor bg, fg;

    this->GetClientRect(&rcClient);
    pDC = dc.CreateCompatibleDC();
    pBmp = dc.CreateCompatibleBitmap(rcClient.GetWidth(), rcClient.GetHeight());
    pDC->SelectObject(pBmp);

    //draw background
    pDC->FillSolidRect(rcClient, CColor(0xFFFFFF));

    pDC->SelectObject(g_defaultFont);
    pDC->GetTextMetricsA(&tm);

    repeat(this->filters.GetNoOfElements(), i)
    {
        pFilter = this->filters[i];

        //set areas
        rcElement = pFilter->GetRect();

        rcText = rcElement;
        rcText.left += 5;
        rcText.right -= 20;
        rcText.top = rcElement.top + rcElement.GetHeight()/2 - tm.tmHeight/2;
        rcText.bottom = rcText.top + tm.tmHeight;

        //set colors
        switch(pFilter->IsOk())
        {
            case FILTERSTATE_NOTOK:
                bg.Set(255, 0, 0);
                fg.Set(255, 255, 255);
                break;
            case FILTERSTATE_ACCEPTED:
                bg.Set(255, 165, 0);
                fg.Set(0, 0, 0);
                break;
            case FILTERSTATE_OK:
                bg.Set(0, 255, 127);
                fg.Set(0, 0, 0);
                break;
        }

        //draw element
        pDC->FillSolidRect(rcElement, bg);
        pDC->SetBackgroundColor(bg);
        pDC->SetTextColor(fg);

        pDC->DrawTextA(pFilter->GetTitle(), &rcText, DT_CENTER | DT_VCENTER | DT_PATH_ELLIPSIS | DT_SINGLELINE);

        //draw borders
        pDC->MoveTo(rcElement.left, rcElement.top);
        pDC->LineTo(rcElement.right, rcElement.top);
        pDC->MoveTo(rcElement.left, rcElement.top);
        pDC->LineTo(rcElement.left, rcElement.bottom);
        pDC->MoveTo(rcElement.left, rcElement.bottom);
        pDC->LineTo(rcElement.right, rcElement.bottom);
        pDC->MoveTo(rcElement.right, rcElement.top);
        pDC->LineTo(rcElement.right, rcElement.bottom+1);

        //draw it's pins
        repeat(pFilter->GetNoOfInputPins(), i)
        {
            rcInputPin = pFilter->GetInputPinRect(i);

            rcInputPinText.left = rcInputPin.right + 8;
            rcInputPinText.top = rcInputPin.top;
            rcInputPinText.right = rcInputPinText.left + 20;
            rcInputPinText.bottom = rcInputPin.bottom;

            pDC->DrawTextA(CString(i), &rcInputPinText, DT_SINGLELINE);
            pDC->FillSolidRect(rcInputPin, CColor(0));
        }

        repeat(pFilter->GetNoOfOutputPins(), i)
        {
            rcOutputPin = pFilter->GetOutputPinRect(i);

            rcOutputPinText.left = rcOutputPin.left - 20;
            rcOutputPinText.top = rcOutputPin.top;
            rcOutputPinText.right = rcOutputPin.left - 8;
            rcOutputPinText.bottom = rcOutputPin.bottom;

            pDC->DrawTextA(CString(i), &rcOutputPinText, DT_SINGLELINE | DT_RIGHT);
            pDC->FillSolidRect(rcOutputPin, CColor(0));
        }
    }

    repeat(this->filters.GetNoOfElements(), i)
    {
        pFilter = this->filters[i];

        //draw the line that gets connected now
        if(this->pCurrentElementBeingConnected == pFilter)
        {
            CRect rcTmp;
            POINT pt;

            rcTmp = pFilter->GetOutputPinRect(this->outputPinForCurrentElementBeingConnected);
            GetCursorPos(&pt);
            this->ScreenToClient(&pt);

            pDC->MoveTo(rcTmp.left + rcTmp.GetWidth()/2, rcTmp.top + rcTmp.GetHeight()/2);
            pDC->LineTo(pt.x, pt.y);
        }
    }

    //draw links
    foreach(it, this->links)
    {
        CRect rcTmp, rcTmp2;

        rcTmp = it.GetValue()->GetSource()->GetOutputPinRect(it.GetValue()->GetSourcePinIndex());
        rcTmp2 = it.GetValue()->GetDestination()->GetInputPinRect(it.GetValue()->GetDestinationPinIndex());

        pDC->MoveTo(rcTmp.left + rcTmp.GetWidth()/2, rcTmp.top + rcTmp.GetHeight()/2);
        pDC->LineTo(rcTmp2.left + rcTmp2.GetWidth()/2, rcTmp2.top + rcTmp2.GetHeight()/2);
    }

    dc.BitBlt(0, 0, rcClient.GetWidth(), rcClient.GetHeight(), pDC, 0, 0, SRCCOPY);
}

void CFiltergraph::OnRightMouseButtonUp(uint16 keys, uint16 x, uint16 y)
{
    POINT pt;
    CMenu menu;

    pt.x = x;
    pt.y = y;
    this->ClientToScreen(&pt);

    menu.CreatePopup();
    menu.AppendItem("Insert Source", TRACKMENU_INSERTSOURCE);
    menu.AppendItem("Insert Sink", TRACKMENU_INSERTSINK);
    menu.AppendSeperator();
    menu.AppendItem("Insert Encoder", TRACKMENU_INSERTENCODER);
    menu.AppendItem("Insert Decoder", TRACKMENU_INSERTDECODER);
    switch(menu.TrackPopupMenu(ALIGN_LEFT, VALIGN_TOP, pt.x, pt.y, *this->GetParent()))
    {
        case TRACKMENU_INSERTSOURCE:
        {
            CSource *pSource;
            CEditSourceDialog dlg;

            pSource = new CSource(x, y);
            dlg.Create(this->GetParent(), pSource);
            if(dlg.Run() == IDOK)
            {
                this->AddFilter(pSource);
            }
            else
            {
                delete pSource;
            }
        }
            break;
        case TRACKMENU_INSERTSINK:
        {
            CSink *pSink;
            CEditSinkDialog dlg;

            pSink = new CSink(x, y);
            dlg.Create(this->GetParent(), pSink);
            if(dlg.Run() == IDOK)
            {
                this->AddFilter(pSink);
            }
            else
            {
                delete pSink;
            }
        }
            break;
        case TRACKMENU_INSERTENCODER:
        {
            CEncoder *pEncoder;
            CSelectDialog dlg;

            const CArray<const AEncoder *> &refEncoders = GetSupportedEncoders();

            dlg.Create("Select codec", this->GetParent());
            repeat(refEncoders.GetNoOfElements(), i)
            {
                dlg.AddSelection(refEncoders[i]->GetName());
            }

            if(dlg.Run() == IDOK)
            {
                pEncoder = new CEncoder(x, y);
                pEncoder->pEncoder = GetEncoder(refEncoders[dlg.GetSelection()]->GetId());
                this->AddFilter(pEncoder);
            }
        }
            break;
        case TRACKMENU_INSERTDECODER:
        {
            this->AddFilter(new CDecoder(x, y));
        }
            break;
        case 0: //canceled menu
        default:
            //do nothing...
            break;
    }
    this->OnChange();
}

//Private Functions
void CFiltergraph::CopyCodecValues(EDataType type, ADecoder *pDecoder, AEncoder *pEncoder)
{
    switch(type)
    {
        case DATATYPE_AUDIO:
        {
            AAudioDecoder *pADec;
            AAudioEncoder *pAEnc;

            pADec = (AAudioDecoder *)pDecoder;
            pAEnc = (AAudioEncoder *)pEncoder;

            pAEnc->SetNumberOfChannels(pADec->GetNumberOfChannels());
            pAEnc->SetSampleRate(pADec->GetSampleRate());
        }
            break;
    }
}

void CFiltergraph::CreateLink(AFilter *pSource, uint16 sourcePinIndex, AFilter *pDestination, uint16 destinationPinIndex)
{
    CLink *pLink;

    switch(pSource->GetType())
    {
        case FILTERTYPE_SOURCE:
        {
            CSource *pSrc;
            AStream *pSrcStream;

            pSrc = (CSource *)pSource;
            pSrcStream = pSrc->pInputCtx->GetStream(sourcePinIndex);

            switch(pDestination->GetType())
            {
                case FILTERTYPE_SINK: //create an encoder which only gets the information data but never gets to encode something
                {
                    AEncoder *pEncoder;

                    pEncoder = GetEncoder(pSrcStream->GetDecoder()->GetId());
                    if(!pEncoder)
                    {
                        this->MessageBoxA("Can't connect because this coded data can't be muxed", "Error", MB_ICONERROR);
                        return;
                    }

                    this->CopyCodecValues(pSrcStream->GetType(), pSrcStream->GetDecoder(), pEncoder);
                }
                    break;
                case FILTERTYPE_DECODER:
                {
                    CDecoder *pDecoderFilter;

                    pDecoderFilter = (CDecoder *)pDestination;

                    pDecoderFilter->pDecoder = pSrcStream->GetDecoder();
                }
                    break;
            }
        }
            break;
        case FILTERTYPE_ENCODER:
        {
            CEncoder *pSrc;

            pSrc = (CEncoder *)pSource;

            switch(pDestination->GetType())
            {
                case FILTERTYPE_SINK:
                {
                    CSink *pDest;

                    pDest = (CSink *)pDestination;

                    pDest->pOutputCtx->GetStream(destinationPinIndex)->SetEncoder(pSrc->pEncoder);
                }
                    break;
            }
        }
            break;
        case FILTERTYPE_DECODER:
        {
            CDecoder *pSrc;

            pSrc = (CDecoder *)pSource;

            switch(pDestination->GetType())
            {
                case FILTERTYPE_ENCODER:
                {
                    CEncoder *pDest;

                    pDest = (CEncoder *)pDestination;

                    this->CopyCodecValues(pSrc->pDecoder->GetDataType(), pSrc->pDecoder, pDest->pEncoder);
                }
                    break;
            }
        }
            break;
    }

    pLink = new CLink(pSource, sourcePinIndex, pDestination, destinationPinIndex);
    this->links.InsertTail(pLink);
    pSource->AddOutputLink(sourcePinIndex, pLink);
    pDestination->AddInputLink(destinationPinIndex, pLink);
}

void CFiltergraph::OnChange()
{
    repeat(this->filters.GetNoOfElements(), i)
    {
        if(this->filters[i]->IsOk() == FILTERSTATE_NOTOK)
        {
            g_mainWindow.toolbar.Enable(false);
            return;
        }
    }

    g_mainWindow.toolbar.Enable();
}

void CFiltergraph::RegisterWindowClass(const CModule &refModule) const
{
    refModule.RegisterWindowClass(NULL, NULL, NULL, NULL, CFILTERGRAPH_CLASSNAME);
}

//Public Functions
void CFiltergraph::AddFilter(AFilter *pFilter)
{
    uint32 index;

    index = this->filters.Push(pFilter);
    this->tooltip.AddTool(pFilter->ToString(), index, pFilter->GetRect());
    this->Invalidate();
}

void CFiltergraph::Create(const CWindow &refParentWindow)
{
    this->RegisterWindowClass(refParentWindow.GetModule());
    CWindow::Create(CFILTERGRAPH_CLASSNAME, "", WS_CHILD | WS_VISIBLE, 0, 0, 0, 0, 0, refParentWindow);
    this->tooltip.Create(this);
    this->tooltip.SetMaxTipWidth(1000);
}