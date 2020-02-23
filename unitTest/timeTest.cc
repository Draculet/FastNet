#include "../Eventloop.h"
#include <iostream>
using namespace std;
using namespace net;

void callback(string str)
{
    cout << "Time out: " << str << endl;
}

int main(void)
{
    Eventloop loop;
    loop.runTimeAfter(10, bind(callback, "default"));
    //loop.runTimeAfter(11, bind(callback, "hello"), "hello");
    //loop.runTimeAfter(11.5, bind(callback, "hello3"), "hello3");
    //loop.runTimeAfter(11, bind(callback, "hello2"), "hello2");
    //loop.runTimeAfter(11.000001, bind(callback, "hello5"), "hello5");
    //loop.runTimeAfter(13, bind(callback, "hello4"), "hello4");
    //sleep(15);
    loop.runTimeAfter(10.1, bind(callback, "default"));
    //loop.runTimeEach(5, bind(callback, "hello4"), "hello4");
    //loop.runTimeEach(10, bind(callback, "hello6"), "hello6");
    //sleep(10);
    //loop.runTimeEach(10, bind(callback, "hello6"), "hello6");
    //loop.runTimeEach(2, bind(callback, "hello7"), "hello7");
    //loop.runTimeEach(10, bind(callback, "hello4"), "hello4");
    //sleep(6);
    //loop.runTimeEach(10, bind(callback, "hello6"), "hello6");
    loop.loop();
}