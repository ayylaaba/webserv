#ifndef CGI_HPP
#define CGI_HPP

#include <iostream>
#include <fstream>
#include <string>
#include <iostream>
#include <string>
#include <stdlib.h>
#include <map>
#include <vector>
#include <fstream>
#include "Client.hpp"
#include <fcntl.h>


class cgi
{
    public:
        int                                 idx;
        int                                 stat_cgi;
        void                                cgi_extention();
        void                                cgi_work(int fd);
        std::vector<std::string>            cgi_env;
        // std::map<std::string, std::string>  cgi_env;
        char**                              env;
        std::map<std::string,std::string>   exten_cgi;
        void                                get_exten_type(int fd);
        void                                fill_env_cgi(Client &obj);
        std::string                         cgi_stat;

        // Envirement //
        std::string                         REQUEST_METHOD;
        std::string                         SCRIPT_NAME;
        std::string                         QUERY_STRING;
        std::string                         CONTENT_TYPE;
        std::string                         CONTENT_LENGTH;
        std::string                         SCRIPT_FILENAME;
        std::string                         REDIRECT_STATUS;
        std::string                         SERVER_PORT;
        // ---------- //
        
        cgi();
        ~cgi();
};

#endif
