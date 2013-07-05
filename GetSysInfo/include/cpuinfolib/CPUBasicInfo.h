#ifndef LIBCPUINFO__BASICCPUINFO_H_
#define LIBCPUINFO__BASICCPUINFO_H_

#include "../cputool_error.h"
#include "../cputool_types.h"
#include "libcpuinfo_constants.h"

/* CPU vendor, as guessed from the Vendor String */
typedef enum {
	VENDOR_INTEL = 0,       /* Intel CPU */
	VENDOR_AMD,             /* AMD CPU */
	VENDOR_CYRIX,           /* Cyrix CPU */
	VENDOR_NEXGEN,          /* NexGen CPU */
	VENDOR_TRANSMETA,       /* Transmeta CPU */
	VENDOR_UMC,             /* x86 CPU by UMC */
	VENDOR_CENTAUR,         /* x86 CPU by IDT */
	VENDOR_RISE,            /* x86 CPU by Rise Technology */
	VENDOR_SIS,             /* x86 CPU by SiS */
	VENDOR_NSC,             /* x86 CPU by National Semiconductor */
	
	NUM_CPU_VENDORS,        /* Valid CPU vendor ids: 0..NUM_CPU_VENDORS - 1 */
	VENDOR_UNKNOWN = -1,
} cpu_vendor_t;

typedef enum {
    CPU_FEATURE_FPU = 0,    /* Floating point unit */
    CPU_FEATURE_MMX,        /* MMX instruction set supported */
    CPU_FEATURE_SSE,        /* Streaming-SIMD Extensions (SSE) supported */
    CPU_FEATURE_SSE2,       /* SSE2 instructions supported */
    CPU_FEATURE_SSE3,       /* SSE3 instructions supported */
    CPU_FEATURE_SSSE3,      /* SSSE3 instructionss supported (this is different from SSE3!) */
    CPU_FEATURE_SSE4_1,     /* SSE 4.1 instructions supported */
    CPU_FEATURE_SSE4_2,     /* SSE 4.2 instructions supported */
    CPU_FEATURE_3DNOW,      /* AMD 3DNow! instructions supported */

    /* termination: */
    NUM_CPU_FEATURES,
} cpu_feature_t;

const struct {
    cpu_feature_t feature;
    const char* name;
} cpu_feature_str[] = {
    { CPU_FEATURE_FPU,      "FPU" },
    { CPU_FEATURE_MMX,      "MMX" },
    { CPU_FEATURE_SSE,      "SSE" },
    { CPU_FEATURE_SSE2,     "SSE2" },
    { CPU_FEATURE_SSE3,     "SSE3" },
    { CPU_FEATURE_SSSE3,    "SSSE3" },
    { CPU_FEATURE_SSE4_1,   "SSE4_1" },
    { CPU_FEATURE_SSE4_2,   "SSE4_2" },
    { CPU_FEATURE_3DNOW,    "3DNow!" },
};


struct CCPUBasicInfo {
    char m_vendor_str[VENDOR_STR_MAX];      // the CPU vendor str, eg. "GenuineIntel"
    char m_brand_str[BRAND_STR_MAX];        // the brand str, eg. "Intel(R) Xeon(TM) CPU 2.40GHZ"
    cpu_vendor_t m_vendor;                  // contains the recognized CPU vendor

    /* the Features */
    uint8_t m_flags[CPU_FLAGS_MAX];

    int32_t m_family;                 // CPU family
    int32_t m_model;                  // CPU model
    int32_t m_stepping;               // CPU stepping

    int32_t m_ext_family;             // CPU extended family
    int32_t m_ext_model;              // CPU extended model

    /* Number of CPU cores on the current processor */
    int32_t m_num_cores;
    int32_t m_num_logical_cpus;
    int32_t m_total_cpus;
};

struct CCPUCacheInfo {
    int32_t m_l1_data_cachesz;
    int32_t m_l1_inst_cachesz;
    int32_t m_l2_cachesz;
    int32_t m_l3_cachesz;

    int32_t m_l1_assoc;
    int32_t m_l2_assoc;
    int32_t m_l3_assoc;

    int32_t m_l1_cacheline;
    int32_t m_l2_cacheline;
    int32_t m_l3_cacheline;
};

#endif  // LIBCPUINFO__BASICCPUINFO_H_
