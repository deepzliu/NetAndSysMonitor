// SensorsTestDlg.h : header file
//

#if !defined(AFX_SENSORSTESTDLG_H__DA7660FD_DBDF_4026_99D3_FF1C2CD40582__INCLUDED_)
#define AFX_SENSORSTESTDLG_H__DA7660FD_DBDF_4026_99D3_FF1C2CD40582__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

/////////////////////////////////////////////////////////////////////////////
// CSensorsTestDlg dialog
#import "msxml3.dll" named_guids
//#include <msxml2.h>
using namespace MSXML2;

class CSensorsTestDlg : public CDialog
{
// Construction
public:
	CSensorsTestDlg(CWnd* pParent = NULL);	// standard constructor
    BOOL SaveTreeToXml(CTreeCtrl *pTree,LPSTR xmlFileName);
	void TreeNodeToXMLNode(CTreeCtrl *pTree,HTREEITEM parentItem,MSXML2::IXMLDOMDocumentPtr pXmlDom,MSXML2::IXMLDOMElementPtr pXmlNode);
    BOOL ReadSetting();
	BOOL CheckSensor();
	BOOL CheckSensor2();
    BOOL AutoStart;
	BOOL AutoSave;
    CString savefile;
	CString modulefile;
	BOOL autocompare;

	int mintemp;		//温度最小范围
	int maxtemp;
	int minfanspeed1;	//风扇转速范围1
	int maxfanspeed1;
	int minfanspeed2;	//风扇转速范围2
	int maxfanspeed2;
	int minfanspeed3;	//风扇转速范围3
	int maxfanspeed3;
	// Dialog Data
	//{{AFX_DATA(CSensorsTestDlg)
	enum { IDD = IDD_SENSORSTEST_DIALOG };
	CTreeCtrl	m_DeviceTree;
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CSensorsTestDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support
	//}}AFX_VIRTUAL
    BOOL SaveTreeToXML(void);
	BOOL SaveTreeToXMLforAutoCompare();
// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	//{{AFX_MSG(CSensorsTestDlg)
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	afx_msg void OnRefresh();
	afx_msg void OnSavexml();
	afx_msg void OnCompare();
	afx_msg void OnTimer(UINT nIDEvent);
	virtual void OnOK();
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
public:
	afx_msg void OnBnClickedButton2();
	//friend bool CompareNodeValue(CString value, CString value1);
};

//{{AFX_INSERT_LOCATION}}
// Microsoft Visual C++ will insert additional declarations immediately before the previous line.

#endif // !defined(AFX_SENSORSTESTDLG_H__DA7660FD_DBDF_4026_99D3_FF1C2CD40582__INCLUDED_)
