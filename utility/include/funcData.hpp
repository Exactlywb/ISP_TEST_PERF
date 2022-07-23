#ifndef _FUNC_DATA_HPP__
#define _FUNC_DATA_HPP__

#include <stdint.h>

#include <boost/heap/fibonacci_heap.hpp>
#include <iostream>
#include <unordered_map>
#include <vector>
#include <tuple>

namespace HFData {

constexpr int C3_CLUSTER_THRESHOLD = 0x1000 * 64;

struct edge;
struct node {
    std::string name_;
    std::size_t size_;
    void *aux_;

    node (const std::string &name, std::size_t size, void *aux)
        : name_ (name), size_ (size), aux_ (aux)
    {
    }

    std::vector<edge *> callers;
};

struct edge {
    node *caller{nullptr};
    node *callee{nullptr};

    std::size_t freq = 0;
    std::size_t miss = 0;
};



struct cluster_edge;
struct cluster {

    cluster() {} 

    std::vector<node *> m_functions;
    std::unordered_map<cluster *, cluster_edge *> m_callers;
    std::size_t m_size = 0;
    std::size_t m_freq = 0;
    std::size_t m_miss = 0;

    void put (cluster *caller, cluster_edge *edge) { m_callers[caller] = edge; }
    cluster_edge *get (cluster *caller)
    {
        auto search_it = m_callers.find (caller);
        return search_it == m_callers.end () ? nullptr : search_it->second;
    }

    void reset_metrics() 
    {
        m_size = 0;
        m_freq = 0;
        m_miss = 0;
        m_callers.clear();
    }

    static void merge_to_caller(cluster *caller, cluster *callee);

};

/* Cluster edge is an oriented edge in between two clusters.  */

struct cluster_edge {
    cluster *m_caller;
    cluster *m_callee;
    uint32_t m_count = 0;
    uint32_t m_miss = 0;

    cluster_edge (cluster *caller, cluster *callee, uint32_t count, uint32_t miss)
        : m_caller (caller), m_callee (callee), m_count (count), m_miss (miss)
    {
    }

    double get_cost_ () const
    {
        double new_dencity = (double)(m_caller->m_freq + m_callee->m_freq) /
                             (double)(m_caller->m_size + m_callee->m_size);
        double old_dencity = ((double)m_caller->m_freq / (double)m_caller->m_size +
                              (double)m_callee->m_freq / (double)m_callee->m_size) *
                             0.5;
        return (double)m_count * new_dencity / old_dencity;
    }

    double get_cost () const { return (double)m_count; }
};

}  // namespace HFData

#endif
