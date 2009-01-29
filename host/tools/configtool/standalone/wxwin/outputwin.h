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
// outputwin.h :
//
//===========================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   julians
// Contact(s):  julians
// Date:        2000/10/2
// Version:     $Id$
// Purpose:
// Description: Header file for ecOutputWindow
// Requires:
// Provides:
// See also:
// Known bugs:
// Usage:
//
//####DESCRIPTIONEND####
//
//===========================================================================

#ifndef _ECOS_OUTPUTWIN_H_
#define _ECOS_OUTPUTWIN_H_

#ifdef __GNUG__
#pragma interface "outputwin.h"
#endif

#include "wx/textctrl.h"

class ecOutputWindow : public wxTextCtrl
{
public:
// Ctor(s)
    ecOutputWindow(wxWindow* parent, wxWindowID id, const wxPoint& pos, const wxSize& size, long style);
    ~ecOutputWindow();

//// Event handlers

    void OnMouseEvent(wxMouseEvent& event);
    void OnClear(wxCommandEvent& event);
    void OnSelectAll(wxCommandEvent& event);
    void OnSave(wxCommandEvent& event);
    void OnUpdateClear(wxUpdateUIEvent& event);
    void OnUpdateSave(wxUpdateUIEvent& event);

//// Operations

//// Accessors
    wxMenu* GetPropertiesMenu() const { return m_propertiesMenu; }

protected:
    wxMenu*     m_propertiesMenu;

private:
    DECLARE_EVENT_TABLE()
    DECLARE_CLASS(ecOutputWindow)
};


#endif
        // _ECOS_OUTPUTWIN_H_
