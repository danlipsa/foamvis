/**
 * @file   Body.h
 * @author Dan R. Lipsa
 *
 * Differences between different OSes
 */
#ifndef __SYSTEM_DIFFERENCES_H__
#define __SYSTEM_DIFFERENCES_H__

#ifdef _MSC_VER

#define strcasecmp _stricmp
#define isatty _isatty

#else  //_MSC_VER


#endif //_MSC_VER
#endif //__SYSTEM_DIFFERENCES_H__

// Local Variables:
// mode: c++
// End:
