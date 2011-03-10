/**
 * @file   ViewSettings.h
 * @author Dan R. Lipsa
 * @date 10 March 2011
 *
 * Per view settings
 */
#ifndef __VIEW_SETTINGS_H__
#define __VIEW_SETTINGS_H__

#include "Enums.h"
class GLWidget;
class DisplayFaceStatistics;
class ColorBarModel;

class ViewSettings
{
public:
    ViewSettings (const GLWidget& glWidget);
    ~ViewSettings ();

    ViewType::Enum GetViewType () const
    {
	return m_viewType;
    }
    void SetViewType (ViewType::Enum viewType)
    {
	m_viewType = viewType;
    }

    BodyProperty::Enum GetBodyProperty () const
    {
	return m_bodyProperty;
    }
    void SetBodyProperty (BodyProperty::Enum bodyProperty)
    {
	m_bodyProperty = bodyProperty;
    }

    boost::shared_ptr<DisplayFaceStatistics> GetDisplayFaceStatistics () const
    {
	return m_displayFaceStatistics;
    }

    GLuint GetListCenterPaths () const
    {
	return m_listCenterPaths;
    }

    boost::shared_ptr<ColorBarModel> GetColorBarModel () const
    {
	return m_colorBarModel;
    }
    void SetColorBarModel (const boost::shared_ptr<ColorBarModel>& colorBarModel)
    {
	m_colorBarModel = colorBarModel;
    }
    void ResetColorBarModel ()
    {
	m_colorBarModel.reset ();
    }

    StatisticsType::Enum GetStatisticsType () const
    {
	return m_statisticsType;
    }
    void SetStatisticsType (StatisticsType::Enum statisticsType)
    {
	m_statisticsType = statisticsType;
    }
    GLuint GetColorBarTexture () const
    {
	return m_colorBarTexture;
    }

private:
    void initTexture ();
    void initList ();

private:
    ViewType::Enum m_viewType;
    BodyProperty::Enum m_bodyProperty;
    StatisticsType::Enum m_statisticsType;
    GLuint m_listCenterPaths;
    GLuint m_colorBarTexture;
    boost::shared_ptr<DisplayFaceStatistics> m_displayFaceStatistics;
    boost::shared_ptr<ColorBarModel> m_colorBarModel;
};


#endif //__VIEW_SETTINGS_H__

// Local Variables:
// mode: c++
// End:
