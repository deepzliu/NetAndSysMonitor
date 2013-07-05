

#pragma once


int GetOSVersion();
int GetCPUInfo();
double GetCPUUsage(int *cpunum);
int GetMemoryInfo();
int GetDiskInfo();
int GetDiskPerformance();
int GetNetInfo();
int GetNetPerformance(char *pAdapterDesc);
int GetConnectedState(char *gw);
int PowerMng();
int GetPowerInfo();
unsigned long GetBatteryState();
int GetSensorsInfo();
int GetLoginInfo();
int GetListeningPorts();
int GetUSBHub();
