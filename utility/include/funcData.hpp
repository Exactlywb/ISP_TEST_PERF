#ifndef _FUNC_DATA_HPP__
#define _FUNC_DATA_HPP__

#include <iostream>
#include <vector>
#include <unordered_map>
#include <stdint.h>

#include <boost/heap/fibonacci_heap.hpp>

namespace HFData {

const int C3_CLUSTER_THRESHOLD = 1024;

struct node {

    std::string name_;
    std::size_t size_;

};

struct cluster_edge;

struct cluster
{
    cluster (node *node, int size, std::size_t freq, std::size_t miss):
        m_functions (), m_callers (), m_size (size), m_freq (freq), m_miss (miss)
    {
        m_functions.push_back (node);
    }

    std::vector <node*> m_functions;
    
    std::unordered_map <cluster *, cluster_edge *> m_callers;
    int m_size;

    std::size_t m_freq;
    std::size_t m_miss;
};

/* Cluster edge is an oriented edge in between two clusters.  */

struct cluster_edge
{
    cluster_edge (cluster *caller, cluster *callee, uint32_t count):
        m_caller (caller), m_callee (callee), m_count (count), m_heap_node (NULL)
    {}

    uint32_t inverted_count ()
    {
        return UINT32_MAX - m_count;
    }

    cluster *m_caller;
    cluster *m_callee;
    uint32_t m_count;
    boost::heap::fibonacci_heap<uint32_t, cluster_edge> *m_heap_node;
};

}

#endif
