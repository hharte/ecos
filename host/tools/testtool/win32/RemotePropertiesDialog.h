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
#if !defined(AFX_REMOTEPROPERTIESDIALOG_H__6CE08442_1806_11D3_8567_BA4E779DE044__INCLUDED_)
#define AFX_REMOTEPROPERTIESDIALOG_H__6CE08442_1806_11D3_8567_BA4E779DE044__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000
// RemotePropertiesDialog.h : header file
//
#include "TestToolRes.h"		// main symbols
#include "eCosPropertyPage.h"

/////////////////////////////////////////////////////////////////////////////
// CRemotePropertiesDialog dialog

class CRemotePropertiesDialog : public CeCosDialog
{
// Construction
public:
	bool m_bFarmed;
	CRemotePropertiesDialog(CWnd* pParent = NULL);   // standard constructor

// Dialog Data
	//{{AFX_DATA(CRemotePropertiesDialog)
	enum { IDD = IDD_TT_PROPERTIES3 };
	CString	m_strResourceHost;
	UINT	m_nResourcePort;
	CString	m_strRemoteHost;
	UINT	m_nRemotePort;
	//}}AFX_DATA


// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CRemotePropertiesDialog)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	void SetButtons(bool bFromDataExchange=false);

	// Generated message map functions
	//{{AFX_MSG(CRemotePropertiesDialog)
	afx_msg void OnFarmed();
	afx_msg void OnExplicit();
	virtual BOOL OnInitDialog();
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_REMOTEPROPERTIESDIALOG_H__6CE08442_1806_11D3_8567_BA4E779DE044__INCLUDED_)
