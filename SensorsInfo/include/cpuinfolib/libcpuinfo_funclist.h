#ifndef LIBCPUINFO_CPUINFOLIST_H_
#define LIBCPUINFO_CPUINFOLIST_H_

#include "CPUBasicInfo.h"

/* This file is used to define the interface of this dll */
#ifndef DLL_EXP_IMP
#ifdef  _DLL_
#define DLL_EXP_IMP extern "C" __declspec(dllexport)
#else
#define DLL_EXP_IMP extern "C" __declspec(dllimport)
#endif
#endif

/* calc the CPU's freq, in MHz. WARN: this may take 1 seconds */
DLL_EXP_IMP int Cpuinfo_getCpuClockMHz(int coreno);

/* get the CPU's basic information, eg. the vendor, family, etc. */
DLL_EXP_IMP CCPUBasicInfo Cpuinfo_getCpuBasicInfo();

/* determine whether the current cpu support the requested feature */
DLL_EXP_IMP bool Cpuinfo_isFeaturePresent(cpu_feature_t feature);

/* return the errorcode or the error string while get CPU info */
DLL_EXP_IMP cpu_error_t Cpuinfo_getCpuInfoErrorCode();
DLL_EXP_IMP const char *Cpuinfo_getCpuInfoErrorStr();

#endif  // LIBCPUINFO_CPUINFOLIST_H_