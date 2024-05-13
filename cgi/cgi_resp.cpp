#include "../headers/get_method.hpp"
#include "../headers/Client.hpp"
#include "../headers/multplixing.hpp"
# include "../headers/cgi.hpp"

extern std::map<int, Client *> fd_maps;
std::ifstream  cgi::output;
std::string    cgi::file_out;
std::string    cgi::cookie;
int headsize;

std::string to_lower(std::string str) {
    std::transform(str.begin(), str.end(), str.begin(), ::tolower);
    return str;
}

std::string to_string(int num) {
    std::ostringstream oss;
    oss << num;
    return oss.str();
}


long estimate_file_size(const std::string& filename, int fd) {
    FILE* file = fopen(filename.c_str(), "rb"); // Open in binary mode

    if (!file) {
        if (fd_maps[fd]->resp.response_error("500", fd)) {
           multplixing::close_fd(fd, fd_maps[fd]->epoll_fd);
           return -1;
        }
    }
    long current_pos = ftell(file);
    fseek(file, 0, SEEK_END);

    size_t file_size = static_cast<size_t>(ftell(file));

    fseek(file, current_pos, SEEK_SET);
    fclose(file);

    return file_size;
}

std::string getHeaders(std::ifstream& file) {
    std::string result;
    char nextChar;

    while (file.get(nextChar)) {
        result += nextChar;
        if (result.size() >= 4 && result.substr(result.size() - 4) == "\r\n\r\n") {
        headsize = result.size();
        result.erase(result.size() - 4);
        break;
        }
    }

    return result;
}

std::string getBody(std::ifstream& file) {
    std::string result;
    char nextChar;
    bool isheader = true;

    while (file.get(nextChar)) {
        if (!isheader)
            result += nextChar;
        if (result.size() >= 4 && result.substr(result.size() - 4) == "\r\n\r\n") {
            result.erase(result.size() - 4);
            isheader = false;
        }
    }
    return result;
}


void cgi::getphpheader(std::string& status, std::string& contenttype, std::string& cookie) {
    std::string header = getHeaders(output);
    std::stringstream stream(header);
    std::string line;
    status = "200 OK";
    contenttype = "text/html";
    while (getline(stream, line)) {
        if (to_lower(line.substr(0, 12)) == "content-type") {
            contenttype = line.substr(14);
        }
        if (to_lower(line.substr(0, 6)) == "status") {
            status = line.substr(8);
        }
        if (to_lower(line.substr(0, 10)) == "set-cookie") {
            cookie = line.substr(12);
        }
    }
}

int cgi::sendResp(int fd) {
    if (!fd_maps[fd]->completed) {
        std::string status;
        std::string contenttype;
        std::string contentlength = to_string(estimate_file_size(file_out, fd));
        cookie = "";
        std::string body;

        if (fd_maps[fd]->requst.uri.substr(fd_maps[fd]->requst.uri.find_last_of(".") + 1) == "php") {
            getphpheader(status, contenttype, cookie);
            if (estimate_file_size(file_out, fd) == -1) {
                return 1;
            }
            contentlength = to_string(estimate_file_size(file_out, fd) - headsize);
        }
        else if (fd_maps[fd]->requst.uri.substr(fd_maps[fd]->requst.uri.find_last_of(".") + 1) == "py" && !fd_maps[fd]->is_error) {
            status = "200 OK";
            contenttype = "text/html";
        }
        else if (fd_maps[fd]->requst.uri.substr(fd_maps[fd]->requst.uri.find_last_of(".") + 1) == "py" && fd_maps[fd]->is_error) {
            status = "500 Internal Server Error";
            contenttype = "text/html";
            body = "<h1>500 Internal Server Error</h1>";
            contentlength = to_string(body.length());
        }
        if (fd_maps[fd]->iscgitimeout) {
            status = "504 Gateway Timeout";
            contenttype = "text/html";
            body = "<h1>504 Gateway Timeout</h1>";
            contentlength = to_string(body.length());
        }
        std::string httpResponse = "HTTP/1.1 " + status + "\r\n";
        httpResponse += "Content-Type: " + contenttype + "\r\n";
        if (cookie != "")
            httpResponse += "Set-Cookie: " + cookie + "\r\n";
        httpResponse += "Content-Length: " + contentlength + "\r\n";
        httpResponse += "\r\n";

        if (fd_maps[fd]->is_error || fd_maps[fd]->iscgitimeout) {
            httpResponse += body;
        }
        int c = send(fd, httpResponse.c_str(), httpResponse.length(), 0);
        fd_maps[fd]->completed = 1;
        output.open(file_out.c_str());
        output.seekg(0, std::ios::end);
        if (c == -1 || c == 0) {
            output.close();
            multplixing::close_fd(fd, fd_maps[fd]->epoll_fd);
            return 0;
        }
        if (fd_maps[fd]->is_error || fd_maps[fd]->iscgitimeout || !output.is_open()) {
            output.close();
            multplixing::close_fd(fd, fd_maps[fd]->epoll_fd);
            return 0;
        }
    }
    else if (fd_maps[fd]->completed) {
        // char buf[1024];
        // int c = output.read(buf, 1024).gcount();
        // if (c == -1 || c == 0) {
        //     output.close();
        //     multplixing::close_fd(fd, fd_maps[fd]->epoll_fd);
        //     return 0;
        // }
        // int x = send(fd, buf, c, 0);
        // if (x == -1 || x == 0) {
        //     output.close();
        //     multplixing::close_fd(fd, fd_maps[fd]->epoll_fd);
        //     return 0;
        // }
        // if (output.eof() || output.gcount() < 1024) {
        //     output.close();
        //     multplixing::close_fd(fd, fd_maps[fd]->epoll_fd);
        //     return 0;
        // }
        std::stringstream ss;
        ss << output.rdbuf(); //MAKHDAMCH
        int x = send(fd, ss.str().c_str(), ss.str().length(), 0);
        if (x == -1 || x == 0) {
            output.close();
            multplixing::close_fd(fd, fd_maps[fd]->epoll_fd);
            return 0;
        }
        if (output.eof() || output.gcount() < 1024) {
            output.close();
            multplixing::close_fd(fd, fd_maps[fd]->epoll_fd);
            return 0;
        }
    }
    return 1;
}

int  cgi::cgiresponse(int fd) {
    int status;
    time_t end = time(NULL);
    int wait = waitpid(fd_maps[fd]->cgi_->clientPid, &status, WNOHANG);
    if (wait == fd_maps[fd]->cgi_->clientPid) {
        output.open(file_out.c_str());
        if (!output.is_open()) {
            if (fd_maps[fd]->resp.response_error("500", fd)) {
                if (multplixing::close_fd(fd, fd_maps[fd]->epoll_fd)) {
                    isfdclosed = true;
                    return 1;
                }
            }
        }
        if (WIFSIGNALED(status) || status) {
            fd_maps[fd]->is_error = 1;
            if (!sendResp(fd)) {
                isfdclosed = true;
                return 1;
            }
        }
        else {
            fd_maps[fd]->is_error = 0;
            if (!sendResp(fd)) {
                isfdclosed = true;
                return 1;
            }
        }
    }
    else if (fd_maps[fd]->cgi_->stat_cgi && fd_maps[fd]->completed) {
        fd_maps[fd]->is_error = 0;
        if (!sendResp(fd)) {
            isfdclosed = true;
            return 1;
        }
    }
    else if (difftime(end, fd_maps[fd]->cgi_->start_time) > 7) {
        fd_maps[fd]->completed = 0;
        fd_maps[fd]->iscgitimeout = 1;
        fd_maps[fd]->is_error = 1;
        if (!sendResp(fd)) {
            isfdclosed = true;
            return 1;
        }
    }
    else {
        fd_maps[fd]->rd_done = 0;
        return 0;
    }
    return 0;
}