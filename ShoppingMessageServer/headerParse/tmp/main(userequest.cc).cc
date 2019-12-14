#include "requestParser.h"
#include <iostream>

using namespace std;
using namespace net;
using namespace http;

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
    while (reqhead.parseBuffer())
    {

    }
    if (reqhead.size() > 0)
    {
        for (int i = 0; i < reqhead.size(); i++)
        {
            cout << "\n\nstart:\n";
            cout << reqhead.reqs[i].toResp("../webapp") << endl;
            cout << "\n\nend\n\n";
        }
    }
}