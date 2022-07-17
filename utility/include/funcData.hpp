#ifndef _FUNC_DATA_HPP__
#define _FUNC_DATA_HPP__

#include <stdint.h>

#include <boost/heap/fibonacci_heap.hpp>
#include <iostream>
#include <unordered_map>
#include <vector>

namespace HFData {

    const int C3_CLUSTER_THRESHOLD = 1024;

    struct node;

    struct edge {
        node *caller{nullptr};
        node *callee{nullptr};

        size_t count;
    };

    struct node {
        std::string name_;
        std::size_t size_;
        void *aux_;

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

        std::size_t m_freq;
        std::size_t m_miss;
    };

    /* Cluster edge is an oriented edge in between two clusters.  */

    struct cluster_edge {
        cluster_edge (cluster *caller, cluster *callee, uint32_t count)
            : m_caller (caller),
              m_callee (callee),
              m_count (count),
              m_heap_node (NULL)
        {
        }

        uint32_t inverted_count () { return UINT32_MAX - m_count; }

        cluster *m_caller;
        cluster *m_callee;
        uint32_t m_count;
        boost::heap::fibonacci_heap<uint64_t, cluster_edge> *m_heap_node;
    };

}  // namespace HFData

#endif
