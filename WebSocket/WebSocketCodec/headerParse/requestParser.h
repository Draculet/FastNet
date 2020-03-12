#ifndef __REQUESE_PARSER_H__
#define __REQUESE_PARSER_H__

#include "../../../FastNet/include/Buffer.h"
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <map>


namespace http
{
    struct resp
    {
        resp(char *resptr, size_t len):
            resptr_(resptr),
            len_(len)
        {
            
        }
        char *resptr_;
        size_t len_;
    };

    class requestHeader
    {
        public:
            requestHeader(std::string method, std::string path, std::string version, std::string body, std::map<std::string, std::string>reqkv);
            resp toResp(std::string basepath);
            std::string toString();
            std::map<std::string, std::string> *getKV(){return &reqkv_;}
        private:
            std::string method_;
            std::string path_;
            std::string version_;
            std::string body_;
            std::map<std::string, std::string> reqkv_;
            std::map<std::string, std::string> type_;
            std::map<std::string, std::string> stat_;
            std::string state_;
            int length_;
            char resp_[1024*1024];
            size_t resplen_;
            char fstr_[1024 * 1024];
    };

    class requestParser
    {
        public:
        requestParser(net::Buffer *buf):
            buf_(buf),
            kfin(true)
        {

        }
        size_t size(){return reqs.size();}
        std::vector<requestHeader> reqs;
        size_t parse();
        bool parseBuffer();
        void parseHead(std::string);
        private:
        net::Buffer *buf_;
        std::string method_;
        std::string path_;
        std::string version_;
        std::string body_;
        std::map<std::string, std::string> reqkv_;
        bool kfin = true;
    };

}

#endif