#ifndef __SEMANTIC_ERROR__
#define __SEMANTIC_ERROR__

#include <string>
#include <exception>

class SemanticError : public std::exception
{
public:
    SemanticError() throw();
    ~SemanticError() throw() {}
    SemanticError (const char* reason): m_reason(reason) {}

    virtual const char* what()
    {
	return m_reason.c_str();
    }
private:
    std::string m_reason;
};

#endif
