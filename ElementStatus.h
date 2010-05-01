/**
 * @file ElementStatus.h
 * @author Dan R. Lipsa
 * @date 1 May 2010
 * 
 * Declaration for ElementStatus
 */
#ifndef __ELEMENT_STATUS_H__
#define __ELEMENT_STATUS_H__

/**
 * Status of vertices, edges, faces and bodies.
 * 
 */
struct ElementStatus
{
    enum Name
    {
	ORIGINAL,
	DUPLICATE_MADE,
	DUPLICATE
    };
    friend ostream& operator<< (ostream& ostr, ElementStatus::Name status);
};


#endif //__ELEMENT_STATUS_H__

// Local Variables:
// mode: c++
// End:
