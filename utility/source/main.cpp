/*
 * This plugin is a truncated version of Martin Liska's c3-pass
 * The original sources you may see on
 * https://gcc.gnu.org/legacy-ml/gcc-patches/2019-09/msg01142.html
 * */

#include <boost/filesystem.hpp>
#include <boost/program_options.hpp>
#include <fstream>
#include <iostream>
#include <tuple>

#include "algorithm.hpp"

static std::tuple<std::string, std::string> CheckInput (
    const int argc, char **argv)  // readelf and
{
    namespace po = boost::program_options;

    po::options_description desc ("Allowed options");
    desc.add_options () ("help,h", "Show help") (
        "readelf,r", po::value<std::string> (), "Input readelf file") (
        "output,o", po::value<std::string> (), "Output file");

    po::variables_map vm;
    po::parsed_options parsed = po::command_line_parser (argc, argv)
                                    .options (desc)
                                    .allow_unregistered ()
                                    .run ();
    po::store (parsed, vm);
    po::notify (vm);

    std::string readelf_file;
    std::string output_file;
    if (vm.count ("help")) {
        std::cout << desc << "\n";
        return {{}, {}};
    }
    if (vm.count ("readelf"))
        readelf_file = vm["readelf"].as<std::string> ();
    if (vm.count ("output"))
        output_file = vm["output"].as<std::string> ();

    return {readelf_file, output_file};
}

/*  The input format is

>  ./c3_utility nm.file resultName

*/
int main (int argc, char **argv)
{
    std::string readelf, output;
    try {
        std::tie (readelf, output) = CheckInput (argc, argv);
    }
    catch (std::runtime_error &err) {
        std::cerr << "[C3_UTILITY] Error: " << err.what () << std::endl;
        return -1;
    }

    FunctionReordering::C3Reorder reord (readelf.c_str (), readelf.c_str ());
    reord.run ();
}
