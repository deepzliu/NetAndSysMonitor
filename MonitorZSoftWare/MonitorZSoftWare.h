#pragma once

//#include "StdAfx.h"
//#include <Windows.h>
//#include <stdio.h>
//#include <share.h>
//#include <vector>
//#include <fstream>
//#include <string>
//#include <psapi.h>
//#include <tlhelp32.h>
//#include <locale>
//using namespace std;
//
//#pragma comment(lib, "Psapi.lib")
//
//#define ARR_SIZE 256

/*

//其中cmd是关闭或其它按钮名称，即对该窗口出现时执行的操作，空，则不做操作

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

typedef void (*callbackfunc)(unsigned int PID, wchar_t ProcessName, wchar_t *msg, int msgLen, void *lparam);

vector<ProcessItem> procItems;
callbackfunc MsgFunc;

int ReadProcessConfig(wstring &line, int nCount);

int ReadConfig(wchar_t *file)
{
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
	//WindowItem wItem;
	wchar_t type[64], name[64];
	int nCount = 0, ret = 0;
	int ItemFlag = 0; //0： 无； 1： process； 2：service

	while(getline(fin, line)){
		nSize = line.size();
		if(nSize < 1) continue;
		if(line[0] == '#'){
			continue;
		}else if(line[0] == '['){			
			//判断是否一个进程的配置，并获取进程名
			ret = swscanf_s(line.c_str(), L"[%[^=]=%[^]]]", type, name);
			if(ret == 0) continue;
			if(wcsncmp(type, L"process", wcslen(type)) == 0){				
				if(ret != 2) continue;
				ItemFlag = 1;
				memset(&pItem, 0, sizeof(ProcessItem));
				memcpy(pItem.name, name, wcslen(name) * 2);
				pItem.start = false;
				pItem.hung = false;
				procItems.push_back(pItem);
				nCount++;
			}
		}else{
			if(ItemFlag == 0){
				//无需处理
			}else if(ItemFlag == 1){
				ReadProcessConfig(line, nCount);
			}else if(ItemFlag == 2){
				//暂不支持
			}else{
				//无需处理
			}
			
		}
	}
	fin.close();
}

int ReadProcessConfig(wstring &line, int nCount)
{	
	wchar_t str1[64], str2[64], str3[64], str4[64];
	int ret = 0;

	if(wcsncmp(line.c_str(), L"start", wcslen(L"start")) == 0){
		ret = swscanf_s(line.c_str(), L"%[^=]=%s", str1, str2);
		if(str2[0] == 'n'){
			procItems[nCount-1].start = false;
		}else if(str2[0] = 'y'){
			procItems[nCount-1].start = true;
		}
	}else if(wcsncmp(line.c_str(), L"hung", wcslen(L"hung")) == 0){
		ret = swscanf_s(line.c_str(), L"%[^=]=%s", str1, str2);
		if(str2[0] == 'n'){
			procItems[nCount-1].hung = false;
		}else if(str2[0] = 'y'){
			procItems[nCount-1].hung = true;
		}
	}else if(wcsncmp(line.c_str(), L"wnd", wcslen(L"wnd")) == 0){
		WindowItem wItem;
		memset(&wItem, 0, sizeof(WindowItem));
		ret = swscanf_s(line.c_str(), L"%[^=]=%[^,],%[^,],%[^,]", str1, str2, str3, str4);
		memcpy(wItem.title, str1, wcslen(str1) * 2);
		memcpy(wItem.content, str2, wcslen(str2) * 2);
		memcpy(wItem.msg, str3, wcslen(str3) * 2);
		procItems[nCount-1].wins.push_back(wItem);
	}else if(wcsncmp(line.c_str(), L"cmd", wcslen(L"cmd")) == 0){
		ret = swscanf_s(line.c_str(), L"%[^=]=%s", str1, str2);
		memcpy(procItems[nCount-1].cmdline, str2, wcslen(str2) * 2);
	}else{
		//无效字段
	}
	return 0;
}

ProcessItem *FindProecss(wchar_t *procname){
	if(procname == NULL) return NULL;


	return NULL;
}

int CheckProecss(ProcessItem *pItem, HANDLE hProcess, DWORD pID){

	return NULL;
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
	memcpy(name, szProcessName, wcslen(szProcessName) * 2);

	//
	ProcessItem *pItem = FindProecss(szProcessName);
	if(pItem != NULL){
		CheckProecss(pItem, hProcess, processID);
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
				//
			}
		}
	}
	return 0;
}


//	timer以秒为单位

int MonitorFunc(callbackfunc func, unsigned int timer)
{
	if(func == NULL){
		MessageBox(NULL, L"请设置回调函数", L"警告", MB_OK | MB_ICONWARNING);
		return 1;
	}

	MsgFunc = func;

	while(true){
		BrowseProcesses();
		Sleep(timer * 1000);
	}

	return 0;
}

*/