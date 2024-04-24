#include "../request.hpp"

int isIP(std::string host) {
    int dots = 0;
    int doubleDots = 0;
    for (size_t i = 0; i < host.length(); i++) {
        if (host[i] == '.')
            dots++;
        if (host[i] == ':')
            doubleDots++;
    }
    if (dots == 3 && doubleDots == 1)
        return 1;
    if (host.substr(0, host.find_first_of(':')) == "localhost")
        return 1;
    return 0;
}

int request::parseHost(std::string hst, server& pars) {
    std::string ip;
    std::string port;
    int         ip_port = 0;
    if (isIP(hst)) {
        ip = hst.substr(0, hst.find(':'));
        port = hst.substr(hst.find(':') + 1);
        // std::cout << "IP: '" << ip << "'" << " PORT: '" << port << "'" << std::endl;
        ip_port = 1;
    }
    for (it = pars.s.begin(); it != pars.s.end(); it++) {
            // std::cout << "CONFPORT: '" << (*it)->cont.find("listen")->second << "'" << " INCOMING PORT: '" << port << "'\n";
        if ((*it)->cont.find("listen")->second == port) {
            if ((*it)->cont["server_name"] == hst)
                break;
            if ((*it)->cont.find("host")->second == ip || ip == "localhost") {
                if (ip_port) {
                    break ;
                }
            }
            else
                continue;
        }
        else
            continue;
    }
    if (it == pars.s.end()) {
        perror("server NOT found");
        exit(404);
    }
    return (0);
}

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
            content_type = line.substr(14);
        else if (line.substr(0, 17) == "Transfer-Encoding")
            transfer_encoding = line.substr(19);
        else if (line.substr(0, 4) == "Host")
            parseHost(line.substr(6), serv);
        if (line == "\r")
            return ;
    }
}