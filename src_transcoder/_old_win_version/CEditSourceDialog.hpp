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
#include "CSource.h"
//Namespaces
using namespace SJCWinLib;

class CSourceDropDown : public CComboBox
{
private:
    //Message-callbacks
    void OnChangeSelection();
};

class CURLEdit : public CEdit
{
private:
    //Message-callbacks
    void OnChange();
};

class CBrowseButton : public CButton
{
private:
    //Message-callbacks
    void OnClick();
};

class CSaveButton : public CButton
{
private:
    //Message-callbacks
    void OnClick();
};

class CEditSourceDialog : public CDialog
{
    friend class CSourceDropDown;
    friend class CURLEdit;
    friend class CBrowseButton;
    friend class CSaveButton;
private:
    //Variables
    CSource *pSource;
    //Controls
    CSourceDropDown source;
    CURLEdit url;
    CBrowseButton urlBrowse;
    CSaveButton save;
    CCommonButtonClose close;
    //Message-callbacks
    void OnInit();
public:
    //Functions
    void Create(const CWindow *pParentWindow, CSource *pSource);
};