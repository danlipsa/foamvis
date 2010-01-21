/**
 * @file   DebugStream.h
 * @author Dan R. Lipsa
 *
 * Stream for printing debug messages. All debug messages
 * have to go to cdbg. It works with both MS Visual C++ and with GCC
 */
#ifndef __DEBUG_STREAM_H__
#define __DEBUG_STREAM_H__

#ifdef _MSC_VER
class DebugStream
{
public:
	typedef ostream& (*EndlFunction) (ostream& o);
    template <class A>
        DebugStream& operator<<(A a)
    {
		ostringstream ostr;
		ostr << a << ends;
        OutputDebugStringA (ostr.str ().c_str ());
        return *this;
    }

    DebugStream& operator<< (EndlFunction)
    {
        OutputDebugStringA ("\n");
		return *this;
    }
};

/**
 * Stream where all debugging output should be sent. Works bor both MS
 * Visual C++ and for g++.
 */
extern DebugStream cdbg;

#else   //_MSC_VER
/**
 * Stream where all debugging output should be sent. Works bor both MS
 * Visual C++ and for g++.
 */
extern ostream& cdbg;

#endif  //_MSC_VER
#endif  //__DEBUG_STREAM_H__

// Local Variables:
// mode: c++
// End:
