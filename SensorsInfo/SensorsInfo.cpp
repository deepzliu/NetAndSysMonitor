// SensorsInfo.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <afxmt.h>
#include <winioctl.h>
#include "Sensors.h"
#include "SmartReader.h"
#include "./include/cpuinfolib/CPUBasicInfo.h"
#include "./include/cpuinfolib/libcpuinfo_funclist.h"
#include "lpc/LPCIO.h"

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
        //info.Format(L"CPU# %d:  %.1f C",i,tempreture[i]);
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
           	//info.Format(L"HardDisk%sTemp: %dC",m_oSmartReader.m_stDrivesInfo[i].m_stInfo.sSerialNumber,tempdisk);
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
					//str.Format(L"Tempreture#%d: %.1f C",++tempsensorcount,m_lpcio.pSuperIO->m_sensorvalue[j].values);//Liudonghua
					printf("Tempreture#%d: %.1f C\n",++tempsensorcount,m_lpcio.pSuperIO->m_sensorvalue[j].values);//Liudonghua
				}
			}

		    if(m_lpcio.pSuperIO->m_sensorvalue[j].m_type == FANSPEED){
				//continue;
                //风扇转速
				//str.Format(L"FanSpeed#%d: %.1f RPM",++fanspeedsensorcount,m_lpcio.pSuperIO->m_sensorvalue[j].values);
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
	int n = 5;
	while(n--){
		printf("\n==========================\n");
		SensorsRefresh();
		Sleep(1000);
		
	}
	return 0;
}

int _tmain(int argc, _TCHAR* argv[])
{
	GetSensorsInfo();
	return 0;
}

