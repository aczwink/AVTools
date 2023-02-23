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
#include "CSink.h"
//Namespaces
using namespace SJCWinLib;

//Forward Declarations
class CEditSinkDialog;

void CheckInput(CEditSinkDialog *pDlg);

class CDestinationDropDown : public CComboBox
{
private:
    //Message-callbacks
    void OnChangeSelection();
};

class CBrowseSaveButton : public CButton
{
private:
    //Message-callbacks
    void OnClick();
};

class CContainerFormatSelect : public CComboBox
{
private:
    //Message-callbacks
    void OnChangeSelection();
};

class CSaveSinkButton : public CButton
{
private:
    //Message-callbacks
    void OnClick();
};

class CEditSinkDialog : public CDialog
{
    friend void CheckInput(CEditSinkDialog *pDlg);
    friend class CDestinationDropDown;
    friend class CURLSaveEdit;
    friend class CBrowseSaveButton;
    friend class CSaveSinkButton;
private:
    //Variables
    CSink *pSink;
    //Controls
    CDestinationDropDown destination;
    CEdit url;
    CBrowseSaveButton urlBrowse;
    CContainerFormatSelect containerFormat;
    CStatic infoNumVideoStreams;
    CEdit numVideoStreams;
    CStatic infoNumAudioStreams;
    CEdit numAudioStreams;
    CStatic infoNumSubtitleStreams;
    CEdit numSubtitleStreams;
    CSaveSinkButton save;
    CCommonButtonClose close;
    //Message-callbacks
    void OnInit();
public:
    //Functions
    void Create(const CWindow *pParentWindow, CSink *pSink);
};