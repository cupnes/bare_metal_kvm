#pragma once

/* #define DEBUG */

#ifdef DEBUG
#define DEBUG_PRINT(...)	fprintf(stderr, __VA_ARGS__)
#else
#define DEBUG_PRINT(...)	((void)0)
#endif
