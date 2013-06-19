/**
 * @file   HistogramSettings.h
 * @author Dan R. Lipsa
 * @date 19 August 2010
 * @brief Dialog for modifying Histogram settings
 * @ingroup ui
 */

#ifndef __HISTOGRAM_SETTINGS_H__
#define __HISTOGRAM_SETTINGS_H__

#include "ui_HistogramSettings.h"

/**
 * @brief Dialog for modifying Histogram settings
 */
class HistogramSettings : 
    public QDialog, private Ui::HistogramSettings
{
public:
    enum State
    {
	MAX_VALUE,
	VALUE
    };

public:
    HistogramSettings (QWidget* parent);

    size_t GetYValue () const
    {
	return m_yAxisValue;
    }
    void SetYValue (size_t value);
    void SetYAxisMaxValue (size_t maxValue)
    {
	m_yAxisMaxValue = maxValue;
    }
    size_t GetYMaximumValue () const
    {
	return m_yAxisMaxValue;
    }
    State GetYState () const
    {
	return m_yState;
    }
    bool IsYAxisLogScale () const
    {
	return m_yAxisLogScale;
    }
    void SetYAxisLogScale (bool logScale);


public Q_SLOTS:
    void ToggledYAxisLogScale (bool checked);
    void ToggledYMaxValue (bool checked);
    void ToggledYValue (bool checked);
    void EditingFinishedYValue ();
    void FocusInYValue ();

private:
    void setValue (size_t value);

private:
    Q_OBJECT
    bool m_yAxisLogScale;
    QIntValidator m_validator;
    size_t m_yAxisValue;
    size_t m_yAxisMaxValue;
    State m_yState;
};


#endif //__HISTOGRAM_SETTINGS_H__

// Local Variables:
// mode: c++
// End:
