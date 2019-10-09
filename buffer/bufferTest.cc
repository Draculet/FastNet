#include "Buffer.h"
#include <iostream>
using namespace std;
using namespace net;

int main(void)
{
    Buffer b;
    char arr[1024 * 1024 * 7 + 1000];
    memset(arr, 89, sizeof arr);
    b.append(arr, 1);
    //b.debug();
    for (int i = 0; i < 10; i++)
    {
       b.append(arr, 1024 * 1024 * 7);
        
    }
    //.debug();
    //cout << b.retrieveAllAsString() << endl;
    cout << b.retrieveAsString(3) << endl;
    b.debug();
    cout << "readable = " << b.readable() << endl;
    //cout << 
    b.retrieveAsString(1024 * 1024 * 80);
    // << endl;
    b.debug();
    b.append(arr, 100);
    b.debug();
    b.append(arr, 2000);
    b.debug();
    printf("=============================END=======================\n");
    Buffer b2;
    char arr2[12] = "hello world";
    b2.append(arr, 1000);
    b2.retrieve(900);
    b2.retrieve(99);
    b2.debug();
    b2.append(arr2, 11);
    cout << b2.preViewAsString(b2.readable()) << endl;
    b2.debug();
    b2.append(arr2, 11);
    cout << b2.preViewAsString(b2.readable()) << endl;
    b2.debug();
    b2.append(arr2, 11);
    b2.retrieve(1);
    cout << b2.preViewAsString(b2.readable()) << endl;
    b2.debug();
    size_t n = b2.readable() + 6530000;
    b2.prepend((char *)&n, 4);
    b2.debug();
    string s = b2.retrieveAsString(4);
    size_t n2 = 100231;
    memcpy(&n2, s.c_str(), 4);
    cout << n2 << endl;
    b2.debug();
}

