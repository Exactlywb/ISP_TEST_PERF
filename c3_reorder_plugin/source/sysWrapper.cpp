#include "../include/sysWrapper.hpp"

std::string exec (const char *cmd)
{
    char buffer[128];

    std::string result = "";

    FILE *pipe = popen (cmd, "r");

    if (!pipe)
        throw std::runtime_error ("popen() failed!");

    try {
        while (fgets (buffer, sizeof buffer, pipe) != NULL)
            result += buffer;
    }
    catch (...) {  // so sorry...

        pclose (pipe);
        throw;
    }

    pclose (pipe);
    return result;
}

std::string cpp_filt (const char *asmName)
{
    std::string cppCommand =
        std::string ("c++filt ") + std::string (asmName);

    return boost::trim_copy (exec (cppCommand.c_str ()));
}
