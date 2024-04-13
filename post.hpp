#ifndef POST_HPP
#define POST_HPP

#include <string>
#include <sys/time.h>
#include <cstring>
#include <cstdlib>
#include <sys/wait.h>
#include <iostream>
#include <cstring>
#include <sys/epoll.h>
#include <arpa/inet.h>
#include <map>
#include <algorithm>
#include <fstream>
#include <sstream>
#include <vector>
#include "server.hpp"
#include "request.hpp"
#include "response.hpp"


#define PORT 8081

typedef std::map<std::string, std::string> map;

class post
{
    private:
        // some variables if needed;
    public:
        int j;
        post();
        post(const post &other);
        post &operator=(const post &other);
        std::string generateUniqueFilename();
        void print_keyVal(map m);
        map read_file_extensions(const char *filename);
        void PutBodyInFile(std::string buffer, std::string extension);
        bool post_method(std::string buffer, request req);
        bool binary(std::string buffer, request req);
        bool chunked(std::string buffer);
        void open_unic_file(std::string contentType);
        void parse_hexa(std::string &remain);
        bool is_end_of_chunk();
        ~post();
};

#endif