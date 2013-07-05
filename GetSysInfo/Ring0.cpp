 
//#include "StdAfx.h"

#include "Ring0.h"
#include <afxmt.h>
#include <io.h>
#include <winioctl.h>

char szWinIoDriverPath[MAX_PATH];
bool GetDriverPath()
{
  PSTR pszSlash;

  if (!GetModuleFileNameA(GetModuleHandle(NULL), szWinIoDriverPath, sizeof(szWinIoDriverPath)))
    return false;

  pszSlash = strrchr(szWinIoDriverPath, '\\');

  if (pszSlash)
    pszSlash[1] = 0;
  else
    return false;
  strcat_s(szWinIoDriverPath, "WinRing0.sys");

  return true;
}


Ring0::Ring0(void)
{
	driver = INVALID_HANDLE_VALUE;
}
Ring0::~Ring0()
{
	if(driver!=INVALID_HANDLE_VALUE)
	{
		Close();
	}
}
// upgrade for x64 & win32
bool Ring0::ExtractDriver(void)
{
    
	typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
	typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);

	PSTR pszSlash;
	OSVERSIONINFOEX osvi;
	SYSTEM_INFO si;
	BOOL bOsVersionInfoEx;
	DWORD dwType;
	PGNSI pGNSI;
	PGPI pGPI;
	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if( !(bOsVersionInfoEx = GetVersionEx ((OSVERSIONINFO *) &osvi)) )
		return false;

	pGNSI = (PGNSI) GetProcAddress(
		GetModuleHandle(TEXT("kernel32.dll")),"GetNativeSystemInfo");
	if(NULL!=pGNSI)
		pGNSI(&si);
	else GetSystemInfo(&si);
    
	if(si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA64  
		|| si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_AMD64
		|| si.wProcessorArchitecture==PROCESSOR_ARCHITECTURE_IA32_ON_WIN64)
	{
       pathname = "WinRing0x64.sys";
	}
	else
	{
       pathname = "WinRing0.sys";
	}

	 if (!GetModuleFileNameA(GetModuleHandle(NULL), szWinIoDriverPath, sizeof(szWinIoDriverPath)))
         return false;
	  pszSlash = strrchr(szWinIoDriverPath, '\\');

    if (pszSlash)
       pszSlash[1] = 0;
    else
      return false;
   char temppath[ MAX_PATH];

   DWORD dwRetVal = GetTempPathA(sizeof(temppath),temppath);
   if (dwRetVal > MAX_PATH || (dwRetVal == 0))
   {
       return false;
   }


   char tempname[ MAX_PATH];
   GetTempFileNameA(temppath,"demo",0,tempname);
   CFile driverfile,tempFile;
   pathname=szWinIoDriverPath+pathname;
   driverfile.Open(pathname,CFile::modeRead);
   CStringA TmpNameA = tempname;
	CString TmpNameW = TmpNameA;
	tempFile.Open(TmpNameW.GetBuffer(),CFile::modeWrite);
   char databuf[4096];// one cluster
   UINT ret;
   while(ret =driverfile.Read(databuf,4096))
   {
	   tempFile.Write(databuf,ret);
   }
   driverfile.Close();
   tempFile.Close();
   strcpy_s(szWinIoDriverPath, tempname);
   return true;
}

bool Ring0::Open(void)
{
	// No implementation for Unix systems
	  
    if(driver!=INVALID_HANDLE_VALUE)
		return false;

	CStringA drivername = "\\\\.\\WinRing0_1_2_0";
	driver = CreateFileA(drivername.GetBuffer(),GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);

    if(driver==INVALID_HANDLE_VALUE)
	{
	   if(!ExtractDriver())
	   {
		   CStringA str;
		   str.Format("释放驱动文件失败");
		   MessageBoxA(NULL, str.GetBuffer(), "警告", MB_OKCANCEL | MB_ICONWARNING);
		   //AfxMessageBox(str);
		   return false;
	   }
	   if(_access(szWinIoDriverPath,0))
	   {
		   CStringA str;
		   str.Format("找不到文件%s",szWinIoDriverPath);
		   MessageBoxA(NULL, str.GetBuffer(), "警告", MB_OKCANCEL | MB_ICONWARNING);
           return false;
	   }

	   if (InstallWinIoDriver(szWinIoDriverPath,true))
	   {
		  if(!StartWinIoDriver())
              return false;
	      driver = CreateFileA(drivername,GENERIC_READ|GENERIC_WRITE,0,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
          if(INVALID_HANDLE_VALUE==driver)
		      return false;
		  
	   }
	   else
	   {
		   //AfxMessageBox("InstallWinIODriver failed");
		   return false;
	   }

	}
    isMutex = new CMutex(false,L"Access_ISABUS.HTP.Method");
	return true;
}

void Ring0::Close()
{
	 if (driver == INVALID_HANDLE_VALUE)
        return;

      uint refCount = 0;
	  DWORD dwByteReturned;
      DeviceIoControl(driver,IOCTL_OLS_GET_REFCOUNT, NULL,0, 
		               &refCount,sizeof(refCount), &dwByteReturned, NULL);

     StopWinIoDriver(); 
     delete isMutex; 
}

bool Ring0::ReleaseIsaBusMutex()
{
    if(isMutex->Unlock())
	   return true;
	return false;
}

bool Ring0::WaitIsaBusMutex(int millisecondsTimeout)
{
	return isMutex->Lock(millisecondsTimeout)?true:false;
}
bool Ring0::Rdmsr(uint index,  uint &eax, uint &edx)
{
	if(driver==INVALID_HANDLE_VALUE)
	{    
		eax = 0;
        edx = 0;
		return false;
	}
	  UINT64  buffer = 0;
	  DWORD dwByteReturned;

      BOOL result = DeviceIoControl(driver,IOCTL_OLS_READ_MSR, &index,sizeof(index),
        &buffer,sizeof(buffer),&dwByteReturned,NULL);

      edx = (UINT)((buffer >> 32) & 0xFFFFFFFF);
      eax = (UINT)(buffer & 0xFFFFFFFF);
      return result?true:false;
}



bool Ring0::Wrmsr(UINT index ,UINT eax,UINT edx)   
{
	if(driver==INVALID_HANDLE_VALUE)
	{    
		eax = 0;
        edx = 0;
		return false;
	}
	UINT buf[3];
    buf[0] = index;
	buf[1] = eax;
	buf[2] = edx;
	DWORD dwByteReturned;

      BOOL result = DeviceIoControl(driver,IOCTL_OLS_WRITE_MSR, buf,sizeof(buf),
        NULL,0,&dwByteReturned,NULL);
      return result;
}


bool Ring0::ReadPciConfig(UINT pciAddress,UINT regAddress,UINT &value)
{
	if(driver == INVALID_HANDLE_VALUE || (regAddress&3)!=0)
	{
		value = 0;
		return false;
	}
	UINT buf[2];
	buf[0] = pciAddress;
	buf[1] = regAddress;
	DWORD dwByteReturned;
    bool result = DeviceIoControl(driver,IOCTL_OLS_READ_PCI_CONFIG, buf,sizeof(buf),
        &value,sizeof(value),&dwByteReturned,NULL);
	return result;
}

bool Ring0::WritePciConfig(UINT pciAddress,UINT regAddress, UINT value)
{
   if(driver == INVALID_HANDLE_VALUE || (regAddress&3)!=0)
	{
		return false;
	}
	UINT buf[2];
	buf[0] = pciAddress;
	buf[1] = regAddress;
	DWORD dwByteReturned;
    bool result = DeviceIoControl(driver,IOCTL_OLS_WRITE_PCI_CONFIG, buf,sizeof(buf),
       NULL,0,&dwByteReturned,NULL);
	return result;
}

void Ring0::WriteIoPort(uint port, byte value){
      if (driver == INVALID_HANDLE_VALUE)
           return;
#pragma  pack(1)
	   typedef struct 
	   {
         uint PortNumber;
		 byte value; 
	   } INPUT_VALUE;
#pragma 
	    INPUT_VALUE input;
        input.PortNumber = port;
        input.value = value;
		DWORD dwByteReturned;
       bool result = DeviceIoControl(driver,IOCTL_OLS_WRITE_IO_PORT_BYTE, &input,sizeof(input),NULL,0,&dwByteReturned,NULL);
	   DWORD derr = GetLastError();
	   ;
       return;    
}

byte Ring0::ReadIoPort(uint port)
{
	 if (driver == INVALID_HANDLE_VALUE)
          return 0;
      uint value = 0;
	  DWORD dwByteReturned;
      DeviceIoControl(driver,IOCTL_OLS_READ_IO_PORT_BYTE, &port,sizeof(port), &value,sizeof(value),&dwByteReturned,NULL);
      return (byte)(value & 0xFF);
}