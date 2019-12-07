#include "../webSocketCodec.h"
#include <iostream>
using namespace std;

int main(void)
{
    uint16_t num = 126;
    num = htons(num);
    uint64_t num2 = 7000;
    num2 = ntoh64t(num2);
    char buf[1024 * 1024] = {0};
    buf[0] = 0x81;
    buf[1] = 0b11111110;
    //buf[2] = 128;
    //buf[3] = 128;
    memcpy(buf + 2, &num, 2);
    printf("%d %d \n", buf[2], buf[3]);
    buf[4] = 'a';
    buf[5] = 'b';
    buf[6] = 'c';
    buf[7] = 'd';
    
    Buffer buffer;
    webSocketCodec codec(&buffer);
    int cur = 0;
    while (1)
    {
        buffer.append(buf + cur, 1);
        int num = codec.parse();
        printf("codec ret %d\n", num);
        for (int i = 0; i < num; i++)
        {
            codec.frames[i].debugPrint();
        }
        cur++;
        if (num == 1)
        {
            sleep(5);
            buf[cur] = 0x81;
            buf[cur + 1] = 0b11111111;
            memcpy(buf + cur + 2, &num2, 8);
            buf[cur + 10] = 'a';
            buf[cur + 11] = 'b';
            buf[cur + 12] = 'c';
            buf[cur + 13] = 'd';
            codec.frames.clear();
        }
        //sleep(1);
    }
}