// SensorsTestDlg.cpp : implementation file
//

#include "stdafx.h"
#include "SensorsTest.h"
#include "SensorsTestDlg.h"
#include "Sensors.h"
#include "SmartReader.h"
#include "./cpuinfolib/CPUBasicInfo.h"
#include "cpuinfolib/libcpuinfo.h"
#include "cpuinfolib/libcpuinfo_funclist.h"
#include "lpc/LPCIO.h"

#include <comutil.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

/////////////////////////////////////////////////////////////////////////////
// CAboutDlg dialog used for App About
//#import "msxml3.dll"

//using namespace MSXML2;

int GetCheckedLength(MSXML2::IXMLDOMNodeListPtr pDeviceInfoNodes){
	int len = 0;
	if(pDeviceInfoNodes == NULL){
		return len;
	}

	for(int j=0;j<pDeviceInfoNodes->Getlength();j++)
	{
        MSXML2::IXMLDOMNodePtr pDevicinfoNode = pDeviceInfoNodes->nextNode();
		_variant_t sattrib  = pDevicinfoNode->Getattributes()->Getitem(0)->GetnodeTypedValue();
		if(CString(sattrib.pbVal)=="1")
		{
			len++;
		}
	}
	return len;
}

bool CompareNodeValue(CString value, CString value1, void *pUser)
{
	bool BFailed = false;
	CSensorsTestDlg* dlg = (CSensorsTestDlg*)pUser;

	CString strTemp = value;
	CString strComp = value1;
	CString strTempLable = strTemp.Left(strTemp.Find(':'));
	CString strCompLable = strComp.Left(strComp.Find(':'));
	int HardDiskFlag = strComp.Find("HardDisk");

	if(HardDiskFlag >= 0 || strTempLable==strCompLable)
	{
		CString strValue = strComp.Mid(strTemp.Find(':')+1);
		if(strValue.Find("C")!=-1)
		{
			strValue = strValue.Left(strValue.Find("C"));
			strValue.TrimLeft();
			strValue.TrimRight();
			float value = atof(strValue.GetBuffer(0));
			if(value>=dlg->mintemp && value<=dlg->maxtemp)
			{
				BFailed = true;
			}
			else
			{
				 
			}
		}
		if(strValue.Find("RPM")!=-1)
		{
            strValue = strValue.Left(strValue.Find("RPM"));
			strValue.TrimLeft();
			strValue.TrimRight();
			float value = atof(strValue.GetBuffer(0));
			if(value>=dlg->minfanspeed1 && value<=dlg->maxfanspeed1)
			{
				BFailed = true;
			}
			else
			{
				
			}
		}
	}
	return BFailed;
}

int GetCheckedStat(MSXML2::IXMLDOMNodeListPtr pDeviceInfoNodes, MSXML2::IXMLDOMNodeListPtr pDeviceInfoNodes1)
{
	if(pDeviceInfoNodes == NULL || pDeviceInfoNodes1 == NULL){
		return -1;
	}

	char a[20];
	int modulecount = 0, localcount = 0;
	memset(a, 1, 20);
	for(int j=0; j<pDeviceInfoNodes->Getlength(); j++)
	{
		MSXML2::IXMLDOMNodePtr pDevicinfoNode = pDeviceInfoNodes->nextNode();
			_variant_t sattrib  = pDevicinfoNode->Getattributes()->Getitem(0)->GetnodeTypedValue();
			if(CString(sattrib.pbVal)=="1")
			{							 
				modulecount ++;
			}else{
				a[j] = 0;
			}
	}
	for(int j=0; j<pDeviceInfoNodes1->Getlength(); j++)
	{
		MSXML2::IXMLDOMNodePtr pDevicinfoNode1 = pDeviceInfoNodes1->nextNode();
			_variant_t sattrib  = pDevicinfoNode1->Getattributes()->Getitem(0)->GetnodeTypedValue();
			if(a[j] == 1 && CString(sattrib.pbVal)=="1")
			{
				localcount ++;
			}
	}

	if(modulecount == localcount)
		return 1;
	else
		return 0;
}


class CAboutDlg : public CDialog
{
public:
	CAboutDlg();

// Dialog Data
	//{{AFX_DATA(CAboutDlg)
	enum { IDD = IDD_ABOUTBOX };
	//}}AFX_DATA

	// ClassWizard generated virtual function overrides
	//{{AFX_VIRTUAL(CAboutDlg)
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV support
	//}}AFX_VIRTUAL

// Implementation
protected:
	//{{AFX_MSG(CAboutDlg)
	//}}AFX_MSG
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialog(CAboutDlg::IDD)
{
	//{{AFX_DATA_INIT(CAboutDlg)
	//}}AFX_DATA_INIT
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CAboutDlg)
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialog)
	//{{AFX_MSG_MAP(CAboutDlg)
		// No message handlers
	//}}AFX_MSG_MAP
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSensorsTestDlg dialog

CSensorsTestDlg::CSensorsTestDlg(CWnd* pParent /*=NULL*/)
	: CDialog(CSensorsTestDlg::IDD, pParent)
{
	//{{AFX_DATA_INIT(CSensorsTestDlg)
		// NOTE: the ClassWizard will add member initialization here
	//}}AFX_DATA_INIT
	// Note that LoadIcon does not require a subsequent DestroyIcon in Win32
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CSensorsTestDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	//{{AFX_DATA_MAP(CSensorsTestDlg)
	DDX_Control(pDX, IDC_TREE1, m_DeviceTree);
	//}}AFX_DATA_MAP
}

BEGIN_MESSAGE_MAP(CSensorsTestDlg, CDialog)
	//{{AFX_MSG_MAP(CSensorsTestDlg)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_REFRESH, OnRefresh)
	ON_WM_TIMER()
	//}}AFX_MSG_MAP
	ON_BN_CLICKED(IDC_BUTTON2, &CSensorsTestDlg::OnBnClickedButton2)
END_MESSAGE_MAP()

/////////////////////////////////////////////////////////////////////////////
// CSensorsTestDlg message handlers

BOOL CSensorsTestDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

	// Add "About..." menu item to system menu.

	// IDM_ABOUTBOX must be in the system command range.
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		CString strAboutMenu;
		strAboutMenu.LoadString(IDS_ABOUTBOX);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here
	GetDlgItem(IDOK)->SetWindowText("生成模板");
	GetDlgItem(IDCANCEL)->SetWindowText("退出");
	GetDlgItem(IDC_REFRESH)->SetWindowText(_T("刷新"));
	GetDlgItem(IDC_BUTTON2)->SetWindowText(_T("比对"));
	//AfxMessageBox("Debug Window");

	if(!InitCPUSenor())
	{
	  MessageBox("CPU Sensor Initial failed!");
	  return FALSE;
	}
	

	::CoInitialize(NULL); 
	ReadSetting();
	SetTimer(0x1100,500,NULL);
	SaveTreeToXMLforAutoCompare();
	return TRUE;  // return TRUE  unless you set the focus to a control
}

void CSensorsTestDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CSensorsTestDlg::OnPaint() 
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, (WPARAM) dc.GetSafeHdc(), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialog::OnPaint();
	}
}

// The system calls this to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CSensorsTestDlg::OnQueryDragIcon()
{
	return (HCURSOR) m_hIcon;
}

void CSensorsTestDlg::OnRefresh() 
{
	// TODO: Add your control notification handler code here
	m_DeviceTree.DeleteAllItems();
	HTREEITEM hItem1,hItem2,hItem3;
	CSmartReader  m_oSmartReader;
	if(!m_oSmartReader.ReadSMARTValuesForAllDrives())
	{
       ;
	}
	hItem1 = m_DeviceTree.InsertItem("Sensor",TVI_ROOT);
        DOUBLE tempreture[20];
	    CCPUBasicInfo m_cpuinfo = Cpuinfo_getCpuBasicInfo();
		int i=0;
		CString info;

			
	     for(int i=0;i<m_cpuinfo.m_num_cores;i++)
		{
		    GetCPUTemp(i,tempreture[i]);
		}
		//CString info;
        hItem2 = m_DeviceTree.InsertItem("CPU core",hItem1);
		for( i=0;i<m_cpuinfo.m_num_cores;i++)
		{
          	info.Format("CPU# %d:  %.1f C",i,tempreture[i]);
			hItem3 = m_DeviceTree.InsertItem(info,hItem2);
			m_DeviceTree.SetCheck(hItem3);
		}
        m_DeviceTree.Expand(hItem2,TVE_EXPAND);
		
		ST_SMART_INFO* pInfo; //;
        hItem2 = m_DeviceTree.InsertItem("HardDisk",hItem1); //硬盘温度
		for(i=0;i<m_oSmartReader.m_ucDrivesWithInfo;i++)
		{
		   BYTE tempdisk=0;
		   pInfo = m_oSmartReader.GetSMARTValue(i,0xc2);
		   if(pInfo ==NULL)
		   {
             pInfo = m_oSmartReader.GetSMARTValue(i,0xe7);
		   }
		   if(pInfo!=NULL)
		   {  
			  if(pInfo->m_ucAttribIndex>pInfo->m_ucWorst)
			  {
                 tempdisk = pInfo->bRawValue[0];
			  }
			  else
			  {
				 tempdisk = pInfo->m_ucValue;
			  }
			  //硬盘#%s温度
           	  info.Format("HardDisk%sTemp: %dC",m_oSmartReader.m_stDrivesInfo[i].m_stInfo.sSerialNumber,tempdisk);
			 hItem3 = m_DeviceTree.InsertItem(info,hItem2);
			 m_DeviceTree.SetCheck(hItem3);
		   }
		 
		}
		 m_DeviceTree.Expand(hItem2,TVE_EXPAND);
		 
		LPCIO m_lpcio;
		CString lpcdesription;
		
		if(m_lpcio.pSuperIO)
		{
			//LCP传感器, CPU温度
		   //lpcdesription.Format("CPU Tempreture");
           //hItem2 = m_DeviceTree.InsertItem(lpcdesription,hItem1);
	       m_lpcio.pSuperIO->Update();
		   int tempsensorcount=0;
		   int fanspeedsensorcount=0;
		   /*
		   for(int j=0;j<m_lpcio.pSuperIO->m_sensorcount;j++)
		   {
			   if(m_lpcio.pSuperIO->m_sensorvalue[j].m_type == TVOLTAGE)
				   continue;
			   CString str;

			   if(m_lpcio.pSuperIO->m_sensorvalue[j].m_type == TTEMPERATURE)
			   {
				   if(m_lpcio.pSuperIO->m_sensorvalue[j].values > 1 &&
					   m_lpcio.pSuperIO->m_sensorvalue[j].values < 126){
					   //温度
					   str.Format("Tempreture#%d: %.1f C",++tempsensorcount,m_lpcio.pSuperIO->m_sensorvalue[j].values);//Liudonghua
					   hItem3 = m_DeviceTree.InsertItem(str,hItem2);
				   }
			   }

		       if(m_lpcio.pSuperIO->m_sensorvalue[j].m_type == FANSPEED){
				   continue;
                   //风扇转速
				   str.Format("FanSpeed#%d: %.1f RPM",++fanspeedsensorcount,m_lpcio.pSuperIO->m_sensorvalue[j].values);
					hItem3 = m_DeviceTree.InsertItem(str,hItem2);
			   }
			   m_DeviceTree.SetCheck(hItem3);
		   }
		   	m_DeviceTree.Expand(hItem2,TVE_EXPAND);
			*/
			//LCP传感器， 风扇转速
		   lpcdesription.Format("LPCIO :%s",m_lpcio.pSuperIO->GetChipName().c_str());
           hItem2 = m_DeviceTree.InsertItem(lpcdesription,hItem1);
	       m_lpcio.pSuperIO->Update();
		   //int tempsensorcount=0;
		   //int fanspeedsensorcount=0;
		   for(int j=0;j<m_lpcio.pSuperIO->m_sensorcount;j++)
		   {
			   if(m_lpcio.pSuperIO->m_sensorvalue[j].m_type == TVOLTAGE)
				   continue;
			   CString str;

			   if(m_lpcio.pSuperIO->m_sensorvalue[j].m_type == TTEMPERATURE)
			   {
				   continue;
				   //温度
				   str.Format("Tempreture#%d: %.1f C",++tempsensorcount,m_lpcio.pSuperIO->m_sensorvalue[j].values,m_lpcio.pSuperIO->m_sensorvalue[j].reg);//Liudonghua
				   hItem3 = m_DeviceTree.InsertItem(str,hItem2);
			   }

		       if(m_lpcio.pSuperIO->m_sensorvalue[j].m_type == FANSPEED){
				   if(m_lpcio.pSuperIO->m_sensorvalue[j].values > 100){
					   //风扇转速
					    str.Format("FanSpeed#%d: %.1f RPM",++fanspeedsensorcount,m_lpcio.pSuperIO->m_sensorvalue[j].values);
						hItem3 = m_DeviceTree.InsertItem(str,hItem2);
				   }
			   }
			   m_DeviceTree.SetCheck(hItem3);
		   }
		   	m_DeviceTree.Expand(hItem2,TVE_EXPAND);
		}
		m_DeviceTree.Expand(hItem1,TVE_EXPAND);
}

BOOL CSensorsTestDlg::SaveTreeToXMLforAutoCompare()
{
	if(!autocompare) return false;
	CString filename = savefile;

	MSXML2::IXMLDOMDocumentPtr pDoc;
	MSXML2::IXMLDOMElementPtr xmlRoot;
	HRESULT hr = pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument30));
	if(!SUCCEEDED(hr))
	{
		MessageBox("Error");
		return FALSE;
	}
	if(SaveTreeToXml(&m_DeviceTree,(LPTSTR)(LPCTSTR)filename))
	{
	  
	}
	else
	{ 
	   //MessageBox("生成文件失败","Error",MB_ICONERROR);
	}
	return true;
}

BOOL CSensorsTestDlg::SaveTreeToXML(void)
{
   // TODO: Add your control notification handler code here
	 CString filename;
	if(!autocompare)
    {
  	   /*CFileDialog dlg(FALSE, NULL,modulefile, OFN_FILEMUSTEXIST,"xml 文件|*.xml|所有文件|*.*||", this);
	   if (dlg.DoModal() != IDOK)
		  return FALSE;
	   filename = dlg.GetPathName();
	   if(filename.Find(".xml") < 0) 
		 filename += ".xml";*/
        filename = modulefile;
	}
	else
	{
		filename = savefile;
	}
	
	MSXML2::IXMLDOMDocumentPtr pDoc;
	MSXML2::IXMLDOMElementPtr xmlRoot;
	HRESULT hr = pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument30));
	if(!SUCCEEDED(hr))
	{
		MessageBox("Error");
		return FALSE;
	}
	if(SaveTreeToXml(&m_DeviceTree,(LPTSTR)(LPCTSTR)filename))
	{
	  if(!autocompare)
	  {
		MessageBox("成功生成样本文件"+modulefile);
	  }
	}
	else
	{ 
	   MessageBox("生成文件失败","Error",MB_ICONERROR);
	}
	return TRUE;
}



BOOL CSensorsTestDlg::SaveTreeToXml(CTreeCtrl *pTree,LPSTR xmlFileName){
	//设置默认文件目录
	TCHAR exeFullPath[MAX_PATH]; // MAX_PATH
	GetModuleFileName(NULL,exeFullPath,MAX_PATH);
	CString ProjectPath;
	ProjectPath=(CString) exeFullPath;
	int n=ProjectPath.ReverseFind('\\');
	ProjectPath=ProjectPath.Left(n+1);
	SetCurrentDirectory(ProjectPath);
	MSXML2::IXMLDOMDocumentPtr pDoc;
	MSXML2::IXMLDOMElementPtr xmlRoot;
	HRESULT hr = pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument30));
	if(!SUCCEEDED(hr))    //if(FAILED(hr))
	{
		//MessageBox("Error");
		return FALSE;
	}
	pDoc->async = VARIANT_FALSE;   //default-true

	//加入头信息
	MSXML2::IXMLDOMProcessingInstructionPtr pPI = NULL;
	pPI = pDoc->createProcessingInstruction("xml","version='1.0' encoding='UTF-8'");
	_variant_t vNullVal;
	vNullVal.vt = VT_NULL;
	pDoc->insertBefore(pPI,vNullVal);

	HTREEITEM rootItem = pTree->GetRootItem();

	CString sName;
    CHAR  buf[0x100];
	DWORD size =0x100;
	GetComputerName(buf,&size);
	sName.Format("%s%s","Computer",buf);
	pDoc->raw_createElement((_bstr_t)sName, &xmlRoot);
	pDoc->raw_appendChild(xmlRoot, NULL);
	//xmlRoot->appendChild(pDoc->createTextNode("\n"));  //换行
	
	TreeNodeToXMLNode(pTree,rootItem,pDoc,xmlRoot);

	pDoc->save(_variant_t(xmlFileName));
	
	return TRUE;
}

void CSensorsTestDlg::TreeNodeToXMLNode(CTreeCtrl *pTree,HTREEITEM parentItem,MSXML2::IXMLDOMDocumentPtr pXmlDom,MSXML2::IXMLDOMElementPtr pXmlNode){
	if(!pTree->ItemHasChildren(parentItem)){
		return;
	}

	MSXML2::IXMLDOMElementPtr pChildNode;
	HTREEITEM hChildItem = pTree->GetChildItem(parentItem);
	while(hChildItem != NULL){
		CString szText = pTree->GetItemText(hChildItem);
		BOOL isModuleName = false;			//捕捉模块名，用于更好地生成相应文档，避免出现模块信息为空时，所存信息为DeviceInfo，而不是Device name
		if(!pTree->ItemHasChildren(hChildItem)) //子为叶子
		{
			pXmlNode->appendChild(pXmlDom->createTextNode("\n"));  //换行
			pXmlDom->raw_createElement((_bstr_t)(char*)"DeviceInfo", &pChildNode);
			long checked = pTree->GetCheck(hChildItem);
		    _variant_t value(checked);
			pChildNode->setAttribute((_bstr_t)"checked",value);
			pChildNode->Puttext((_bstr_t)szText);
			pXmlNode->appendChild(pChildNode);
		}
		else //子又有子
		{
			pXmlNode->appendChild(pXmlDom->createTextNode("\n"));  //换行
			pXmlDom->raw_createElement((_bstr_t)(char*)"Device", &pChildNode);
			pChildNode->setAttribute("name",_variant_t(szText));
			pXmlNode->appendChild(pChildNode);
			//pXmlNode->appendChild(pXmlDom->createTextNode("\n"));  //换行
			TreeNodeToXMLNode(pTree,hChildItem,pXmlDom,pChildNode);  //递归
		}
		hChildItem = pTree->GetNextItem(hChildItem,TVGN_NEXT);
		if(hChildItem ==NULL) 
			pXmlNode->appendChild(pXmlDom->createTextNode("\n"));  //换行
	}
	//if(hChildItem == NULL && parentItem == pTree->GetRootItem()) 
		//pXmlNode->appendChild(pXmlDom->createTextNode("\n"));  //换行
}

void CSensorsTestDlg::OnSavexml() 
{
	// TODO: Add your control notification handler code here
	SaveTreeToXML();
	
}



BOOL CSensorsTestDlg::ReadSetting(){
	//纠正到程序运行的当下目录
	TCHAR exeFullPath[MAX_PATH]; // MAX_PATH
	GetModuleFileName(NULL,exeFullPath,MAX_PATH);
	CString ProjectPath;
	ProjectPath=(CString) exeFullPath;
	int n=ProjectPath.ReverseFind('\\');
	ProjectPath=ProjectPath.Left(n+1);
	SetCurrentDirectory(ProjectPath);

	CString setfilename = "configcn.xml";
	CString strTemp;
	CFileStatus m_status;
	if(!CFile::GetStatus(setfilename,m_status)){
		MessageBox("配置文件不存在，请重新配置");
		return false;
	}
	
	MSXML2::IXMLDOMDocumentPtr pXmlDoc;
	MSXML2::IXMLDOMNamedNodeMapPtr pAttrs = NULL;
	HRESULT hr = pXmlDoc.CreateInstance(__uuidof(MSXML2::DOMDocument30));
	
	if (FAILED(hr)) 
	{
	    AfxMessageBox(_T("无法创建Xml对象实例。"));
		return false;
	}
	if(!pXmlDoc->load((LPCTSTR)setfilename) )
	{
		AfxMessageBox(_T("无法加载配置Xml文件。"));
		return false;
	}
	//获取Xml文件首个元素
	MSXML2::IXMLDOMElementPtr pRoot = pXmlDoc->GetdocumentElement();
	
	if(pRoot != NULL)
	{
		pRoot->get_attributes(&pAttrs);
		//将元素显示在tree控件中
		//	HTREEITEM hItem = m_tree.InsertItem(pRoot->GetnodeName());
		//获取所有的子节点
		MSXML2::IXMLDOMNodeListPtr pNodes = pRoot->GetchildNodes();
		_bstr_t modulename;
		int modulestaus = false;
		for(int i=0; i<pNodes->Getlength(); i++)
		{
			//将节点及其下所有子节点显示在tree中
			MSXML2::IXMLDOMNodePtr pNode = pNodes->nextNode();
			_bstr_t str = pNode->GetnodeName();
			if(pNode != NULL)
			{
				if(pNode->GetnodeName() == (_bstr_t)"AutoSave"){
					AutoSave = atoi(pNode->Gettext());
				}
				else if(pNode->GetnodeName() == (_bstr_t)"SaveFile"){
					savefile = (LPSTR)pNode->Gettext();
				}
				else if(pNode->GetnodeName() == (_bstr_t)"AutoCompare"){
					autocompare = atoi(pNode->Gettext());
				}
				else if(pNode->GetnodeName() == (_bstr_t)"ModuleFile"){
					modulefile = (LPSTR)pNode->Gettext();
				}
				else if(pNode->GetnodeName() == (_bstr_t)"Device"){
					LCID lcid = GetSystemDefaultLCID();
					modulename = pNode->Getattributes()->Getitem(0)->GetnodeTypedValue();
					strTemp = (char *)pNode->Gettext();
					if(lcid == 0x804){
						if(strTemp.Find("-") < 0)
							modulestaus = (atoi)(pNode->Gettext());
						else if(modulename == (_bstr_t)"温度范围"){  //温度范围
							mintemp = (atoi)(strTemp.Left(strTemp.Find("-")));
							maxtemp = (atoi)(strTemp.Mid(strTemp.Find("-")+1));
						}
						else if(modulename == (_bstr_t)"风扇转速范围1"){    //"风扇转速范围1"
							minfanspeed1 = (atoi)(strTemp.Left(strTemp.Find("-")));
							maxfanspeed1 = (atoi)(strTemp.Mid(strTemp.Find("-")+1));
						}
						else if(modulename == (_bstr_t)"风扇转速范围2"){  //"风扇转速范围2"
							minfanspeed2 = (atoi)(strTemp.Left(strTemp.Find("-")));
							maxfanspeed2 = (atoi)(strTemp.Mid(strTemp.Find("-")+1));
						}
						else if(modulename == (_bstr_t)"风扇转速范围3"){  //"风扇转速范围3"
							minfanspeed3 = (atoi)(strTemp.Left(strTemp.Find("-")));
							maxfanspeed3 = (atoi)(strTemp.Mid(strTemp.Find("-")+1));
						}
					}else if(lcid == 0x409){
						if(strTemp.Find("-") < 0)
							modulestaus = (atoi)(pNode->Gettext());
						else if(modulename == (_bstr_t)"Temperature"){  //温度范围
							mintemp = (atoi)(strTemp.Left(strTemp.Find("-")));
							maxtemp = (atoi)(strTemp.Mid(strTemp.Find("-")+1));
						}
						else if(modulename == (_bstr_t)"FanSpeed1"){    //"风扇转速范围1"
							minfanspeed1 = (atoi)(strTemp.Left(strTemp.Find("-")));
							maxfanspeed1 = (atoi)(strTemp.Mid(strTemp.Find("-")+1));
						}
						else if(modulename == (_bstr_t)"FanSpeed2"){  //"风扇转速范围2"
							minfanspeed2 = (atoi)(strTemp.Left(strTemp.Find("-")));
							maxfanspeed2 = (atoi)(strTemp.Mid(strTemp.Find("-")+1));
						}
						else if(modulename == (_bstr_t)"FanSpeed3"){  //"风扇转速范围3"
							minfanspeed3 = (atoi)(strTemp.Left(strTemp.Find("-")));
							maxfanspeed3 = (atoi)(strTemp.Mid(strTemp.Find("-")+1));
						}
					}else{
						AfxMessageBox(_T("The language in your system does not support, may case some incorrect."));
						return false;
					}					
				}
			}
		}
	}

	//CoUninitialize();
	return true;
}

BOOL CSensorsTestDlg::CheckSensor()
{
	CString strTemp;

	CString resultfilename = "result.txt";
	CStdioFile resultfile(resultfilename,CFile::modeCreate|CFile::modeWrite);

	int compareNum = 0;   //已匹对模块数目
	int compareSumNum = 0; //共需对比模块数目
	CString result = "";
	//初始化
	CStringArray  TempstrArray;
	CStringArray  DeviceArray;
	CByteArray    byteArray;
	
	TempstrArray.RemoveAll();
	DeviceArray.RemoveAll();
	byteArray.RemoveAll();
	MSXML2::IXMLDOMDocumentPtr pXmlDoc;
	MSXML2::IXMLDOMNamedNodeMapPtr pAttrs = NULL;
	HRESULT hr = pXmlDoc.CreateInstance(__uuidof(MSXML2::DOMDocument30));
	

	if (FAILED(hr)) 
	{
		AfxMessageBox(_T("无法创建Xml对象实例。"));//_T("无法创建Xml对象实例。")
		return false;
	}
	if(!pXmlDoc->load((LPCTSTR)modulefile))
	{
		AfxMessageBox(_T("无法加载模板Xml文件。"));//_T("无法加载模板Xml文件。")
		return false;
	}
	//获取Xml文件首个元素

	MSXML2::IXMLDOMElementPtr pRoot = pXmlDoc->GetdocumentElement();
	
	if(pRoot != NULL)
	{
		pRoot->get_attributes(&pAttrs);
		//将元素显示在tree控件中
		//	HTREEITEM hItem = m_tree.InsertItem(pRoot->GetnodeName());
		//获取所有的子节点
		MSXML2::IXMLDOMNodeListPtr pNodes = pRoot->selectNodes("Device");
		_bstr_t comparemodulename;
		BOOL modulestaus = false;
		for(int i=0; i<pNodes->Getlength(); i++)
		{
			//将节点及其下所有子节点显示在tree中
			MSXML2::IXMLDOMNodePtr pNode = pNodes->nextNode();
			if(pNode != NULL)
			{
				MSXML2::IXMLDOMNodeListPtr pDeviceInfoNodes  =  pNode->GetchildNodes();
				for(int j=0;j<pDeviceInfoNodes->Getlength();j++)
				{
                   MSXML2::IXMLDOMNodePtr pDevicinfoNode = pDeviceInfoNodes->nextNode();
				   _variant_t sattrib  = pDevicinfoNode->Getattributes()->Getitem(0)->GetnodeTypedValue();
				    if(CString(sattrib.pbVal)=="1")
					{
				      byteArray.Add(1);
					}
					else
					{
                      byteArray.Add(0);
					}
                    _bstr_t nodevalue = pDevicinfoNode->Gettext();
					TempstrArray.Add(nodevalue);
				}

			}
		}//if(pNode != NULL)
	}//for
    
    MSXML2::IXMLDOMDocumentPtr pXmlDoc1;
	MSXML2::IXMLDOMNamedNodeMapPtr pAttrs1 = NULL;
	HRESULT hr1 = pXmlDoc1.CreateInstance(__uuidof(MSXML2::DOMDocument30));
	

	if (FAILED(hr1)) 
	{
		AfxMessageBox(_T("无法创建Xml对象实例。"));//_T("无法创建Xml对象实例。")
		return false;
	}
	if(!pXmlDoc1->load((LPCTSTR)savefile))
	{
		AfxMessageBox(_T("无法加载模板Xml文件。"));//_T("无法加载模板Xml文件。")
		return false;
	}
	//获取Xml文件首个元素

	MSXML2::IXMLDOMElementPtr pRoot1 = pXmlDoc1->GetdocumentElement();
	
	if(pRoot1 != NULL)
	{
		pRoot1->get_attributes(&pAttrs1);
		//将元素显示在tree控件中
		//	HTREEITEM hItem = m_tree.InsertItem(pRoot->GetnodeName());
		//获取所有的子节点
		MSXML2::IXMLDOMNodeListPtr pNodes = pRoot1->selectNodes("Device");
		_bstr_t comparemodulename;
		BOOL modulestaus = false;
		for(int i=0; i<pNodes->Getlength(); i++)
		{
			//将节点及其下所有子节点显示在tree中
			MSXML2::IXMLDOMNodePtr pNode = pNodes->nextNode();
			if(pNode != NULL)
			{
				MSXML2::IXMLDOMNodeListPtr pDeviceInfoNodes  =  pNode->GetchildNodes();
				for(int j=0;j<pDeviceInfoNodes->Getlength();j++)
				{
                   MSXML2::IXMLDOMNodePtr pDevicinfoNode = pDeviceInfoNodes->nextNode();
				  // _variant_t sattrib  = pDevicinfoNode->Getattributes()->Getitem(0)->GetnodeTypedValue();
				  
                    _bstr_t nodevalue = pDevicinfoNode->Gettext();
					DeviceArray.Add(nodevalue);
				}
			}
		}//if(pNode != NULL)
	}//for
    BOOL BFailed = FALSE;
	if(TempstrArray.GetSize() != DeviceArray.GetSize())
	{
       BFailed = TRUE;
	}
	else
	{
		BFailed = FALSE;
		for(int i=0;i<TempstrArray.GetSize();i++)
		{
		  if(byteArray.GetAt(i)==(BYTE)1)
		  {
		   	  CString strTemp = TempstrArray.GetAt(i);
		      CString strComp = DeviceArray.GetAt(i);
			  CString strTempLable = strTemp.Left(strTemp.Find(':'));
			  CString strCompLable = strComp.Left(strComp.Find(':'));
			  if(strTempLable==strCompLable)
			  {
				 CString strValue = strComp.Mid(strTemp.Find(':')+1);
				 if(strValue.Find("C")!=-1)
				 {
					 strValue = strValue.Left(strValue.Find("C"));
					 strValue.TrimLeft();
					 strValue.TrimRight();
					 float value = atof(strValue.GetBuffer(0));
					 if(value>=mintemp && value<=maxtemp)
					 {
                        
					 }
					 else
					 {
                       BFailed = TRUE; 
					   break;
					 }
				 }
				 if(strValue.Find("RPM")!=-1)
				 {
                     strValue = strValue.Left(strValue.Find("RPM"));
					 strValue.TrimLeft();
					 strValue.TrimRight();
					 float value = atof(strValue.GetBuffer(0));
					 if(value>=minfanspeed1 && value<=maxfanspeed1)
					 {
                        
					 }
					 else
					 {
					   BFailed = TRUE;
                       break;
					 }
				 }
			  }
		  }
		}
	}
	if(BFailed)
	{
       resultfile.WriteString("1\n");
	   resultfile.WriteString("本地信息\n");
	   {
		   int i;
		   for(i=0;i<byteArray.GetSize() && i<DeviceArray.GetSize();i++)
		 {
		   if(byteArray.GetAt(i))
		   {  
			 resultfile.WriteString(DeviceArray.GetAt(i));
			 resultfile.WriteString("\n");
		   }
		 }
		   for(; i < DeviceArray.GetSize(); i++)
		   {
			   resultfile.WriteString(DeviceArray.GetAt(i));
				resultfile.WriteString("\n");
		   }
	   }
	   resultfile.WriteString("模板信息\n");
	   {
		   int i;
	     for(i=0;i<byteArray.GetSize() && i<TempstrArray.GetSize();i++)
		 {
		   if(byteArray.GetAt(i))
		   {  
			  resultfile.WriteString(TempstrArray.GetAt(i));
			  resultfile.WriteString("\n");
		   }
		 }
	   }
	}
	else
	{
		resultfile.WriteString("0\n");
	}


	return true;
}

BOOL CSensorsTestDlg::CheckSensor2()
{
	CString resultfilename = "result.txt";
	CStdioFile resultfile(resultfilename,CFile::modeCreate|CFile::modeWrite);

	//int compareNum = 0;   //已匹对模块数目
	//int compareSumNum = 0; //共需对比模块数目
	//CString result = "";

	//初始化
	BOOL BFailed = TRUE; 
	CStringArray moduleArray, localArray;
	moduleArray.RemoveAll();
	localArray.RemoveAll();

	MSXML2::IXMLDOMDocumentPtr pXmlDoc, pXmlDoc1;
	MSXML2::IXMLDOMNamedNodeMapPtr pAttrs = NULL, pAttrs1 = NULL;
	HRESULT hr = pXmlDoc.CreateInstance(__uuidof(MSXML2::DOMDocument30));
	HRESULT hr1 = pXmlDoc1.CreateInstance(__uuidof(MSXML2::DOMDocument30));

	if (FAILED(hr)) 
	{
		AfxMessageBox(_T("无法创建Xml对象实例。"));//_T("无法创建Xml对象实例。")
		return false;
	}
	if(!pXmlDoc->load((LPCTSTR)modulefile))
	{
		AfxMessageBox(_T("无法加载模板Xml文件。"));//_T("无法加载模板Xml文件。")
		return false;
	}

	if (FAILED(hr1)) 
	{
		AfxMessageBox(_T("无法创建Xml对象实例。"));//_T("无法创建Xml对象实例。")
		return false;
	}
	if(!pXmlDoc1->load((LPCTSTR)savefile))
	{
		AfxMessageBox(_T("无法加载Xml文件。"));//_T("无法加载模板Xml文件。")
		return false;
	}


	//获取Xml文件首个元素

	MSXML2::IXMLDOMElementPtr pRoot = pXmlDoc->GetdocumentElement();
	MSXML2::IXMLDOMElementPtr pRoot1 = pXmlDoc1->GetdocumentElement();

	if(pRoot != NULL && pRoot1 != NULL)
	{
		pRoot->get_attributes(&pAttrs);
		pRoot1->get_attributes(&pAttrs1);

		//获取所有的子节点
		MSXML2::IXMLDOMNodeListPtr pNodes = pRoot->selectNodes("Device");
		MSXML2::IXMLDOMNodeListPtr pNodes1 = pRoot1->selectNodes("Device");
		//_bstr_t comparemodulename;
		//BOOL modulestaus = false;
		int pLen = pNodes->Getlength();
		int pLen1 = pNodes1->Getlength();
		int pMaxLen = max(pNodes->Getlength(), pNodes1->Getlength());
		for(int i=0; i<pMaxLen; i++)
		{
			MSXML2::IXMLDOMNodePtr pNode = pNodes->nextNode();
			MSXML2::IXMLDOMNodePtr pNode1 = pNodes1->nextNode();
			if(pNode != NULL && pNode1 != NULL)
			{
				MSXML2::IXMLDOMNodeListPtr pDeviceInfoNodes  =  pNode->GetchildNodes();
				MSXML2::IXMLDOMNodeListPtr pDeviceInfoNodes1  =  pNode1->GetchildNodes();

				//int pCheckedLen = GetCheckedLength(pDeviceInfoNodes);
				//int pCheckedLen1 = GetCheckedLength(pDeviceInfoNodes1);
				//
				////需要重新获取开始位置
				//pDeviceInfoNodes  =  pNode->GetchildNodes();				
				//pDeviceInfoNodes1  =  pNode1->GetchildNodes();

				//需要对比的数据段长度不一样时，只把字段保存起来，用于写入result.txt
				//if(pCheckedLen != pCheckedLen1){
				if(1){
					char moduleFlags[20],localFlags[20]; //0,无此项；1，有且比对；2，有但不比对
										
					//int moduleCount = 0, localCount = 0;
					memset(moduleFlags, 0, 20);
					memset(localFlags, 0, 20);
					for(int j=0; j<pDeviceInfoNodes->Getlength(); j++)
					{
						MSXML2::IXMLDOMNodePtr pDevicinfoNode = pDeviceInfoNodes->nextNode();
						 _variant_t sattrib  = pDevicinfoNode->Getattributes()->Getitem(0)->GetnodeTypedValue();
						 if(CString(sattrib.pbVal)=="1")
						 {							 
							 _bstr_t nodevalue = pDevicinfoNode->Gettext();
							 moduleArray.Add(nodevalue);
							// moduleCount ++;
							 moduleFlags[j] = 1;
						 }else{
							 moduleFlags[j] = 2;
						 }
					}
					for(int j=0; j<pDeviceInfoNodes1->Getlength(); j++)
					{
						MSXML2::IXMLDOMNodePtr pDevicinfoNode1 = pDeviceInfoNodes1->nextNode();
						 _variant_t sattrib  = pDevicinfoNode1->Getattributes()->Getitem(0)->GetnodeTypedValue();
						 if(CString(sattrib.pbVal)=="1")
						 {
							 _bstr_t nodevalue = pDevicinfoNode1->Gettext();
							 localArray.Add(nodevalue);
							 localFlags[j] = 1;
						 }else{
							 localFlags[j] = 2;
						 }
					}

					//比对原则：在对应位置上，模板中的比对项在本地必须有，同样本地的比对项在模板中也必须有；

					for(int i = 0; i < 20; i++){
						if(moduleFlags[i] == 1){
							if(localFlags[i] == 0){
								BFailed = FALSE;
							}
						}
						if(localFlags[i] == 1){
							if(moduleFlags[i] == 0){
								BFailed = FALSE;
							}
						}
					}

					if(BFailed == TRUE){
						//需要重新获取开始位置
						pDeviceInfoNodes  =  pNode->GetchildNodes();				
						pDeviceInfoNodes1  =  pNode1->GetchildNodes();
					}else{
						continue;
					}
				}//if(1)

				int pCheckedLen = GetCheckedLength(pDeviceInfoNodes);
				pDeviceInfoNodes  =  pNode->GetchildNodes();

				//需要对比的数据段长度一样时，继续判断其它条件，并保存字段
				for(int j=0; j<pCheckedLen; j++)
				{
                   MSXML2::IXMLDOMNodePtr pDevicinfoNode = pDeviceInfoNodes->nextNode();
				   MSXML2::IXMLDOMNodePtr pDevicinfoNode1 = pDeviceInfoNodes1->nextNode();

				   _variant_t sattrib  = pDevicinfoNode->Getattributes()->Getitem(0)->GetnodeTypedValue();
				   _variant_t sattrib1  = pDevicinfoNode1->Getattributes()->Getitem(0)->GetnodeTypedValue();
				    while(++j < pCheckedLen && (CString(sattrib.pbVal)=="0" || CString(sattrib1.pbVal)=="0"))
					{
						//j++;
						pDevicinfoNode = pDeviceInfoNodes->nextNode();
						sattrib  = pDevicinfoNode->Getattributes()->Getitem(0)->GetnodeTypedValue();
						pDevicinfoNode1 = pDeviceInfoNodes1->nextNode();
						sattrib1  = pDevicinfoNode1->Getattributes()->Getitem(0)->GetnodeTypedValue();						
					}

					if(j <= pCheckedLen){
						_bstr_t nodevalue = pDevicinfoNode->Gettext();
						_bstr_t nodevalue1 = pDevicinfoNode1->Gettext();
						//moduleArray.Add(nodevalue);
						//localArray.Add(nodevalue1);
						bool ret = CompareNodeValue((LPCSTR)nodevalue, (LPCSTR)nodevalue1, this);
						if(ret == FALSE){
							BFailed = FALSE;
						}
					}
				}
				//if(BFailed == FALSE) break;
			}else{//if(pNode != NULL && pNode1 != NULL)
				if(pNode != NULL)
				{
					MSXML2::IXMLDOMNodeListPtr pDeviceInfoNodes  =  pNode->GetchildNodes();
					int pCheckedLen = GetCheckedLength(pDeviceInfoNodes);				
					//需要重新获取开始位置
					pDeviceInfoNodes  =  pNode->GetchildNodes();	

					char a[20];
					memset(a, 1, 20);
					for(int j=0; j<pDeviceInfoNodes->Getlength(); j++)
					{
						MSXML2::IXMLDOMNodePtr pDevicinfoNode = pDeviceInfoNodes->nextNode();
							_variant_t sattrib  = pDevicinfoNode->Getattributes()->Getitem(0)->GetnodeTypedValue();
							if(CString(sattrib.pbVal)=="1")
							{							 
								_bstr_t nodevalue = pDevicinfoNode->Gettext();
								moduleArray.Add(nodevalue);
							}else{
								a[j] = 0;
							}
					}
					BFailed = FALSE;
				}
				if(pNode1 != NULL)
				{
					MSXML2::IXMLDOMNodeListPtr pDeviceInfoNodes  =  pNode1->GetchildNodes();
					int pCheckedLen = GetCheckedLength(pDeviceInfoNodes);				
					//需要重新获取开始位置
					pDeviceInfoNodes  =  pNode->GetchildNodes();	

					char a[20];
					memset(a, 1, 20);
					for(int j=0; j<pDeviceInfoNodes->Getlength(); j++)
					{
						MSXML2::IXMLDOMNodePtr pDevicinfoNode = pDeviceInfoNodes->nextNode();						 
						_bstr_t nodevalue = pDevicinfoNode->Gettext();
						localArray.Add(nodevalue);
					}
					BFailed = FALSE;
				}
			}//if(pNode != NULL && pNode1 != NULL)
		}//for
	}//for  
	
	if(BFailed == FALSE)
	{
       resultfile.WriteString("1\n");
	   resultfile.WriteString("本地信息\n");
	   {
			for(int i=0;i<localArray.GetSize();i++)
			{
				resultfile.WriteString(localArray.GetAt(i));
				resultfile.WriteString("\n");
			}			
	   }
	   resultfile.WriteString("模板信息\n");
	   {
		 for(int i=0;i<moduleArray.GetSize() && i<moduleArray.GetSize();i++)
		 {
			  resultfile.WriteString(moduleArray.GetAt(i));
			  resultfile.WriteString("\n");
		 }

	   }
	}
	else
	{
		resultfile.WriteString("0\n");
	}

	resultfile.Close();

	return BFailed;
}

void CSensorsTestDlg::OnCompare() 
{
	// TODO: Add your control notification handler code here
	//CheckSensor();
	CheckSensor2();
}

void CSensorsTestDlg::OnTimer(UINT nIDEvent) 
{
	// TODO: Add your message handler code here and/or call default
	if(nIDEvent==0x1100)
	{
		KillTimer(nIDEvent);
		OnRefresh();
		Sleep(100);
		OnRefresh();
		if(autocompare)
		{
			SaveTreeToXML();
			//CheckSensor();
			CheckSensor2();
			OnOK();
		}
     
	}
	CDialog::OnTimer(nIDEvent);
}

void CSensorsTestDlg::OnOK() 
{
	// TODO: Add extra validation here
	SaveTreeToXML();
	CDialog::OnOK();
}

void CSensorsTestDlg::OnBnClickedButton2()
{
	CString filename = savefile;
	
	MSXML2::IXMLDOMDocumentPtr pDoc;
	MSXML2::IXMLDOMElementPtr xmlRoot;
	HRESULT hr = pDoc.CreateInstance(__uuidof(MSXML2::DOMDocument30));
	if(!SUCCEEDED(hr))
	{
		MessageBox("Error");
		return ;
	}

	//CheckSensor2(); //Liudonghua
	//return;

	if(SaveTreeToXml(&m_DeviceTree,(LPTSTR)(LPCTSTR)filename))
	{
		//CheckSensor();
		CheckSensor2();
	}
	else
	{ 
	   MessageBox("生成文件失败","Error",MB_ICONERROR);
	}
	
}
