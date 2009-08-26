#ifndef __DEBUG_STREAM_H__
#define __DEBUG_STREAM_H__

#ifdef _MSC_VER
#include <sstream>
class DebugStream
{
public:
    template <class A>
        DebugStream& operator<<(A a)
    {
		std::ostringstream ostr;
		ostr << a << std::ends;
        OutputDebugStringA (ostr.str ().c_str ());
        return *this;
    }
};

extern DebugStream cdbg;
#else   //_MSC_VER
extern ostream& cdbg;

#endif  //_MSC_VER


#endif  //__DEBUG_STREAM_H__
