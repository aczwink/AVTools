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
#include "CMainWindow.h"
//Local
#include "globals.h"
#include "resource.h"
#include "Transcode.h"

//CMainToolBar Message-callbacks
bool CMainToolBar::OnClick(uint16 commandId)
{
    switch(commandId)
    {
        case IDT_RUN:
            Transcode(g_mainWindow.pGraph->filters);
            break;
    }
    return false;
}

//Message-callbacks
bool CMainWindow::OnCreate()
{
    g_defaultFont.LoadSystemFont(DEFAULT_GUI_FONT);

    this->toolbar.Create(TBSTYLE_FLAT | TBSTYLE_LIST, *this);
    this->toolbar.Enable(false);
    this->toolbar.AddButton("Run", 0, IDT_RUN, TBSTATE_ENABLED, 0);

    this->tabs.Create(*this);
    this->tabs.SetFont(g_defaultFont);
    this->tabs.AddTab("Filtergraph", this->pGraph.Cast<CWindow>());
    this->pGraph->Create(this->tabs);

    return true;
}

void CMainWindow::OnDestroy()
{
    PostQuitMessage(EXIT_SUCCESS);
}

void CMainWindow::OnSize(WPARAM resizingType, uint32 newWidth, uint32 newHeight)
{
    CRect rcClient, rcToolBar, rcTab, rcGraph;

    this->toolbar.AutoSize();
    this->toolbar.GetClientRect(rcToolBar);

    this->GetClientRect(rcClient);

    rcTab = CRect(5, rcToolBar.bottom + 5, rcClient.right-5, rcClient.bottom-5);
    this->tabs.SetPos(rcTab, SWP_NOZORDER);

    this->tabs.GetChildWindowArea(rcGraph);
    rcGraph.left += 5;
    rcGraph.top += rcToolBar.bottom + 5;
    this->pGraph->SetPos(rcGraph, SWP_NOZORDER);
}