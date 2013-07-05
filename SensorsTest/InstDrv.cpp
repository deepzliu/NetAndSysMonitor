// ---------------------------------------------------- //
//                      WinIo v2.0                      //
//  Direct Hardware Access Under Windows 9x/NT/2000/XP  //
//           Copyright 1998-2002 Yariv Kaplan           //
//               http://www.internals.com               //
// ---------------------------------------------------- //
#include "StdAfx.h"
#include <windows.h>
#include <winsvc.h>
bool _stdcall RemoveWinIoDriver();
bool _stdcall StartWinIoDriver();
bool _stdcall StopWinIoDriver();




bool _stdcall InstallWinIoDriver(LPCSTR pszWinIoDriverPath, bool IsDemandLoaded)
{
  SC_HANDLE hSCManager;
  SC_HANDLE hService;

  // Remove any previous instance of the driver

  RemoveWinIoDriver();

  hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

  if (hSCManager)
  {
    // Install the driver
    
    hService = CreateService(hSCManager,
                             "WinRing0_1_2_0",
                             "WinRing0_1_2_0",
                             SERVICE_ALL_ACCESS,
                             SERVICE_KERNEL_DRIVER,
                             (IsDemandLoaded == true) ? SERVICE_DEMAND_START : SERVICE_SYSTEM_START,
                             SERVICE_ERROR_NORMAL,
                             pszWinIoDriverPath,
                             NULL,
                             NULL,
                             NULL,
                             NULL,
                             NULL);

    CloseServiceHandle(hSCManager);

    if (hService == NULL)
      return false;
  }
  else
    return false;

  CloseServiceHandle(hService);
  
  return true;
}


bool _stdcall RemoveWinIoDriver()
{
  SC_HANDLE hSCManager;
  SC_HANDLE hService;
  bool bResult;

  StopWinIoDriver();

  hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

  if (hSCManager)
  {
    hService = OpenService(hSCManager, "WinRing0_1_2_0", SERVICE_ALL_ACCESS);

    CloseServiceHandle(hSCManager);

    if (hService)
    {
      bResult = DeleteService(hService);
      
      CloseServiceHandle(hService);
    }
    else
      return false;
  }
  else
    return false;

  return bResult;
}


bool _stdcall StartWinIoDriver()
{
  SC_HANDLE hSCManager;
  SC_HANDLE hService;
  bool bResult;

  hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
  if (hSCManager)
  {
    hService = OpenService(hSCManager, "WinRing0_1_2_0", SERVICE_ALL_ACCESS);
    CloseServiceHandle(hSCManager); //liudonghua

    if (hService)
    {
      bResult = StartService(hService, 0, NULL); 

	 if(GetLastError() == ERROR_SERVICE_ALREADY_RUNNING)
	 {
		  bResult =true;
	  }
	 
      CloseServiceHandle(hService);
    }
    else{
      return false;
	}
  }
  else{	
    return false;
  }

  return bResult;
}


bool _stdcall StopWinIoDriver()
{
  SC_HANDLE hSCManager;
  SC_HANDLE hService;
  SERVICE_STATUS ServiceStatus;
  BOOL bResult;

  hSCManager = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);

  if (hSCManager)
  {
    hService = OpenService(hSCManager, "WinRing0_1_2_0", SERVICE_ALL_ACCESS);

    CloseServiceHandle(hSCManager);

    if (hService)
    {
      bResult = ControlService(hService, SERVICE_CONTROL_STOP, &ServiceStatus);
      CloseServiceHandle(hService);
    }
    else
      return false;
  }
  else
    return false;

  return bResult;
}


