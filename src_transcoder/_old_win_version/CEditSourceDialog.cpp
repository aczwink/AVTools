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
#include "CEditSourceDialog.h"
//Local
#include "CFiltergraph.h"
#include "globals.h"
//Namespaces
using namespace SJCLib;
//Definitions
#define DIALOG_WIDTH 200
#define DIALOG_HEIGHT 100

#define DIALOG_MARGIN 5
#define DIALOG_ROWSPACING 8
#define DIALOG_INPUTCTRLHEIGHT 14
#define DIALOG_CTRLWIDTH DIALOG_WIDTH - 2*DIALOG_MARGIN
#define DIALOG_BUTTONWIDTH 50

#define DIALOG_ROW1_Y DIALOG_MARGIN
#define DIALOG_ROW2_Y DIALOG_MARGIN + DIALOG_INPUTCTRLHEIGHT + DIALOG_ROWSPACING

//CSourceDropDown Message-callbacks
void CSourceDropDown::OnChangeSelection()
{
    CEditSourceDialog *pDlg;

    pDlg = (CEditSourceDialog *)this->GetParent();

    pDlg->url.Show(SW_SHOW);
    pDlg->urlBrowse.Show(SW_SHOW);
}

//CURLEdit Message-callbacks
void CURLEdit::OnChange()
{
    CEditSourceDialog *pParent;

    pParent = (CEditSourceDialog *)this->GetParent();
    if(FileExists(this->GetText()) && !IsDirectory(this->GetText()))
    {
        pParent->save.Enable();
        return;
    }
    pParent->save.Enable(false);
}

//CBrowseButton Message-callbacks
void CBrowseButton::OnClick()
{
    CEditSourceDialog *pParent;
    CCommonItemOpenDialog dlg;

    pParent = (CEditSourceDialog *)this->GetParent();
    dlg.Create(pParent);
    if(dlg.Run())
    {
        pParent->url.SetText(dlg.GetResult());
    }
}

//CInsertButton Message-callbacks
void CSaveButton::OnClick()
{
    CEditSourceDialog *pDlg;
    CInputContext *pInputCtx;
    CFileInputStream *pInput;

    pDlg = (CEditSourceDialog *)this->GetParent();
    pInputCtx = new CInputContext;
    pInput = new CFileInputStream(pDlg->url.GetText());

    if(pInputCtx->Init(pInput))
    {
        pDlg->pSource->SetInputContext(pInputCtx);
        pDlg->pSource->SetSourceName(GetFullFileName(pDlg->url.GetText()));

        pDlg->End(IDOK);
    }
    else
    {
        delete pInputCtx;
        delete pInput;
        pDlg->MessageBoxA("Couldn't find correct container format!", "ERROR", MB_ICONERROR);
    }
}

//Message-callbacks
void CEditSourceDialog::OnInit()
{
    CRect rcSource, rcURL, rcURLBrowse, rcInsert, rcClose;

    rcSource = this->DialogToScreenUnits(CRect().Init(DIALOG_MARGIN, DIALOG_ROW1_Y, DIALOG_CTRLWIDTH, DIALOG_INPUTCTRLHEIGHT));

    rcURL = this->DialogToScreenUnits(CRect().Init(DIALOG_MARGIN, DIALOG_ROW2_Y, DIALOG_CTRLWIDTH - DIALOG_BUTTONWIDTH - DIALOG_MARGIN, DIALOG_INPUTCTRLHEIGHT));
    rcURLBrowse = this->DialogToScreenUnits(CRect().Init(DIALOG_WIDTH - DIALOG_BUTTONWIDTH - DIALOG_MARGIN, DIALOG_ROW2_Y, DIALOG_BUTTONWIDTH, DIALOG_INPUTCTRLHEIGHT));

    rcInsert = this->DialogToScreenUnits(CRect().Init(DIALOG_MARGIN, DIALOG_HEIGHT - DIALOG_MARGIN - DIALOG_INPUTCTRLHEIGHT, DIALOG_BUTTONWIDTH, DIALOG_INPUTCTRLHEIGHT));
    rcClose = this->DialogToScreenUnits(CRect().Init(2*DIALOG_MARGIN + DIALOG_BUTTONWIDTH, DIALOG_HEIGHT - DIALOG_MARGIN - DIALOG_INPUTCTRLHEIGHT, DIALOG_BUTTONWIDTH, DIALOG_INPUTCTRLHEIGHT));

    this->source.CreateDropDownList(0, rcSource, *this);
    this->source.SetFont(g_defaultFont);
    this->source.SetCueBanner(L"Source");
    this->source.AddItem("File");

    this->url.Create("", ES_AUTOHSCROLL, WS_EX_CLIENTEDGE, rcURL, *this);
    this->url.SetFont(g_defaultFont);
    this->url.SetCueBanner(L"URL");
    this->url.Show(SW_HIDE);

    this->urlBrowse.Create("Browse...", 0, rcURLBrowse, *this);
    this->urlBrowse.SetFont(g_defaultFont);
    this->urlBrowse.Show(SW_HIDE);

    this->save.Create("Save", 0, rcInsert, *this);
    this->save.SetFont(g_defaultFont);
    this->save.Enable(false);

    this->close.Create("Close", 0, rcClose, *this);
    this->close.SetFont(g_defaultFont);
}

//Public Functions
void CEditSourceDialog::Create(const CWindow *pParentWindow, CSource *pSource)
{
    this->pSource = pSource;
    CDialog::Create("Edit Source", DS_MODALFRAME | DS_CENTER | WS_POPUP | WS_CAPTION | WS_SYSMENU, 0, 0, DIALOG_WIDTH, DIALOG_HEIGHT, pParentWindow, g_defaultFont);
}