#include "../Client.hpp"

extern std::map<int, Client>  fd_maps;

/*-- My Global variables --*/

std::ofstream outFile;
std::string extension;
std::string file = "";

// for binary;
ssize_t body_size = 0;

// for chunked;
size_t chunk_length = 0;
std::stringstream ss;
std::string hexa;
std::string concat;
int chunked_len = 0;
int f = 0;

post::post()
{
    // // "Default constructor called\n";
}

post::post(const post &other)
{
    // // "Copy constructor called\n";
    *this = other;
}

post &post::operator=(const post &other)
{
    (void)other;
    // // "Copy assignment operator called\n";
    // if (this != &other)
    return *this;
}

post::~post()
{
    // // "Destructor called\n";
}

bool post::is_end_of_chunk(std::string max_body_size, std::string upload_path)
{
    if (concat.find("\r\n0\r\n\r\n") != std::string::npos || chunk_length == 0)
    {
        outFile << concat.substr(0, concat.find("\r\n0\r\n\r\n"));
        chunked_len += concat.substr(0, concat.find("\r\n0\r\n\r\n")).length();
        outFile.close();
        outFile.clear();
        concat.clear();
        content_type.clear();
        f = 0;
        if (chunked_len > atoi(max_body_size.c_str()))
        {
            // // "so????\n";
            g = 3;
            remove((upload_path + file).c_str());
            chunked_len = 0;
            return true;
        }
        // // "done;\n";
        return true;
    }
    return false;
}

bool post::extension_founded(std::string contentType)
{
    extension = "";
    map m = read_file_extensions("fileExtensions");
    map::iterator it = m.find(contentType);
    // std::cout <<"$#" <<contentType << "$%" << std::endl;
    if (it != m.end())
        extension = it->second;
    else
    {
        // std::cerr << "extension not found\n";
        return false;
    }
    return true;
}

std::string sep = "";

bool post::post_method(std::string buffer, int fd)
{
    std::map<int, Client>::iterator   it_ = fd_maps.find(fd);
    // "is cgi: =>" << it_->second.is_cgi << std::endl;
    // // "Upload_path = " << it_->second.requst.upload_path << "\n";
    // std::cout << "max_body = '" << (*fd_maps[fd].requst.it)->max_body << "'\n";
    // // "upload: " << it_->second.requst.upload_state << std::endl;
    // // "====================\n";
    // // buffer << std::endl;
    // // "====================\n";
    g = 0;
    if (buffer.find("\r\n\r\n") != std::string::npos && f == 0)
    {
        parse_header(buffer);
        if (content_type.empty() || (content_length.empty() && transfer_encoding != "chunked"))
        {
            // // "content type is empty " << content_type << std::endl;
            g = 1;
            return true;
        }
        if (!extension_founded(content_type) && content_type.substr(0, 19) != "multipart/form-data")
        {
            // // "content type: " << content_type << std::endl;
            g = 2;
            return true;
        }
        if (extension_founded(content_type))
        {
            if (it_->second.is_cgi)
            {
                // "here\n";
                outFile.open(("/tmp/" + generateCgiName()).c_str());
            }
            else
            {
                file = generateUniqueFilename() + extension;
                outFile.open((it_->second.requst.upload_path + file).c_str());
                // it_->second.requst.upload_path << std::endl;
            }
        }
        else if (it_->second.is_cgi && content_type.substr(0, 19) == "multipart/form-data")
        {
            // "boundary CGI.\n";
            outFile.open(("/tmp/" + generateCgiName()).c_str());
        }
        else if (content_type.substr(0, 19) != "multipart/form-data")
            return true;
        buffer = buffer.substr(buffer.find("\r\n\r\n") + 4);
        if (transfer_encoding == "chunked" && content_type.substr(0, 19) == "multipart/form-data")
        {
            g = 4;
            return true;
        }
        if (transfer_encoding == "chunked")
        {
            chunked_len = 0;
            parse_hexa(buffer);
        }
        else if (transfer_encoding != "chunked" && g == 10)
        {
            // // "$$$" << transfer_encoding << "$$$" << std::endl;
            g = 1;
            buffer.clear();
            return true;
        }
        else if (content_type.substr(0, 19) == "multipart/form-data")
        {
            sep = "--" + content_type.substr(30);
            content_type = content_type.substr(0, 19);
        }
        f = 1;
    }
    if (transfer_encoding == "chunked")
        return chunked(buffer, it_->second.serv_.max_body, it_->second.requst.upload_path);
    else if (content_type == "multipart/form-data" && !it_->second.is_cgi)
        return boundary(buffer, it_->second.serv_.max_body, it_->second.requst.upload_path);
    else
        return binary(buffer, it_->second.serv_.max_body, it_->second.requst.upload_path);
    return false;
}

std::string post::parse_boundary_header(std::string buffer)
{
    std::string CT = "";
    if (buffer.find("Content-Type") != std::string::npos && buffer.find("\r\n\r\n") != std::string::npos)
    {
        CT = buffer.substr(buffer.find("Content-Type"));
        CT = CT.substr(14);
        CT = CT.substr(0, CT.find("\r\n\r\n"));
    }
    return CT;
}

std::string get_name(std::string buffer)
{
    std::string name = "";
    if (buffer.find("name") != std::string::npos && buffer.find("\r\n\r\n") != std::string::npos)
    {
        name = buffer.substr(buffer.find("name"));
        name = name.substr(6);
        name = name.substr(0, name.find("\""));
    }
    return name;
}

bool post::nameExistsInVector(std::vector<std::string> vec, std::string target)
{
    return std::find(vec.begin(), vec.end(), target) != vec.end();
}

std::string post::cat_header(std::string buffer)
{
    return buffer.substr(buffer.find("\r\n\r\n") + 4);
}

int v = 0;
std::string CType = "";
std::string name = "";
std::vector<std::string> vec;
int suffix = 0;
int len = 0;

bool post::boundary(std::string buffer, std::string max_body_size, std::string upload_path)
{
    /* ----------------------------261896924513075486597166
    Content-Disposition: form-data; name=""; filename="boundary.txt"
    Content-Type: text/plain \r\n\r\n*/
    concat += buffer;
    std::string file;
    std::stringstream ss;
    while (1)
    {
        if (v == 0 && concat.find(sep) == 0)
        {
            if (concat.find("\r\n\r\n") != std::string::npos)
            {
                CType = parse_boundary_header(concat.substr(0, concat.find("\r\n\r\n") + 4));
                name = get_name(concat.substr(0, concat.find("\r\n\r\n") + 4));
                if (extension_founded(CType) && !name.empty())
                {
                    file = name + generateUniqueSuffix() + extension;
                    outFile.open((upload_path + file).c_str());
                    vec.push_back(upload_path + file);
                    v = 1;
                }
                else if (extension_founded(CType) && name.empty())
                {
                    file = generateUniqueFilename() + extension;
                    outFile.open((upload_path + file).c_str());
                    vec.push_back(upload_path + file);
                    v = 1;
                }
                else if (concat.substr(0, concat.find("\r\n\r\n") + 4).find("filename") == std::string::npos && !name.empty())
                {
                    file = name + generateUniqueSuffix() + ".txt";
                    outFile.open((upload_path + file).c_str());
                    vec.push_back(upload_path + file);
                    v = 1;
                }
                else if (concat.substr(0, concat.find("\r\n\r\n") + 4).find("filename") == std::string::npos && name.empty())
                {
                    file = generateUniqueFilename() + ".txt";
                    outFile.open((upload_path + file).c_str());
                    vec.push_back(upload_path + file);
                    v = 1;
                }
                else if (concat.substr(0, concat.find("\r\n\r\n") + 4).find("filename") != std::string::npos && concat.substr(0, concat.find("\r\n\r\n") + 4).find("Content-Type") == std::string::npos && !name.empty())
                {
                    file = name + generateUniqueSuffix() + ".txt";
                    outFile.open((upload_path + file).c_str());
                    vec.push_back(upload_path + file);
                    v = 1;
                }
                else if (concat.substr(0, concat.find("\r\n\r\n") + 4).find("filename") != std::string::npos && concat.substr(0, concat.find("\r\n\r\n") + 4).find("Content-Type") == std::string::npos && name.empty())
                {
                    file = generateUniqueFilename() + ".txt";
                    outFile.open((upload_path + file).c_str());
                    vec.push_back(upload_path + file);
                    v = 1;
                }
                else
                {
                    for (size_t i = 0; i < vec.size(); i++)
                        remove(vec.at(i).c_str());
                    outFile.close();
                    vec.clear();
                    concat.clear();
                    CType.clear();
                    g = 2;
                    v = 0;
                    f = 0;
                    return true;
                }
                concat = cat_header(concat);
            }
            else
                return false;
        }
        if (outFile.is_open() == true && (concat.find("\r\n" + sep) != std::string::npos))
        {
            outFile << concat.substr(0, concat.find("\r\n" + sep));
            len += concat.substr(0, concat.find("\r\n" + sep)).length();
            outFile.close();
            concat = concat.substr(concat.find(sep));
            v = 0;
        }
        else if (outFile.is_open() == true)
        {
            if (concat.length() > sep.length())
            {
                outFile << concat.substr(0, concat.length() - sep.length());
                len += concat.substr(0, concat.length() - sep.length()).length();
                if (len > atoi(max_body_size.c_str()))
                {
                    for (size_t i = 0; i < vec.size(); i++)
                        remove(vec.at(i).c_str());
                    outFile.close();
                    vec.clear();
                    concat.clear();
                    CType.clear();
                    f = 0; // header flag;
                    v = 0;
                    g = 3; // request flag;
                    return true;
                }
                concat = concat.substr(concat.length() - sep.length());
            }
            return false;
        }
        if (concat == (sep + "--\r\n"))
        {
            // "done1.\n";
            concat.clear();
            outFile.close();
            CType.clear();
            vec.clear();
            v = 0;
            f = 0;
            return true;
        }
    }
    return false;
}

void post::parse_hexa(std::string &remain)
{
    ss << std::hex << remain.substr(0, remain.find("\r\n"));
    ss >> chunk_length;
    ss.str("");
    remain = remain.substr(remain.find("\r\n") + 2); // the remaining body after hexa\r\n. if after hexa is \r\n it means that "\r\n0\r\n\r\n".
}

bool post::chunked(std::string buffer, std::string max_body_size, std::string upload_path)
{
    if (outFile.is_open())
    {
        concat += buffer;
        if (concat.length() >= (chunk_length + 9))
        {
            outFile << concat.substr(0, chunk_length);
            chunked_len += concat.substr(0, chunk_length).length();
            concat = concat.substr(chunk_length + 2);
            parse_hexa(concat);
        }
        return is_end_of_chunk(max_body_size, upload_path);
    }
    else
        std::cerr << "Error opening file.\n";
    return false;
}

bool post::binary(std::string buffer, std::string max_body_size, std::string upload_path)
{
    // buffer << std::endl;
    if (outFile.is_open())
    {
        outFile << buffer;
        body_size += buffer.size();
        if (body_size > atoi(max_body_size.c_str()) || atoi(content_length.c_str()) > atoi(max_body_size.c_str()))
        {
            outFile.close();
            remove((upload_path + file).c_str());
            buffer.clear();
            body_size = 0;
            content_type.clear();
            f = 0; // header flag;
            g = 3; // request flag;
            return true;
        }
        else if (body_size > atoi(content_length.c_str()))
        {
            outFile.close();
            std::cout << "here.\n";
            remove((upload_path + file).c_str());
            buffer.clear();
            body_size = 0;
            content_type.clear();
            f = 0; // header flag;
            g = 1; // request flag;
            return true;
        }
        else if (body_size == atoi(content_length.c_str()))
        {
            outFile.close();
            buffer.clear();
            content_type.clear();
            body_size = 0;
            f = 0; // header flag;
            g = 0; // request flag;
            // "The End.\n";
            return true;
        }
        // time out should be handled in multiplixing;
    }
    else
        std::cerr << "Error opening file.\n";
    return false;
}