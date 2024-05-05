#include "../get_method.hpp"
#include "../Client.hpp"
#include "../multplixing.hpp"
# include "../cgi.hpp"

int checkcontent = 0;
extern std::map<int, Client *> fd_maps;
std::string cookie = "";

get_method::get_method(){
}

std::string parsephpheader(std::string buffer)
{
    std::istringstream stream (buffer);
    std::string line;
    std::string contenttype;
    int check = 0;
    while (getline(stream, line))
    {
        if (line.substr(0, 12) == "Content-Type") {
            contenttype = line.substr(14);
            check = 1;
        }
        else if (line.substr(0, 10) == "Set-Cookie") {
            cookie = line.substr(12);
        }
    }
    if (!check)
        contenttype = "text/html";
    return contenttype;
}

std::string getContent (std::string file, int fd, std::string& contenttype) {
    std::string content;
    std::ifstream fileStream;
    fileStream.open(file.c_str());
    if (fileStream.is_open()) {
        std::stringstream sstr;
        sstr << fileStream.rdbuf();
        content = sstr.str();
        fileStream.close();
    }
    if (fd_maps[fd]->cgi_.extension == "php" && !fd_maps[fd]->cgi_.is_error) {
        size_t pos = content.find("\r\n\r\n");
        contenttype = parsephpheader(content.substr(0, pos));
        std::cout << contenttype << std::endl;
        if (pos != std::string::npos) {
            content = content.substr(pos + 4);
            return content;
        }
    }
    else {
        if (!checkcontent)
            contenttype = "text/html";
    }
    return content;
}

std::string getErrorPage(int fd, std::string stat, std::string& status, std::string& contenttype) {
    if (stat == "500")
        status = "500 Internal Server Error";
    else if (stat == "504")
        status = "504 Gateway Timeout";
    if (fd_maps[fd]->serv_.err_page.find(stat) != fd_maps[fd]->serv_.err_page.end()) {
        std::cout << "\033[1;32m test:::: " << stat  << " | '" << fd_maps[fd]->serv_.err_page[stat] << "'\033[0m" << std::endl;
        contenttype = fd_maps[fd]->requst.extentions[fd_maps[fd]->serv_.err_page[stat].substr(fd_maps[fd]->serv_.err_page[stat].find_last_of(".") + 1)];
        std::cout << "contenttype: " << contenttype << std::endl;
        return getContent(fd_maps[fd]->serv_.err_page[stat], fd, contenttype);
    }
    else {
        contenttype = "text/html";
        if (stat == "500")
            return "500 Internal Server Error";
        else if (stat == "504")
            return "504 Gateway Timeout";
    }
    return "";
}

void cgi::sendResponse(int fd, std::string& response, std::string stat, std::string& contenttype) {
    std::stringstream iss;
    std::string status = "200 OK";
    if (fd_maps[fd]->cgi_.is_error) {
        checkcontent = 1;
        response = getErrorPage(fd, stat, status, contenttype);
    };
    iss << response.length();
    std::string responseLength = iss.str();
    std::string httpResponse = "HTTP/1.1 " + status + "\r\n";
    if (cookie != "")
        httpResponse += "Set-Cookie: " + cookie + "\r\n";
    httpResponse += "Content-Type: " + contenttype + "\r\n";
    httpResponse += "Content-Length: " + responseLength + "\r\n";
    httpResponse += "\r\n";
    httpResponse += response;
    send(fd, httpResponse.c_str(), httpResponse.length(), 0);
    multplixing::close_fd(fd, fd_maps[fd]->epoll_fd);
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
    std::string         contenttype;
    cookie = "";
    check_path = check_exist(it->second->requst.uri);
    if (it == fd_maps.end()) // print error
        exit(1);
    fileSize = get_fileLenth(it->second->requst.uri); // get full lenth of the file
    extention_type = it->second->requst.get_exten_type(it->second->requst.uri);
    StringSize << fileSize;

    if (check_path == 1)
    {
        if (fd_maps[fd]->cgi_.stat_cgi) {
            time_t end = time(NULL);
            std::string cgi_file = fd_maps[fd]->cgi_.file_out; // 0000 update
            int status;
            int wait = waitpid(fd_maps[fd]->cgi_.clientPid, &status, WNOHANG);
            if (wait == fd_maps[fd]->cgi_.clientPid) {
                std::string content;
                checkcontent = 0;
                fd_maps[fd]->cgi_.is_error = 0;
                content = getContent(cgi_file, fd, contenttype);
                if (WIFSIGNALED(status) || status) {
                    fd_maps[fd]->cgi_.is_error = 1;
                    cgi::sendResponse(fd, content, "500", contenttype);
                    isfdclosed = true;
                    return 1;
                }
                else {
                    fd_maps[fd]->cgi_.is_error = 0;
                    cgi::sendResponse(fd, content, "200", contenttype);
                    isfdclosed = true;
                    return 1;
                }
            }
            else if (difftime(end, fd_maps[fd]->cgi_.start_time) > 7) {
                fd_maps[fd]->cgi_.is_error = 1;
                std::string timeout = "GATEWAY TIMEOUT";
                std::cout << "\033[1;34m" << fd_maps[fd]->cgi_.clientPid << "\033[0m" << std::endl;
                kill(fd_maps[fd]->cgi_.clientPid, 9);
                waitpid(fd_maps[fd]->cgi_.clientPid, NULL, 0);
                cgi::sendResponse(fd, timeout, "504", contenttype);
                isfdclosed = true;
                return 1;
            }
            else {
                it->second->rd_done = 0;
                return 0;
            }
        }
        if (!it->second->res_header) {
            response = it->second->resp.get_header("200", extention_type, StringSize.str(), *it->second);
            it->second->read_f.open(it->second->requst.uri.c_str());
            send(fd, response.c_str(), response.size(), 0);
        }
        else
        {
            char    buff[1024];
            int     x = it->second->read_f.read(buff, 1024).gcount();
            std::cout << x << " 3DAK W 3ADAK \n";
            if (it->second->read_f.gcount() > 0) {
                send(fd, buff, x, 0);
            }
            if (it->second->read_f.eof() || it->second->read_f.gcount() < 1024)
            {
                std::cout << "the END \n";
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
            send(fd, response.c_str(), response.size(), 0);
            fd_maps[fd]->start_time = time(NULL);
            it->second->res_header = 1;
        }
        else if (it->second->res_header)
        { 
            send(fd, buff_s.c_str(), buff_s.size(), 0);
            fd_maps[fd]->start_time = time(NULL);
            it->second->rd_done = 1;
            return 1;
        }
    }
    else // generic function .
    { 
        err_stat = 0;
        if (access(it->second->requst.uri.c_str(), F_OK) < 0)
             err_stat = it->second->resp.response_error("404", fd);
        if (check_path == 3 || (check_path == 2  && !it->second->requst.auto_index_stat)) // permission
            err_stat = it->second->resp.response_error("403", fd);
        if (err_stat)
            return 1;
    }
    return 0;
}

std::streampos  get_method::get_fileLenth(std::string path)
{
    std::ifstream file(path.c_str(), std::ios::binary | std::ios::ate);
    if (!file.is_open()) {
        return -1; // Return -1 to indicate error
    }
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
        exit(1);
    }
    return resp;
}

int     get_method::check_exist(const std::string& path) 
{
    struct stat fileStat;
    if (stat(path.c_str(), &fileStat) == 0) 
    {
        if ((fileStat.st_mode & S_IROTH) == 0)  // User does not have read permission
            return 3; // Path exists but user doesn't have permission
        if (S_ISREG(fileStat.st_mode)) 
            return 1; // Path exists and is a regular file
        if (S_ISDIR(fileStat.st_mode))
            return 2; // Path exists and is a directory
    }
    return 0; // Path does not exist
}

