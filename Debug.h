/**
 * @file   Debug.h
 * @author Dan R. Lipsa
 * @date 15 Feb. 2006
 *

 */
#ifndef __DEBUG_H__
#define __DEBUG_H__

inline void RuntimeAssert (bool condition, const string& message)
{
    if (! condition)
	throw logic_error (message);
}


inline void ThrowException (const string& message)
{
    throw logic_error (message);
}


template<typename T>
void RuntimeAssert (bool condition, const string& message, const T& t)
{
    if (! condition)
    {
	ostringstream ostr;
	ostr << message << t << ends;
	ThrowException (ostr.str ());
    }
}

template<typename T1, typename T2>
void RuntimeAssert (bool condition, const string& message, const T1& t1,
		    const T2& t2)
{
    if (! condition)
    {
	ostringstream ostr;
	ostr << message << t1 << t2 << ends;
	ThrowException (ostr.str ());
    }
}


template<typename T1, typename T2, typename T3>
void RuntimeAssert (bool condition,
		    const string& message,
		    const T1& t1, const T2& t2, const T3& t3)
{
    if (! condition)
    {
	ostringstream ostr;
	ostr << message << t1 << t2 << t3 << ends;
	ThrowException (ostr.str ());
    }
}

template<typename T1, typename T2, typename T3, typename T4>
void RuntimeAssert (bool condition,
		    const string& message,
		    const T1& t1, const T2& t2, const T3& t3, const T4& t4)
{
    if (! condition)
    {
	ostringstream ostr;
	ostr << message << t1 << t2 << t3 << t4 << ends;
	ThrowException (ostr.str ());
    }
}

template<typename T1, typename T2, typename T3, typename T4, typename T5>
void RuntimeAssert (bool condition,
		    const string& message,
		    const T1& t1, const T2& t2, const T3& t3, const T4& t4, 
		    const T5& t5)
{
    if (! condition)
    {
	ostringstream ostr;
	ostr << message << t1 << t2 << t3 << t4 << t5 << ends;
	ThrowException (ostr.str ());
    }
}



template<typename T>
void ThrowException (const string& message, const T& t)
{
    RuntimeAssert (false, message, t);
}

template<typename T1, typename T2>
void ThrowException (const string& message, const T1& t1, const T2& t2)
{
    RuntimeAssert (false, message, t1, t2);
}

template<typename T1, typename T2, typename T3>
void ThrowException (const string& message, const T1& t1, const T2& t2,
		     const T3& t3)
{
    RuntimeAssert (false, message, t1, t2, t3);
}


#endif  //__DEBUG_H__

// Local Variables:
// mode: c++
// End:
