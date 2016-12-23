#ifndef TCHAR_H_FAKE
#define TCHAR_H_FAKE

#include <wchar.h>

#define TCHAR	char
#define _T(a) a

#define _tcslen	strlen

#define _stprintf sprintf


//#define	MAX_PATH	1024


#ifndef RECT
/*
typedef struct {
	int left;
	int top;
	int right;
	int bottom;
} RECT;
*/
#endif

#endif
