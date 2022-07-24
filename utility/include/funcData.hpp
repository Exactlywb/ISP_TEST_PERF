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
struct cluster;
struct node {
    std::string name_;
    std::size_t size_;
    std::size_t freq_;
    cluster *aux_;

    node (const std::string &name, std::size_t size, cluster *aux)
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

    std::tuple<cluster *, cluster *, std::size_t, std::size_t> unpack_data() const {
        return {caller->aux_, callee->aux_, freq, miss};
    }
};



struct cluster_edge;
struct cluster {

    cluster() {} 

    std::vector<node *> m_functions;
    std::unordered_map<cluster *, cluster_edge *> m_callers;
    std::size_t m_size = 0;
    std::size_t m_freq = 0;
    std::size_t m_miss = 0;
    std::size_t ID = 0;


    bool try_best_reorder();

    void add_function_node(node *function_node) {
        m_functions.push_back(function_node);
        m_size += function_node->size_;
    }
    void drop_all_functions() {
        m_functions.clear();
        m_functions.shrink_to_fit();
    }


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
    static bool comparator(cluster *lhs, cluster *rhs) 
    {
        constexpr double MAX_DENSITY = 1e+8;
        double da = lhs->m_size == 0 ? MAX_DENSITY : (double)lhs->m_freq / (double)lhs->m_size;
        double db = rhs->m_size == 0 ? MAX_DENSITY : (double)rhs->m_freq / (double)rhs->m_size;
        if (std::abs(da - db) < 1e-5) 
            return lhs->ID < rhs->ID;
        return da < db;
    }

    template<typename StreamT>
    void print(StreamT &stream, bool only_funcs)
    {
        if (only_funcs) {
            for (auto node : m_functions)
                stream << node->name_ << '\n';
            return;
        }

        if (m_functions.empty()) return;
        stream << "Cluster{\n  size = " << m_size << "\n";
        stream << "  samples = " << m_freq << "\n";
        stream << "  dencity = " << (double)m_freq / m_size << "\n";
        stream << "  functions = " << m_functions.size() << "\n";
        if(0)for (auto node : m_functions) {
            stream << "    func.name = " << node->name_ << "\n";
        }
        stream << "}\n";
    }

};

/* Cluster edge is an oriented edge in between two clusters.  */

struct cluster_edge {
    cluster *m_caller;
    cluster *m_callee;
    uint32_t m_count = 0;
    uint32_t m_miss = 0;
    uint32_t ID = 0;

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

    double get_cost () const { return (double)m_count/(1 + m_callee->m_size + m_caller->m_size); }
    static bool comparator(cluster_edge *lhs, cluster_edge *rhs) 
    {
        auto cl = lhs->get_cost();
        auto cr = rhs->get_cost();
        if (cl != cr) return cl < cr;

        auto sl = lhs->m_callee->m_size + lhs->m_caller->m_size;
        auto sr = rhs->m_callee->m_size + rhs->m_caller->m_size;
        if (sl != sr) return sl > sr; // prefer small blocks

        return lhs->ID < rhs->ID;
    }
};

}  // namespace HFData

#endif
