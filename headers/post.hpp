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

public:
    std::string transfer_encoding;
    std::string content_length;
    std::string content_type;
    std::ofstream outFile;
    std::string extension;
    std::string file;

    // for binary;
    ssize_t body_size;

    // for chunked;
    size_t chunk_length;
    std::stringstream ss;
    std::string hexa;
    std::string concat;
    int chunked_len;
    std::string sep;

    // boundary;
    int v;
    std::string CType;
    std::string name;
    std::vector<std::string> vec;
    int suffix;
    long len;

    int g;
    int j;
	post();
	post(const post &other);
	post &operator=(const post &other);
	~post();
    std::string generateUniqueFilename();
    void print_keyVal(map m);
    map read_file_extensions(const char *filename);
    void parse_header(std::string buffer);
    void PutBodyInFile(std::string buffer, std::string extension);
    bool post_method(std::string buffer, int fd);
    bool binary(std::string buffer, std::string max_body_size, std::string upload_path);
    bool chunked(std::string buffer, std::string max_body_size, std::string upload_path);
    bool extension_founded(std::string contentType);
    void parse_hexa(std::string &remain);
    bool is_end_of_chunk(std::string max_body_size, std::string upload_path);
    bool boundary(std::string buffer, std::string max_body_size, std::string upload_path);
    bool nameExistsInVector(std::vector<std::string> vec, std::string target);
    std::string parse_boundary_header(std::string buffer);
    std::string cut_header(std::string buffer);
    bool containsValidCharacters(const std::string& str);
    std::string generateUniqueSuffix();
    std::string generateCgiName();
    bool boundary_CGI(std::string buffer, std::string max_body_size);
    bool is_valid_hexa(std::string concat);
    static std::string keysToLower(std::string str);
};

#endif