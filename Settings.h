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


public:
    Settings (const Simulation& simulation, float w, float h, 
	      bool t1sShiftLower);

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
    ViewCount::Enum GetViewCount () const
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
    size_t GetViewSettingsSize ()
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
    ColorBarType::Enum GetColorBarType (ViewNumber::Enum viewNumber) const;
    ColorBarType::Enum GetColorBarType () const;
    // @}

    /**
     * @{
     * @name Time
     */
    size_t GetCurrentTime (ViewNumber::Enum viewNumber) const;
    size_t GetCurrentTime () const
    {
	return GetCurrentTime (GetViewNumber ());
    }
    void SetCurrentTime (
	size_t time, 
	boost::array<int, ViewNumber::COUNT>* direction = 0, 
	bool setLastStep = false);
    size_t GetTimeSteps (ViewNumber::Enum viewNumber) const;
    // @}

    /**
     * @{
     * @name LinkedTime
     */
    void AddLinkedTimeEvent ();
    void ResetLinkedTimeEvents ();
    void SetTimeLinkage (TimeLinkage::Enum timeLinkage);
    TimeLinkage::Enum GetTimeLinkage () const
    {
	return m_timeLinkage;
    }
    size_t GetLinkedTime () const
    {
	return m_linkedTime;
    }
    float GetLinkedTimeStretch (ViewNumber::Enum viewNumber, 
                                 size_t eventIndex) const;
    pair<size_t, ViewNumber::Enum> GetLinkedTimeMaxInterval (
        size_t eventIndex) const;
    const vector<size_t>& GetLinkedTimeEvents (
        ViewNumber::Enum viewNumber) const;
    
    vector<ViewNumber::Enum> GetLinkedTimeViewNumbers (
	ViewNumber::Enum viewNumber) const;
    vector<ViewNumber::Enum> GetLinkedTimeViewNumbers () const
    {
	return GetLinkedTimeViewNumbers (GetViewNumber ());
    }
    size_t GetLinkedTimeTimeSteps () const;
    bool HasEqualNumberOfEvents () const;
    // @}


    /**
     * @{
     * @name Gl, Vtk, and Histogram views
     */
    bool IsVtkView (ViewNumber::Enum viewNumber) const;
    bool IsGlView (ViewNumber::Enum viewNumber) const;
    bool IsHistogramShown (ViewNumber::Enum viewNumber) const;
    ViewCount::Enum GetVtkCount (vector<ViewNumber::Enum>* mapping = 0) const;
    ViewCount::Enum GetGlCount (vector<ViewNumber::Enum>* mapping = 0) const;
    ViewCount::Enum GetHistogramCount (
	vector<ViewNumber::Enum>* mapping = 0) const;
    // @}

    /**
     * @{
     * @name Various
     */
    InteractionMode::Enum GetInteractionMode () const
    {
        return m_interactionMode;
    }
    void SetInteractionMode (InteractionMode::Enum mode)
    {
        m_interactionMode = mode;
    }
    float GetContextAlpha () const
    {
	return m_contextAlpha;
    }
    void SetContextAlpha (float contextAlpha)
    {
	m_contextAlpha = contextAlpha;
    }
    float GetEdgeRadius () const 
    {
	return m_edgeRadius;
    }
    void SetEdgeRadius (float edgeRadius)
    {
	m_edgeRadius = edgeRadius;
    }
    float GetEdgeWidth () const 
    {
	return m_edgeWidth;
    }
    void SetEdgeWidth (float edgeWidth)
    {
	m_edgeWidth = edgeWidth;
    }
    float GetArrowBaseRadius () const 
    {
	return m_arrowBaseRadius;
    }
    void SetArrowBaseRadius (float arrowBaseRadius)
    {
	m_arrowBaseRadius = arrowBaseRadius;
    }
    float GetArrowHeight () const 
    {
	return m_arrowHeight;
    }
    void SetArrowHeight (float arrowHeight)
    {
	m_arrowHeight = arrowHeight;
    }
    void SetEdgeArrow (float onePixelInObjectSpace);
    float GetEdgeRadiusRatio () const
    {
	return m_edgeRadiusRatio;
    }
    void SetEdgeRadiusRatio (float edgeRadiusRatio)
    {
	m_edgeRadiusRatio = edgeRadiusRatio;
    }
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
    QColor GetBubblePathsContextColor () const;
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
    bool IsBarLarge () const
    {
        return m_barLarge;
    }
    void SetBarLarge (bool large)
    {
        m_barLarge = large;
    }
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
			     ViewCount::Enum viewCount) const;
    G3D::Rect2D GetViewRect (float w, float h,
			     ViewNumber::Enum viewNumber) const
    {
	return GetViewRect (w, h, viewNumber, GetViewCount ());
    }

    G3D::Rect2D GetViewRect (float w, float h) const
    {
	return GetViewRect (w, h, GetViewNumber ());
    }
    G3D::Rect2D GetViewColorBarRect (const G3D::Rect2D& viewRect);
    G3D::Rect2D GetViewOverlayBarRect (const G3D::Rect2D& viewRect);
    // @}

Q_SIGNALS:
    void ViewChanged (ViewNumber::Enum prevViewNumber);
    void SelectionChanged (ViewNumber::Enum viewNumber);

public:
    const static pair<float,float> CONTEXT_ALPHA;
    const static size_t QUADRIC_SLICES;
    const static size_t QUADRIC_STACKS;

private Q_SLOTS:
    void selectionChanged (int viewNumber);

private:
    float getXOverY (float w, float h, ViewNumber::Enum viewNumber, 
		     ViewCount::Enum viewCount) const;
    void setScaleCenter (ViewNumber::Enum viewNumber, 
			 const Simulation& simulation, float w, float h);
    void initEndTranslationColor ();
    void initViewSettings (const Simulation& simulation, float w, float h,
			   bool t1sShiftLower);
    void checkLinkedTimesValid () const;
    ViewCount::Enum getViewCount (
	vector<ViewNumber::Enum>* mapping, IsViewType isView) const;
    /**
     * @return positive if time for the view 
     *         has moved forward or negative otherwise
     */
    int setCurrentTime (ViewNumber::Enum viewNumber, size_t linkedTime, 
                        bool setLastStep);

private:
    Q_OBJECT
    float m_contextAlpha;
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
     * @see GetLinkedTimeMaxInterval, @see GetLinkedTimeMaxSteps
     */
    size_t m_linkedTime;
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
    boost::shared_ptr<QSignalMapper> m_signalMapperSelectionChanged;    
    InteractionMode::Enum m_interactionMode;
};



#endif //__SETTINGS_H__

// Local Variables:
// mode: c++
// End:
