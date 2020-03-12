#include "Server.h"
using namespace std;
using namespace net;
using namespace base;
using namespace http;

int main(void)
{
    Server s(4, 8832);
    map<shared_ptr<Connection>, WebSocketCodec *> codecMap;
    s.setReadCallback(bind(onRead, &codecMap,  placeholders::_1, placeholders::_2 ));
    s.setDisConnCallback( bind(onDisConnect, &codecMap, placeholders::_1) );
    s.start();
}