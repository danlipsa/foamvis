/**
 * @file   InteractionMode.h
 * @author Dan R. Lipsa
 * @data 28 April 2010
 * 
 * Ways to interact with the interafce
 */
#ifndef __INTERACTION_MODE_H__
#define __INTERACTION_MODE_H__

/**
 * Ways to interact with the interface
 */
class InteractionMode
{
public:
    /**
     * Names of colors
     */
    enum Name
    {
	ROTATE,
	SCALE,
	SCALE_VIEWPORT,
	TRANSLATE_VIEWPORT,
	INTERACTION_MODE_COUNT
    };
};

#endif //__INTERACTION_MODE_H__

// Local Variables:
// mode: c++
// End:
