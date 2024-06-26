#include "../headers/Client.hpp"

void post::print_keyVal(map m)
{
    map::iterator it = m.begin();
    while (it != m.end())
    {
        it++;
    }
}

std::string post::generateUniqueFilename()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    std::ostringstream filename_stream;
    filename_stream << "outfile_" << tv.tv_sec << "-" << tv.tv_usec;

    return filename_stream.str();
}

std::string post::generateCgiName()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    std::ostringstream filename_stream;
    filename_stream << "CGI_in_" << tv.tv_sec << "-" << tv.tv_usec;

    return filename_stream.str();
}

std::string post::generateUniqueSuffix()
{
    struct timeval tv;
    gettimeofday(&tv, NULL);

    std::ostringstream filename_stream;
    filename_stream << "_" << tv.tv_sec << "-" << tv.tv_usec;

    return filename_stream.str();
}