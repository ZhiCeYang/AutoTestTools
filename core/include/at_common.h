#ifndef __AT_COMMON_H__
#define __AT_COMMON_H__

#ifndef OK
#define OK      1
#endif
#endif
#ifndef FAILED
#define FAILED  -1
#endif
#ifndef ERROR
#define ERROR   0
#endif

#ifndef LOCAL
#define LOCAL   static
#endif
#ifndef IMPORT
#define IMPORT  extern

#ifndef UINT8
#define UINT8
typedef unsigned char       uc8_t;
#endif
#ifndef UINT16
#define UINT16
typedef unsigned short      uint16_t;
#endif
#ifndef UINT32
#define UINT32
typedef unsigned int        uint32_t;
#endif
#ifndef UINT64
#define UINT64
typedef unsigned long       uint64_t;
#endif

#ifndef INT8
#define INT8
typedef signed char         c8_t;
#endif

#ifndef INT16
#define INT16
typedef signed short int    int16_t;
#endif
#ifndef INT32
#define INT32
typedef signed int         int32_t;
#endif
#ifndef INT64
#define INT64
typedef long                int64_t;
#endif

#ifndef NULL                               
#define NULL (void*)(0)
#endif

#define MAX8				(0xff)
#define MAX16				(0xffff)
#define MAX32				(0xffffffff)

typedef enum atstat_e{
    AT_OK,
    AT_FAILED,
    AT_ERROR
}atstat_e;

#define RET_ERROR    -1
#define RET_OK       1

typedef enum atbool_e{
    AT_TRUE,
    AT_FALSE
}atbool_e;

#endif

