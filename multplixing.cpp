#include "multplixing.hpp"

#define MAX_CLIENTS 

std::map<int, Client>  fd_maps;
int flag = 0;

extern std::map<int, std::vector<server*>::iterator> server_history;
extern std::map<int, int> client_history;


in_addr_t multplixing::convertIpv4toBinary(const std::string& ip) {
    unsigned int parts[4];
    if (sscanf(ip.c_str(), "%u.%u.%u.%u", &parts[0], &parts[1], &parts[2], &parts[3]) != 4) {
        std::cerr << "Invalid IP address" << std::endl;
        exit(EXIT_FAILURE);
    }
    for (int i = 0; i < 4; i++) {
        if (parts[i] > 255) {
            std::cerr << "Invalid IP address" << std::endl;
            exit(EXIT_FAILURE);
        }
    }
    uint32_t addr = (parts[0] << 24) | (parts[1] << 16) | (parts[2] << 8) | parts[3];
    return htonl(addr);
}



int        multplixing::close_fd(int fd)
{
    std::cout << "Client " << fd << " Was Removed From Map\n";
    std::cout << "it is Done\n";

    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, fd , NULL);
    close(fd);
    fd_maps.erase(fd_maps.find(fd));
    // exit(120);
    return 1;
}

int     multplixing::string_to_int(std::string str)
{
    return (atoi(str.c_str()));////////////////
}

void        multplixing::lanch_server(server parse)
{
    int bytesRead;
    int respo;
    request     rq;
    response     resp_;
    std::vector<server*>::iterator it;
    time_t  start;
    time_t  end;

    epoll_fd = epoll_create(5);
    for (it = parse.s.begin(); it != parse.s.end(); it++)
    {
        int sockfd;
        if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
            perror("Socket creation failed");
            exit(EXIT_FAILURE);
        }
        serverSocket.push_back(sockfd);
        server_history[sockfd] = it;

        sockaddr_in sock_info;

        sock_info.sin_family = AF_INET;
        sock_info.sin_port = htons(string_to_int((*it)->cont["listen"]));
        uint32_t ip = convertIpv4toBinary((*it)->cont["host"]);
        sock_info.sin_addr.s_addr = ip;
        std::cout << "Ip Address : " << inet_ntoa(sock_info.sin_addr) << std::endl;
        int sp = 1;
        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR|SO_REUSEPORT, &sp, sizeof(sp));
        if (bind(sockfd, (struct sockaddr *)&sock_info, sizeof(sock_info))) {
            perror("Bind failed");
            exit(EXIT_FAILURE);
        }
        if (listen(sockfd, 5)) {
            perror("Listen failed");
            exit(EXIT_FAILURE);
        }

        int flag;
    
        if ((flag = fcntl(sockfd, F_GETFL, 0)) < 0) {
            perror("Failed getting file status");
            exit(EXIT_FAILURE);
        }
        flag |= O_NONBLOCK;
        if (fcntl(sockfd, F_SETFL, flag) < 0) {
            perror("Failed setting file status");
            exit(EXIT_FAILURE);
        }

        
        struct epoll_event envts;
        envts.data.fd = sockfd;
        envts.events = EPOLLIN;
    
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, sockfd, &envts);

        std::cout << "Server is listening on port '" << (*it)->cont["listen"] << "'...\n";
    }

    while (true) 
    {
        signal(SIGPIPE, SIG_IGN); // magic this line ignore sigpip when you write to close fd the program exit by sigpip sign
        std::string buffer;
        std::vector<int>::iterator it;

        signal(SIGPIPE, SIG_IGN); // magic this line ignore sigpip when you write to close fd the program exit by sigpip sign

        int num = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        for (int i = 0; i < num; i++) {
            if ((it = std::find(serverSocket.begin(), serverSocket.end(), events[i].data.fd)) != serverSocket.end()) {
                std::cout << "BEFORE CLIENT FD VALUE :" << events[i].data.fd << std::endl;
                std::cout << "New Client Connected\n";
                int client_socket = accept(*it, NULL, NULL);
                struct epoll_event envts_client;
                envts_client.data.fd = client_socket;
                envts_client.events = EPOLLIN | EPOLLOUT | EPOLLERR | EPOLLRDHUP | EPOLLHUP ;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &envts_client) == -1)
                {
                    close(client_socket);
                    perror("Issue In Adding Client To Epoll");
                    exit(1);
                }
                fd_maps[client_socket] = Client();
                fd_maps[client_socket].serv_      = parse;
                client_history[client_socket] = *it;
                start = time(NULL);
                std::cout << "AFTER CLIENT FD VALUE :" << events[i].data.fd << std::endl;
                std::cout << "Client " << client_socket << " Added To Map\n";
            }
            else {
                std::map<int, Client>::iterator it_fd = fd_maps.find(events[i].data.fd);
                end = time(NULL);
                if (difftime(end, start) > 4) {
                    std::cerr << "TIMEOUT" << std::endl;
                    exit(111);
                }
                std::cout << "Client with an event :" << events[i].data.fd << std::endl;
                if (events[i].events & EPOLLRDHUP || events[i].events & EPOLLERR  || events[i].events & EPOLLHUP) 
                {
                    if (close_fd( events[i].data.fd ))
                        continue ;
                }
                else if (events[i].events & EPOLLIN)
                {
                    std::cout << "FD READY TO READ -_- = " << events[i].data.fd << " \n";
                    buffer.resize(BUFFER_SIZE);
                    bytesRead = recv(events[i].data.fd , &buffer[0], BUFFER_SIZE, 0);
                    std::cout << "\n\n\t -> bytesRead ==== " << bytesRead << std::endl;
                    if (bytesRead > 0)
                        buffer.resize(bytesRead);
                    if (bytesRead <= 0)
                    {
                       if (close_fd( events[i].data.fd ))
                            continue ;
                    }
                    std::cout << buffer << "\n";

                    if (flag == 0)
                    {
                        if (buffer.find("\r\n\r\n") != std::string::npos)
                        {
                            rq.parse_req(buffer, parse, events[i].data.fd);
                            fd_maps[events[i].data.fd].requst     = rq;
                            fd_maps[events[i].data.fd].resp       = resp_;
                            // fd_maps[events[i].data.fd].cgi        = cgi_;
                            // std::cout << " stat = " << it_fd->second.not_allow_method << "\n"; Segv
                            // if (it_fd->second.not_allow_method || it_fd->second.version_not_suported)
                            // {
                            //     it_fd->second.not_allow_method      = 0;
                            //     it_fd->second.version_not_suported  = 0;
                            //     if (close_fd( events[i].data.fd ))
                            //         continue ;
                            // }
                            // flag = 1;
                        }
                    }
                    // exit (22);
                    if (rq.method == "POST" && flag == 1 && !it_fd->second.not_allow_method)
                    {
                        fd_maps[events[i].data.fd].post_.j = 0;
                        if (fd_maps[events[i].data.fd].post_.post_method(buffer, rq)  && !it_fd->second.not_allow_method)
                            fd_maps[events[i].data.fd].post_.j = 1;
                    }
                    fd_maps[events[i].data.fd].u_can_send = 1;
                            // exit (102);
                    // std::cout << "request_ uri = " << it_fd ->second.requst.uri << "\n";
                    // if (!it_fd->second.cgi.cgi_stat.compare("cgi_state"))
                    // {
                    //     it_fd->second.resp.response_error("415", events[i].data.fd);
                    //     close_fd(events[i].data.fd);
                    //     continue ;
                    // }
                }
                else if (events[i].events & EPOLLOUT && !it_fd->second.rd_done && it_fd->second.u_can_send) // must not always enter to here i think ask about it 
                {
                    // flag = 0;
                    std::cout << "ready  writing " << " \n";
                    respo = 0;
                    if (!fd_maps[events[i].data.fd].requst.method.compare("GET"))
                        respo = (*it_fd).second.get.get_mthod(events[i].data.fd);
                    else if (!fd_maps[events[i].data.fd].requst.method.compare("DELETE"))
                    {
                        std:: string res_delete = (*it_fd).second.delet.delet_method((*it_fd).second.requst.uri, (*it_fd).second.serv_, events[i].data.fd);
                        if (!res_delete.compare("delete_ok"))
                            respo = 1;
                        else if (!res_delete.compare("delete"))
                        {
                            if (it_fd->second.resp.response_error("204", events[i].data.fd))
                                respo = 1;
                        }
                    }
                    else if (!fd_maps[events[i].data.fd].requst.method.compare("POST") && fd_maps[events[i].data.fd].post_.j)
                    {
                        std::string response = "HTTP/1.1 201 OK\r\nContent-Type: text/html\r\n\r\nhello";
                        send(events[i].data.fd,response.c_str(), response.length(), 0);
                        respo = 1;
                    }
                    std::cout << "\t\t stat kaml wla ba9i == "      << it_fd->second.rd_done << std::endl;
                    std::cout << "\t\t second.not_allow_method == " << it_fd->second.not_allow_method<< std::endl;
                    if (respo || it_fd->second.not_allow_method)
                    {
                        it_fd->second.not_allow_method = 0;
                        std::cout << "\t\t SF KAML GHADI UTM7A HAD "  << events[i].data.fd << std::endl;
                        if (close_fd( events[i].data.fd ))
                            continue ;
                    }
                }
            }
        }
    }
}


