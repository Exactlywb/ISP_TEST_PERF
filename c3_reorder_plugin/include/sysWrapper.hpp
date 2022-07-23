#ifndef SYS_WRAPPER_HPP__
#define SYS_WRAPPER_HPP__

#include <iostream>
#include <string>
#include <boost/algorithm/string.hpp>

std::string exec (const char *cmd);
std::string cpp_filt (const char *asmName);

#endif
