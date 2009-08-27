#ifndef __DEBUG_STREAM_H__
#define __DEBUG_STREAM_H__

#ifdef _MSC_VER
#include <iostream>
#include <sstream>
class DebugStream
{
public:
	typedef std::ostream& (*EndlFunction) (std::ostream& o);
    template <class A>
        DebugStream& operator<<(A a)
    {
		std::ostringstream ostr;
		ostr << a << std::ends;
        OutputDebugStringA (ostr.str ().c_str ());
        return *this;
    }

    DebugStream& operator<< (EndlFunction)
    {
        OutputDebugStringA ("\n");
		return *this;
    }
};

extern DebugStream cdbg;
#else   //_MSC_VER
extern ostream& cdbg;

#endif  //_MSC_VER


#endif  //__DEBUG_STREAM_H__
