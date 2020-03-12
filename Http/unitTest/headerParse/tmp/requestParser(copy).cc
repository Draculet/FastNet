#include "../Buffer.h"
#include <iostream>
#include <vector>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h> 
#include <map>

using namespace std;
using namespace net;
struct requestHeader
{
    requestHeader(string method, string path, string version, string body, map<string, string>reqkv):
        method_(method),
        path_(path),
        version_(version),
        body_(body),
        reqkv_(reqkv)
    {

    }
    private:
        string method_;
        string path_;
        string version_;
        string body_;
        map<string, string> reqkv_;
};

class requestParser
{
    public:
    requestParser(Buffer *buf):
        buf_(buf),
        kfin(true)
        {

        }
    
    size_t size(){return reqs.size();}
    vector<requestHeader> reqs;
    bool parseBuffer();
    void parseHead(string);
    private:
    Buffer *buf_;
    string method_;
    string path_;
    string version_;
    string body_;
    map<string, string> reqkv_;
    bool kfin = true;
};

bool requestParser::parseBuffer()
{
    //处理请求body不完整的情况
    if (kfin == false)
    {
        auto iter = reqkv_.find("Content-Length");//有buf中只有一半body的可能
        if (iter != reqkv_.end())
        {
            //cout << iter->second << endl;
            int n = atoi(iter->second.c_str());
            string bodystr = buf_->preViewAsString(buf_->readable());
            if (bodystr.size() >= n)
            {
                body_ = string(bodystr, 0, n);
                buf_->retrieve(n);
                kfin = true;
            }
            else
            {
                kfin = false;
                printf("请求body不完整\n");
                //函数出口未统一在最后
                return false;
            }
        }
        printf("\n\n不完整body解析完成\n");
        printf("Result:\nmethod: %s\npath: %s\nversion: %s\nbody: %s\n", method_.c_str(), path_.c_str(), version_.c_str(), body_.c_str());
        for (auto &iter : reqkv_)
        {
            printf("key: %s\nvalue: %s\n", iter.first.c_str(), iter.second.c_str());
        }
        reqs.push_back(requestHeader(method_, path_, version_, body_, reqkv_));
        //函数出口未统一在最后
        return true;
    }

    //正常开始处理
    int begin = 0;
    int cur = 0;
    string s = buf_->preViewAsString(buf_->readable());
    int reqtail = s.find("\r\n\r\n", 0);
    if (reqtail != string::npos)
    {
        string headpart = string(s, 0, reqtail);
        //cout << headpart.size() << endl;
        cout << "解析开始" << endl << endl;
        while (true)
        {
            cur = headpart.find("\r\n", begin);
            if (cur != string::npos)
            {
                string line(s, begin, cur - begin);
                //cout << line << endl;
                int ret = line.find(':');
                if (ret == string::npos)
                {
                    parseHead(line);
                }
                else
                {
                    string key = string(line, 0, ret);
                    string value = string(line, ret + 2);
                    reqkv_[key] = value;
                    //cout << "key: " << key << endl;
                    //cout << "value: " << value << endl;
                }
                begin = cur + 2;
            }
            else
                break;
        }
        buf_->retrieve(headpart.size() + 4);
        auto iter = reqkv_.find("Content-Length");//有buf中只有一半body的可能
        if (iter != reqkv_.end())
        {
            //cout << iter->second << endl;
            int n = atoi(iter->second.c_str());
            string bodystr = buf_->preViewAsString(buf_->readable());
            if (bodystr.size() > n)
            {
                body_ = string(s, headpart.size() + 4, n);
                buf_->retrieve(n);
                kfin = true;
            }
            else
            {
                kfin = false;
                //函数出口未统一在最后
                printf("请求body不完整\n");
                return false;
            }
            //debug
        }
        printf("\n\n解析完成\n");
        printf("Result:\nmethod: %s\npath: %s\nversion: %s\nbody: %s\n", method_.c_str(), path_.c_str(), version_.c_str(), body_.c_str());
        for (auto &iter : reqkv_)
        {
            printf("key: %s\nvalue: %s\n", iter.first.c_str(), iter.second.c_str());
        }
        reqs.push_back(requestHeader(method_, path_, version_, body_, reqkv_));
        return true;
    }
    else//如果找不到\r\n\r\n直接返回,等下一次调用
    {
        printf("未找到\\r\\n\\r\\n,header不完整\n");
        return false;
    }
}

void requestParser::parseHead(string line)
{
    int begin = 0;
    int cur = 0;
    cur = line.find(" ", begin);
    method_ = string(line, begin, cur - begin);
    begin = cur + 1;
    cur = line.find(" ", begin);
    path_ = string(line, begin, cur - begin);
    if (path_ == "/")
        path_ = "/index.html";
    begin = cur + 1;
    version_ = string(line, begin);
}

int main(void)
{
    char fbuf[1024 * 1024 * 4] = {0};
    int ffd = open("../header/requestReal.txt", O_RDONLY);
    int ret = read(ffd, fbuf, 1024 * 1024);
    char *ptr = fbuf;
    printf("size: %d\n", ret);
    printf("content: %s\n", fbuf);
    Buffer buf;
    //buf.append(fbuf, 100);
    requestParser reqhead(&buf);
    
    while(1)
    {
        reqhead.parseBuffer();
        if (ptr - fbuf > ret)
            break;
        //sleep(1);
        buf.append(ptr, 1);
        ptr += 1;
    }
}