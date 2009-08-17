#ifndef __SYSTEM_DIFFERENCES_H__
#define __SYSTEM_DIFFERENCES_H__

#ifdef _MSC_VER
#define strcasecmp _stricmp
#define strncasecmp _strnicmp
#define isatty _isatty
#endif



#endif
