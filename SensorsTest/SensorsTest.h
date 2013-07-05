// SensorsTest.h : main header file for the SENSORSTEST application
//

#if !defined(AFX_SENSORSTEST_H__11435EF8_0766_4A83_A339_D52EE6EEA47A__INCLUDED_)
#define AFX_SENSORSTEST_H__11435EF8_0766_4A83_A339_D52EE6EEA47A__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		// main symbols

/////////////////////////////////////////////////////////////////////////////
// CSensorsTestApp:
// See SensorsTest.cpp for the implementation of this class
//

class CSensorsTestApp : public CWinApp
{
public:
	CSensorsTestApp();

// Overrides
	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSensorsTestApp)
	public:
	virtual BOOL InitInstance();
	//}}AFX_VIRTUAL

// Implementation

	//{{AFX_MSG(CSensorsTestApp)
		// NOTE - the ClassWizard will add and remove member functions here.
		//    DO NOT EDIT what you see in these blocks of generated code !
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};


/////////////////////////////////////////////////////////////////////////////

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENSORSTEST_H__11435EF8_0766_4A83_A339_D52EE6EEA47A__INCLUDED_)
