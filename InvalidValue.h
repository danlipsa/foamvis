/**
 * @file SemanticError.h
 * @author Dan R. Lipsa
 *
 * Declaration of the InvalidValue exception
 */
#ifndef __INVALID_VALUE_H__
#define __INVALID_VALUE_H__

#include <string>
#include <exception>
using namespace std;

/**
 * Exception that signals  that a variable has an invalid value
 */
class InvalidValue : public exception
{
public:
    /**
     * Constructs a InvalidValue object
     * @param reason message describing the reason for the exception
     */
    InvalidValue (const char* reason): m_reason(reason) {}
    /**
     * Constructs a InvalidValue object
     * @param reason message describing the reason for the exception
     */
    InvalidValue (const string& reason): m_reason(reason) {}


    /**
     * Destroys the object
     */
    ~InvalidValue() throw() {}
    /**
     * Function that describes the reason for the error
     * @return message describing the reason for the error
     */
    virtual const char* what()
    {
	return m_reason.c_str();
    }

private:
    /**
     * Stores a string describing the reason for the error.
     */
    string m_reason;
};

#endif
