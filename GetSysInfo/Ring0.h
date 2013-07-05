/********************************************************************
	created:	2010/11/11
	created:	11:11:2010   11:00
	filename: 	I:\”≤º˛≈‰÷√…®√Ë2010.9.15\”≤º˛≈‰÷√…®√Ë2010.11.5\HardwareScanXML\Ring0.h
	file path:	I:\”≤º˛≈‰÷√…®√Ë2010.9.15\”≤º˛≈‰÷√…®√Ë2010.11.5\HardwareScanXML
	file base:	Ring0
	file ext:	h
	author:		zhao
	
	purpose:	 π”√ƒ⁄∫À«˝∂Ø≤Ÿ◊˜…Ë±∏∂Àø⁄
*********************************************************************/
// InstDrv
#include <afxmt.h>
#include <winioctl.h>


typedef UINT uint;

bool _stdcall InstallWinIoDriver(char* pszWinIoDriverPath, bool IsDemandLoaded);
bool _stdcall RemoveWinIoDriver();
bool _stdcall StartWinIoDriver();
bool _stdcall StopWinIoDriver();

//

      const UINT OLS_TYPE = 40000;
      const UINT IOCTL_OLS_GET_REFCOUNT = CTL_CODE(OLS_TYPE, 0x801,METHOD_BUFFERED,FILE_ANY_ACCESS);
      const UINT IOCTL_OLS_GET_DRIVER_VERSION =  CTL_CODE(OLS_TYPE, 0x800,METHOD_BUFFERED,FILE_ANY_ACCESS);
      const UINT IOCTL_OLS_READ_MSR = CTL_CODE(OLS_TYPE, 0x821,METHOD_BUFFERED,FILE_ANY_ACCESS);
      const UINT IOCTL_OLS_WRITE_MSR =CTL_CODE(OLS_TYPE, 0x822,METHOD_BUFFERED,FILE_ANY_ACCESS); 
      const UINT IOCTL_OLS_READ_IO_PORT_BYTE =CTL_CODE(OLS_TYPE, 0x833,METHOD_BUFFERED,FILE_READ_ACCESS);
      const UINT IOCTL_OLS_WRITE_IO_PORT_BYTE = CTL_CODE(OLS_TYPE, 0x836,METHOD_BUFFERED,FILE_WRITE_ACCESS);
      const UINT IOCTL_OLS_READ_PCI_CONFIG = CTL_CODE(OLS_TYPE, 0x851,METHOD_BUFFERED,FILE_READ_ACCESS);
      const UINT IOCTL_OLS_WRITE_PCI_CONFIG =CTL_CODE(OLS_TYPE, 0x852,METHOD_BUFFERED,FILE_WRITE_ACCESS);
      const uint InvalidPciAddress = 0xFFFFFFFF;

 class Ring0 
 {

 private:
	  HANDLE  driver;
	  CMutex *isMutex;
      //Mutex  isaBusMutex;
	  CString  pathname;
      bool ExtractDriver(void);
 public:
	  Ring0(void);
	  ~Ring0(void);
      bool Open(void);
      void Close();
	  bool WaitIsaBusMutex(int millisecondsTimeOUT);
      bool ReleaseIsaBusMutex();
      bool Rdmsr(uint index, OUT uint &eax, OUT uint &edx);
      bool Wrmsr(uint index, uint eax, uint edx);
      byte ReadIoPort(uint port);
	  bool IsOpen(void)
	  {
		  if(driver!=INVALID_HANDLE_VALUE)
			  return true;
		  return false;
	  }

    void WriteIoPort(uint port, byte value);
   
   uint GetPciAddress(byte bus, byte device, byte function) {
      return (uint)(((bus & 0xFF) << 8) | ((device & 0x1F) << 3) | (function & 7));
    };

    bool ReadPciConfig(uint pciAddress, uint regAddress, uint &value);
    bool WritePciConfig(uint pciAddress, uint regAddress, uint value); 
 };