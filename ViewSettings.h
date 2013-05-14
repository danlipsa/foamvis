/**
 * @file   ViewSettings.h
 * @author Dan R. Lipsa
 * @date 10 March 2011
 *
 * Per view settings
 */
#ifndef __VIEW_SETTINGS_H__
#define __VIEW_SETTINGS_H__

#include "DataProperties.h"
#include "Enums.h"
#include "ObjectPosition.h"
class AllBodySelector;
class BodySelector;
class ColorBarModel;
class Foam;
class WidgetGl;
class IdBodySelector;
class ValueBodySelector;
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

    /**
     * body scalar or DMP_COLOR
     */
    void SetBodyOrFaceScalar (size_t bodyOrFaceScalar)
    {
	m_bodyOrFaceScalar = bodyOrFaceScalar;
    }
    /**
     * body scalar or DMP_COLOR or T1_KDE
     */
    size_t GetBodyOrOtherScalar () const;
    
    StatisticsType::Enum GetStatisticsType () const
    {
	return m_statisticsType;
    }
    void SetStatisticsType (StatisticsType::Enum statisticsType)
    {
	m_statisticsType = statisticsType;
    }
    EdgeVis::Enum GetEdgeVis () const
    {
        return m_edgeVis;
    }
    void SetEdgeVis (EdgeVis::Enum vis)
    {
        m_edgeVis = vis;
    }
    // @}

    /**
     * @{
     * @name KDE
     */
    // Streamlines seeded based on KDE
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
    bool IsT1KDEKernelBoxShown () const
    {
	return m_T1KDEKernelBoxShown;
    }
    void SetT1KDEKernelBoxShown (bool kernelTextureShown)
    {
	m_T1KDEKernelBoxShown = kernelTextureShown;
    }    
    float GetT1KDESigmaInBubbleDiameter () const
    {
        return m_T1KDESigmaInBubbleDiameter;
    }
    void SetT1KDESigmaInBubbleDiameter (float sigma)
    {
        m_T1KDESigmaInBubbleDiameter = sigma;
    }
    float GetT1KDEIsosurfaceValue () const
    {
        return m_t1KDEIsosurfaceValue;
    }
    void SetT1KDEIsosurfaceValue (float value)
    {
        m_t1KDEIsosurfaceValue = value;
    }
    // @}

    /**
     * @{
     * @name Scalar
     */
    bool IsScalarShown () const
    {
        return m_scalarShown;
    }
    void SetScalarShown (bool scalarShown)
    {
        m_scalarShown = scalarShown;
    }
    bool IsScalarContext () const
    {
        return m_scalarContext;
    }
    void SetScalarContext (bool context)
    {
        m_scalarContext = context;
    }
    bool IsScalarContourShown () const
    {
        return m_scalarContourShown;
    }
    void SetScalarContourShown (bool shown)
    {
        m_scalarContourShown = shown;
    }
    // @}


    /**
     * @{
     * @name Force     
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

    float GetForceRatio () const
    {
	return m_forceRatio;
    }
    void SetForceRatio (float value)
    {
	m_forceRatio = value;
        Q_EMIT ViewChanged ();
    }
    float GetTorqueDistanceRatio () const
    {
	return m_torqueDistance;
    }
    void SetTorqueDistanceRatio (float value)
    {
	m_torqueDistance = value;
    }    
    // @}


    /**
     * @{
     * @name Deformation
     */
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
    // @}

    /**
     * @{
     * @name ColorBar
     */
    boost::shared_ptr<ColorBarModel> GetColorMapScalar () const
    {
	return m_colorMapScalar;
    }
    boost::shared_ptr<ColorBarModel> GetColorMapVelocity () const
    {
	return m_colorMapVelocity;
    }
    void SetColorMapScalar (
	const boost::shared_ptr<ColorBarModel>& colorBarModel);
    void SetColorMapVelocity (
	const boost::shared_ptr<ColorBarModel>& colorBarModel);
    void ResetColorBarModel ()
    {
	m_colorMapScalar.reset ();
    }
    void CopyColorMapScalar (const ViewSettings& from);
    void CopyColorMapVelocity (const ViewSettings& from);
    void CopyColorMapVelocityFromScalar ();
    ColorMapScalarType::Enum GetColorMapType () const;    
    static ColorMapScalarType::Enum GetColorMapType (
        ViewType::Enum viewType, size_t property,
        StatisticsType::Enum statisticsType);
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
    void SetAxisOrder (AxisOrderName::Enum axesOrder)
    {
	m_axesOrder = axesOrder;
    }
    AxisOrderName::Enum GetAxisOrder () const
    {
	return m_axesOrder;
    }
    G3D::Matrix3 GetRotationForAxisOrder (const Foam& foam) const;

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

    // pixel in object space
    void SetOnePixelInObjectSpace (float pixel)
    {
        m_onePixelInObjectSpace = pixel;
    }
    float GetOnePixelInObjectSpace () const
    {
        return m_onePixelInObjectSpace;
    }
    bool IsDmpTransformShown () const
    {
        return m_dmpTransformShown;
    }
    void SetDmpTransformShown (bool shown)
    {
        m_dmpTransformShown = shown;
    }
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
    bool IsVelocityColorMapped () const
    {
        return m_velocityColorMapped;
    }
    void SetVelocityColorMapped (bool colorMapped)
    {
        m_velocityColorMapped = colorMapped;
    }
    void SetVelocityVis (VectorVis::Enum vis)
    {
        m_velocityVis = vis;
    }
    VectorVis::Enum GetVelocityVis () const
    {
        return m_velocityVis;
    }
    // is in between 
    float GetVelocityInverseClampMaxRatio () const;
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
    float GetObjectAlpha () const
    {
	return m_objectAlpha;
    }
    void SetObjectAlpha (float alpha)
    {
	m_objectAlpha = alpha;
    }
    float GetT1KDEIsosurfaceAlpha () const
    {
        return m_t1KDEIsosurfaceAlpha;
    }
    void SetT1KDEIsosurfaceAlpha (float alpha)
    {
        m_t1KDEIsosurfaceAlpha = alpha;
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

    
    /**
     * @{
     * @name Body selection
     */
    boost::shared_ptr<BodySelector> GetBodySelector () const
    {
	return m_bodySelector;
    }
    void SetBodySelector (
	boost::shared_ptr<ValueBodySelector> selector);
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
     * @name Bubble path
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
    // @}


    /**
     * @{
     * @name Various
     */
    size_t GetSimulationIndex () const
    {
	return m_simulationIndex;
    }

    bool T1sShiftLower () const
    {
	return m_t1ShiftLower;
    }
    void SetT1sShiftLower (bool t1sShiftLower)
    {
	m_t1ShiftLower = t1sShiftLower;
    }
    float GetT1Size () const
    {
        return m_t1Size;
    }
    void SetT1Size (float size)
    {
        m_t1Size = size;
    }
    bool IsT1Shown () const
    {
        return m_t1Shown;
    }
    void SetT1Shown (bool shown)
    {
        m_t1Shown = shown;
    }
    bool IsT1AllTimeSteps () const
    {
        return m_t1AllTimeSteps;
    }
    void SetT1AllTimeSteps (bool all)
    {
        m_t1AllTimeSteps = all;
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
    float GetContextAlpha () const
    {
	return m_contextAlpha;
    }
    void SetContextAlpha (float contextAlpha)
    {
	m_contextAlpha = contextAlpha;
    }
    bool AxesShown () const
    {
        return m_axesShown;
    }
    void SetAxesShown (bool shown)
    {
        m_axesShown = shown;
    }
    bool IsBoundingBoxSimulationShown () const
    {
        return m_boundingBoxSimulationShown;
    }
    void SetBoundingBoxSimulationShown (bool shown)
    {
        m_boundingBoxSimulationShown = shown;
    }
    bool IsTorusDomainShown () const
    {
        return m_torusDomainShown;
    }
    void SetTorusDomainShown (bool shown)
    {
        m_torusDomainShown = shown;
    }
    void SetDimension (Dimension::Enum dimension);
    //@}

    /**
     * @{
     * @name Arrow display
     */
    /**
     * These afect the bubble paths as well.
     */
    void SetArrowParameters (float onePixelInObjectSpace);
    static void SetArrowParameters (
        float onePixelInObjectSpace,
        float* edgeRadius, float* arrowHeadRadius, float* arrowHeadHeight, 
        float edgeRadiusRatio = 0,
        float* edgeWidth = 0);

    float GetEdgeWidth () const 
    {
	return m_edgeWidth;
    }
    float GetEdgeRadius () const 
    {
	return m_edgeRadius;
    }
    float GetEdgeRadiusRatio () const
    {
	return m_edgeRadiusRatio;
    }
    void SetEdgeRadiusRatio (float edgeRadiusRatio)
    {
	m_edgeRadiusRatio = edgeRadiusRatio;
    }
    float GetArrowHeadRadius () const 
    {
	return m_arrowHeadRadius;
    }
    float GetArrowHeadHeight () const 
    {
	return m_arrowHeadHeight;
    }
    // @}

    /**
     * @{
     * @name Bubble paths
     */
    /**
     * Switches between line and tube/quadric
     */
    bool IsBubblePathsLineUsed () const
    {
	return m_centerPathLineUsed;
    }
    void SetBubblePathsLineUsed (bool used)
    {
	m_centerPathLineUsed = used;
    }
    /**
     * Switches between tube and quadric
     */
    bool IsBubblePathsTubeUsed () const
    {
	return m_centerPathTubeUsed;
    }
    void SetBubblePathsTubeUsed (bool used)
    {
	m_centerPathTubeUsed = used;
    }
    // @}



public:
    static const double STREAMLINE_LENGTH;
    static const double STREAMLINE_STEP_LENGTH;
    const static pair<float,float> ALPHA_RANGE;
    const static pair<float,float> TENSOR_LINE_WIDTH_EXP2;
    const static pair<float,float> T1_SIZE;
    const static pair<float,float> EDGE_RADIUS_RATIO;
    const static size_t MAX_RADIUS_MULTIPLIER;

Q_SIGNALS:
    void SelectionChanged ();
    void ViewChanged ();

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
    boost::shared_ptr<ColorBarModel> m_colorMapScalar;
    boost::shared_ptr<ColorBarModel> m_colorMapVelocity;
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
    AxisOrderName::Enum m_axesOrder;
    /**
     * Distance from the camera to the center of the bounding box for the foam.
     */
    float m_cameraDistance;

    //Stationary
    bool m_scalarShown;
    bool m_scalarContext;
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
    float m_t1KDEIsosurfaceAlpha;
    bool m_centerPathHidden;
    // Simulation related variables
    size_t m_simulationIndex;
    /**
     * Index into m_foam that shows the current DMP file displayed
     */
    size_t m_time;
    size_t m_timeSteps;
    size_t m_timeWindow;
    bool m_t1ShiftLower;
    vector<size_t> m_linkedTimeEvent;
    float m_deformationSize;
    float m_deformationLineWidth;
    float m_velocityLineWidth;
    bool m_velocityColorMapped;
    float m_forceRatio;
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
    bool m_T1KDEKernelBoxShown;
    float m_T1KDESigmaInBubbleDiameter;
    float m_t1KDEIsosurfaceValue;
    float m_t1Size;
    bool m_t1Shown;
    bool m_t1AllTimeSteps;
    float m_onePixelInObjectSpace;
    bool m_axesShown;
    bool m_boundingBoxSimulationShown;
    bool m_torusDomainShown;
    bool m_scalarContourShown;
    bool m_dmpTransformShown;
    float m_edgeWidth;
    float m_edgeRadius;
    float m_edgeRadiusRatio;
    float m_arrowHeadRadius;
    float m_arrowHeadHeight;
    bool m_centerPathTubeUsed;
    bool m_centerPathLineUsed;
    EdgeVis::Enum m_edgeVis;
};


#endif //__VIEW_SETTINGS_H__

// Local Variables:
// mode: c++
// End:
