/**
 * @file   HistogramStatistics.cpp
 * @author Dan R. Lipsa
 * @date 13 Jan. 2011
 *
 * Implementation of the HistogramStatistics class
 */

#include "HistogramStatistics.h"
#include "Debug.h"

size_t HistogramStatistics::GetCountPerBin (size_t bin) const
{
    // density has a underflow and overflow bin before and after 
    // the data bins
    ++bin;
    Result histogramResult = acc::density (*this);
    Result::iterator it = histogramResult.begin () + bin;
    size_t count = floor(it->second * acc::count (*this) + 0.5);
    if (bin == 1 && acc::min (*this) != acc::max(*this))
	--count;
    else if (bin == size ())
    {
	it = (histogramResult.end () - 1);
	size_t overflowCount = floor (it->second * acc::count (*this) + 0.5);
	count += (overflowCount - 1);
    }
    return count;
}

size_t HistogramStatistics::GetMaxCountPerBin () const
{
    size_t size = this->size ();
    size_t max = 0;
    for (size_t i = 0; i < size; ++i)
	max = std::max (max, GetCountPerBin (i));
    return max;
}


QwtDoubleInterval HistogramStatistics::GetBinInterval (size_t bin) const
{
    ++bin;
    Result histogramResult = acc::density (*this);
    Result::iterator it = histogramResult.begin () + bin;
    Result::iterator nextIt = it + 1;
    QwtDoubleInterval interval ((*it).first, (*nextIt).first, 
				QwtDoubleInterval::ExcludeMaximum);
    if (bin == static_cast<size_t>(histogramResult.size () - 2))
	interval.setBorderFlags (QwtDoubleInterval::IncludeBorders);
    return interval;
}

size_t HistogramStatistics::size () const
{
    Result histogramResult = acc::density (*this);
    // remove the underflow and overflow bins
    return histogramResult.size () - 2;
}


QwtIntervalData HistogramStatistics::ToQwtIntervalData () const
{
    size_t size = this->size ();
    QwtArray<QwtDoubleInterval> intervals (size);
    QwtArray<double> values (size);
    for (size_t bin = 0; bin < size; ++bin)
    {
	intervals[bin] = GetBinInterval (bin);
	values[bin] = GetCountPerBin (bin);
    }
    return QwtIntervalData (intervals, values);
}


size_t HistogramStatistics::GetBin (
    double value, size_t binCount, double beginInterval, double endInterval)
{
    if (beginInterval == endInterval || value < beginInterval)
        return 0;
    else if (value >= endInterval)
        return binCount - 1;
    else
    {
        double step = (endInterval - beginInterval) / binCount;
        return floor ((value - beginInterval) / step);
    }
}

string HistogramStatistics::ToString () const
{
    ostringstream ostr;
    for (size_t i = 0; i < size (); ++i)
	ostr << GetCountPerBin (i) << " ";
    return ostr.str ();
}

string HistogramStatistics::RawToString () const
{
    ostringstream ostr;
    Result histogramResult = acc::density (*this);
    for (Result::iterator it = histogramResult.begin ();
	 it != histogramResult.end (); ++it)
	ostr << (it->second * acc::count(*this)) << " ";
    return ostr.str ();
}
