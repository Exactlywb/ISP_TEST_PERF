#ifndef _METRICS_HPP__
#define _METRICS_HPP__

#include "FuncData.hpp"

struct RegularFunctionMetric final {
    double operator() (const HFData::cluster &clust, const std::vector<int> &state) const;
};

struct RegularClusterMetric final {
    double operator() (const std::vector<HFData::cluster *> &graph,
                       const std::vector<int> &state) const;
};

#endif
