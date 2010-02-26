/**
 * @file   Debug.h
 * @author Dan R. Lipsa
 * @date 15 Feb. 2006
 *

 */
#ifndef __DEBUG_H__
#define __DEBUG_H__

void _RuntimeAssert (bool condition,  string& message);


#define RuntimeAssert(condition, message)				\
    ostringstream __ostr;						\
    __ostr << __FILE__ << ": " << __LINE__ << ":"			\
    << message << ends;							\
    _RuntimeAssert (condition, __ostr.str ())




#endif  //__DEBUG_H__

// Local Variables:
// mode: c++
// End:
