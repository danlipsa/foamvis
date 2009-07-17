/**
 * @file SemanticError.h
 * @author Dan R. Lipsa
 *
 * Declaration of the SemanticError exception
 */
#ifndef __SEMANTIC_ERROR__
#define __SEMANTIC_ERROR__

#include <string>
#include <exception>
using namespace std;

/**
 * Exception that signals  a semantic error when parsing  the DMP data
 * file (produced by the Surface Evolver software)
 */
class SemanticError : public exception
{
public:
    /**
     * Constructs a SemanticError object
     * @param reason message describing the reason for the exception
     */
    SemanticError (const char* reason): m_reason(reason) {}
    /**
     * Constructs a SemanticError object
     * @param reason message describing the reason for the exception
     */
    SemanticError (const string& reason): m_reason(reason) {}


    /**
     * Destroys the object
     */
    ~SemanticError() throw() {}
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
