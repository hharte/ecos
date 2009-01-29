// ####ECOSHOSTGPLCOPYRIGHTBEGIN####                                        
// -------------------------------------------                              
// This file is part of the eCos host tools.                                
// Copyright (C) 1998, 1999, 2000 Free Software Foundation, Inc.            
//
// This program is free software; you can redistribute it and/or modify     
// it under the terms of the GNU General Public License as published by     
// the Free Software Foundation; either version 2 or (at your option) any   
// later version.                                                           
//
// This program is distributed in the hope that it will be useful, but      
// WITHOUT ANY WARRANTY; without even the implied warranty of               
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU        
// General Public License for more details.                                 
//
// You should have received a copy of the GNU General Public License        
// along with this program; if not, write to the                            
// Free Software Foundation, Inc., 51 Franklin Street,                      
// Fifth Floor, Boston, MA  02110-1301, USA.                                
// -------------------------------------------                              
// ####ECOSHOSTGPLCOPYRIGHTEND####                                          
// outputwin.cpp :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/10/02
// Version:     $Id$
// Purpose:
// Description: Implementation file for ecOutputWindow
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

// ============================================================================
// declarations
// ============================================================================

// ----------------------------------------------------------------------------
// headers
// ----------------------------------------------------------------------------
#ifdef __GNUG__
    #pragma implementation "outputwin.h"
#endif

// Includes other headers for precompiled compilation
#include "ecpch.h"

#ifdef __BORLANDC__
    #pragma hdrstop
#endif

#include "configtool.h"
#include "outputwin.h"

/*
 * ecOutputWindow
 */

IMPLEMENT_CLASS(ecOutputWindow, wxTextCtrl)

BEGIN_EVENT_TABLE(ecOutputWindow, wxTextCtrl)
//    EVT_PAINT(ecOutputWindow::OnPaint)
    EVT_MOUSE_EVENTS(ecOutputWindow::OnMouseEvent)
    EVT_MENU(wxID_CLEAR, ecOutputWindow::OnClear)
    EVT_MENU(wxID_SELECTALL, ecOutputWindow::OnSelectAll)
    EVT_MENU(wxID_SAVE, ecOutputWindow::OnSave)
    EVT_UPDATE_UI(wxID_CLEAR, ecOutputWindow::OnUpdateClear)
    EVT_UPDATE_UI(wxID_SAVE, ecOutputWindow::OnUpdateSave)
END_EVENT_TABLE()

ecOutputWindow::ecOutputWindow(wxWindow* parent, wxWindowID id, const wxPoint& pt,
        const wxSize& sz, long style):
        wxTextCtrl(parent, id, wxEmptyString, pt, sz, style)
{
    if (!wxGetApp().GetSettings().GetWindowSettings().GetUseDefaults() &&
         wxGetApp().GetSettings().GetWindowSettings().GetFont(wxT("Output")).Ok())
    {
        SetFont(wxGetApp().GetSettings().GetWindowSettings().GetFont(wxT("Output")));
    }

    m_propertiesMenu = new wxMenu;

    m_propertiesMenu->Append(ecID_WHATS_THIS, _("&What's This?"));
    m_propertiesMenu->AppendSeparator();
    m_propertiesMenu->Append(wxID_COPY, _("&Copy"));
    m_propertiesMenu->Append(wxID_CLEAR, _("C&lear"));
    m_propertiesMenu->Append(wxID_SELECTALL, _("Select &All"));
    m_propertiesMenu->AppendSeparator();
    m_propertiesMenu->Append(wxID_SAVE, _("&Save..."));
}

ecOutputWindow::~ecOutputWindow()
{
    delete m_propertiesMenu;
}

void ecOutputWindow::OnMouseEvent(wxMouseEvent& event)
{
    if (event.RightDown())
    {
        PopupMenu(GetPropertiesMenu(), event.GetX(), event.GetY());
    }
    else
    {
        event.Skip();
    }
}

void ecOutputWindow::OnClear(wxCommandEvent& event)
{
    Clear();
}

void ecOutputWindow::OnSelectAll(wxCommandEvent& event)
{
    SetSelection(0, GetLastPosition());
}

void ecOutputWindow::OnSave(wxCommandEvent& event)
{
    wxFileDialog dialog(this, _("Save output as"),
        wxT(""), wxT("output.txt"), wxT("*.txt"), wxSAVE|wxOVERWRITE_PROMPT);
    if (dialog.ShowModal() == wxID_OK)
    {
        if (!SaveFile(dialog.GetPath()))
        {
            wxMessageBox(_("Error saving output file."), wxGetApp().GetSettings().GetAppName(),
                wxICON_EXCLAMATION|wxID_OK);
        }
    }
}

void ecOutputWindow::OnUpdateClear(wxUpdateUIEvent& event)
{
    event.Enable(!IsEmpty());
}

void ecOutputWindow::OnUpdateSave(wxUpdateUIEvent& event)
{
    event.Enable(!IsEmpty());
}
