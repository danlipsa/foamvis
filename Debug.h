/**
 * @file   Debug.h
 * @author Dan R. Lipsa
 * @date 15 Feb. 2006
 * @ingroup utils
 * @brief Debugging support
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
	ostr << message 
	     << " " << t << ends;
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
	ostr << message 
	     << " " << t1 
	     << " " << t2 << ends;
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
	ostr << message 
	     << " " << t1 
	     << " " << t2 
	     << " " << t3 << ends;
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
	ostr << message 
	     << " " << t1 
	     << " " << t2 
	     << " " << t3 
	     << " " << t4 << ends;
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
	ostr << message 
	     << " " << t1 
	     << " " << t2 
	     << " " << t3 
	     << " " << t4 
	     << " " << t5 << ends;
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

/**
 * Use the following pattern to log debugging code in functions:
 * __ENABLE_LOGGING__;
 * ...
 * __LOG__ (logging code);
 * When finished debugging, delete the __ENABLE_LOGGING__ statement
 */
#define __LOG__(code) if (__LOGGED__) {code}
#define __ENABLE_LOGGING__ const bool __LOGGED__ = true
const bool __LOGGED__ = false;



#ifdef _MSC_VER
/**
 * @brief Debugging output stream for the debug console in Visual Studio.
 */
class DebugStream
{
public:
	typedef ostream& (*EndlFunction) (ostream& o);
    template <typename A>
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

/**
 * @brief Measures time
 */
class MeasureTime
{
public:
    MeasureTime ();
    void StartInterval ();
    void EndInterval (const char* intervalName);
    void EndInterval (const string& intervalName)
    {
	EndInterval (intervalName.c_str ());
    }
private:
    clock_t m_start;
};

/**
 * @brief Measures time for VTK routines
 */
class MeasureTimeVtk : public vtkCommand
{
public:
    MeasureTimeVtk ()
    {
    }
    void Execute (vtkObject *caller, unsigned long eventId, void *callData);
    void Measure (vtkObject *caller);

private:
    MeasureTime m_measure;
};


#endif  //__DEBUG_H__

// Local Variables:
// mode: c++
// End:
