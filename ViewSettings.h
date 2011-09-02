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
#include "AverageInterface.h"
class AllBodySelector;
class BodySelector;
class ColorBarModel;
class ScalarAverage;
class T1sPDE;
class TensorAverage;
class ForceAverage;
class GLWidget;
class IdBodySelector;
class PropertyValueBodySelector;



class ViewSettings : public AverageInterface
{
public:
    // Average around
    enum AverageAroundType
    {
	AVERAGE_AROUND_TRANSLATION,
	AVERAGE_AROUND_ROTATION,
	AVERAGE_AROUND_NONE
    };

    // ContextStationary
    enum ContextStationaryType
    {
	CONTEXT_AVERAGE_AROUND_FOAM,
	CONTEXT_AVERAGE_AROUND_NONE
    };


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

    size_t GetBodyOrFaceProperty () const
    {
	return m_bodyOrFaceProperty;
    }
    void SetBodyOrFaceProperty (size_t bodyOrFaceProperty)
    {
	m_bodyOrFaceProperty = bodyOrFaceProperty;
    }

    ScalarAverage& GetScalarAverage () const
    {
	return *m_scalarAverage;
    }
    
    T1sPDE& GetT1sPDE () const
    {
	return *m_t1sPDE;
    }

    TensorAverage& GetTensorAverage () const
    {
	return *m_tensorAverage;
    }

    ForceAverage& GetForceAverage () const
    {
	return *m_forceAverage;
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


    double GetGridScaleRatio () const
    {
	return m_gridScaleRatio;
    }
    void SetGridScaleRatio (double gridScaleRatio)
    {
	m_gridScaleRatio = gridScaleRatio;
    }

    double GetContextScaleRatio () const
    {
	return m_contextScaleRatio;
    }
    
    void SetContextScaleRatio (double contextScaleRatio)
    {
	m_contextScaleRatio = contextScaleRatio;
    }


    const G3D::Vector3& GetTranslation () const
    {
	return m_translation;
    }
    const G3D::Vector3& GetGridTranslation () const
    {
	return m_gridTranslation;
    }
    
    void SetTranslation (const G3D::Vector3& translation)
    {
	m_translation = translation;
    }
    void SetGridTranslation (const G3D::Vector3& gridTranslation)
    {
	m_gridTranslation = gridTranslation;
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
    G3D::Matrix3 GetRotationForAxesOrder (const Foam& foam) const;

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

    void CopyTransformation (const ViewSettings& from);
    void CopyColorBar (const ViewSettings& from);

    size_t GetAverageAroundBodyId () const
    {
	return m_averageAroundBodyId;
    }
    void SetAverageAroundBodyId (size_t id)
    {
	m_averageAroundBodyId = id;
    }

    AverageAroundType GetAverageAroundType () const
    {
	return m_averageAroundType;
    }
    void SetAverageAroundType (AverageAroundType type)
    {
	m_averageAroundType = type;
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
    void SetForceNetworkShown (bool value)
    {
	m_forceNetworkShown = value;
    }
    bool IsForceNetworkShown () const
    {
	return m_forceNetworkShown;
    }
    void SetForcePressureShown (bool value)
    {
	m_forcePressureShown = value;
    }
    bool IsForcePressureShown () const
    {
	return m_forcePressureShown;
    }
    void SetForceResultShown (bool value)
    {
	m_forceResultShown = value;
    }
    bool IsForceResultShown () const
    {
	return m_forceResultShown;
    }

    void SetDeformationTensorShown (bool deformationTensorShown)
    {
	m_deformationTensorShown = deformationTensorShown;
    }
    bool IsDeformationTensorShown () const
    {
	return m_deformationTensorShown;
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

    ContextStationaryType GetContextStationaryType () const
    {
	return m_contextStationaryType;
    }
    void SetContextStationaryType (ContextStationaryType type)
    {
	m_contextStationaryType = type;
    }
    
    const BodySelector& GetBodySelector () const
    {
	return *m_bodySelector;
    }
    void SetBodySelector (
	boost::shared_ptr<PropertyValueBodySelector> selector);
    void SetBodySelector (boost::shared_ptr<IdBodySelector> bodySelector);
    void SetBodySelector (boost::shared_ptr<AllBodySelector> selector, 
			  BodySelectorType::Enum type);
    void UnionBodySelector (const vector<size_t>& bodyIds);
    void DifferenceBodySelector (const Foam& foam, 
				 const vector<size_t>& bodyIds);
    void CopySelection (const ViewSettings& other);
    bool IsContextHidden () const
    {
	return m_contextHidden;
    }
    void SetContextHidden (bool contextHidden)
    {
	m_contextHidden = contextHidden;
    }
    bool IsCenterPathHidden () const
    {
	return m_centerPathHidden;
    }
    void SetCenterPathHidden (bool centerPathHidden)
    {
	m_centerPathHidden = centerPathHidden;
    }

    virtual void Init (ViewNumber::Enum viewNumber);
    virtual void SetTimeWindow (size_t timeSteps);
    virtual void Step (ViewNumber::Enum viewNumber, int timeStep);
    virtual void RotateAndDisplay (
	ViewNumber::Enum viewNumber, StatisticsType::Enum displayType,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;
    virtual void Release ();


private:
    static G3D::Matrix3 getRotation2DTimeDisplacement ();
    static G3D::Matrix3 getRotation2DRight90 ();
    static G3D::Matrix3 getRotation2DLeft90 ();

    void initTexture ();
    void initList ();
    void setInitialLightParameters ();
    /**
     * Rotates the view so that we get the same image as in Surface Evolver
     */
    G3D::Matrix3 getRotation3D (const Foam& foam) const;

private:
    ViewType::Enum m_viewType;
    size_t m_bodyOrFaceProperty;
    StatisticsType::Enum m_statisticsType;
    GLuint m_listCenterPaths;
    GLuint m_colorBarTexture;
    boost::shared_ptr<ScalarAverage> m_scalarAverage;
    boost::shared_ptr<T1sPDE> m_t1sPDE;
    boost::shared_ptr<TensorAverage> m_tensorAverage;
    boost::shared_ptr<ForceAverage> m_forceAverage;
    boost::shared_ptr<ColorBarModel> m_colorBarModel;
    G3D::Matrix3 m_rotationModel;
    G3D::Rect2D m_viewport;
    double m_scaleRatio;
    double m_gridScaleRatio;
    double m_contextScaleRatio;
    G3D::Vector3 m_translation;
    G3D::Vector3 m_gridTranslation;
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
    AverageAroundType m_averageAroundType;
    size_t m_averageAroundBodyId;
    // Context view
    bool m_contextView;
    bool m_forceNetworkShown;
    bool m_forcePressureShown;
    bool m_forceResultShown;
    bool m_deformationTensorShown;
    // Context display
    set<size_t> m_contextBody;
    // Context stationary
    ContextStationaryType m_contextStationaryType;
    boost::shared_ptr<BodySelector> m_bodySelector;
    bool m_contextHidden;
    bool m_centerPathHidden;
};


#endif //__VIEW_SETTINGS_H__

// Local Variables:
// mode: c++
// End:
