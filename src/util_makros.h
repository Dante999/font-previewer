#ifndef UTIL_DEFINES_H
#define UTIL_DEFINES_H

#ifndef ARRAY_SIZE
#	define ARRAY_SIZE(x) (sizeof(x) / sizeof(x[0]))
#endif

#ifndef MIN
#	define MIN(x,y) (x < y ? x : y)
#endif

#ifndef MAX 
#	define MAX(x,y) (x > y ? x : y)
#endif

#endif // UTIL_DEFINES_H
