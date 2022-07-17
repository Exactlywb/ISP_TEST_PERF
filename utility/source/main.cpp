/*
 * This plugin is a truncated version of Martin Liska's c3-pass
 * The original sources you may see on
 * https://gcc.gnu.org/legacy-ml/gcc-patches/2019-09/msg01142.html
 * */

#include <iostream>

#include "algorithm.hpp"
#include <fstream>
#include <boost/filesystem.hpp>

static void CheckInput (const int argc, char** argv) {

    if (argc != 4)
        throw std::runtime_error ("Wrong number of arguments. Look the input format");

    const boost::filesystem::path nmFilePath (argv [1]);
    const boost::filesystem::path perfFilePath (argv [2]);

    if (!boost::filesystem::exists (nmFilePath))
        throw std::runtime_error ("No such nm file");
    
    if (!boost::filesystem::exists (perfFilePath))
        throw std::runtime_error ("No such perf file");

}

/*  The input format is

>  ./c3_utility nm.file perf.file resultName

*/
int main (int argc, char** argv) {

    try {
        CheckInput (argc, argv);
    } catch (std::runtime_error& err) {
        std::cerr << "[C3_UTILITY] Error: "<< err.what () << std::endl;
        return -1;
    }

    FunctionReordering::C3Reorder reord (argv [1], argv [2], argv [3]);
    reord.run ();

}
