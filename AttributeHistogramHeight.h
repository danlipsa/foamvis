/**
 * @file   AttributeHistogramHeight.h
 * @author Dan R. Lipsa
 * @date 19 August 2010
 *
 * Declaration of the AttributeHistogramHeight class
 */

#ifndef __ATTRIBUTE_HISTOGRAM_HEIGHT_H__
#define __ATTRIBUTE_HISTOGRAM_HEIGHT_H__

#include "ui_AttributeHistogramHeight.h"

class AttributeHistogramHeight : 
    public QDialog, private Ui::AttributeHistogramHeight
{
public:
    AttributeHistogramHeight (QWidget* parent) :
	QDialog (parent)
    {
	setupUi (this);
    }
};

#endif //__ATTRIBUTE_HISTOGRAM_HEIGHT_H__

// Local Variables:
// mode: c++
// End:
