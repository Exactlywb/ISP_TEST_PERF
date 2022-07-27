#include "algos.hpp"
#include <unordered_map>

bool QAP::runOncluster(HFData::cluster &cluster)
{
    auto &functions = cluster.m_functions;
    const auto size = functions.size();

    if (functions.size() <= 1) return false;

    std::cerr << "Functions = " << size << "\n";

    // create weigth
    using ii = long long;
    using mat1 = std::vector<ii>;
    using mat2 = std::vector<mat1>;

    mat2 D(size, mat1(size));
    for (int i = 0; i < size; i++)
    for (int j = 0; j < size; j++)
        D[i][j] = std::abs(1+2*(i-j));
    
    std::unordered_map<HFData::node *, int> f2int;
    for (auto &f : functions) f2int[f] = f2int.size();

    mat2 W(size, mat1(size));
    cluster.visit_edges([&](HFData::edge *edge){
        W[f2int[edge->caller]][f2int[edge->callee]] = edge->freq;
    });

    std::string in_file = "/home/dmitry/ISP_TEST_PERF/utility/QAP/data.dat";
    std::string out_file = "/home/dmitry/ISP_TEST_PERF/utility/QAP/res.out";
    std::string exc = "/home/dmitry/ISP_TEST_PERF/utility/QAP/a.out";

    int iters = 3400 / (size + 8);

    std::ofstream in(in_file, std::ios::out);
    in << size << "\n";
    for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) 
        in << W[i][j] << " ";
    in << "\n";
    }
    for (int i = 0; i < size; i++) {
    for (int j = 0; j < size; j++) 
        in << D[i][j] << " ";
    in << "\n";
    }
    in.close();

    system((exc + " " + in_file + " " + std::to_string(iters) + " > " + out_file).c_str());

    std::ifstream out(out_file);
    mat1 perm(size);
    for (int i = 0; i < size; i++)
        out >> perm[i];

    auto funcs = functions;
    for (int i = 0; i < size; i++)
        funcs[i] = functions[perm[i]];
    functions = funcs;

    return true;
}
