#include "Algos.hpp"

using HFData::cluster_edge;
using HFData::node;

// This file implements percise algorithm for metric minimization problem

bool Percise::runOncluster (HFData::cluster &cluster)
{
    auto &functions = cluster.functions_;
    const auto size = cluster.m_size;

    if (functions.size () <= 1 )
        return true;
    if (functions.size () >= 7)
        return false;

    // todo: need move functions with 0 size to the end of array

    std::vector<size_t> dists (functions.size ());
    auto dist_for_func = [&functions, &dists, size] (node *f) -> int {
        for (std::size_t i = 0; i < functions.size (); i++) {
            if (functions[i] == f)
                return dists[i];
        }
        return size;
    };
    auto get_dist_for_call = [&] (node *caller, node *callee) -> int {
        return std::abs (dist_for_func (caller) + (int)caller->size_ / 2 -
                         dist_for_func (callee));
    };
    auto get_metric = [&] () -> double {
        dists[0] = 0;
        for (std::size_t i = 1; i < functions.size (); i++) {
            dists[i] = dists[i - 1] + functions[i]->size_;
        }

        double cur_metric = 0;
        for (auto node : functions) {
            for (auto e : node->callers_) {
                auto caller = e->caller;
                auto callee = e->callee;
                if (callee->aux_ != &cluster || caller->aux_ != &cluster)
                    continue;
                cur_metric += e->freq * get_dist_for_call (caller, callee);
            }
        }
        return cur_metric;
    };

    // Find best permutation
    std::sort (functions.begin (), functions.end ());
    double metric = get_metric ();
    int idx = 0;
    int answ_prem = 0;
    do {
        auto cur_metric = get_metric ();
        if (cur_metric < metric && idx != 0) {
            metric = cur_metric;
            answ_prem = idx;
        }
        idx++;
    } while (std::next_permutation (functions.begin (), functions.end ()));

    // And apply it
    std::sort (functions.begin (), functions.end ());
    while (answ_prem--) {
        std::next_permutation (functions.begin (), functions.end ());
    }

    return true;
}