/**
 * @file   DebugStream.h
 * @author Dan R. Lipsa
 *
 * Stream for printing debug messages in Visual Studio. All debug messages
 * have to go to cdbg.
 */
#ifndef __DEBUG_STREAM_H__
#define __DEBUG_STREAM_H__

#ifdef _MSC_VER
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
extern std::ostream& cdbg;

#endif  //_MSC_VER
#endif  //__DEBUG_STREAM_H__

// Local Variables:
// mode: c++
// End:
