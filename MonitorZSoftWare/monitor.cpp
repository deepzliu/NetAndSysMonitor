#pragma once

#include "monitor.h"
#include <Windows.h>
#include <vector>
#include <fstream>
#include <locale>
#include <psapi.h>
#include <tlhelp32.h>
#include <Shlwapi.h>

using namespace std;

#pragma comment(lib, "Psapi.lib")
#pragma comment(lib, "User32.lib")
#pragma comment(lib, "Shlwapi.lib")

#define ARR_SIZE 256

/*
其中cmd是关闭或其它按钮名称，即对该窗口出现时执行的操作，空，则不做操作
*/
#define W_STR
#ifdef W_STR
typedef struct _WindowItem{
	wchar_t title[ARR_SIZE];
	wchar_t content[ARR_SIZE];
	wchar_t msg[ARR_SIZE];
}WindowItem;

typedef struct _ProcessItem{
	bool start;
	bool hung;
	wchar_t name[ARR_SIZE];
	wchar_t cmdline[ARR_SIZE];
	vector<WindowItem> wins;
}ProcessItem;
#else
typedef struct _WindowItem{
	string title;
	string content;
	string cmd;
}WindowItem;

typedef struct _ProcessItem{
	bool start;
	bool hung;
	string name;
	vector<WindowItem> wins;
}ProcessItem;
#endif

typedef struct _CallbackParam{ //用于传递给回调函数的数据
	int PID;//进程ID	
	int nItem; //procItems的下标
	wchar_t ProcName[260];//进程名称
	wchar_t BaseDlgTitle[260]; //父窗口的标题
	bool WinReported; //父窗口是否报过异常
	bool HungReported; //进程挂起是否报过异常
}CallbackParam;
//#define MAX_PROC 64
vector<ProcessItem> procItems;
int gProcSize; //监控进程和服务的数量，用于确定ProcExist的大小
bool *ProcExist;
int timer; //监控的时间间隔，以秒为单位
msgcallback MsgFunc; //监控信息的回调函数，系统资源和异常情况时调用

int ReadProcessConfig(wstring &line, int nCount);
int ReadSettingConfig(wstring &line);

int Tolower(wchar_t *str){
	int n = wcslen(str);
	for(int i = 0; i < n; i++){
		if(str[i] <= L'Z' && str[i] >= L'A'){
			str[i] = str[i] + (L'a' - L'A');
		}
	}
	return 0;
}

int ReadConfig(wchar_t *file)
{
	wchar_t dir[256];
	::GetCurrentDirectoryW(256, dir);
	swprintf(dir, L"%s\\%s", dir, file);

	int nSize = 0;
	wstring line;
	locale lang("chs");
	wifstream fin(file);
	if(!fin.is_open()){
		MessageBox(NULL, L"配置文件打开失败", L"警告", MB_OK | MB_ICONWARNING);
		return -1;
	}else{
		fin.imbue(lang);
	}

	ProcessItem pItem;
	procItems.clear(); //每执行一次都会读配置文件，所以需要清空
	//WindowItem wItem;
	wchar_t type[64], name[64];
	int nCount = 0, ret = 0;
	int ItemFlag = 0; //0： 无； 1： process； 2：service； 3：setting

	while(getline(fin, line)){
		nSize = line.size();
		if(nSize < 1) continue;
		if(line[0] == '#'){
			continue;
		}else if(line[0] == '['){			
			//判断是否一个进程的配置，并获取进程名
			int pos = line.find(L'=');
			if(pos >= 0){
				ret = swscanf(line.c_str(), L"[%[^=]=%[^]]]", type, name);
			}else{
				ret = swscanf(line.c_str(), L"[%[^]]]", type);
				//ret = swscanf(line.c_str(), L"[%[^=]=%[^]]]", type, name);
			}
			
			if(ret == 0) continue;
			if(wcsncmp(type, L"process", wcslen(type)) == 0){				
				if(ret != 2) continue;
				ItemFlag = 1;
				Tolower(name);
				memset(&pItem, 0, sizeof(ProcessItem));
				memcpy(pItem.name, name, wcslen(name) * 2);
				pItem.start = false;
				pItem.hung = false;
				procItems.push_back(pItem);
				nCount++;
			}else if(wcsncmp(type, L"setting", wcslen(type)) == 0){
				ItemFlag = 3;
			}else if(wcsncmp(type, L"service", wcslen(type)) == 0){
				ItemFlag = 2;
			}else {
				ItemFlag = 0;
			}
		}else{
			if(ItemFlag == 0){
				//无需处理
			}else if(ItemFlag == 1){
				ReadProcessConfig(line, nCount);
			}else if(ItemFlag == 2){
				//暂不支持
			}else if(ItemFlag == 3){
				ReadSettingConfig(line);
			}else{
				//无需处理
			}
			
		}
	}

	gProcSize = procItems.size();
	delete[] ProcExist;
	ProcExist = new bool[gProcSize];
	memset(ProcExist, 0, gProcSize);

	fin.close();
	return 0;
}

int ReadProcessConfig(wstring &line, int nCount)
{	
	wchar_t str1[64], str2[64], str3[64], str4[64];
	int ret = 0;

	if(wcsncmp(line.c_str(), L"start", wcslen(L"start")) == 0){
		ret = swscanf(line.c_str(), L"%[^=]=%s", str1, str2);
		if(str2[0] == 'n'){
			procItems[nCount-1].start = false;
		}else if(str2[0] = 'y'){
			procItems[nCount-1].start = true;
		}
	}else if(wcsncmp(line.c_str(), L"hung", wcslen(L"hung")) == 0){
		ret = swscanf(line.c_str(), L"%[^=]=%s", str1, str2);
		if(str2[0] == 'n'){
			procItems[nCount-1].hung = false;
		}else if(str2[0] = 'y'){
			procItems[nCount-1].hung = true;
		}
	}else if(wcsncmp(line.c_str(), L"wnd", wcslen(L"wnd")) == 0){
		WindowItem wItem;
		memset(&wItem, 0, sizeof(WindowItem));
		ret = swscanf(line.c_str(), L"%[^=]=%[^,],%[^,],%[^,]", str1, str2, str3, str4);
		memcpy(wItem.title, str2, wcslen(str2) * 2);
		memcpy(wItem.content, str3, wcslen(str3) * 2);
		memcpy(wItem.msg, str4, wcslen(str4) * 2);
		procItems[nCount-1].wins.push_back(wItem);
	}else if(wcsncmp(line.c_str(), L"cmd", wcslen(L"cmd")) == 0){
		ret = swscanf(line.c_str(), L"%[^=]=%s", str1, str2);
		memcpy(procItems[nCount-1].cmdline, str2, wcslen(str2) * 2);
	}else{
		//无效字段
	}
	return 0;
}

int ReadSettingConfig(wstring &line)
{
	int ret = 0;
	wchar_t str1[64], str2[64];
	if(wcsncmp(line.c_str(), L"timer", wcslen(L"timer")) == 0){
		ret = swscanf(line.c_str(), L"%[^=]=%s", str1, str2);
		timer = StrToInt(str2);
	}
	return 0;
}

BOOL CALLBACK EnumThreadChildProc(HWND hwnd, LPARAM lParam)  
{
	CallbackParam *nParam = (CallbackParam*)lParam;
	//HWND pHwnd = ::GetParent(hwnd);;
	//枚举子窗口的回调函数	
	setlocale(LC_ALL,"chs");
	TCHAR lpWinTitle[256],lpClassName[256];  
	::GetWindowTextW(hwnd,(LPWSTR)lpWinTitle,256-1); //获得窗口caption
	::GetClassNameW(hwnd,(LPWSTR)lpClassName,256-1); //获得窗口类名

	//线程是否挂起
	if(::IsHungAppWindow(hwnd) && procItems[nParam->nItem].hung){		
		//MsgFunc( nParam->PID, nParam->ProcName, L"Thread Hung", wcslen(L"Thread Hung"),NULL);
	}

	//控件内容判断
	int nSize = procItems[nParam->nItem].wins.size();
	wchar_t *ptr, *ptrTitle;
	bool flag = false;
	for(int i = 0; i < nSize; i++){		
		ptr = procItems[nParam->nItem].wins[i].content;
		ptrTitle = procItems[nParam->nItem].wins[i].title;
		if(wcsncmp(ptr, lpWinTitle, wcslen(ptr)) == 0 /*&&
			wcsncmp(ptrTitle, nParam->BaseDlgTitle, wcslen(ptrTitle)) == 0*/){
				flag = true;
				break;
		}
	}
	
	if(flag && !nParam->WinReported)
		MsgFunc( nParam->PID, nParam->ProcName, AB_DLG, wcslen(AB_DLG), (void*)lpWinTitle);

	return TRUE;
}

BOOL CALLBACK ThreadEnumFunc(HWND hwnd, LPARAM lParam)
{
	CallbackParam *nParam = (CallbackParam*)lParam;
	wchar_t sWinTitle[100];
	::GetWindowText(hwnd,sWinTitle,sizeof(sWinTitle));
	
	//进程是否挂起
	if(::IsHungAppWindow(hwnd) && !nParam->HungReported && procItems[nParam->nItem].hung){
		nParam->HungReported = true;
		MsgFunc( nParam->PID, nParam->ProcName, PROC_HUNG, wcslen(PROC_HUNG),NULL);
	}

	//窗口标题判断
	int nSize = procItems[nParam->nItem].wins.size();
	wchar_t *ptr;
	bool flag = false;
	for(int i = 0; i < nSize; i++){
		ptr = procItems[nParam->nItem].wins[i].title;
		if( wcsncmp(ptr, sWinTitle, wcslen(ptr)) == 0/* &&
			procItems[nParam->nItem].wins[i].content == NULL*/){
				flag = true;
				break;
		}
	}
	memcpy(nParam->BaseDlgTitle, sWinTitle, wcslen(sWinTitle) * 2 + 2);
	
	if(flag){
		nParam->WinReported = true;
		MsgFunc( nParam->PID, nParam->ProcName, AB_DLG, wcslen(AB_DLG), (void*)sWinTitle);
	}else{
		nParam->WinReported = false;
	}

	EnumChildWindows(hwnd, EnumThreadChildProc, lParam);
	return 1;
}

BOOL EnumThreadList(DWORD dwOwnerPID, CallbackParam *nParam) 
{ 
    HANDLE        hThreadSnap = NULL; 
    BOOL          bRet        = FALSE; 
    THREADENTRY32 te32        = {0}; 
 
    // Take a snapshot of all threads currently in the system. 
    hThreadSnap = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0); 
    if (hThreadSnap == INVALID_HANDLE_VALUE) 
        return (FALSE); 
 
    // Fill in the size of the structure before using it. 
    te32.dwSize = sizeof(THREADENTRY32); 
 
    // Walk the thread snapshot to find all threads of the process. 
    // If the thread belongs to the process, add its information 
    // to the display list.
    if (Thread32First(hThreadSnap, &te32)){ 
        do{ 
            if (te32.th32OwnerProcessID == dwOwnerPID){ 
				EnumThreadWindows(te32.th32ThreadID, ThreadEnumFunc, (LPARAM)nParam);				
            } 
        }while (Thread32Next(hThreadSnap, &te32)); 
        bRet = TRUE; 
    }else 
        bRet = FALSE;          // could not walk the list of threads 
 
    // Do not forget to clean up the snapshot object. 
    CloseHandle (hThreadSnap); 
 
    return (bRet); 
} 

//根据进程名称，如果是需要监控的，返回procItems的下标，-1表示不需要监控的进程
int FindProecss(wchar_t *procname){
	if(procname == NULL) return -1;

	Tolower(procname);
	unsigned int n = procItems.size();
	for(int i = 0; i < n; i++){
		int s = wcslen(procItems[i].name);
		wchar_t *p = procItems[i].name;
		s = wcsncmp(procItems[i].name, procname, wcslen(procItems[i].name));
		if(wcsncmp(procname, procItems[i].name, wcslen(procItems[i].name)) == 0){
			return i;
		}
	}

	return -1;
}

int CheckProecss(CallbackParam *nParam){
	//MsgFunc( nParam->PID, nParam->ProcName, L"Process found", wcslen(L"Process found"),NULL);
	EnumThreadList(nParam->PID, nParam);
	return 0;
}

int IfAnyProcessExit(){
	for(int i = 0; i < gProcSize; i ++){
		if(ProcExist[i] == 0){
			MsgFunc(0, procItems[i].name, PROC_NOT_FOUND, wcslen(PROC_NOT_FOUND),NULL);
		}
	}
	return 0;
}

int GetProcessName(wchar_t *name, int len, DWORD processID)
{
	if(name == NULL || len < 32) return -1;

    TCHAR szProcessName[MAX_PATH] = TEXT("<unknown>");
    // Get a handle to the process.
    HANDLE hProcess = OpenProcess( PROCESS_QUERY_INFORMATION |
                                   PROCESS_VM_READ,
                                   FALSE, processID );
    // Get the process name.
    if (NULL != hProcess )
    {
        HMODULE hMod;
        DWORD cbNeeded;

        if ( EnumProcessModules( hProcess, &hMod, sizeof(hMod), 
             &cbNeeded) )
        {
            GetModuleBaseName( hProcess, hMod, szProcessName, 
                               sizeof(szProcessName)/sizeof(TCHAR) );
        }
    }
	memcpy(name, szProcessName, wcslen(szProcessName) * 2 + 2);

	//查找进程是否需要监控
	CallbackParam nParam;
	int nItem = FindProecss(szProcessName);
	if(nItem != -1 && nItem < gProcSize){
		ProcExist[nItem] = 1;//标记需要监控且正在运行的进程，便于后续判断进程是否存在
		nParam.PID = processID;
		//nParam.ProcName = szProcessName;
		memcpy(nParam.ProcName, szProcessName, wcslen(szProcessName) * 2 + 2);
		nParam.nItem = nItem;
		memset(nParam.BaseDlgTitle, 0, 2);
		nParam.WinReported = false;
		nParam.HungReported = false;
		//CheckProecss(nItem, hProcess, processID, szProcessName);
		CheckProecss(&nParam);
	}
    // Release the handle to the process.
    CloseHandle( hProcess );
	return 0;
}

int BrowseProcesses( void )
{
	wchar_t cProecssName[ARR_SIZE];
	int len = ARR_SIZE;
    // Get the list of process identifiers.
    DWORD aProcesses[1024], cbNeeded, cProcesses;
    unsigned int i;
    if ( !EnumProcesses(aProcesses, sizeof(aProcesses), &cbNeeded ) )
    {
        return 1;
    }	
    // Calculate how many process identifiers were returned.
    cProcesses = cbNeeded / sizeof(DWORD);

	// Print the name and process identifier for each process.
	for ( i = 0; i < cProcesses; i++ )
	{
		if( aProcesses[i] != 0 ){
			if(GetProcessName(cProecssName, len, aProcesses[i]) == 0){
				//MsgFunc( aProcesses[i], cProecssName, L"find a process", wcslen(L"find a process"),NULL);
			}
		}
	}

	IfAnyProcessExit();
	MsgFunc( -1, NULL, L"END", wcslen(L"END"),NULL);

	return 0;
}

/*
	timer以秒为单位
*/
int MonitorFunc(wchar_t *confFile)
{
	

	while(true){
		ReadConfig(confFile);		
		BrowseProcesses();
		Sleep(timer * 1000);
	}
	return 0;
}

int StartMonitor(msgcallback msgfunc, int pTimer, wchar_t *confFile)
{
	if(msgfunc == NULL){
		MessageBox(NULL, L"请设置回调函数", L"警告", MB_OK | MB_ICONWARNING);
		return 1;
	}
	wchar_t *cfgfile, file[] = L"config.ini";
	if(confFile == NULL){
		cfgfile = file;
	}else{
		cfgfile = confFile;
	}
	timer = pTimer;
	MsgFunc = msgfunc;
	//ReadConfig(cfgfile);
	MonitorFunc(cfgfile);
	return 0;
}