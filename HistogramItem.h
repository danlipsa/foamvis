/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
 * Changed by Dan Lipsa
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the Qwt License, Version 1.0
 *****************************************************************************/

#ifndef HISTOGRAM_ITEM_H
#define HISTOGRAM_ITEM_H

class QwtIntervalData;
class QString;

class HistogramItem: public QwtPlotItem
{
public:
    enum HistogramAttribute
    {
        Auto = 0,
        Xfy = 1
    };

public:
    explicit HistogramItem (const QString &title = QString::null);
    explicit HistogramItem (const QwtText &title);
    virtual ~HistogramItem ();

    double baseline () const;
    virtual QwtDoubleRect boundingRect () const;
    QColor contextColor () const;
    const QwtIntervalData& data () const;
    const QwtDoubleInterval GetDataInterval () const;
    virtual void draw (QPainter *, const QwtScaleMap &xMap, 
        const QwtScaleMap &yMap, const QRect &) const;
    QColor focusColor () const;
    const QwtLinearColorMap& getColorMap () const;
    void getSelectedIntervals (vector<QwtDoubleInterval>* intervals) const;
    /**
     * Gets the bins that have selection 'selected'.
     */
    void getSelectedBins (
	vector< pair<size_t, size_t> >* intervals, bool selected = true) const;
    double getMaxValueYAxis () const;
    double GetMinValueYAxis () const;
    bool isLogValueAxis () const;


    virtual int rtti () const;
    void setFocusColor (const QColor& color);
    void setContextColor (const QColor& color);
    void setOutOfBoundsColor (const QColor& color);
    void setBaseline (double reference);
    void setData (
	const QwtIntervalData &data,
	double maxValue,
	const vector< pair<size_t, size_t> >* selectedBins = 0);
    void setMaxValueAxis (double maxValue);
    void setHistogramAttribute (HistogramAttribute, bool on = true);
    void setColorCoded (bool enable);
    void setColorMap (const QwtLinearColorMap& colorMap);
    void setAllItemsSelection (bool selected);
    void setSelected (bool selected, size_t begin, size_t end);
    void setSelectedBins (
	const vector< pair<size_t, size_t> >& intervals);
    void setLogValueAxis (bool logValueAxis);
    bool testHistogramAttribute (HistogramAttribute) const;



private:
    QColor getBarColor (size_t i) const;
    void drawBars (
	QPainter *painter, const QwtScaleMap &xMap, 
	const QwtScaleMap &yMap) const;
    void drawBar (
	QPainter *painter, const QColor& color,
	const QRect &r, bool outOfBounds = false) const;
    void drawBar (
	size_t i, QPainter *painter, const QwtScaleMap &xMap, 
	const QwtScaleMap &yMap) const;
    void drawDeselectedRegion (
	size_t beginRegion, size_t endRegion,
	QPainter *painter, 
	const QwtScaleMap &xMap, const QwtScaleMap &yMap) const;
    void drawDeselectedRegions (
	QPainter *painter, 
	const QwtScaleMap &xMap, const QwtScaleMap &yMap) const;
    void init ();

private:
    static const double logScaleZero;
    class PrivateData;
    PrivateData *d_data;
};

#endif
