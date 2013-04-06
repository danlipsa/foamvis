/**
 * @file   Settings.h
 * @author Dan R. Lipsa
 * @date 17 July 2012
 *
 */
#ifndef __SETTINGS_H__
#define __SETTINGS_H__

#include "Hashes.h"
#include "Enums.h"

class ViewSettings;
class Simulation;
class SimulationGroup;

/**
 * Settings that apply to all the views
 */
class Settings : public QObject
{
public:
    typedef boost::unordered_map<G3D::Vector3int16, QColor,
	Vector3int16Hash> EndLocationColor;
    typedef bool (Settings::*IsViewType) (
	ViewNumber::Enum viewNumber) const;
    typedef G3D::Rect2D (Settings::*GetBarRectType) (
        ViewNumber::Enum viewNumber, const G3D::Rect2D& viewRect) const;

public:
    Settings (boost::shared_ptr<const SimulationGroup> simulationGroup, 
              float w, float h);

    /**
     * @{
     * @name TwoHalves view
     */
    vector<ViewNumber::Enum> GetTwoHalvesViewNumbers (
	ViewNumber::Enum viewNumber) const;
    vector<ViewNumber::Enum> GetTwoHalvesViewNumbers () const
    {
	return GetTwoHalvesViewNumbers (GetViewNumber ());
    }
    bool IsTwoHalvesView () const
    {
	return m_splitHalfView;
    }
    void SetTwoHalvesView (bool splitHalfView,
			   const Simulation& simulation, float w, float h);
    G3D::Vector2 CalculateScaleCenter (
	ViewNumber::Enum viewNumber, const G3D::Rect2D& rect) const;
    ViewType::Enum SetTwoHalvesViewType (ViewType::Enum viewType);
    template<typename T>
    void SetOneOrTwoViews (T* t,void (T::*f) (ViewNumber::Enum));
    //@}


    /**
     * @{
     * @name Views
     */
    size_t GetViewCount () const
    {
	return m_viewCount;
    }
    void SetViewCount (ViewCount::Enum viewCount)
    {
	m_viewCount = viewCount;
    }
    void SetViewNumber (ViewNumber::Enum viewNumber);
    ViewNumber::Enum GetViewNumber () const
    {
	return m_viewNumber;
    }
    ViewSettings& GetViewSettings (ViewNumber::Enum viewNumber) const
    {
	return *m_viewSettings[viewNumber];
    }
    ViewSettings& GetViewSettings () const
    {
	return GetViewSettings (GetViewNumber ());
    }
    size_t GetViewSettingsSize () const
    {
	return m_viewSettings.size ();
    }
    ViewLayout::Enum GetViewLayout () const
    {
	return m_viewLayout;
    }
    void SetViewLayout (ViewLayout::Enum viewLayout)
    {
	m_viewLayout = viewLayout;
    }
    // @}

    /**
     * @{
     * @name Time and LinkedTime
     */
    size_t GetViewTime (ViewNumber::Enum viewNumber) const;
    size_t GetViewTime () const
    {
	return GetViewTime (GetViewNumber ());
    }
    size_t GetLinkedTime () const
    {
	return m_linkedTime;
    }
    void SetTime (
	size_t time, 
	boost::array<int, ViewNumber::COUNT>* direction = 0, 
	bool setLastStep = false);
    size_t GetTimeSteps (ViewNumber::Enum viewNumber) const;


    void AddLinkedTimeEvent ();
    void ResetLinkedTimeEvents ();
    void SetTimeLinkage (TimeLinkage::Enum timeLinkage);
    TimeLinkage::Enum GetTimeLinkage () const
    {
	return m_timeLinkage;
    }
    float GetLinkedTimeStretch (ViewNumber::Enum viewNumber, 
                                size_t eventIndex) const;
    pair<size_t, ViewNumber::Enum> GetLinkedTimeMaxInterval (
        size_t eventIndex) const;
    size_t GetLinkedTimeEventTime (size_t eventIndex) const;
    const vector<size_t>& GetLinkedTimeEvents (
        ViewNumber::Enum viewNumber) const;
    
    vector<ViewNumber::Enum> GetLinkedTimeViewNumbers (
	ViewNumber::Enum viewNumber) const;
    vector<ViewNumber::Enum> GetLinkedTimeViewNumbers () const
    {
	return GetLinkedTimeViewNumbers (GetViewNumber ());
    }
    size_t GetLinkedTimeSteps () const;
    bool HasEqualNumberOfEvents () const;
    void SetAverageTimeWindow (size_t timeSteps);
    void UpdateAverageTimeWindow ();
    void SetLinkedTimeWindow (size_t timeWindow)
    {
        m_linkedTimeWindow = timeWindow;
    }
    size_t GetLinkedTimeWindow () const
    {
        return m_linkedTimeWindow;
    }
    size_t CalculateViewTimeWindow (
        ViewNumber::Enum viewNumber, size_t timeStep) const;

    // @}


    /**
     * @{
     * @name Save velocity field.
     *
     * Used for showing T1s KDE together with 
     * velocity fields.
     */
    bool IsVelocityFieldSaved () const
    {
        return m_velocityFieldSaved;
    }
    void SetVelocityFieldSaved (bool saved)
    {
        m_velocityFieldSaved = saved;
    }
    // @}

    /**
     * @{
     * @name Attributes
     */
    bool IsMissingPropertyShown (BodyScalar::Enum bodyProperty) const;
    void SetMissingPressureShown (bool shown)
    {
	m_missingPressureShown = shown;
    }
    void SetMissingVolumeShown (bool shown)
    {
	m_missingVolumeShown = shown;
    }
    void SetObjectVelocityShown (bool shown)
    {
	m_objectVelocityShown = shown;
    }
    // @}

    /**
     * @{
     * @name Color and overlay bars
     */
    bool IsBarLarge () const
    {
        return m_barLarge;
    }
    void SetBarLarge (bool large)
    {
        m_barLarge = large;
    }
    bool BarLabelsShown () const
    {
        return m_barLabelsShown;
    }
    void SetBarLabelsShown (bool shown)
    {
        m_barLabelsShown = shown;
    }
    ColorBarType::Enum GetColorBarType (ViewNumber::Enum viewNumber) const;
    ColorBarType::Enum GetColorBarType () const;
    G3D::Rect2D GetColorBarRect (ViewNumber::Enum viewNumber, 
                                 const G3D::Rect2D& viewRect) const;
    G3D::Rect2D GetColorBarRectWithLabels (
        ViewNumber::Enum viewNumber, const G3D::Rect2D& viewRect) const;
    G3D::Rect2D GetOverlayBarRectWithLabels (
        ViewNumber::Enum viewNumber, const G3D::Rect2D& viewRect) const;
    G3D::Rect2D GetOverlayBarRect (ViewNumber::Enum viewNumber, 
                                   const G3D::Rect2D& viewRect) const;
    G3D::Vector2 GetColorBarLabelSize (ViewNumber::Enum viewNumber) const;
    G3D::Vector2 GetOverlayBarLabelSize (ViewNumber::Enum viewNumber) const;
    // @}

    /**
     * @{
     * @name Interaction
     */
    InteractionMode::Enum GetInteractionMode () const
    {
        return m_interactionMode;
    }
    void SetInteractionMode (InteractionMode::Enum mode)
    {
        m_interactionMode = mode;
    }
    InteractionObject::Enum GetInteractionObject () const
    {
        return m_interactionObject;
    }
    void SetInteractionObject (InteractionObject::Enum mode)
    {
        m_interactionObject = mode;
    }    
    // @}


    /**
     * @{
     * @name Arrow display
     */
    void SetEdgeArrow (float onePixelInObjectSpace);
    float GetEdgeRadius () const 
    {
	return m_edgeRadius;
    }
    float GetEdgeWidth () const 
    {
	return m_edgeWidth;
    }
    float GetArrowBaseRadius () const 
    {
	return m_arrowBaseRadius;
    }
    float GetArrowHeight () const 
    {
	return m_arrowHeight;
    }
    float GetEdgeRadiusRatio () const
    {
	return m_edgeRadiusRatio;
    }
    void SetEdgeRadiusRatio (float edgeRadiusRatio)
    {
	m_edgeRadiusRatio = edgeRadiusRatio;
    }
    // @}


    /**
     * @{
     * @name Various
     */
    const QColor& GetEndTranslationColor (const G3D::Vector3int16& di) const;
    bool EdgesTessellationShown () const
    {
	return m_edgesTessellationShown;
    }
    void SetEdgesTessellationShown (bool shown)
    {
	m_edgesTessellationShown = shown;
    }
    bool ConstraintsShown () const
    {
	return m_constraintsShown;
    }
    void SetConstraintsShown (bool shown)
    {
	m_constraintsShown = shown;
    }
    bool ConstraintPointsShown () const
    {
	return m_constraintPointsShown;
    }
    void SetConstraintPointsShown (bool shown)
    {
	m_constraintPointsShown = shown;
    }
    QColor GetHighlightColor (ViewNumber::Enum viewNumber, 
			      HighlightNumber::Enum highlight) const;
    bool IsBubblePathsLineUsed () const
    {
	return m_centerPathLineUsed;
    }
    bool IsBubblePathsTubeUsed () const
    {
	return m_centerPathTubeUsed;
    }
    void SetBubblePathsTubeUsed (bool used)
    {
	m_centerPathTubeUsed = used;
    }
    void SetBubblePathsLineUsed (bool used)
    {
	m_centerPathLineUsed = used;
    }
    bool IsTitleShown () const
    {
	return m_titleShown;
    }
    void SetTitleShown (bool shown)
    {
	m_titleShown = shown;
    }
    bool IsViewFocusShown () const
    {
	return m_viewFocusShown;
    }
    void SetViewFocusShown (bool shown)
    {
	m_viewFocusShown = shown;
    }
    bool AxesShown () const
    {
        return m_axesShown;
    }
    void SetAxesShown (bool shown)
    {
        m_axesShown = shown;
    }
    // @}
    


    /**
     * @{
     * @name Computation
     */
    G3D::AABox CalculateViewingVolume (
	ViewNumber::Enum viewNumber, ViewCount::Enum viewCount, 
	const Simulation& simulation, 
	float w, float h, ViewingVolumeOperation::Enum enclose = 
	ViewingVolumeOperation::DONT_ENCLOSE2D) const;
    G3D::AABox CalculateCenteredViewingVolume (
	ViewNumber::Enum viewNumber, ViewCount::Enum viewCount, 
	const Simulation& simulation, 
	float w, float h, ViewingVolumeOperation::Enum enclose = 
	ViewingVolumeOperation::DONT_ENCLOSE2D) const;
    G3D::AABox CalculateEyeViewingVolume (
	ViewNumber::Enum viewNumber, ViewCount::Enum viewCount, 
	const Simulation& simulation, 
	float w, float h, ViewingVolumeOperation::Enum enclose = 
	ViewingVolumeOperation::DONT_ENCLOSE2D) const;
    G3D::Rect2D GetViewRect (float w, float h,
			     ViewNumber::Enum viewNumber, 
			     size_t viewCount) const;
    G3D::Rect2D GetViewRect (float w, float h,
			     ViewNumber::Enum viewNumber) const
    {
	return GetViewRect (w, h, viewNumber, GetViewCount ());
    }

    G3D::Rect2D GetViewRect (float w, float h) const
    {
	return GetViewRect (w, h, GetViewNumber ());
    }
    // @}

Q_SIGNALS:
    void ViewChanged (ViewNumber::Enum prevViewNumber);
    void SelectionChanged (ViewNumber::Enum viewNumber);

public:
    const static size_t QUADRIC_SLICES;
    const static size_t QUADRIC_STACKS;
    const static size_t BAR_MARGIN_DISTANCE;
    const static size_t BAR_WIDTH;
    const static size_t BAR_IN_BETWEEN_DISTANCE;

private Q_SLOTS:
    void selectionChanged (int viewNumber);

private:
    int calculateViewTime (
        ViewNumber::Enum viewNumber, int linkedTime) const;
    float getXOverY (float w, float h, ViewNumber::Enum viewNumber, 
		     ViewCount::Enum viewCount) const;
    void setScaleCenter (ViewNumber::Enum viewNumber, 
			 const Simulation& simulation, float w, float h);
    void initEndTranslationColor ();
    size_t initViewSettings (
        ViewNumber::Enum viewNumber,
        const SimulationGroup& simulationGroup, float w, float h);
    void initAllViewsSettings (
        const SimulationGroup& simulationGroup, float w, float h);
    void checkLinkedTimesValid () const;
    /**
     * @return positive if time for the view 
     *         has moved forward or negative otherwise
     */
    int setCurrentTime (ViewNumber::Enum viewNumber, size_t linkedTime, 
                        bool setLastStep);
    G3D::Rect2D getBarRectWithLabels (
        const G3D::Rect2D& barRect, G3D::Vector2 labelSize) const;
    void setEdgeRadius (float edgeRadius)
    {
	m_edgeRadius = edgeRadius;
    }
    void setEdgeWidth (float edgeWidth)
    {
	m_edgeWidth = edgeWidth;
    }
    void setArrowBaseRadius (float arrowBaseRadius)
    {
	m_arrowBaseRadius = arrowBaseRadius;
    }
    void setArrowHeight (float arrowHeight)
    {
	m_arrowHeight = arrowHeight;
    }


private:
    Q_OBJECT
    /**
     * For displaying edges as tubes
     */
    float m_edgeRadius;
    float m_edgeWidth;
    float m_edgeRadiusRatio;
    /**
     * For displaying arrows in the Torus Model edges
     */
    float m_arrowBaseRadius;
    float m_arrowHeight;
    EndLocationColor m_endTranslationColor;
    bool m_edgesTessellationShown;
    bool m_constraintsShown;
    bool m_constraintPointsShown;
    ViewNumber::Enum m_viewNumber;
    boost::array<
	boost::shared_ptr<ViewSettings>, ViewNumber::COUNT> m_viewSettings;
    TimeLinkage::Enum m_timeLinkage;
    /**
     * Used to keep trak of time for TimeLinkage::LINKED.
     * It has the resolution of the view that has the maximum interval and the 
     * range of the view that has the maximum range.
     * @see GetLinkedTimeMaxInterval
     */
    size_t m_linkedTime;
    size_t m_linkedTimeWindow;
    // View related variables
    ViewCount::Enum m_viewCount;
    ViewLayout::Enum m_viewLayout;
    bool m_missingPressureShown;
    bool m_missingVolumeShown;
    bool m_objectVelocityShown;
    bool m_centerPathTubeUsed;
    bool m_centerPathLineUsed;
    bool m_splitHalfView;
    bool m_titleShown;
    bool m_viewFocusShown;
    bool m_barLarge;
    bool m_velocityFieldSaved;
    bool m_barLabelsShown;
    bool m_axesShown;
    boost::shared_ptr<QSignalMapper> m_signalMapperSelectionChanged;    
    InteractionMode::Enum m_interactionMode;
    InteractionObject::Enum m_interactionObject;
};



#endif //__SETTINGS_H__

// Local Variables:
// mode: c++
// End:
