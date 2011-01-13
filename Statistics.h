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
    double, acc::features<acc::tag::density> > HistogramStatistics;
typedef acc::impl::density_impl<double>::result_type HistogramStatisticsResult;
size_t BinCount (const HistogramStatistics& histogram, size_t i);

typedef acc::accumulator_set<
    double, 
    acc::features<acc::tag::min, 
		  acc::tag::max, acc::tag::count> > MinMaxStatistics;

#endif //__STATISTICS_H__

// Local Variables:
// mode: c++
// End:
