// SmartReader.cpp: implementation of the CSmartReader class.
//
//////////////////////////////////////////////////////////////////////

#include "stdafx.h"
#include "SmartReader.h"


#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;
#define new DEBUG_NEW
#endif

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////
#define DRIVE_HEAD_REG	0xA0

CSmartReader::CSmartReader()
{
	InitAll();
	//FillAttribGenericDetails();
}

CSmartReader::~CSmartReader()
{
	CloseAll();
}

VOID CSmartReader::InitAll()
{
	m_ucDrivesWithInfo=m_ucDrives=0;
//	m_oSmartInfo.clear();
}

VOID CSmartReader::CloseAll()
{
	InitAll();
}

BOOL CSmartReader::ReadSMARTValuesForAllDrives()
{
	DWORD dwBits,dwBitVal;
	BOOL bFlag=0;
	char szDrv[MAX_PATH]={0};
	BYTE ucDriveIndex=0,ucT2=0;

	CloseAll();
	dwBits=GetLogicalDrives();
	dwBitVal=1;ucT2=0;
	bFlag=(dwBits & dwBitVal);
	m_ucDrivesWithInfo =0;
/*	while(ucT2<32)
	{
		if(bFlag)
		{
			wsprintf(szDrv,"%c:\\",'A'+ucT2);
			switch(GetDriveType(szDrv))
			{
				case DRIVE_FIXED:
						ucDriveIndex=ucT2-2;
						if(ReadSMARTInfo(ucDriveIndex))
							m_ucDrivesWithInfo++;
						m_ucDrives++;
					break;

				default:
					
					break;
			}
		}
		dwBitVal=dwBitVal*2;
		bFlag=(dwBits & dwBitVal);
		++ucT2;
	}*/
	while(ReadSMARTInfo(ucDriveIndex))
	{
		m_ucDrivesWithInfo++;
		ucDriveIndex++;
		m_ucDrives++;

	}
	if(m_ucDrives==m_ucDrivesWithInfo)
		return TRUE;
	else
		return FALSE;
}



BOOL CSmartReader::ReadSMARTInfo(BYTE ucDriveIndex)
{
	HANDLE hDevice=NULL;
	char szT1[MAX_PATH]={0};
	BOOL bRet=FALSE;
	DWORD dwRet=0;
	
	wsprintf(szT1,"\\\\.\\PHYSICALDRIVE%d",ucDriveIndex);
	hDevice=CreateFile(szT1,GENERIC_READ|GENERIC_WRITE,FILE_SHARE_READ|FILE_SHARE_WRITE,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_SYSTEM,NULL);
	if(hDevice!=INVALID_HANDLE_VALUE)
	{
		bRet=DeviceIoControl(hDevice,SMART_GET_VERSION,NULL,0,&m_stDrivesInfo[ucDriveIndex].m_stGVIP,sizeof(GETVERSIONINPARAMS),&dwRet,NULL);
Lable1:		if(bRet)
		{			
			if((m_stDrivesInfo[ucDriveIndex].m_stGVIP.fCapabilities & CAP_SMART_CMD)==CAP_SMART_CMD)
			{
				if(IsSmartEnabled(hDevice,ucDriveIndex))
				{
					bRet=CollectDriveInfo(hDevice,ucDriveIndex);
					bRet=ReadSMARTAttributes(hDevice,ucDriveIndex);
					bRet=ReadGEOMETRYInfo(hDevice,ucDriveIndex);  //获取磁盘容量等信息
				}
			}
		}
		else
		{	
			bRet=DeviceIoControl(hDevice,SMART_GET_VERSION,NULL,0,&m_stDrivesInfo[ucDriveIndex].m_stGVIP,sizeof(GETVERSIONINPARAMS),&dwRet,NULL);
		   if(bRet)
			   goto Lable1;
		} 
		CloseHandle(hDevice);
	}
	return bRet;
}

BOOL CSmartReader::IsSmartEnabled(HANDLE hDevice,UCHAR ucDriveIndex)
{
	SENDCMDINPARAMS stCIP={0};
	SENDCMDOUTPARAMS stCOP={0};
	DWORD dwRet=0;
	BOOL bRet=FALSE;

	stCIP.cBufferSize=0;
	stCIP.bDriveNumber =ucDriveIndex;
	stCIP.irDriveRegs.bFeaturesReg=ENABLE_SMART;
	stCIP.irDriveRegs.bSectorCountReg = 1;
	stCIP.irDriveRegs.bSectorNumberReg = 1;
	stCIP.irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	stCIP.irDriveRegs.bCylHighReg = SMART_CYL_HI;
	stCIP.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
	stCIP.irDriveRegs.bCommandReg = SMART_CMD;
	
	bRet=DeviceIoControl(hDevice,SMART_SEND_DRIVE_COMMAND,&stCIP,sizeof(stCIP),&stCOP,sizeof(stCOP),&dwRet,NULL);
	if(bRet)
	{

	}
	else
	{
		dwRet=GetLastError();
		m_stDrivesInfo[ucDriveIndex].m_csErrorString.Format("Error %d in reading SMART Enabled flag",dwRet);
	}
	return bRet;
}

BOOL CSmartReader::CollectDriveInfo(HANDLE hDevice,UCHAR ucDriveIndex)
{
	BOOL bRet=FALSE;
	SENDCMDINPARAMS stCIP={0};
	DWORD dwRet=0;
	#define OUT_BUFFER_SIZE IDENTIFY_BUFFER_SIZE+16
	char szOutput[OUT_BUFFER_SIZE]={0};

	stCIP.cBufferSize=IDENTIFY_BUFFER_SIZE;
	stCIP.bDriveNumber =ucDriveIndex;
	stCIP.irDriveRegs.bFeaturesReg=0;
	stCIP.irDriveRegs.bSectorCountReg = 1;
	stCIP.irDriveRegs.bSectorNumberReg = 1;
	stCIP.irDriveRegs.bCylLowReg = 0;
	stCIP.irDriveRegs.bCylHighReg = 0;
	stCIP.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
	stCIP.irDriveRegs.bCommandReg = ID_CMD;

	bRet=DeviceIoControl(hDevice,SMART_RCV_DRIVE_DATA,&stCIP,sizeof(stCIP),szOutput,OUT_BUFFER_SIZE,&dwRet,NULL);
	if(bRet)
	{
		//CopyMemory(&m_stDrivesInfo[ucDriveIndex].m_stInfo,szOutput+16,sizeof(ST_IDSECTOR));
		ST_IDSECTOR* pst_idsec =(ST_IDSECTOR*)(szOutput+16);
		m_stDrivesInfo[ucDriveIndex].m_stInfo = *pst_idsec;
		ConvertString(m_stDrivesInfo[ucDriveIndex].m_stInfo.sModelNumber,40);
		ConvertString(m_stDrivesInfo[ucDriveIndex].m_stInfo.sSerialNumber,20);
		ConvertString(m_stDrivesInfo[ucDriveIndex].m_stInfo.sFirmwareRev,8);
	}
	else
		dwRet=GetLastError();
	return bRet;
}



VOID CSmartReader::ConvertString(PBYTE pString,DWORD cbData)
{
	CString csT1;
	char szT1[MAX_PATH]={0};
	for(int nC1=0;nC1<cbData;nC1+=2)
	{
		szT1[nC1]=pString[nC1+1];
		szT1[nC1+1]=pString[nC1];
	}
	csT1=szT1;csT1.TrimLeft();csT1.TrimRight();
	lstrcpy(szT1,(PCHAR)(LPCTSTR)csT1);
	memcpy(pString,szT1,cbData);
}

VOID CSmartReader::FillAttribGenericDetails()
{
	char szINIFileName[MAX_PATH]={0},szKeyName[MAX_PATH]={0},szValue[1024]={0};
	int nC1,nSmartAttribs = 0;
	ST_SMART_DETAILS stSD;

	m_oSMARTDetails.clear();
//	if(IsDebuggerPresent()==FALSE)
	{
		GetModuleFileName(NULL,szINIFileName,MAX_PATH);
		szINIFileName[lstrlen(szINIFileName)-3]=0;
		lstrcat(szINIFileName,"ini");
	}
//	else
//		wsprintf(szINIFileName,"D:\\Saneesh\\Projects\\Helpers\\SMART\\Smart.ini");
	nSmartAttribs=GetPrivateProfileInt("General","Max Attributes",0,szINIFileName);
	for(nC1=0;nC1<nSmartAttribs;++nC1)
	{
		wsprintf(szKeyName,"Attrib%d",nC1);
		stSD.m_ucAttribId=GetPrivateProfileInt(szKeyName,"Id",0,szINIFileName);
		stSD.m_bCritical=GetPrivateProfileInt(szKeyName,"Critical",0,szINIFileName);
		GetPrivateProfileString(szKeyName,"Name","",szValue,1024,szINIFileName);
		stSD.m_csAttribName=szValue;
		if(nC1==16 && stSD.m_csAttribName == "") 
			stSD.m_csAttribName = "Temperature";
		GetPrivateProfileString(szKeyName,"Details","",szValue,1024,szINIFileName);
		stSD.m_csAttribDetails=szValue;
		m_oSMARTDetails.insert(SMARTDETAILSMAP::value_type(stSD.m_ucAttribId,stSD));
	}


}

ST_SMART_DETAILS *CSmartReader::GetSMARTDetails(BYTE ucAttribIndex)
{
	SMARTDETAILSMAP::iterator pIt;
	ST_SMART_DETAILS *pRet=NULL;

	pIt=m_oSMARTDetails.find(ucAttribIndex);
	if(pIt!=m_oSMARTDetails.end())
		pRet=&pIt->second;
	return pRet;
}

ST_SMART_INFO *CSmartReader::GetSMARTValue(BYTE ucDriveIndex,BYTE ucAttribIndex)
{
	SMARTINFOMAP::iterator pIt;
	ST_SMART_INFO *pRet=NULL;

	pIt=m_oSmartInfo.find(MAKELPARAM(ucAttribIndex,ucDriveIndex));
	if(pIt!=m_oSmartInfo.end())
		pRet=(ST_SMART_INFO *)pIt->second;
	return pRet;
}

BOOL CSmartReader::ReadSMARTAttributes(HANDLE hDevice,UCHAR ucDriveIndex)
{
	SENDCMDINPARAMS stCIP={0};
	DWORD dwRet=0;
	BOOL bRet=FALSE;
	BYTE	szAttributes[sizeof(ST_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1];
	UCHAR ucT1;
	PBYTE pT1,pT3;PDWORD pT2;
	ST_SMART_INFO *pSmartValues;

	stCIP.cBufferSize=READ_ATTRIBUTE_BUFFER_SIZE;
	stCIP.bDriveNumber =ucDriveIndex;
	stCIP.irDriveRegs.bFeaturesReg=READ_ATTRIBUTES;
	stCIP.irDriveRegs.bSectorCountReg = 1;
	stCIP.irDriveRegs.bSectorNumberReg = 1;
	stCIP.irDriveRegs.bCylLowReg = SMART_CYL_LOW;
	stCIP.irDriveRegs.bCylHighReg = SMART_CYL_HI;
	stCIP.irDriveRegs.bDriveHeadReg = DRIVE_HEAD_REG;
	stCIP.irDriveRegs.bCommandReg = SMART_CMD;
	RtlZeroMemory(szAttributes,sizeof(szAttributes));
	bRet=DeviceIoControl(hDevice,SMART_RCV_DRIVE_DATA,&stCIP,sizeof(stCIP),szAttributes,sizeof(ST_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1,&dwRet,NULL);
	if(bRet)
	{
		m_stDrivesInfo[ucDriveIndex].m_ucSmartValues=0;
		m_stDrivesInfo[ucDriveIndex].m_ucDriveIndex=ucDriveIndex;
		pT1=(PBYTE)(((ST_ATAOUTPARAM*)szAttributes)->bBuffer);
		ST_SMART_INFO* p = (ST_SMART_INFO*)pT1; 
		for(ucT1=0;ucT1<30;++ucT1)
		{
			//pT3=&pT1[ucT1*12];
			//pT2=(PDWORD)&pT3[INDEX_ATTRIB_RAW];
			//pT3[INDEX_ATTRIB_RAW+2]=pT3[INDEX_ATTRIB_RAW+3]=pT3[INDEX_ATTRIB_RAW+4]=pT3[INDEX_ATTRIB_RAW+5]=pT3[INDEX_ATTRIB_RAW+6]=0;
			if(p->m_ucAttribIndex!=0)
			{
				pSmartValues=&m_stDrivesInfo[ucDriveIndex].m_stSmartInfo[m_stDrivesInfo[ucDriveIndex].m_ucSmartValues];
			/*	pSmartValues->m_ucAttribIndex=pT3[INDEX_ATTRIB_INDEX];
				pSmartValues->wStatusFlags = *((PWORD)pT3[INDEX_ATTRIB_INDEX+1]);
				pSmartValues->m_ucValue=pT3[INDEX_ATTRIB_VALUE];
				pSmartValues->m_ucWorst=pT3[INDEX_ATTRIB_WORST];
				memcpy(pSmartValues->bRawValue,pT2,sizeof(BYTE)*6);
				pSmartValues->dwReserved=MAXDWORD;
			*/
				*pSmartValues = *p;
				m_oSmartInfo[MAKELPARAM(pSmartValues->m_ucAttribIndex,ucDriveIndex)]=pSmartValues;
				m_stDrivesInfo[ucDriveIndex].m_ucSmartValues++;
				
			}
			p++;
		}
	}
	else
		dwRet=GetLastError();
    RtlZeroMemory(szAttributes,sizeof(szAttributes));
	stCIP.irDriveRegs.bFeaturesReg=READ_THRESHOLDS;
	stCIP.cBufferSize=READ_THRESHOLD_BUFFER_SIZE; // Is same as attrib size

	bRet=DeviceIoControl(hDevice,SMART_RCV_DRIVE_DATA,&stCIP,sizeof(stCIP),szAttributes,sizeof(ST_ATAOUTPARAM) + READ_ATTRIBUTE_BUFFER_SIZE - 1,&dwRet,NULL);
	if(bRet)
	{
		pT1=(PBYTE)(((ST_ATAOUTPARAM*)szAttributes)->bBuffer);
		ST_SMART_THRESHOLD* pTHR = (ST_SMART_THRESHOLD*)pT1;
		for(ucT1=0;ucT1<30;++ucT1)
		{
			if(pTHR->bAttrID!=0)
			{
				pSmartValues=GetSMARTValue(ucDriveIndex,pTHR->bAttrID);
				if(pSmartValues)
					pSmartValues->m_dwThreshold=pTHR->bWarrantyThreshold;
			}
			pTHR++;
		}
	}
	return bRet;
}

BOOL CSmartReader::ReadGEOMETRYInfo(HANDLE hDevice,BYTE ucDriveIndex)
{
    
	if (hDevice == INVALID_HANDLE_VALUE){
		return (FALSE);
	}
	DWORD cbByteReturned;
   DISK_GEOMETRY* pGeometry = &m_stDrivesInfo[ucDriveIndex].m_geometr;

    BOOL  bResult = DeviceIoControl(hDevice,   

         IOCTL_DISK_GET_DRIVE_GEOMETRY,   

         NULL, 0,             

         pGeometry, sizeof(DISK_GEOMETRY),   

         &cbByteReturned, NULL);

     return (bResult);
}

ST_DRIVE_INFO *CSmartReader::GetDriveInfo(BYTE ucDriveIndex)
{
	return &m_stDrivesInfo[ucDriveIndex];
}
/*
		switch(pT3[0])
			{
				case SMART_ATTRIB_RAW_READ_ERROR_RATE:// Raw Read Error Rate
						TRACE("\n Raw Read Error Rate: %d",pT2[0]);
					break;

				case SMART_ATTRIB_SPIN_UP_TIME:// Spin up Time
						TRACE("\n Spin up Time:%d",pT2[0]);
					break;

				case SMART_ATTRIB_START_STOP_COUNT:// Start stop count
						TRACE("\n Start stop count:%d",pT2[0]);
					break;

				case SMART_ATTRIB_START_REALLOCATION_SECTOR_COUNT:// Reallocation Sector Count
						TRACE("\n Reallocation Sector Count:%d",pT2[0]);
					break;

				case SMART_ATTRIB_SEEK_ERROR_RATE:// Seek Error Rate
						TRACE("\n Seek Error Rate:%d",pT2[0]);
					break;

				case SMART_ATTRIB_POWER_ON_HOURS_COUNT:// Power On hours count
						TRACE("\n Power On hours count:%d",pT2[0]);
					break;

				case SMART_ATTRIB_SPIN_RETRY_COUNT:// Spin retry count
						TRACE("\n Spin retry count:%d",pT2[0]);
					break;

				case SMART_ATTRIB_DEVICE_POWER_CYCLE_COUNT:// Device Power cycle count
						TRACE("\n Device Power cycle count:%d",pT2[0]);
					break;

				case SMART_ATTRIB_POWER_OFF_RETRACT_COUNT:// Power-off Retract Count
						TRACE("\n Power-off Retract Count:%d",pT2[0]);
					break;

				case SMART_ATTRIB_LOAD_UNLOAD_CYCLE_COUNT:// Load/Unload cycle Count
						TRACE("\n Load/Unload cycle Count:%d",pT2[0]);
					break;

				case SMART_ATTRIB_TEMPERATURE: // Temperature
						TRACE("\n Temperature: %d C",pT2[0]);
					break;

				case SMART_ATTRIB_ECC_ON_THE_FLY_COUNT:// ECC on the fly count
						TRACE("\n ECC on the fly count: %d",pT2[0]);
					break;

				case SMART_ATTRIB_REALLOCATION_EVENT_COUNT:// Reallocation Event Count
						TRACE("\n Reallocation Event Count: %d",pT2[0]);
					break;

				case SMART_ATTRIB_CURRENT_PENDING_SECTOR_COUNT:// Current Pending Sector count
						TRACE("\n Current Pending Sector count: %d",pT2[0]);
					break;

				case SMART_ATTRIB_UNCORRECTABLE_SECTOR_COUNT:// Uncorrectable sector count
						TRACE("\n Uncorrectable sector count: %d",pT2[0]);
					break;

				case SMART_ATTRIB_ULTRA_DMA_CRC_ERROR_COUNT:// Ultra DMA CRC Error count
						TRACE("\n Ultra DMA CRC Error count: %d",pT2[0]);
					break;

				case SMART_ATTRIB_WRITE_ERROR_RATE://Write Error rate
						TRACE("\n Write Error rate: %d",pT2[0]);
					break;

				case SMART_ATTRIB_TA_COUNTER_INCREASED:// TA Counter increased
						TRACE("\n TA Counter increased: %d",pT2[0]);
					break;

				default:
						if(pT3[0]!=0)
						{
							TRACE("\n ID %d Value %d",pT3[0],pT2[0]);
						}
						else
							m_stDrivesInfo[ucDriveIndex].m_ucSmartValues--;
					break;
			}

*/