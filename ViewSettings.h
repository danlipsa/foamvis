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
    void SetColorBarModel (
	const boost::shared_ptr<ColorBarModel>& colorBarModel);
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

    const G3D::Matrix3& GetRotationModel () const
    {
	return m_rotationModel;
    }
    void SetRotationModel (const G3D::Matrix3& rotationModel)
    {
	m_rotationModel = rotationModel;
    }

    const G3D::Matrix3& GetRotationLight (LightNumber::Enum i) const
    {
	return m_rotationLight[i];
    }
    
    void SetRotationLight (LightNumber::Enum i, const G3D::Matrix3& rl)
    {
	m_rotationLight[i] = rl;
    }

    const G3D::Rect2D& GetViewport () const
    {
	return m_viewport;
    }

    void SetViewport (const G3D::Rect2D& v)
    {
	m_viewport = v;
    }
    

    double GetScaleRatio () const
    {
	return m_scaleRatio;
    }

    void SetScaleRatio (double scaleRatio)
    {
	m_scaleRatio = scaleRatio;
    }

    const G3D::Vector3& GetTranslation () const
    {
	return m_translation;
    }
    
    void SetTranslation (const G3D::Vector3& translation)
    {
	m_translation = translation;
    }
    bool IsLightingEnabled () const
    {
	return m_lightingEnabled;
    }

    bool IsLightEnabled (LightNumber::Enum i) const
    {
	return m_lightEnabled[i];
    }
    void SetLightEnabled (LightNumber::Enum i, bool enabled);
    bool IsDirectionalLightEnabled (LightNumber::Enum i) const
    {
	return m_directionalLightEnabled [i];
    }
    void SetDirectionalLightEnabled (LightNumber::Enum i, bool enabled)
    {
	m_directionalLightEnabled [i] = enabled;
    }
    bool IsLightPositionShown (LightNumber::Enum i) const
    {
	return m_lightPositionShown[i];
    }
    void SetLightPositionShown (LightNumber::Enum i, bool shown)
    {
	m_lightPositionShown[i] = shown;
    }
    void PositionLight (LightNumber::Enum i, 
			const G3D::Vector3& initialLightPosition);
    const boost::array<GLfloat, 4> GetLight (
	LightNumber::Enum lightNumber, LightType::Enum lightType) const
    {
	return m_light[lightNumber][lightType];
    }
    void SetLight (
	LightNumber::Enum lightNumber, LightType::Enum lightType, 
	size_t colorIndex, GLfloat color)
    {
	m_light[lightNumber][lightType][colorIndex] = color;
    }
    double GetLightNumberRatio (LightNumber::Enum i) const
    {
	return m_lightPositionRatio [i];
    }
    void SetLightNumberRatio (LightNumber::Enum i, double lpr)
    {
	m_lightPositionRatio [i] = lpr;
    }
    LightNumber::Enum GetSelectedLight () const
    {
	return m_selectedLight;
    }
    void SetSelectedLight (LightNumber::Enum i)
    {
	m_selectedLight = i;
    }
    void SetInitialLightPosition (LightNumber::Enum i);
    void SetAxesOrder (AxesOrder::Enum axesOrder)
    {
	m_axesOrder = axesOrder;
    }
    AxesOrder::Enum GetAxesOrder () const
    {
	return m_axesOrder;
    }
    double GetAngleOfView () const
    {
	return m_angleOfView;
    }
    void SetAngleOfView (double value)
    {
	m_angleOfView = value;
    }
    double GetCameraDistance () const
    {
	return m_cameraDistance;
    }
    void CalculateCameraDistance (const G3D::AABox& centeredViewingVolume);
    void SetLightingParameters (const G3D::Vector3& initialLightPosition);
    void EnableLighting ();

    void CopyTransformations (const ViewSettings& from);
    void CopyColorBar (const ViewSettings& from);

    // Stationary
    enum StationaryType
    {
	STATIONARY_BODY,
	STATIONARY_CONSTRAINT,
	STATIONARY_NONE
    };

    size_t GetStationaryBodyId () const
    {
	return m_stationaryBodyId;
    }
    void SetStationaryBodyId (size_t id)
    {
	m_stationaryBodyId = id;
    }

    StationaryType GetStationaryType () const
    {
	return m_stationaryType;
    }
    void SetStationaryType (StationaryType type)
    {
	m_stationaryType = type;
    }

    // Context View
    void SetContextView (bool contextView)
    {
	m_contextView = contextView;
    }
    bool IsContextView () const
    {
	return m_contextView;
    }
    double GetContextScaleRatio () const
    {
	return m_contextScaleRatio;
    }
    
    void SetContextScaleRatio (double contextScaleRatio)
    {
	m_contextScaleRatio = contextScaleRatio;
    }

    // ContextDisplay
    void AddContextDisplayBody (size_t bodyId)
    {
	m_contextBody.insert (bodyId);
    }
    void ContextDisplayReset ()
    {
	m_contextBody.clear ();
    }
    bool IsContextDisplayBody (size_t bodyId) const;
    size_t GetContextDisplayBodySize () const
    {
	return m_contextBody.size ();
    }

    // ContextStationary
    enum ContextStationaryType
    {
	CONTEXT_STATIONARY_FOAM,
	CONTEXT_STATIONARY_NONE
    };
    ContextStationaryType GetContextStationaryType () const
    {
	return m_contextStationaryType;
    }
    void SetContextStationaryType (ContextStationaryType type)
    {
	m_contextStationaryType = type;
    }

    

private:
    void initTexture ();
    void initList ();
    void setInitialLightParameters ();

private:
    ViewType::Enum m_viewType;
    BodyProperty::Enum m_bodyProperty;
    StatisticsType::Enum m_statisticsType;
    GLuint m_listCenterPaths;
    GLuint m_colorBarTexture;
    boost::shared_ptr<DisplayFaceStatistics> m_displayFaceStatistics;
    boost::shared_ptr<ColorBarModel> m_colorBarModel;
    G3D::Matrix3 m_rotationModel;
    G3D::Rect2D m_viewport;
    double m_scaleRatio;
    G3D::Vector3 m_translation;
    // lighting state
    bool m_lightingEnabled;
    LightNumber::Enum m_selectedLight;
    bitset<LightNumber::COUNT> m_lightEnabled;
    bitset<LightNumber::COUNT> m_directionalLightEnabled;
    bitset<LightNumber::COUNT> m_lightPositionShown;    
    boost::array<G3D::Matrix3, LightNumber::COUNT> m_rotationLight;
    boost::array<double, LightNumber::COUNT> m_lightPositionRatio;
    boost::array<
	boost::array<boost::array<GLfloat, 4>, LightNumber::COUNT>, 
	LightType::COUNT> m_light;

    //  requires camera distance
    double m_angleOfView;
    AxesOrder::Enum m_axesOrder;
    /**
     * Distance from the camera to the center of the bounding box for the foam.
     */
    double m_cameraDistance;

    //Stationary
    StationaryType m_stationaryType;
    size_t m_stationaryBodyId;
    // Context view
    bool m_contextView;
    double m_contextScaleRatio;
    // Context display
    set<size_t> m_contextBody;
    // Context stationary
    ContextStationaryType m_contextStationaryType;
};


#endif //__VIEW_SETTINGS_H__

// Local Variables:
// mode: c++
// End:
