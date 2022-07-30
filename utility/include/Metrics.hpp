#ifndef _METRICS_HPP__
#define _METRICS_HPP__

#include "FuncData.hpp"

struct RegularFunctionMetric final {

        double operator() (const HFData::cluster& clust, const std::vector<int> &state) const
        {
            const std::vector<HFData::node *>& funcs = clust.functions_;
            std::vector<size_t> dists (funcs.size ());
            auto dist_for_func = [&dists, funcs, state, clust] (HFData::node *f) -> int {
                for (std::size_t i = 0; i < funcs.size (); i++) {
                    if (funcs[state[i]] == f)
                        return dists[state[i]];
                }
                return clust.m_size;
            };

            auto get_dist_for_call = [&] (HFData::node *caller, HFData::node *callee) -> int {
                return std::abs (dist_for_func (caller) + (int)caller->size_ / 2 -
                                 dist_for_func (callee));
            };

            dists[state[0]] = 0;
            for (std::size_t i = 1; i < funcs.size (); i++) {
                dists[state[i]] = dists[state[i - 1]] + funcs[state[i]]->size_;
            }

            double cur_metric = 0;
            for (auto node : funcs) {
                for (auto e : node->callers_) {
                    auto caller = e->caller;
                    auto callee = e->callee;
                    if (callee->aux_ != &clust || caller->aux_ != &clust)
                        continue;
                    cur_metric += e->freq * (get_dist_for_call (caller, callee));
                }
            }
            return cur_metric;

        }

};

#endif
