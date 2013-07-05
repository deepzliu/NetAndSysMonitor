#ifndef CPUTOOL_TYPES_H_
#define CPUTOOL_TYPES_H_

#if defined (HAVE_STDINT_H)
    #include <stdint.h>
#else

#  if !defined(HAVE_INT32_T) && !defined(__int32_t_defined)
    typedef int int32_t;
#  endif

#  if !defined(HAVE_UINT32_T) && !defined(__uint32_t_defined)
    typedef unsigned uint32_t;
#  endif

typedef signed char         int8_t;
typedef unsigned char       uint8_t;
typedef signed short        int16_t;
typedef unsigned short      uint16_t;
//typedef signed long long    int64_t;
//typedef unsigned long long  uint64_t;
typedef __int64             int64_t;
typedef unsigned __int64    uint64_t;

#endif

#endif /* CPUTOOL_TYPES_H_ */