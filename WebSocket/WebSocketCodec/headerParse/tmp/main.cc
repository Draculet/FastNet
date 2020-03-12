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
class requestHeader
{
    public:
    requestHeader(string method, string path, string version, string body, map<string, string>reqkv):
        method_(method),
        path_(path),
        version_(version),
        body_(body),
        reqkv_(reqkv)
    {
        type_["css"] = "text/css";
        type_["js"] = "application/javascript";
        type_["html"] = "text/html";//TODO 指定charset=GBK ?
        type_["jpg"] = "image/jpeg";
        type_["png"] = "image/png";
        type_["gif"] = "image/gif";
        type_["ico"] = "image/x-icon";
        type_["json"] = "text/json";
        stat_["200"] = "OK";
        stat_["304"] = "Not Modified";
        stat_["404"] = "Not Found";
    }
    void toResp(string basepath)
    {
        string path = basepath + path_;
        char fsize_c[10] = {0};
        cout << path << endl;
        int pos = path_.find(".");
        string type = string(path_, pos + 1);
        cout << type << endl;
        int ffd = open(path.c_str(), O_RDONLY);
        if (ffd == -1)
        {
            state_ = "404";
            cout << "Not Found" << endl;
        }
        else
        {
            int fsize = 0;
            struct stat st;
            if (fstat(ffd, &st) == 0)
            {
                fsize = st.st_size;
                char fbuf[2 * fsize] = {0};
                int ret = read(ffd, fbuf, fsize * 2);
                fstr_ = string(fbuf, 0, fsize);
                cout << fstr_ << "EOF" << endl;
                state_ = "200";
                sprintf(fsize_c, "%d", fsize);
            }
            else
            {
                state_ = "404";
                //TODO 错误处理
            }
        }
        if (state_ == "200")
        {
            resp_ = version_ + " " + state_ + " " + stat_[state_] + "\n";
            resp_ += "Server: My Server\n";
            resp_ = resp_ + "Content-Type: " + type_[type] + "\n";
            resp_ = resp_ + "Content-Length: " + string(fsize_c) + "\n\n";
            resp_ = resp_ + fstr_;
            cout << resp_ << endl;
        }
        else if (state_ == "404")
        {
            string s = "404 Not Found";
            sprintf(fsize_c, "%d", s.size());
            resp_ = version_ + " " + state_ + " " + stat_[state_] + "\n";
            resp_ += "Server: My Server\n";
            resp_ = resp_ + "Content-Type: text/html\n";
            resp_ = resp_ + "Content-Length: " + string(fsize_c) + "\n\n";
            resp_ = resp_ + s;
            cout << resp_ << endl;
        }
    }
    private:
        string method_;
        string path_;
        string version_;
        string body_;
        map<string, string> reqkv_;
        map<string, string> type_;
        map<string, string> stat_;
        string state_;
        int length_;
        string resp_;
        string fstr_;
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
    char fbuf[1024 * 1024] = {0};
    int ffd = open("../header/requestReal.txt", O_RDONLY);
    int ret = read(ffd, fbuf, 1024 * 1024);
    char *ptr = fbuf;
    printf("size: %d\n", ret);
    printf("content: %s\n", fbuf);
    Buffer buf;
    buf.append(fbuf, ret);
    requestParser reqhead(&buf);
    reqhead.parseBuffer();
    reqhead.reqs[0].toResp("../webapp");
}