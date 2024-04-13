#include "../post.hpp"

/*-- My Global variables --*/

std::string fileName;
std::ofstream outFile;
std::string extension;

// for parsing header;
// std::string contentType;
// std::string content_length;
// std::string transfer_encoding;
server serv;
map m;

// for binary;
ssize_t body_size = 0;

//for chunked;
size_t chunk_length = 0;
std::stringstream ss;
std::string hexa;
std::string concat;
int f = 0;

post::post()
{
    // std::cout << "Default constructor called\n";
}

post::post(const post &other)
{
    // std::cout << "Copy constructor called\n";
    *this = other;
}

post& post::operator=(const post &other)
{
    (void)other;
    // std::cout << "Copy assignment operator called\n";
    // if (this != &other)
    return *this;
}

post::~post()
{
    // std::cout << "Destructor called\n";
}

bool post::is_end_of_chunk()
{
    if (concat.find("\r\n0\r\n\r\n") != std::string::npos || chunk_length == 0)
    {
        outFile << concat.substr(0, concat.find("\r\n0\r\n\r\n"));
        outFile.close();
        outFile.clear();
        concat.clear();
        f = 0;
        return true;
    }
    return false;
}

void post::open_unic_file(std::string contentType)
{
    map m = read_file_extensions("fileExtensions");
    map::iterator it = m.find(contentType);
    if (it != m.end())
        extension = it->second;
    else
        std::cerr << "extension not found\n";
    fileName = generateUniqueFilename();
    outFile.open((fileName + extension).c_str());
}

bool post::post_method(std::string buffer, request req)
{
    if (buffer.find("\r\n\r\n") != std::string::npos && f == 0)
    {
        std::cout << "======> " << req.content_type << std::endl;
        // exit(1);
        open_unic_file(req.content_type);
        buffer = buffer.substr(buffer.find("\r\n\r\n") + 4);
        if (req.transfer_encoding == "chunked")
            parse_hexa(buffer);
        f = 1;
    }
    if (req.transfer_encoding == "chunked")
        return chunked(buffer);
    else
        return binary(buffer, req);
    return false;
}

void post::parse_hexa(std::string &remain)
{
    ss << std::hex << remain.substr(0, remain.find("\r\n"));
    ss >> chunk_length;
    ss.str("");
    remain = remain.substr(remain.find("\r\n") + 2); // the remaining body after hexa\r\n. if after hexa is \r\n it means that "\r\n0\r\n\r\n".
}

bool post::chunked(std::string buffer)
{
    if (outFile.is_open())
    {
        concat += buffer;
        if (concat.length() >= (chunk_length + 9))
        {
            outFile << concat.substr(0, chunk_length);
            concat = concat.substr(chunk_length + 2);
            parse_hexa(concat);
        }
        return is_end_of_chunk();
    }
    else
        std::cerr << "Error opening file.\n";
    return false;
}

bool post::binary(std::string buffer, request req)
{
    if (outFile.is_open())
    {
        outFile << buffer;
        body_size += buffer.size();
        if (body_size == atoi(req.content_length.c_str()))
        {
            outFile.close();
            buffer.clear();
            body_size = 0;
            f = 0;
            return true;
        }
    }
    else
        std::cerr << "Error opening file.\n";
    return false;
}