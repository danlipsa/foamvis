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
     * @name Attributes
     */
    void SetForceShown (ForceType::Enum type, bool value)
    {
	m_forceShown[type] = value;
    }
    bool IsForceShown (ForceType::Enum type) const
    {
	return m_forceShown[type];
    }

    void SetTorqueShown (ForceType::Enum type, bool value)
    {
	m_torqueShown[type] = value;
    }
    bool IsTorqueShown (ForceType::Enum type) const
    {
	return m_torqueShown[type];
    }

    float GetForceSize () const
    {
	return m_forceSize;
    }
    void SetForceSize (float value)
    {
	m_forceSize = value;
    }
    float GetTorqueDistance () const
    {
	return m_torqueDistance;
    }
    void SetTorqueDistance (float value)
    {
	m_torqueDistance = value;
    }
    float GetForceLineWidth () const
    {
	return m_forceLineWidth;
    }
    void SetForceLineWidth (float value)
    {
	m_forceLineWidth = value;
    }
    void SetDeformationTensorShown (bool deformationTensorShown)
    {
	m_deformationShown = deformationTensorShown;
    }

    bool IsDeformationShown () const
    {
	return m_deformationShown;
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
    float GetKDEValue () const
    {
        return m_kdeValue;
    }
    void SetKDEValue (float value)
    {
        m_kdeValue = value;
    }
    /**
     * Streamlines seeded based on T1s KDE have higher resolution
     * in grid squares with high KDE value.
     * The number of seeds of a KDE grid square is 
     * (2*m + 1)^2 where m is the kde multiplier
     */
    int GetKDEMultiplier () const
    {
        return m_kdeMultiplier;
    }
    void SetKDEMultiplier (int multiplier)
    {
        m_kdeMultiplier = multiplier;
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
    void ColorMapCopy (const ViewSettings& from);
    void ColorBarToOverlayBarPaletteClamping ();
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
    float GetAngleOfView () const
    {
	return m_angleOfView;
    }
    void SetAngleOfView (float value)
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
     * @name Velocity
     */
    void SetVelocityShown (bool velocityShown)
    {
	m_velocityShown = velocityShown;
    }
    bool IsVelocityShown () const
    {
	return m_velocityShown;
    }
    bool IsVelocityGlyphSameSize () const
    {
        return m_velocityGlyphSameSize;
    }
    void SetVelocityGlyphSameSize (bool same)
    {
        m_velocityGlyphSameSize = same;
    }
    
    void SetVelocityVis (VectorVis::Enum vis)
    {
        m_velocityVis = vis;
    }
    VectorVis::Enum GetVelocityVis () const
    {
        return m_velocityVis;
    }
    float GetVelocitySize () const;
    float GetVelocityLineWidth () const
    {
	return m_velocityLineWidth;
    }
    void SetVelocityLineWidth (float value)
    {
	m_velocityLineWidth = value;
    }
    double GetStreamlineLength () const
    {
        return m_streamlineLength;
    }
    void SetStreamlineLength (double value)
    {
        m_streamlineLength = value;
    }
    double GetStreamlineStepLength () const
    {
        return m_streamlineStepLength;
    }
    void SetStreamlineStepLength (double steps)
    {
        m_streamlineStepLength = steps;
    }
    // @}

    /**
     * @{
     * @name Seeds for glyphs and streamlines
     */
    bool IsSeedShown () const
    {
        return m_seedShown;
    }
    void SetSeedShown (bool shown)
    {
        m_seedShown = shown;
    }
    size_t GetGlyphSeedsCount () const
    {
        return m_glyphSeedsCount;
    }
    void SetGlyphSeedsCount (size_t count)
    {
        m_glyphSeedsCount = count;
    }
    bool IsKDESeedEnabled () const
    {
        return m_kdeSeedEnabled;
    }
    void SetKDESeedEnabled (bool enabled)
    {
        m_kdeSeedEnabled = enabled;
    }
    float GetSeedScaleRatio () const
    {
	return m_seedScaleRatio;
    }
    void SetSeedScaleRatio (float gridScaleRatio)
    {
	m_seedScaleRatio = gridScaleRatio;
    }
    const G3D::Vector3& GetSeedTranslation () const
    {
	return m_seedTranslation;
    }
    void SetSeedTranslation (const G3D::Vector3& gridTranslation)
    {
	m_seedTranslation = gridTranslation;
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
     * @name Average
     */
    bool IsAverageShown () const
    {
        return m_averageShown;
    }
    void SetAverageShown (bool averageShown)
    {
        m_averageShown = averageShown;
    }
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
    float GetContextAlpha () const
    {
	return m_contextAlpha;
    }
    void SetContextAlpha (float contextAlpha)
    {
	m_contextAlpha = contextAlpha;
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
    size_t GetTime () const
    {
	return m_time;
    }    
    /**
     * @return positive if time has moved forward or negative otherwise
     */
    int SetTime (size_t time);
    void SetTimeWindow (size_t timeWindow)
    {
        m_timeWindow = timeWindow;
    }
    size_t GetTimeWindow () const
    {
        return m_timeWindow;
    }
    size_t GetTimeSteps () const
    {
	return m_timeSteps;
    }
    void AddLinkedTimeEvent (size_t timeEvent);
    void ResetLinkedTimeEvents ();
    const vector<size_t>& GetLinkedTimeEvents () const
    {
	return m_linkedTimeEvent;
    }
    size_t GetLinkedTimeInterval (size_t eventIndex) const;
    bool IsTimeDisplacementUsed () const
    {
        return GetTimeDisplacement () > 0;
    }

    float GetTimeDisplacement () const
    {
	return m_timeDisplacement;
    }
    void SetTimeDisplacement (double time)
    {
        m_timeDisplacement = time;
    }    
    size_t GetBubblePathsTimeBegin () const
    {
        return m_bubblePathsTimeBegin;
    }
    void SetBubblePathsTimeBegin (size_t time)
    {
        m_bubblePathsTimeBegin = time;
    }
    size_t GetBubblePathsTimeEnd () const
    {
        return m_bubblePathsTimeEnd;
    }
    void SetBubblePathsTimeEnd (size_t time)
    {
        m_bubblePathsTimeEnd = time;
    }
    // @}


    /**
     * @{
     * @name Various
     */
    QColor GetBubblePathsContextColor () const;
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
			G3D::Vector3 viewingVolumeCenter);
    string GetTitle (ViewNumber::Enum viewNumber) const;
    bool DomainClipped () const
    {
        return m_domainClipped;
    }
    void SetDomainClipped (bool clipped)
    {
        m_domainClipped = clipped;
    }
    float GetObjectAlpha () const
    {
	return m_objectAlpha;
    }
    void SetObjectAlpha (float alpha)
    {
	m_objectAlpha = alpha;
    }
    //@}

public:
    static const double STREAMLINE_LENGTH;
    static const double STREAMLINE_STEP_LENGTH;
    const static pair<float,float> ALPHA_RANGE;
    const static pair<float,float> TENSOR_LINE_WIDTH_EXP2;
    const static pair<float,float> FORCE_SIZE_EXP2;

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
    bool m_seedShown;
    size_t m_glyphSeedsCount;
    bool m_kdeSeedEnabled;
    float m_seedScaleRatio;
    G3D::Vector3 m_seedTranslation;
    double m_contextScaleRatio;
    G3D::Vector3 m_translation;
    // lighting state
    bool m_lightingEnabled;
    LightNumber::Enum m_selectedLight;
    bitset<LightNumber::COUNT> m_lightEnabled;
    bitset<LightNumber::COUNT> m_directionalLightEnabled;
    bitset<LightNumber::COUNT> m_lightPositionShown;    
    boost::array<G3D::Matrix3, LightNumber::COUNT> m_rotationLight;
    boost::array<double, LightNumber::COUNT> m_lightPositionRatio;
    boost::array<boost::array<boost::array<GLfloat, 4>, LightType::COUNT>, 
                 LightNumber::COUNT> m_light;

    //  requires camera distance
    float m_angleOfView;
    AxesOrder::Enum m_axesOrder;
    /**
     * Distance from the camera to the center of the bounding box for the foam.
     */
    float m_cameraDistance;

    //Stationary
    bool m_averageShown;
    bool m_averageAround;
    boost::array<size_t, 2> m_averageAroundBodyId;
    bool m_averageAroundRotationShown;
    vector<ObjectPosition> m_averageAroundPositions;
    size_t m_differenceBodyId;
    // Context view
    bool m_contextView;
    boost::array<bool, 4> m_forceShown;
    boost::array<bool, 3> m_torqueShown;
    bool m_deformationShown;
    bool m_velocityShown;
    bool m_velocityGlyphSameSize;
    VectorVis::Enum m_velocityVis;
    // Context display
    set<size_t> m_contextBody;
    // Context stationary
    boost::shared_ptr<BodySelector> m_bodySelector;
    bool m_selectionContextShown;
    float m_contextAlpha;
    float m_objectAlpha;
    bool m_centerPathHidden;
    // Simulation related variables
    size_t m_simulationIndex;
    /**
     * Index into m_foam that shows the current DMP file displayed
     */
    size_t m_time;
    size_t m_timeSteps;
    size_t m_timeWindow;
    bool m_t1sShiftLower;
    vector<size_t> m_linkedTimeEvent;
    float m_deformationSize;
    float m_deformationLineWidth;
    float m_velocityLineWidth;
    float m_forceSize;
    float m_forceLineWidth;
    float m_torqueDistance;
    G3D::Vector2 m_scaleCenter;
    bool m_histogramShown;
    HistogramType::Options m_histogramOptions;
    bool m_domainClipped;
    double m_streamlineLength;
    double m_streamlineStepLength;
    float m_timeDisplacement;
    size_t m_bubblePathsTimeBegin;
    size_t m_bubblePathsTimeEnd;
    float m_kdeValue;
    int m_kdeMultiplier;
};


#endif //__VIEW_SETTINGS_H__

// Local Variables:
// mode: c++
// End:
