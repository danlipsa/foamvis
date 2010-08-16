/* -*- mode: C++ ; c-file-style: "stroustrup" -*- *****************************
 * Qwt Widget Library
 * Copyright (C) 1997   Josef Wilgen
 * Copyright (C) 2002   Uwe Rathmann
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
    explicit HistogramItem(const QString &title = QString::null);
    explicit HistogramItem(const QwtText &title);
    virtual ~HistogramItem();

    void setData(const QwtIntervalData &data);
    const QwtIntervalData &data() const;

    void setFocusColor(const QColor& color);
    void setContextColor (const QColor& color);
    QColor focusColor () const;
    QColor contextColor () const;

    virtual QwtDoubleRect boundingRect() const;

    virtual int rtti() const;

    virtual void draw(QPainter *, const QwtScaleMap &xMap, 
        const QwtScaleMap &yMap, const QRect &) const;

    void setBaseline(double reference);
    double baseline() const;

    enum HistogramAttribute
    {
        Auto = 0,
        Xfy = 1
    };

    void setHistogramAttribute(HistogramAttribute, bool on = true);
    bool testHistogramAttribute(HistogramAttribute) const;
    void setSelected (bool selected);
    void setSelected (bool selected, size_t begin, size_t end);
    void getSelectedIntervals (vector<QwtDoubleInterval>* intervals) const;
    void getSelectedBins (
	vector< pair<size_t, size_t> >* intervals, bool selected = true) const;
    void setSelectedBins (
	const vector< pair<size_t, size_t> >& intervals);

protected:
    virtual void drawBar(QPainter *,
        Qt::Orientation o, const QRect &) const;

private:
    void init();
    void drawHistogramBars (
	QPainter *painter, const QwtScaleMap &xMap, 
	const QwtScaleMap &yMap) const;
    void drawSelectionRegions (
	QPainter *painter, const QwtScaleMap &xMap, 
	const QwtScaleMap &yMap) const;
    void drawYx (size_t i, QPainter *painter, const QwtScaleMap &xMap, 
		  const QwtScaleMap &yMap) const;
    void drawXy (
	size_t i, QPainter *painter, const QwtScaleMap &xMap, 
	const QwtScaleMap &yMap) const;
    void drawRegion (size_t beginRegion, size_t endRegion,
		     QPainter *painter, 
		     const QwtScaleMap &xMap, const QwtScaleMap &yMap) const;



    class PrivateData;
    PrivateData *d_data;
};

#endif
