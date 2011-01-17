/**
 * @file   Statistics.h
 * @author Dan R. Lipsa
 * @date 13 Jan 2011
 *
 * Declaration for Statistics
 */

#ifndef __STATISTICS_H__
#define __STATISTICS_H__


typedef acc::accumulator_set<
    double, acc::features<acc::tag::density> > HistogramStatisticsBase;


/**
 * Calculates a histogram using boost::accumulators::density.
 *
 * It uses cache_size of 2 and it adds min and max values before it
 * adds all other values.
 */
class HistogramStatistics : public HistogramStatisticsBase
{
public:
    typedef acc::impl::density_impl<double>::result_type Result;

public:
    HistogramStatistics (size_t numBins) :
	HistogramStatisticsBase (
	    acc::tag::density::cache_size = 2,
	    acc::tag::density::num_bins = numBins)
    {
    }
    size_t GetCountPerBin (size_t bin) const;
    QwtDoubleInterval GetBinInterval (size_t bin) const;
    size_t size () const;
    QwtIntervalData ToQwtIntervalData () const;
    size_t GetMaxCountPerBin () const;
    static size_t GetBin (double value, size_t binCount,
                          double beginInterval, double endInterval);

};


typedef acc::accumulator_set<
    double, 
    acc::features<acc::tag::min, 
		  acc::tag::max, acc::tag::count> > MinMaxStatistics;

typedef acc::accumulator_set<
    double, acc::features<acc::tag::median> > MedianStatistics;


#endif //__STATISTICS_H__

// Local Variables:
// mode: c++
// End:
