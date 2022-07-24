#include "algorithm.hpp"

#include <filesystem>
#include <fstream>
#include <algorithm>
#include <vector>

using pair_string = std::pair<std::string, std::string>;
using FreqTable = std::map<pair_string, uint64_t>;
FreqTable get_freq_table (const std::string &command, const int runs, const int delta);

namespace FunctionReordering {

void C3Reorder::build_edges_cg (std::unordered_map<std::string, HFData::node *> &f2n)
{
    auto table = get_freq_table (command_, runs_, delta_);
    for (const auto &[names, count] : table) {
        const auto &caller = names.first;
        const auto &callee = names.second;

        if (f2n.find (caller) == f2n.end () || f2n.find (callee) == f2n.end ()) {
            continue;
        }

        auto edge = new HFData::edge;
        edge->caller = f2n[caller];
        edge->callee = f2n[callee];
        edge->freq = count;

        edges_.push_back (edge);
    }
}

void C3Reorder::build_cg ()
{
    const std::string SAVED_PROFILE_FILE{"saved.edges"};

    /* Declare node for each function, readed from final binary */
    for (const auto &func : nmFunctions_) {
        nodes_.push_back ({func.name_, func.size_, nullptr});
    }

    /* Make map for fast searching node by the function name */
    std::unordered_map<std::string, HFData::node *> f2n;  // func to node
    for (auto &node : nodes_) {
        f2n[node.name_] = &node;
    }

    if (!std::filesystem::exists (SAVED_PROFILE_FILE)) {
        std::cout << "Cant fild saved file with profile, do perf ....\n";
        build_edges_cg (f2n);
        std::ofstream output (SAVED_PROFILE_FILE);
        for (auto &e : edges_) {
            output << e->caller->name_ << "/" << e->callee->name_ << "/" << e->freq << "\n";
        }
    }
    else {
        std::cout << "Found saved profile, use it\n";
        std::ifstream in (SAVED_PROFILE_FILE);
        std::string buf;
        while (std::getline (in, buf)) {
            boost::trim (buf);
            std::vector<std::string> noWS;
            boost::split (noWS, buf, boost::is_any_of ("/"), boost::token_compress_on);

            auto caller = f2n[noWS[0]];
            auto callee = f2n[noWS[1]];
            if (caller == nullptr) {
                std::cerr << "Not found node for function: " << noWS[0] << "\n";
                return;
            }
            if (callee == nullptr) {
                std::cerr << "Not found node for function: " << noWS[1] << "\n";
                return;
            }

            auto edge = new HFData::edge;
            edge->caller = f2n[noWS[0]];
            edge->callee = f2n[noWS[1]];

            edge->freq = boost::lexical_cast<std::size_t> (noWS[2]);
            edges_.push_back (edge);
        }
    }

    /* After each edges was created, attach it no nodes */
    for (auto e : edges_) {
        e->callee->callers.push_back (e);
    }
}

void C3Reorder::run ()
{
    nmParser::parse_nm_data (nmFunctions_, nmPath_);

    /* Build cg from perf data */
    build_cg ();

    /* Create a cluster for each function, and accociate function to cluster.  */
    std::vector<HFData::cluster *> clusters(nodes_.size());
    for (std::size_t i = 0; i < clusters.size(); i++) {
        clusters[i] = new HFData::cluster();
        clusters[i]->add_function_node(&nodes_[i]);
        clusters[i]->ID = i;
        nodes_[i].aux_ = clusters[i];
    }

    /* Insert edges between clusters that have a profile. */
    std::vector<HFData::cluster_edge *> cedges;
    for (auto &cluster : clusters) {
        for (auto function_node : cluster->m_functions) {
            for (auto &edge : function_node->callers) {
                auto [caller_cluster, callee_cluster, freq, miss] = edge->unpack_data();

                caller_cluster->m_freq += freq;
                caller_cluster->m_miss += miss;

                auto cedge = callee_cluster->get (caller_cluster);
                if (cedge != nullptr) {
                    cedge->m_count += freq;
                    cedge->m_miss += miss;
                } else {
                    auto cedge = new HFData::cluster_edge (caller_cluster, callee_cluster, freq, miss);
                    cedges.push_back (cedge);
                    cedge->ID = cedges.size();
                    callee_cluster->put (caller_cluster, cedge);
                }
            }
        }
    }

    /* Now insert all created edges into a heap.  */
    auto &heap = cedges;
    auto extract_max = [&heap]() {
        auto &compare = HFData::cluster_edge::comparator;
        auto max_it = std::max_element(heap.begin (), heap.end (), compare);
        std::iter_swap(max_it, std::prev(heap.end()));
        auto cedge = heap.back();
        heap.pop_back();
        return cedge;
    };

    /* Main loop */
    while (!heap.empty ()) {
        auto cedge = extract_max();

        auto caller = cedge->m_caller;
        auto callee = cedge->m_callee;

        if (caller == callee)
            continue;

        if (caller->m_size + callee->m_size > HFData::C3_CLUSTER_THRESHOLD)
            continue;

        HFData::cluster::merge_to_caller(caller, callee);
    }


    /* Sort the candidate clusters.  */
    std::sort (clusters.begin (), clusters.end (), HFData::cluster::comparator);

    for (auto &cluster : clusters) {
        for (auto function_node : cluster->m_functions) {
            function_node->aux_ = cluster;
            function_node->freq_ = 0;
        }
    }

    std::map<std::pair<HFData::node *, HFData::node *>, HFData::edge *> f2e;
    for (auto e : edges_) {
        if (e->caller->aux_ == e->callee->aux_) {
            e->caller->freq_ += e->freq;
            f2e[{e->caller, e->callee}] = e;
        }
    }



    #if 0
    for (auto cluster : clusters) {
        auto fnc_cmp = [](HFData::node *a, HFData::node *b) -> bool
        {
            //return a->freq_ < b->freq_;
            return a->freq_ * b->size_ > b->freq_ * a->size_;
        };
        std::sort(cluster->m_functions.begin(), cluster->m_functions.end(), fnc_cmp);
    }
    #endif

    /* Dump function order */
    std::ofstream output (resPath_);
    for (auto &c : clusters) {
        c->try_best_reorder();
        c->print(std::cerr, false); /* only_funcs = false */
        c->print(output, true);     /* only_funcs = true */
    }

    /* Release memory */
    for (auto &it : clusters)
        delete it;
    for (auto &it : edges_)
        delete it;
}

}  // namespace FunctionReordering
