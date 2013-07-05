// Monitor.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

//#include <afxwin.h>
//int _tmain(int argc, _TCHAR* argv[])
//{
//	return 0;
//}


#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#include <psapi.h>
#include "process_stat.h"
#include <tlhelp32.h>
#include <locale>

#pragma comment(lib, "Psapi.lib")
//#pragma comment(linker, "/subsystem:\"windows\" /entry:\"mainCRTStartup\"" )// 设置入口地址

// To ensure correct resolution of symbols, add Psapi.lib to TARGETLIBS
// and compile with -DPSAPI_VERSION=1

void PrintProcessNameAndID( DWORD processID )
{
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

    // Print the process name and identifier.
    _tprintf( TEXT("%s  (PID: %u)\n"), szProcessName, processID );

    // Release the handle to the process.
    CloseHandle( hProcess );
}

void useToolHelp()
{
	HANDLE procSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if(procSnap == INVALID_HANDLE_VALUE)
	{
		printf("CreateToolhelp32Snapshot failed, %d \n",GetLastError());
		return;
	}
	//
	PROCESSENTRY32 procEntry = { 0 };
	procEntry.dwSize = sizeof(PROCESSENTRY32);
	BOOL bRet = Process32First(procSnap,&procEntry);
	while(bRet)
	{
		wprintf(L"PID: %d (%s) \n", procEntry.th32ProcessID, procEntry.szExeFile); 
		bRet = Process32Next(procSnap, &procEntry);
	}
	CloseHandle(procSnap);
}

int ShowProcesses( void )
{
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
		if( aProcesses[i] != 0 )
		{
			PrintProcessNameAndID( aProcesses[i] );
		}
	}
	return 0;
}

int count;
FILE *fp;
BOOL CALLBACK EnumChildProc(HWND hwnd,LPARAM lParam)  
{
	//枚举子窗口的回调函数
	
	setlocale(LC_ALL,"chs");
	count++;
	TCHAR lpWinTitle[256],lpClassName[256];  
	::GetWindowTextW(hwnd,(LPWSTR)lpWinTitle,256-1); //获得窗口caption
	::GetClassNameW(hwnd,(LPWSTR)lpClassName,256-1); //获得窗口类名
	if (wcsicmp((LPWSTR)lpWinTitle,L"")!=NULL && wcsicmp((LPWSTR)lpClassName,L"")!=NULL) //过滤掉没有名子和类名的窗口
	{
		fwprintf(fp,_T("\n"));
		for (int i=1;i<=count;i++) fprintf(fp," ");

		fwprintf(fp,_T("-0x%x:'%ws':'%ws'\n"),hwnd,lpWinTitle,lpClassName);
  
		//EnumChildWindows(hwnd,EnumChildProc,0); //递归枚举子窗口
	}
	return TRUE;
}

BOOL CALLBACK EnumWindowsProc(HWND hwnd,LPARAM lparam)
{
	//枚举窗口的回调函数
	setlocale(LC_ALL,"chs");
	count = 1;
	TCHAR lpWinTitle[256],lpClassName[256]; 
	::GetWindowTextW(hwnd,lpWinTitle,256-1); //获得窗口caption
	::GetClassNameW(hwnd,lpClassName,256-1); //获得窗口类名
	if (wcsicmp(lpWinTitle,L"")!=NULL && wcsicmp(lpClassName,L"")!=NULL) //过滤掉没有名子和类名的窗口
	{
		fwprintf(fp,_T("*-0x%x:'%ws':'%ws'\n"),hwnd,lpWinTitle,lpClassName); 
		if(hwnd == (HWND)0x1B12CC){
			system("pause");
		}
		EnumChildWindows(hwnd, EnumChildProc,0); //继续枚举子窗口，传递给回调函数该子窗口的句柄
	}
	return TRUE ;
}

void ShowWindows(){
	//fp = fopen("D:\\tmp\\windows.txt", "w");
	fp = stdout;
	::EnumWindows(EnumWindowsProc,0);
}

BOOL CALLBACK EnumThreadChildProc(HWND hwnd,LPARAM lParam)  
{
	//枚举子窗口的回调函数	
	setlocale(LC_ALL,"chs");
	TCHAR lpWinTitle[256],lpClassName[256];  
	::GetWindowTextW(hwnd,(LPWSTR)lpWinTitle,256-1); //获得窗口caption
	::GetClassNameW(hwnd,(LPWSTR)lpClassName,256-1); //获得窗口类名
	fwprintf(stdout,_T("     [+] %ws, %ws\n"), lpWinTitle, lpClassName);
	if(::IsHungAppWindow(hwnd)){
		printf("         Hung\n");
	}else{
		printf("         Not Hung\n");
	}
	return TRUE;
}

BOOL CALLBACK ThreadEnumFunc(HWND hwnd, LPARAM lParam)
{
	wchar_t str[100];
	::GetWindowText(hwnd,str,sizeof(str));
	wprintf(_T("  [+] %ws\n"), str);	
	if(::IsHungAppWindow(hwnd)){
		printf("      Hung\n");
	}else{
		printf("      Not Hung\n");
	}
	EnumChildWindows(hwnd, EnumThreadChildProc, 0);
	return 1;
}

BOOL RefreshThreadList (DWORD dwOwnerPID) 
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

    if (Thread32First(hThreadSnap, &te32)) 
    { 
        do 
        { 
            if (te32.th32OwnerProcessID == dwOwnerPID) 
            { 
                printf( "\nTID = %x, ", te32.th32ThreadID); 
                printf( "Owner PID = %x\n", te32.th32OwnerProcessID); 
                //printf( "Delta Priority\t%x\n", te32.tpDeltaPri); 
                //printf( "Base Priority\t%x\n", te32.tpBasePri); 

				EnumThreadWindows(te32.th32ThreadID, ThreadEnumFunc, 0);
				//break;
				
            } 
        } 
        while (Thread32Next(hThreadSnap, &te32)); 
        bRet = TRUE; 
    } 
    else 
        bRet = FALSE;          // could not walk the list of threads 
 
    // Do not forget to clean up the snapshot object. 

    CloseHandle (hThreadSnap); 
 
    return (bRet); 
} 

void func(void* lp){
	while(1){
		Sleep(10000);
	}
}

int main( int argc, char *argv[] ){

	//HWND hwnd = GetConsoleWindow();
	//ShowWindow(hwnd,0);
	HWND   hWnd;
	//SetConsoleTitle((LPCWSTR)argv[0]);
	//hWnd=::FindWindow(NULL, (LPCWSTR)argv[0]);
	hWnd=GetConsoleWindow();
	//ShowWindow(hWnd,SW_HIDE);

	int id;
	if(argc == 2){
	}
	RefreshThreadList(19064);
	func(NULL);
	//CWinThread *pt = AfxBeginThread((AFX_THREADPROC)func, NULL);

	system("pause");
    return 0;
}


