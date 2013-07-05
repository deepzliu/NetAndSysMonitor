#include "StdAfx.h"
#include "./cpuinfolib/libcpuinfo_funclist.h"
#include "Sensors.h"

#include "Ring0.h"
//#include "WinIo.h"

#pragma comment(lib, "libcpuinfo.lib")
UINT Tjunction=85;
UINT tScope = 1;
UINT MaxMultipler=0;
CCPUBasicInfo m_cpuinfo;
UINT CPUType =0;

//FOR AMD
UINT miscellaneousControlAddress =0;
float  param =0.0f ;


const uint PERF_CTL_0 = 0xC0010000;
const uint PERF_CTR_0 = 0xC0010004;
const uint P_STATE_0 = 0xC0010064;
const uint COFVID_STATUS = 0xC0010071;
const byte PCI_BUS = 0;
const byte PCI_BASE_DEVICE = 24;
const byte DEVICE_VENDOR_ID_REGISTER = 0;
const USHORT AMD_VENDOR_ID = 0x1022;

const uint FIDVID_STATUS = 0xC0010042;
const byte MISCELLANEOUS_CONTROL_FUNCTION = 3;
const USHORT AMD0F_MISCELLANEOUS_CONTROL_DEVICE_ID = 0x1103;
const uint THERMTRIP_STATUS_REGISTER = 0xE4;
const byte THERM_SENSE_CORE_SEL_CPU0 = 0x4;
const byte THERM_SENSE_CORE_SEL_CPU1 = 0x0;



const USHORT AMD10_MISCELLANEOUS_CONTROL_DEVICE_ID = 0x1203;
const uint REPORTED_TEMPERATURE_CONTROL_REGISTER = 0xA4;

Ring0 m_ring0;

static UINT GetPciAddress(BYTE bus,BYTE device,BYTE function)
{
	return (UINT)(((bus & 0xFF) << 8) | ((device & 0x1F) << 3) | (function & 7));
}



BOOL InitCPUSenor(void)
{
    UINT eax,edx;
	m_cpuinfo =  Cpuinfo_getCpuBasicInfo();
	if(!m_ring0.Open())
          return false;
    if(m_cpuinfo.m_vendor == VENDOR_INTEL)
	{
		//获取INTELCPU温度参数
      if(m_ring0.Rdmsr(0xee,eax,edx)) //0Xee 没有查到出处
	  {
         if(eax & 0x40000000)
		      Tjunction = 85;
		 else
			 Tjunction = 100;
	  }
	  else
	  {
		 Tjunction = 100;  // is i7?
	  }
	   //获取最大倍频数
	  if(m_ring0.Rdmsr(0x198,eax,edx))
	  {
		 MaxMultipler = (eax>>8)&0x1f;
	  }
	 
	  if(m_ring0.Rdmsr(0xC0010042, eax,edx)) //最大倍频
		{
          MaxMultipler = ((eax>>16)&0x3F)/2 + 4;
        }
	}
	else
	if(m_cpuinfo.m_vendor == VENDOR_AMD)
	{
       
        if(m_cpuinfo.m_ext_family==0x0F)
		{
             float offset = -49.0f;

            // AM2+ 65nm +21 offset
             UINT model = m_cpuinfo.m_model;
           if (model >= 0x69 && model != 0xc1 && model != 0x6c && model != 0x7c) 
               offset += 21;

			 param = 0;
		     USHORT deviceId = AMD0F_MISCELLANEOUS_CONTROL_DEVICE_ID;
			 byte   function = MISCELLANEOUS_CONTROL_FUNCTION;
			 int  processorIndex = 0;
             UINT address = GetPciAddress(PCI_BUS,
             (PCI_BASE_DEVICE + processorIndex), function);

      // verify that we have the correct bus, device and function
             UINT deviceVendor;
           if (!m_ring0.ReadPciConfig(
              address, DEVICE_VENDOR_ID_REGISTER, deviceVendor))
                return FALSE;
      
             if (deviceVendor != (deviceId << 16 | AMD_VENDOR_ID))
                return FALSE;

            miscellaneousControlAddress =   address;
		}
		else
		if(m_cpuinfo.m_ext_family == 0x10)
		{
			 USHORT deviceId = AMD10_MISCELLANEOUS_CONTROL_DEVICE_ID;
			 byte   function = MISCELLANEOUS_CONTROL_FUNCTION;
			 int  processorIndex = 0;
             UINT address = GetPciAddress(PCI_BUS,
             (PCI_BASE_DEVICE + processorIndex), function);

      // verify that we have the correct bus, device and function
             UINT deviceVendor;
           if (!m_ring0.ReadPciConfig(
              address, DEVICE_VENDOR_ID_REGISTER, deviceVendor))
                return FALSE;
      
          if (deviceVendor != (deviceId << 16 | AMD_VENDOR_ID))
                return FALSE;

            miscellaneousControlAddress =   address;
		}
		
	}
    return TRUE;
}

BOOL GetCPUTemp(UINT coreindex,OUT DOUBLE &temperTure)
{
	UINT eax,edx;
	UINT value;
    if(coreindex<m_cpuinfo.m_num_cores)
	{
	  if(m_cpuinfo.m_vendor == VENDOR_INTEL)
	  {
	      SetThreadAffinityMask(GetCurrentThread(),1<<coreindex);
		  if(m_ring0.Rdmsr(IA32_THERM_STATUS_MSR,eax,edx))
		  {
             if ((eax & 0x80000000) != 0) 
			 {
            // get the dist from tjMax from bits 22:16
                DWORD deltaT = ((eax & 0x007F0000) >> 16);
                temperTure = Tjunction - tScope * deltaT;
			    return TRUE;
			 } 
		  }
	  }  
	  else if(m_cpuinfo.m_vendor == VENDOR_AMD)
	  {
		if(miscellaneousControlAddress !=0)
		{
			if(m_cpuinfo.m_ext_family==0x0f)
			{
			
			  if(m_ring0.WritePciConfig(miscellaneousControlAddress,THERMTRIP_STATUS_REGISTER,
              coreindex > 0 ? THERM_SENSE_CORE_SEL_CPU1 : THERM_SENSE_CORE_SEL_CPU0))
			  {  // UINT value;
				if(m_ring0.ReadPciConfig(miscellaneousControlAddress,THERMTRIP_STATUS_REGISTER,value))
				{
					temperTure = ((value >> 16) & 0xFF) + param;
					return TRUE;
				}
			  }
			}
			else
			if(m_cpuinfo.m_ext_family == 0x10)
			{
                //UINT value;
				if(m_ring0.ReadPciConfig(miscellaneousControlAddress,REPORTED_TEMPERATURE_CONTROL_REGISTER,value))
				{
					temperTure = ((value >> 21) & 0x7FF)/8.0f + param;
					return TRUE;
				}
			}
		}

	}
}		
       return FALSE;
}

BOOL GetCPUMaxMuiltipler(UINT &MaxMuiltipler)
{
  /* 
   if(MaxMultipler!=0)
   {	
	   MaxMuiltipler = MaxMultipler;
	   return TRUE;
   }
   return FALSE;
   */
	UINT eax,edx;
     if(m_ring0.Rdmsr(0x198,eax,edx))
	  {
		 MaxMuiltipler = (eax>>8)&0x1f;
	  }
	  else
	  {
	   return FALSE;
	  }
	 return TRUE;
}
BOOL GetCPUMuiltipler(UINT &Muiltipler)
{
	UINT eax,edx;
	Muiltipler =0;
	if(m_cpuinfo.m_vendor==VENDOR_INTEL)
	{
      if(m_ring0.Rdmsr(0x2A,eax,edx))
           Muiltipler = (eax>>22)&0x1F;
    }
    else if(m_cpuinfo.m_vendor==VENDOR_AMD)
	{
       if(m_ring0.Rdmsr(0xC0010042,eax,edx))
		  Muiltipler = (eax&0x3f)*0.5+4;
	}
    if(Muiltipler == 0)
	{
        Muiltipler = MaxMultipler;
	}
	return TRUE;
}