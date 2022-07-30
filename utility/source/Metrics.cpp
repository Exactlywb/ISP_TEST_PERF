#include "Metrics.hpp"

double RegularFunctionMetric::operator() (const HFData::cluster &clust,
                                          const std::vector<int> &state) const
{
    const std::vector<HFData::node *> &funcs = clust.functions_;
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

double RegularClusterMetric::operator() (const std::vector<HFData::cluster *> &graph,
                                         const std::vector<int> &state) const
{
    std::vector<size_t> dists (graph.size ());
    auto dist_for_clust = [&dists, graph, state] (HFData::cluster *c) -> int {
        for (std::size_t i = 0; i < graph.size (); i++) {
            if (graph[state[i]] == c)
                return dists[state[i]];
        }
        return c->m_size;
    };

    auto get_dist_for_call = [&] (HFData::cluster *caller, HFData::cluster *callee) -> int {
        return std::abs (dist_for_clust (caller) + (int)caller->m_size / 2 -
                         dist_for_clust (callee));
    };

    dists[state[0]] = 0;
    for (std::size_t i = 1; i < graph.size (); i++) {
        dists[state[i]] = dists[state[i - 1]] + graph[state[i]]->m_size;
    }

    double cur_metric = 0;
    for (auto node : graph) {
        for (auto e : node->callers_) {
            auto caller = e.second->m_caller;
            auto callee = e.second->m_callee;
            cur_metric += e.second->m_count * (get_dist_for_call (caller, callee));
        }
    }
    return cur_metric;
}
