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

static std::tuple<std::string, std::string, std::string, int>
CheckInput (  // command, readelf, output, runs number
    const int argc,
    char **argv)
{
    namespace po = boost::program_options;

    po::options_description desc ("Allowed options");
    auto registartor = desc.add_options ();
    registartor ("help,h", "Show help");
    registartor ("readelf,r", po::value<std::string> (), "Input readelf file");
    registartor ("output,o", po::value<std::string> (), "Output file");
    registartor ("number,N", po::value<int> (), "Number of runs");
    registartor ("command,C", po::value<std::string> (), "Command to run");

    po::variables_map vm;
    po::parsed_options parsed =
        po::command_line_parser (argc, argv).options (desc).allow_unregistered ().run ();
    po::store (parsed, vm);
    po::notify (vm);

    std::string readelf_file;
    std::string output_file;
    std::string command;
    int number_of_runs = 0;
    if (vm.count ("help")) {
        std::cout << desc << "\n";
        return {{}, {}, {}, number_of_runs};
    }
    if (vm.count ("readelf"))
        readelf_file = vm["readelf"].as<std::string> ();
    if (vm.count ("output"))
        output_file = vm["output"].as<std::string> ();
    if (vm.count ("number"))
        number_of_runs = vm["number"].as<int> ();
    if (vm.count ("command"))
        command = vm["command"].as<std::string> ();

    return {command, readelf_file, output_file, number_of_runs};
}

/*  The input format is

>  ./c3_utility nm.file resultName

*/
int main (int argc, char **argv)
{
    std::string readelf, output, command;
    int runs;
    try {
        std::tie (command, readelf, output, runs) = CheckInput (argc, argv);
        if (readelf.empty ())
            throw std::runtime_error ("No readelf file");
    }
    catch (std::runtime_error &err) {
        std::cerr << "[C3_UTILITY] Error: " << err.what () << std::endl;
        return -1;
    }

    std::cout << "File with symbols: " << readelf << std::endl;
    std::cout << "Output file: " << output << std::endl;
    std::cout << "Total runs: " << runs << std::endl;
    FunctionReordering::C3Reorder reord (command, readelf.c_str (), output.c_str (), runs);
    reord.run ();
}
