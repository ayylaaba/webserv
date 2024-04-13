#include "../request.hpp"

void request::parse_header(std::string buffer, server &serv)
{
    std::istringstream stream (buffer);
    std::string line;
    getline(stream, line);
    std::vector<std::string> vec = serv.isolate_str(line , ' ');
    method = vec[0];
    path   = vec[1];
    while (getline(stream, line))
    {
        if (line.find("\r") != std::string::npos)
            line.erase(line.find("\r"));
        if (line.substr(0, 14) == "Content-Length")
            content_length = line.substr(16);
        else if (line.substr(0, 12) == "Content-Type")
        {
            content_type = line.substr(14);
        }
        else if (line.substr(0, 17) == "Transfer-Encoding")
            transfer_encoding = line.substr(19);
        if (line == "\r")
            return ;
    }
}