#include "Client.hpp"
#include "cgi.hpp"
extern std::map<int, Client> fd_maps;

cgi::cgi(){
    // std::cout << "Cgi Constructor \n";
}

void        cgi::fill_env_cgi(Client &obj)
{
    // SCRIPT_NAME = ;
    // QUERY_STRING    = ;
    // CONTENT_LENGTH  = ;
    idx = 0;
    cgi_env.push_back("REQUEST_METHOD=" + obj.requst.method);
    cgi_env.push_back("CONTENT_TYPE=text/plain");
    cgi_env.push_back("SCRIPT_FILENAME=" + obj.requst.uri);
    cgi_env.push_back("REDIRECT_STATUS=200OK");
    cgi_env.push_back("SERVER_PORT=8080");//change it !!
    std::vector<std::string>::iterator  ite = cgi_env.end();
    for(std::vector<std::string>::iterator  itb = cgi_env.begin(); itb != ite; itb++)
        env[idx++] = strdup(itb->c_str());
    env[idx] = NULL;
}

void        cgi::cgi_work(int fd)
{
    std::map<int, Client>::iterator it = fd_maps.find(fd);
    std::cout <<"hahhahahhahahahha" << it->second.requst.uri << "\n";
    fill_env_cgi(it->second);

    int fd_out = open("file.txt", O_RDWR | O_CREAT, 0666);

    if (fd_out < 1)
        it->second.serv_.print_err("file error");
    if (dup2(fd_out, 1) == -1) 
        it->second.serv_.print_err("dup error");
    close(fd_out);
    // char* pythonScriptPath = requestedFilePath;
    char* pythonInterpreter = strdup("/usr/bin/python3");
    char* args[] = {pythonInterpreter, strdup(it->second.requst.uri.c_str()), NULL};

    if (execve(pythonInterpreter, args, env) == -1) 
        it->second.serv_.print_err("execve error");
    // std::ofstream   cgi_file("file.txt");
    exit (12);
}

// void     cgi::get_exten_type(int fd)
// {s
//     std::cout << "\nCGI Function"<< "\n";
//     std::string exten;
//     size_t      pos = it->second.requst.uri.find_last_of(".");

//     std::cout << "uri = " << it->second.requst.uri << "\n";

//     exten = it->second.requst.uri.substr(pos + 1);

//     std::map<std::string, std::string>::iterator it_bi = it->second.serv_.cgi_map.begin();
//     std::map<std::string, std::string>::iterator it_end = it->second.serv_.cgi_map.end();
//     while (it_bi != it_end)
//     {
//         std::cout << "first = " << it_bi->first << "\n";
//         std::cout << "second = " << it_bi->second << "\n";
//         it_bi++;
//     }
//     exit (21);
//     std::map<std::string, std::string>::iterator b_conf = it->second.serv_.cgi_map.find(exten);
//     std::map<std::string, std::string>::iterator b_uri = it->second.requst.extentions.find(exten);

//     if (b_uri != it->second.requst.extentions.end())
//         cgi_stat = "static_file";
//     if (b_conf != it->second.serv_.cgi_map.end())
//         cgi_stat = "cgi_state";
//     std::cout << "exten == " << exten << "\n";
//     std::cout << "cgi_stat == " << cgi_stat << "\n";
//     exit (20);
// }

void    cgi::cgi_extention()
{
    exten_cgi["py"] = "text/plain";
    exten_cgi["php"] = "text/plain"; 
}

cgi::~cgi(){
}