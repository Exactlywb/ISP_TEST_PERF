#ifndef _FUNC_DATA_HPP__
#define _FUNC_DATA_HPP__

#include <stdint.h>

#include <boost/heap/fibonacci_heap.hpp>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace HFData {

    constexpr int C3_CLUSTER_THRESHOLD = 0x1000 * 64;

    struct node;

    struct edge {
        node *caller{nullptr};
        node *callee{nullptr};

        std::size_t freq = 0;
        std::size_t miss = 0;
    };

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

    struct cluster_edge;

    struct cluster {
        cluster (node *node, int size, std::size_t freq, std::size_t miss)
            : m_functions (),
              m_callers (),
              m_size (size),
              m_freq (freq),
              m_miss (miss)
        {
            m_functions.push_back (node);
        }

        std::vector<node *> m_functions;

        cluster_edge *get (cluster *caller)
        {
            auto search_it = m_callers.find (caller);
            return search_it == m_callers.end () ? nullptr : search_it->second;
        }

        void put (cluster *caller, cluster_edge *edge)
        {
            m_callers[caller] = edge;
        }

        std::unordered_map<cluster *, cluster_edge *> m_callers;
        int m_size;

        std::size_t m_freq = 0;
        std::size_t m_miss = 0;
    };

    /* Cluster edge is an oriented edge in between two clusters.  */

    struct cluster_edge {
        cluster_edge (cluster *caller,
                      cluster *callee,
                      uint32_t count,
                      uint32_t miss)
            : m_caller (caller),
              m_callee (callee),
              m_count (count),
              m_miss (miss)
        {
        }

        double get_cost_() const {
            double new_dencity = (double)(m_caller->m_freq + m_callee->m_freq)/(double)(m_caller->m_size + m_callee->m_size);
            double old_dencity = ((double)m_caller->m_freq/(double)m_caller->m_size + (double)m_callee->m_freq/(double)m_callee->m_size) * 0.5;
            return (double)m_count * new_dencity / old_dencity;
        }

        double get_cost() const {
            return (double)m_count;
        }

        uint32_t inverted_count () const { return UINT32_MAX - m_count; }

        cluster *m_caller;
        cluster *m_callee;
        uint32_t m_count = 0;
        uint32_t m_miss = 0;
    };

}  // namespace HFData

#endif
