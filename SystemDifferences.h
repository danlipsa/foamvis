/**
 * @file   SystemDifferences.h
 * @author Dan R. Lipsa
 * @ingroup utils
 * @brief Differences between Unix and Windows
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
