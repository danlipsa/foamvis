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
#include "ObjectPosition.h"
class AllBodySelector;
class BodySelector;
class ColorBarModel;
class ForceAverage;
class GLWidget;
class IdBodySelector;
class PropertyValueBodySelector;
class ScalarAverage;
class Simulation;
class T1sPDE;
class TensorAverage;
class VectorAverage;


class ViewSettings : public AverageInterface
{
public:
    // Average around
    enum AverageAroundType
    {
	AVERAGE_AROUND,
	AVERAGE_AROUND_NONE
    };

    // Average around movement
    enum AverageAroundMovementShown
    {
	AVERAGE_AROUND_MOVEMENT_ROTATION,
	AVERAGE_AROUND_MOVEMENT_NONE
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

    TensorAverage& GetDeformationAverage () const
    {
	return *m_deformationAverage;
    }
    VectorAverage& GetVelocityAverage () const
    {
	return *m_velocityAverage;
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
    float GetCameraDistance () const
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
	return m_averageAroundBodyId[0];
    }    
    void SetAverageAroundBodyId (size_t id)
    {
	m_averageAroundBodyId[0] = id;
    }
    size_t GetAverageAroundSecondBodyId () const
    {
	return m_averageAroundBodyId[1];
    }
    void SetAverageAroundSecondBodyId (size_t id)
    {
	m_averageAroundBodyId[1] = id;
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

    bool IsDeformationShown () const
    {
	return m_deformationTensorShown;
    }

    void SetVelocityShown (bool velocityShown)
    {
	m_velocityShown = velocityShown;
    }

    bool IsVelocityShown () const
    {
	return m_velocityShown;
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

    AverageAroundMovementShown GetAverageAroundMovementShown () const
    {
	return m_averageAroundMovementShown;
    }
    void SetAverageAroundMovementShown (AverageAroundMovementShown movement)
    {
	m_averageAroundMovementShown = movement;
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

    size_t GetSimulationIndex () const
    {
	return m_simulationIndex;
    }
    void SetSimulationIndex (size_t i)
    {
	m_simulationIndex = i;
    }

    size_t GetCurrentTime () const;
    void SetCurrentTime (size_t time, ViewNumber::Enum viewNumber);
    bool IsT1sShiftLower () const
    {
	return m_t1sShiftLower;
    }
    void SetT1sShiftLower (bool t1sShiftLower)
    {
	m_t1sShiftLower = t1sShiftLower;
    }

    ObjectPosition GetAverageAroundPosition (size_t timeStep) const
    {
	return m_averageAroundPositions[timeStep];
    }
    void SetAverageAroundPositions (const Simulation& simulation);
    void SetAverageAroundPositions (const Simulation& simulation, size_t bodyId);
    void SetAverageAroundPositions (const Simulation& simulation,
				    size_t bodyId, size_t secondBodyId);
    void SetLinkedTimeBegin (size_t begin)
    {
	m_syncViewTimeBegin = begin;
    }
    size_t GetLinkedTimeBegin () const
    {
	return m_syncViewTimeBegin;
    }
    void SetLinkedTimeEnd (size_t end)
    {
	m_syncViewTimeEnd = end;
    }
    size_t GetLinkedTimeEnd () const
    {
	return m_syncViewTimeEnd;
    }
    size_t GetLinkedTimeInterval () const
    {
	return m_syncViewTimeEnd - m_syncViewTimeBegin + 1;
    }
    float GetDeformationSize () const
    {
	return m_deformationSize;
    }
    void SetDeformationSize (float value)
    {
	m_deformationSize = value;
    }
    float GetDeformationLineWidth () const
    {
	return m_deformationLineWidth;
    }
    void SetDeformationLineWidth (float value)
    {
	m_deformationLineWidth = value;
    }
    float GetVelocitySize () const
    {
	return m_velocitySize;
    }
    void SetVelocitySize (float value)
    {
	m_velocitySize = value;
    }
    float GetVelocityLineWidth () const
    {
	return m_velocityLineWidth;
    }
    void SetVelocityLineWidth (float value)
    {
	m_velocityLineWidth = value;
    }
    float GetForceSize () const
    {
	return m_forceSize;
    }
    void SetForceSize (float value)
    {
	m_forceSize = value;
    }
    float GetForceLineWidth () const
    {
	return m_forceLineWidth;
    }
    void SetForceLineWidth (float value)
    {
	m_forceLineWidth = value;
    }
    G3D::Vector2 GetScaleCenter () const
    {
	return m_scaleCenter;
    }
    void SetScaleCenter (G3D::Vector2 scaleCenter)
    {
	m_scaleCenter = scaleCenter;
    }
    virtual void AverageInit (ViewNumber::Enum viewNumber);
    virtual void AverageSetTimeWindow (size_t timeSteps);
    virtual void AverageStep (ViewNumber::Enum viewNumber, int direction);
    virtual void AverageRotateAndDisplay (
	ViewNumber::Enum viewNumber, 
	StatisticsType::Enum displayType = StatisticsType::AVERAGE,
	G3D::Vector2 rotationCenter = G3D::Vector2::zero (), 
	float angleDegrees = 0) const;
    virtual void AverageRelease ();


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
    boost::shared_ptr<TensorAverage> m_deformationAverage;
    boost::shared_ptr<VectorAverage> m_velocityAverage;
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
    float m_cameraDistance;

    //Stationary
    AverageAroundType m_averageAroundType;
    boost::array<size_t, 2> m_averageAroundBodyId;
    // Context view
    bool m_contextView;
    bool m_forceNetworkShown;
    bool m_forcePressureShown;
    bool m_forceResultShown;
    bool m_deformationTensorShown;
    bool m_velocityShown;
    // Context display
    set<size_t> m_contextBody;
    // Context stationary
    AverageAroundMovementShown m_averageAroundMovementShown;
    boost::shared_ptr<BodySelector> m_bodySelector;
    bool m_contextHidden;
    bool m_centerPathHidden;
    // Simulation related variables
    size_t m_simulationIndex;
    vector<ObjectPosition> m_averageAroundPositions;
    /**
     * Index into m_foam that shows the current DMP file displayed
     */
    size_t m_currentTime;
    bool m_t1sShiftLower;
    size_t m_syncViewTimeBegin;
    size_t m_syncViewTimeEnd;
    float m_deformationSize;
    float m_deformationLineWidth;
    float m_velocitySize;
    float m_velocityLineWidth;
    float m_forceSize;
    float m_forceLineWidth;
    G3D::Vector2 m_scaleCenter;
};


#endif //__VIEW_SETTINGS_H__

// Local Variables:
// mode: c++
// End:
