/**
 * @file SemanticError.h
 * @author Dan R. Lipsa
 *
 * Declaration of the SemanticError exception
 */
#ifndef __SEMANTIC_ERROR_H__
#define __SEMANTIC_ERROR_H__


/**
 * Exception that signals  a semantic error when parsing  the .DMP data
 * file (produced by the Surface Evolver software)
 */
class SemanticError : public logic_error
{
public:
    /**
     * Constructs a SemanticError object
     * @param reason message describing the reason for the exception
     */
    SemanticError (const char* reason): logic_error(reason) {}
    /**
     * Constructs a SemanticError object
     * @param reason message describing the reason for the exception
     */
    SemanticError (const string& reason): logic_error(reason) {}
    /**
     * Destroys the object
     */
    ~SemanticError() throw() {}
};

#endif //__SEMANTIC_ERROR_H__

// Local Variables:
// mode: c++
// End:
