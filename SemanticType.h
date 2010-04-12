/**
 * @file   SemanticType.h
 * @author Dan R. Lipsa
 * @date 25 March 2010
 *
 * Types of attributes
 */
#ifndef __SEMANTIC_TYPE_H__
#define __SEMANTIC_TYPE_H__

struct SemanticType
{
    /**
     * Types of attributes
     */
    enum Name
    {
        INT,
        REAL,
        COLOR,
        INT_ARRAY,
        REAL_ARRAY,
	METHOD_OR_QUANTITY_NAME,
        COUNT
    };

    /**
     * Pretty print for a Type
     * @param ostr where to print
     * @param type what to print
     * @return where to print next
     */
    friend ostream& operator<< (ostream& ostr, SemanticType::Name type);
};

#endif //__SEMANTIC_TYPE_H__

// Local Variables:
// mode: c++
// End:
