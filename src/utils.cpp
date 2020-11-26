#include "utils.h"
#include <fstream>

int readMiddleburyDMin(char* filname)
{
    std::ifstream f;
    std::string s_dmin;

    f.open(filname, std::ifstream::in);
    std::getline(f, s_dmin);

    return std::stoi(s_dmin);
}
