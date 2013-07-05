// GetSysInfo.cpp : 定义控制台应用程序的入口点。
//

//#include "stdafx.h"
#include "sysinfo.h"
#include <stdio.h>
#include <afxmt.h>
#include <winioctl.h>
//#include <Windows.h>


int _tmain(int argc, _TCHAR* argv[])
{
	//printf("操作系统\n");
	//printf("==============================\n");
	//GetOSVersion();
	//
	//printf("\n\n处理器\n");
	//printf("==============================\n");
	//GetCPUInfo();

	//printf("\n\n内存\n");
	//printf("==============================\n");
	//GetMemoryInfo();

	printf("\n\n电源\n");
	printf("==============================\n");
	PowerMng();

	//printf("\n\nUSB Root Hub\n");
	//printf("==============================\n");
	//GetUSBHub();

	//printf("\n\n网卡\n");
	//printf("==============================\n");
	//GetNetInfo();
	//
	//printf("\n\n监听端口\n");
	//printf("==============================\n");
	//GetListeningPorts();

	//printf("\n\n硬盘\n");
	//printf("==============================\n");
	//GetDiskInfo();

	//printf("\n\n传感器\n");
	//printf("==============================\n");
	//GetSensorsInfo();	

	//printf("\n\n用户登录日志\n");
	//printf("==============================\n");
	//GetLoginInfo();

	//printf("Press Enter to exit\n");
	//getc(stdin);
	printf("\n");
	system("pause");

	return 0;
}

