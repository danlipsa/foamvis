/**
 * @file   Debug.h
 * @author Dan R. Lipsa
 * @date 15 Feb. 2006
 *

 */
#ifndef __DEBUG_H__
#define __DEBUG_H__

void RuntimeAssert (bool condition, const string& message);

template<typename T>
void RuntimeAssert (bool condition, const string& message, const T& t)
{
    ostringstream ostr;
    ostr << message << t << ends;
    RuntimeAssert (condition, ostr.str ());
}

template<typename T1, typename T2>
void RuntimeAssert (bool condition, const string& message, const T1& t1,
		    const T2& t2)
{
    ostringstream ostr;
    ostr << message << t1 << t2 << ends;
    RuntimeAssert (condition, ostr.str ());
}


template<typename T1, typename T2, typename T3>
void RuntimeAssert (bool condition,
		    const string& message,
		    const T1& t1, const T2& t2, const T3& t3)
{
    ostringstream ostr;
    ostr << message << t1 << t2 << t3 << ends;
    RuntimeAssert (condition, ostr.str ());
}



#endif  //__DEBUG_H__

// Local Variables:
// mode: c++
// End:
