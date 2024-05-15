#include "../headers/get_method.hpp"
#include "../headers/Client.hpp"
#include "../headers/multplixing.hpp"
# include "../headers/cgi.hpp"

int checkcontent = 0;
extern std::map<int, Client *> fd_maps;
std::string cookie = "";

get_method::get_method(){
}

int    get_method::get_mthod(int fd)
{

    std::map<int, Client *>::iterator it = fd_maps.find(fd);
    std::string         response;
    std::string         extention_type;
    std::stringstream   StringSize;
    std::streampos      fileSize;
    std::string         buff_s;
    std::stringstream   size;
    int                 check_path;
    int                 err_stat;
    int                 stat_;
    check_path = check_exist(it->second->requst.uri);

    if (it == fd_maps.end())
        return (1);
    fileSize = get_fileLenth(it->second->requst.uri); 
    extention_type = it->second->requst.get_exten_type(it->second->requst.uri);
    StringSize << fileSize;

    err_stat = 0;
    if (access(it->second->requst.uri.c_str(), F_OK) < 0)
    {
        err_stat = it->second->resp.response_error("404", fd);
        return 1;
    }
    if ((access(it->second->requst.uri.c_str(), R_OK) < 0) || (check_path == 2  && !it->second->requst.auto_index_stat))
    {
        err_stat = it->second->resp.response_error("403", fd);
        return 1;
    }         
    if (check_path == 1)
    {
        if (fd_maps[fd]->cgi_->stat_cgi)
            return cgi::cgiresponse(fd);
        if (!it->second->res_header) {

            response = it->second->resp.get_header("200", extention_type, StringSize.str(), *it->second);
            it->second->read_f.open(it->second->requst.uri.c_str());
            if (!it->second->read_f.is_open())
                return 1;
            stat_ = send(fd, response.c_str(), response.size(), 0);
            if (stat_ == -1 || stat_ == 0)
            {
                it->second->read_f.close();
                return 1;
            }
            fd_maps[fd]->start_time = time(NULL);
        }
        else
        {
            char    buff[1024];
            int     x = it->second->read_f.read(buff, 1024).gcount();
            if (it->second->read_f.gcount() > 0) {
                stat_ = send(fd, buff, x, 0);
                if (stat_ == -1 || stat_ == 0)
                {
                    it->second->read_f.close();
                    return 1;
                }
                fd_maps[fd]->start_time = time(NULL);
            }
            if (it->second->read_f.eof() || it->second->read_f.gcount() < 1024)
            {
                it->second->rd_done = 1;
                it->second->read_f.close();
                return 1;
            }
        }
    }
    else if (check_path == 2 && it->second->requst.auto_index_stat)
    {
        if (it->second->requst.uri[it->second->requst.uri.length() -1] != '/')
        {
            err_stat = it->second->resp.response_error("301", fd);
            if (err_stat)
                return 1;
        }
        buff_s = generat_html_list(it->second->requst.uri.substr(0, it->second->requst.uri.find_last_of("/")));
        size << buff_s.size();
        if (!it->second->res_header)
        {
            response = it->second->resp.get_header("200", "text/html", size.str(), *it->second);
            stat_ = send(fd, response.c_str(), response.size(), 0);
            if (stat_ == -1 || stat_ == 0)
                return 1;
            fd_maps[fd]->start_time = time(NULL);
            it->second->res_header = 1;
        }
        else if (it->second->res_header)
        { 
            stat_ = send(fd, buff_s.c_str(), buff_s.size(), 0);
            if (stat_ == -1 || stat_ == 0)
                return 1;
            fd_maps[fd]->start_time = time(NULL);
            it->second->rd_done = 1;
            return 1;
        }
    }
    return 0;
}

std::string     get_method::get_exten_type(std::string path, std::map<std::string, std::string> &exta)
{
    std::string exten;
    size_t      pos = path.find_last_of(".");
    exten = path.substr(pos + 1);
    if (!exten.empty())
        return ("application/octet-stream");
    std::map<std::string, std::string>::iterator b = exta.find(exten);
    if (b != exta.end())
        return ((*b).second);
    return ("Unsupported");
}

std::streampos  get_method::get_fileLenth(std::string path)
{
    std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open())
        return -1;
    file.seekg(0, std::ios::end);
    std::streampos file_Size = file.tellg();
    file.seekg(0, std::ios::beg);
    file.close();
    return file_Size;
}

std::string    get_method::generat_html_list(std::string directory)
{
    std::string resp;

    DIR *dir = opendir(directory.c_str());

    if (dir)
    {
        resp += "<html><head><title>Index of " + directory + " </title></head><body>";
        resp += "<h1>Index of " + directory + "</h1><hr>";

        struct dirent* entry;
        while ((entry = readdir(dir)))
        {
            if (std::string(entry->d_name).compare(".") && std::string(entry->d_name).compare(".."))
                resp += "<a href=\""+ std::string(entry->d_name) + " \">" + std::string(entry->d_name) + "</a><br>";
        }
        resp += "<hr></body></html>";
        closedir(dir);
    }
    else
    {
        perror("Folder Not Found");
        exit(EXIT_FAILURE);
    }
    return resp;
}

std::string            get_method::get_index_file(std::map<std::string, std::string> &loca_map)
{
    std::map<std::string, std::string>::iterator it_e = loca_map.end();
    for (std::map<std::string, std::string>::iterator it_b = loca_map.begin(); it_b != it_e; it_b++)
    {
        if (!(*it_b).first.compare("index"))
        {
            return ((*it_b).second);
        }
    }
    return (0);
}

bool            get_method::check_autoindex(std::map<std::string, std::string> loca_map)
{
    std::map<std::string, std::string>::iterator it_e = loca_map.end();
    for (std::map<std::string, std::string>::iterator it_b = loca_map.begin(); it_b != it_e; it_b++)
    {
        if (!(*it_b).first.compare("autoindex"))
        {
            if (!(*it_b).second.compare("on"))
                checki = true;
            break;
        }
    }
    return (checki);
}

int     get_method::check_exist(const std::string& path) 
{
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == 0) 
    {
        if (S_ISREG(fileStat.st_mode)) 
            return 1;
        if (S_ISDIR(fileStat.st_mode))
            return 2;
    }
    return 0;
}

