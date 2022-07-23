#include "algorithm.hpp"

#include <filesystem>
#include <fstream>
#include <vector>

using pair_string = std::pair<std::string, std::string>;
using FreqTable = std::map<pair_string, uint64_t>;
FreqTable get_freq_table (const std::string &command, const int runs);

namespace FunctionReordering {

void C3Reorder::build_edges_cg (std::unordered_map<std::string, HFData::node *> &f2n,
                                const std::vector<perfParser::LbrSample> &samples
                                __attribute__ ((unused)),
                                const perfParser::LbrTraceType type)
{
    auto table = get_freq_table (command_, runs_);
    for (const auto &[names, count] : table) {
        const auto &caller = names.first;
        const auto &callee = names.second;

        if (f2n.find (caller) == f2n.end () || f2n.find (callee) == f2n.end ()) {
            continue;
        }

        auto edge = new HFData::edge;
        edge->caller = f2n[caller];
        edge->callee = f2n[callee];
        if (type == perfParser::LbrTraceType::CYCLE)
            edge->freq = count;
        else if (type == perfParser::LbrTraceType::TLB_MISS)
            edge->miss = count;
        else
            throw std::runtime_error ("Bad samples type in CG building");

        edges_.push_back (edge);
    }
}

void C3Reorder::build_cg ()
{
    /* Declare node for each function, readed from final binary */
    for (const auto &func : nmFunctions_) {
        nodes_.push_back ({func.name_, func.size_, nullptr});
    }

    /* Make map for fast searching node by the function name */
    std::unordered_map<std::string, HFData::node *> f2n;  // func to node
    for (auto &node : nodes_) {
        f2n[node.name_] = &node;
    }

    if (!std::filesystem::exists ("saved.edges")) {
        std::cout << "Cant fild saved file with profile, do perf ....\n";
        /* Iterate over all lbr samples and increment edge counter */
        build_edges_cg (f2n, cyclesSample_, perfParser::LbrTraceType::CYCLE);
        // build_edges_cg (f2e, f2n, tlbMissesSamples_, perfParser::LbrTraceType::TLB_MISS);
        std::ofstream output ("saved.edges");
        for (auto &e : edges_) {
            output << e->caller->name_ << "/" << e->callee->name_ << "/" << e->freq << "\n";
        }
    }
    else {
        std::cout << "Found saved profile, use it\n";
        std::ifstream in ("saved.edges");
        std::string buf;
        while (std::getline (in, buf)) {
            boost::trim (buf);
            std::vector<std::string> noWS;
            boost::split (noWS, buf, boost::is_any_of ("/"), boost::token_compress_on);
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

    /* Create a cluster for each function.  */
    std::vector<HFData::cluster *> clusters;
    for (auto &node : nodes_) {
        auto c = new HFData::cluster (&node, node.size_, 0u, 0u);
        node.aux_ = c;
        clusters.push_back (c);
    }

    /* Insert edges_ between clusters that have a profile.  */
    std::vector<HFData::cluster_edge *> edges;
    for (auto &c : clusters) {
        auto node = c->m_functions[0];
        for (auto &cs : node->callers) {
            auto caller = (HFData::cluster *)cs->caller->aux_;
            caller->m_freq += cs->freq;
            caller->m_miss += cs->miss;
            auto callee = (HFData::cluster *)cs->callee->aux_;
            auto freq = cs->freq;
            auto miss = cs->miss;

            auto cedge = callee->get (caller);
            if (cedge != nullptr) {
                cedge->m_count += freq;
                cedge->m_miss += miss;
            }
            else {
                auto cedge = new HFData::cluster_edge (caller, callee, freq, miss);
                edges.push_back (cedge);
                callee->put (caller, cedge);
            }
        }
    }

    /* Now insert all created edges into a heap.  */
    // in original code we extract min from heap, and use inverted count
    std::vector<HFData::cluster_edge *> heap (edges);

    auto edge_cmp = [] (const HFData::cluster_edge *l, const HFData::cluster_edge *r) {
        return l->get_cost () < r->get_cost ();
    };

    /* Main loop */
    while (!heap.empty ()) {
        std::sort (heap.begin (), heap.end (), edge_cmp);
        auto cedge = heap.back ();  // extract edge with max weigth
        heap.pop_back ();

        auto caller = cedge->m_caller;
        auto callee = cedge->m_callee;

        if (caller == callee)
            continue;
        if (caller->m_size + callee->m_size <= HFData::C3_CLUSTER_THRESHOLD) {
            caller->m_size += callee->m_size;

            caller->m_freq += callee->m_freq;  // Instead of m_time.
            caller->m_miss += callee->m_miss;  // Same.
            // caller->m_time += callee->m_time;

            /* Append all cgraph_nodes from callee to caller.  */
            for (unsigned i = 0; i < callee->m_functions.size (); i++) {
                caller->m_functions.push_back (callee->m_functions[i]);
            }

            callee->m_functions.clear ();
            callee->m_size = 0;
            callee->m_freq = 0;
            callee->m_miss = 0;

            /* Iterate all cluster_edges of callee and add them to the
             * caller. */
            for (auto &it : callee->m_callers) {
                it.second->m_callee = caller;
                auto ce = caller->get (it.first);

                if (ce != nullptr) {
                    ce->m_count += it.second->m_count;
                    ce->m_miss += it.second->m_miss;
                }
                else
                    caller->put (it.first, it.second);
            }

            callee->m_callers.clear ();
        }
    }

    /* Sort the candidate clusters.  */
    std::sort (
        clusters.begin (),
        clusters.end (),
        [&] (HFData::cluster *a, HFData::cluster *b) -> bool {
            constexpr double MAX_DENSITY = 1e+8;
            double da = a->m_size == 0 ? MAX_DENSITY : (double)a->m_freq / (double)a->m_size;
            double db = b->m_size == 0 ? MAX_DENSITY : (double)b->m_freq / (double)b->m_size;
            return da < db;
        });

    /* Dump function order */
    std::ofstream output (resPath_);
    for (auto &c : clusters) {
        if (!c->m_functions.empty ()) {
            std::cerr << "New cluster, size = " << c->m_size
                      << " samples"
                         "= "
                      << c->m_freq << "\n";
            std::cerr << "Density = " << (double)c->m_freq / c->m_size << "\n";
        }
        for (auto &func : c->m_functions) {
            output << func->name_ << '\n';
            std::cerr << func->name_ << '\n';
        }
        // std::cerr << "\n\n";
    }

    /* Release memory */
    for (auto &it : clusters)
        delete it;
    for (auto &it : edges_)
        delete it;
}

}  // namespace FunctionReordering
