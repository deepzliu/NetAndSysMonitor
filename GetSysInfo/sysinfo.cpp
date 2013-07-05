
#include "SmartReader.h"
#include "./include/cpuinfolib/CPUBasicInfo.h"
#include "./include/cpuinfolib/libcpuinfo_funclist.h"
#include "lpc/LPCIO.h"
#include "Sensors.h"
#include "sysinfo.h"
#include "EventLog.h"

#include <iostream>
#include <stdio.h>
#include <conio.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <Wbemidl.h>
#include <Iphlpapi.h>
#include <Wininet.h>
#include <PowrProf.h>
#include <SetupAPI.h>
#include <Batclass.h>
//#include <Poclass.h>
//#include <windows.h>

#pragma comment(lib, "wbemuuid.lib") 
#pragma comment(lib, "pdh.lib")
#pragma comment(lib,"Iphlpapi.lib")
#pragma comment(lib,"Wininet.lib")
#pragma comment(lib, "PowrProf.lib")
#pragma comment(lib, "setupapi.lib")
using namespace std;

#define _WIN32_WINNT _WIN32_WINNT_MAXVER
#define SystemBasicInformation 0 
#define SystemPerformanceInformation 2 
#define SystemTimeInformation 3
#define SystemProcessorPerformanceInformation 8
 
DEFINE_GUID( GUID_DEVCLASS_BATTERY, 0x72631E54, 0x78A4, 0x11D0, 0xBC, 0xF7, 0x00, 0xAA, 0x00, 0xB7, 0xB3, 0x2A );
DEFINE_GUID( GUID_DEVCLASS_USB,  0x36FC9E60, 0xC465, 0x11CF, 0x80, 0x56, 0x44, 0x45, 0x53, 0x54, 0x00, 0x00 );

#define Li2Double(x) ((double)((x).HighPart) * 4.294967296E9 + (double)((x).LowPart))
#define OS_BITS (((int)((int *)0 + 1)) << 3) //判断操作系统位数

typedef struct 
{ 
 DWORD dwUnknown1; 
 ULONG uKeMaximumIncrement; 
 ULONG uPageSize; 
 ULONG uMmNumberOfPhysicalPages; 
 ULONG uMmLowestPhysicalPage; 
 ULONG uMmHighestPhysicalPage; 
 ULONG uAllocationGranularity; 
 PVOID pLowestUserAddress; 
 PVOID pMmHighestUserAddress; 
 ULONG uKeActiveProcessors; 
 BYTE bKeNumberProcessors; 
 BYTE bUnknown2; 
 WORD wUnknown3; 
} SYSTEM_BASIC_INFORMATION;
 
typedef struct 
{ 
 LARGE_INTEGER liIdleTime; 
 DWORD dwSpare[76]; 
} SYSTEM_PERFORMANCE_INFORMATION;
 
typedef struct 
{ 
 LARGE_INTEGER liKeBootTime; 
 LARGE_INTEGER liKeSystemTime; 
 LARGE_INTEGER liExpTimeZoneBias; 
 ULONG uCurrentTimeZoneId; 
 DWORD dwReserved; 
} SYSTEM_TIME_INFORMATION;
 
typedef struct
_SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION {
    LARGE_INTEGER IdleTime;
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER DpcTime;
    LARGE_INTEGER InterruptTime;
    ULONG Reserved2;
} SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION;
 
typedef struct
_m_PROCESSORS_USE_TIME {
    double dbOldIdleTime;	// save old total time
	double dbOldCurrentTime;
	double dbIdleTime;		// save time after calc
	double dbCurrentTime;
	float fUse;
}m_PROCESSORS_USE_TIME;

typedef struct _BasicCPUInfo{
	long MHz;
	int PlatformID;
	char Identifier[64];
	char VendorIdentifier[64];
	char NameString[128];
}BasicCPUInfo;


m_PROCESSORS_USE_TIME * m_PUT;
 
SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION * m_pSPPI = NULL;
 
int m_iNumberProcessors;
 
typedef LONG (WINAPI *PROCNTQSI)(UINT,PVOID,ULONG,PULONG);
 
PROCNTQSI NtQuerySystemInformation;

static LARGE_INTEGER liOldIdleTime = {0,0}; 
static LARGE_INTEGER liOldSystemTime = {0,0};
 
double dbIdleTime = 0; 
double dbSystemTime = 0; 
double alldbIdleTime = 0;

char *w2c(char *pcstr,const wchar_t *pwstr, size_t len){
	int nlength=wcslen(pwstr);

	//获取转换后的长度
	int nbytes = WideCharToMultiByte( 0, // specify the code page used to perform the conversion
					0,         // no special flags to handle unmapped characters
					pwstr,     // wide character string to convert
					nlength,   // the number of wide characters in that string
					NULL,      // no output buffer given, we just want to know how long it needs to be
					0,
					NULL,      // no replacement character given
					NULL );    // we don't want to know if a character didn't make it through the translation

	// make sure the buffer is big enough for this, making it larger if necessary
	if(nbytes>len)   nbytes=len;
	//if(nbytes < len)   nbytes=len; //liudonghua

	// 通过以上得到的结果，转换unicode 字符为ascii 字符
	WideCharToMultiByte( 0, // specify the code page used to perform the conversion
					0,         // no special flags to handle unmapped characters
					pwstr,   // wide character string to convert
					nlength,   // the number of wide characters in that string
					pcstr, // put the output ascii characters at the end of the buffer
					nbytes, // there is at least this much space there
					NULL,      // no replacement character given
					NULL );
	pcstr[nbytes] = '\0';
	//pcstr[nbytes+1] = '\0';
	return pcstr ;

}

int GetBasicCPUInfo(int cpunum, BasicCPUInfo *bCPUInfo){
	wchar_t reg[] = L"HARDWARE\\DESCRIPTION\\System\\CentralProcessor";
	wchar_t regkey[256];
	int ret = 0;
	HKEY hKey;//定义有关的hKEY,在查询结束时要关闭
	unsigned char data[256];
	wchar_t tmpbuf[256];
	DWORD dataLen = 256;
	DWORD dataType = REG_SZ;

	for(int i = 0; i < cpunum; i++){
		swprintf(regkey, L"%s\\%d", reg, i);
		ret = RegOpenKeyEx(HKEY_LOCAL_MACHINE, regkey, 0, KEY_READ, &hKey);
		if(ret != ERROR_SUCCESS){//如果无法打开hKEY,则中止程序的执行
			printf("Regedit Open Key Failed\n");
			return -1;
		}

		dataType = REG_DWORD;
		ret = RegQueryValueEx(hKey, L"~MHz", NULL, &dataType, (LPBYTE)&bCPUInfo[i].MHz, &dataLen);

		dataLen = 256 * 2;
		dataType = REG_SZ;
		ret = RegQueryValueEx(hKey, L"Identifier", NULL, &dataType, (LPBYTE)tmpbuf, &dataLen);
		if(ret == ERROR_SUCCESS){
			w2c(bCPUInfo[i].Identifier, tmpbuf, dataLen);
		}

		dataLen = 256;
		dataType = REG_DWORD;
		ret = RegQueryValueEx(hKey, L"Platform ID", NULL, &dataType, (LPBYTE)&bCPUInfo[i].PlatformID, &dataLen);

		dataLen = 256 * 2;
		dataType = REG_SZ;
		ret = RegQueryValueEx(hKey, L"ProcessorNameString", NULL, &dataType, (LPBYTE)tmpbuf, &dataLen);
		if(ret == ERROR_SUCCESS){
			w2c(bCPUInfo[i].NameString, tmpbuf, dataLen);
		}

		dataLen = 256;
		dataType = REG_SZ;
		ret = RegQueryValueEx(hKey, L"VendorIdentifier", NULL, &dataType, (LPBYTE)tmpbuf, &dataLen);
		if(ret == ERROR_SUCCESS){
			w2c(bCPUInfo[i].VendorIdentifier, tmpbuf, dataLen);
		}
	}
	return 0;
}
int GetCPUInfo(){
	int cpunum = 0;
	double cpuusage = 0;
	BasicCPUInfo *bCPUInfo;
	SYSTEM_INFO lSysInfo;
	GetSystemInfo(&lSysInfo);
	bCPUInfo = new BasicCPUInfo[lSysInfo.dwNumberOfProcessors];

	//printf("Arc: %d; Process: num [%d], type [%d], level [%d];\n", lSysInfo.wProcessorArchitecture, 
	//	lSysInfo.dwNumberOfProcessors, lSysInfo.dwProcessorType, lSysInfo.wProcessorLevel);

	GetBasicCPUInfo(lSysInfo.dwNumberOfProcessors, bCPUInfo);
	for(int i = 0; i < lSysInfo.dwNumberOfProcessors; i++){
		printf("(%s) CPU %d: %s\n", bCPUInfo[i].VendorIdentifier, i, bCPUInfo[i].NameString);
	}

	cpuusage = GetCPUUsage(&cpunum);
	Sleep(500);
	cpuusage = GetCPUUsage(&cpunum);
	printf("CPU Usage: %2.2f%%\n", cpuusage);

	delete[] bCPUInfo;
	return 0;
}

double GetCPUUsage(int *cpunum){
	SYSTEM_PERFORMANCE_INFORMATION SysPerfInfo; 
	SYSTEM_TIME_INFORMATION SysTimeInfo; 
	SYSTEM_BASIC_INFORMATION SysBaseInfo; 
 
	LONG status; 
 
	NtQuerySystemInformation = (PROCNTQSI)GetProcAddress(GetModuleHandle(L"ntdll"),"NtQuerySystemInformation");
	// get number of processors in the system 
	status = NtQuerySystemInformation(SystemBasicInformation,&SysBaseInfo,sizeof(SysBaseInfo),NULL); 
	if (status != NO_ERROR) 
		return -1;
 
	if (!NtQuerySystemInformation) 
		return -1;
 
	// get new system time 
	status = NtQuerySystemInformation(SystemTimeInformation,&SysTimeInfo,sizeof(SysTimeInfo),0); 
	if (status!=NO_ERROR) 
		return -1;
 

	// get new CPU's idle time 
	status =NtQuerySystemInformation(SystemPerformanceInformation,&SysPerfInfo,sizeof(SysPerfInfo),NULL); 
	if (status != NO_ERROR) 
		return -1;
 
	if ( m_iNumberProcessors != SysBaseInfo.bKeNumberProcessors)
	{
		//save
		m_iNumberProcessors = SysBaseInfo.bKeNumberProcessors;
		//if sppi not null clear
		if (m_pSPPI != NULL) delete []m_pSPPI;
		if (m_PUT != NULL) delete []m_PUT;
		//malloc and point
		m_pSPPI = new SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION[m_iNumberProcessors];
		m_PUT = new m_PROCESSORS_USE_TIME[m_iNumberProcessors];
	}
 
	// get ProcessorPer time 
	status =NtQuerySystemInformation(SystemProcessorPerformanceInformation, m_pSPPI, sizeof(SYSTEM_PROCESSOR_PERFORMANCE_INFORMATION) * m_iNumberProcessors, NULL); 
	if (status != NO_ERROR) 
		return -1;
 
	// if it's a first call - skip it 
	if (liOldIdleTime.QuadPart != 0) 
	{ 
		// CurrentValue = NewValue - OldValue 
		dbIdleTime = Li2Double(SysPerfInfo.liIdleTime) - Li2Double(liOldIdleTime); 
 
		dbSystemTime = Li2Double(SysTimeInfo.liKeSystemTime) - Li2Double(liOldSystemTime);
 
		// CurrentCpuIdle = IdleTime / SystemTime 
		dbIdleTime = dbIdleTime / dbSystemTime;
 
		// CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors 
		dbIdleTime = 100.0 - dbIdleTime * 100.0 / (double)SysBaseInfo.bKeNumberProcessors + 0.5;
 
		//calc Processors
		for (int i = 0; i < m_iNumberProcessors; i++)
		{
			m_PUT[i].dbCurrentTime = Li2Double(m_pSPPI[i].KernelTime) + Li2Double(m_pSPPI[i].UserTime) + 
										Li2Double(m_pSPPI[i].DpcTime) + Li2Double(m_pSPPI[i].InterruptTime) - m_PUT[i].dbOldCurrentTime;
			m_PUT[i].dbIdleTime = Li2Double(m_pSPPI[i].IdleTime) - m_PUT[i].dbOldIdleTime;
 
			// CurrentCpuIdle = IdleTime / SystemTime 
			m_PUT[i].dbIdleTime = m_PUT[i].dbIdleTime / m_PUT[i].dbCurrentTime;
 
			// CurrentCpuUsage% = 100 - (CurrentCpuIdle * 100) / NumberOfProcessors 
			m_PUT[i].dbIdleTime = 100.0 - m_PUT[i].dbIdleTime * 100.0 + 0.5;
 
		}
	}
 
	// store new CPU's idle and system time 
	liOldIdleTime = SysPerfInfo.liIdleTime; 
	liOldSystemTime = SysTimeInfo.liKeSystemTime;
 
	for (int i = 0; i < m_iNumberProcessors; i++)
	{
		m_PUT[i].dbOldCurrentTime = Li2Double(m_pSPPI[i].KernelTime) + Li2Double(m_pSPPI[i].UserTime) + 
										Li2Double(m_pSPPI[i].DpcTime) + Li2Double(m_pSPPI[i].InterruptTime);
 
		m_PUT[i].dbOldIdleTime = Li2Double(m_pSPPI[i].IdleTime);
	}
	
	*cpunum = SysBaseInfo.bKeNumberProcessors;
	//printf("Number of CPUs: %d, CPU: %2.2f%%\n", SysBaseInfo.bKeNumberProcessors, dbIdleTime);

	return dbIdleTime;
}

int GetOSVersion()
{
	OSVERSIONINFO osvi;
	memset(&osvi, 0, sizeof(OSVERSIONINFO));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
	GetVersionEx(&osvi);



	wprintf(L"OS Version: %d %s, Type: %d bit\n", osvi.dwBuildNumber, osvi.szCSDVersion, OS_BITS);

	return 0;
}

int GetMemoryInfo()
{
	MEMORYSTATUSEX memstatus;
	memstatus.dwLength   =sizeof(MEMORYSTATUSEX);   
	GlobalMemoryStatusEx(&memstatus);

	printf("Total Physical MM: %lld MB, Physical MM Usage: %lld MB\n",
		memstatus.ullTotalPhys / 1024 / 1024, 
		(memstatus.ullTotalPhys - memstatus.ullAvailPhys) / 1024 / 1024);
	//printf("MM Usage Rate: %2ld%%\n", memstatus.dwMemoryLoad);
	printf("MM Usage Rate: %4.2lf%%\n", (double)(memstatus.ullTotalPhys - memstatus.ullAvailPhys) / memstatus.ullTotalPhys * 100);

	return 0;
}

int GetDiskInfo()
{
	unsigned long nVolumesMask = GetLogicalDrives();
	unsigned long mask = nVolumesMask;
	int nVolumes = 0;
	long len = 64;
	char buf[64];

	while(mask){
		if(mask & 1) nVolumes++;
		mask >>= 1;
	}
	
	unsigned __int64 i64FreeBytesToCaller;
    unsigned __int64 i64TotalBytes;
    unsigned __int64 i64FreeBytes;

	printf("Disk Partions: %d\n", nVolumes);
	GetLogicalDriveStringsA(len, buf);
	for(int i = 0; i < nVolumes; i++){		
		GetDiskFreeSpaceExA(
			buf+i*4,
			(PULARGE_INTEGER)&i64FreeBytesToCaller,
			(PULARGE_INTEGER)&i64TotalBytes,
			(PULARGE_INTEGER)&i64FreeBytes);

		printf("%s, Type: %d, Total: %6.2lf GB, Free: %6.2lf GB\n",//, Free To User: %6.2lf GB
			buf+i*4, 
			GetDriveTypeA(buf+i*4),
			(double)i64TotalBytes / 1024 / 1024 / 1024,
			(double)i64FreeBytesToCaller / 1024 / 1024 / 1024);
	}

	GetDiskPerformance();

	//Win32_PerfFormattedData_PerfDisk_LogicalDisk wppl;

	return 0;
}

int GetDiskPerformance()
{
	//////////////////
	// initail
	/////////////////
	PDH_STATUS Status;
	HQUERY Query1 = NULL, Query2 = NULL, Query3 = NULL;
	HCOUNTER Counter1, Counter2, Counter3;
	PDH_FMT_COUNTERVALUE DisplayValue1, DisplayValue2, DisplayValue3;
	DWORD CounterType1, CounterType2, CounterType3;
	char CounterPathBuffer1[] = "\\PhysicalDisk(*)\\Disk Transfers/sec";
	char CounterPathBuffer2[] = "\\PhysicalDisk(*)\\Disk Reads/sec";
	char CounterPathBuffer3[] = "\\PhysicalDisk(*)\\Disk Writes/sec";
	
 
	//创建总IO流量计数器
	// Create a query.
	Status = PdhOpenQuery(NULL, NULL, &Query1);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhOpenQuery1 failed with status 0x%x.", Status);
		goto Cleanup;
	}
	// Add the selected counter to the query.
	Status = PdhAddCounterA(Query1, CounterPathBuffer1, 0, &Counter1);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhAddCounter1 failed with status 0x%x.", Status);
		goto Cleanup;
	}
 
	Status = PdhCollectQueryData(Query1);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhCollectQueryData1 failed with 0x%x.\n", Status);
		goto Cleanup;
	}
	
	//创建读IO流量计数器
	Status = PdhOpenQuery(NULL, NULL, &Query2);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhOpenQuery2 failed with status 0x%x.", Status);
		goto Cleanup;
	}
	// Add the selected counter to the query.
	Status = PdhAddCounterA(Query2, CounterPathBuffer2, 0, &Counter2);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhAddCounter2 failed with status 0x%x.", Status);
		goto Cleanup;
	}
 
	Status = PdhCollectQueryData(Query2);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhCollectQueryData2 failed with 0x%x.\n", Status);
		goto Cleanup;
	}

	//创建写IO流量计数器
	Status = PdhOpenQuery(NULL, NULL, &Query3);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhOpenQuery3 failed with status 0x%x.", Status);
		goto Cleanup;
	}
	// Add the selected counter to the query.
	Status = PdhAddCounterA(Query3, CounterPathBuffer3, 0, &Counter3);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhAddCounter3 failed with status 0x%x.", Status);
		goto Cleanup;
	}
 
	Status = PdhCollectQueryData(Query3);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhCollectQueryData3 failed with 0x%x.\n", Status);
		goto Cleanup;
	}


	// 获取计数数据， 循环10次， 间隔1秒
	int n = 5;
	while ( n-- > 0) 
	{
		Sleep(1000);
		
		//总IO流量
		Status = PdhCollectQueryData(Query1);
		if (Status != ERROR_SUCCESS) 
		{
			wprintf(L"\nPdhCollectQueryData1 failed with status 0x%x.", Status);
		} 
		Status = PdhGetFormattedCounterValue(Counter1,
			PDH_FMT_DOUBLE,//PDH_FMT_DOUBLE
			&CounterType1,
			&DisplayValue1);
		if (Status != ERROR_SUCCESS) 
		{
			wprintf(L"\nPdhGetFormattedCounterValue1 failed with status 0x%x.", Status);
			//goto Cleanup;
		}
 
		//读IO流量
		Status = PdhCollectQueryData(Query2);
		if (Status != ERROR_SUCCESS) 
		{
			wprintf(L"\nPdhCollectQueryData2 failed with status 0x%x.", Status);
		} 
		Status = PdhGetFormattedCounterValue(Counter2,
			PDH_FMT_DOUBLE,//PDH_FMT_DOUBLE
			&CounterType2,
			&DisplayValue2);
		if (Status != ERROR_SUCCESS) 
		{
			wprintf(L"\nPdhGetFormattedCounterValue2 failed with status 0x%x.", Status);
			//goto Cleanup;
		}

		//写IO流量
		Status = PdhCollectQueryData(Query3);
		if (Status != ERROR_SUCCESS) 
		{
			wprintf(L"\nPdhCollectQueryData3 failed with status 0x%x.", Status);
		} 
		Status = PdhGetFormattedCounterValue(Counter3,
			PDH_FMT_DOUBLE,//PDH_FMT_DOUBLE
			&CounterType3,
			&DisplayValue3);
		if (Status != ERROR_SUCCESS) 
		{
			wprintf(L"\nPdhGetFormattedCounterValue3 failed with status 0x%x.", Status);
			//goto Cleanup;
		}

		//输出
		wprintf(L"Total: %6.2f MB/s, Read: %6.2f MB/s, Write: %6.2f MB/s\n", 
			DisplayValue1.doubleValue,
			DisplayValue2.doubleValue,
			DisplayValue3.doubleValue);
	}


Cleanup:
	if (Query1) 
	{
		PdhCloseQuery(Query1);
	}
	if (Query2) 
	{
		PdhCloseQuery(Query2);
	}
	if (Query3) 
	{
		PdhCloseQuery(Query3);
	}
	return -1;
}

int GetNetInfo()
{
    PIP_ADAPTER_INFO pIpAdapterInfo = new IP_ADAPTER_INFO();
    unsigned long stSize = sizeof(IP_ADAPTER_INFO);
    int nRel = GetAdaptersInfo(pIpAdapterInfo,&stSize), ret = 0;
    int netCardNum = 0;
    int IPnumPerNetCard = 0;
    if (ERROR_BUFFER_OVERFLOW == nRel)
    {
        delete pIpAdapterInfo;
        pIpAdapterInfo = (PIP_ADAPTER_INFO)new BYTE[stSize];
        nRel=GetAdaptersInfo(pIpAdapterInfo,&stSize);    
    }
    if (ERROR_SUCCESS == nRel)
    {
    while (pIpAdapterInfo)
    {
        cout<<"Network Adapter "<<++netCardNum<< ":" << endl;
        cout<<"\tDesc："<<pIpAdapterInfo->Description<<endl;
		cout<<"\tType: ";
        switch(pIpAdapterInfo->Type)
        {
        case MIB_IF_TYPE_OTHER:
            cout<<"OTHER"<<endl;
            break;
        case MIB_IF_TYPE_ETHERNET:
            cout<<"ETHERNET"<<endl;
            break;
        case MIB_IF_TYPE_TOKENRING:
            cout<<"TOKENRING"<<endl;
            break;
        case MIB_IF_TYPE_FDDI:
            cout<<"FDDI"<<endl;
            break;
        case MIB_IF_TYPE_PPP:
            //printf("PP\n");
            cout<<"PPP"<<endl;
            break;
        case MIB_IF_TYPE_LOOPBACK:
            cout<<"LOOPBACK"<<endl;
            break;
        case MIB_IF_TYPE_SLIP:
            cout<<"SLIP"<<endl;
            break;
        default:
            break;
        }
        cout<<"\tMAC：";
        for (DWORD i = 0; i < pIpAdapterInfo->AddressLength; i++){
            if (i < pIpAdapterInfo->AddressLength-1){
                printf("%02X-", pIpAdapterInfo->Address[i]);
            }else{
                printf("%02X\n", pIpAdapterInfo->Address[i]);
            }
		}
        //可能网卡有多IP,因此通过循环去判断
		int ips = 0;
        IP_ADDR_STRING *pIpAddrString =&(pIpAdapterInfo->IpAddressList);
        do{
            cout<<"\tIP "<< ++ips << "："<<pIpAddrString->IpAddress.String<<endl;
			//pIpAdapterInfo->GatewayList.IpAddress.String; //网关
			pIpAddrString=pIpAddrString->Next;
        } while (pIpAddrString);

		GetNetPerformance(pIpAdapterInfo->Description);

        pIpAdapterInfo = pIpAdapterInfo->Next;
        cout<<endl;
    }
    
    }
    //释放内存空间
    if (pIpAdapterInfo)
    {
        delete pIpAdapterInfo;
    }
 
    return 0;
}

//能ping通网关说明LAN是连接
int GetConnectedState(char *gw)
{
	if(gw == NULL){
		return -1;
	}
	BOOL ret = InternetCheckConnectionA(gw, FLAG_ICC_FORCE_CONNECTION, 0);
	if(ret){
		ret = GetLastError();
		return 0;
	}else{
		ret = GetLastError();
		return 1;
	}
	
}

int ping(char *ip){
	/*
	char szDestip[]="10.15.72.27"; 
	SOCKET sRaw=::socket(AF_INET,SOCK_RAW,IPPROTO_ICMP); 
	SetTimeOut(sRaw,1000,TRUE);  
	SOCKADDR_IN dest;
	dest.sin_family=AF_INET;
	dest.sin_port=htons(0);
	dest.sin_addr.S_un.S_addr=inet_addr(szDestip);
	char buff[sizeof(ICMP_HDR)+32]; 
	ICMP_HDR* pIcmp=(ICMP_HDR*)buff;
	pIcmp->icmp_type=8;  pIcmp->icmp_code=0;
	pIcmp->icmp_id=(USHORT)::GetCurrentProcessId();
	pIcmp->icmp_checksum=0;
	pIcmp->icmp_sequence=0;
	memset(&buff[sizeof(ICMP_HDR)],'E',32);
	USHORT nSeq=0;  char recvBuf[1024]={0};
	SOCKADDR_IN from;
	int nLen=sizeof(from);
	while(TRUE){
		static int nCount=0;
		int nRet;
		if (nCount++==4){ 
			break;
		}
		pIcmp->icmp_checksum=0;
		pIcmp->icmp_timestamp=::GetTickCount();
		pIcmp->icmp_sequence=nSeq++; 
		pIcmp->icmp_checksum=checksum((USHORT*)buff,sizeof(ICMP_HDR)+32);
		nRet=::sendto(sRaw,buff,sizeof(ICMP_HDR)+32,0,(SOCKADDR*)&dest,sizeof(dest));
		if (nRet==SOCKET_ERROR){
			printf("sendto()failed:%d\n",::WSAGetLastError());
			return;
		}
		nRet=::recvfrom(sRaw,recvBuf,1024,0,(sockaddr*)&from,&nLen);
		if (nRet==SOCKET_ERROR){
			if (::WSAGetLastError()==WSAETIMEDOUT){
				printf("time out\n");
				continue;
			}
			printf("recvform()failed:%d\n",::WSAGetLastError());
			return;
		}
		int nTick=::GetTickCount();
		if (nRet<sizeof(IPHeader)+sizeof(ICMP_HDR)){
			printf("Too few bytes from%s\n",::inet_ntoa(from.sin_addr));
		}
		ICMP_HDR* pRecvIcmp=(ICMP_HDR*)(recvBuf+sizeof(IPHeader));
		if(pRecvIcmp->icmp_type!=0){
			printf("nonecho type%d recvd\n",pRecvIcmp->icmp_type);
			return;
		}
		if(pRecvIcmp->icmp_id!=::GetCurrentProcessId()){
			printf("someone else's packet!");
			return;
		}
		printf("%d bytes from %s:",nRet,inet_ntoa(from.sin_addr));
		printf("icmp_seq=%d",pRecvIcmp->icmp_sequence);
		printf("time:%d ms",nTick-pRecvIcmp->icmp_timestamp);
		printf("\n");
		::Sleep(1000);
	}
	*/
	return 0;
}

int GetNetPerformance(char *pAdapterDesc)
{
	if(pAdapterDesc == NULL){
		printf("Network adapter description can't be null\n");
		return -1;
	}
	//////////////////
	// initail
	/////////////////
	PDH_STATUS Status;
	HQUERY Query1 = NULL, Query2 = NULL, Query3 = NULL;
	HCOUNTER Counter1, Counter2, Counter3;
	PDH_FMT_COUNTERVALUE DisplayValue1, DisplayValue2, DisplayValue3;
	DWORD CounterType1, CounterType2, CounterType3;
	char CounterPathBuffer1[256];
	char CounterPathBuffer2[256];
	char CounterPathBuffer3[256];

	sprintf(CounterPathBuffer1, "\\Network Interface(%s)\\Bytes Received/sec", pAdapterDesc);
	sprintf(CounterPathBuffer2, "\\Network Interface(%s)\\Bytes Sent/sec", pAdapterDesc);
	sprintf(CounterPathBuffer3, "\\Network Interface(%s)\\Bytes Total/sec", pAdapterDesc);

 
	//创建总网络流量计数器
	// Create a query.
	Status = PdhOpenQuery(NULL, NULL, &Query1);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhOpenQuery1 failed with status 0x%x.", Status);
		goto Cleanup;
	}
	// Add the selected counter to the query.
	Status = PdhAddCounterA(Query1, CounterPathBuffer1, 0, &Counter1);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhAddCounter1 failed with status 0x%x.", Status);
		goto Cleanup;
	}
 
	Status = PdhCollectQueryData(Query1);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhCollectQueryData1 failed with 0x%x.\n", Status);
		goto Cleanup;
	}
	
	//创建接收网络流量计数器
	Status = PdhOpenQuery(NULL, NULL, &Query2);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhOpenQuery2 failed with status 0x%x.", Status);
		goto Cleanup;
	}
	// Add the selected counter to the query.
	Status = PdhAddCounterA(Query2, CounterPathBuffer2, 0, &Counter2);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhAddCounter2 failed with status 0x%x.", Status);
		goto Cleanup;
	}
 
	Status = PdhCollectQueryData(Query2);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhCollectQueryData2 failed with 0x%x.\n", Status);
		goto Cleanup;
	}

	//创建发送网络流量计数器
	Status = PdhOpenQuery(NULL, NULL, &Query3);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhOpenQuery3 failed with status 0x%x.", Status);
		goto Cleanup;
	}
	// Add the selected counter to the query.
	Status = PdhAddCounterA(Query3, CounterPathBuffer3, 0, &Counter3);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhAddCounter3 failed with status 0x%x.", Status);
		goto Cleanup;
	}
 
	Status = PdhCollectQueryData(Query3);
	if (Status != ERROR_SUCCESS) 
	{
		wprintf(L"\nPdhCollectQueryData3 failed with 0x%x.\n", Status);
		goto Cleanup;
	}


	// 获取计数数据， 循环10次， 间隔1秒
	int n = 5;
	while (n-- > 0) 
	{
		Sleep(1000);
		
		//总网络流量
		Status = PdhCollectQueryData(Query1);
		if (Status != ERROR_SUCCESS) 
		{
			wprintf(L"\nPdhCollectQueryData1 failed with status 0x%x.", Status);
		} 
		Status = PdhGetFormattedCounterValue(Counter1,
			PDH_FMT_DOUBLE,//PDH_FMT_DOUBLE
			&CounterType1,
			&DisplayValue1);
		if (Status != ERROR_SUCCESS) 
		{
			wprintf(L"\nPdhGetFormattedCounterValue1 failed with status 0x%x.", Status);
			//goto Cleanup;
		}
 
		//接收网络流量
		Status = PdhCollectQueryData(Query2);
		if (Status != ERROR_SUCCESS) 
		{
			wprintf(L"\nPdhCollectQueryData2 failed with status 0x%x.", Status);
		} 
		Status = PdhGetFormattedCounterValue(Counter2,
			PDH_FMT_DOUBLE,//PDH_FMT_DOUBLE
			&CounterType2,
			&DisplayValue2);
		if (Status != ERROR_SUCCESS) 
		{
			wprintf(L"\nPdhGetFormattedCounterValue2 failed with status 0x%x.", Status);
			//goto Cleanup;
		}

		//发送网络流量
		Status = PdhCollectQueryData(Query3);
		if (Status != ERROR_SUCCESS) 
		{
			wprintf(L"\nPdhCollectQueryData3 failed with status 0x%x.", Status);
		} 
		Status = PdhGetFormattedCounterValue(Counter3,
			PDH_FMT_DOUBLE,//PDH_FMT_DOUBLE
			&CounterType3,
			&DisplayValue3);
		if (Status != ERROR_SUCCESS) 
		{
			wprintf(L"\nPdhGetFormattedCounterValue3 failed with status 0x%x.", Status);
			//goto Cleanup;
		}

		//输出
		wprintf(L"\tTotal: %6.2f KB/s, Received: %6.2f KB/s, Sent: %6.2f KB/s\n", 
			DisplayValue3.doubleValue / 1024,
			DisplayValue1.doubleValue / 1024,
			DisplayValue2.doubleValue / 1024);
	}


Cleanup:
	if (Query1) 
	{
		PdhCloseQuery(Query1);
	}
	if (Query2) 
	{
		PdhCloseQuery(Query2);
	}
	if (Query3) 
	{
		PdhCloseQuery(Query3);
	}
	return -1;
}

int PowerMng()
{
	// Define and initialize our return variables.
	LPWSTR  pRetBuf = NULL;
	ULONG bufSize = MAX_PATH * sizeof(WCHAR);
	ULONG uIndex = 0;
	BOOLEAN bRet = FALSE;
	char pBuf[512];

	// Open the device list, querying all devices
	if (!DevicePowerOpen(0)) 
	{
		printf("ERROR: The device database failed to initialize.\n");
		return FALSE;
	}

	// Enumerate the device list, searching for devices that support 
	// waking from either the S1 or S2 sleep state and are currently 
	// present in the system, and not devices that have drivers 
	// installed but are not currently attached to the system, such as 
	// in a laptop docking station.

	pRetBuf = (LPWSTR)LocalAlloc(LPTR, bufSize);

	while (NULL != pRetBuf && 
			0 != (bRet = DevicePowerEnumDevices(uIndex,
			DEVICEPOWER_FILTER_DEVICES_PRESENT,
			PDCAP_WAKE_FROM_S1_SUPPORTED|PDCAP_WAKE_FROM_S2_SUPPORTED,
			(PBYTE)pRetBuf,
			&bufSize)))
	{
		w2c(pBuf, pRetBuf, wcslen(pRetBuf)*2);
		printf("Device name: %s\n", pBuf);
		//printf(L"Device name: %S\n", pRetBuf);

		// For the devices we found that have support for waking from 
		// S1 and S2 sleep states, disable them from waking the system.
		bRet = (0 != DevicePowerSetDeviceState((LPCWSTR)pRetBuf, 
					/*DEVICEPOWER_SET_WAKEENABLED,*/
					DEVICEPOWER_CLEAR_WAKEENABLED, 
					NULL));

		if (0 != bRet) 
		{
			printf("Warning: Failed to set device state. [%d]\n", GetLastError());
			#if 0
			 LPVOID lpMsgBuf;
			 FormatMessage(
			 FORMAT_MESSAGE_ALLOCATE_BUFFER |
			 FORMAT_MESSAGE_FROM_SYSTEM |
			 FORMAT_MESSAGE_IGNORE_INSERTS,
			 NULL,
			 GetLastError(),
			 MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), // Default language
			 (LPTSTR) &lpMsgBuf,
			 0,
			 NULL
			 );
			 wprintf(L"Error: %s\n", lpMsgBuf);
			 LocalFree( lpMsgBuf );
			 #endif
		}
		uIndex++;
	}

	// Close the device list.
	DevicePowerClose();
	if (pRetBuf!= NULL) LocalFree(pRetBuf);
	pRetBuf = NULL;

	 GetBatteryState();

	return 0;
}

int GetPowerInfo()
{
	//long ret = CallNtPowerInformation(
	return 0;
}

unsigned long GetBatteryState()
 {
#define GBS_HASBATTERY 0x1
#define GBS_ONBATTERY  0x2
  // Returned value includes GBS_HASBATTERY if the system has a 
  // non-UPS battery, and GBS_ONBATTERY if the system is running on 
  // a battery.
  //
  // dwResult & GBS_ONBATTERY means we have not yet found AC power.
  // dwResult & GBS_HASBATTERY means we have found a non-UPS battery.

  DWORD dwResult = GBS_ONBATTERY;

  // IOCTL_BATTERY_QUERY_INFORMATION,
  // enumerate the batteries and ask each one for information.

  HDEVINFO hdev =
            SetupDiGetClassDevs(&GUID_DEVCLASS_BATTERY, 
                                0, 
                                0, 
                                DIGCF_PRESENT | DIGCF_DEVICEINTERFACE);
  if (INVALID_HANDLE_VALUE != hdev)
   {
    // Limit search to 100 batteries max
    for (int idev = 0; idev < 100; idev++)
     {
      SP_DEVICE_INTERFACE_DATA did = {0};
      did.cbSize = sizeof(did);

      if (SetupDiEnumDeviceInterfaces(hdev,
                                      0,
                                      &GUID_DEVCLASS_BATTERY,
                                      idev,
                                      &did))
       {
        DWORD cbRequired = 0;

        SetupDiGetDeviceInterfaceDetail(hdev,
                                        &did,
                                        0,
                                        0,
                                        &cbRequired,
                                        0);
        if (ERROR_INSUFFICIENT_BUFFER == GetLastError())
         {
          PSP_DEVICE_INTERFACE_DETAIL_DATA pdidd =
            (PSP_DEVICE_INTERFACE_DETAIL_DATA)LocalAlloc(LPTR,
                                                         cbRequired);
          if (pdidd)
           {
            pdidd->cbSize = sizeof(*pdidd);
            if (SetupDiGetDeviceInterfaceDetail(hdev,
                                                &did,
                                                pdidd,
                                                cbRequired,
                                                &cbRequired,
                                                0))
             {

				 wprintf(L"%d. %s\n", idev, pdidd->DevicePath);
              // Enumerated a battery.  Ask it for information.
              HANDLE hBattery = 
                      CreateFile(pdidd->DevicePath,
                                 GENERIC_READ | GENERIC_WRITE,
                                 FILE_SHARE_READ | FILE_SHARE_WRITE,
                                 NULL,
                                 OPEN_EXISTING,
                                 FILE_ATTRIBUTE_NORMAL,
                                 NULL);
              if (INVALID_HANDLE_VALUE != hBattery)
               {
                // Ask the battery for its tag.
                BATTERY_QUERY_INFORMATION bqi = {0};

                DWORD dwWait = 0;
                DWORD dwOut;

                if (DeviceIoControl(hBattery,
                                    IOCTL_BATTERY_QUERY_TAG,
                                    &dwWait,
                                    sizeof(dwWait),
                                    &bqi.BatteryTag,
                                    sizeof(bqi.BatteryTag),
                                    &dwOut,
                                    NULL)
                    && bqi.BatteryTag)
                 {
                  // With the tag, you can query the battery info.
                  BATTERY_INFORMATION bi = {0};
                  bqi.InformationLevel = BatteryInformation;

                  if (DeviceIoControl(hBattery,
                                      IOCTL_BATTERY_QUERY_INFORMATION,
                                      &bqi,
                                      sizeof(bqi),
                                      &bi,
                                      sizeof(bi),
                                      &dwOut,
                                      NULL))
                   {
                    // Only non-UPS system batteries count
                    if (bi.Capabilities & BATTERY_SYSTEM_BATTERY)
                     {
                      if (!(bi.Capabilities & BATTERY_IS_SHORT_TERM))
                       {
                        dwResult |= GBS_HASBATTERY;
                       }

                      // Query the battery status.
                      BATTERY_WAIT_STATUS bws = {0};
                      bws.BatteryTag = bqi.BatteryTag;

                      BATTERY_STATUS bs;
                      if (DeviceIoControl(hBattery,
                                          IOCTL_BATTERY_QUERY_STATUS,
                                          &bws,
                                          sizeof(bws),
                                          &bs,
                                          sizeof(bs),
                                          &dwOut,
                                          NULL))
                       {
                        if (bs.PowerState & BATTERY_POWER_ON_LINE)
                         {
                          dwResult &= ~GBS_ONBATTERY;
                         }
                       }
                     }
                   }
                 }
                CloseHandle(hBattery);
               }
             }
            LocalFree(pdidd);
           }
         }
       }
        else  if (ERROR_NO_MORE_ITEMS == GetLastError())
         {
          break;  // Enumeration failed - perhaps we're out of items
         }
     }
    SetupDiDestroyDeviceInfoList(hdev);
   }

  //  Final cleanup:  If we didn't find a battery, then presume that we
  //  are on AC power.

  if (!(dwResult & GBS_HASBATTERY))
    dwResult &= ~GBS_ONBATTERY;

  return dwResult;
 }

void SensorsRefresh() 
{
	// TODO: Add your control notification handler code here
	CSmartReader  m_oSmartReader;
	if(!m_oSmartReader.ReadSMARTValuesForAllDrives())
	{
       ;
	}
    DOUBLE tempreture[20];
	CCPUBasicInfo m_cpuinfo = Cpuinfo_getCpuBasicInfo();
	int i=0;
	CString info;
	     
	//CPU温度
	for(int i=0;i<m_cpuinfo.m_num_cores;i++)
	{
		GetCPUTemp(i,tempreture[i]);
	}
	for( i=0;i<m_cpuinfo.m_num_cores;i++)
	{
		printf("CPU# %d:  %.1f C\n",i,tempreture[i]);
	}
		
	ST_SMART_INFO* pInfo;
    //硬盘温度
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
			printf("HardDisk %s Temp: %dC\n",m_oSmartReader.m_stDrivesInfo[i].m_stInfo.sSerialNumber,tempdisk);
		}
		 
	}
		 
	LPCIO m_lpcio;
	CString lpcdesription;		
	if(m_lpcio.pSuperIO)
	{
		//LCP传感器, CPU温度
	    m_lpcio.pSuperIO->Update();
		int tempsensorcount=0;
		int fanspeedsensorcount=0;
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
					printf("Tempreture#%d: %.1f C\n",++tempsensorcount,m_lpcio.pSuperIO->m_sensorvalue[j].values);//Liudonghua
				}
			}

		    if(m_lpcio.pSuperIO->m_sensorvalue[j].m_type == FANSPEED){
                //风扇转速
				printf("FanSpeed#%d: %.1f RPM\n",++fanspeedsensorcount,m_lpcio.pSuperIO->m_sensorvalue[j].values);
			}
		}			
	}

	return ;
}
int GetSensorsInfo()
{
	if(!InitCPUSenor())
	{
	  MessageBoxA(NULL, "CPU Sensor Initial failed!", "警告", MB_OKCANCEL | MB_ICONWARNING);
	  return -1;
	}
	int n = 3;
	while(n--){
		
		SensorsRefresh();
		printf("\n");
		Sleep(1000);
		
	}
	return 0;
}


int GetLoginInfo()
{
	GetLoginEventLog();
	return 0;
}

BOOL ExecDosCmd(char *cmd) 
{    
	//#define EXECDOSCMD "dir c:" //可以换成你的命令

	SECURITY_ATTRIBUTES sa; 
	HANDLE hRead,hWrite;

	sa.nLength = sizeof(SECURITY_ATTRIBUTES); 
	sa.lpSecurityDescriptor = NULL; 
	sa.bInheritHandle = TRUE; 
	if (!CreatePipe(&hRead,&hWrite,&sa,0)) 
	{ 
		return FALSE; 
	} 
	char command[128];    //命令行长度 
	strcpy(command,"Cmd.exe /C "); 
	strcat(command, cmd); 
	STARTUPINFOA si; 
	PROCESS_INFORMATION pi; 
	si.cb = sizeof(STARTUPINFOA); 
	GetStartupInfoA(&si); 
	si.hStdError = hWrite;            //把创建进程的标准错误输出重定向到管道输入 
	si.hStdOutput = hWrite;           //把创建进程的标准输出重定向到管道输入 
	si.wShowWindow = SW_HIDE; 
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES; 
	//关键步骤，CreateProcess函数参数意义请查阅MSDN 
	if (!CreateProcessA(NULL, command,NULL,NULL,TRUE,NULL,NULL,NULL,&si,&pi)) 
	{ 
		CloseHandle(hWrite); 
		CloseHandle(hRead); 
		return FALSE; 
	} 
	CloseHandle(hWrite);

	char buffer[4096] = {0};          //用4K的空间来存储输出的内容，只要不是显示文件内容，一般情况下是够用了。
	DWORD bytesRead; 
	while (true) 
	{ 
		if (ReadFile(hRead,buffer,4095,&bytesRead,NULL) == NULL) 
			break; 
		//buffer中就是执行的结果
		//printf("# %s\n", buffer);
		
		int n = -1;
		char srcip[16] = {0}, destip[16] = {0};
		char *ptr = buffer;
		int srcport, destport, pid;
		while(ptr != '\0'){
			
			if(ptr[9] != '['){ //IPv4
				n = sscanf(ptr, "%*[^0-9]%[^:]:%d%*[^0-9]%[^:]:%d%*[^0-9]%d", srcip, &srcport, destip, &destport, &pid);
				//printf("src: %s[%d], dest: %s[%d], pid: %d\n", srcip, srcport, destip, destport, pid);
				if(n > 0) printf("IP: %15s,\tPort: %5d,\tPID: %5d\n", srcip, srcport, pid);
			}else{//IPv6
			}
			int i = 0;
			while(1){
				if(ptr[i] == '\r'){
					i++;
					if(ptr[i] == '\n') i++;
					ptr += i;
					break;
				}else if(ptr[i] == '\0'){
					goto end;
				}else{
					i++;
				}
			}//end while(1)
		}
	} 

end:
	CloseHandle(hRead); 
	return TRUE; 
}

int GetListeningPorts()
{
	char cmd[] = "netstat -ano | findstr \"LISTENING\"";
	//char cmd[] = "ping 127.0.0.1";
	ExecDosCmd(cmd);
	return 0;
}


int GetUSBHub()
 {
	HKEY hKEY;
	HKEY  hKeyRoot = HKEY_LOCAL_MACHINE;
	wchar_t hSubKey[] = L"SYSTEM\\CurrentControlSet\\Control\\Class\\{36FC9E60-C465-11CF-8056-444553540000}";
	wchar_t hnSubKey[512];
	int nsKey = 0;
	long ret = 0;
	
	BYTE getValue[512];//得到的键值
	DWORD keyType = REG_SZ;//定义数据类型
	DWORD DataLen = 80;//定义数据长度
	wchar_t strUser[] = _T("DriverDesc");//要查询的键名称

	while(1){
		wsprintf(hnSubKey, L"%s\\%04d", hSubKey, nsKey++);
		ret = ::RegOpenKeyEx(hKeyRoot, hnSubKey,0,KEY_READ,&hKEY);
		if(ret != ERROR_SUCCESS){
			break;
		}
		//wprintf(L" SubKey: %s\n", hnSubKey);

		ret = ::RegQueryValueEx(hKEY,strUser,NULL,&keyType,getValue,&DataLen);
		if(ret != ERROR_SUCCESS){
			continue;
		}

		wprintf(L"SubKey: %s\nDesc: %s\n", hnSubKey, getValue);

		RegCloseKey(hKEY);
	}

	

	return 0;
 }