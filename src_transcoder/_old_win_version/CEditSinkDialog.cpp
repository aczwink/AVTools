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
#include "CEditSinkDialog.h"
//Local
#include "globals.h"
//Definitions
#define DIALOG_WIDTH 200
#define DIALOG_HEIGHT 200

#define DIALOG_MARGIN 5
#define DIALOG_ROWSPACING 8
#define DIALOG_INNERROWSPACE 2
#define DIALOG_TEXTHEIGHT 8
#define DIALOG_INPUTCTRLHEIGHT 14
#define DIALOG_CTRLWIDTH DIALOG_WIDTH - 2*DIALOG_MARGIN
#define DIALOG_BUTTONWIDTH 50

#define DIALOG_ROW1_Y DIALOG_MARGIN
#define DIALOG_ROW2_Y DIALOG_MARGIN + DIALOG_INPUTCTRLHEIGHT + DIALOG_ROWSPACING
#define DIALOG_ROW3_Y DIALOG_ROW2_Y + DIALOG_INPUTCTRLHEIGHT + DIALOG_ROWSPACING
#define DIALOG_ROW4INFO_Y DIALOG_ROW3_Y + DIALOG_INPUTCTRLHEIGHT + DIALOG_ROWSPACING
#define DIALOG_ROW4_Y DIALOG_ROW4INFO_Y + DIALOG_TEXTHEIGHT + DIALOG_INNERROWSPACE
#define DIALOG_ROW5INFO_Y DIALOG_ROW4_Y + DIALOG_INPUTCTRLHEIGHT + DIALOG_ROWSPACING
#define DIALOG_ROW5_Y DIALOG_ROW5INFO_Y + DIALOG_TEXTHEIGHT + DIALOG_INNERROWSPACE
#define DIALOG_ROW6INFO_Y DIALOG_ROW5_Y + DIALOG_INPUTCTRLHEIGHT + DIALOG_ROWSPACING
#define DIALOG_ROW6_Y DIALOG_ROW6INFO_Y + DIALOG_TEXTHEIGHT + DIALOG_INNERROWSPACE

void CheckInput(CEditSinkDialog *pDlg)
{
    const IFormat *pFormat;
    bool ok;

    ok = true;

    if(pDlg->containerFormat.GetSelection() == CB_ERR)
    {
        pDlg->numVideoStreams.Enable(false);
        pDlg->numAudioStreams.Enable(false);
        pDlg->numSubtitleStreams.Enable(false);
        ok = false;
    }
    else
    {
        pFormat = GetSupportedContainerFormats()[(uint32)pDlg->containerFormat.GetItemData(pDlg->containerFormat.GetSelection())];

        pDlg->infoNumAudioStreams.Show(SW_SHOW);
        pDlg->infoNumSubtitleStreams.Show(SW_SHOW);
        pDlg->infoNumVideoStreams.Show(SW_SHOW);
        pDlg->numAudioStreams.Show(SW_SHOW);
        pDlg->numSubtitleStreams.Show(SW_SHOW);
        pDlg->numVideoStreams.Show(SW_SHOW);

        if(pFormat->GetMaxNoOfStreams(DATATYPE_VIDEO) == 0)
        {
            pDlg->numVideoStreams.SetText("0");
            pDlg->numVideoStreams.Enable(false);
        }

        if(pFormat->GetMaxNoOfStreams(DATATYPE_AUDIO) == 0)
        {
            pDlg->numAudioStreams.SetText("0");
            pDlg->numAudioStreams.Enable(false);
        }

        if(pFormat->GetMinNoOfStreams(DATATYPE_AUDIO) == pFormat->GetMaxNoOfStreams(DATATYPE_AUDIO))
        {
            pDlg->numAudioStreams.SetText(CString(pFormat->GetMinNoOfStreams(DATATYPE_AUDIO)));
            pDlg->numAudioStreams.Enable(false);
        }

        if(pFormat->GetMaxNoOfStreams(DATATYPE_SUBTITLE) == 0)
        {
            pDlg->numSubtitleStreams.SetText("0");
            pDlg->numSubtitleStreams.Enable(false);
        }
    }

    if(ok)
        pDlg->save.Enable();
    else
        pDlg->save.Enable(false);
}

//CDestinationDropDown Message-callbacks
void CDestinationDropDown::OnChangeSelection()
{
    CEditSinkDialog *pDlg;

    pDlg = (CEditSinkDialog *)this->GetParent();

    pDlg->url.Show(SW_SHOW);
    pDlg->urlBrowse.Show(SW_SHOW);
    pDlg->containerFormat.Show(SW_SHOW);
}

//CBrowseSaveButton Message-callbacks
void CBrowseSaveButton::OnClick()
{
    CEditSinkDialog *pParent;
    CCommonItemSaveDialog dlg;

    pParent = (CEditSinkDialog *)this->GetParent();
    dlg.Create(pParent);
    if(dlg.Run())
    {
        pParent->url.SetText(dlg.GetResult());
    }
}

//CContainerFormatSelect Message-callbacks
void CContainerFormatSelect::OnChangeSelection()
{
    CheckInput((CEditSinkDialog *)this->GetParent());
}

//CSaveSinkButton Message-callbacks
void CSaveSinkButton::OnClick()
{
    CEditSinkDialog *pDlg;
    const IFormat *pFormat;
    CFileOutputStream *pOutput;
    COutputContext *pOutputCtx;

    pDlg = (CEditSinkDialog *)this->GetParent();
    pFormat = GetSupportedContainerFormats()[(uint32)pDlg->containerFormat.GetItemData(pDlg->containerFormat.GetSelection())];
    pOutput = new CFileOutputStream;

    if(!pOutput->Open(pDlg->url.GetText()))
    {
        pDlg->MessageBoxA("File couldn't be created", "Error", MB_ICONERROR);
        delete pOutput;
        return;
    }

    pOutputCtx = new COutputContext;
    pOutputCtx->Init(pOutput, pFormat->GetContainerFormat());

    repeat(pDlg->numAudioStreams.GetText().ToUInt32(), i)
    {
        pOutputCtx->AddStream(new CAudioStream);
    }
    repeat(pDlg->numSubtitleStreams.GetText().ToUInt32(), i)
    {
        pOutputCtx->AddStream(new CSubtitleStream);
    }
    repeat(pDlg->numVideoStreams.GetText().ToUInt32(), i)
    {
        pOutputCtx->AddStream(new CVideoStream);
    }

    pDlg->pSink->SetNoOfInputPins(pDlg->numAudioStreams.GetText().ToUInt32() + pDlg->numSubtitleStreams.GetText().ToUInt32() + pDlg->numVideoStreams.GetText().ToUInt32());
    pDlg->pSink->SetOutputContext(pOutputCtx);
    pDlg->pSink->SetDestinationName(GetFullFileName(pDlg->url.GetText()));

    pDlg->End(IDOK);
}

//Message-callbacks
void CEditSinkDialog::OnInit()
{
    uint32 index;
    CRect rcDestination, rcURL, rcURLBrowse, rcContainerFormat, rcInfoNumVideoStreams, rcNumVideoStreams, rcInfoNumAudioStreams, rcNumAudioStreams, rcInfoNumSubtitleStreams, rcNumSubtitleStreams, rcSave, rcClose;

    const CArray<const IFormat *> &refFormats = GetSupportedContainerFormats();

    rcDestination = this->DialogToScreenUnits(CRect().Init(DIALOG_MARGIN, DIALOG_ROW1_Y, DIALOG_CTRLWIDTH, DIALOG_INPUTCTRLHEIGHT));

    rcURL = this->DialogToScreenUnits(CRect().Init(DIALOG_MARGIN, DIALOG_ROW2_Y, DIALOG_CTRLWIDTH - DIALOG_BUTTONWIDTH - DIALOG_MARGIN, DIALOG_INPUTCTRLHEIGHT));
    rcURLBrowse = this->DialogToScreenUnits(CRect().Init(DIALOG_WIDTH - DIALOG_BUTTONWIDTH - DIALOG_MARGIN, DIALOG_ROW2_Y, DIALOG_BUTTONWIDTH, DIALOG_INPUTCTRLHEIGHT));

    rcContainerFormat = this->DialogToScreenUnits(CRect().Init(DIALOG_MARGIN, DIALOG_ROW3_Y, DIALOG_CTRLWIDTH, DIALOG_INPUTCTRLHEIGHT));

    rcInfoNumVideoStreams = this->DialogToScreenUnits(CRect().Init(DIALOG_MARGIN, DIALOG_ROW4INFO_Y, DIALOG_CTRLWIDTH, DIALOG_TEXTHEIGHT));
    rcNumVideoStreams = this->DialogToScreenUnits(CRect().Init(DIALOG_MARGIN, DIALOG_ROW4_Y, DIALOG_CTRLWIDTH, DIALOG_INPUTCTRLHEIGHT));
    rcInfoNumAudioStreams = this->DialogToScreenUnits(CRect().Init(DIALOG_MARGIN, DIALOG_ROW5INFO_Y, DIALOG_CTRLWIDTH, DIALOG_TEXTHEIGHT));
    rcNumAudioStreams = this->DialogToScreenUnits(CRect().Init(DIALOG_MARGIN, DIALOG_ROW5_Y, DIALOG_CTRLWIDTH, DIALOG_INPUTCTRLHEIGHT));
    rcInfoNumSubtitleStreams = this->DialogToScreenUnits(CRect().Init(DIALOG_MARGIN, DIALOG_ROW6INFO_Y, DIALOG_CTRLWIDTH, DIALOG_TEXTHEIGHT));
    rcNumSubtitleStreams = this->DialogToScreenUnits(CRect().Init(DIALOG_MARGIN, DIALOG_ROW6_Y, DIALOG_CTRLWIDTH, DIALOG_INPUTCTRLHEIGHT));

    rcSave = this->DialogToScreenUnits(CRect().Init(DIALOG_MARGIN, DIALOG_HEIGHT - DIALOG_MARGIN - DIALOG_INPUTCTRLHEIGHT, DIALOG_BUTTONWIDTH, DIALOG_INPUTCTRLHEIGHT));
    rcClose = this->DialogToScreenUnits(CRect().Init(2*DIALOG_MARGIN + DIALOG_BUTTONWIDTH, DIALOG_HEIGHT - DIALOG_MARGIN - DIALOG_INPUTCTRLHEIGHT, DIALOG_BUTTONWIDTH, DIALOG_INPUTCTRLHEIGHT));

    this->destination.CreateDropDownList(0, rcDestination, *this);
    this->destination.SetFont(g_defaultFont);
    this->destination.SetCueBanner(L"Destination");
    this->destination.AddItem("File");

    this->url.Create("", ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, rcURL, *this);
    this->url.SetFont(g_defaultFont);
    this->url.SetCueBanner(L"URL");
    this->url.Show(SW_HIDE);

    this->urlBrowse.Create("Browse...", 0, rcURLBrowse, *this);
    this->urlBrowse.SetFont(g_defaultFont);
    this->urlBrowse.Show(SW_HIDE);

    this->containerFormat.CreateDropDownList(CBS_SORT, rcContainerFormat, *this);
    this->containerFormat.SetFont(g_defaultFont);
    this->containerFormat.SetCueBanner(L"Container format");
    this->containerFormat.Show(SW_HIDE);

    repeat(refFormats.GetNoOfElements(), i)
    {
        if(refFormats[i]->GetMuxer())
        {
            index = this->containerFormat.AddItem(refFormats[i]->GetName());
            this->containerFormat.SetItemData(index, i);
        }
    }

    this->infoNumVideoStreams.Create("Number of Video Streams", 0, 0, rcInfoNumVideoStreams, *this);
    this->infoNumVideoStreams.SetFont(g_defaultFont);
    this->infoNumVideoStreams.Show(SW_HIDE);

    this->numVideoStreams.Create("", ES_NUMBER, WS_EX_CLIENTEDGE, rcNumVideoStreams, *this);
    this->numVideoStreams.SetFont(g_defaultFont);
    this->numVideoStreams.Enable(false);
    this->numVideoStreams.Show(SW_HIDE);

    this->infoNumAudioStreams.Create("Number of Audio Streams", 0, 0, rcInfoNumAudioStreams, *this);
    this->infoNumAudioStreams.SetFont(g_defaultFont);
    this->infoNumAudioStreams.Show(SW_HIDE);

    this->numAudioStreams.Create("", ES_NUMBER, WS_EX_CLIENTEDGE, rcNumAudioStreams, *this);
    this->numAudioStreams.SetFont(g_defaultFont);
    this->numAudioStreams.Enable(false);
    this->numAudioStreams.Show(SW_HIDE);

    this->infoNumSubtitleStreams.Create("Number of Subtitle Streams", 0, 0, rcInfoNumSubtitleStreams, *this);
    this->infoNumSubtitleStreams.SetFont(g_defaultFont);
    this->infoNumSubtitleStreams.Show(SW_HIDE);

    this->numSubtitleStreams.Create("", ES_NUMBER, WS_EX_CLIENTEDGE, rcNumSubtitleStreams, *this);
    this->numSubtitleStreams.SetFont(g_defaultFont);
    this->numSubtitleStreams.Enable(false);
    this->numSubtitleStreams.Show(SW_HIDE);

    this->save.Create("Save", 0, rcSave, *this);
    this->save.SetFont(g_defaultFont);
    this->save.Enable(false);

    this->close.Create("Close", 0, rcClose, *this);
    this->close.SetFont(g_defaultFont);
}

//Public Functions
void CEditSinkDialog::Create(const CWindow *pParentWindow, CSink *pSink)
{
    this->pSink = pSink;
    CDialog::Create("Edit Sink", DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU, 0, 0, DIALOG_WIDTH, DIALOG_HEIGHT, pParentWindow, g_defaultFont);
}