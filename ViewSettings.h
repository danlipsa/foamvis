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
#include "ObjectPosition.h"
class AllBodySelector;
class BodySelector;
class ColorBarModel;
class Foam;
class WidgetGl;
class IdBodySelector;
class PropertyValueBodySelector;
class Simulation;

/**
 * Settings that apply to one view
 */
class ViewSettings : public QObject
{
public:
    enum RotationCenterType
    {
	ROTATION_CENTER_FOAM,
	ROTATION_CENTER_BODY
    };

    enum RotateAndTranslateOperation
    {
        TRANSLATE,
        DONT_TRANSLATE
    };


public:
    ViewSettings ();
    ~ViewSettings ();

    /**
     * @{
     * @name Type
     */
    ViewType::Enum GetViewType () const
    {
	return m_viewType;
    }
    void SetViewType (ViewType::Enum viewType)
    {
	m_viewType = viewType;
    }

    size_t GetBodyOrFaceScalar () const
    {
	return m_bodyOrFaceScalar;
    }
    void SetBodyOrFaceScalar (size_t bodyOrFaceScalar)
    {
	m_bodyOrFaceScalar = bodyOrFaceScalar;
    }
    
    StatisticsType::Enum GetStatisticsType () const
    {
	return m_statisticsType;
    }
    void SetStatisticsType (StatisticsType::Enum statisticsType)
    {
	m_statisticsType = statisticsType;
    }
    ColorBarType::Enum GetColorBarType () const;    
    static ColorBarType::Enum GetColorBarType (
        ViewType::Enum viewType, size_t property,
        StatisticsType::Enum statisticsType);
    // @}

    /**
     * @{
     * @name Overlays
     */
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

    void SetTorqueNetworkShown (bool value)
    {
	m_torqueNetworkShown = value;
    }
    bool IsTorqueNetworkShown () const
    {
	return m_torqueNetworkShown;
    }
    void SetTorquePressureShown (bool value)
    {
	m_torquePressureShown = value;
    }
    bool IsTorquePressureShown () const
    {
	return m_torquePressureShown;
    }
    void SetTorqueResultShown (bool value)
    {
	m_torqueResultShown = value;
    }
    bool IsTorqueResultShown () const
    {
	return m_torqueResultShown;
    }

    void SetForceDifferenceShown (bool value)
    {
	m_forceDifferenceShown = value;
    }
    bool IsForceDifferenceShown () const
    {
	return m_forceDifferenceShown;
    }

    void SetDeformationTensorShown (bool deformationTensorShown)
    {
	m_deformationShown = deformationTensorShown;
    }

    bool IsDeformationShown () const
    {
	return m_deformationShown;
    }

    void SetVelocityShown (bool velocityShown)
    {
	m_velocityShown = velocityShown;
    }

    bool IsVelocityShown () const
    {
	return m_velocityShown;
    }
    
    void SetVelocityVis (VectorVis::Enum vis)
    {
        m_velocityVis = vis;
    }
    VectorVis::Enum GetVelocityVis () const
    {
        return m_velocityVis;
    }
    float GetVelocityClampingRatio () const;
    float GetVelocityLineWidth () const
    {
	return m_velocityLineWidth;
    }
    void SetVelocityLineWidth (float value)
    {
	m_velocityLineWidth = value;
    }
    double GetStreamlineMaxPropagation () const
    {
        return m_streamlineMaxPropagation;
    }
    void SetStreamlineMaxPropagation (double value)
    {
        m_streamlineMaxPropagation = value;
    }
    int GetStreamlineMaxSteps () const
    {
        return m_streamlineMaxSteps;
    }
    void SetStreamlineMaxSteps (int steps)
    {
        m_streamlineMaxSteps = steps;
    }
    // @}

    /**
     * @{
     * @name ColorBar
     */
    boost::shared_ptr<ColorBarModel> GetColorBarModel () const
    {
	return m_colorBarModel;
    }
    boost::shared_ptr<ColorBarModel> GetOverlayBarModel () const
    {
	return m_velocityOverlayBarModel;
    }
    void SetColorBarModel (
	const boost::shared_ptr<ColorBarModel>& colorBarModel);
    void SetOverlayBarModel (
	const boost::shared_ptr<ColorBarModel>& colorBarModel);
    void ResetColorBarModel ()
    {
	m_colorBarModel.reset ();
    }
    void CopyColorBar (const ViewSettings& from);
    // @}

    /**
     * @{
     * @name Transforms focus
     */
    // rotation
    const G3D::Matrix3& GetRotation () const
    {
	return m_rotationFocus;
    }
    void SetRotation (const G3D::Matrix3& rotationFocus)
    {
	m_rotationFocus = rotationFocus;
    }
    G3D::Vector3 GetRotationCenter () const
    {
	return m_rotationCenter;
    }
    void SetRotationCenter (const G3D::Vector3& rotationCenter)
    {
	m_rotationCenter = rotationCenter;
    }
    void SetRotationCenterType (RotationCenterType type)
    {
	m_rotationCenterType = type;
    }
    RotationCenterType GetRotationCenterType () const
    {
	return m_rotationCenterType;
    }
    void SetAxesOrder (AxesOrder::Enum axesOrder)
    {
	m_axesOrder = axesOrder;
    }
    AxesOrder::Enum GetAxesOrder () const
    {
	return m_axesOrder;
    }
    G3D::Matrix3 GetRotationForAxesOrder (const Foam& foam) const;

    // scale
    float GetScaleRatio () const
    {
	return m_scaleRatio;
    }
    void SetScaleRatio (float scaleRatio)
    {
	m_scaleRatio = scaleRatio;
    }
    G3D::Vector2 GetScaleCenter () const
    {
	return m_scaleCenter;
    }
    void SetScaleCenter (G3D::Vector2 scaleCenter)
    {
	m_scaleCenter = scaleCenter;
    }
    
    // translation
    const G3D::Vector3& GetTranslation () const
    {
	return m_translation;
    }   
    void SetTranslation (const G3D::Vector3& translation)
    {
	m_translation = translation;
    }

    // projection - angle of view
    double GetAngleOfView () const
    {
	return m_angleOfView;
    }
    void SetAngleOfView (double value)
    {
	m_angleOfView = value;
    }
    // projection - camera distance
    float GetCameraDistance () const
    {
	return m_cameraDistance;
    }
    void CalculateCameraDistance (const G3D::AABox& centeredViewingVolume);

    void CopyTransformation (const ViewSettings& from);
    // @}

    /**
     * @{
     * @name Transforms grid
     */
    double GetGridScaleRatio () const
    {
	return m_gridScaleRatio;
    }
    void SetGridScaleRatio (double gridScaleRatio)
    {
	m_gridScaleRatio = gridScaleRatio;
    }

    const G3D::Vector3& GetGridTranslation () const
    {
	return m_gridTranslation;
    }
    void SetGridTranslation (const G3D::Vector3& gridTranslation)
    {
	m_gridTranslation = gridTranslation;
    }
    // @}

    /**
     * @{
     * @name Context view
     */
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
    // @}

    /**
     * @{
     * @name Lights
     */
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
    void SetGlLightParameters (LightNumber::Enum i, 
			       G3D::AABox centeredViewingVolume) const;
    void SetGlLightParameters (G3D::AABox centeredViewingVolume) const;
    void SetInitialLightParameters (LightNumber::Enum i);
    const boost::array<GLfloat, 4> GetLight (
	LightNumber::Enum lightNumber, LightType::Enum lightType) const
    {
	return m_light[lightNumber][lightType];
    }
    void SetLight (
	LightNumber::Enum lightNumber, LightType::Enum lightType, 
	size_t colorIndex, GLfloat color);
    void SetLight (
	LightNumber::Enum lightNumber, LightType::Enum lightType, 
	const boost::array<GLfloat,4>& color);
    LightNumber::Enum GetSelectedLight () const
    {
	return m_selectedLight;
    }
    void SetSelectedLight (LightNumber::Enum i)
    {
	m_selectedLight = i;
    }
    static G3D::Vector3 GetInitialLightPosition (
	G3D::AABox centeredViewingVolume, LightNumber::Enum lightNumber);
    double GetLightPositionRatio (LightNumber::Enum lightNumber) const
    {
	return m_lightPositionRatio [lightNumber];
    }
    void SetLightPositionRatio (LightNumber::Enum lightNumber, double lpr)
    {
	m_lightPositionRatio [lightNumber] = lpr;
    }

    const G3D::Matrix3& GetRotationLight (LightNumber::Enum i) const
    {
	return m_rotationLight[i];
    }
    void SetRotationLight (LightNumber::Enum i, const G3D::Matrix3& rl)
    {
	m_rotationLight[i] = rl;
    }
    // @}


    /**
     * @{
     * @name Average around
     */
    bool IsAverageAround () const
    {
	return m_averageAround;
    }
    void SetAverageAround (bool averageAround)
    {
	m_averageAround = averageAround;
    }

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
    size_t GetDifferenceBodyId () const
    {
	return m_differenceBodyId;
    }
    void SetDifferenceBodyId (size_t id)
    {
	m_differenceBodyId = id;
    }
    bool IsAverageAroundRotationShown () const
    {
	return m_averageAroundRotationShown;
    }
    void SetAverageAroundRotationShown (bool shown)
    {
	m_averageAroundRotationShown = shown;
    }
    ObjectPosition GetAverageAroundPosition (size_t timeStep) const
    {
	return m_averageAroundPositions[timeStep];
    }
    void SetAverageAroundPositions (const Simulation& simulation);
    void SetAverageAroundPositions (const Simulation& simulation, size_t bodyId);
    void SetAverageAroundPositions (const Simulation& simulation,
				    size_t bodyId, size_t secondBodyId);
    void RotateAndTranslateAverageAround (
        size_t timeStep, int direction, RotateAndTranslateOperation op) const;
    // @}

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
    // @}
    
    /**
     * @{
     * @name Body selection
     */
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
    void UnionBodySelector (size_t bodyId);
    void DifferenceBodySelector (const Foam& foam, 
				 const vector<size_t>& bodyIds);
    void CopySelection (const ViewSettings& other);
    bool IsSelectionContextShown () const
    {
	return m_selectionContextShown;
    }
    void SetSelectionContextShown (bool shown)
    {
	m_selectionContextShown = shown;
    }
    // @}
    
    /**
     * @{
     * @name Histogram
     */
    bool IsHistogramShown () const
    {
	return m_histogramShown;
    }
    void SetHistogramShown (bool shown)
    {
	m_histogramShown = shown;
    }
    bool HasHistogramOption (HistogramType::Option option) const;
    void SetHistogramOption (HistogramType::Option option, bool on = true);
    void ResetHistogramOption (HistogramType::Option option);
    // @}

    /**
     * @{
     * @name Time and LinkedTime
     */
    size_t GetCurrentTime () const
    {
	return m_currentTime;
    }    
    
    /**
     * @return positive if time has moved forward or negative otherwise
     */
    int SetCurrentTime (size_t time);
    size_t GetTimeSteps () const
    {
	return m_timeSteps;
    }
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
    // @}


    /**
     * @{
     * @name Various
     */
    bool IsPartialPathHidden () const
    {
	return m_centerPathHidden;
    }
    void SetPartialPathHidden (bool centerPathHidden)
    {
	m_centerPathHidden = centerPathHidden;
    }

    size_t GetSimulationIndex () const
    {
	return m_simulationIndex;
    }

    bool T1sShiftLower () const
    {
	return m_t1sShiftLower;
    }
    void SetT1sShiftLower (bool t1sShiftLower)
    {
	m_t1sShiftLower = t1sShiftLower;
    }

    float AngleDisplay (float angle) const;

    void SetSimulation (int i, const Simulation& simulation,
			G3D::Vector3 viewingVolumeCenter,
			bool t1sShiftLower);
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
    float GetForceTorqueSize () const
    {
	return m_forceTorqueSize;
    }
    void SetForceTorqueSize (float value)
    {
	m_forceTorqueSize = value;
    }
    float GetTorqueDistance () const
    {
	return m_torqueDistance;
    }
    void SetTorqueDistance (float value)
    {
	m_torqueDistance = value;
    }
    float GetForceTorqueLineWidth () const
    {
	return m_forceTorqueLineWidth;
    }
    void SetForceTorqueLineWidth (float value)
    {
	m_forceTorqueLineWidth = value;
    }
    string GetTitle (ViewNumber::Enum viewNumber) const;
    bool DomainClipped () const
    {
        return m_domainClipped;
    }
    void SetDomainClipped (bool clipped)
    {
        m_domainClipped = clipped;
    }
    //@}

Q_SIGNALS:
    void SelectionChanged ();

private:
    static G3D::Matrix3 getRotation2DTimeDisplacement ();
    static G3D::Matrix3 getRotation2DRight90 ();
    static G3D::Matrix3 getRotation2DRight90Reflection ();
    static G3D::Matrix3 getRotation2DLeft90 ();

    void setInitialLightParameters ();
    /**
     * Rotates the view so that we get the same image as in Surface Evolver
     */
    G3D::Matrix3 getRotation3D (const Foam& foam) const;
    void setSimulationIndex (size_t i)
    {
	m_simulationIndex = i;
    }
    void setTimeSteps (size_t timeSteps)
    {
	m_timeSteps = timeSteps;
    }

private:
    Q_OBJECT
    ViewType::Enum m_viewType;
    size_t m_bodyOrFaceScalar;
    StatisticsType::Enum m_statisticsType;
    boost::shared_ptr<ColorBarModel> m_colorBarModel;
    boost::shared_ptr<ColorBarModel> m_velocityOverlayBarModel;
    G3D::Matrix3 m_rotationFocus;
    G3D::Vector3 m_rotationCenter;
    RotationCenterType m_rotationCenterType;
    float m_scaleRatio;
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
	boost::array<boost::array<GLfloat, 4>, LightType::COUNT>, 
	LightNumber::COUNT> m_light;

    //  requires camera distance
    double m_angleOfView;
    AxesOrder::Enum m_axesOrder;
    /**
     * Distance from the camera to the center of the bounding box for the foam.
     */
    float m_cameraDistance;

    //Stationary
    bool m_averageAround;
    boost::array<size_t, 2> m_averageAroundBodyId;
    size_t m_differenceBodyId;
    // Context view
    bool m_contextView;
    bool m_forceNetworkShown;
    bool m_forcePressureShown;
    bool m_forceResultShown;
    bool m_torqueNetworkShown;
    bool m_torquePressureShown;
    bool m_torqueResultShown;
    bool m_forceDifferenceShown;
    bool m_deformationShown;
    bool m_velocityShown;
    VectorVis::Enum m_velocityVis;
    // Context display
    set<size_t> m_contextBody;
    // Context stationary
    bool m_averageAroundRotationShown;
    boost::shared_ptr<BodySelector> m_bodySelector;
    bool m_selectionContextShown;
    bool m_centerPathHidden;
    // Simulation related variables
    size_t m_simulationIndex;
    vector<ObjectPosition> m_averageAroundPositions;
    /**
     * Index into m_foam that shows the current DMP file displayed
     */
    size_t m_currentTime;
    size_t m_timeSteps;
    bool m_t1sShiftLower;
    size_t m_syncViewTimeBegin;
    size_t m_syncViewTimeEnd;
    float m_deformationSize;
    float m_deformationLineWidth;
    float m_velocityLineWidth;
    float m_forceTorqueSize;
    float m_torqueDistance;
    float m_forceTorqueLineWidth;
    G3D::Vector2 m_scaleCenter;
    bool m_missingPressureShown;
    bool m_missingVolumeShown;
    bool m_objectVelocityShown;

    bool m_histogramShown;
    HistogramType::Options m_histogramOptions;
    bool m_domainClipped;
    double m_streamlineMaxPropagation;
    int m_streamlineMaxSteps;
};


#endif //__VIEW_SETTINGS_H__

// Local Variables:
// mode: c++
// End:
