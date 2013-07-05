#ifndef CPUTOOL_error_H_
#define CPUTOOL_error_H_

typedef enum {
    ERR_OK       =  0,      /* "No error" */
    ERR_NO_CPUID = -1,      /* "CPUID instruction is not supported" */
    ERR_NO_RDTSC = -2,      /* "RDTSC instruction is not supported" */
    ERR_NO_MEM   = -3,      /* "Memory allocation failed" */
    ERR_NOT_IMP  = -6,      /* "Unimplemented" */
    ERR_CPU_UNKN = -7,      /* "Unsupported processor" */
    
    ERR_PRIME_TEST_FAIL         = -9,   /* Prime test failed */
    ERR_INST_BASIC_TEST_FAIL    = -10,  /* Basic instruction test failed */
    ERR_INST_FPU_TEST_FAIL      = -11,  /* FPU instruction test failed */
	ERR_INST_3DNOW_TEST_FAIL    = -12,  /* 3DNow! instruction test failed */
    ERR_INST_MMX_TEST_FAIL      = -13,  /* MMX instruction test failed */
    ERR_INST_SSE_TEST_FAIL      = -14,  /* SSE instruction test failed */
    ERR_INST_SSE2_TEST_FAIL     = -15,  /* SSE2 instruction test failed */
    ERR_INST_SSE3_TEST_FAIL     = -16,  /* SSE3 instruction test failed */
    ERR_INST_SSE41_TEST_FAIL    = -17,  /* SSE41 instruction test failed */
    ERR_INST_SSE42_TEST_FAIL    = -18,  /* SSE42 instruction test failed */

	ERR_UNKNOWN					= -20,


} cpu_error_t;

#endif /* CPUTOOL_ERROR_H_ */