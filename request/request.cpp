#include "../request.hpp"
#include "../Client.hpp"
extern std::map<int, Client> fd_maps;

int               request::one_of_allowed(std::string mehod, std::vector<std::string> allowed_methods)
{
    std::vector<std::string>::iterator it = allowed_methods.begin();
    std::vector<std::string>::iterator ite = allowed_methods.end();
    if ((*it).compare("allow_methods"))
        it++;
    while (it != ite)
    {
        if (!it->compare(mehod))
            return 1;
        it++;   
    }
    return 0;
}

int            request::check_cgi_exten(std::string exten)
{
    std::map<std::string, std::string>::iterator it_bi = cgi_map.begin();
    std::map<std::string, std::string>::iterator it_end = cgi_map.end();
    while (it_bi != it_end)
    {
        if (!it_bi->first.compare(exten))
        {
            x_cgi = 1;
            return 1;
        }
        it_bi++;
    }
    return 0;
}

void            request::parse_req(std::string   rq, server &server, int fd) // you can remove the server argenent
{
    parse_header(rq, server);
    std::map<int, Client>::iterator it = fd_maps.find(fd);
    int             state;

    last          = rq.find("\r\n");
    vec           = server.isolate_str(rq.substr(0, last) , ' ');
    method        = vec[0];
    path          = vec[1];
    http_version  = vec[2];
    uri = get_full_uri(server, it->second); //
    x = it->second.get.check_exist(uri);
    it->second.resp.response_message = server.response_message;
    std::cout << "Full Path = " << uri << std::endl;
    std::cout << " N checkiw Wach L2mor Tayba wla la " << std::endl;

    if (http_version.compare("HTTP/1.1"))
    {
        state = it->second.resp.response_error("505", fd);    
        it->second.version_not_suported = 1;
        return ;
    }
    if (vec.size() != 3 || last == std::string::npos)
    {
        state = it->second.resp.response_error("505", fd);    
        it->second.version_not_suported = 1;
        return ;
    }
    if ((method.compare("DELETE") && method.compare("POST") && method.compare("GET")) || !method_state)
    {
        state = it->second.resp.response_error("405", fd);
        it->second.not_allow_method = 1;
        return ;
    }
    if (!get_exten_type(uri).compare("Unsupported")/*&& it->second.cgi.cgi_stat.compare("cgi_state")*/)
    {
        state = it->second.resp.response_error("415", fd);
        it->second.not_allow_method = 1;
        return ;
    }
    reset();
}

std::string     request::find_longest_path(server &server, Client &obj)
{
    (void)obj;
    std::string long_path;
    
   for (size_t i = 0; i < server.s.size(); i++) // which server !! so must not implement this for loop
   {
        std::vector<std::string>::iterator it  = server.s[i]->vec_of_locations.begin();
        std::vector<std::string>::iterator ite = server.s[i]->vec_of_locations.end();
        for(; it != ite; it++)
        {
            if (path.find(*it) == 0)
            {
                if (it->length() > long_path.length())
                    long_path = *it;
            }
        }
   }
    std::cout << "long_path = " << long_path << "\n";
    return (long_path);
}

std::string     request:: get_full_uri(server &server, Client& obj)
{

    int     loca_found = 0;
    longest_loca = find_longest_path(server, obj);
    rest_fldr    = path.substr(longest_loca.length()); 
    for (size_t j = 0; j < (*it)->l.size(); j++)
    {
        if (one_of_allowed(method, (*it)->l[j]->allowed_methods))
            method_state = true;
        loca_found = rewrite_location((*it)->l[j]->cont_l);
        if (loca_found)
            break ;
        cgi_map = (*it)->l[j]->cgi_map;
    }
    return full_path;
}

int           request::rewrite_location(std::map<std::string, std::string> location_map)
{

    std::map<std::string, std::string>::iterator      ite = location_map.end();
    for (std::map<std::string, std::string>::iterator itb = location_map.begin(); itb != ite; itb++)
    {
        if ((!(*itb).first.compare("location") && !(*itb).second.compare("/"))) // found bool is false in case location not found !
            root_map = location_map;
        if ((!(*itb).first.compare("location") &&  !itb->second.compare(longest_loca)))
        {
            if ((!(*itb).first.compare("upload")))
                upload_state = (*itb).second;
            if ((!itb->first.compare("cgi_status")))  
                stat_cgi = itb->second;
            found = true;
            auto_index_stat = check_autoindex(location_map);
            std::map<std::string, std::string>::iterator it_b = location_map.find("root");
            if (!rest_fldr.empty()) // rest 3amr
            {
                full_path = (*it_b).second + "/" + rest_fldr;
                check = 1;
                return 1;
            }
            else // add index on specify location
            {
                std::map<std::string, std::string>::iterator indx = location_map.find("index");
                if (indx != location_map.end())
                {
                    full_path = (*it_b).second + "/" + (*indx).second; 
                    check = 1;
                    return 1;
                }
                else
                {
                    full_path = (*it_b).second; 
                    check = 1;
                    return 1;
                }
            }
        }
    }
    return 0;
} 

bool            request::check_autoindex(std::map<std::string, std::string> loca_map)
{
    std::map<std::string, std::string>::iterator it_e = loca_map.end();
    for (std::map<std::string, std::string>::iterator it_b = loca_map.begin(); it_b != it_e; it_b++)
    {

        if (!(*it_b).first.compare("autoindex"))
        {
            if (!(*it_b).second.compare("on"))
                auto_index_stat = true;
            break;
        }
    }
    return (auto_index_stat);
}

void        request::fill_extentions()
{   
    // i think you need more than that 
    extentions["html"] = "text/html"; 
    extentions["txt"]  = "text/plain"; 
    extentions["jpg"] = "image/jpg"; 
    extentions["jpeg"] = "image/jpeg";
    extentions["png"] = "image/png";
    extentions["mp3"] = "audio/mpeg";
    extentions["mp4"] = "video/mp4";
    extentions["webm"] = "video/webm";
    extentions["pdf"] = "application/pdf";
    extentions["zip"] = "application/zip";
    extentions["woff"] = "application/font-woff";
}

void request::reset() 
{
    http_version.clear();
    loca_fldr.clear();
    rest_fldr.clear();
    full_path.clear();
    full_rest.clear();
}

std::string     request::get_exten_type(std::string path)
{
    std::string exten;
    size_t      pos = path.find_last_of(".");
    
    if (pos != std::string::npos)
        exten = path.substr(pos + 1);
    if (pos == std::string::npos && x == 1)
        return ("application/octet-stream");        
    std::map<std::string, std::string>::iterator b = extentions.find(exten);
    if (b != extentions.end())
        return ((*b).second);
    if ((b == extentions.end() ) && !check_cgi_exten(exten) && x == 1)
        return ("Unsupported");
    return "";
}

std::streampos  request::get_fileLenth(std::string path)
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

request::request(/* args */){
    found = false;
    check = false;
    auto_index_stat = false;
    method_state = false;
    redirct_loca = 0;
    loca_fldr = "";
    longest_loca = "";
    rest_fldr = "";
    full_path = "";
    full_rest = "";
    x_cgi = 0;
    x = 0;
    fill_extentions();
}

request::~request(){
    std::cout << "hshshshshshhshshshhshshshshshshhshshshhshsh\n\n\n\n\n";
    reset();
}
